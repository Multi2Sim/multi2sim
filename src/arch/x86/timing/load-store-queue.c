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


#include <lib/util/linked-list.h>

#include "core.h"
#include "cpu.h"
#include "load-store-queue.h"
#include "uop.h"
#include "thread.h"


char *x86_lsq_kind_map[] = { "Shared", "Private" };
enum x86_lsq_kind_t x86_lsq_kind;
int x86_lsq_size;



/*
 * Class 'X86Thread'
 */

void X86ThreadInitLSQ(X86Thread *self)
{
	self->lq = linked_list_create();
	self->sq = linked_list_create();
	self->preq = linked_list_create();
}


void X86ThreadFreeLSQ(X86Thread *self)
{
	struct linked_list_t *lq;
	struct linked_list_t *sq;
	struct linked_list_t *preq;
	struct x86_uop_t *uop;

	/* Load queue */
	lq = self->lq;
	linked_list_head(lq);
	while (linked_list_count(lq))
	{
		uop = linked_list_get(lq);
		uop->in_lq = 0;
		linked_list_remove(lq);
		x86_uop_free_if_not_queued(uop);
	}
	linked_list_free(lq);

	/* Store queue */
	sq = self->sq;
	linked_list_head(sq);
	while (linked_list_count(sq))
	{
		uop = linked_list_get(sq);
		uop->in_sq = 0;
		linked_list_remove(sq);
		x86_uop_free_if_not_queued(uop);
	}
	linked_list_free(sq);

	/* Prefetch queue */
	preq = self->preq;
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


int X86ThreadCanInsertInLSQ(X86Thread *self, struct x86_uop_t *uop)
{
	X86Core *core = self->core;

	int count;
	int size;

	assert(uop->thread == self);
	size = x86_lsq_kind == x86_lsq_kind_private ? x86_lsq_size : x86_lsq_size * x86_cpu_num_threads;
	count = x86_lsq_kind == x86_lsq_kind_private ? self->lsq_count : core->lsq_count;
	return count < size;
}


/* Insert uop into corresponding load/store queue */
void X86ThreadInsertInLSQ(X86Thread *self, struct x86_uop_t *uop)
{
	X86Core *core = self->core;

	struct linked_list_t *lq = self->lq;
	struct linked_list_t *sq = self->sq;
	struct linked_list_t *preq = self->preq;

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
	core->lsq_count++;
	self->lsq_count++;
}


/* Remove all speculative uops from a given load/store queue in the
 * given thread. */
void X86ThreadRecoverLSQ(X86Thread *self)
{
	struct linked_list_t *lq = self->lq;
	struct linked_list_t *sq = self->sq;
	struct x86_uop_t *uop;

	/* Recover load queue */
	linked_list_head(lq);
	while (!linked_list_is_end(lq))
	{
		uop = linked_list_get(lq);
		if (uop->specmode)
		{
			X86ThreadRemoveFromLQ(self);
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
			X86ThreadRemoveFromSQ(self);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(sq);
	}
}


/* Remove the uop in the current position of the linked list representing
 * the load queue of the specified thread. */
void X86ThreadRemoveFromLQ(X86Thread *self)
{
	X86Core *core = self->core;

	struct linked_list_t *lq = self->lq;
	struct x86_uop_t *uop;

	uop = linked_list_get(lq);
	assert(x86_uop_exists(uop));
	linked_list_remove(lq);
	uop->in_lq = 0;

	assert(core->lsq_count && self->lsq_count);
	core->lsq_count--;
	self->lsq_count--;
}


/* Remove an uop in the current position of the store queue */
void X86ThreadRemoveFromSQ(X86Thread *self)
{
	X86Core *core = self->core;

	struct linked_list_t *sq = self->sq;
	struct x86_uop_t *uop;

	uop = linked_list_get(sq);
	assert(x86_uop_exists(uop));
	assert(uop->in_sq);
	linked_list_remove(sq);
	uop->in_sq = 0;

	assert(core->lsq_count && self->lsq_count);
	core->lsq_count--;
	self->lsq_count--;
}

/* Remove an uop in the current position of the prefetch queue */
void X86ThreadRemovePreQ(X86Thread *self)
{
	X86Core *core = self->core;

	struct linked_list_t *preq = self->preq;
	struct x86_uop_t *uop;
 
	uop = linked_list_get(preq);
	assert(x86_uop_exists(uop));
	assert(uop->in_preq);
	linked_list_remove(preq);
	uop->in_preq = 0;
 
	assert(core->lsq_count && self->lsq_count);
	core->lsq_count--;
	self->lsq_count--;
}

