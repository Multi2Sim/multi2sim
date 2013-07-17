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

#ifndef ARCH_ARM_EMU_CONTEXT_H
#define ARCH_ARM_EMU_CONTEXT_H

#include <arch/arm/asm/asm.h>
#include <arch/arm/asm/asm-thumb.h>
#include <lib/util/config.h>

#include "emu.h"


#define arm_ctx_debug(...) debug(arm_ctx_debug_category, __VA_ARGS__)
extern int arm_ctx_debug_category;


enum arm_thumb_iteq_t
{
	ITEQ_DISABLED = 0,
	ITEQ_ENABLED
};


enum arm_mode_t
{
	ARM = 1,
	THUMB
};



enum arm_inst_mode_t
{
	ARM32 = 1,
	THUMB16,
	THUMB32
};

struct arm_ctx_t
{
	/* Number of extra contexts using this loader */
	int num_links;

	/* Parent context */
	struct arm_ctx_t *parent;


	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	struct arm_ctx_t *group_parent;

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
	unsigned int last_ip;  /* Address of last emulated instruction */
	unsigned int curr_ip;  /* Address of currently emulated instruction */
	unsigned int target_ip;  /* Target address for branch, even if not taken */
	unsigned int inst_type;	/* The type of the current instruction ARM/Thumb16/Thumb32 */

	/* Currently emulated instruction */
	struct arm_inst_t inst;
	struct arm_thumb16_inst_t inst_th_16;
	struct arm_thumb32_inst_t inst_th_32;

	/* Links to contexts forming a linked list. */
	struct arm_ctx_t *context_list_next, *context_list_prev;
	struct arm_ctx_t *running_list_next, *running_list_prev;
	struct arm_ctx_t *suspended_list_next, *suspended_list_prev;
	struct arm_ctx_t *finished_list_next, *finished_list_prev;
	struct arm_ctx_t *zombie_list_next, *zombie_list_prev;
	struct arm_ctx_t *alloc_list_next, *alloc_list_prev;

	/* For segmented memory access in glibc */
	unsigned int glibc_segment_base;
	unsigned int glibc_segment_limit;

	/* When debugging function calls with 'arm_isa_debug_call', function call level. */
	int function_level;

	/* For checking if the instruction is in IF-THEN Block */
	unsigned int iteq_inst_num;
	unsigned int iteq_block_flag;

	/* Variables used to wake up suspended contexts. */
	long long wakeup_time;  /* arm_emu_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	unsigned int wakeup_futex;  /* Address of futex where context is suspended */
	unsigned int wakeup_futex_bitset;  /* Bit mask for selective futex wakeup */
	long long wakeup_futex_sleep;  /* Assignment from x86_emu->futex_sleep_count */

	/* Substructures */
	struct mem_t *mem; /* Virtual Memory image */
	struct arm_regs_t *regs; /* Logical register file */
	struct arm_file_desc_table_t *file_desc_table;  /* File descriptor table */
	struct arm_signal_mask_table_t *signal_mask_table;
	struct arm_signal_handler_table_t *signal_handler_table;

	/* Fault Management */
	unsigned int fault_addr;
	int fault_value;

	/* Call Debug Stack */
	struct arm_isa_cstack_t *cstack;

	/* ARM/Thumb Symbol List */
	struct list_t *thumb_symbol_list;

	/* Statistics */

	/* Number of non-speculate instructions.
	 * Updated by the architectural simulator at the commit stage. */
	long long inst_count;
};

enum arm_ctx_status_t
{
	arm_ctx_running      = 0x00001,  /* it is able to run instructions */
	arm_ctx_spec_mode    = 0x00002,  /* executing in speculative mode */
	arm_ctx_suspended    = 0x00004,  /* suspended in a system call */
	arm_ctx_finished     = 0x00008,  /* no more inst to execute */
	arm_ctx_exclusive    = 0x00010,  /* executing in excl mode */
	arm_ctx_locked       = 0x00020,  /* another context is running in excl mode */
	arm_ctx_handler      = 0x00040,  /* executing a signal handler */
	arm_ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	arm_ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	arm_ctx_poll         = 0x00200,  /* 'poll' system call */
	arm_ctx_read         = 0x00400,  /* 'read' system call */
	arm_ctx_write        = 0x00800,  /* 'write' system call */
	arm_ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	arm_ctx_zombie       = 0x02000,  /* zombie context */
	arm_ctx_futex        = 0x04000,  /* suspended in a futex */
	arm_ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	arm_ctx_callback     = 0x10000,  /* suspended after syscall with callback */
	arm_ctx_none         = 0x00000
};

struct arm_ctx_t *arm_ctx_create();

#define arm_loader_debug(...) debug(arm_loader_debug_category, __VA_ARGS__)
extern int arm_loader_debug_category;

int arm_ctx_get_status(struct arm_ctx_t *ctx, enum arm_ctx_status_t status);
void arm_ctx_set_status(struct arm_ctx_t *ctx, enum arm_ctx_status_t status);
void arm_ctx_clear_status(struct arm_ctx_t *ctx, enum arm_ctx_status_t status);

void arm_ctx_execute(struct arm_ctx_t *ctx);
void arm_ctx_free(struct arm_ctx_t *ctx);
void arm_ctx_loader_get_full_path(struct arm_ctx_t *ctx, char *file_name, char *full_path, int size);
void arm_ctx_finish(struct arm_ctx_t *ctx, int status);
void arm_ctx_finish_group(struct arm_ctx_t *ctx, int status);
void arm_ctx_load_from_command_line(int argc, char **argv);
void arm_ctx_load_from_ctx_config(struct config_t *config, char *section);
void arm_ctx_gen_proc_self_maps(struct arm_ctx_t *ctx, char *path);
void arm_ctx_thumb_symbol_list_sort(struct list_t * thumb_symbol_list, struct elf_file_t *elf_file);
enum arm_mode_t arm_ctx_operate_mode_tag(struct list_t * thumb_symbol_list, unsigned int addr);

unsigned int arm_ctx_check_fault(struct arm_ctx_t *ctx);


#endif

