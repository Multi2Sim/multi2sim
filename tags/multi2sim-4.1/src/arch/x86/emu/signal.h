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

void x86_signal_handler_run(struct x86_ctx_t *ctx, int sig);
void x86_signal_handler_return(struct x86_ctx_t *ctx);
void x86_signal_handler_check(struct x86_ctx_t *ctx);
void x86_signal_handler_check_intr(struct x86_ctx_t *ctx);

char *x86_signal_name(int signum);
void x86_sigaction_dump(struct x86_sigaction_t *sim_sigaction, FILE *f);
void x86_sigaction_flags_dump(unsigned int flags, FILE *f);
void x86_sigset_dump(unsigned long long sim_sigset, FILE *f);
void x86_sigset_add(unsigned long long *sim_sigset, int signal);
void x86_sigset_del(unsigned long long *sim_sigset, int signal);
int x86_sigset_member(unsigned long long *sim_sigset, int signal);


#endif

