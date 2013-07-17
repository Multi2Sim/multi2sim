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

#include <poll.h>
#include <unistd.h>

#include <arch/x86/timing/cpu.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>
#include <mem-system/mmu.h>
#include <mem-system/spec-mem.h>

#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "isa.h"
#include "loader.h"
#include "regs.h"
#include "signal.h"
#include "syscall.h"


/*
 * Class 'X86Context'
 */

CLASS_IMPLEMENTATION(X86Context);

static void X86ContextDoCreate(X86Context *self, X86Emu *emu)
{
	int num_nodes;
	int i;
	
	/* Initialize */
	self->emu = emu;
	self->pid = emu->current_pid++;

	/* Update state so that the context is inserted in the
	 * corresponding lists. The x86_ctx_running parameter has no
	 * effect, since it will be updated later. */
	X86ContextSetState(self, X86ContextRunning);
	DOUBLE_LINKED_LIST_INSERT_HEAD(emu, context, self);

	/* Structures */
	self->regs = x86_regs_create();
	self->backup_regs = x86_regs_create();
	self->signal_mask_table = x86_signal_mask_table_create();

	/* Thread affinity mask, used only for timing simulation. It is
	 * initialized to all 1's. */
	num_nodes = x86_cpu_num_cores * x86_cpu_num_threads;
	self->affinity = bit_map_create(num_nodes);
	for (i = 0; i < num_nodes; i++)
		bit_map_set(self->affinity, i, 1, 1);

	/* Virtual functions */
	asObject(self)->Dump = X86ContextDump;
}


void X86ContextCreate(X86Context *self, X86Emu *emu)
{
	/* Baseline initialization */
	X86ContextDoCreate(self, emu);

	/* Loader */
	self->loader = x86_loader_create();

	/* Memory */
	self->address_space_index = mmu_address_space_new();
	self->mem = mem_create();
	self->spec_mem = spec_mem_create(self->mem);

	/* Signal handlers and file descriptor table */
	self->signal_handler_table = x86_signal_handler_table_create();
	self->file_desc_table = x86_file_desc_table_create();
}


void X86ContextCreateAndClone(X86Context *self, X86Context *cloned)
{
	/* Baseline initialization */
	X86ContextDoCreate(self, cloned->emu);

	/* Register file contexts are copied from parent. */
	x86_regs_copy(self->regs, cloned->regs);

	/* The memory image of the cloned context if the same.
	 * The memory structure must be only freed by the parent
	 * when all its children have been killed.
	 * The set of signal handlers is the same, too. */
	self->address_space_index = cloned->address_space_index;
	self->mem = mem_link(cloned->mem);
	self->spec_mem = spec_mem_create(self->mem);

	/* Loader */
	self->loader = x86_loader_link(cloned->loader);

	/* Signal handlers and file descriptor table */
	self->signal_handler_table = x86_signal_handler_table_link(cloned->signal_handler_table);
	self->file_desc_table = x86_file_desc_table_link(cloned->file_desc_table);

	/* Libc segment */
	self->glibc_segment_base = cloned->glibc_segment_base;
	self->glibc_segment_limit = cloned->glibc_segment_limit;

	/* Update other fields. */
	self->parent = cloned;
}


void X86ContextCreateAndFork(X86Context *self, X86Context *forked)
{
	/* Initialize baseline contect */
	X86ContextDoCreate(self, forked->emu);

	/* Copy registers */
	x86_regs_copy(self->regs, forked->regs);

	/* Memory */
	self->address_space_index = mmu_address_space_new();
	self->mem = mem_create();
	self->spec_mem = spec_mem_create(self->mem);
	mem_clone(self->mem, forked->mem);

	/* Loader */
	self->loader = x86_loader_link(forked->loader);

	/* Signal handlers and file descriptor table */
	self->signal_handler_table = x86_signal_handler_table_create();
	self->file_desc_table = x86_file_desc_table_create();

	/* Libc segment */
	self->glibc_segment_base = forked->glibc_segment_base;
	self->glibc_segment_limit = forked->glibc_segment_limit;

	/* Set parent */
	self->parent = forked;
}


