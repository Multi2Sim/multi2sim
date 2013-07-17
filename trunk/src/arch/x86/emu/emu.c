/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include <arch/x86/timing/cpu.h>
#include <driver/glew/glew.h>
#include <driver/glu/glu.h>
#include <driver/glut/glut.h>
#include <driver/opengl/opengl.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "loader.h"
#include "regs.h"
#include "signal.h"
#include "syscall.h"


/*
 * Global variables
 */

/* Configuration parameters */
long long x86_emu_max_inst = 0;
long long x86_emu_max_cycles = 0;
char x86_emu_last_inst_bytes[20];
int x86_emu_last_inst_size = 0;
int x86_emu_process_prefetch_hints = 0;

X86Emu *x86_emu;




/*
 * Class 'X86Emu'
 */

CLASS_IMPLEMENTATION(X86Emu);

void X86EmuCreate(X86Emu *self)
{
	/* Parent */
	EmuCreate(asEmu(self), "x86");

	/* Initialize */
	self->current_pid = 100;
	pthread_mutex_init(&self->process_events_mutex, NULL);

	/* Virtual functions */
	asObject(self)->Dump = X86EmuDump;
	asEmu(self)->DumpSummary = X86EmuDumpSummary;
	asEmu(self)->Run = X86EmuRun;
}


void X86EmuDestroy(X86Emu *self)
{
	X86Context *ctx;

	/* Finish all contexts */
	for (ctx = self->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!X86ContextGetState(ctx, X86ContextFinished))
			X86ContextFinish(ctx, 0);

	/* Free contexts */
	while (self->context_list_head)
		delete(self->context_list_head);
	
}


void X86EmuDump(Object *self, FILE *f)
{
	X86Context *context;
	X86Emu *emu = asX86Emu(self);

	/* Call parent */
	EmuDump(self, f);

	/* More */
	fprintf(f, "List of contexts (shows in any order)\n\n");
	DOUBLE_LINKED_LIST_FOR_EACH(emu, context, context)
		X86ContextDump(asObject(context), f);
}


void X86EmuDumpSummary(Emu *self, FILE *f)
{
	X86Emu *emu = asX86Emu(self);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "Contexts = %d\n", emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);
}


/* Schedule a call to 'X86EmuProcessEvents' */
void X86EmuProcessEventsSchedule(X86Emu *self)
{
	pthread_mutex_lock(&self->process_events_mutex);
	self->process_events_force = 1;
	pthread_mutex_unlock(&self->process_events_mutex);
}


/* Check for events detected in spawned host threads, like waking up contexts or
 * sending signals.
 * The list is only processed if flag 'self->process_events_force' is set. */
