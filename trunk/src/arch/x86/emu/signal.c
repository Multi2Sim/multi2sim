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
#include <signal.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "regs.h"
#include "signal.h"
#include "syscall.h"


/*
 * Public Stuff (no class)
 */

struct str_map_t x86_signal_map =
{
	31, {
		{ "SIGHUP",           1 },
		{ "SIGINT",           2 },
		{ "SIGQUIT",          3 },
		{ "SIGILL",           4 },
		{ "SIGTRAP",          5 },
		{ "SIGABRT",          6 },
		{ "SIGBUS",           7 },
		{ "SIGFPE",           8 },
		{ "SIGKILL",          9 },
		{ "SIGUSR1",         10 },
		{ "SIGSEGV",         11 },
		{ "SIGUSR2",         12 },
		{ "SIGPIPE",         13 },
		{ "SIGALRM",         14 },
		{ "SIGTERM",         15 },
		{ "SIGSTKFLT",       16 },
		{ "SIGCHLD",         17 },
		{ "SIGCONT",         18 },
		{ "SIGSTOP",         19 },
		{ "SIGTSTP",         20 },
		{ "SIGTTIN",         21 },
		{ "SIGTTOU",         22 },
		{ "SIGURG",          23 },
		{ "SIGXCPU",         24 },
		{ "SIGXFSZ",         25 },
		{ "SIGVTALRM",       26 },
		{ "SIGPROF",         27 },
		{ "SIGWINCH",        28 },
		{ "SIGIO",           29 },
		{ "SIGPWR",          30 },
		{ "SIGSYS",          31 }
	}
};


struct str_map_t x86_sigaction_flags_map =
{
	9, {
		{ "SA_NOCLDSTOP",    0x00000001u },
		{ "SA_NOCLDWAIT",    0x00000002u },
		{ "SA_SIGINFO",      0x00000004u },
		{ "SA_ONSTACK",      0x08000000u },
		{ "SA_RESTART",      0x10000000u },
		{ "SA_NODEFER",      0x40000000u },
		{ "SA_RESETHAND",    0x80000000u },
		{ "SA_INTERRUPT",    0x20000000u },
		{ "SA_RESTORER",     0x04000000u }
	}
};


char *x86_signal_name(int signum)
{
	return str_map_value(&x86_signal_map, signum);
}


void x86_sigaction_dump(struct x86_sigaction_t *sim, FILE *f)
{
	fprintf(f, "handler=0x%x, flags=0x%x, restorer=0x%x, mask=0x%llx",
		sim->handler, sim->flags,
		sim->restorer, sim->mask);
}


void x86_sigaction_flags_dump(unsigned int flags, FILE *f)
{
	char buf[0x200];
	str_map_flags(&x86_sigaction_flags_map, flags, buf, 0x200);
	fprintf(f, "%s", buf);
}


int x86_sigset_member(unsigned long long *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return 0;
	return (*sim_sigset & (1ULL << (sig - 1))) > 0;
}


void x86_sigset_add(unsigned long long *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return;
	*sim_sigset |= 1ULL << (sig - 1);
}


void x86_sigset_del(unsigned long long *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return;
	*sim_sigset &= ~(1ULL << (sig - 1));
}


void x86_sigset_dump(unsigned long long sim_sigset, FILE *f)
{
	int i;
	char *comma = "", *name;
	if (sim_sigset == (unsigned long long) -1)
	{
		fprintf(f, "{<all>}");
		return;
	}
	fprintf(f, "{");
	for (i = 1; i <= 64; i++)
	{
		if (x86_sigset_member(&sim_sigset, i))
		{
			if (i < 32)
			{
				name = str_map_value(&x86_signal_map, i);
				fprintf(f, "%s%s", comma, name);
			}
			else
			{
				fprintf(f, "%s%d", comma, i);
			}
			comma = ",";
		}
	}
	fprintf(f, "}");
}




/*
 * Object 'x86_signal_mask_table_t'
 */

struct x86_signal_mask_table_t *x86_signal_mask_table_create(void)
{
	struct x86_signal_mask_table_t *table;