void X86ContextDestroy(X86Context *self)
{
	X86Emu *emu = self->emu;

	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!X86ContextGetState(self, X86ContextFinished | X86ContextZombie))
		X86ContextFinish(self, 0);
	
	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!DOUBLE_LINKED_LIST_MEMBER(emu, running, self));
	assert(!DOUBLE_LINKED_LIST_MEMBER(emu, suspended, self));
	assert(!DOUBLE_LINKED_LIST_MEMBER(emu, zombie, self));
	assert(DOUBLE_LINKED_LIST_MEMBER(emu, finished, self));
	DOUBLE_LINKED_LIST_REMOVE(emu, finished, self);
		
	/* Free private structures */
	x86_regs_free(self->regs);
	x86_regs_free(self->backup_regs);
	x86_signal_mask_table_free(self->signal_mask_table);
	spec_mem_free(self->spec_mem);
	bit_map_free(self->affinity);

	/* Unlink shared structures */
	x86_loader_unlink(self->loader);
	x86_signal_handler_table_unlink(self->signal_handler_table);
	x86_file_desc_table_unlink(self->file_desc_table);
	mem_unlink(self->mem);

	/* Remove context from contexts list and free */
	DOUBLE_LINKED_LIST_REMOVE(emu, context, self);
	X86ContextDebug("inst %lld: context %d freed\n",
			asEmu(emu)->instructions, self->pid);
}


void X86ContextDump(Object *self, FILE *f)
{
	X86Context *context = asX86Context(self);
	char state_str[MAX_STRING_SIZE];

	/* Title */
	fprintf(f, "------------\n");
	fprintf(f, "Context %d\n", context->pid);
	fprintf(f, "------------\n\n");

	str_map_flags(&x86_context_state_map, context->state, state_str, sizeof state_str);
	fprintf(f, "State = %s\n", state_str);
	if (!context->parent)
		fprintf(f, "Parent = None\n");
	else
		fprintf(f, "Parent = %d\n", context->parent->pid);
	fprintf(f, "Heap break: 0x%x\n", context->mem->heap_break);

	/* Bit masks */
	fprintf(f, "BlockedSignalMask = 0x%llx ", context->signal_mask_table->blocked);
	x86_sigset_dump(context->signal_mask_table->blocked, f);
	fprintf(f, "\nPendingSignalMask = 0x%llx ", context->signal_mask_table->pending);
	x86_sigset_dump(context->signal_mask_table->pending, f);
	fprintf(f, "\nAffinity = ");
	bit_map_dump(context->affinity, 0, x86_cpu_num_cores * x86_cpu_num_threads, f);
	fprintf(f, "\n");

	/* End */
	fprintf(f, "\n\n");
}


void X86ContextExecute(X86Context *self)
{
	X86Emu *emu = self->emu;

	struct x86_regs_t *regs = self->regs;
	struct mem_t *mem = self->mem;

	unsigned char buffer[20];
	unsigned char *buffer_ptr;

	int spec_mode;

	/* Memory permissions should not be checked if the context is executing in
	 * speculative mode. This will prevent guest segmentation faults to occur. */
	spec_mode = X86ContextGetState(self, X86ContextSpecMode);
	mem->safe = spec_mode ? 0 : mem_safe_mode;

	/* Read instruction from memory. Memory should be accessed here in unsafe mode
	 * (i.e., allowing segmentation faults) if executing speculatively. */
	buffer_ptr = mem_get_buffer(mem, regs->eip, 20, mem_access_exec);
	if (!buffer_ptr)
	{
		/* Disable safe mode. If a part of the 20 read bytes does not belong to the
		 * actual instruction, and they lie on a page with no permissions, this would
		 * generate an undesired protection fault. */
		mem->safe = 0;
		buffer_ptr = buffer;
		mem_access(mem, regs->eip, 20, buffer_ptr, mem_access_exec);
	}
	mem->safe = mem_safe_mode;

	/* Disassemble */
	x86_inst_decode(&self->inst, regs->eip, buffer_ptr);
	if (self->inst.opcode == x86_inst_opcode_invalid && !spec_mode)
		fatal("0x%x: not supported x86 instruction (%02x %02x %02x %02x...)",
			regs->eip, buffer_ptr[0], buffer_ptr[1], buffer_ptr[2], buffer_ptr[3]);


	/* Stop if instruction matches last instruction bytes */
	if (x86_emu_last_inst_size &&
		x86_emu_last_inst_size == self->inst.size &&
		!memcmp(x86_emu_last_inst_bytes, buffer_ptr, x86_emu_last_inst_size))
		esim_finish = esim_finish_x86_last_inst;

	/* Execute instruction */
	X86ContextExecuteInst(self);
	
	/* Statistics */
	asEmu(emu)->instructions++;
}


