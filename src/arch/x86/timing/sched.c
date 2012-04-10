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

#include <x86-timing.h>


int cpu_pipeline_empty(int core, int thread)
{
	return !THREAD.rob_count && !list_count(THREAD.fetchq) &&
		!list_count(THREAD.uopq);
}


/* Return the node identifier that best fits to the context with the following priority:
 *  1) If the node where the context was allocated before is free, return it.
 *  2) If there is any node that has not been used yet, return it.
 *  3) If there is any free node, return it.
 *  4) Return -1
 */
int cpu_context_to_cpu(struct x86_ctx_t *ctx)
{
	int node, free_cpu;
	int core, thread;
	assert(!x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(x86_emu->alloc_list_count <= cpu_cores * cpu_threads);

	/* No free node */
	if (x86_emu->alloc_list_count == cpu_cores * cpu_threads)
		return -1;
	
	/* Try to allocate previous node, if the contexts has ever been
	 * allocated before. */
	if (ctx->alloc_when && !cpu->core[ctx->alloc_core].thread[ctx->alloc_thread].ctx)
		return ctx->alloc_core * cpu_threads + ctx->alloc_thread;
	
	/* Find a node that has not been used before. This is useful in case
	 * a context was suspended and tries to allocate later the same node. */
	free_cpu = -1;
	for (node = 0; node < cpu_cores * cpu_threads; node++)
	{
		core = node / cpu_threads;
		thread = node % cpu_threads;
		if (!THREAD.ctx && free_cpu < 0)
			free_cpu = node;
		if (!THREAD.last_alloc_pid)
			return node;
	}
	assert(free_cpu >= 0);
	return free_cpu;
}


void cpu_map_context(int core, int thread, struct x86_ctx_t *ctx)
{
	assert(!THREAD.ctx);
	assert(!x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(x86_emu->alloc_list_count < cpu_cores * cpu_threads);
	assert(!ctx->dealloc_signal);

	THREAD.ctx = ctx;
	THREAD.last_alloc_pid = ctx->pid;
	THREAD.fetch_neip = ctx->regs->eip;

	x86_ctx_set_status(ctx, x86_ctx_alloc);
	ctx->alloc_core = core;
	ctx->alloc_thread = thread;
	ctx->alloc_when = cpu->cycle;

	x86_ctx_debug("cycle %lld: ctx %d allocated to c%dt%d\n",
		cpu->cycle, ctx->pid, core, thread);
}


void cpu_unmap_context(int core, int thread)
{
	struct x86_ctx_t *ctx = THREAD.ctx;

	assert(ctx);
	assert(x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(!x86_ctx_get_status(ctx, x86_ctx_specmode));
	assert(!THREAD.rob_count);
	assert(ctx->dealloc_signal);
	assert(cpu->ctx_dealloc_signals > 0);

	THREAD.ctx = NULL;
	THREAD.fetch_neip = 0;

	x86_ctx_clear_status(ctx, x86_ctx_alloc);
	ctx->dealloc_when = cpu->cycle;
	ctx->dealloc_signal = 0;
	cpu->ctx_dealloc_signals--;

	x86_ctx_debug("cycle %lld: ctx %d evicted from c%dt%d\n",
		cpu->cycle, ctx->pid, core, thread);
	
	/* If context is finished, free it. */
	if (x86_ctx_get_status(ctx, x86_ctx_finished))
		x86_ctx_free(ctx);
}


/* Activate the 'dealloc_signal' flag of a context. The context
 * will be deallocated in the commit stage as soon as the pipeline
 * is empty. Also, no newer instructions will be enter the pipeline
 * hereafter. */
void cpu_unmap_context_signal(struct x86_ctx_t *ctx)
{
	int core, thread;

	assert(ctx);
	assert(x86_ctx_get_status(ctx, x86_ctx_alloc));
	assert(!ctx->dealloc_signal);
	assert(cpu->ctx_dealloc_signals < cpu_cores * cpu_threads);

	ctx->dealloc_signal = 1;
	cpu->ctx_dealloc_signals++;
	core = ctx->alloc_core;
	thread = ctx->alloc_thread;
	x86_ctx_debug("cycle %lld: ctx %d receives eviction signal from c%dt%d\n",
		cpu->cycle, ctx->pid, core, thread);
	if (cpu_pipeline_empty(core, thread))
		cpu_unmap_context(core, thread);
		
}


void cpu_static_schedule()
{
	struct x86_ctx_t *ctx;
	int node;

	x86_ctx_debug("cycle %lld: static scheduler called\n",
		cpu->cycle);
	
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
		node = cpu_context_to_cpu(ctx);
		if (node < 0)
			fatal("no core/thread free for context %d; increase number of cores/threads"
				" or activate the context scheduler.", ctx->pid);

		/* Allocate context. */
		cpu_map_context(node / cpu_threads, node % cpu_threads, ctx);
	}
}


void cpu_dynamic_schedule()
{
	struct x86_ctx_t *ctx, *found_ctx;
	int node;

	x86_ctx_debug("cycle %lld: scheduler called\n",
		cpu->cycle);
	
	/* Evict non-running contexts */
	for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
		if (!ctx->dealloc_signal && !x86_ctx_get_status(ctx, x86_ctx_running))
			cpu_unmap_context_signal(ctx);

	/* If all running contexts are allocated, just update the ctx_alloc_oldest counter,
	 * and exit. */
	if (x86_emu->alloc_list_count == x86_emu->running_list_count)
	{
		cpu->ctx_alloc_oldest = cpu->cycle;
		for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
			ctx->alloc_when = cpu->cycle;
		return;
	}

	/* If any quantum expired and no context eviction signal is activated,
	 * send signal to evict the oldest allocated context. */
	if (!cpu->ctx_dealloc_signals && cpu->ctx_alloc_oldest + cpu_context_quantum <= cpu->cycle) {
		found_ctx = NULL;
		for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
			if (!found_ctx || ctx->alloc_when < found_ctx->alloc_when)
				found_ctx = ctx;
		if (found_ctx)
			cpu_unmap_context_signal(found_ctx);
	}
	
	/* Allocate running contexts */
	while (x86_emu->alloc_list_count < x86_emu->running_list_count && x86_emu->alloc_list_count < cpu_cores * cpu_threads)
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
		node = cpu_context_to_cpu(ctx);
		assert(node >= 0 && node < cpu_cores * cpu_threads);
		cpu_map_context(node / cpu_threads, node % cpu_threads, ctx);
	}

	/* Calculate the context that was allocated first */
	cpu->ctx_alloc_oldest = cpu->cycle;
	for (ctx = x86_emu->alloc_list_head; ctx; ctx = ctx->alloc_list_next)
		if (!ctx->dealloc_signal && ctx->alloc_when < cpu->ctx_alloc_oldest)
			cpu->ctx_alloc_oldest = ctx->alloc_when;
}

