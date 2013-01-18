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

#include <assert.h>

#include <arch/southern-islands/emu/emu.h>
#include <driver/opencl-old/evergreen/opencl.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
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


int x86_ctx_debug_category;

int EV_X86_CTX_IPC_REPORT;

static char *help_x86_ctx_ipc_report =
	"The IPC (instructions-per-cycle) report file shows performance value for a\n"
	"context at specific intervals. If a context spawns child contexts, only IPC\n"
	"statistics for the parent context are shown. The following fields are shown in\n"
	"each record:\n"
	"\n"
	"  <cycle>\n"
	"      Current simulation cycle. The increment between this value and the value\n"
	"      shown in the next record is the interval specified in the context\n"
	"      configuration file.\n"
	"\n"
	"  <inst>\n"
	"      Number of non-speculative instructions executed in the current interval.\n"
	"\n"
	"  <ipc-glob>\n"
	"      Global IPC observed so far. This value is equal to the number of executed\n"
	"      non-speculative instructions divided by the current cycle.\n"
	"\n"
	"  <ipc-int>\n"
	"      IPC observed in the current interval. This value is equal to the number\n"
	"      of instructions executed in the current interval divided by the number of\n"
	"      cycles of the interval.\n"
	"\n";


static struct str_map_t x86_ctx_status_map =
{
	16, {
		{ "running",      x86_ctx_running },
		{ "specmode",     x86_ctx_spec_mode },
		{ "suspended",    x86_ctx_suspended },
		{ "finished",     x86_ctx_finished },
		{ "exclusive",    x86_ctx_exclusive },
		{ "locked",       x86_ctx_locked },
		{ "handler",      x86_ctx_handler },
		{ "sigsuspend",   x86_ctx_sigsuspend },
		{ "nanosleep",    x86_ctx_nanosleep },
		{ "poll",         x86_ctx_poll },
		{ "read",         x86_ctx_read },
		{ "write",        x86_ctx_write },
		{ "waitpid",      x86_ctx_waitpid },
		{ "zombie",       x86_ctx_zombie },
		{ "futex",        x86_ctx_futex },
		{ "alloc",        x86_ctx_alloc }
	}
};


static struct x86_ctx_t *ctx_do_create()
{
	struct x86_ctx_t *ctx;

	/* Initialize */
	ctx = xcalloc(1, sizeof(struct x86_ctx_t));
	ctx->pid = x86_emu->current_pid++;

	/* Update status so that the context is inserted in the
	 * corresponding lists. The x86_ctx_running parameter has no
	 * effect, since it will be updated later. */
	x86_ctx_set_status(ctx, x86_ctx_running);
	x86_emu_list_insert_head(x86_emu_list_context, ctx);

	/* Structures */
	ctx->regs = x86_regs_create();
	ctx->backup_regs = x86_regs_create();
	ctx->signal_mask_table = x86_signal_mask_table_create();

	/* Return context */
	return ctx;
}


struct x86_ctx_t *x86_ctx_create(void)
{
	struct x86_ctx_t *ctx;
	
	ctx = ctx_do_create();

	/* Loader */
	ctx->loader = x86_loader_create();

	/* Memory */
	ctx->address_space_index = mmu_address_space_new();
	ctx->mem = mem_create();
	ctx->spec_mem = spec_mem_create(ctx->mem);

	/* Signal handlers and file descriptor table */
	ctx->signal_handler_table = x86_signal_handler_table_create();
	ctx->file_desc_table = x86_file_desc_table_create();
	
	return ctx;
}


struct x86_ctx_t *x86_ctx_clone(struct x86_ctx_t *ctx)
{
        struct x86_ctx_t *new;

	new = ctx_do_create();

	/* Register file contexts are copied from parent. */
	x86_regs_copy(new->regs, ctx->regs);

	/* The memory image of the cloned context if the same.
	 * The memory structure must be only freed by the parent
	 * when all its children have been killed.
	 * The set of signal handlers is the same, too. */
	new->address_space_index = ctx->address_space_index;
	new->mem = mem_link(ctx->mem);
	new->spec_mem = spec_mem_create(new->mem);

	/* Loader */
	new->loader = x86_loader_link(ctx->loader);

