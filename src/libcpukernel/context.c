/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpukernel.h>


int ctx_debug_category;

int EV_CTX_IPC_REPORT;

static char *help_ctx_ipc_report =
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


static struct string_map_t ctx_status_map =
{
	16, {
		{ "running",      ctx_running },
		{ "specmode",     ctx_specmode },
		{ "suspended",    ctx_suspended },
		{ "finished",     ctx_finished },
		{ "exclusive",    ctx_exclusive },
		{ "locked",       ctx_locked },
		{ "handler",      ctx_handler },
		{ "sigsuspend",   ctx_sigsuspend },
		{ "nanosleep",    ctx_nanosleep },
		{ "poll",         ctx_poll },
		{ "read",         ctx_read },
		{ "write",        ctx_write },
		{ "waitpid",      ctx_waitpid },
		{ "zombie",       ctx_zombie },
		{ "futex",        ctx_futex },
		{ "alloc",        ctx_alloc }
	}
};


static struct ctx_t *ctx_do_create()
{
	struct ctx_t *ctx;

	/* Create context and set its status */
	ctx = calloc(1, sizeof(struct ctx_t));
	ctx->pid = ke->current_pid++;

	/* Update status so that the context is inserted in the
	 * corresponding lists. The ctx_running parameter has no
	 * effect, since it will be updated later. */
	ctx_set_status(ctx, ctx_running);
	ke_list_insert_head(ke_list_context, ctx);

	/* Structures */
	ctx->regs = regs_create();
	ctx->backup_regs = regs_create();
	ctx->signal_mask_table = signal_mask_table_create();

	/* Return context */
	return ctx;
}


struct ctx_t *ctx_create(void)
{
	struct ctx_t *ctx;
	
	ctx = ctx_do_create();

	/* Loader */
	ctx->loader = ld_create();

	/* Memory */
	ctx->mid = ke->current_mid++;
	ctx->mem = mem_create();
	ctx->spec_mem = spec_mem_create(ctx->mem);

	/* Signal handlers and file descriptor table */
	ctx->signal_handler_table = signal_handler_table_create();
	ctx->fdt = fdt_create();
	
	return ctx;
}


struct ctx_t *ctx_clone(struct ctx_t *ctx)
{
        struct ctx_t *new;

	new = ctx_do_create();

	/* Register file contexts are copied from parent. */
	regs_copy(new->regs, ctx->regs);

	/* The memory image of the cloned context if the same.
	 * The memory structure must be only freed by the parent
	 * when all its children have been killed.
	 * The set of signal handlers is the same, too. */
	new->mid = ctx->mid;
	new->mem = mem_link(ctx->mem);
	new->spec_mem = spec_mem_create(new->mem);

	/* Loader */
	new->loader = ld_link(ctx->loader);

	/* Signal handlers and file descriptor table */
	new->signal_handler_table = signal_handler_table_link(ctx->signal_handler_table);
	new->fdt = fdt_link(ctx->fdt);

	/* Libc segment */
	new->glibc_segment_base = ctx->glibc_segment_base;
	new->glibc_segment_limit = ctx->glibc_segment_limit;

	/* Update other fields. */
	new->parent = ctx;

	/* Return new context */
	return new;

}


struct ctx_t *ctx_fork(struct ctx_t *ctx)
{
	struct ctx_t *new;

	new = ctx_do_create();

	/* Copy registers */
	regs_copy(new->regs, ctx->regs);

	/* Memory */
	new->mid = ke->current_mid++;
	new->mem = mem_create();
	new->spec_mem = spec_mem_create(new->mem);
	mem_clone(new->mem, ctx->mem);

	/* Loader */
	new->loader = ld_link(ctx->loader);

	/* Signal handlers and file descriptor table */
	new->signal_handler_table = signal_handler_table_create();
	new->fdt = fdt_create();

	/* Libc segment */
	new->glibc_segment_base = ctx->glibc_segment_base;
	new->glibc_segment_limit = ctx->glibc_segment_limit;

	/* Set parent */
	new->parent = ctx;

	/* Return new context */
	return new;
}


