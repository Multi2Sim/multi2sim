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
#include <mem-system/module.h>

#include "cpu.h"
#include "fetch-queue.h"
#include "uop.h"
#include "uop-queue.h"


static void x86_cpu_decode_thread(int core, int thread)
{
	struct list_t *fetchq = X86_THREAD.fetch_queue;
	struct list_t *uopq = X86_THREAD.uop_queue;
	struct x86_uop_t *uop;
	int i;

	for (i = 0; i < x86_cpu_decode_width; i++)
	{
		/* Empty fetch queue, full uop_queue */
		if (!list_count(fetchq))
			break;
		if (list_count(uopq) >= x86_uop_queue_size)
			break;
		uop = list_get(fetchq, 0);
		assert(x86_uop_exists(uop));

		/* If instructions come from the trace cache, i.e., are located in
		 * the trace cache queue, copy all of them
		 * into the uop queue in one single decode slot. */
		if (uop->trace_cache)
		{
			do {
				x86_fetch_queue_remove(core, thread, 0);
				list_add(uopq, uop);
				uop->in_uop_queue = 1;
				uop = list_get(fetchq, 0);
			} while (uop && uop->trace_cache);
			break;
		}

		/* Decode one macro-instruction coming from a block in the instruction
		 * cache. If the cache access finished, extract it from the fetch queue. */
		assert(!uop->mop_index);
		if (!mod_in_flight_access(X86_THREAD.inst_mod, uop->fetch_access, uop->fetch_address))
		{
			do {
				/* Move from fetch queue to uop queue */
				x86_fetch_queue_remove(core, thread, 0);
				list_add(uopq, uop);
				uop->in_uop_queue = 1;

				/* Trace */
				x86_trace("x86.inst id=%lld core=%d stg=\"dec\"\n",
					uop->id_in_core, uop->core);

				/* Next */
				uop = list_get(fetchq, 0);

			} while (uop && uop->mop_index);
		}
	}
}


static void x86_cpu_decode_core(int core)
{
	int thread;

	X86_THREAD_FOR_EACH
		x86_cpu_decode_thread(core, thread);
}


void x86_cpu_decode()
{
	int core;

	x86_cpu->stage = "decode";
	X86_CORE_FOR_EACH
		x86_cpu_decode_core(core);
}