	/* Signal handlers and file descriptor table */
	new->signal_handler_table = x86_signal_handler_table_link(ctx->signal_handler_table);
	new->file_desc_table = x86_file_desc_table_link(ctx->file_desc_table);

	/* Libc segment */
	new->glibc_segment_base = ctx->glibc_segment_base;
	new->glibc_segment_limit = ctx->glibc_segment_limit;

	/* Update other fields. */
	new->parent = ctx;

	/* Return new context */
	return new;

}


struct x86_ctx_t *x86_ctx_fork(struct x86_ctx_t *ctx)
{
	struct x86_ctx_t *new;

	new = ctx_do_create();

	/* Copy registers */
	x86_regs_copy(new->regs, ctx->regs);

	/* Memory */
	new->address_space_index = mmu_address_space_new();
	new->mem = mem_create();
	new->spec_mem = spec_mem_create(new->mem);
	mem_clone(new->mem, ctx->mem);

	/* Loader */
	new->loader = x86_loader_link(ctx->loader);

	/* Signal handlers and file descriptor table */
	new->signal_handler_table = x86_signal_handler_table_create();
	new->file_desc_table = x86_file_desc_table_create();

	/* Libc segment */
	new->glibc_segment_base = ctx->glibc_segment_base;
	new->glibc_segment_limit = ctx->glibc_segment_limit;

	/* Set parent */
	new->parent = ctx;

	/* Return new context */
	return new;
}


/* Free a context */
void x86_ctx_free(struct x86_ctx_t *ctx)
{
	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!x86_ctx_get_status(ctx, x86_ctx_finished | x86_ctx_zombie))
		x86_ctx_finish(ctx, 0);
	
	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!x86_emu_list_member(x86_emu_list_running, ctx));
	assert(!x86_emu_list_member(x86_emu_list_suspended, ctx));
	assert(!x86_emu_list_member(x86_emu_list_zombie, ctx));
	assert(x86_emu_list_member(x86_emu_list_finished, ctx));
	x86_emu_list_remove(x86_emu_list_finished, ctx);
		
	/* Free private structures */
	x86_regs_free(ctx->regs);
	x86_regs_free(ctx->backup_regs);
	x86_signal_mask_table_free(ctx->signal_mask_table);
	spec_mem_free(ctx->spec_mem);

	/* Unlink shared structures */
	x86_loader_unlink(ctx->loader);
	x86_signal_handler_table_unlink(ctx->signal_handler_table);
	x86_file_desc_table_unlink(ctx->file_desc_table);
	mem_unlink(ctx->mem);

	/* Remove context from contexts list and free */
	x86_emu_list_remove(x86_emu_list_context, ctx);
	x86_ctx_debug("context %d freed\n", ctx->pid);

	/* Free context */
	free(ctx);
}


void x86_ctx_dump(struct x86_ctx_t *ctx, FILE *f)
{
	char status_str[MAX_STRING_SIZE];

	fprintf(f, "  pid=%d\n", ctx->pid);
	str_map_flags(&x86_ctx_status_map, ctx->status, status_str, sizeof status_str);
	fprintf(f, "  status=%s\n", status_str);
	if (!ctx->parent)
		fprintf(f, "  parent=(null)\n");
	else
		fprintf(f, "  parent=%d\n", ctx->parent->pid);
	fprintf(f, "  heap break: 0x%x\n", ctx->mem->heap_break);

	/* Signal masks */
	fprintf(f, "  blocked signal mask: 0x%llx ", ctx->signal_mask_table->blocked);
	x86_sigset_dump(ctx->signal_mask_table->blocked, f);
	fprintf(f, "\n  pending signals: 0x%llx ", ctx->signal_mask_table->pending);
	x86_sigset_dump(ctx->signal_mask_table->pending, f);
	fprintf(f, "\n");
}


