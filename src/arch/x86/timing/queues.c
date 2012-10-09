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

#include "timing.h"




/* Instruction Fetch Queue */

int x86_fetch_queue_size;


void x86_fetch_queue_init()
{
	int core, thread;
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		X86_THREAD.fetch_queue = list_create_with_size(x86_fetch_queue_size);
}


void x86_fetch_queue_done()
{
	int core, thread;
	struct list_t *fetchq;
	struct x86_uop_t *uop;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH {
		fetchq = X86_THREAD.fetch_queue;
		while (list_count(fetchq)) {
			uop = list_remove_at(fetchq, 0);
			uop->in_fetch_queue = 0;
			x86_uop_free_if_not_queued(uop);
		}
		list_free(fetchq);
	}
}


struct x86_uop_t *x86_fetch_queue_remove(int core, int thread, int index)
{
	struct list_t *fetchq = X86_THREAD.fetch_queue;
	struct x86_uop_t *uop;
	assert(index >= 0 && index < list_count(fetchq));
	uop = list_remove_at(fetchq, index);
	uop->in_fetch_queue = 0;
	if (!uop->fetch_trace_cache && !uop->mop_index)
	{
		X86_THREAD.fetchq_occ -= uop->mop_size;
		assert(X86_THREAD.fetchq_occ >= 0);
	}
	if (uop->fetch_trace_cache)
	{
		X86_THREAD.trace_cache_queue_occ--;
		assert(X86_THREAD.trace_cache_queue_occ >= 0);
	}
	if (!list_count(fetchq))
	{
		assert(!X86_THREAD.fetchq_occ);
		assert(!X86_THREAD.trace_cache_queue_occ);
	}
	return uop;
}


void x86_fetch_queue_recover(int core, int thread)
{
	struct list_t *fetchq = X86_THREAD.fetch_queue;
	struct x86_uop_t *uop;

	while (list_count(fetchq))
	{
		uop = list_get(fetchq, list_count(fetchq) - 1);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		uop = x86_fetch_queue_remove(core, thread, list_count(fetchq) - 1);

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"sq\"\n",
				uop->id_in_core, uop->core);
			x86_cpu_uop_trace_list_add(uop);
		}

		/* Free */
		x86_uop_free_if_not_queued(uop);
	}
}




/* Uop Queue */

int x86_uop_queue_size;


void x86_uop_queue_init()
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		X86_THREAD.uop_queue = list_create_with_size(x86_uop_queue_size);
}


void x86_uop_queue_done()
{
	int core, thread;
	struct list_t *uop_queue;
	struct x86_uop_t *uop;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		uop_queue = X86_THREAD.uop_queue;
		while (list_count(uop_queue))
		{
			uop = list_remove_at(uop_queue, 0);
			uop->in_uop_queue = 0;
			x86_uop_free_if_not_queued(uop);
		}
		list_free(uop_queue);
	}
}


void x86_uop_queue_recover(int core, int thread)
{
	struct list_t *uop_queue = X86_THREAD.uop_queue;
	struct x86_uop_t *uop;

	while (list_count(uop_queue))
	{
		uop = list_get(uop_queue, list_count(uop_queue) - 1);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		list_remove_at(uop_queue, list_count(uop_queue) - 1);
		uop->in_uop_queue = 0;

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"sq\"\n",
				uop->id_in_core, uop->core);
			x86_cpu_uop_trace_list_add(uop);
		}

		/* Free */
		x86_uop_free_if_not_queued(uop);
	}
}




/*
 * Instruction Queue
 */

char *x86_iq_kind_map[] = { "Shared", "Private" };
enum x86_iq_kind_t x86_iq_kind;
int x86_iq_size;


void x86_iq_init()
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		X86_THREAD.iq = linked_list_create();
}


void x86_iq_done()
{
	struct linked_list_t *iq;
	struct x86_uop_t *uop;

	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		iq = X86_THREAD.iq;
		linked_list_head(iq);
		while (linked_list_count(iq))
		{
			uop = linked_list_get(iq);
			uop->in_iq = 0;
			linked_list_remove(iq);
			x86_uop_free_if_not_queued(uop);
		}
		linked_list_free(iq);
	}
}


int x86_iq_can_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	size = x86_iq_kind == x86_iq_kind_private ? x86_iq_size : x86_iq_size * x86_cpu_num_threads;
	count = x86_iq_kind == x86_iq_kind_private ? X86_THREAD.iq_count : X86_CORE.iq_count;
	return count < size;
}