/* Free a context */
void ctx_free(struct ctx_t *ctx)
{
	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!ctx_get_status(ctx, ctx_finished | ctx_zombie))
		ctx_finish(ctx, 0);
	
	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!ke_list_member(ke_list_running, ctx));
	assert(!ke_list_member(ke_list_suspended, ctx));
	assert(!ke_list_member(ke_list_zombie, ctx));
	assert(ke_list_member(ke_list_finished, ctx));
	ke_list_remove(ke_list_finished, ctx);
		
	/* Free private structures */
	regs_free(ctx->regs);
	regs_free(ctx->backup_regs);
	signal_mask_table_free(ctx->signal_mask_table);
	spec_mem_free(ctx->spec_mem);

	/* Unlink shared structures */
	ld_unlink(ctx->loader);
	signal_handler_table_unlink(ctx->signal_handler_table);
	fdt_unlink(ctx->fdt);
	mem_unlink(ctx->mem);

	/* Warn about unresolved attempts to access OpenCL library */
	if (ctx->libopencl_open_attempt)
		gk_libopencl_failed(ctx->pid);

	/* Remove context from contexts list and free */
	ke_list_remove(ke_list_context, ctx);
	if (isa_ctx == ctx)
		isa_ctx = NULL;
	ctx_debug("context %d freed\n", ctx->pid);
	free(ctx);
}


void ctx_dump(struct ctx_t *ctx, FILE *f)
{
	char sstatus[200];
	fprintf(f, "  pid=%d\n", ctx->pid);
	map_flags(&ctx_status_map, ctx->status, sstatus, 200);
	fprintf(f, "  status=%s\n", sstatus);
	if (!ctx->parent)
		fprintf(f, "  parent=(null)\n");
	else
		fprintf(f, "  parent=%d\n", ctx->parent->pid);
	fprintf(f, "  heap break: 0x%x\n", ctx->mem->heap_break);

	/* Signal masks */
	fprintf(f, "  blocked signal mask: 0x%llx ",
		(long long) ctx->signal_mask_table->blocked);
	sim_sigset_dump(ctx->signal_mask_table->blocked, f);
	fprintf(f, "\n  pending signals: 0x%llx ",
		(long long) ctx->signal_mask_table->pending);
	sim_sigset_dump(ctx->signal_mask_table->pending, f);
	fprintf(f, "\n");
}


void ctx_execute_inst(struct ctx_t *ctx)
{
	unsigned char buffer[20];
	unsigned char *buffer_ptr;

	/* The isa_xxx functions work on these global
	 * variables. */
	isa_ctx = ctx;
	isa_regs = ctx->regs;
	isa_mem = ctx->mem;
	isa_eip = isa_regs->eip;
	isa_spec_mode = ctx_get_status(isa_ctx, ctx_specmode);
	isa_inst_count++;
	if (!isa_spec_mode)
		ctx->inst_count++;

	/* Read instruction from memory. Memory should be accessed here in unsafe mode
	 * (i.e., allowing segmentation faults) if executing speculatively. */
	isa_mem->safe = isa_spec_mode ? 0 : mem_safe_mode;
	buffer_ptr = mem_get_buffer(isa_mem, isa_regs->eip, 20, mem_access_exec);
	if (!buffer_ptr)
	{
		/* Disable safe mode. If a part of the 20 read bytes does not belong to the
		 * actual instruction, and they lie on a page with no permissions, this would
		 * generate an undesired protection fault. */
		isa_mem->safe = 0;
		buffer_ptr = buffer;
		mem_access(isa_mem, isa_regs->eip, 20, buffer_ptr, mem_access_exec);
	}
	isa_mem->safe = mem_safe_mode;

	/* Disassemble */
	x86_disasm(buffer_ptr, isa_eip, &isa_inst);
	if (isa_inst.opcode == x86_op_none && !isa_spec_mode)
		fatal("0x%x: not supported x86 instruction (%02x %02x %02x %02x...)",
			isa_eip, buffer_ptr[0], buffer_ptr[1], buffer_ptr[2], buffer_ptr[3]);


	/* Execute instruction */
	isa_execute_inst();
	
	/* Stats */
	ke->inst_count++;
}


/* Force a new 'eip' value for the context. The forced value should be the same as
 * the current 'eip' under normal circumstances. If it is not, speculative execution
 * starts, which will end on the next call to 'ctx_recover'. */