void x86_ctx_execute(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned char buffer[20];
	unsigned char *buffer_ptr;

	int spec_mode;

	/* Memory permissions should not be checked if the context is executing in
	 * speculative mode. This will prevent guest segmentation faults to occur. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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
	x86_disasm(buffer_ptr, regs->eip, &ctx->inst);
	if (ctx->inst.opcode == x86_op_none && !spec_mode)
		fatal("0x%x: not supported x86 instruction (%02x %02x %02x %02x...)",
			regs->eip, buffer_ptr[0], buffer_ptr[1], buffer_ptr[2], buffer_ptr[3]);


	/* Stop if instruction matches last instruction bytes */
	if (x86_emu_last_inst_size &&
		x86_emu_last_inst_size == ctx->inst.size &&
		!memcmp(x86_emu_last_inst_bytes, buffer_ptr, x86_emu_last_inst_size))
		esim_finish = esim_finish_x86_last_inst;

	/* Execute instruction */
	x86_isa_execute_inst(ctx);
	
	/* Statistics */
	x86_emu->inst_count++;
}


/* Force a new 'eip' value for the context. The forced value should be the same as
 * the current 'eip' under normal circumstances. If it is not, speculative execution
 * starts, which will end on the next call to 'x86_ctx_recover'. */
void x86_ctx_set_eip(struct x86_ctx_t *ctx, unsigned int eip)
{
	/* Entering specmode */
	if (ctx->regs->eip != eip && !x86_ctx_get_status(ctx, x86_ctx_spec_mode))
	{
		x86_ctx_set_status(ctx, x86_ctx_spec_mode);
		x86_regs_copy(ctx->backup_regs, ctx->regs);
		ctx->regs->fpu_ctrl |= 0x3f; /* mask all FP exceptions on wrong path */
	}
	
	/* Set it */
	ctx->regs->eip = eip;
}


void x86_ctx_recover(struct x86_ctx_t *ctx)
{
	assert(x86_ctx_get_status(ctx, x86_ctx_spec_mode));
	x86_ctx_clear_status(ctx, x86_ctx_spec_mode);
	x86_regs_copy(ctx->regs, ctx->backup_regs);
	spec_mem_clear(ctx->spec_mem);
}


int x86_ctx_get_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status)
{
	return (ctx->status & status) > 0;
}


static void x86_ctx_update_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status)
{
	enum x86_ctx_status_t status_diff;

	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if (x86_emu_list_member(x86_emu_list_running, ctx))
		x86_emu_list_remove(x86_emu_list_running, ctx);
	if (x86_emu_list_member(x86_emu_list_suspended, ctx))
		x86_emu_list_remove(x86_emu_list_suspended, ctx);
	if (x86_emu_list_member(x86_emu_list_zombie, ctx))
		x86_emu_list_remove(x86_emu_list_zombie, ctx);
	if (x86_emu_list_member(x86_emu_list_finished, ctx))
		x86_emu_list_remove(x86_emu_list_finished, ctx);
	if (x86_emu_list_member(x86_emu_list_alloc, ctx))
		x86_emu_list_remove(x86_emu_list_alloc, ctx);
	
	/* If the difference between the old and new status lies in other
	 * states other than 'x86_ctx_specmode', a reschedule is marked. */
	status_diff = ctx->status ^ status;
	if (status_diff & ~x86_ctx_spec_mode)
		x86_emu->context_reschedule = 1;
	
	/* Update status */
	ctx->status = status;
	if (ctx->status & x86_ctx_finished)
		ctx->status = x86_ctx_finished | (status & x86_ctx_alloc);
	if (ctx->status & x86_ctx_zombie)
		ctx->status = x86_ctx_zombie | (status & x86_ctx_alloc);
	if (!(ctx->status & x86_ctx_suspended) &&
		!(ctx->status & x86_ctx_finished) &&
		!(ctx->status & x86_ctx_zombie) &&
		!(ctx->status & x86_ctx_locked))
		ctx->status |= x86_ctx_running;
	else
		ctx->status &= ~x86_ctx_running;
	
	/* Insert context into the corresponding lists. */
	if (ctx->status & x86_ctx_running)
		x86_emu_list_insert_head(x86_emu_list_running, ctx);
	if (ctx->status & x86_ctx_zombie)
		x86_emu_list_insert_head(x86_emu_list_zombie, ctx);
	if (ctx->status & x86_ctx_finished)
		x86_emu_list_insert_head(x86_emu_list_finished, ctx);
	if (ctx->status & x86_ctx_suspended)
		x86_emu_list_insert_head(x86_emu_list_suspended, ctx);
	if (ctx->status & x86_ctx_alloc)
		x86_emu_list_insert_head(x86_emu_list_alloc, ctx);
	
	/* Dump new status (ignore 'x86_ctx_specmode' status, it's too frequent) */
	if (debug_status(x86_ctx_debug_category) && (status_diff & ~x86_ctx_spec_mode))
	{
		char sstatus[200];
		str_map_flags(&x86_ctx_status_map, ctx->status, sstatus, 200);
		x86_ctx_debug("ctx %d changed status to %s\n",
			ctx->pid, sstatus);
	}

	/* Start/stop x86 timer depending on whether there are any contexts
	 * currently running. */
	if (x86_emu->running_list_count)
		m2s_timer_start(x86_emu->timer);
	else
		m2s_timer_stop(x86_emu->timer);
}


