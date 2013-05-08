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

#include <arch/common/arch.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/list.h>

#include "cpu.h"


/*
 * This file contains the implementation of the x86 context scheduler. The following
 * definitions are assumed in the description of the algorithm:
 *   - A node is a pair core/thread where a context can run.
 *   - Map a context to a node: associate a context with a node (core/thread). This
 *     association is done once when the context is created, or during context migration
 *     (e.g., when it changes its thread affinity bitmap). A node has a list of
 *     mapped contexts.
 *   - Unmap a context: remove association between a context and a node. The context
 *     is removed from the node's list of mapped contexts.
 *   - Allocate a context: select the context from the node's list of mapped context
 *     and start effectively executing it, allocating pipeline resources in the node.
 *   - Evict a context: Deallocate pipeline resources from the executing context, but
 *     still keep it in the node's list of mapped contexts.
 *
 * The following is a description of the implemented scheduling algorithm, implemented
 * in function 'x86_cpu_schedule()'.
 *
 *   - Only start running if a schedule signal has been received (variable
 *     'x86_emu->schedule_signal' is set, or one of the allocated contexts has
 *     exhaused its quantum. A schedule signal is triggered every time a
 *     context changes any of its state bits.
 *
 *   - Uncheck 'x86_emu->schedule_signal' at this point to allow any of the
 *     subsequent actions to force the scheduler to run in the next cycle.
 *
 *   - Check the list of running contexts for any unmapped context. Map them by
 *     selecting the node that has the lowest number of contexts currently mapped
 *     to it. The context will always execute on that node, unless it changes its
 *     affinity.
 *
 *   - For each node:
 *
 *         - If the allocated context is not in 'running' state, signal eviction.
 *
 *         - If the allocated context has exhausted its quantum, signal eviction.
 *           As an exception, the context will continue running if there is no
 *           other candidate in the mapped context list; in this case, the running
 *           context resets its quantum to keep the scheduler from trying to
 *           evict the context right away.
 *
 *         - If the allocated context lost affinity with the node, signal
 *           eviction.
 *
 *         - If any mapped context other than the allocated context lost
 *           affinity with the node, unmap it.
 *
 *         - If any mapped context other than the allocated context finished
 *           execution, unmap it and free it.
 *
 *         - If there is no allocated context, search the node's list of mapped
 *           contexts for the context in state running and with valid affinity
 *           that was evicted least recently. Allocate it, if found.
 *
 *   - Update global variable 'min_alloc_cycle' with the cycle of the least
 *     recently allocated context. The scheduler needs to be called again due
 *     to quantum expiration only when this variable indicates so.
 */



/*
 * Private Functions
 */


/* Allocate (effectively start running) a context that is already mapped to a
 * node (core/thread). */
static void x86_cpu_alloc_context(struct x86_ctx_t *ctx)
{
	int core = ctx->core;
	int thread = ctx->thread;

	assert(!X86_THREAD.ctx);
	assert(x86_ctx_get_state(ctx, x86_ctx_mapped));
	assert(!x86_ctx_get_state(ctx, x86_ctx_alloc));
	assert(!ctx->evict_signal);

	/* Update context state */
	ctx->alloc_cycle = arch_x86->cycle;
	x86_ctx_set_state(ctx, x86_ctx_alloc);

	/* Update node state */
	X86_THREAD.ctx = ctx;
	X86_THREAD.fetch_neip = ctx->regs->eip;

	/* Debug */
	x86_ctx_debug("#%lld ctx %d in node %d/%d allocated\n",
		arch_x86->cycle, ctx->pid, core, thread);

	/* Trace */
	x86_trace("x86.map_ctx ctx=%d core=%d thread=%d ppid=%d\n",
		ctx->pid, core, thread, ctx->parent ? ctx->parent->pid : 0);
}


/* Map a context to a node. The node is chosen with the minimum number of
 * contexts currently mapped to it. */
static void x86_cpu_map_context(struct x86_ctx_t *ctx)
{
	int min_core;
	int min_thread;

	int core;
	int thread;
	int node;

	assert(!x86_ctx_get_state(ctx, x86_ctx_alloc));
	assert(!x86_ctx_get_state(ctx, x86_ctx_mapped));
	assert(!ctx->evict_signal);

	/* From the nodes (core/thread) that the context has affinity with, find
	 * the one with the smalled number of contexts mapped. */
	min_core = -1;
	min_thread = -1;
	node = 0;
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		/* Context does not have affinity with this node */
		node = core * x86_cpu_num_threads + thread;
		if (!bit_map_get(ctx->affinity, node, 1))
			continue;

		/* Check if this node is better */
		if (min_core < 0 || X86_THREAD.mapped_list_count <
			X86_CORE_THREAD_IDX(min_core, min_thread).mapped_list_count)
		{
			min_core = core;
			min_thread = thread;
		}
	}

	/* Final values */
	core = min_core;
	thread = min_thread;
	if (core < 0 || thread < 0)
		panic("%s: no node with affinity found", __FUNCTION__);

	/* Update context state */
	ctx->core = core;
	ctx->thread = thread;
	x86_ctx_set_state(ctx, x86_ctx_mapped);

	/* Add context to the node's mapped list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(&X86_THREAD, mapped, ctx);

	/* Debug */
	x86_ctx_debug("#%lld ctx %d mapped to node %d/%d\n",
		arch_x86->cycle, ctx->pid, core, thread);
}