void ctx_set_eip(struct ctx_t *ctx, uint32_t eip)
{
	/* Entering specmode */
	if (ctx->regs->eip != eip && !ctx_get_status(ctx, ctx_specmode))
	{
		ctx_set_status(ctx, ctx_specmode);
		regs_copy(ctx->backup_regs, ctx->regs);
		ctx->regs->fpu_ctrl |= 0x3f; /* mask all FP exceptions on wrong path */
	}
	
	/* Set it */
	ctx->regs->eip = eip;
}


void ctx_recover(struct ctx_t *ctx)
{
	assert(ctx_get_status(ctx, ctx_specmode));
	ctx_clear_status(ctx, ctx_specmode);
	regs_copy(ctx->regs, ctx->backup_regs);
	spec_mem_clear(ctx->spec_mem);
}


int ctx_get_status(struct ctx_t *ctx, enum ctx_status_t status)
{
	return (ctx->status & status) > 0;
}


static void ctx_update_status(struct ctx_t *ctx, enum ctx_status_t status)
{
	enum ctx_status_t status_diff;

	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if (ke_list_member(ke_list_running, ctx))
		ke_list_remove(ke_list_running, ctx);
	if (ke_list_member(ke_list_suspended, ctx))
		ke_list_remove(ke_list_suspended, ctx);
	if (ke_list_member(ke_list_zombie, ctx))
		ke_list_remove(ke_list_zombie, ctx);
	if (ke_list_member(ke_list_finished, ctx))
		ke_list_remove(ke_list_finished, ctx);
	if (ke_list_member(ke_list_alloc, ctx))
		ke_list_remove(ke_list_alloc, ctx);
	
	/* If the difference between the old and new status lies in other
	 * states other than 'ctx_specmode', a reschedule is marked. */
	status_diff = ctx->status ^ status;
	if (status_diff & ~ctx_specmode)
		ke->context_reschedule = 1;
	
	/* Update status */
	ctx->status = status;
	if (ctx->status & ctx_finished)
		ctx->status = ctx_finished | (status & ctx_alloc);
	if (ctx->status & ctx_zombie)
		ctx->status = ctx_zombie | (status & ctx_alloc);
	if (!(ctx->status & ctx_suspended) &&
		!(ctx->status & ctx_finished) &&
		!(ctx->status & ctx_zombie) &&
		!(ctx->status & ctx_locked))
		ctx->status |= ctx_running;
	else
		ctx->status &= ~ctx_running;
	
	/* Insert context into the corresponding lists. */
	if (ctx->status & ctx_running)
		ke_list_insert_head(ke_list_running, ctx);
	if (ctx->status & ctx_zombie)
		ke_list_insert_head(ke_list_zombie, ctx);
	if (ctx->status & ctx_finished)
		ke_list_insert_head(ke_list_finished, ctx);
	if (ctx->status & ctx_suspended)
		ke_list_insert_head(ke_list_suspended, ctx);
	if (ctx->status & ctx_alloc)
		ke_list_insert_head(ke_list_alloc, ctx);
	
	/* Dump new status (ignore 'ctx_specmode' status, it's too frequent) */
	if (debug_status(ctx_debug_category) && (status_diff & ~ctx_specmode))
	{
		char sstatus[200];
		map_flags(&ctx_status_map, ctx->status, sstatus, 200);
		ctx_debug("ctx %d changed status to %s\n",
			ctx->pid, sstatus);
	}
}


void ctx_set_status(struct ctx_t *ctx, enum ctx_status_t status)
{
	ctx_update_status(ctx, ctx->status | status);
}


void ctx_clear_status(struct ctx_t *ctx, enum ctx_status_t status)
{
	ctx_update_status(ctx, ctx->status & ~status);
}


/* Look for a context matching pid in the list of existing
 * contexts of the kernel. */
struct ctx_t *ctx_get(int pid)
{
	struct ctx_t *ctx;

	ctx = ke->context_list_head;
	while (ctx && ctx->pid != pid)
		ctx = ctx->context_list_next;
	return ctx;
}


/* Look for zombie child. If 'pid' is -1, the first finished child
 * in the zombie contexts list is return. Otherwise, 'pid' is the
 * pid of the child process. If no child has finished, return NULL. */
