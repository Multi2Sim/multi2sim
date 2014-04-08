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

#ifndef ARCH_MIPS_EMU_CONTEXT_H
#define ARCH_MIPS_EMU_CONTEXT_H

#include <arch/mips/asm/asm.h>
#include <lib/util/config.h>

#include "emu.h"


#define mips_ctx_debug(...) debug(mips_ctx_debug_category, __VA_ARGS__)
extern int mips_ctx_debug_category;

struct mips_ctx_t
{
	/* Number of extra contexts using this loader */
	int num_links; //???

	/* Parent context */
	struct mips_ctx_t *parent;


	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	struct mips_ctx_t *group_parent;

	/* Context properties */
	int status;
	int pid;  /* Context ID */
	int address_space_index;  /* Virtual memory address space index */

	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie contexts */

	/* Program data */
	struct elf_file_t *elf_file;
	struct linked_list_t *args;
	struct linked_list_t *env;
	char *exe;  /* Executable file name */
	char *cwd;  /* Current working directory */
	char *stdin_file;  /* File name for stdin */
	char *stdout_file;  /* File name for stdout */

	/* Stack */
	unsigned int stack_base;
	unsigned int stack_top;
	unsigned int stack_size;
	unsigned int environ_base;

	/* Lowest address initialized */
	unsigned int bottom;

	/* Program entries */
	unsigned int prog_entry;
	unsigned int interp_prog_entry;

	/* Program headers */
	unsigned int phdt_base;
	unsigned int phdr_count;

	/* Random bytes */
	unsigned int at_random_addr;
	unsigned int at_random_addr_holder;

	/* Instruction pointers */
	unsigned int next_ip;  /* Address of next emulated instruction */
	unsigned int curr_ip;  /* Address of currently emulated instruction */
	unsigned int n_next_ip;  /* Address for secon-next instruction */

	/* Currently emulated instruction */
	struct mips_inst_t inst;

	/* Links to contexts forming a linked list. */
	struct mips_ctx_t *context_list_next, *context_list_prev;
	struct mips_ctx_t *running_list_next, *running_list_prev;
	struct mips_ctx_t *suspended_list_next, *suspended_list_prev;
	struct mips_ctx_t *finished_list_next, *finished_list_prev;
	struct mips_ctx_t *zombie_list_next, *zombie_list_prev;
	struct mips_ctx_t *alloc_list_next, *alloc_list_prev;

	/* For segmented memory access in glibc */
	unsigned int glibc_segment_base;
	unsigned int glibc_segment_limit;

	/* When debugging function calls with 'mips_isa_debug_call', function call level. */
	int function_level;


	/* Variables used to wake up suspended contexts. */
	long long wakeup_time;  /* mips_emu_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	unsigned int wakeup_futex;  /* Address of futex where context is suspended */
	unsigned int wakeup_futex_bitset;  /* Bit mask for selective futex wakeup */
	long long wakeup_futex_sleep;  /* Assignment from x86_emu->futex_sleep_count */

	/* Substructures */
	struct mem_t *mem; /* Virtual Memory image */
	struct mips_regs_t *regs; /* Logical register file */
	struct mips_file_desc_table_t *file_desc_table;  /* File descriptor table */
	struct mips_signal_mask_table_t *signal_mask_table;
	struct mips_signal_handler_table_t *signal_handler_table;

	/* Fault Management */
	unsigned int fault_addr;
	int fault_value;

	/* Call Debug Stack */
	struct mips_isa_cstack_t *cstack;

	/* Statistics */

	/* Number of non-speculate instructions.
	 * Updated by the architectural simulator at the commit stage. */
	long long inst_count;
};

enum mips_ctx_status_t
{
	mips_ctx_running      = 0x00001,  /* it is able to run instructions */
	mips_ctx_spec_mode    = 0x00002,  /* executing in speculative mode */
	mips_ctx_suspended    = 0x00004,  /* suspended in a system call */
	mips_ctx_finished     = 0x00008,  /* no more inst to execute */
	mips_ctx_exclusive    = 0x00010,  /* executing in excl mode */
	mips_ctx_locked       = 0x00020,  /* another context is running in excl mode */
	mips_ctx_handler      = 0x00040,  /* executing a signal handler */
	mips_ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	mips_ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	mips_ctx_poll         = 0x00200,  /* 'poll' system call */
	mips_ctx_read         = 0x00400,  /* 'read' system call */
	mips_ctx_write        = 0x00800,  /* 'write' system call */
	mips_ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	mips_ctx_zombie       = 0x02000,  /* zombie context */
	mips_ctx_futex        = 0x04000,  /* suspended in a futex */
	mips_ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	mips_ctx_callback     = 0x10000,  /* suspended after syscall with callback */
	mips_ctx_none         = 0x00000
};

struct mips_ctx_t *mips_ctx_create();

#define mips_loader_debug(...) debug(mips_loader_debug_category, __VA_ARGS__)
extern int mips_loader_debug_category;

int mips_ctx_get_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status);
void mips_ctx_set_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status);
void mips_ctx_clear_status(struct mips_ctx_t *ctx, enum mips_ctx_status_t status);

void mips_ctx_execute(struct mips_ctx_t *ctx);
void mips_ctx_free(struct mips_ctx_t *ctx);
void mips_ctx_loader_get_full_path(struct mips_ctx_t *ctx, char *file_name, char *full_path, int size);
void mips_ctx_finish(struct mips_ctx_t *ctx, int status);
void mips_ctx_finish_group(struct mips_ctx_t *ctx, int status);
void mips_ctx_load_from_command_line(int argc, char **argv);
void mips_ctx_load_from_ctx_config(struct config_t *config, char *section);
void mips_ctx_gen_proc_self_maps(struct mips_ctx_t *ctx, char *path);

unsigned int mips_ctx_check_fault(struct mips_ctx_t *ctx);


#endif