	/* Return */
	table = xcalloc(1, sizeof(struct x86_signal_mask_table_t));
	return table;
}

void x86_signal_mask_table_free(struct x86_signal_mask_table_t *table)
{
	free(table);
}




/*
 * Object 'x86_signal_handler_table_t'
 */

struct x86_signal_handler_table_t *x86_signal_handler_table_create(void)
{
	struct x86_signal_handler_table_t *table;

	/* Return */
	table = xcalloc(1, sizeof(struct x86_signal_handler_table_t));
	return table;
}


void x86_signal_handler_table_free(struct x86_signal_handler_table_t *table)
{
	assert(!table->num_links);
	free(table);
}


struct x86_signal_handler_table_t *x86_signal_handler_table_link(struct x86_signal_handler_table_t *table)
{
	table->num_links++;
	return table;
}


void x86_signal_handler_table_unlink(struct x86_signal_handler_table_t *table)
{
	assert(table->num_links >= 0);
	if (table->num_links)
		table->num_links--;
	else
		x86_signal_handler_table_free(table);
}




/*
 * Class 'X86Context'
 * Additional functions
 */


/* Structure representing the signal stack frame */
struct x86_sigframe
{
	unsigned int pretcode;  /* Pointer to return code */
	unsigned int sig;  /* Received signal */

	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp, esp;
	unsigned int ebx, edx, ecx, eax;
	unsigned int trapno, err, eip, cs;
	unsigned int eflags;
	unsigned int esp_at_signal;
	unsigned int ss;
	unsigned int pfpstate;  /* Pointer to floating-point state */
	unsigned int oldmask;
	unsigned int cr2;
};


/* Signal return code. The return address in the signal handler points
 * to this code, which performs system call 'sigreturn'. The disassembled
 * corresponding code is:
 *     mov eax, 0x77
 *     int 0x80
 */
static char x86_signal_retcode[] = "\x58\xb8\x77\x00\x00\x00\xcd\x80";


void X86ContextRunSignalHandler(X86Context *ctx, int sig)
{
	unsigned int handler;
	struct x86_sigframe sigframe;

	/* Debug */
	assert(IN_RANGE(sig, 1, 64));
	x86_sys_debug("context %d executes signal handler for signal %d\n",
		ctx->pid, sig);

	/* Signal SIGCHLD ignored if no signal handler installed */
	if (sig == SIGCHLD && !ctx->signal_handler_table->sigaction[sig - 1].handler)
		return;

	/* Save a copy of the register file */
	ctx->signal_mask_table->regs = x86_regs_create();
	x86_regs_copy(ctx->signal_mask_table->regs, ctx->regs);

	/* Create a memory page with execution permission, and copy return code on it. */
	ctx->signal_mask_table->pretcode = mem_map_space(ctx->mem, MEM_PAGE_SIZE, MEM_PAGE_SIZE);
	mem_map(ctx->mem, ctx->signal_mask_table->pretcode, MEM_PAGE_SIZE, mem_access_exec | mem_access_init);
	x86_sys_debug("  return code of signal handler allocated at 0x%x\n", ctx->signal_mask_table->pretcode);
	mem_access(ctx->mem, ctx->signal_mask_table->pretcode, sizeof(x86_signal_retcode), x86_signal_retcode, mem_access_init);

	/* Initialize stack frame */
	sigframe.pretcode = ctx->signal_mask_table->pretcode;
	sigframe.sig = sig;
	sigframe.gs = ctx->regs->gs;
	sigframe.fs = ctx->regs->fs;
	sigframe.es = ctx->regs->es;
	sigframe.ds = ctx->regs->ds;
	sigframe.edi = ctx->regs->edi;
	sigframe.esi = ctx->regs->esi;
	sigframe.ebp = ctx->regs->ebp;
	sigframe.esp = ctx->regs->esp;
	sigframe.ebx = ctx->regs->ebx;
	sigframe.edx = ctx->regs->edx;
	sigframe.ecx = ctx->regs->ecx;
	sigframe.eax = ctx->regs->eax;
	sigframe.trapno = 0;
	sigframe.err = 0;
	sigframe.eip = ctx->regs->eip;
	sigframe.cs = ctx->regs->cs;
	sigframe.eflags = ctx->regs->eflags;
	sigframe.esp_at_signal = ctx->regs->esp;
	sigframe.ss = ctx->regs->ss;
	sigframe.pfpstate = 0;
	sigframe.oldmask = 0;
	sigframe.cr2 = 0;

	/* Push signal frame */
	ctx->regs->esp -= sizeof(sigframe);
	mem_write(ctx->mem, ctx->regs->esp, sizeof(sigframe), &sigframe);
	
	/* The program will continue now executing the signal handler.
	 * In the current implementation, we do not allow other signals to
	 * interrupt the signal handler, so we notify it in the context status. */
	if (X86ContextGetState(ctx, X86ContextHandler))
		fatal("signal_handler_run: already running a handler");
	X86ContextSetState(ctx, X86ContextHandler);

	/* Set eip to run handler */
	handler = ctx->signal_handler_table->sigaction[sig - 1].handler;
	if (!handler)
		fatal("%s: invalid signal handler", __FUNCTION__);
	ctx->regs->eip = handler;
}


