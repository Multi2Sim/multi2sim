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
#include "fetch-queue.h"
#include "uop.h"
#include "thread.h"


void X86ThreadInitFetchQueue(X86Thread *self)
{
	self->fetch_queue = list_create_with_size(x86_fetch_queue_size);
}


void X86ThreadFreeFetchQueue(X86Thread *self)
{
	struct list_t *fetchq;
	struct x86_uop_t *uop;

	fetchq = self->fetch_queue;
	while (list_count(fetchq)) {
		uop = list_remove_at(fetchq, 0);
		uop->in_fetch_queue = 0;
		x86_uop_free_if_not_queued(uop);
	}
	list_free(fetchq);
}


struct x86_uop_t *X86ThreadRemoveFromFetchQueue(X86Thread *self, int index)
{
	struct list_t *fetchq = self->fetch_queue;
	struct x86_uop_t *uop;
	assert(index >= 0 && index < list_count(fetchq));
	uop = list_remove_at(fetchq, index);
	uop->in_fetch_queue = 0;
	if (!uop->trace_cache && !uop->mop_index)
	{
		self->fetchq_occ -= uop->mop_size;
		assert(self->fetchq_occ >= 0);
	}
	if (uop->trace_cache)
	{
		self->trace_cache_queue_occ--;
		assert(self->trace_cache_queue_occ >= 0);
	}
	if (!list_count(fetchq))
	{
		assert(!self->fetchq_occ);
		assert(!self->trace_cache_queue_occ);
	}
	return uop;
}


void X86ThreadRecoverFetchQueue(X86Thread *self)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct list_t *fetchq = self->fetch_queue;
	struct x86_uop_t *uop;

	while (list_count(fetchq))
	{
		uop = list_get(fetchq, list_count(fetchq) - 1);
		assert(uop->thread == self);
		if (!uop->specmode)
			break;
		uop = X86ThreadRemoveFromFetchQueue(self, list_count(fetchq) - 1);

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



/*
 * Public
 */


int x86_fetch_queue_size;