/* Insert a uop into the corresponding IQ. Since this is a non-FIFO queue,
 * the insertion position doesn't matter. */
void x86_iq_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct linked_list_t *iq = X86_THREAD.iq;

	assert(!uop->in_iq);
	linked_list_out(iq);
	linked_list_insert(iq, uop);
	uop->in_iq = 1;

	X86_CORE.iq_count++;
	X86_THREAD.iq_count++;
}


/* Remove the uop in the current position of the linked list representing
 * the IQ of the specified thread. */
void x86_iq_remove(int core, int thread)
{
	struct linked_list_t *iq = X86_THREAD.iq;
	struct x86_uop_t *uop;

	uop = linked_list_get(iq);
	assert(x86_uop_exists(uop));
	linked_list_remove(iq);
	uop->in_iq = 0;

	assert(X86_CORE.iq_count && X86_THREAD.iq_count);
	X86_CORE.iq_count--;
	X86_THREAD.iq_count--;
}


/* Remove all speculative uops from the current thread */
void x86_iq_recover(int core, int thread)
{
	struct linked_list_t *iq = X86_THREAD.iq;
	struct x86_uop_t *uop;

	linked_list_head(iq);
	while (!linked_list_is_end(iq))
	{
		uop = linked_list_get(iq);
		if (uop->specmode) {
			x86_iq_remove(core, thread);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(iq);
	}
}




/*
 * Load/Store Queue
 */

char *x86_lsq_kind_map[] = { "Shared", "Private" };
enum x86_lsq_kind_t x86_lsq_kind;
int x86_lsq_size;


void x86_lsq_init()
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		X86_THREAD.lq = linked_list_create();
		X86_THREAD.sq = linked_list_create();
	}
}


void x86_lsq_done()
{
	struct linked_list_t *lq, *sq;
	struct x86_uop_t *uop;

	int core;
	int thread;

	/* Load queue */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		lq = X86_THREAD.lq;
		linked_list_head(lq);
		while (linked_list_count(lq))
		{
			uop = linked_list_get(lq);
			uop->in_lq = 0;
			linked_list_remove(lq);
			x86_uop_free_if_not_queued(uop);
		}
		linked_list_free(lq);
	}

	/* Store queue */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		sq = X86_THREAD.sq;
		linked_list_head(sq);
		while (linked_list_count(sq))
		{
			uop = linked_list_get(sq);
			uop->in_sq = 0;
			linked_list_remove(sq);
			x86_uop_free_if_not_queued(uop);
		}
		linked_list_free(sq);
	}
}


int x86_lsq_can_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	int count, size;

	size = x86_lsq_kind == x86_lsq_kind_private ? x86_lsq_size : x86_lsq_size * x86_cpu_num_threads;
	count = x86_lsq_kind == x86_lsq_kind_private ? X86_THREAD.lsq_count : X86_CORE.lsq_count;
	return count < size;
}


/* Insert uop into corresponding load/store queue */
void x86_lsq_insert(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	struct linked_list_t *lq = X86_THREAD.lq;
	struct linked_list_t *sq = X86_THREAD.sq;

	assert(!uop->in_lq && !uop->in_sq);
	assert(uop->uinst->opcode == x86_uinst_load || uop->uinst->opcode == x86_uinst_store);
	if (uop->uinst->opcode == x86_uinst_load)
	{
		linked_list_out(lq);
		linked_list_insert(lq, uop);
		uop->in_lq = 1;
	}
	else
	{
		linked_list_out(sq);
		linked_list_insert(sq, uop);
		uop->in_sq = 1;
	}
	X86_CORE.lsq_count++;
	X86_THREAD.lsq_count++;
}


/* Remove all speculative uops from a given load/store queue in the
 * given thread. */