/* Force a new 'eip' value for the context. The forced value should be the same as
 * the current 'eip' under normal circumstances. If it is not, speculative execution
 * starts, which will end on the next call to 'x86_ctx_recover'. */
void X86ContextSetEip(X86Context *self, unsigned int eip)
{
	/* Entering specmode */
	if (self->regs->eip != eip && !X86ContextGetState(self, X86ContextSpecMode))
	{
		X86ContextSetState(self, X86ContextSpecMode);
		x86_regs_copy(self->backup_regs, self->regs);
		self->regs->fpu_ctrl |= 0x3f; /* mask all FP exceptions on wrong path */
	}
	
	/* Set it */
	self->regs->eip = eip;
}


void X86ContextRecover(X86Context *self)
{
	assert(X86ContextGetState(self, X86ContextSpecMode));
	X86ContextClearState(self, X86ContextSpecMode);
	x86_regs_copy(self->regs, self->backup_regs);
	spec_mem_clear(self->spec_mem);
}


int X86ContextGetState(X86Context *self, X86ContextState state)
{
	return (self->state & state) > 0;
}


static void X86ContextUpdateState(X86Context *self, X86ContextState state)
{
	X86Emu *emu = self->emu;

	X86ContextState status_diff;
	char state_str[MAX_STRING_SIZE];

	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if (DOUBLE_LINKED_LIST_MEMBER(emu, running, self))
		DOUBLE_LINKED_LIST_REMOVE(emu, running, self);
	if (DOUBLE_LINKED_LIST_MEMBER(emu, suspended, self))
		DOUBLE_LINKED_LIST_REMOVE(emu, suspended, self);
	if (DOUBLE_LINKED_LIST_MEMBER(emu, zombie, self))
		DOUBLE_LINKED_LIST_REMOVE(emu, zombie, self);
	if (DOUBLE_LINKED_LIST_MEMBER(emu, finished, self))
		DOUBLE_LINKED_LIST_REMOVE(emu, finished, self);
	
	/* If the difference between the old and new state lies in other
	 * states other than 'x86_ctx_specmode', a reschedule is marked. */
	status_diff = self->state ^ state;
	if (status_diff & ~X86ContextSpecMode)
		emu->schedule_signal = 1;
	
	/* Update state */
	self->state = state;
	if (self->state & X86ContextFinished)
		self->state = X86ContextFinished
				| (state & X86ContextAlloc)
				| (state & X86ContextMapped);
	if (self->state & X86ContextZombie)
		self->state = X86ContextZombie
				| (state & X86ContextAlloc)
				| (state & X86ContextMapped);
	if (!(self->state & X86ContextSuspended) &&
		!(self->state & X86ContextFinished) &&
		!(self->state & X86ContextZombie) &&
		!(self->state & X86ContextLocked))
		self->state |= X86ContextRunning;
	else
		self->state &= ~X86ContextRunning;
	
	/* Insert context into the corresponding lists. */
	if (self->state & X86ContextRunning)
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, running, self);
	if (self->state & X86ContextZombie)
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, zombie, self);
	if (self->state & X86ContextFinished)
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, finished, self);
	if (self->state & X86ContextSuspended)
		DOUBLE_LINKED_LIST_INSERT_HEAD(emu, suspended, self);
	
	/* Dump new state (ignore 'x86_ctx_specmode' state, it's too frequent) */
	if (debug_status(x86_context_debug_category) && (status_diff & ~X86ContextSpecMode))
	{
		str_map_flags(&x86_context_state_map, self->state, state_str, sizeof state_str);
		X86ContextDebug("inst %lld: ctx %d changed state to %s\n",
			asEmu(emu)->instructions, self->pid, state_str);
	}

	/* Start/stop x86 timer depending on whether there are any contexts
	 * currently running. */
	if (emu->running_list_count)
		m2s_timer_start(asEmu(emu)->timer);
	else
		m2s_timer_stop(asEmu(emu)->timer);
}


void X86ContextSetState(X86Context *self, X86ContextState state)
{
	X86ContextUpdateState(self, self->state | state);
}


void X86ContextClearState(X86Context *self, X86ContextState state)
{
	X86ContextUpdateState(self, self->state & ~state);
}


