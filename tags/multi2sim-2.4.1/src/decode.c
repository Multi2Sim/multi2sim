/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>

static void decode_thread(int core, int thread)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct list_t *uopq = THREAD.uopq;
	struct uop_t *uop;
	int i;

	for (i = 0; i < p_decode_width; i++) {
		
		/* Empty fetch queue, full uopq */
		if (!list_count(fetchq))
			break;
		if (list_count(uopq) >= uopq_size)
			break;
		uop = list_get(fetchq, 0);
		assert(uop_exists(uop));

		/* If instructions come from the trace cache, i.e., are located in
		 * the trace cache queue, copy all of them
		 * into the uop queue in one single decode slot. */
		if (uop->fetch_tcache) {
			do {
				fetchq_remove(core, thread, 0);
				list_add(uopq, uop);
				uop->in_uopq = 1;
				uop = list_get(fetchq, 0);
			} while (uop && uop->fetch_tcache);
			break;
		}

		/* Decode one macroinstruction coming from a block in the instruction
		 * cache. If the cache access finished, extract it from the fetch queue. */
		assert(!uop->mop_index);
		if (!cache_system_pending_access(core, thread, cache_kind_inst, uop->fetch_access)) {
			do {
				fetchq_remove(core, thread, 0);
				list_add(uopq, uop);
				uop->in_uopq = 1;
				uop = list_get(fetchq, 0);
			} while (uop && uop->mop_index);
		}
	}
}


static void decode_core(int core)
{
	int thread;
	FOREACH_THREAD
		decode_thread(core, thread);
}


void p_decode()
{
	int core;
	p->stage = "decode";
	FOREACH_CORE
		decode_core(core);
}