struct ctx_t *ctx_get_zombie(struct ctx_t *parent, int pid)
{
	struct ctx_t *ctx;

	for (ctx = ke->zombie_list_head; ctx; ctx = ctx->zombie_list_next)
	{
		if (ctx->parent != parent)
			continue;
		if (ctx->pid == pid || pid == -1)
			return ctx;
	}
	return NULL;
}


/* If the context is running a 'ke_host_thread_suspend' thread,
 * cancel it and schedule call to 'ke_process_events' */

void __ctx_host_thread_suspend_cancel(struct ctx_t *ctx)
{
	if (ctx->host_thread_suspend_active)
	{
		if (pthread_cancel(ctx->host_thread_suspend))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, ctx->pid);
		ctx->host_thread_suspend_active = 0;
		ke->process_events_force = 1;
	}
}

void ctx_host_thread_suspend_cancel(struct ctx_t *ctx)
{
	pthread_mutex_lock(&ke->process_events_mutex);
	__ctx_host_thread_suspend_cancel(ctx);
	pthread_mutex_unlock(&ke->process_events_mutex);
}


/* If the context is running a 'ke_host_thread_timer' thread,
 * cancel it and schedule call to 'ke_process_events' */

void __ctx_host_thread_timer_cancel(struct ctx_t *ctx)
{
	if (ctx->host_thread_timer_active)
	{
		if (pthread_cancel(ctx->host_thread_timer))
			fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, ctx->pid);
		ctx->host_thread_timer_active = 0;
		ke->process_events_force = 1;
	}
}

void ctx_host_thread_timer_cancel(struct ctx_t *ctx)
{
	pthread_mutex_lock(&ke->process_events_mutex);
	__ctx_host_thread_timer_cancel(ctx);
	pthread_mutex_unlock(&ke->process_events_mutex);
}


/* Finish a context group. This call does a subset of action of the 'ctx_finish'
 * call, but for all parent and child contexts sharing a memory map. */
void ctx_finish_group(struct ctx_t *ctx, int status)
{
	struct ctx_t *aux;

	/* Get group parent */
	if (ctx->group_parent)
		ctx = ctx->group_parent;
	assert(!ctx->group_parent);  /* Only one level */
	
	/* Context already finished */
	if (ctx_get_status(ctx, ctx_finished | ctx_zombie))
		return;

	/* Finish all contexts in the group */
	DOUBLE_LINKED_LIST_FOR_EACH(ke, context, aux)
	{
		if (aux->group_parent != ctx && aux != ctx)
			continue;

		if (ctx_get_status(aux, ctx_zombie))
			ctx_set_status(aux, ctx_finished);
		if (ctx_get_status(aux, ctx_handler))
			signal_handler_return(aux);
		ctx_host_thread_suspend_cancel(aux);
		ctx_host_thread_timer_cancel(aux);

		/* If context has a parent, set its status to zombie, and let the
		 * parent 'waitpid' it. */
		ctx_set_status(aux, aux->parent ? ctx_zombie : ctx_finished);
		aux->exit_code = status;
	}

	/* Process events */
	ke_process_events_schedule();
}


/* Finish a context. If the context has no parent, its status will be set
 * to 'ctx_finished'. If it has, its status is set to 'ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void ctx_finish(struct ctx_t *ctx, int status)
{
	struct ctx_t *aux;
	
	/* Context already finished */
	if (ctx_get_status(ctx, ctx_finished | ctx_zombie))
		return;
	
	/* If context is waiting for host events, cancel spawned host threads. */
	ctx_host_thread_suspend_cancel(ctx);
	ctx_host_thread_timer_cancel(ctx);

	/* From now on, all children have lost their parent. If a child is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore. */
	DOUBLE_LINKED_LIST_FOR_EACH(ke, context, aux)
	{
		if (aux->parent == ctx)
		{
			aux->parent = NULL;
			if (ctx_get_status(aux, ctx_zombie))
				ctx_set_status(aux, ctx_finished);
		}
	}

	/* Send finish signal to parent */
	if (ctx->exit_signal && ctx->parent)
	{
		syscall_debug("  sending signal %d to pid %d\n",
			ctx->exit_signal, ctx->parent->pid);
		sim_sigset_add(&ctx->parent->signal_mask_table->pending,
			ctx->exit_signal);
		ke_process_events_schedule();
	}

	/* If clear_child_tid was set, a futex() call must be performed on
	 * that pointer. Also wake up futexes in the robust list. */
	if (ctx->clear_child_tid)
	{
		uint32_t zero = 0;
		mem_write(ctx->mem, ctx->clear_child_tid, 4, &zero);
		ctx_futex_wake(ctx, ctx->clear_child_tid, 1, -1);
	}
	ctx_exit_robust_list(ctx);

	/* If we are in a signal handler, stop it. */
	if (ctx_get_status(ctx, ctx_handler))
		signal_handler_return(ctx);

	/* Finish context */
	ctx_set_status(ctx, ctx->parent ? ctx_zombie : ctx_finished);
	ctx->exit_code = status;
	ke_process_events_schedule();
}