/* Look for zombie child. If 'pid' is -1, the first finished child
 * in the zombie contexts list is return. Otherwise, 'pid' is the
 * pid of the child process. If no child has finished, return NULL. */
X86Context *X86ContextGetZombie(X86Context *self, int pid)
{
	X86Emu *emu = self->emu;
	X86Context *context;

	for (context = emu->zombie_list_head; context;
			context = context->zombie_list_next)
	{
		if (context->parent != self)
			continue;
		if (context->pid == pid || pid == -1)
			return context;
	}
	return NULL;
}


/* If the context is running a 'x86_emu_host_thread_suspend' thread,
 * cancel it and schedule call to 'x86_emu_process_events' */
void X86ContextHostThreadSuspendCancelUnsafe(X86Context *self)
{
	X86Emu *emu = self->emu;

	if (self->host_thread_suspend_active)
	{
		if (pthread_cancel(self->host_thread_suspend))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, self->pid);
		self->host_thread_suspend_active = 0;
		emu->process_events_force = 1;
	}
}


void X86ContextHostThreadSuspendCancel(X86Context *self)
{
	X86Emu *emu = self->emu;

	pthread_mutex_lock(&emu->process_events_mutex);
	X86ContextHostThreadSuspendCancelUnsafe(self);
	pthread_mutex_unlock(&emu->process_events_mutex);
}


/* If the context is running a 'ke_host_thread_timer' thread,
 * cancel it and schedule call to 'x86_emu_process_events' */
void X86ContextHostThreadTimerCancelUnsafe(X86Context *self)
{
	X86Emu *emu = self->emu;

	if (self->host_thread_timer_active)
	{
		if (pthread_cancel(self->host_thread_timer))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, self->pid);
		self->host_thread_timer_active = 0;
		emu->process_events_force = 1;
	}
}

void X86ContextHostThreadTimerCancel(X86Context *self)
{
	X86Emu *emu = self->emu;

	pthread_mutex_lock(&emu->process_events_mutex);
	X86ContextHostThreadTimerCancelUnsafe(self);
	pthread_mutex_unlock(&emu->process_events_mutex);
}


/* Suspend a context, using the specified callback function and data to decide
 * whether the process can wake up every time the x86 emulation events are
 * processed. */
void X86ContextSuspend(X86Context *self, X86ContextCanWakeupFunc can_wakeup_callback_func,
	void *can_wakeup_callback_data, X86ContextWakeupFunc wakeup_callback_func,
	void *wakeup_callback_data)
{
	X86Emu *emu = self->emu;

	/* Checks */
	assert(!X86ContextGetState(self, X86ContextSuspended));
	assert(!self->can_wakeup_callback_func);
	assert(!self->can_wakeup_callback_data);

	/* Suspend context */
	self->can_wakeup_callback_func = can_wakeup_callback_func;
	self->can_wakeup_callback_data = can_wakeup_callback_data;
	self->wakeup_callback_func = wakeup_callback_func;
	self->wakeup_callback_data = wakeup_callback_data;
	X86ContextSetState(self, X86ContextSuspended | X86ContextCallback);
	X86EmuProcessEventsSchedule(emu);
}


/* Finish a context group. This call does a subset of action of the 'x86_ctx_finish'
 * call, but for all parent and child contexts sharing a memory map. */
void X86ContextFinishGroup(X86Context *self, int state)
{
	X86Emu *emu = self->emu;
	X86Context *aux;

	/* Get group parent */
	if (self->group_parent)
		self = self->group_parent;
	assert(!self->group_parent);  /* Only one level */
	
	/* Context already finished */
	if (X86ContextGetState(self, X86ContextFinished | X86ContextZombie))
		return;

	/* Finish all contexts in the group */
	DOUBLE_LINKED_LIST_FOR_EACH(emu, context, aux)
	{
		if (aux->group_parent != self && aux != self)
			continue;

		if (X86ContextGetState(aux, X86ContextZombie))
			X86ContextSetState(aux, X86ContextFinished);
		if (X86ContextGetState(aux, X86ContextHandler))
			X86ContextReturnFromSignalHandler(aux);
		X86ContextHostThreadSuspendCancel(aux);
		X86ContextHostThreadTimerCancel(aux);

		/* Child context of 'ctx' goes to state 'finished'.
		 * Context 'ctx' goes to state 'zombie' or 'finished' if it has a parent */
		if (aux == self)
			X86ContextSetState(aux, aux->parent ? X86ContextZombie : X86ContextFinished);
		else
			X86ContextSetState(aux, X86ContextFinished);
		aux->exit_code = state;
	}

	/* Process events */
	X86EmuProcessEventsSchedule(emu);
}


