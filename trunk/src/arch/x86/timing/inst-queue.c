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
#include "inst-queue.h"
#include "thread.h"


char *x86_iq_kind_map[] = { "Shared", "Private" };
enum x86_iq_kind_t x86_iq_kind;
int x86_iq_size;


void X86ThreadInitIQ(X86Thread *self)
{
	self->iq = linked_list_create();
}


void X86ThreadFreeIQ(X86Thread *self)
{
	struct linked_list_t *iq;
	struct x86_uop_t *uop;

	iq = self->iq;
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


int X86ThreadCanInsertInIQ(X86Thread *self, struct x86_uop_t *uop)
{
	X86Core *core = self->core;

	int count;
	int size;

	size = x86_iq_kind == x86_iq_kind_private ? x86_iq_size : x86_iq_size * x86_cpu_num_threads;
	count = x86_iq_kind == x86_iq_kind_private ? self->iq_count : core->iq_count;
	return count < size;
}


/* Insert a uop into the corresponding IQ. Since this is a non-FIFO queue,
 * the insertion position doesn't matter. */
void X86ThreadInsertInIQ(X86Thread *self, struct x86_uop_t *uop)
{
	X86Core *core = self->core;
	struct linked_list_t *iq = self->iq;

	assert(!uop->in_iq);
	linked_list_out(iq);
	linked_list_insert(iq, uop);
	uop->in_iq = 1;

	core->iq_count++;
	self->iq_count++;
}


/* Remove the uop in the current position of the linked list representing
 * the IQ of the specified thread. */
void X86ThreadRemoveFromIQ(X86Thread *self)
{
	X86Core *core = self->core;
	struct linked_list_t *iq = self->iq;
	struct x86_uop_t *uop;

	uop = linked_list_get(iq);
	assert(x86_uop_exists(uop));
	linked_list_remove(iq);
	uop->in_iq = 0;

	assert(core->iq_count && self->iq_count);
	core->iq_count--;
	self->iq_count--;
}


/* Remove all speculative uops from the current thread */
void X86ThreadRecoverIQ(X86Thread *self)
{
	struct linked_list_t *iq = self->iq;
	struct x86_uop_t *uop;

	linked_list_head(iq);
	while (!linked_list_is_end(iq))
	{
		uop = linked_list_get(iq);
		if (uop->specmode) {
			X86ThreadRemoveFromIQ(self);
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(iq);
	}
}