/* Unmap a context from a node, i.e., remove it from the list of contexts mapped
 * to the node. A context is unmapped from a node either because it lost
 * affinity with the node, or because it finished execution.
 * A context must have been evicted from the node before being unmapped. */
static void x86_cpu_unmap_context(struct x86_ctx_t *ctx)
{
	int core = ctx->core;
	int thread = ctx->thread;

	assert(DOUBLE_LINKED_LIST_MEMBER(&X86_THREAD, mapped, ctx));
	assert(x86_ctx_get_state(ctx, x86_ctx_mapped));
	assert(!x86_ctx_get_state(ctx, x86_ctx_alloc));

	/* Update context state */
	x86_ctx_clear_state(ctx, x86_ctx_mapped);

	/* Remove context from node's mapped list */
	DOUBLE_LINKED_LIST_REMOVE(&X86_THREAD, mapped, ctx);

	/* Debug */
	x86_ctx_debug("#%lld ctx %d unmapped from node %d/%d\n",
		arch_x86->cycle, ctx->pid, core, thread);

	/* If context is finished, free it. */
	if (x86_ctx_get_state(ctx, x86_ctx_finished))
	{
		/* Trace */
		x86_trace("x86.end_ctx ctx=%d\n", ctx->pid);

		/* Free context */
		x86_ctx_free(ctx);
	}
}


/* Recalculate the oldest allocation cycle from all allocated contexts
 * (i.e., contexts currently occupying the nodes' pipelines). Discard
 * from the calculation those contexts that have received an eviction
 * signal (pipelines are being flushed for impending eviction.
 * By looking at this variable later, we can know right away whether
 * there is any allocated context that has exceeded its quantum. */
static void x86_cpu_update_min_alloc_cycle(void)
{
	struct x86_ctx_t *ctx;

	int core;
	int thread;

	x86_cpu->min_alloc_cycle = arch_x86->cycle;
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		ctx = X86_THREAD.ctx;
		if (ctx && !ctx->evict_signal &&
				ctx->alloc_cycle < x86_cpu->min_alloc_cycle)
			x86_cpu->min_alloc_cycle = ctx->alloc_cycle;
	}
}


/* Activate the 'evict_signal' flag of an allocated context and start
 * flushing the node pipeline. Once the last instruction reaches the
 * commit stage, the context will be effectively evicted. */
static void x86_cpu_evict_context_signal(struct x86_ctx_t *ctx)
{
	int core = ctx->core;
	int thread = ctx->thread;

	assert(ctx);
	assert(x86_ctx_get_state(ctx, x86_ctx_alloc));
	assert(x86_ctx_get_state(ctx, x86_ctx_mapped));
	assert(X86_THREAD.ctx == ctx);
	assert(DOUBLE_LINKED_LIST_MEMBER(&X86_THREAD, mapped, ctx));
	assert(!ctx->evict_signal);

	/* Set eviction signal. */
	ctx->evict_signal = 1;
	x86_ctx_debug("#%lld ctx %d signaled for eviction from node %d/%d\n",
		arch_x86->cycle, ctx->pid, core, thread);

	/* If pipeline is already empty for the thread, effective eviction can
	 * happen straight away. */
	if (x86_cpu_pipeline_empty(core, thread))
		x86_cpu_evict_context(core, thread);

}