/* Finish a context. If the context has no parent, its state will be set
 * to 'x86_ctx_finished'. If it has, its state is set to 'x86_ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void X86ContextFinish(X86Context *self, int state)
{
	X86Emu *emu = self->emu;
	X86Context *aux;
	
	/* Context already finished */
	if (X86ContextGetState(self, X86ContextFinished | X86ContextZombie))
		return;
	
	/* If context is waiting for host events, cancel spawned host threads. */
	X86ContextHostThreadSuspendCancel(self);
	X86ContextHostThreadTimerCancel(self);

	/* From now on, all children have lost their parent. If a child is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore. */
	DOUBLE_LINKED_LIST_FOR_EACH(emu, context, aux)
	{
		if (aux->parent == self)
		{
			aux->parent = NULL;
			if (X86ContextGetState(aux, X86ContextZombie))
				X86ContextSetState(aux, X86ContextFinished);
		}
	}

	/* Send finish signal to parent */
	if (self->exit_signal && self->parent)
	{
		x86_sys_debug("  sending signal %d to pid %d\n",
			self->exit_signal, self->parent->pid);
		x86_sigset_add(&self->parent->signal_mask_table->pending,
			self->exit_signal);
		X86EmuProcessEventsSchedule(emu);
	}

	/* If clear_child_tid was set, a futex() call must be performed on
	 * that pointer. Also wake up futexes in the robust list. */
	if (self->clear_child_tid)
	{
		unsigned int zero = 0;
		mem_write(self->mem, self->clear_child_tid, 4, &zero);
		X86ContextFutexWake(self, self->clear_child_tid, 1, -1);
	}
	X86ContextExitRobustList(self);

	/* If we are in a signal handler, stop it. */
	if (X86ContextGetState(self, X86ContextHandler))
		X86ContextReturnFromSignalHandler(self);

	/* Finish context */
	X86ContextSetState(self, self->parent ? X86ContextZombie : X86ContextFinished);
	self->exit_code = state;
	X86EmuProcessEventsSchedule(emu);
}


int X86ContextFutexWake(X86Context *self, unsigned int futex, unsigned int count,
		unsigned int bitset)
{
	X86Emu *emu = self->emu;
	X86Context *wakeup_ctx;

	int wakeup_count = 0;

	/* Look for threads suspended in this futex */
	while (count)
	{
		wakeup_ctx = NULL;
		for (self = emu->suspended_list_head; self; self = self->suspended_list_next)
		{
			if (!X86ContextGetState(self, X86ContextFutex) || self->wakeup_futex != futex)
				continue;
			if (!(self->wakeup_futex_bitset & bitset))
				continue;
			if (!wakeup_ctx || self->wakeup_futex_sleep < wakeup_ctx->wakeup_futex_sleep)
				wakeup_ctx = self;
		}

		if (wakeup_ctx)
		{
			/* Wake up context */
			X86ContextClearState(wakeup_ctx, X86ContextSuspended | X86ContextFutex);
			x86_sys_debug("  futex 0x%x: thread %d woken up\n", futex, wakeup_ctx->pid);
			wakeup_count++;
			count--;

			/* Set system call return value */
			wakeup_ctx->regs->eax = 0;
		}
		else
		{
			break;
		}
	}
	return wakeup_count;
}


void X86ContextExitRobustList(X86Context *self)
{
	unsigned int next, lock_entry, offset, lock_word;

	/* Read the offset from the list head. This is how the structure is
	 * represented in the kernel:
	 * struct robust_list {
	 *      struct robust_list __user *next;
	 * }
	 * struct robust_list_head {
	 *	struct robust_list list;
	 *	long futex_offset;
	 *	struct robust_list __user *list_op_pending;
	 * }
	 * See linux/Documentation/robust-futex-ABI.txt for details
	 * about robust futex wake up at thread exit.
	 */

	lock_entry = self->robust_list_head;
	if (!lock_entry)
		return;
	
	x86_sys_debug("ctx %d: processing robust futex list\n",
		self->pid);
	for (;;)
	{
		mem_read(self->mem, lock_entry, 4, &next);
		mem_read(self->mem, lock_entry + 4, 4, &offset);
		mem_read(self->mem, lock_entry + offset, 4, &lock_word);

		x86_sys_debug("  lock_entry=0x%x: offset=%d, lock_word=0x%x\n",
			lock_entry, offset, lock_word);

		/* Stop processing list if 'next' points to robust list */
		if (!next || next == self->robust_list_head)
			break;
		lock_entry = next;
	}
}


