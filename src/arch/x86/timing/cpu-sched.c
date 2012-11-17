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

#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "cpu.h"


int x86_cpu_pipeline_empty(int core, int thread)
{
	return !X86_THREAD.rob_count && !list_count(X86_THREAD.fetch_queue) &&
		!list_count(X86_THREAD.uop_queue);
}


/* Return the node identifier that best fits to the context with the following priority:
 *  1) If the node where the context was allocated before is free, return it.
 *  2) If there is any node that has not been used yet, return it.
 *  3) If there is any free node, return it.
 *  4) Return -1
 */
static int x86_cpu_context_to_cpu(struct x86_ctx_t *ctx)
{
	int node;
	int free_cpu;

	int core;
	int thread;

	assert(!x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(x86_emu->alloc_list_count <= x86_cpu_num_cores * x86_cpu_num_threads);

	/* No free node */
	if (x86_emu->alloc_list_count == x86_cpu_num_cores * x86_cpu_num_threads)
		return -1;
	
	/* Try to allocate previous node, if the contexts has ever been
	 * allocated before. */
	if (ctx->alloc_when && !x86_cpu->core[ctx->alloc_core].thread[ctx->alloc_thread].ctx)
		return ctx->alloc_core * x86_cpu_num_threads + ctx->alloc_thread;
	
	/* Find a node that has not been used before. This is useful in case
	 * a context was suspended and tries to allocate later the same node. */
	free_cpu = -1;
	for (node = 0; node < x86_cpu_num_cores * x86_cpu_num_threads; node++)
	{
		core = node / x86_cpu_num_threads;
		thread = node % x86_cpu_num_threads;
		if (!X86_THREAD.ctx && free_cpu < 0)
			free_cpu = node;
		if (!X86_THREAD.last_alloc_pid)
			return node;
	}
	assert(free_cpu >= 0);
	return free_cpu;
}


void x86_cpu_map_context(int core, int thread, struct x86_ctx_t *ctx)
{
	assert(!X86_THREAD.ctx);
	assert(!x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(x86_emu->alloc_list_count < x86_cpu_num_cores * x86_cpu_num_threads);
	assert(!ctx->dealloc_signal);

	X86_THREAD.ctx = ctx;
	X86_THREAD.last_alloc_pid = ctx->pid;
	X86_THREAD.fetch_neip = ctx->regs->eip;

	x86_ctx_set_status(ctx, x86_ctx_alloc);
	ctx->alloc_core = core;
	ctx->alloc_thread = thread;
	ctx->alloc_when = x86_cpu->cycle;

	x86_ctx_debug("cycle %lld: ctx %d allocated to c%dt%d\n",
		x86_cpu->cycle, ctx->pid, core, thread);

	/* Trace */
	x86_trace("x86.map_ctx ctx=%d core=%d thread=%d ppid=%d\n",
		ctx->pid, core, thread, ctx->parent ? ctx->parent->pid : 0);
}


void x86_cpu_unmap_context(int core, int thread)
{
	struct x86_ctx_t *ctx = X86_THREAD.ctx;

	assert(ctx);
	assert(x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(!x86_ctx_get_status(ctx, x86_ctx_spec_mode));
	assert(!X86_THREAD.rob_count);
	assert(ctx->dealloc_signal);
	assert(x86_cpu->ctx_dealloc_signals > 0);

	X86_THREAD.ctx = NULL;
	X86_THREAD.fetch_neip = 0;

	x86_ctx_clear_status(ctx, x86_ctx_alloc);
	ctx->dealloc_when = x86_cpu->cycle;
	ctx->dealloc_signal = 0;
	x86_cpu->ctx_dealloc_signals--;

	x86_ctx_debug("cycle %lld: ctx %d evicted from c%dt%d\n",
		x86_cpu->cycle, ctx->pid, core, thread);
	
	/* Trace */
	x86_trace("x86.unmap_ctx ctx=%d core=%d thread=%d\n",
		ctx->pid, core, thread);

	/* If context is finished, free it. */
	if (x86_ctx_get_status(ctx, x86_ctx_finished))
	{
		/* Trace */
		x86_trace("x86.end_ctx ctx=%d\n", ctx->pid);

		/* Free context */
		x86_ctx_free(ctx);
	}
}


/* Activate the 'dealloc_signal' flag of a context. The context
 * will be deallocated in the commit stage as soon as the pipeline
 * is empty. Also, no newer instructions will be enter the pipeline
 * hereafter. */
void x86_cpu_unmap_context_signal(struct x86_ctx_t *ctx)
{
	int core, thread;

	assert(ctx);
	assert(x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(!ctx->dealloc_signal);
	assert(x86_cpu->ctx_dealloc_signals < x86_cpu_num_cores * x86_cpu_num_threads);

	ctx->dealloc_signal = 1;
	x86_cpu->ctx_dealloc_signals++;
	core = ctx->alloc_core;
	thread = ctx->alloc_thread;
	x86_ctx_debug("cycle %lld: ctx %d receives eviction signal from c%dt%d\n",
		x86_cpu->cycle, ctx->pid, core, thread);
	if (x86_cpu_pipeline_empty(core, thread))
		x86_cpu_unmap_context(core, thread);
		
}


void x86_cpu_static_schedule()
{
	struct x86_ctx_t *ctx;
	int node;

	x86_ctx_debug("cycle %lld: static scheduler called\n",
		x86_cpu->cycle);
	
	/* If there is no new unallocated context, exit. */
	assert(x86_emu->alloc_list_count <= x86_emu->context_list_count);
	if (x86_emu->alloc_list_count == x86_emu->context_list_count)
		return;
	
	/* Allocate all unallocated contexts. */
	for (ctx = x86_emu->context_list_head; ctx; ctx = ctx->context_list_next)
	{
		/* Context is allocated. */
		if (x86_ctx_get_status(ctx, x86_ctx_alloc))
			continue;

		/* Find free node. If none free, static scheduler aborts
		 * simulation with an error. */
		node = x86_cpu_context_to_cpu(ctx);
		if (node < 0)
			fatal("no core/thread free for context %d; increase number of cores/threads"
				" or activate the context scheduler.", ctx->pid);

		/* Allocate context. */
		x86_cpu_map_context(node / x86_cpu_num_threads, node % x86_cpu_num_threads, ctx);
	}
}


void x86_cpu_dynamic_schedule()
{
	struct x86_ctx_t *ctx, *found_ctx;
	int node;

	x86_ctx_debug("cycle %lld: scheduler called\n",
		x86_cpu->cycle);
	
	/* Evict non-running contexts */
	for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
		if (!ctx->dealloc_signal && !x86_ctx_get_status(ctx, x86_ctx_running))
			x86_cpu_unmap_context_signal(ctx);

	/* If all running contexts are allocated, just update the ctx_alloc_oldest counter,
	 * and exit. */
	if (x86_emu->alloc_list_count == x86_emu->running_list_count)
	{
		x86_cpu->ctx_alloc_oldest = x86_cpu->cycle;
		for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
			ctx->alloc_when = x86_cpu->cycle;
		return;
	}

	/* If any quantum expired and no context eviction signal is activated,
	 * send signal to evict the oldest allocated context. */
	if (!x86_cpu->ctx_dealloc_signals && x86_cpu->ctx_alloc_oldest + x86_cpu_context_quantum <= x86_cpu->cycle)
	{
		found_ctx = NULL;
		for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
			if (!found_ctx || ctx->alloc_when < found_ctx->alloc_when)
				found_ctx = ctx;
		if (found_ctx)
			x86_cpu_unmap_context_signal(found_ctx);
	}
	
	/* Allocate running contexts */
	while (x86_emu->alloc_list_count < x86_emu->running_list_count && x86_emu->alloc_list_count < x86_cpu_num_cores * x86_cpu_num_threads)
	{
		/* Find running, non-allocated context with lowest dealloc_when value. */
		found_ctx = NULL;
		for (ctx = x86_emu->running_list_head; ctx; ctx = ctx->running_list_next)
			if (!x86_ctx_get_status(ctx, x86_ctx_alloc) && (!found_ctx || ctx->dealloc_when < found_ctx->dealloc_when))
				found_ctx = ctx;
		if (!found_ctx)
			break;
		ctx = found_ctx;

		/* Allocate context */
		node = x86_cpu_context_to_cpu(ctx);
		assert(node >= 0 && node < x86_cpu_num_cores * x86_cpu_num_threads);
		x86_cpu_map_context(node / x86_cpu_num_threads, node % x86_cpu_num_threads, ctx);
	}

	/* Calculate the context that was allocated first */
	x86_cpu->ctx_alloc_oldest = x86_cpu->cycle;
	for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
		if (!ctx->dealloc_signal && ctx->alloc_when < x86_cpu->ctx_alloc_oldest)
			x86_cpu->ctx_alloc_oldest = ctx->alloc_when;
}
