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

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include <arch/common/arch.h>
#include <arch/x86/timing/cpu.h>
#include <driver/glew/glew.h>
#include <driver/glu/glu.h>
#include <driver/glut/glut.h>
#include <driver/opengl/opengl.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "isa.h"
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

struct x86_emu_t *x86_emu;




/*
 * Public functions
 */


/* Initialization */

void x86_emu_init(void)
{
	union
	{
		unsigned int as_uint;
		unsigned char as_uchar[4];
	} endian;

	/* Endian check */
	endian.as_uint = 0x33221100;
	if (endian.as_uchar[0])
		fatal("%s: host machine is not little endian", __FUNCTION__);

	/* Host types */
	M2S_HOST_GUEST_MATCH(sizeof(long long), 8);
	M2S_HOST_GUEST_MATCH(sizeof(int), 4);
	M2S_HOST_GUEST_MATCH(sizeof(short), 2);

	/* Create */
	x86_emu = xcalloc(1, sizeof(struct x86_emu_t));

	/* Initialize */
	x86_sys_init();
	x86_isa_init();

	/* Initialize */
	x86_emu->current_pid = 100;  /* Initial assigned pid */
	
	/* Initialize mutex for variables controlling calls to 'x86_emu_process_events()' */
	pthread_mutex_init(&x86_emu->process_events_mutex, NULL);

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
	struct x86_ctx_t *ctx;

#ifdef HAVE_OPENGL
	glut_done();
	glew_done();
	glu_done();
#endif

	/* Finalize OpenGl */
	opengl_done();

	/* Finish all contexts */
	for (ctx = x86_emu->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!x86_ctx_get_state(ctx, x86_ctx_finished))
			x86_ctx_finish(ctx, 0);

	/* Free contexts */
	while (x86_emu->context_list_head)
		x86_ctx_free(x86_emu->context_list_head);
	
	/* Free */
	free(x86_emu);

	/* End */
	x86_isa_done();
	x86_sys_done();
}


void x86_emu_dump(FILE *f)
{
	struct x86_ctx_t *ctx;

	fprintf(f, "List of contexts (shows in any order)\n\n");
	DOUBLE_LINKED_LIST_FOR_EACH(x86_emu, context, ctx)
		x86_ctx_dump(ctx, f);
}


void x86_emu_dump_summary(FILE *f)
{
	/* Functional simulation */
	fprintf(f, "Contexts = %d\n", x86_emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);
}


/* Schedule a call to 'x86_emu_process_events' */
void x86_emu_process_events_schedule()
{
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	x86_emu->process_events_force = 1;
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
}


/* Function that suspends the host thread waiting for an event to occur.
 * When the event finally occurs (i.e., before the function finishes, a
 * call to 'x86_emu_process_events' is scheduled.
 * The argument 'arg' is the associated guest context. */
static void *x86_emu_host_thread_suspend(void *arg)
{
	struct x86_ctx_t *ctx = (struct x86_ctx_t *) arg;
	long long now = esim_real_time();

	/* Detach this thread - we don't want the parent to have to join it to release
	 * its resources. The thread termination can be observed by atomically checking
	 * the 'ctx->host_thread_suspend_active' flag. */
	pthread_detach(pthread_self());

	/* Context suspended in 'poll' system call */
	if (x86_ctx_get_state(ctx, x86_ctx_nanosleep))
	{
		long long timeout;
		
		/* Calculate remaining sleep time in microseconds */
		timeout = ctx->wakeup_time > now ? ctx->wakeup_time - now : 0;
		usleep(timeout);
	
	}
	else if (x86_ctx_get_state(ctx, x86_ctx_poll))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err, timeout;
		
		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'poll': invalid 'wakeup_fd'");

		/* Calculate timeout for host call in milliseconds from now */
		if (!ctx->wakeup_time)
			timeout = -1;
		else if (ctx->wakeup_time < now)
			timeout = 0;
		else
			timeout = (ctx->wakeup_time - now) / 1000;

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = ((ctx->wakeup_events & 4) ? POLLOUT : 0) | ((ctx->wakeup_events & 1) ? POLLIN : 0);
		err = poll(&host_fds, 1, timeout);
		if (err < 0)
			fatal("syscall 'poll': unexpected error in host 'poll'");
	}
	else if (x86_ctx_get_state(ctx, x86_ctx_read))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'read': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLIN;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'read': unexpected error in host 'poll'");
	}
	else if (x86_ctx_get_state(ctx, x86_ctx_write))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'write': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLOUT;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'write': unexpected error in host 'write'");

	}

	/* Event occurred - thread finishes */
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	x86_emu->process_events_force = 1;
	ctx->host_thread_suspend_active = 0;
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
	return NULL;
}


/* Function that suspends the host thread waiting for a timer to expire,
 * and then schedules a call to 'x86_emu_process_events'. */
