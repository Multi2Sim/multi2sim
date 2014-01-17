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

#include <lib/util/config.h>

#include "emu.h"


/*
 * Class 'MIPSContext'
 */

typedef enum
{
	MIPSContextRunning      = 0x00001,  /* it is able to run instructions */
	MIPSContextSpecMode     = 0x00002,  /* executing in speculative mode */
	MIPSContextSuspended    = 0x00004,  /* suspended in a system call */
	MIPSContextFinished     = 0x00008,  /* no more inst to execute */
	MIPSContextExclusive    = 0x00010,  /* executing in excl mode */
	MIPSContextLocked       = 0x00020,  /* another context is running in excl mode */
	MIPSContextHandler      = 0x00040,  /* executing a signal handler */
	MIPSContextSigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	MIPSContextNanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	MIPSContextPoll         = 0x00200,  /* 'poll' system call */
	MIPSContextRead         = 0x00400,  /* 'read' system call */
	MIPSContextWrite        = 0x00800,  /* 'write' system call */
	MIPSContextWaitpid      = 0x01000,  /* 'waitpid' system call */
	MIPSContextZombie       = 0x02000,  /* zombie context */
	MIPSContextFutex        = 0x04000,  /* suspended in a futex */
	MIPSContextAlloc        = 0x08000,  /* allocated to a core/thread */
	MIPSContextCallback     = 0x10000,  /* suspended after syscall with callback */
	MIPSContextInvalid      = 0x00000
} MIPSContextState;


CLASS_BEGIN(MIPSContext, Object)

	/* Emulator */
	MIPSEmu *emu;

	/* Parent context */
	MIPSContext *parent;

	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	MIPSContext *group_parent;

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
	struct MIPSInstWrap *inst;

	/* Links to contexts forming a linked list. */
	MIPSContext *context_list_next, *context_list_prev;
	MIPSContext *running_list_next, *running_list_prev;
	MIPSContext *suspended_list_next, *suspended_list_prev;
	MIPSContext *finished_list_next, *finished_list_prev;
	MIPSContext *zombie_list_next, *zombie_list_prev;
	MIPSContext *alloc_list_next, *alloc_list_prev;

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

CLASS_END(MIPSContext)


void MIPSContextCreate(MIPSContext *self, MIPSEmu *emu);
void MIPSContextDestroy(MIPSContext *self);

int MIPSContextGetState(MIPSContext *ctx, MIPSContextState status);
void MIPSContextSetState(MIPSContext *ctx, MIPSContextState status);
void MIPSContextClearState(MIPSContext *ctx, MIPSContextState status);

void MIPSContextExecute(MIPSContext *ctx);
void MIPSContextGetFullPath(MIPSContext *ctx, char *file_name, char *full_path, int size);
void MIPSContextFinish(MIPSContext *ctx, int status);
void MIPSContextFinishGroup(MIPSContext *ctx, int status);
void MIPSContextGenerateProcSelfMaps(MIPSContext *ctx, char *path);

void MIPSContextAddArgsVector(MIPSContext *self, int argc, char **argv);

/* Add environment variables from the actual environment plus the list attached
 * in the argument 'env'. */
void MIPSContextAddEnviron(MIPSContext *self, char *env);

void MIPSContextLoadExecutable(MIPSContext *self, char *path);


/*
 * Public
 */

#define MIPSContextDebug(...) debug(mips_context_debug_category, __VA_ARGS__)
extern int mips_context_debug_category;

#define mips_loader_debug(...) debug(mips_loader_debug_category, __VA_ARGS__)
extern int mips_loader_debug_category;


#endif