void x86_ctx_set_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status)
{
	x86_ctx_update_status(ctx, ctx->status | status);
}


void x86_ctx_clear_status(struct x86_ctx_t *ctx, enum x86_ctx_status_t status)
{
	x86_ctx_update_status(ctx, ctx->status & ~status);
}


/* Look for a context matching pid in the list of existing
 * contexts of the kernel. */
struct x86_ctx_t *x86_ctx_get(int pid)
{
	struct x86_ctx_t *ctx;

	ctx = x86_emu->context_list_head;
	while (ctx && ctx->pid != pid)
		ctx = ctx->context_list_next;
	return ctx;
}


/* Look for zombie child. If 'pid' is -1, the first finished child
 * in the zombie contexts list is return. Otherwise, 'pid' is the
 * pid of the child process. If no child has finished, return NULL. */
struct x86_ctx_t *x86_ctx_get_zombie(struct x86_ctx_t *parent, int pid)
{
	struct x86_ctx_t *ctx;

	for (ctx = x86_emu->zombie_list_head; ctx; ctx = ctx->zombie_list_next)
	{
		if (ctx->parent != parent)
			continue;
		if (ctx->pid == pid || pid == -1)
			return ctx;
	}
	return NULL;
}


/* If the context is running a 'x86_emu_host_thread_suspend' thread,
 * cancel it and schedule call to 'x86_emu_process_events' */

void __x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx)
{
	if (ctx->host_thread_suspend_active)
	{
		if (pthread_cancel(ctx->host_thread_suspend))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, ctx->pid);
		ctx->host_thread_suspend_active = 0;
		x86_emu->process_events_force = 1;
	}
}

void x86_ctx_host_thread_suspend_cancel(struct x86_ctx_t *ctx)
{
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	__x86_ctx_host_thread_suspend_cancel(ctx);
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
}


/* If the context is running a 'ke_host_thread_timer' thread,
 * cancel it and schedule call to 'x86_emu_process_events' */

void __x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx)
{
	if (ctx->host_thread_timer_active)
	{
		if (pthread_cancel(ctx->host_thread_timer))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, ctx->pid);
		ctx->host_thread_timer_active = 0;
		x86_emu->process_events_force = 1;
	}
}

void x86_ctx_host_thread_timer_cancel(struct x86_ctx_t *ctx)
{
	pthread_mutex_lock(&x86_emu->process_events_mutex);
	__x86_ctx_host_thread_timer_cancel(ctx);
	pthread_mutex_unlock(&x86_emu->process_events_mutex);
}


/* Suspend a context, using the specified callback function and data to decide
 * whether the process can wake up every time the x86 emulation events are
 * processed. */
void x86_ctx_suspend(struct x86_ctx_t *ctx, x86_ctx_can_wakeup_callback_func_t can_wakeup_callback_func,
	void *can_wakeup_callback_data, x86_ctx_wakeup_callback_func_t wakeup_callback_func,
	void *wakeup_callback_data)
{
	/* Checks */
	assert(!x86_ctx_get_status(ctx, x86_ctx_suspended));
	assert(!ctx->can_wakeup_callback_func);
	assert(!ctx->can_wakeup_callback_data);

	/* Suspend context */
	ctx->can_wakeup_callback_func = can_wakeup_callback_func;
	ctx->can_wakeup_callback_data = can_wakeup_callback_data;
	ctx->wakeup_callback_func = wakeup_callback_func;
	ctx->wakeup_callback_data = wakeup_callback_data;
	x86_ctx_set_status(ctx, x86_ctx_suspended | x86_ctx_callback);
	x86_emu_process_events_schedule();
}


