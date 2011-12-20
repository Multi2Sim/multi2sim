/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpukernel.h>


/* Signals */

struct string_map_t signal_map = {
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


struct string_map_t sigaction_flags_map = {
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


char *sim_signal_name(int signum)
{
	return map_value(&signal_map, signum);
}


void sim_sigaction_dump(struct sim_sigaction *sim, FILE *f)
{
	fprintf(f, "handler=0x%x, flags=0x%x, restorer=0x%x, mask=0x%llx",
		sim->handler, sim->flags,
		sim->restorer, (long long) sim->mask);
}


void sim_sigaction_flags_dump(uint32_t flags, FILE *f)
{
	char buf[0x200];
	map_flags(&sigaction_flags_map, flags, buf, 0x200);
	fprintf(f, "%s", buf);
}


int sim_sigset_member(uint64_t *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return 0;
	return (*sim_sigset & (1ULL << (sig - 1))) > 0;
}


void sim_sigset_add(uint64_t *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return;
	*sim_sigset |= 1ULL << (sig - 1);
}


void sim_sigset_del(uint64_t *sim_sigset, int sig)
{
	if (sig < 1 || sig > 64)
		return;
	*sim_sigset &= ~(1ULL << (sig - 1));
}


void sim_sigset_dump(uint64_t sim_sigset, FILE *f)
{
	int i;
	char *comma = "", *name;
	if (sim_sigset == (uint64_t) -1) {
		fprintf(f, "{<all>}");
		return;
	}
	fprintf(f, "{");
	for (i = 1; i <= 64; i++) {
		if (sim_sigset_member(&sim_sigset, i)) {
			if (i < 32) {
				name = map_value(&signal_map, i);
				fprintf(f, "%s%s", comma, name);
			} else {
				fprintf(f, "%s%d", comma, i);
			}
			comma = ",";
		}
	}
	fprintf(f, "}");
}




/* Signal Masks */

struct signal_masks_t *signal_masks_create(void)
{
	return calloc(1, sizeof(struct signal_masks_t));
}

void signal_masks_free(struct signal_masks_t *signal_masks)
{
	free(signal_masks);
}


struct signal_handlers_t *signal_handlers_create(void)
{
	return calloc(1, sizeof(struct signal_handlers_t));
}


void signal_handlers_free(struct signal_handlers_t *signal_handlers)
{
	free(signal_handlers);
}


/* Structure representing the signal stack frame */
struct sim_sigframe {
	uint32_t pretcode;  /* Pointer to return code */
	uint32_t sig;  /* Received signal */

	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp;
	uint32_t ebx, edx, ecx, eax;
	uint32_t trapno, err, eip, cs;
	uint32_t eflags;
	uint32_t esp_at_signal;
	uint32_t ss;
	uint32_t pfpstate;  /* Pointer to floating-point state */
	uint32_t oldmask;
	uint32_t cr2;
};


/* Signal return code. The return address in the signal handler points
 * to this code, which performs system call 'sigreturn'. The disassembled
 * corresponding code is:
 *     mov eax, 0x77
 *     int 0x80
 */
static char signal_retcode[] = "\x58\xb8\x77\x00\x00\x00\xcd\x80";


/* Run a signal handler */
void signal_handler_run(struct ctx_t *ctx, int sig)
{
	uint32_t handler;
	struct sim_sigframe sigframe;

	/* Debug */
	syscall_debug("context %d executes signal handler for signal %d\n",
		ctx->pid, sig);

	/* Save a copy of the register file */
	ctx->signal_masks->regs = regs_create();
	regs_copy(ctx->signal_masks->regs, ctx->regs);

	/* Create a memory page with execution permission, and copy return code on it. */
	ctx->signal_masks->pretcode = mem_map_space(ctx->mem, MEM_PAGESIZE, MEM_PAGESIZE);
	mem_map(ctx->mem, ctx->signal_masks->pretcode, MEM_PAGESIZE, mem_access_exec | mem_access_init);
	syscall_debug("  return code of signal handler allocated at 0x%x\n", ctx->signal_masks->pretcode);
	mem_access(ctx->mem, ctx->signal_masks->pretcode, sizeof(signal_retcode), signal_retcode, mem_access_init);

	/* Initialize stack frame */
	sigframe.pretcode = ctx->signal_masks->pretcode;
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
	if (ctx_get_status(ctx, ctx_handler))
		fatal("signal_handler_run: already running a handler");
	ctx_set_status(ctx, ctx_handler);

	/* Set eip to run handler */
	handler = ctx->signal_handlers->sigaction[sig - 1].handler;
	if (!handler)
		fatal("signal_handler_run: invalid signal handler");
	ctx->regs->eip = handler;
}


/* Return from a signal handler */
void signal_handler_return(struct ctx_t *ctx)
{
	/* Change context status */
	if (!ctx_get_status(ctx, ctx_handler))
		fatal("signal_handler_return: not handling a signal");
	ctx_clear_status(ctx, ctx_handler);

	/* Free signal frame */
	mem_unmap(ctx->mem, ctx->signal_masks->pretcode, MEM_PAGESIZE);
	syscall_debug("  signal handler return code at 0x%x deallocated\n",
		ctx->signal_masks->pretcode);

	/* Restore saved register file and free backup */
	regs_copy(ctx->regs, ctx->signal_masks->regs);
	regs_free(ctx->signal_masks->regs);
}


/* Check any pending signal, and run the corresponding signal handler by
 * considering that the signal interrupted a system call ('syscall_intr').
 * This has the following implication on the return address from the signal
 * handler:
 *   -If flag 'SA_RESTART' is set for the handler, the return address is the
 *    system call itself, which must be repeated.
 *   -If flag 'SA_RESTART' is not set, the return address is the instruction
 *    next to the system call, and register 'eax' is set to -EINTR. */
void signal_handler_check_intr(struct ctx_t *ctx)
{
	int sig;

	/* Context cannot be running a signal handler */
	/* A signal must be pending and unblocked */
	assert(!ctx_get_status(ctx, ctx_handler));
	assert(ctx->signal_masks->pending & ~ctx->signal_masks->blocked);

	/* Get signal number */
	for (sig = 1; sig <= 64; sig++)
		if (sim_sigset_member(&ctx->signal_masks->pending, sig) &&
			!sim_sigset_member(&ctx->signal_masks->blocked, sig))
			break;
	assert(sig <= 64);

	/* If signal handling uses 'SA_RESTART' flag, set return address to
	 * system call. */
	if (ctx->signal_handlers->sigaction[sig - 1].flags & 0x10000000u)
	{
		unsigned char buf[2];
		ctx->regs->eip -= 2;
		mem_read(ctx->mem, ctx->regs->eip, 2, buf);
		assert(buf[0] == 0xcd && buf[1] == 0x80);  /* 'int 0x80' */
	} else {
		
		/* Otherwise, return -EINTR */
		ctx->regs->eax = -EINTR;
	}

	/* Run the signal handler */
	signal_handler_run(ctx, sig);
	sim_sigset_del(&ctx->signal_masks->pending, sig);

}


void signal_handler_check(struct ctx_t *ctx)
{
	int sig;

	/* If context is already running a signal handler, do nothing. */
	if (ctx_get_status(ctx, ctx_handler))
		return;
	
	/* If there is no pending unblocked signal, do nothing. */
	if (!(ctx->signal_masks->pending & ~ctx->signal_masks->blocked))
		return;
	
	/* There is some unblocked pending signal, prepare signal handler to
	 * be executed. */
	for (sig = 1; sig <= 64; sig++) {
		if (sim_sigset_member(&ctx->signal_masks->pending, sig) &&
			!sim_sigset_member(&ctx->signal_masks->blocked, sig))
		{
			signal_handler_run(ctx, sig);
			sim_sigset_del(&ctx->signal_masks->pending, sig);
			break;
		}
	}
}