/* Generate virtual file '/proc/self/maps' and return it in 'path'. */
void X86ContextProcSelfMaps(X86Context *self, char *path, int size)
{
	unsigned int start, end;
	enum mem_access_t perm, page_perm;
	struct mem_page_t *page;
	struct mem_t *mem = self->mem;
	int fd;
	FILE *f = NULL;

	/* Create temporary file */
	snprintf(path, size, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		fatal("ctx_gen_proc_self_maps: cannot create temporary file");

	/* Get the first page */
	end = 0;
	for (;;)
	{
		/* Get start of next range */
		page = mem_page_get_next(mem, end);
		if (!page)
			break;
		start = page->tag;
		end = page->tag;
		perm = page->perm & (mem_access_read | mem_access_write | mem_access_exec);

		/* Get end of range */
		for (;;)
		{
			page = mem_page_get(mem, end + MEM_PAGE_SIZE);
			if (!page)
				break;
			page_perm = page->perm & (mem_access_read | mem_access_write | mem_access_exec);
			if (page_perm != perm)
				break;
			end += MEM_PAGE_SIZE;
			perm = page_perm;
		}

		/* Dump range */ 
		fprintf(f, "%08x-%08x %c%c%c%c 00000000 00:00", start, end + MEM_PAGE_SIZE,
			perm & mem_access_read ? 'r' : '-',
			perm & mem_access_write ? 'w' : '-',
			perm & mem_access_exec ? 'x' : '-',
			'p');
		fprintf(f, "\n");
	}

	/* Close file */
	fclose(f);
}


/* Generate virtual file '/proc/cpuinfo' and return it in 'path'. */
void X86ContextProcCPUInfo(X86Context *self, char *path, int size)
{
	FILE *f = NULL;
	
	int i;
	int j;
	int node;
	int fd;

	/* Create temporary file */
	snprintf(path, size, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		fatal("ctx_gen_proc_self_maps: cannot create temporary file");

	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			node = i * x86_cpu_num_threads + j;
			fprintf(f, "processor : %d\n", node);
			fprintf(f, "vendor_id : Multi2Sim\n");
			fprintf(f, "cpu family : 6\n");
			fprintf(f, "model : 23\n");
			fprintf(f, "model name : Multi2Sim\n");
			fprintf(f, "stepping : 6\n");
			fprintf(f, "microcode : 0x607\n");
			fprintf(f, "cpu MHz : 800.000\n");
			fprintf(f, "cache size : 3072 KB\n");
			fprintf(f, "physical id : 0\n");
			fprintf(f, "siblings : %d\n", x86_cpu_num_cores * x86_cpu_num_threads);
			fprintf(f, "core id : %d\n", i);
			fprintf(f, "cpu cores : %d\n", x86_cpu_num_cores);
			fprintf(f, "apicid : %d\n", node);
			fprintf(f, "initial apicid : %d\n", node);
			fprintf(f, "fpu : yes\n");
			fprintf(f, "fpu_exception : yes\n");
			fprintf(f, "cpuid level : 10\n");
			fprintf(f, "wp : yes\n");
			fprintf(f, "flags : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr "
					"pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse "
					"sse2 ss ht tm pbe syscall nx lm constant_tsc arch_perfmon "
					"pebs bts rep_good nopl aperfmperf pni dtes64 monitor ds_cpl "
					"vmx est tm2 ssse3 cx16 xtpr pdcm sse4_1 lahf_lm ida dtherm "
					"tpr_shadow vnmi flexpriority\n");
			fprintf(f, "bogomips : 4189.40\n");
			fprintf(f, "clflush size : 32\n");
			fprintf(f, "cache_alignment : 32\n");
			fprintf(f, "address sizes : 32 bits physical, 32 bits virtual\n");
			fprintf(f, "power management :\n");
			fprintf(f, "\n");
		}
	}

	/* Close file */
	fclose(f);
}


