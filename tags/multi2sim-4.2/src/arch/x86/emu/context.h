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

#include <pthread.h>

#include <arch/x86/asm/asm.h>
#include <arch/x86/asm/inst.h>
#include <lib/util/class.h>


/* Forward declarations */
struct bit_map_t;



/*
 * Class 'X86Context'
 */

typedef int (*X86ContextCanWakeupFunc)(X86Context *self, void *data);
typedef void (*X86ContextWakeupFunc)(X86Context *self, void *data);

typedef enum
{
	X86ContextRunning      = 0x00001,  /* it is able to run instructions */
	X86ContextSpecMode     = 0x00002,  /* executing in speculative mode */
	X86ContextSuspended    = 0x00004,  /* suspended in a system call */
	X86ContextFinished     = 0x00008,  /* no more inst to execute */
	X86ContextExclusive    = 0x00010,  /* executing in excl mode */
	X86ContextLocked       = 0x00020,  /* another context is running in excl mode */
	X86ContextHandler      = 0x00040,  /* executing a signal handler */
	X86ContextSigsuspend   = 0x00080,  /* suspended after syscall 'sigsuspend' */
	X86ContextNanosleep    = 0x00100,  /* suspended after syscall 'nanosleep' */
	X86ContextPoll         = 0x00200,  /* 'poll' system call */
	X86ContextRead         = 0x00400,  /* 'read' system call */
	X86ContextWrite        = 0x00800,  /* 'write' system call */
	X86ContextWaitpid      = 0x01000,  /* 'waitpid' system call */
	X86ContextZombie       = 0x02000,  /* zombie context */
	X86ContextFutex        = 0x04000,  /* suspended in a futex */
	X86ContextAlloc        = 0x08000,  /* allocated to a core/thread */
	X86ContextCallback     = 0x10000,  /* suspended after syscall with callback */
	X86ContextMapped       = 0x20000,  /* mapped to a core/thread */
	X86ContextNone         = 0x00000
} X86ContextState;


CLASS_BEGIN(X86Context, Object)
	
	/* Emulator it belongs to */
	X86Emu *emu;

	/* Context properties */
	int state;
	int pid;  /* Context ID */
	int address_space_index;  /* Virtual memory address space index */

	/* Parent context */
	X86Context *parent;

	/* Context group initiator. There is only one group parent (if not NULL)
	 * with many group children, no tree organization. */
	X86Context *group_parent;

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
	int core_index;
	int thread_index;



	/* For segmented memory access in glibc */
	unsigned int glibc_segment_base;
	unsigned int glibc_segment_limit;

	/* When debugging function calls with 'x86_isa_debug_call', function call level. */
	int function_level;

	/* Host thread that suspends and then schedules call to 'x86_emu_process_events'. */
	/* The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	 * is launched for this context (by caller).
	 * It is clear when the context finished (by the host thread).
	 * It should be accessed safely by locking global mutex 'process_events_mutex'. */
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
	long long wakeup_futex_sleep;  /* Assignment from futex_sleep_count */

	/* Generic callback function (and data to pass to it) to call when a
	 * context gets suspended in a system call to check whether it should be
	 * waken up, and once it is waken up, respectively */
	X86ContextCanWakeupFunc can_wakeup_callback_func;
	X86ContextWakeupFunc wakeup_callback_func;
	void *can_wakeup_callback_data;
	void *wakeup_callback_data;

	/* Links to contexts forming a linked list. */
	X86Context *context_list_next, *context_list_prev;
	X86Context *running_list_next, *running_list_prev;
	X86Context *suspended_list_next, *suspended_list_prev;
	X86Context *finished_list_next, *finished_list_prev;
	X86Context *zombie_list_next, *zombie_list_prev;

	/* List of contexts mapped to a hardware core/thread. This list is
	 * managed by the timing simulator for scheduling purposes. */
	X86Context *mapped_list_next, *mapped_list_prev;

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

CLASS_END(X86Context)


void X86ContextCreate(X86Context *self, X86Emu *emu);
void X86ContextCreateAndClone(X86Context *self, X86Context *cloned);
void X86ContextCreateAndFork(X86Context *self, X86Context *forked);

void X86ContextDestroy(X86Context *self);

void X86ContextDump(Object *self, FILE *f);

/* Thread safe/unsafe versions */
void X86ContextHostThreadSuspendCancelUnsafe(X86Context *self);
void X86ContextHostThreadSuspendCancel(X86Context *self);
void X86ContextHostThreadTimerCancelUnsafe(X86Context *self);
void X86ContextHostThreadTimerCancel(X86Context *self);

void X86ContextSuspend(X86Context *self,
	X86ContextCanWakeupFunc can_wakeup_callback_func,
	void *can_wakeup_callback_data, X86ContextWakeupFunc wakeup_callback_func,
	void *wakeup_callback_data);

void X86ContextFinish(X86Context *self, int state);
void X86ContextFinishGroup(X86Context *self, int state);
void X86ContextExecute(X86Context *self);

void X86ContextSetEip(X86Context *self, unsigned int eip);
void X86ContextRecover(X86Context *self);

X86Context *X86ContextGetZombie(X86Context *parent, int pid);

int X86ContextGetState(X86Context *self, X86ContextState state);
void X86ContextSetState(X86Context *self, X86ContextState state);
void X86ContextClearState(X86Context *self, X86ContextState state);

int X86ContextFutexWake(X86Context *self, unsigned int futex,
	unsigned int count, unsigned int bitset);
void X86ContextExitRobustList(X86Context *self);

void X86ContextProcSelfMaps(X86Context *self, char *path, int size);
void X86ContextProcCPUInfo(X86Context *self, char *path, int size);

/* Function that suspends the host thread waiting for an event to occur.
 * When the event finally occurs (i.e., before the function finishes, a
 * call to 'X86EmuProcessEvents' is scheduled.
 * The argument 'arg' is the associated guest context. */
void *X86EmuHostThreadSuspend(void *self);

/* Function that suspends the host thread waiting for a timer to expire,
 * and then schedules a call to 'X86EmuProcessEvents'. */
void *X86ContextHostThreadTimer(void *self);




/*
 * Non-Class
 */

#define X86ContextDebug(...) debug(x86_context_debug_category, __VA_ARGS__)
extern int x86_context_debug_category;

extern struct str_map_t x86_context_state_map;

#endif

