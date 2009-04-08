/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "m2skernel.h"


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


/* Process pending signals and execute signal handlers
 * accordingly. */
void signal_process(struct ctx_t *ctx)
{
	int sig;

	/* If we are executing a signal handler, we cannot be
	 * interrupted. After the signal handler finishes, pending
	 * signals will be processed again. */
	if (ctx_get_status(ctx, ctx_handler))
		return;

	/* If there is no unblocked pending signal, we
	 * can exit the function immediately. */
	if (!(ctx->signal_masks->pending & ~ctx->signal_masks->blocked))
		return;
	
	/* There is some unblocked pending signal, prepare signal handler to
	 * be executed. If context is suspended, wake it up. */
	for (sig = 1; sig <= 64; sig++) {
		if (sim_sigset_member(&ctx->signal_masks->pending, sig) &&
			!sim_sigset_member(&ctx->signal_masks->blocked, sig))
		{
			ke_event_signal();
			signal_handler_run(ctx, sig);
			sim_sigset_del(&ctx->signal_masks->pending, sig);
			break;
		}
	}
}


/* Run a signal handler */
void signal_handler_run(struct ctx_t *ctx, int sig)
{
	unsigned char code[] = "\x58\xb8\x77\x00\x00\x00\xcd\x80";
	uint32_t retaddr, handler;

	/* Debug */
	syscall_debug("context %d executes signal handler for signal %d\n",
		ctx->pid, sig);

	/* Save a copy of the register file */
	ctx->signal_masks->regs = regs_create();
	regs_copy(ctx->signal_masks->regs, ctx->regs);

	/* Push this piece of code into the stack:
	 *   mov eax, 0x77 (syscall_code_sigreturn)
	 *   int 0x80 (system call)
	 * Then save the base address for this code, since it will be the
	 * return address for the signal handler to execute. */
	ctx->regs->esp -= 8;
	mem_write(ctx->mem, ctx->regs->esp, 8, &code);
	retaddr = ctx->regs->esp;
	
	/* Push argument (signal number) and return address */
	ctx->regs->esp -= 8;
	mem_write(ctx->mem, ctx->regs->esp + 4, 4, &sig);
	mem_write(ctx->mem, ctx->regs->esp, 4, &retaddr);

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

	/* Restore saved register file and free backup */
	regs_copy(ctx->regs, ctx->signal_masks->regs);
	regs_free(ctx->signal_masks->regs);
}

