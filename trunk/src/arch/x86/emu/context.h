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

#ifndef ARCH_X86_EMU_CONTEXT_H
#define ARCH_X86_EMU_CONTEXT_H

#include <arch/x86/asm/asm.h>


/* Forward declarations */
struct bit_map_t;
struct x86_ctx_t;



#define x86_ctx_debug(...) debug(x86_ctx_debug_category, __VA_ARGS__)
extern int x86_ctx_debug_category;

typedef int (*x86_ctx_can_wakeup_callback_func_t)(struct x86_ctx_t *ctx, void *data);
typedef void (*x86_ctx_wakeup_callback_func_t)(struct x86_ctx_t *ctx, void *data);

struct x86_ctx_t
{
	/* Context properties */
	int state;
	int pid;  /* Context ID */
	int address_space_index;  /* Virtual memory address space index */

	/* Parent context */
	struct x86_ctx_t *parent;

	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	struct x86_ctx_t *group_parent;

	int exit_signal;  /* Signal to send parent when finished */
	int exit_code;  /* For zombie contexts */

	unsigned int clear_child_tid;
	unsigned int robust_list_head;  /* robust futex list */

	/* Instruction pointers */
	unsigned int last_eip;  /* Address of last emulated instruction */
	unsigned int curr_eip;  /* Address of currently emulated instruction */
	unsigned int target_eip;  /* Target address for branch, even if not taken */

	/* Currently emulated instruction */
	struct x86_inst_t inst;

	/* Recorded virtual memory address for last emulated instruction */
	unsigned int effective_address;

	/* For emulation of string operations */
	unsigned int str_op_esi;  /* Initial value for register 'esi' in string operation */
	unsigned int str_op_edi;  /* Initial value for register 'edi' in string operation */
	int str_op_dir;  /* Direction: 1 = forward, -1 = backward */
	int str_op_count;  /* Number of iterations in string operation */



	/*
	 * Context scheduling (timing simulation)
	 */

	/* Cycle when the context was allocated and evicted to a node (core/thread),
	 * respectively. */
	long long alloc_cycle;
	long long evict_cycle;

	/* The context is mapped and allocated, but its eviction is in progress.
	 * It will be effectively evicted once the last instruction reaches the
	 * commit stage. This value is set by 'x86_cpu_context_evict_signal'. */
	int evict_signal;

	/* If context is in state 'mapped', these two variables represent the
	 * node (core/thread) associated with the context. */
	int core;
	int thread;



	/* For segmented memory access in glibc */
	unsigned int glibc_segment_base;
	unsigned int glibc_segment_limit;

	/* When debugging function calls with 'x86_isa_debug_call', function call level. */
	int function_level;

	/* Host thread that suspends and then schedules call to 'x86_emu_process_events'. */
	/* The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	 * is launched for this context (by caller).
	 * It is clear when the context finished (by the host thread).
	 * It should be accessed safely by locking global mutex 'x86_emu->process_events_mutex'. */
	pthread_t host_thread_suspend;  /* Thread */
	int host_thread_suspend_active;  /* Thread-spawned flag */

	/* Host thread that lets time elapse and schedules call to 'x86_emu_process_events'. */
	pthread_t host_thread_timer;  /* Thread */
	int host_thread_timer_active;  /* Thread-spawned flag */
	long long host_thread_timer_wakeup;  /* Time when the thread will wake up */

	/* Three timers used by 'setitimer' system call - real, virtual, and prof. */
	long long itimer_value[3];  /* Time when current occurrence of timer expires (0=inactive) */
	long long itimer_interval[3];  /* Interval (in usec) of repetition (0=inactive) */

	/* Variables used to wake up suspended contexts. */
	long long wakeup_time;  /* x86_emu_timer time to wake up (poll/nanosleep) */
	int wakeup_fd;  /* File descriptor (read/write/poll) */
	int wakeup_events;  /* Events for wake up (poll) */
	int wakeup_pid;  /* Pid waiting for (waitpid) */
	unsigned int wakeup_futex;  /* Address of futex where context is suspended */
	unsigned int wakeup_futex_bitset;  /* Bit mask for selective futex wakeup */
	long long wakeup_futex_sleep;  /* Assignment from x86_emu->futex_sleep_count */

	/* Generic callback function (and data to pass to it) to call when a
	 * context gets suspended in a system call to check whether it should be
	 * waken up, and once it is waken up, respectively */
	x86_ctx_can_wakeup_callback_func_t can_wakeup_callback_func;
	x86_ctx_wakeup_callback_func_t wakeup_callback_func;
	void *can_wakeup_callback_data;
	void *wakeup_callback_data;

