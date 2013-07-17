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

#ifndef ARCH_X86_EMU_SIGNAL_H
#define ARCH_X86_EMU_SIGNAL_H


/*
 * Class 'X86Context'
 * Additional functions
 */

/* Run a signal handler */
void X86ContextRunSignalHandler(X86Context *self, int sig);

/* Return from a signal handler */
void X86ContextReturnFromSignalHandler(X86Context *self);

void X86ContextCheckSignalHandler(X86Context *self);

/* Check any pending signal, and run the corresponding signal handler by
 * considering that the signal interrupted a system call ('syscall_intr').
 * This has the following implication on the return address from the signal
 * handler:
 *   -If flag 'SA_RESTART' is set for the handler, the return address is the
 *    system call itself, which must be repeated.
 *   -If flag 'SA_RESTART' is not set, the return address is the instruction
 *    next to the system call, and register 'eax' is set to -EINTR. */
void X86ContextCheckSignalHandlerIntr(X86Context *self);




/*
 * Object 'x86_signal_mask_table_t'
 */

/* Every context (parent and children) has its own masks */
struct x86_signal_mask_table_t
{
	unsigned long long pending;  /* Mask of pending signals */
	unsigned long long blocked;  /* Mask of blocked signals */
	unsigned long long backup;  /* Backup of blocked signals while suspended */
	struct x86_regs_t *regs;  /* Backup of regs while executing handler */
	unsigned int pretcode;  /* Base address of a memory page allocated for retcode execution */
};

struct x86_signal_mask_table_t *x86_signal_mask_table_create(void);
void x86_signal_mask_table_free(struct x86_signal_mask_table_t *table);



/*
 * Object 'x86_signal_handler_table_t'
 */

struct x86_signal_handler_table_t
{
	/* Number of extra contexts sharing the table */
	int num_links;

	/* Signal handlers */
	struct x86_sigaction_t
	{
		unsigned int handler;
		unsigned int flags;
		unsigned int restorer;
		unsigned long long mask;
	} sigaction[64];
};

struct x86_signal_handler_table_t *x86_signal_handler_table_create(void);
void x86_signal_handler_table_free(struct x86_signal_handler_table_t *table);

struct x86_signal_handler_table_t *x86_signal_handler_table_link(struct x86_signal_handler_table_t *table);
void x86_signal_handler_table_unlink(struct x86_signal_handler_table_t *table);



/*
 * Public Functions
 */

char *x86_signal_name(int signum);
void x86_sigaction_dump(struct x86_sigaction_t *sim_sigaction, FILE *f);
void x86_sigaction_flags_dump(unsigned int flags, FILE *f);
void x86_sigset_dump(unsigned long long sim_sigset, FILE *f);
void x86_sigset_add(unsigned long long *sim_sigset, int signal);
void x86_sigset_del(unsigned long long *sim_sigset, int signal);
int x86_sigset_member(unsigned long long *sim_sigset, int signal);


#endif