/* Finish a context group. This call does a subset of action of the 'x86_ctx_finish'
 * call, but for all parent and child contexts sharing a memory map. */
void x86_ctx_finish_group(struct x86_ctx_t *ctx, int status)
{
	struct x86_ctx_t *aux;

	/* Get group parent */
	if (ctx->group_parent)
		ctx = ctx->group_parent;
	assert(!ctx->group_parent);  /* Only one level */
	
	/* Context already finished */
	if (x86_ctx_get_status(ctx, x86_ctx_finished | x86_ctx_zombie))
		return;

	/* Finish all contexts in the group */
	DOUBLE_LINKED_LIST_FOR_EACH(x86_emu, context, aux)
	{
		if (aux->group_parent != ctx && aux != ctx)
			continue;

		if (x86_ctx_get_status(aux, x86_ctx_zombie))
			x86_ctx_set_status(aux, x86_ctx_finished);
		if (x86_ctx_get_status(aux, x86_ctx_handler))
			x86_signal_handler_return(aux);
		x86_ctx_host_thread_suspend_cancel(aux);
		x86_ctx_host_thread_timer_cancel(aux);

		/* Child context of 'ctx' goes to state 'finished'.
		 * Context 'ctx' goes to state 'zombie' or 'finished' if it has a parent */
		if (aux == ctx)
			x86_ctx_set_status(aux, aux->parent ? x86_ctx_zombie : x86_ctx_finished);
		else
			x86_ctx_set_status(aux, x86_ctx_finished);
		aux->exit_code = status;
	}

	/* Process events */
	x86_emu_process_events_schedule();
}


/* Finish a context. If the context has no parent, its status will be set
 * to 'x86_ctx_finished'. If it has, its status is set to 'x86_ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void x86_ctx_finish(struct x86_ctx_t *ctx, int status)
{
	struct x86_ctx_t *aux;
	
	/* Context already finished */
	if (x86_ctx_get_status(ctx, x86_ctx_finished | x86_ctx_zombie))
		return;
	
	/* If context is waiting for host events, cancel spawned host threads. */
	x86_ctx_host_thread_suspend_cancel(ctx);
	x86_ctx_host_thread_timer_cancel(ctx);

	/* From now on, all children have lost their parent. If a child is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore. */
	DOUBLE_LINKED_LIST_FOR_EACH(x86_emu, context, aux)
	{
		if (aux->parent == ctx)
		{
			aux->parent = NULL;
			if (x86_ctx_get_status(aux, x86_ctx_zombie))
				x86_ctx_set_status(aux, x86_ctx_finished);
		}
	}

	/* Send finish signal to parent */
	if (ctx->exit_signal && ctx->parent)
	{
		x86_sys_debug("  sending signal %d to pid %d\n",
			ctx->exit_signal, ctx->parent->pid);
		x86_sigset_add(&ctx->parent->signal_mask_table->pending,
			ctx->exit_signal);
		x86_emu_process_events_schedule();
	}

	/* If clear_child_tid was set, a futex() call must be performed on
	 * that pointer. Also wake up futexes in the robust list. */
	if (ctx->clear_child_tid)
	{
		unsigned int zero = 0;
		mem_write(ctx->mem, ctx->clear_child_tid, 4, &zero);
		x86_ctx_futex_wake(ctx, ctx->clear_child_tid, 1, -1);
	}
	x86_ctx_exit_robust_list(ctx);

	/* If we are in a signal handler, stop it. */
	if (x86_ctx_get_status(ctx, x86_ctx_handler))
		x86_signal_handler_return(ctx);

	/* Finish context */
	x86_ctx_set_status(ctx, ctx->parent ? x86_ctx_zombie : x86_ctx_finished);
	ctx->exit_code = status;
	x86_emu_process_events_schedule();
}