void x86_lsq_recover(int core, int thread)
{
	struct linked_list_t *lq = X86_THREAD.lq;
	struct linked_list_t *sq = X86_THREAD.sq;
	struct x86_uop_t *uop;

	/* Recover load queue */
	linked_list_head(lq);
	while (!linked_list_is_end(lq))
	{
		uop = linked_list_get(lq);
		if (uop->specmode)
		{
			x86_lq_remove(core, thread);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(lq);
	}

	/* Recover store queue */
	linked_list_head(sq);
	while (!linked_list_is_end(sq))
	{
		uop = linked_list_get(sq);
		if (uop->specmode)
		{
			x86_sq_remove(core, thread);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(sq);
	}
}


/* Remove the uop in the current position of the linked list representing
 * the load queue of the specified thread. */
void x86_lq_remove(int core, int thread)
{
	struct linked_list_t *lq = X86_THREAD.lq;
	struct x86_uop_t *uop;

	uop = linked_list_get(lq);
	assert(x86_uop_exists(uop));
	linked_list_remove(lq);
	uop->in_lq = 0;

	assert(X86_CORE.lsq_count && X86_THREAD.lsq_count);
	X86_CORE.lsq_count--;
	X86_THREAD.lsq_count--;
}


/* Remove an uop in the current position of the store queue */
void x86_sq_remove(int core, int thread)
{
	struct linked_list_t *sq = X86_THREAD.sq;
	struct x86_uop_t *uop;

	uop = linked_list_get(sq);
	assert(x86_uop_exists(uop));
	assert(uop->in_sq);
	linked_list_remove(sq);
	uop->in_sq = 0;

	assert(X86_CORE.lsq_count && X86_THREAD.lsq_count);
	X86_CORE.lsq_count--;
	X86_THREAD.lsq_count--;
}




/*
 * Event Queue
 */

void x86_event_queue_init()
{
	int core;

	X86_CORE_FOR_EACH
		X86_CORE.event_queue = linked_list_create();
}


void x86_event_queue_done()
{
	int core;

	X86_CORE_FOR_EACH
	{
		while (linked_list_count(X86_CORE.event_queue))
			x86_uop_free_if_not_queued(x86_event_queue_extract(X86_CORE.event_queue));
		linked_list_free(X86_CORE.event_queue);
	}
}


static int eventq_compare(const void *item1, const void *item2)
{
	const struct x86_uop_t *uop1 = item1;
	const struct x86_uop_t *uop2 = item2;
	return uop1->when != uop2->when ? uop1->when - uop2->when
		: uop1->id - uop2->id;
}


int x86_event_queue_long_latency(int core, int thread)
{
	struct linked_list_t *event_queue = X86_CORE.event_queue;
	struct x86_uop_t *uop;
	
	LINKED_LIST_FOR_EACH(event_queue)
	{
		uop = linked_list_get(event_queue);
		if (uop->thread != thread)
			continue;
		if (x86_cpu->cycle - uop->issue_when > 20)
			return 1;
	}
	return 0;
}


int x86_event_queue_cache_miss(int core, int thread)
{
	struct linked_list_t *event_queue = X86_CORE.event_queue;
	struct x86_uop_t *uop;

	LINKED_LIST_FOR_EACH(event_queue)
	{
		uop = linked_list_get(event_queue);
		if (uop->thread != thread || uop->uinst->opcode != x86_uinst_load)
			continue;
		if (x86_cpu->cycle - uop->issue_when > 5)
			return 1;
	}
	return 0;
}


void x86_event_queue_insert(struct linked_list_t *event_queue, struct x86_uop_t *uop)
{
	struct x86_uop_t *item;

	assert(!uop->in_event_queue);
	linked_list_head(event_queue);
	for (;;) {
		item = linked_list_get(event_queue);
		if (!item || eventq_compare(uop, item) < 0)
			break;
		linked_list_next(event_queue);
	}
	linked_list_insert(event_queue, uop);
	uop->in_event_queue = 1;
}


struct x86_uop_t *x86_event_queue_extract(struct linked_list_t *event_queue)
{
	struct x86_uop_t *uop;

	if (!linked_list_count(event_queue))
		return NULL;

	linked_list_head(event_queue);
	uop = linked_list_get(event_queue);
	assert(x86_uop_exists(uop));
	assert(uop->in_event_queue);
	linked_list_remove(event_queue);
	uop->in_event_queue = 0;
	return uop;
}


void x86_event_queue_recover(int core, int thread)
{
	struct linked_list_t *event_queue = X86_CORE.event_queue;
	struct x86_uop_t *uop;

	linked_list_head(event_queue);
	while (!linked_list_is_end(event_queue))
	{
		uop = linked_list_get(event_queue);
		if (uop->thread == thread && uop->specmode)
		{
			linked_list_remove(event_queue);
			uop->in_event_queue = 0;
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(event_queue);
	}
}