	/* Links to contexts forming a linked list. */
	struct x86_ctx_t *context_list_next, *context_list_prev;
	struct x86_ctx_t *running_list_next, *running_list_prev;
	struct x86_ctx_t *suspended_list_next, *suspended_list_prev;
	struct x86_ctx_t *finished_list_next, *finished_list_prev;
	struct x86_ctx_t *zombie_list_next, *zombie_list_prev;

	/* List of contexts mapped to a hardware core/thread. This list is
	 * managed by the timing simulator for scheduling purposes. */
	struct x86_ctx_t *mapped_list_next, *mapped_list_prev;

	/* Substructures */
	struct x86_loader_t *loader;
	struct mem_t *mem;  /* Virtual memory image */
	struct spec_mem_t *spec_mem;  /* Speculative memory */
	struct x86_regs_t *regs;  /* Logical register file */
	struct x86_regs_t *backup_regs;  /* Backup when entering in speculative mode */
	struct x86_file_desc_table_t *file_desc_table;  /* File descriptor table */
	struct x86_signal_mask_table_t *signal_mask_table;
	struct x86_signal_handler_table_t *signal_handler_table;

	/* Thread affinity mask */
	struct bit_map_t *affinity;


	/* Statistics */

	/* Number of non-speculate micro-instructions.
	 * Updated by the architectural simulator at the commit stage. */
	long long inst_count;
};

enum x86_ctx_state_t
{
	x86_ctx_running      = 0x00001,  /* it is able to run instructions */
	x86_ctx_spec_mode    = 0x00002,  /* executing in speculative mode */
	x86_ctx_suspended    = 0x00004,  /* suspended in a system call */
	x86_ctx_finished     = 0x00008,  /* no more inst to execute */
	x86_ctx_exclusive    = 0x00010,  /* executing in excl mode */
	x86_ctx_locked       = 0x00020,  /* another context is running in excl mode */
	x86_ctx_handler      = 0x00040,  /* executing a signal handler */
	x86_ctx_sigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	x86_ctx_nanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	x86_ctx_poll         = 0x00200,  /* 'poll' system call */
	x86_ctx_read         = 0x00400,  /* 'read' system call */
	x86_ctx_write        = 0x00800,  /* 'write' system call */
	x86_ctx_waitpid      = 0x01000,  /* 'waitpid' system call */
	x86_ctx_zombie       = 0x02000,  /* zombie context */
	x86_ctx_futex        = 0x04000,  /* suspended in a futex */
	x86_ctx_alloc        = 0x08000,  /* allocated to a core/thread */
	x86_ctx_callback     = 0x10000,  /* suspended after syscall with callback */
	x86_ctx_mapped       = 0x20000,  /* mapped to a core/thread */
	x86_ctx_none         = 0x00000
};

struct x86_ctx_t *x86_ctx_create(void);
void x86_ctx_free(struct x86_ctx_t *ctx);

void x86_ctx_dump(struct x86_ctx_t *ctx, FILE *f);

struct x86_ctx_t *x86_ctx_clone(struct x86_ctx_t *ctx);
struct x86_ctx_t *x86_ctx_fork(struct x86_ctx_t *ctx);

/* Thread safe/unsafe versions */
void __x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx);
void x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx);
void __x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx);
void x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx);

void x86_ctx_suspend(struct x86_ctx_t *ctx,
	x86_ctx_can_wakeup_callback_func_t can_wakeup_callback_func,
	void *can_wakeup_callback_data, x86_ctx_wakeup_callback_func_t wakeup_callback_func,
	void *wakeup_callback_data);

void x86_ctx_finish(struct x86_ctx_t *ctx, int state);
void x86_ctx_finish_group(struct x86_ctx_t *ctx, int state);
void x86_ctx_execute(struct x86_ctx_t *ctx);

void x86_ctx_set_eip(struct x86_ctx_t *ctx, unsigned int eip);
void x86_ctx_recover(struct x86_ctx_t *ctx);

struct x86_ctx_t *x86_ctx_get(int pid);
struct x86_ctx_t *x86_ctx_get_zombie(struct x86_ctx_t *parent, int pid);

int x86_ctx_get_state(struct x86_ctx_t *ctx, enum x86_ctx_state_t state);
void x86_ctx_set_state(struct x86_ctx_t *ctx, enum x86_ctx_state_t state);
void x86_ctx_clear_state(struct x86_ctx_t *ctx, enum x86_ctx_state_t state);

int x86_ctx_futex_wake(struct x86_ctx_t *ctx, unsigned int futex,
	unsigned int count, unsigned int bitset);
void x86_ctx_exit_robust_list(struct x86_ctx_t *ctx);

void x86_ctx_gen_proc_self_maps(struct x86_ctx_t *ctx, char *path, int size);
void x86_ctx_gen_proc_cpuinfo(struct x86_ctx_t *ctx, char *path, int size);

#endif

