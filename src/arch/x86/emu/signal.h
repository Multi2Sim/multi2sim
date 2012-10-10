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
struct signal_mask_table_t
{
	unsigned long long pending;  /* Mask of pending signals */
	unsigned long long blocked;  /* Mask of blocked signals */
	unsigned long long backup;  /* Backup of blocked signals while suspended */
	struct x86_regs_t *regs;  /* Backup of regs while executing handler */
	unsigned int pretcode;  /* Base address of a memory page allocated for retcode execution */
};

struct signal_mask_table_t *signal_mask_table_create(void);
void signal_mask_table_free(struct signal_mask_table_t *table);


struct signal_handler_table_t
{
	/* Number of extra contexts sharing the table */
	int num_links;

	/* Signal handlers */
	struct sim_sigaction
	{
		unsigned int handler;
		unsigned int flags;
		unsigned int restorer;
		unsigned long long mask;
	} sigaction[64];
};

struct signal_handler_table_t *signal_handler_table_create(void);
void signal_handler_table_free(struct signal_handler_table_t *table);

struct signal_handler_table_t *signal_handler_table_link(struct signal_handler_table_t *table);
void signal_handler_table_unlink(struct signal_handler_table_t *table);

void signal_handler_run(struct x86_ctx_t *ctx, int sig);
void signal_handler_return(struct x86_ctx_t *ctx);
void signal_handler_check(struct x86_ctx_t *ctx);
void signal_handler_check_intr(struct x86_ctx_t *ctx);

char *sim_signal_name(int signum);
void sim_sigaction_dump(struct sim_sigaction *sim_sigaction, FILE *f);
void sim_sigaction_flags_dump(unsigned int flags, FILE *f);
void sim_sigset_dump(unsigned long long sim_sigset, FILE *f);
void sim_sigset_add(unsigned long long *sim_sigset, int signal);
void sim_sigset_del(unsigned long long *sim_sigset, int signal);
int sim_sigset_member(unsigned long long *sim_sigset, int signal);


#endif

