/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>


int p_pipeline_empty(int core, int thread)
{
	return !THREAD.rob_count && !list_count(THREAD.fetchq) &&
		!list_count(THREAD.uopq);
}


/* Return the cpu identifier that best fits to the context with the following priority:
 *  1) If the cpu where the context was allocated before is free, return it.
 *  2) If there is any cpu that has not been used yet, return it.
 *  3) If there is any free cpu, return it.
 *  4) Return -1
 */
int p_context_to_cpu(struct ctx_t *ctx)
{
	int cpu, free_cpu;
	int core, thread;
	assert(!ctx_get_status(ctx, ctx_alloc));
	assert(ke->alloc_count <= p_cpus);

	/* No free cpu */
	if (ke->alloc_count == p_cpus)
		return -1;
	
	/* Try to allocate previous cpu, if the contexts has ever been
	 * allocated before. */
	if (ctx->alloc_when && !p->core[ctx->alloc_core].thread[ctx->alloc_thread].ctx)
		return ctx->alloc_core * p_threads + ctx->alloc_thread;
	
	/* Find a cpu that has not been used before. This is useful in case
	 * a context was suspended and tries to allocate later the same cpu. */
	free_cpu = -1;
	for (cpu = 0; cpu < p_cpus; cpu++) {
		core = cpu / p_threads;
		thread = cpu % p_threads;
		if (!THREAD.ctx && free_cpu < 0)
			free_cpu = cpu;
		if (!THREAD.last_alloc_pid)
			return cpu;
	}
	assert(free_cpu >= 0);
	return free_cpu;
}


void p_map_context(int core, int thread, struct ctx_t *ctx)
{
	assert(!THREAD.ctx);
	assert(!ctx_get_status(ctx, ctx_alloc));
	assert(ke->alloc_count < p_cpus);
	assert(!ctx->dealloc_signal);

	THREAD.ctx = ctx;
	THREAD.last_alloc_pid = ctx->pid;
	THREAD.fetch_neip = ctx->regs->eip;

	ctx_set_status(ctx, ctx_alloc);
	ctx->alloc_core = core;
	ctx->alloc_thread = thread;
	ctx->alloc_when = sim_cycle;

	ctx_debug("cycle %lld: ctx %d allocated to c%dt%d\n",
		(long long) sim_cycle, ctx->pid, core, thread);
}


void p_unmap_context(int core, int thread)
{
	struct ctx_t *ctx = THREAD.ctx;

	assert(ctx);
	assert(ctx_get_status(ctx, ctx_alloc));
	assert(!ctx_get_status(ctx, ctx_specmode));
	assert(!THREAD.rob_count);
	assert(ctx->dealloc_signal);
	assert(p->ctx_dealloc_signals > 0);

	THREAD.ctx = NULL;
	THREAD.fetch_neip = 0;

	ctx_clear_status(ctx, ctx_alloc);
	ctx->dealloc_when = sim_cycle;
	ctx->dealloc_signal = 0;
	p->ctx_dealloc_signals--;

	ctx_debug("cycle %lld: ctx %d evicted from c%dt%d\n",
		(long long) sim_cycle, ctx->pid, core, thread);
	
	/* If context is finished, free it. */
	if (ctx_get_status(ctx, ctx_finished))
		ctx_free(ctx);
}


/* Activate the 'dealloc_signal' flag of a context. The context
 * will be deallocated in the commit stage as soon as the pipeline
 * is empty. Also, no newer instructions will be enter the pipeline
 * hereafter. */
void p_unmap_context_signal(struct ctx_t *ctx)
{
	int core, thread;

	assert(ctx);
	assert(ctx_get_status(ctx, ctx_alloc));
	assert(!ctx->dealloc_signal);
	assert(p->ctx_dealloc_signals < p_cpus);

	ctx->dealloc_signal = 1;
	p->ctx_dealloc_signals++;
	core = ctx->alloc_core;
	thread = ctx->alloc_thread;
	ctx_debug("cycle %lld: ctx %d receives eviction signal from c%dt%d\n",
		(long long) sim_cycle, ctx->pid, core, thread);
	if (p_pipeline_empty(core, thread))
		p_unmap_context(core, thread);
		
}


void p_static_schedule()
{
	struct ctx_t *ctx;
	int cpu;

	ctx_debug("cycle %lld: static scheduler called\n",
		(long long) sim_cycle);
	
	/* If there is no new unallocated context, exit. */
	assert(ke->alloc_count <= ke->context_count);
	if (ke->alloc_count == ke->context_count)
		return;
	
	/* Allocate all unallocated contexts. */
	for (ctx = ke->context_list_head; ctx; ctx = ctx->context_next) {
		
		/* Context is allocated. */
		if (ctx_get_status(ctx, ctx_alloc))
			continue;

		/* Find free cpu. If none free, static scheduler aborts
		 * simulation with an error. */
		cpu = p_context_to_cpu(ctx);
		if (cpu < 0)
			fatal("no cpu free for context %d; increase number of cores/threads"
				" or activate the context scheduler.", ctx->pid);

		/* Allocate context. */
		p_map_context(cpu / p_threads, cpu % p_threads, ctx);
	}
}


void p_dynamic_schedule()
{
	struct ctx_t *ctx, *found_ctx;
	int cpu;

	ctx_debug("cycle %lld: scheduler called\n",
		(long long) sim_cycle);
	
	/* Evict non-running contexts */
	for (ctx = ke->alloc_list_head; ctx; ctx = ctx->alloc_next)
		if (!ctx->dealloc_signal && !ctx_get_status(ctx, ctx_running))
			p_unmap_context_signal(ctx);

	/* If all running contexts are allocated, just update the ctx_alloc_oldest counter,
	 * and exit. */
	if (ke->alloc_count == ke->running_count) {
		p->ctx_alloc_oldest = sim_cycle;
		for (ctx = ke->alloc_list_head; ctx; ctx = ctx->alloc_next)
			ctx->alloc_when = sim_cycle;
		return;
	}

	/* If any quantum expired and no context eviction signal is activated,
	 * send signal to evict the oldest allocated context. */
	if (!p->ctx_dealloc_signals && p->ctx_alloc_oldest + p_context_quantum <= sim_cycle) {
		found_ctx = NULL;
		for (ctx = ke->alloc_list_head; ctx; ctx = ctx->alloc_next)
			if (!found_ctx || ctx->alloc_when < found_ctx->alloc_when)
				found_ctx = ctx;
		if (found_ctx)
			p_unmap_context_signal(found_ctx);
	}
	
	/* Allocate running contexts */
	while (ke->alloc_count < ke->running_count && ke->alloc_count < p_cpus) {
		
		/* Find running, non-allocated context with lowest dealloc_when value. */
		found_ctx = NULL;
		for (ctx = ke->running_list_head; ctx; ctx = ctx->running_next)
			if (!ctx_get_status(ctx, ctx_alloc) && (!found_ctx || ctx->dealloc_when < found_ctx->dealloc_when))
				found_ctx = ctx;
		if (!found_ctx)
			break;
		ctx = found_ctx;

		/* Allocate context */
		cpu = p_context_to_cpu(ctx);
		assert(cpu >= 0 && cpu < p_cpus);
		p_map_context(cpu / p_threads, cpu % p_threads, ctx);
	}

	/* Calculate the context that was allocated first */
	p->ctx_alloc_oldest = sim_cycle;
	for (ctx = ke->alloc_list_head; ctx; ctx = ctx->alloc_next)
		if (!ctx->dealloc_signal && ctx->alloc_when < p->ctx_alloc_oldest)
			p->ctx_alloc_oldest = ctx->alloc_when;
}

