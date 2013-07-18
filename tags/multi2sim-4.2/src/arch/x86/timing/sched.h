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

#ifndef ARCH_X86_TIMING_SCHED_H
#define ARCH_X86_TIMING_SCHED_H


/*
 * Class 'X86Thread'
 * Additional functions
 */

/* Unmap a context from a node, i.e., remove it from the list of contexts mapped
 * to the node. A context is unmapped from a node either because it lost
 * affinity with the node, or because it finished execution.
 * A context must have been evicted from the node before being unmapped. */
void X86ThreadUnmapContext(X86Thread *self, X86Context *ctx);

/* Activate the 'evict_signal' flag of an allocated context and start
 * flushing the thread's pipeline. Once the last instruction reaches the
 * commit stage, the context will be effectively evicted. */
void X86ThreadEvictContextSignal(X86Thread *self, X86Context *context);

/* Evict a context allocated to a thread. The context must be currectly
 * allocated to the thread. */
void X86ThreadEvictContext(X86Thread *self, X86Context *context);

/* Scheduling actions for all contexts currently mapped to a thread. */
void X86ThreadSchedule(X86Thread *self);




/*
 * Class 'X86Cpu'
 * Additional functions
 */

/* Allocate (effectively start running) a context that is already mapped to a
 * node (core/thread). */
void X86CpuAllocateContext(X86Cpu *self, X86Context *ctx);

/* Map a context to a thread. The thread is chosen with the minimum number of
 * contexts currently mapped to it. */
void X86CpuMapContext(X86Cpu *self, X86Context *ctx);

/* Recalculate the oldest allocation cycle from all allocated contexts
 * (i.e., contexts currently occupying the nodes' pipelines). Discard
 * from the calculation those contexts that have received an eviction
 * signal (pipelines are being flushed for impending eviction.
 * By looking at this variable later, we can know right away whether
 * there is any allocated context that has exceeded its quantum. */
void X86CpuUpdateMinAllocCycle(X86Cpu *self);

/* Main scheduler function. This function should be called every timing
 * simulation cycle. If no scheduling is required, the function will exit with
 * practically no cost. */
void X86CpuSchedule(X86Cpu *self);


#endif

