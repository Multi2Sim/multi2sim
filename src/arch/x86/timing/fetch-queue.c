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

#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "cpu.h"
#include "uop.h"


int x86_fetch_queue_size;


void x86_fetch_queue_init()
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		X86_THREAD.fetch_queue = list_create_with_size(x86_fetch_queue_size);
}


void x86_fetch_queue_done()
{
	int core;
	int thread;

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
	if (!uop->trace_cache && !uop->mop_index)
	{
		X86_THREAD.fetchq_occ -= uop->mop_size;
		assert(X86_THREAD.fetchq_occ >= 0);
	}
	if (uop->trace_cache)
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