void X86ContextReturnFromSignalHandler(X86Context *ctx)
{
	/* Change context status */
	if (!X86ContextGetState(ctx, X86ContextHandler))
		fatal("signal_handler_return: not handling a signal");
	X86ContextClearState(ctx, X86ContextHandler);

	/* Free signal frame */
	mem_unmap(ctx->mem, ctx->signal_mask_table->pretcode, MEM_PAGE_SIZE);
	x86_sys_debug("  signal handler return code at 0x%x deallocated\n",
		ctx->signal_mask_table->pretcode);

	/* Restore saved register file and free backup */
	x86_regs_copy(ctx->regs, ctx->signal_mask_table->regs);
	x86_regs_free(ctx->signal_mask_table->regs);
}


void X86ContextCheckSignalHandlerIntr(X86Context *ctx)
{
	int sig;

	/* Context cannot be running a signal handler */
	/* A signal must be pending and unblocked */
	assert(!X86ContextGetState(ctx, X86ContextHandler));
	assert(ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked);

	/* Get signal number */
	for (sig = 1; sig <= 64; sig++)
		if (x86_sigset_member(&ctx->signal_mask_table->pending, sig) &&
			!x86_sigset_member(&ctx->signal_mask_table->blocked, sig))
			break;
	assert(sig <= 64);

	/* If signal handling uses 'SA_RESTART' flag, set return address to
	 * system call. */
	if (ctx->signal_handler_table->sigaction[sig - 1].flags & 0x10000000u)
	{
		unsigned char buf[2];

		ctx->regs->eip -= 2;
		mem_read(ctx->mem, ctx->regs->eip, 2, buf);
		assert(buf[0] == 0xcd && buf[1] == 0x80);  /* 'int 0x80' */
	}
	else
	{
		
		/* Otherwise, return -EINTR */
		ctx->regs->eax = -EINTR;
	}

	/* Run the signal handler */
	X86ContextRunSignalHandler(ctx, sig);
	x86_sigset_del(&ctx->signal_mask_table->pending, sig);

}


void X86ContextCheckSignalHandler(X86Context *ctx)
{
	int sig;

	/* If context is already running a signal handler, do nothing. */
	if (X86ContextGetState(ctx, X86ContextHandler))
		return;
	
	/* If there is no pending unblocked signal, do nothing. */
	if (!(ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked))
		return;
	
	/* There is some unblocked pending signal, prepare signal handler to
	 * be executed. */
	for (sig = 1; sig <= 64; sig++)
	{
		if (x86_sigset_member(&ctx->signal_mask_table->pending, sig) &&
			!x86_sigset_member(&ctx->signal_mask_table->blocked, sig))
		{
			X86ContextRunSignalHandler(ctx, sig);
			x86_sigset_del(&ctx->signal_mask_table->pending, sig);
			break;
		}
	}
}
