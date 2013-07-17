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


#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "core.h"
#include "cpu.h"
#include "sched.h"
#include "thread.h"


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
 * in function 'X86CpuSchedule()'.
 *
 *   - Only start running if a schedule signal has been received (variable
 *     'emu->schedule_signal' is set, or one of the allocated contexts has
 *     exhaused its quantum. A schedule signal is triggered every time a
 *     context changes any of its state bits.
 *
 *   - Uncheck 'emu->schedule_signal' at this point to allow any of the
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
 * Class 'X86Thread'
 * Additional functions
 */

void X86ThreadUnmapContext(X86Thread *self, X86Context *ctx)
{
	X86Cpu *cpu = self->cpu;

	assert(ctx);
	assert(DOUBLE_LINKED_LIST_MEMBER(self, mapped, ctx));
	assert(X86ContextGetState(ctx, X86ContextMapped));
	assert(!X86ContextGetState(ctx, X86ContextAlloc));

	/* Update context state */
	X86ContextClearState(ctx, X86ContextMapped);

	/* Remove context from node's mapped list */
	DOUBLE_LINKED_LIST_REMOVE(self, mapped, ctx);

	/* Debug */
	X86ContextDebug("#%lld ctx %d unmapped from thread %s\n",
		asTiming(cpu)->cycle, ctx->pid, self->name);

	/* If context is finished, free it. */
	if (X86ContextGetState(ctx, X86ContextFinished))
	{
		/* Trace */
		x86_trace("x86.end_ctx ctx=%d\n", ctx->pid);

		/* Free context */
		delete(ctx);
	}
}


void X86ThreadEvictContextSignal(X86Thread *self, X86Context *context)
{
	assert(context);
	assert(self->ctx == context);
	assert(X86ContextGetState(context, X86ContextAlloc));
	assert(X86ContextGetState(context, X86ContextMapped));
	assert(self->ctx == context);
	assert(DOUBLE_LINKED_LIST_MEMBER(self, mapped, context));
	assert(!context->evict_signal);

	/* Set eviction signal. */
	context->evict_signal = 1;
	X86ContextDebug("#%lld ctx %d signaled for eviction from thread %s\n",
			asTiming(self)->cycle, context->pid, self->name);

	/* If pipeline is already empty for the thread, effective eviction can
	 * happen straight away. */
	if (X86ThreadIsPipelineEmpty(self))
		X86ThreadEvictContext(self, context);

}


void X86ThreadEvictContext(X86Thread *self, X86Context *context)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	assert(context);
	assert(self->ctx == context);
	assert(X86ContextGetState(context, X86ContextAlloc));
	assert(X86ContextGetState(context, X86ContextMapped));
	assert(!X86ContextGetState(context, X86ContextSpecMode));
	assert(!self->rob_count);
	assert(context->evict_signal);

	/* Update node state */
	self->ctx = NULL;
	self->fetch_neip = 0;

	/* Update context state */
	X86ContextClearState(context, X86ContextAlloc);
	context->evict_cycle = asTiming(cpu)->cycle;
	context->evict_signal = 0;

	/* Debug */
	X86ContextDebug("#%lld ctx %d evicted from thread %s\n",
			asTiming(cpu)->cycle, context->pid, self->name);

	/* Trace */
	x86_trace("x86.unmap_ctx ctx=%d core=%d thread=%d\n",
			context->pid, core->id, self->id_in_core);
}


