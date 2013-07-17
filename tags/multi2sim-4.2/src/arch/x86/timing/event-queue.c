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
#include "event-queue.h"
#include "thread.h"
#include "uop.h"



/*
 * Class 'X86Core'
 */

void X86CoreInitEventQueue(X86Core *self)
{
	self->event_queue = linked_list_create();
}


void X86CoreFreeEventQueue(X86Core *self)
{
	struct x86_uop_t *uop;

	while (linked_list_count(self->event_queue))
	{
		uop = X86CoreExtractFromEventQueue(self);
		x86_uop_free_if_not_queued(uop);
	}
	linked_list_free(self->event_queue);
}


static int eventq_compare(const void *item1, const void *item2)
{
	const struct x86_uop_t *uop1 = item1;
	const struct x86_uop_t *uop2 = item2;
	return uop1->when != uop2->when ? uop1->when - uop2->when
		: uop1->id - uop2->id;
}


void X86CoreInsertInEventQueue(X86Core *self, struct x86_uop_t *uop)
{
	struct linked_list_t *event_queue = self->event_queue;
	struct x86_uop_t *item;

	assert(!uop->in_event_queue);
	linked_list_head(event_queue);
	for (;;)
	{
		item = linked_list_get(event_queue);
		if (!item || eventq_compare(uop, item) < 0)
			break;
		linked_list_next(event_queue);
	}
	linked_list_insert(event_queue, uop);
	uop->in_event_queue = 1;
}


struct x86_uop_t *X86CoreExtractFromEventQueue(X86Core *self)
{
	struct linked_list_t *event_queue = self->event_queue;
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




/*
 * Class 'X86Thread'
 */

int X86ThreadLongLatencyInEventQueue(X86Thread *self)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct linked_list_t *event_queue = core->event_queue;
	struct x86_uop_t *uop;
	
	LINKED_LIST_FOR_EACH(event_queue)
	{
		uop = linked_list_get(event_queue);
		if (uop->thread != self)
			continue;
		if (asTiming(cpu)->cycle - uop->issue_when > 20)
			return 1;
	}
	return 0;
}


int X86ThreadCacheMissInEventQueue(X86Thread *self)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct linked_list_t *event_queue = core->event_queue;
	struct x86_uop_t *uop;

	LINKED_LIST_FOR_EACH(event_queue)
	{
		uop = linked_list_get(event_queue);
		if (uop->thread != self || uop->uinst->opcode != x86_uinst_load)
			continue;
		if (asTiming(cpu)->cycle - uop->issue_when > 5)
			return 1;
	}
	return 0;
}


void X86ThreadRecoverEventQueue(X86Thread *self)
{
	X86Core *core = self->core;

	struct linked_list_t *event_queue = core->event_queue;
	struct x86_uop_t *uop;

	linked_list_head(event_queue);
	while (!linked_list_is_end(event_queue))
	{
		uop = linked_list_get(event_queue);
		if (uop->thread == self && uop->specmode)
		{
			linked_list_remove(event_queue);
			uop->in_event_queue = 0;
			x86_uop_free_if_not_queued(uop);
			continue;
		}
		linked_list_next(event_queue);
	}
}

