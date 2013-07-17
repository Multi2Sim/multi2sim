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


#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "core.h"
#include "cpu.h"
#include "uop.h"
#include "uop-queue.h"
#include "thread.h"


int x86_uop_queue_size;


void X86ThreadInitUopQueue(X86Thread *self)
{
	self->uop_queue = list_create_with_size(x86_uop_queue_size);
}


void X86ThreadFreeUopQueue(X86Thread *self)
{
	struct list_t *uop_queue;
	struct x86_uop_t *uop;

	uop_queue = self->uop_queue;
	while (list_count(uop_queue))
	{
		uop = list_remove_at(uop_queue, 0);
		uop->in_uop_queue = 0;
		x86_uop_free_if_not_queued(uop);
	}
	list_free(uop_queue);
}


void X86ThreadRecoverUopQueue(X86Thread *self)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct list_t *uop_queue = self->uop_queue;
	struct x86_uop_t *uop;

	while (list_count(uop_queue))
	{
		uop = list_get(uop_queue, list_count(uop_queue) - 1);
		assert(uop->thread == self);
		if (!uop->specmode)
			break;
		list_remove_at(uop_queue, list_count(uop_queue) - 1);
		uop->in_uop_queue = 0;

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"sq\"\n",
				uop->id_in_core, core->id);
			X86CpuAddToTraceList(cpu, uop);
		}

		/* Free */
		x86_uop_free_if_not_queued(uop);
	}
}