void X86EmuProcessEvents(X86Emu *self)
{
	X86Context *ctx, *next;
	long long now = esim_real_time();
	
	/* Check if events need actually be checked. */
	pthread_mutex_lock(&self->process_events_mutex);
	if (!self->process_events_force)
	{
		pthread_mutex_unlock(&self->process_events_mutex);
		return;
	}
	
	/* By default, no subsequent call to 'X86EmuProcessEvents' is assumed */
	self->process_events_force = 0;

	/*
	 * LOOP 1
	 * Look at the list of suspended contexts and try to find
	 * one that needs to be waken up.
	 */
	for (ctx = self->suspended_list_head; ctx; ctx = next)
	{
		/* Save next */
		next = ctx->suspended_list_next;

		/* Context is suspended in 'nanosleep' system call. */
		if (X86ContextGetState(ctx, X86ContextNanosleep))
		{
			unsigned int rmtp = ctx->regs->ecx;
			unsigned long long zero = 0;
			unsigned int sec, usec;
			unsigned long long diff;

			/* If 'X86EmuHostThreadSuspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Timeout expired */
			if (ctx->wakeup_time <= now)
			{
				if (rmtp)
					mem_write(ctx->mem, rmtp, 8, &zero);
				x86_sys_debug("syscall 'nanosleep' - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextNanosleep);
				continue;
			}

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				if (rmtp)
				{
					diff = ctx->wakeup_time - now;
					sec = diff / 1000000;
					usec = diff % 1000000;
					mem_write(ctx->mem, rmtp, 4, &sec);
					mem_write(ctx->mem, rmtp + 4, 4, &usec);
				}
				ctx->regs->eax = -EINTR;
				x86_sys_debug("syscall 'nanosleep' - interrupted by signal (pid %d)\n", ctx->pid);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextNanosleep);
				continue;
			}

			/* No event available, launch 'X86EmuHostThreadSuspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, X86EmuHostThreadSuspend, ctx))
				fatal("syscall 'poll': could not create child thread");
			continue;
		}

		/* Context suspended in 'rt_sigsuspend' system call */
		if (X86ContextGetState(ctx, X86ContextSigsuspend))
		{
			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				X86ContextCheckSignalHandlerIntr(ctx);
				ctx->signal_mask_table->blocked = ctx->signal_mask_table->backup;
				x86_sys_debug("syscall 'rt_sigsuspend' - interrupted by signal (pid %d)\n", ctx->pid);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextSigsuspend);
				continue;
			}

			/* No event available. The context will never awake on its own, so no
			 * 'X86EmuHostThreadSuspend' is necessary. */
			continue;
		}

		/* Context suspended in 'poll' system call */
		if (X86ContextGetState(ctx, X86ContextPoll))
		{
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0;
			struct x86_file_desc_t *fd;
			struct pollfd host_fds;
			int err;

			/* If 'X86EmuHostThreadSuspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Get file descriptor */
			fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'poll': invalid 'wakeup_fd'");

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				X86ContextCheckSignalHandlerIntr(ctx);
				x86_sys_debug("syscall 'poll' - interrupted by signal (pid %d)\n", ctx->pid);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextPoll);
				continue;
			}

			/* Perform host 'poll' call */
			host_fds.fd = fd->host_fd;
			host_fds.events = ((ctx->wakeup_events & 4) ? POLLOUT : 0) | ((ctx->wakeup_events & 1) ? POLLIN : 0);
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'poll': unexpected error in host 'poll'");

			/* POLLOUT event available */
			if (ctx->wakeup_events & host_fds.revents & POLLOUT)
			{
				revents = POLLOUT;
				mem_write(ctx->mem, prevents, 2, &revents);
				ctx->regs->eax = 1;
				x86_sys_debug("syscall poll - continue (pid %d) - POLLOUT occurred in file\n", ctx->pid);
				x86_sys_debug("  retval=%d\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextPoll);
				continue;
			}

			/* POLLIN event available */
			if (ctx->wakeup_events & host_fds.revents & POLLIN)
			{
				revents = POLLIN;
				mem_write(ctx->mem, prevents, 2, &revents);
				ctx->regs->eax = 1;
				x86_sys_debug("syscall poll - continue (pid %d) - POLLIN occurred in file\n", ctx->pid);
				x86_sys_debug("  retval=%d\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextPoll);
				continue;
			}

			/* Timeout expired */
			if (ctx->wakeup_time && ctx->wakeup_time < now)
			{
				revents = 0;
				mem_write(ctx->mem, prevents, 2, &revents);
				x86_sys_debug("syscall poll - continue (pid %d) - time out\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextPoll);
				continue;
			}

			/* No event available, launch 'X86EmuHostThreadSuspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, X86EmuHostThreadSuspend, ctx))
				fatal("syscall 'poll': could not create child thread");
			continue;
		}


		/* Context suspended in a 'write' system call  */
		if (X86ContextGetState(ctx, X86ContextWrite))
		{
			struct x86_file_desc_t *fd;
			int count, err;
			uint32_t pbuf;
			void *buf;
			struct pollfd host_fds;

			/* If 'X86EmuHostThreadSuspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				X86ContextCheckSignalHandlerIntr(ctx);
				x86_sys_debug("syscall 'write' - interrupted by signal (pid %d)\n", ctx->pid);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextWrite);
				continue;
			}

			/* Get file descriptor */
			fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'write': invalid 'wakeup_fd'");

			/* Check if data is ready in file by polling it */
			host_fds.fd = fd->host_fd;
			host_fds.events = POLLOUT;
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'write': unexpected error in host 'poll'");

			/* If data is ready in the file, wake up context */
			if (host_fds.revents) {
				pbuf = ctx->regs->ecx;
				count = ctx->regs->edx;
				buf = xmalloc(count);
				mem_read(ctx->mem, pbuf, count, buf);

				count = write(fd->host_fd, buf, count);
				if (count < 0)
					fatal("syscall 'write': unexpected error in host 'write'");

				ctx->regs->eax = count;
				free(buf);

				x86_sys_debug("syscall write - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextWrite);
				continue;
			}

			/* Data is not ready to be written - launch 'X86EmuHostThreadSuspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, X86EmuHostThreadSuspend, ctx))
				fatal("syscall 'write': could not create child thread");
			continue;
		}

		/* Context suspended in 'read' system call */
		if (X86ContextGetState(ctx, X86ContextRead))
		{
			struct x86_file_desc_t *fd;
			uint32_t pbuf;
			int count, err;
			void *buf;
			struct pollfd host_fds;

			/* If 'X86EmuHostThreadSuspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				X86ContextCheckSignalHandlerIntr(ctx);
				x86_sys_debug("syscall 'read' - interrupted by signal (pid %d)\n", ctx->pid);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextRead);
				continue;
			}

			/* Get file descriptor */
			fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'read': invalid 'wakeup_fd'");

			/* Check if data is ready in file by polling it */
			host_fds.fd = fd->host_fd;
			host_fds.events = POLLIN;
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'read': unexpected error in host 'poll'");

			/* If data is ready, perform host 'read' call and wake up */
			if (host_fds.revents)
			{
				pbuf = ctx->regs->ecx;
				count = ctx->regs->edx;
				buf = xmalloc(count);
				
				count = read(fd->host_fd, buf, count);
				if (count < 0)
					fatal("syscall 'read': unexpected error in host 'read'");

				ctx->regs->eax = count;
				mem_write(ctx->mem, pbuf, count, buf);
				free(buf);

				x86_sys_debug("syscall 'read' - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextRead);
				continue;
			}

			/* Data is not ready. Launch 'X86EmuHostThreadSuspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, X86EmuHostThreadSuspend, ctx))
				fatal("syscall 'read': could not create child thread");
			continue;
		}

		/* Context suspended in a 'waitpid' system call */
		if (X86ContextGetState(ctx, X86ContextWaitpid))
		{
			X86Context *child;
			uint32_t pstatus;

			/* A zombie child is available to 'waitpid' it */
			child = X86ContextGetZombie(ctx, ctx->wakeup_pid);
			if (child)
			{
				/* Continue with 'waitpid' system call */
				pstatus = ctx->regs->ecx;
				ctx->regs->eax = child->pid;
				if (pstatus)
					mem_write(ctx->mem, pstatus, 4, &child->exit_code);
				X86ContextSetState(child, X86ContextFinished);

				x86_sys_debug("syscall waitpid - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextWaitpid);
				continue;
			}

			/* No event available. Since this context won't wake up on its own, no
			 * 'X86EmuHostThreadSuspend' is needed. */
			continue;
		}

		/* Context suspended in a system call using a custom wake up check call-back
		 * function. NOTE: this is a new mechanism. It'd be nice if all other system
		 * calls started using it. It is nicer, since it allows for a check of wake up
		 * conditions together with the system call itself, without having distributed
		 * code for the implementation of a system call (e.g. 'read'). */
		if (X86ContextGetState(ctx, X86ContextCallback))
		{
			assert(ctx->can_wakeup_callback_func);
			if (ctx->can_wakeup_callback_func(ctx, ctx->can_wakeup_callback_data))
			{
				/* Set context status to 'running' again. */
				X86ContextClearState(ctx, X86ContextSuspended | X86ContextCallback);

				/* Call wake up function */
				if (ctx->wakeup_callback_func)
					ctx->wakeup_callback_func(ctx, ctx->wakeup_callback_data);

				/* Reset call-back info */
				ctx->wakeup_callback_func = NULL;
				ctx->wakeup_callback_data = NULL;
				ctx->can_wakeup_callback_func = NULL;
				ctx->can_wakeup_callback_data = NULL;
			}
			continue;
		}
	}


	/*
	 * LOOP 2
	 * Check list of all contexts for expired timers.
	 */
	for (ctx = self->context_list_head; ctx; ctx = ctx->context_list_next)
	{
		int sig[3] = { 14, 26, 27 };  /* SIGALRM, SIGVTALRM, SIGPROF */
		int i;

		/* If there is already a 'ke_host_thread_timer' running, do nothing. */
		if (ctx->host_thread_timer_active)
			continue;

		/* Check for any expired 'itimer': itimer_value < now
		 * In this case, send corresponding signal to process.
		 * Then calculate next 'itimer' occurrence: itimer_value = now + itimer_interval */
		for (i = 0; i < 3; i++ )
		{
			/* Timer inactive or not expired yet */
			if (!ctx->itimer_value[i] || ctx->itimer_value[i] > now)
				continue;

			/* Timer expired - send a signal.
			 * The target process might be suspended, so the host thread is canceled, and a new
			 * call to 'X86EmuProcessEvents' is scheduled. Since 'ke_process_events_mutex' is
			 * already locked, the thread-unsafe version of 'x86_ctx_host_thread_suspend_cancel' is used. */
			X86ContextHostThreadSuspendCancelUnsafe(ctx);
			self->process_events_force = 1;
			x86_sigset_add(&ctx->signal_mask_table->pending, sig[i]);

			/* Calculate next occurrence */
			ctx->itimer_value[i] = 0;
			if (ctx->itimer_interval[i])
				ctx->itimer_value[i] = now + ctx->itimer_interval[i];
		}

		/* Calculate the time when next wakeup occurs. */
		ctx->host_thread_timer_wakeup = 0;
		for (i = 0; i < 3; i++)
		{
			if (!ctx->itimer_value[i])
				continue;
			assert(ctx->itimer_value[i] >= now);
			if (!ctx->host_thread_timer_wakeup || ctx->itimer_value[i] < ctx->host_thread_timer_wakeup)
				ctx->host_thread_timer_wakeup = ctx->itimer_value[i];
		}

		/* If a new timer was set, launch ke_host_thread_timer' again */
		if (ctx->host_thread_timer_wakeup)
		{
			ctx->host_thread_timer_active = 1;
			if (pthread_create(&ctx->host_thread_timer, NULL, X86ContextHostThreadTimer, ctx))
				fatal("%s: could not create child thread", __FUNCTION__);
		}
	}


	/*
	 * LOOP 3
	 * Process pending signals in running contexts to launch signal handlers
	 */
	for (ctx = self->running_list_head; ctx; ctx = ctx->running_list_next)
	{
		X86ContextCheckSignalHandler(ctx);
	}

	
	/* Unlock */
	pthread_mutex_unlock(&self->process_events_mutex);
}


int X86EmuRun(Emu *self)
{
	X86Emu *emu = asX86Emu(self);
	X86Context *ctx;

	/* Stop if there is no context running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (x86_emu_max_inst && asEmu(self)->instructions >= x86_emu_max_inst)
		esim_finish = esim_finish_x86_max_inst;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = emu->running_list_head; ctx; ctx = ctx->running_list_next)
		X86ContextExecute(ctx);

	/* Free finished contexts */
	while (emu->finished_list_head)
		delete(emu->finished_list_head);

	/* Process list of suspended contexts */
	X86EmuProcessEvents(emu);

	/* Still running */
	return TRUE;
}


/* Search a context based on its PID */
X86Context *X86EmuGetContext(X86Emu *self, int pid)
{
	X86Context *context;

	context = self->context_list_head;
	while (context && context->pid != pid)
		context = context->context_list_next;
	return context;
}


void X86EmuLoadContextsFromConfig(X86Emu *self, struct config_t *config, char *section)
{
	X86Context *ctx;
	struct x86_loader_t *loader;

	char buf[MAX_STRING_SIZE];

	char *exe;
	char *cwd;
	char *args;
	char *env;

	char *in;
	char *out;

	char *config_file_name;

	/* Get configuration file name for errors */
	config_file_name = config_get_file_name(config);

	/* Create new context */
	ctx = new(X86Context, self);
	loader = ctx->loader;

	/* Executable */
	exe = config_read_string(config, section, "Exe", "");
	exe = str_set(NULL, exe);
	if (!*exe)
		fatal("%s: [%s]: invalid executable", config_file_name,
			section);

	/* Arguments */
	args = config_read_string(config, section, "Args", "");
	linked_list_add(loader->args, exe);
	X86ContextAddArgsString(ctx, args);

	/* Environment variables */
	env = config_read_string(config, section, "Env", "");
	X86ContextAddEnv(ctx, env);

	/* Current working directory */
	cwd = config_read_string(config, section, "Cwd", "");
	if (*cwd)
		loader->cwd = str_set(NULL, cwd);
	else
	{
		/* Get current directory */
		loader->cwd = getcwd(buf, sizeof buf);
		if (!loader->cwd)
			panic("%s: buffer too small", __FUNCTION__);

		/* Duplicate string */
		loader->cwd = str_set(NULL, loader->cwd);
	}

	/* Standard input */
	in = config_read_string(config, section, "Stdin", "");
	loader->stdin_file = str_set(NULL, in);

	/* Standard output */
	out = config_read_string(config, section, "Stdout", "");
	loader->stdout_file = str_set(NULL, out);

	/* Load executable */
	X86ContextLoadExe(ctx, exe);
}


void X86EmuLoadContextFromCommandLine(X86Emu *self, int argc, char **argv)
{
	X86Context *ctx;
	struct x86_loader_t *loader;

	char buf[MAX_STRING_SIZE];

	/* Create context */
	ctx = new(X86Context, self);
	loader = ctx->loader;

	/* Arguments and environment */
	X86ContextAddArgsVector(ctx, argc, argv);
	X86ContextAddEnv(ctx, "");

	/* Get current directory */
	loader->cwd = getcwd(buf, sizeof buf);
	if (!loader->cwd)
		panic("%s: buffer too small", __FUNCTION__);
	loader->cwd = str_set(NULL, loader->cwd);

	/* Redirections */
	loader->stdin_file = str_set(NULL, "");
	loader->stdout_file = str_set(NULL, "");

	/* Load executable */
	X86ContextLoadExe(ctx, argv[0]);
}




/*
 * Non-Class Functions
 */


void x86_emu_init(void)
{
	/* Classes */
	CLASS_REGISTER(X86Emu);
	CLASS_REGISTER(X86Context);

	/* Endian check */
	union
	{
		unsigned int as_uint;
		unsigned char as_uchar[4];
	} endian;
	endian.as_uint = 0x33221100;
	if (endian.as_uchar[0])
		fatal("%s: host machine is not little endian", __FUNCTION__);

	/* Host types */
	M2S_HOST_GUEST_MATCH(sizeof(long long), 8);
	M2S_HOST_GUEST_MATCH(sizeof(int), 4);
	M2S_HOST_GUEST_MATCH(sizeof(short), 2);

	/* Create x86 emulator */
	x86_emu = new(X86Emu);

	/* Initialize */
	x86_asm_init();
	x86_uinst_init();

#ifdef HAVE_OPENGL
	/* GLUT */
	glut_init();
	/* GLEW */
	glew_init();
	/* GLU */
	glu_init();
#endif

	/* OpenGL */
	opengl_init();
}


/* Finalization */
void x86_emu_done(void)
{

#ifdef HAVE_OPENGL
	glut_done();
	glew_done();
	glu_done();
#endif

	/* Finalize OpenGl */
	opengl_done();

	/* End */
	x86_uinst_done();
	x86_asm_done();

	/* Print system call summary */
	if (debug_status(x86_sys_debug_category))
		x86_sys_dump_stats(debug_file(x86_sys_debug_category));

	/* Free emulator */
	delete(x86_emu);
}