/* Scheduling actions for all contexts currently mapped to a node (core/thread). */
static void x86_cpu_schedule_node(int core, int thread)
{
	struct x86_ctx_t *ctx;
	struct x86_ctx_t *tmp_ctx;

	int node = core * x86_cpu_num_threads + thread;

	/* Actions for the context allocated to this node. */
	ctx = X86_THREAD.ctx;
	if (ctx)
	{
		assert(x86_ctx_get_state(ctx, x86_ctx_alloc));
		assert(x86_ctx_get_state(ctx, x86_ctx_mapped));

		/* Context not in 'running' state */
		if (!ctx->evict_signal && !x86_ctx_get_state(ctx, x86_ctx_running))
			x86_cpu_evict_context_signal(ctx);

		/* Context lost affinity with node */
		if (!ctx->evict_signal && !bit_map_get(ctx->affinity, node, 1))
			x86_cpu_evict_context_signal(ctx);

		/* Context quantum expired */
		if (!ctx->evict_signal && arch_x86->cycle >= ctx->alloc_cycle
				+ x86_cpu_context_quantum)
		{
			int found = 0;

			/* Find a running context mapped to the same node */
			DOUBLE_LINKED_LIST_FOR_EACH(&X86_THREAD, mapped, tmp_ctx)
			{
				if (tmp_ctx != ctx && x86_ctx_get_state(tmp_ctx,
						x86_ctx_running))
				{
					found = 1;
					break;
				}
			}

			/* If a context was found, there are other candidates
			 * for allocation in the node. We need to evict the
			 * current context. If there are no other running
			 * candidates, there is no need to evict. But we
			 * update the allocation time, so that the
			 * scheduler is not called constantly hereafter. */
			if (found)
				x86_cpu_evict_context_signal(ctx);
			else
				ctx->alloc_cycle = arch_x86->cycle;
		}
	}

	/* Actions for mapped contexts, other than the allocated context if any. */
	DOUBLE_LINKED_LIST_FOR_EACH(&X86_THREAD, mapped, ctx)
	{
		/* Ignore the currently allocated context */
		if (ctx == X86_THREAD.ctx)
			continue;

		/* Unmap a context if it lost affinity with the node or if it
		 * finished execution. */
		if (!bit_map_get(ctx->affinity, node, 1) ||
				x86_ctx_get_state(ctx, x86_ctx_finished))
			x86_cpu_unmap_context(ctx);
	}

	/* If node is available, try to allocate a context mapped to it. */
	if (!X86_THREAD.ctx)
	{
		/* Search the mapped context with the oldest 'evict_cycle'
		 * that is state 'running' and has affinity with the node. */
		ctx = NULL;
		DOUBLE_LINKED_LIST_FOR_EACH(&X86_THREAD, mapped, tmp_ctx)
		{
			/* No affinity */
			if (!bit_map_get(tmp_ctx->affinity, node, 1))
				continue;

			/* Not running */
			if (!x86_ctx_get_state(tmp_ctx, x86_ctx_running))
				continue;

			/* Good candidate */
			if (!ctx || ctx->evict_cycle > tmp_ctx->evict_cycle)
				ctx = tmp_ctx;
		}

		/* Allocate context if found */
		if (ctx)
			x86_cpu_alloc_context(ctx);
	}
}




/*
 * Public Functions
 */

int x86_cpu_pipeline_empty(int core, int thread)
{
	return !X86_THREAD.rob_count && !list_count(X86_THREAD.fetch_queue) &&
		!list_count(X86_THREAD.uop_queue);
}


void x86_cpu_evict_context(int core, int thread)
{
	struct x86_ctx_t *ctx = X86_THREAD.ctx;

	assert(ctx);
	assert(x86_ctx_get_state(ctx, x86_ctx_alloc));
	assert(x86_ctx_get_state(ctx, x86_ctx_mapped));
	assert(!x86_ctx_get_state(ctx, x86_ctx_spec_mode));
	assert(!X86_THREAD.rob_count);
	assert(ctx->evict_signal);

	/* Update node state */
	X86_THREAD.ctx = NULL;
	X86_THREAD.fetch_neip = 0;

	/* Update context state */
	x86_ctx_clear_state(ctx, x86_ctx_alloc);
	ctx->evict_cycle = arch_x86->cycle;
	ctx->evict_signal = 0;

	/* Debug */
	x86_ctx_debug("#%lld ctx %d evicted from node %d/%d\n",
		arch_x86->cycle, ctx->pid, core, thread);

	/* Trace */
	x86_trace("x86.unmap_ctx ctx=%d core=%d thread=%d\n",
		ctx->pid, core, thread);
}


void x86_cpu_schedule(void)
{
	struct x86_ctx_t *ctx;

	int quantum_expired;
	int core;
	int thread;

	/* Check if any context quantum could have expired */
	quantum_expired = arch_x86->cycle >= x86_cpu->min_alloc_cycle +
			x86_cpu_context_quantum;

	/* Check for quick scheduler end. The only way to effectively execute
	 * the scheduler is that either a quantum expired or a signal to
	 * reschedule has been flagged. */
	if (!quantum_expired && !x86_emu->schedule_signal)
		return;

	/* OK, we have to schedule. Uncheck the schedule signal here, since
	 * upcoming actions might set it again for a second scheduler call. */
	x86_emu->schedule_signal = 0;
	x86_ctx_debug("#%lld schedule\n", arch_x86->cycle);

	/* Check if there is any running context that is currently not mapped
	 * to any node (core/thread); for example, a new context, or a
	 * context that has changed its affinity. */
	DOUBLE_LINKED_LIST_FOR_EACH(x86_emu, running, ctx)
		if (!x86_ctx_get_state(ctx, x86_ctx_mapped))
			x86_cpu_map_context(ctx);

	/* Scheduling done individually for each node (core/thread) */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		x86_cpu_schedule_node(core, thread);

	/* Update oldest allocation time of allocated contexts to determine
	 * when is the next time the scheduler should be invoked. */
	x86_cpu_update_min_alloc_cycle();
}