int x86_ctx_futex_wake(struct x86_ctx_t *ctx, unsigned int futex, unsigned int count, unsigned int bitset)
{
	int wakeup_count = 0;
	struct x86_ctx_t *wakeup_ctx;

	/* Look for threads suspended in this futex */
	while (count)
	{
		wakeup_ctx = NULL;
		for (ctx = x86_emu->suspended_list_head; ctx; ctx = ctx->suspended_list_next)
		{
			if (!x86_ctx_get_status(ctx, x86_ctx_futex) || ctx->wakeup_futex != futex)
				continue;
			if (!(ctx->wakeup_futex_bitset & bitset))
				continue;
			if (!wakeup_ctx || ctx->wakeup_futex_sleep < wakeup_ctx->wakeup_futex_sleep)
				wakeup_ctx = ctx;
		}

		if (wakeup_ctx)
		{
			x86_ctx_clear_status(wakeup_ctx, x86_ctx_suspended | x86_ctx_futex);
			x86_sys_debug("  futex 0x%x: thread %d woken up\n", futex, wakeup_ctx->pid);
			wakeup_count++;
			count--;
		}
		else
		{
			break;
		}
	}
	return wakeup_count;
}


void x86_ctx_exit_robust_list(struct x86_ctx_t *ctx)
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

	lock_entry = ctx->robust_list_head;
	if (!lock_entry)
		return;
	
	x86_sys_debug("ctx %d: processing robust futex list\n",
		ctx->pid);
	for (;;)
	{
		mem_read(ctx->mem, lock_entry, 4, &next);
		mem_read(ctx->mem, lock_entry + 4, 4, &offset);
		mem_read(ctx->mem, lock_entry + offset, 4, &lock_word);

		x86_sys_debug("  lock_entry=0x%x: offset=%d, lock_word=0x%x\n",
			lock_entry, offset, lock_word);

		/* Stop processing list if 'next' points to robust list */
		if (!next || next == ctx->robust_list_head)
			break;
		lock_entry = next;
	}
}


/* Generate virtual file '/proc/self/maps' and return it in 'path'. */
void x86_ctx_gen_proc_self_maps(struct x86_ctx_t *ctx, char *path)
{
	unsigned int start, end;
	enum mem_access_t perm, page_perm;
	struct mem_page_t *page;
	struct mem_t *mem = ctx->mem;
	int fd;
	FILE *f = NULL;

	/* Create temporary file */
	strcpy(path, "/tmp/m2s.XXXXXX");
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




/*
 * IPC report
 */

struct x86_ctx_ipc_report_stack_t
{
	int pid;
	long long inst_count;
};

void x86_ctx_ipc_report_schedule(struct x86_ctx_t *ctx)
{
	struct x86_ctx_ipc_report_stack_t *stack;
	FILE *f = ctx->loader->ipc_report_file;
	int i;

	/* Initialize */
	stack = xcalloc(1, sizeof(struct x86_ctx_ipc_report_stack_t));
	assert(ctx->loader->ipc_report_file);
	assert(ctx->loader->ipc_report_interval > 0);
	stack->pid = ctx->pid;

	/* Print header */
	fprintf(f, "%s", help_x86_ctx_ipc_report);
	fprintf(f, "%10s %8s %10s %10s\n", "cycle", "inst", "ipc-glob", "ipc-int");
	for (i = 0; i < 43; i++)
		fprintf(f, "-");
	fprintf(f, "\n");

	/* Schedule first event */
	esim_schedule_event(EV_X86_CTX_IPC_REPORT, stack,
		ctx->loader->ipc_report_interval);
}

void x86_ctx_ipc_report_handler(int event, void *data)
{
	struct x86_ctx_ipc_report_stack_t *stack = data;
	struct x86_ctx_t *ctx;

	long long inst_count;
	double ipc_interval;
	double ipc_global;

	/* Get context. If it does not exist anymore, no more
	 * events to schedule. */
	ctx = x86_ctx_get(stack->pid);
	if (!ctx || x86_ctx_get_status(ctx, x86_ctx_finished) || esim_finish)
	{
		free(stack);
		return;
	}

	/* Dump new IPC */
	assert(ctx->loader->ipc_report_interval);
	inst_count = ctx->inst_count - stack->inst_count;
	ipc_global = esim_cycle ? (double) ctx->inst_count / esim_cycle : 0.0;
	ipc_interval = (double) inst_count / ctx->loader->ipc_report_interval;
	fprintf(ctx->loader->ipc_report_file, "%10lld %8lld %10.4f %10.4f\n",
		esim_cycle, inst_count, ipc_global, ipc_interval);

	/* Schedule new event */
	stack->inst_count = ctx->inst_count;
	esim_schedule_event(event, stack, ctx->loader->ipc_report_interval);
}