int ctx_futex_wake(struct ctx_t *ctx, uint32_t futex, uint32_t count, uint32_t bitset)
{
	int wakeup_count = 0;
	struct ctx_t *wakeup_ctx;

	/* Look for threads suspended in this futex */
	while (count)
	{
		wakeup_ctx = NULL;
		for (ctx = ke->suspended_list_head; ctx; ctx = ctx->suspended_list_next)
		{
			if (!ctx_get_status(ctx, ctx_futex) || ctx->wakeup_futex != futex)
				continue;
			if (!(ctx->wakeup_futex_bitset & bitset))
				continue;
			if (!wakeup_ctx || ctx->wakeup_futex_sleep < wakeup_ctx->wakeup_futex_sleep)
				wakeup_ctx = ctx;
		}

		if (wakeup_ctx)
		{
			ctx_clear_status(wakeup_ctx, ctx_suspended | ctx_futex);
			syscall_debug("  futex 0x%x: thread %d woken up\n", futex, wakeup_ctx->pid);
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


void ctx_exit_robust_list(struct ctx_t *ctx)
{
	uint32_t next, lock_entry, offset, lock_word;

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
	
	syscall_debug("ctx %d: processing robust futex list\n",
		ctx->pid);
	for (;;)
	{
		mem_read(ctx->mem, lock_entry, 4, &next);
		mem_read(ctx->mem, lock_entry + 4, 4, &offset);
		mem_read(ctx->mem, lock_entry + offset, 4, &lock_word);

		syscall_debug("  lock_entry=0x%x: offset=%d, lock_word=0x%x\n",
			lock_entry, offset, lock_word);

		/* Stop processing list if 'next' points to robust list */
		if (!next || next == ctx->robust_list_head)
			break;
		lock_entry = next;
	}
}


/* Generate virtual file '/proc/self/maps' and return it in 'path'. */
void ctx_gen_proc_self_maps(struct ctx_t *ctx, char *path)
{
	uint32_t start, end;
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

struct ctx_ipc_report_stack_t
{
	int pid;
	long long inst_count;
};

void ctx_ipc_report_schedule(struct ctx_t *ctx)
{
	struct ctx_ipc_report_stack_t *stack;
	FILE *f = ctx->loader->ipc_report_file;
	int i;

	/* Create new stack */
	stack = calloc(1, sizeof(struct ctx_ipc_report_stack_t));
	if (!stack)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	assert(ctx->loader->ipc_report_file);
	assert(ctx->loader->ipc_report_interval > 0);
	stack->pid = ctx->pid;

	/* Print header */
	fprintf(f, "%s", help_ctx_ipc_report);
	fprintf(f, "%10s %8s %10s %10s\n", "cycle", "inst", "ipc-glob", "ipc-int");
	for (i = 0; i < 43; i++)
		fprintf(f, "-");
	fprintf(f, "\n");

	/* Schedule first event */
	esim_schedule_event(EV_CTX_IPC_REPORT, stack,
		ctx->loader->ipc_report_interval);
}

void ctx_ipc_report_handler(int event, void *data)
{
	struct ctx_ipc_report_stack_t *stack = data;
	struct ctx_t *ctx;

	long long inst_count;
	double ipc_interval;
	double ipc_global;

	/* Get context. If it does not exist anymore, no more
	 * events to schedule. */
	ctx = ctx_get(stack->pid);
	if (!ctx || ctx_get_status(ctx, ctx_finished) || ke_sim_finish)
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