static void *x86_emu_host_thread_timer(void *arg)
{
	struct x86_ctx_t *ctx = (struct x86_ctx_t *) arg;
	long long now = esim_real_time();
	struct timespec ts;
	long long sleep_time;  /* In usec */

	/* Detach this thread - we don't want the parent to have to join it to release
	 * its resources. The thread termination can be observed by thread-safely checking
	 * the 'ctx->host_thread_timer_active' flag. */
	pthread_detach(pthread_self());

	/* Calculate sleep time, and sleep only if it is greater than 0 */
	if (ctx->host_thread_timer_wakeup > now)
	{
		sleep_time = ctx->host_thread_timer_wakeup - now;
		ts.tv_sec = sleep_time / 1000000;
		ts.tv_nsec = (sleep_time % 1000000) * 1000;  /* nsec */
		nanosleep(&ts, NULL);
	}

	/* Timer expired, schedule call to 'x86_emu_process_events' */
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	x86_emu->process_events_force = 1;
	ctx->host_thread_timer_active = 0;
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
	return NULL;
}


/* Check for events detected in spawned host threads, like waking up contexts or
 * sending signals.
 * The list is only processed if flag 'x86_emu->process_events_force' is set. */
void x86_emu_process_events()
{
	struct x86_ctx_t *ctx, *next;
	long long now = esim_real_time();
	
	/* Check if events need actually be checked. */
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	if (!x86_emu->process_events_force)
	{
		pthread_mutex_unlock(&x86_emu->process_events_mutex);
		return;
	}
	
	/* By default, no subsequent call to 'x86_emu_process_events' is assumed */
	x86_emu->process_events_force = 0;

	/*
	 * LOOP 1
	 * Look at the list of suspended contexts and try to find
	 * one that needs to be waken up.
	 */
	for (ctx = x86_emu->suspended_list_head; ctx; ctx = next)
	{
		/* Save next */
		next = ctx->suspended_list_next;

		/* Context is suspended in 'nanosleep' system call. */
		if (x86_ctx_get_state(ctx, x86_ctx_nanosleep))
		{
			uint32_t rmtp = ctx->regs->ecx;
			uint64_t zero = 0;
			uint32_t sec, usec;
			uint64_t diff;

			/* If 'x86_emu_host_thread_suspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Timeout expired */
			if (ctx->wakeup_time <= now)
			{
				if (rmtp)
					mem_write(ctx->mem, rmtp, 8, &zero);
				x86_sys_debug("syscall 'nanosleep' - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_nanosleep);
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
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_nanosleep);
				continue;
			}

			/* No event available, launch 'x86_emu_host_thread_suspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, x86_emu_host_thread_suspend, ctx))
				fatal("syscall 'poll': could not create child thread");
			continue;
		}

		/* Context suspended in 'rt_sigsuspend' system call */
		if (x86_ctx_get_state(ctx, x86_ctx_sigsuspend))
		{
			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				x86_signal_handler_check_intr(ctx);
				ctx->signal_mask_table->blocked = ctx->signal_mask_table->backup;
				x86_sys_debug("syscall 'rt_sigsuspend' - interrupted by signal (pid %d)\n", ctx->pid);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_sigsuspend);
				continue;
			}

			/* No event available. The context will never awake on its own, so no
			 * 'x86_emu_host_thread_suspend' is necessary. */
			continue;
		}

		/* Context suspended in 'poll' system call */
		if (x86_ctx_get_state(ctx, x86_ctx_poll))
		{
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0;
			struct x86_file_desc_t *fd;
			struct pollfd host_fds;
			int err;

			/* If 'x86_emu_host_thread_suspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Get file descriptor */
			fd = x86_file_desc_table_entry_get(ctx->file_desc_table, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'poll': invalid 'wakeup_fd'");

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				x86_signal_handler_check_intr(ctx);
				x86_sys_debug("syscall 'poll' - interrupted by signal (pid %d)\n", ctx->pid);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_poll);
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
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_poll);
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
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_poll);
				continue;
			}

			/* Timeout expired */
			if (ctx->wakeup_time && ctx->wakeup_time < now)
			{
				revents = 0;
				mem_write(ctx->mem, prevents, 2, &revents);
				x86_sys_debug("syscall poll - continue (pid %d) - time out\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_poll);
				continue;
			}

			/* No event available, launch 'x86_emu_host_thread_suspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, x86_emu_host_thread_suspend, ctx))
				fatal("syscall 'poll': could not create child thread");
			continue;
		}


		/* Context suspended in a 'write' system call  */
		if (x86_ctx_get_state(ctx, x86_ctx_write))
		{
			struct x86_file_desc_t *fd;
			int count, err;
			uint32_t pbuf;
			void *buf;
			struct pollfd host_fds;

			/* If 'x86_emu_host_thread_suspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				x86_signal_handler_check_intr(ctx);
				x86_sys_debug("syscall 'write' - interrupted by signal (pid %d)\n", ctx->pid);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_write);
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
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_write);
				continue;
			}

			/* Data is not ready to be written - launch 'x86_emu_host_thread_suspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, x86_emu_host_thread_suspend, ctx))
				fatal("syscall 'write': could not create child thread");
			continue;
		}

		/* Context suspended in 'read' system call */
		if (x86_ctx_get_state(ctx, x86_ctx_read))
		{
			struct x86_file_desc_t *fd;
			uint32_t pbuf;
			int count, err;
			void *buf;
			struct pollfd host_fds;

			/* If 'x86_emu_host_thread_suspend' is still running for this context, do nothing. */
			if (ctx->host_thread_suspend_active)
				continue;

			/* Context received a signal */
			if (ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked)
			{
				x86_signal_handler_check_intr(ctx);
				x86_sys_debug("syscall 'read' - interrupted by signal (pid %d)\n", ctx->pid);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_read);
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
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_read);
				continue;
			}

			/* Data is not ready. Launch 'x86_emu_host_thread_suspend' again */
			ctx->host_thread_suspend_active = 1;
			if (pthread_create(&ctx->host_thread_suspend, NULL, x86_emu_host_thread_suspend, ctx))
				fatal("syscall 'read': could not create child thread");
			continue;
		}

		/* Context suspended in a 'waitpid' system call */
		if (x86_ctx_get_state(ctx, x86_ctx_waitpid))
		{
			struct x86_ctx_t *child;
			uint32_t pstatus;

			/* A zombie child is available to 'waitpid' it */
			child = x86_ctx_get_zombie(ctx, ctx->wakeup_pid);
			if (child)
			{
				/* Continue with 'waitpid' system call */
				pstatus = ctx->regs->ecx;
				ctx->regs->eax = child->pid;
				if (pstatus)
					mem_write(ctx->mem, pstatus, 4, &child->exit_code);
				x86_ctx_set_state(child, x86_ctx_finished);

				x86_sys_debug("syscall waitpid - continue (pid %d)\n", ctx->pid);
				x86_sys_debug("  return=0x%x\n", ctx->regs->eax);
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_waitpid);
				continue;
			}

			/* No event available. Since this context won't wake up on its own, no
			 * 'x86_emu_host_thread_suspend' is needed. */
			continue;
		}

		/* Context suspended in a system call using a custom wake up check call-back
		 * function. NOTE: this is a new mechanism. It'd be nice if all other system
		 * calls started using it. It is nicer, since it allows for a check of wake up
		 * conditions together with the system call itself, without having distributed
		 * code for the implementation of a system call (e.g. 'read'). */
		if (x86_ctx_get_state(ctx, x86_ctx_callback))
		{
			assert(ctx->can_wakeup_callback_func);
			if (ctx->can_wakeup_callback_func(ctx, ctx->can_wakeup_callback_data))
			{
				/* Set context status to 'running' again. */
				x86_ctx_clear_state(ctx, x86_ctx_suspended | x86_ctx_callback);

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
	for (ctx = x86_emu->context_list_head; ctx; ctx = ctx->context_list_next)
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
			 * call to 'x86_emu_process_events' is scheduled. Since 'ke_process_events_mutex' is
			 * already locked, the thread-unsafe version of 'x86_ctx_host_thread_suspend_cancel' is used. */
			__x86_ctx_host_thread_suspend_cancel(ctx);
			x86_emu->process_events_force = 1;
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
			if (pthread_create(&ctx->host_thread_timer, NULL, x86_emu_host_thread_timer, ctx))
				fatal("%s: could not create child thread", __FUNCTION__);
		}
	}


	/*
	 * LOOP 3
	 * Process pending signals in running contexts to launch signal handlers
	 */
	for (ctx = x86_emu->running_list_head; ctx; ctx = ctx->running_list_next)
	{
		x86_signal_handler_check(ctx);
	}

	
	/* Unlock */
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
}




/*
 * Functional simulation loop
 */


/* Run one iteration of the x86 emulation loop. Return TRUE if still running. */
int x86_emu_run(void)
{
	struct x86_ctx_t *ctx;

	/* Stop if there is no context running */
	if (x86_emu->finished_list_count >= x86_emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (x86_emu_max_inst && arch_x86->inst_count >= x86_emu_max_inst)
		esim_finish = esim_finish_x86_max_inst;

	/* Stop if maximum number of cycles exceeded */
	if (x86_emu_max_cycles && arch_x86->cycle >= x86_emu_max_cycles)
		esim_finish = esim_finish_x86_max_cycles;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = x86_emu->running_list_head; ctx; ctx = ctx->running_list_next)
		x86_ctx_execute(ctx);

	/* Free finished contexts */
	while (x86_emu->finished_list_head)
		x86_ctx_free(x86_emu->finished_list_head);

	/* Process list of suspended contexts */
	x86_emu_process_events();

	/* Still running */
	return TRUE;
}