void X86ThreadSchedule(X86Thread *self)
{
	X86Cpu *cpu = self->cpu;
	X86Context *ctx;
	X86Context *tmp_ctx;

	int node;

	/* Actions for the context allocated to this node. */
	node = self->id_in_cpu;
	ctx = self->ctx;
	if (ctx)
	{
		assert(X86ContextGetState(ctx, X86ContextAlloc));
		assert(X86ContextGetState(ctx, X86ContextMapped));

		/* Context not in 'running' state */
		if (!ctx->evict_signal && !X86ContextGetState(ctx, X86ContextRunning))
			X86ThreadEvictContextSignal(self, ctx);

		/* Context lost affinity with node */
		if (!ctx->evict_signal && !bit_map_get(ctx->affinity, node, 1))
			X86ThreadEvictContextSignal(self, ctx);

		/* Context quantum expired */
		if (!ctx->evict_signal && asTiming(cpu)->cycle >= ctx->alloc_cycle
				+ x86_cpu_context_quantum)
		{
			int found = 0;

			/* Find a running context mapped to the same node */
			DOUBLE_LINKED_LIST_FOR_EACH(self, mapped, tmp_ctx)
			{
				if (tmp_ctx != ctx && X86ContextGetState(tmp_ctx,
						X86ContextRunning))
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
				X86ThreadEvictContextSignal(self, ctx);
			else
				ctx->alloc_cycle = asTiming(cpu)->cycle;
		}
	}

	/* Actions for mapped contexts, other than the allocated context if any. */
	DOUBLE_LINKED_LIST_FOR_EACH(self, mapped, ctx)
	{
		/* Ignore the currently allocated context */
		if (ctx == self->ctx)
			continue;

		/* Unmap a context if it lost affinity with the node or if it
		 * finished execution. */
		if (!bit_map_get(ctx->affinity, node, 1) ||
				X86ContextGetState(ctx, X86ContextFinished))
			X86ThreadUnmapContext(self, ctx);
	}

	/* If node is available, try to allocate a context mapped to it. */
	if (!self->ctx)
	{
		/* Search the mapped context with the oldest 'evict_cycle'
		 * that is state 'running' and has affinity with the node. */
		ctx = NULL;
		DOUBLE_LINKED_LIST_FOR_EACH(self, mapped, tmp_ctx)
		{
			/* No affinity */
			if (!bit_map_get(tmp_ctx->affinity, node, 1))
				continue;

			/* Not running */
			if (!X86ContextGetState(tmp_ctx, X86ContextRunning))
				continue;

			/* Good candidate */
			if (!ctx || ctx->evict_cycle > tmp_ctx->evict_cycle)
				ctx = tmp_ctx;
		}

		/* Allocate context if found */
		if (ctx)
			X86CpuAllocateContext(cpu, ctx);
	}
}




/*
 * Class 'X86Cpu'
 * Additional functions
 */


void X86CpuAllocateContext(X86Cpu *self, X86Context *ctx)
{
	int core_index = ctx->core_index;
	int thread_index = ctx->thread_index;

	X86Core *core;
	X86Thread *thread;

	core = self->cores[core_index];
	thread = core->threads[thread_index];

	assert(!thread->ctx);
	assert(X86ContextGetState(ctx, X86ContextMapped));
	assert(!X86ContextGetState(ctx, X86ContextAlloc));
	assert(!ctx->evict_signal);

	/* Update context state */
	ctx->alloc_cycle = asTiming(self)->cycle;
	X86ContextSetState(ctx, X86ContextAlloc);

	/* Update node state */
	thread->ctx = ctx;
	thread->fetch_neip = ctx->regs->eip;

	/* Debug */
	X86ContextDebug("#%lld ctx %d in thread %s allocated\n",
		asTiming(self)->cycle, ctx->pid, thread->name);

	/* Trace */
	x86_trace("x86.map_ctx ctx=%d core=%d thread=%d ppid=%d\n",
		ctx->pid, core_index, thread_index,
		ctx->parent ? ctx->parent->pid : 0);
}


void X86CpuMapContext(X86Cpu *self, X86Context *ctx)
{
	int min_core;
	int min_thread;

	int core;
	int thread;
	int node;

	assert(!X86ContextGetState(ctx, X86ContextAlloc));
	assert(!X86ContextGetState(ctx, X86ContextMapped));
	assert(!ctx->evict_signal);

	/* From the nodes (core/thread) that the context has affinity with, find
	 * the one with the smalled number of contexts mapped. */
	min_core = -1;
	min_thread = -1;
	node = 0;
	for (core = 0; core < x86_cpu_num_cores; core++)
	{
		for (thread = 0; thread < x86_cpu_num_threads; thread++)
		{
			/* Context does not have affinity with this thread */
			node = core * x86_cpu_num_threads + thread;
			if (!bit_map_get(ctx->affinity, node, 1))
				continue;

			/* Check if this thread is better */
			if (min_core < 0 ||
					self->cores[core]->threads[thread]
						->mapped_list_count <
					self->cores[min_core]->threads[min_thread]
						->mapped_list_count)
			{
				min_core = core;
				min_thread = thread;
			}
		}
	}

	/* Final values */
	core = min_core;
	thread = min_thread;
	if (core < 0 || thread < 0)
		panic("%s: no node with affinity found", __FUNCTION__);

	/* Update context state */
	ctx->core_index = core;
	ctx->thread_index = thread;
	X86ContextSetState(ctx, X86ContextMapped);

	/* Add context to the node's mapped list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(self->cores[core]->threads[thread],
			mapped, ctx);

	/* Debug */
	X86ContextDebug("#%lld ctx %d mapped to node %d/%d\n",
		asTiming(self)->cycle, ctx->pid, core, thread);
}


void X86CpuUpdateMinAllocCycle(X86Cpu *self)
{
	X86Context *ctx;

	int i;
	int j;

	self->min_alloc_cycle = asTiming(self)->cycle;
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			ctx = self->cores[i]->threads[j]->ctx;
			if (ctx && !ctx->evict_signal &&
					ctx->alloc_cycle < self->min_alloc_cycle)
				self->min_alloc_cycle = ctx->alloc_cycle;
		}
	}
}


void X86CpuSchedule(X86Cpu *self)
{
	X86Emu *emu = self->emu;
	X86Context *ctx;

	int quantum_expired;

	int i;
	int j;

	/* Check if any context quantum could have expired */
	quantum_expired = asTiming(self)->cycle >= self->min_alloc_cycle +
			x86_cpu_context_quantum;

	/* Check for quick scheduler end. The only way to effectively execute
	 * the scheduler is that either a quantum expired or a signal to
	 * reschedule has been flagged. */
	if (!quantum_expired && !emu->schedule_signal)
		return;

	/* OK, we have to schedule. Uncheck the schedule signal here, since
	 * upcoming actions might set it again for a second scheduler call. */
	emu->schedule_signal = 0;
	X86ContextDebug("#%lld schedule\n", asTiming(self)->cycle);

	/* Check if there is any running context that is currently not mapped
	 * to any node (core/thread); for example, a new context, or a
	 * context that has changed its affinity. */
	DOUBLE_LINKED_LIST_FOR_EACH(emu, running, ctx)
		if (!X86ContextGetState(ctx, X86ContextMapped))
			X86CpuMapContext(self, ctx);

	/* Scheduling done individually for each node (core/thread) */
	for (i = 0; i < x86_cpu_num_cores; i++)
		for (j = 0; j < x86_cpu_num_threads; j++)
			X86ThreadSchedule(self->cores[i]->threads[j]);

	/* Update oldest allocation time of allocated contexts to determine
	 * when is the next time the scheduler should be invoked. */
	X86CpuUpdateMinAllocCycle(self);
}