void *X86EmuHostThreadSuspend(void *arg)
{
	X86Context *self = asX86Context(arg);
	X86Emu *emu = self->emu;

	long long now = esim_real_time();

	/* Detach this thread - we don't want the parent to have to join it to release
	 * its resources. The thread termination can be observed by atomically checking
	 * the 'self->host_thread_suspend_active' flag. */
	pthread_detach(pthread_self());

	/* Context suspended in 'poll' system call */
	if (X86ContextGetState(self, X86ContextNanosleep))
	{
		long long timeout;

		/* Calculate remaining sleep time in microseconds */
		timeout = self->wakeup_time > now ? self->wakeup_time - now : 0;
		usleep(timeout);

	}
	else if (X86ContextGetState(self, X86ContextPoll))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err, timeout;

		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(self->file_desc_table, self->wakeup_fd);
		if (!fd)
			fatal("syscall 'poll': invalid 'wakeup_fd'");

		/* Calculate timeout for host call in milliseconds from now */
		if (!self->wakeup_time)
			timeout = -1;
		else if (self->wakeup_time < now)
			timeout = 0;
		else
			timeout = (self->wakeup_time - now) / 1000;

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = ((self->wakeup_events & 4) ? POLLOUT : 0) | ((self->wakeup_events & 1) ? POLLIN : 0);
		err = poll(&host_fds, 1, timeout);
		if (err < 0)
			fatal("syscall 'poll': unexpected error in host 'poll'");
	}
	else if (X86ContextGetState(self, X86ContextRead))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(self->file_desc_table, self->wakeup_fd);
		if (!fd)
			fatal("syscall 'read': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLIN;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'read': unexpected error in host 'poll'");
	}
	else if (X86ContextGetState(self, X86ContextWrite))
	{
		struct x86_file_desc_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = x86_file_desc_table_entry_get(self->file_desc_table, self->wakeup_fd);
		if (!fd)
			fatal("syscall 'write': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLOUT;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'write': unexpected error in host 'write'");

	}

	/* Event occurred - thread finishes */
	pthread_mutex_lock(&emu->process_events_mutex);
	emu->process_events_force = 1;
	self->host_thread_suspend_active = 0;
	pthread_mutex_unlock(&emu->process_events_mutex);
	return NULL;
}


void *X86ContextHostThreadTimer(void *arg)
{
	X86Context *self = asX86Context(arg);
	X86Emu *emu = self->emu;

	long long now = esim_real_time();
	struct timespec ts;
	long long sleep_time;  /* In usec */

	/* Detach this thread - we don't want the parent to have to join it to release
	 * its resources. The thread termination can be observed by thread-safely checking
	 * the 'self->host_thread_timer_active' flag. */
	pthread_detach(pthread_self());

	/* Calculate sleep time, and sleep only if it is greater than 0 */
	if (self->host_thread_timer_wakeup > now)
	{
		sleep_time = self->host_thread_timer_wakeup - now;
		ts.tv_sec = sleep_time / 1000000;
		ts.tv_nsec = (sleep_time % 1000000) * 1000;  /* nsec */
		nanosleep(&ts, NULL);
	}

	/* Timer expired, schedule call to 'X86EmuProcessEvents' */
	pthread_mutex_lock(&emu->process_events_mutex);
	emu->process_events_force = 1;
	self->host_thread_timer_active = 0;
	pthread_mutex_unlock(&emu->process_events_mutex);
	return NULL;
}




/*
 * Non-Class
 */

int x86_context_debug_category;

struct str_map_t x86_context_state_map =
{
	18, {
		{ "running",      X86ContextRunning },
		{ "specmode",     X86ContextSpecMode },
		{ "suspended",    X86ContextSuspended },
		{ "finished",     X86ContextFinished },
		{ "exclusive",    X86ContextExclusive },
		{ "locked",       X86ContextLocked },
		{ "handler",      X86ContextHandler },
		{ "sigsuspend",   X86ContextSigsuspend },
		{ "nanosleep",    X86ContextNanosleep },
		{ "poll",         X86ContextPoll },
		{ "read",         X86ContextRead },
		{ "write",        X86ContextWrite },
		{ "waitpid",      X86ContextWaitpid },
		{ "zombie",       X86ContextZombie },
		{ "futex",        X86ContextFutex },
		{ "alloc",        X86ContextAlloc },
		{ "callback",     X86ContextCallback },
		{ "mapped",       X86ContextMapped }
	}
};
