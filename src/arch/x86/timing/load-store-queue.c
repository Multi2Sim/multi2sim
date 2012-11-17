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

#include <lib/util/linked-list.h>

#include "cpu.h"
#include "load-store-queue.h"
#include "uop.h"


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
		X86_THREAD.preq = linked_list_create();
	}
}


void x86_lsq_done()
{
	struct linked_list_t *lq, *sq, *preq;
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

	/* Prefetch queue */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		preq = X86_THREAD.preq;
		linked_list_head(preq);
		while (linked_list_count(preq))
		{
			uop = linked_list_get(preq);
			uop->in_preq = 0;
			linked_list_remove(preq);
			x86_uop_free_if_not_queued(uop);
		}
		linked_list_free(preq);
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
	struct linked_list_t *preq = X86_THREAD.preq;

	assert(!uop->in_lq && !uop->in_sq);
	assert(uop->uinst->opcode == x86_uinst_load || uop->uinst->opcode == x86_uinst_store ||
		uop->uinst->opcode == x86_uinst_prefetch);

	if (uop->uinst->opcode == x86_uinst_load)
	{
		linked_list_out(lq);
		linked_list_insert(lq, uop);
		uop->in_lq = 1;
	}
	else if (uop->uinst->opcode == x86_uinst_store)
	{
		linked_list_out(sq);
		linked_list_insert(sq, uop);
		uop->in_sq = 1;
	}
	else
	{
		linked_list_out(preq);
		linked_list_insert(preq, uop);
		uop->in_preq = 1;
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

/* Remove an uop in the current position of the prefetch queue */
void x86_preq_remove(int core, int thread)
{
	struct linked_list_t *preq = X86_THREAD.preq;
	struct x86_uop_t *uop;
 
	uop = linked_list_get(preq);
	assert(x86_uop_exists(uop));
	assert(uop->in_preq);
	linked_list_remove(preq);
	uop->in_preq = 0;
 
	assert(X86_CORE.lsq_count && X86_THREAD.lsq_count);
	X86_CORE.lsq_count--;
	X86_THREAD.lsq_count--;
}
