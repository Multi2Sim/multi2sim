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
#include "uop-queue.h"


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




