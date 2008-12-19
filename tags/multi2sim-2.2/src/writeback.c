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



void writeback_core(int core)
{
	struct uop_t *uop;
	int thread, recover = 0;

	for (;;) {
	
		/* Peek element from the head of the event queue */
		lnlist_head(CORE.eventq);
		uop = lnlist_get(CORE.eventq);
		if (lnlist_error(CORE.eventq) || uop->when > sim_cycle)
			break;
		
		/* Check element integrity */
		assert(uop_exists(uop));
		assert(uop->when == sim_cycle);
		assert(uop->core == core);
		assert(uop->ready);
		assert(!uop->completed);
		
		/* Ok, can extract element */
		lnlist_remove(CORE.eventq);
		uop->in_eventq = 0;
		thread = uop->thread;
		
		/* Loads and stores: even if they appear as completed in the event queue,
		 * they are not until the data witness says so. */
		if ((uop->flags & FMEM) && !uop->data_witness) {
			uop->when = sim_cycle + 1;
			eventq_insert(CORE.eventq, uop);
			continue;
		}

		/* If a mispredicted branch is solved and recovery is configured to be
		 * performed at writeback, schedule it for the end of the iteration. */
		if (p_recover_kind == p_recover_kind_writeback &&
			uop->neip != uop->pred_neip)
			recover = 1;

		/* Writeback */
		uop->completed = 1;
		phregs_write(uop);
		ptrace_new_stage(uop, ptrace_writeback);
		uop_free_if_not_queued(uop);

		/* Recovery. This must be performed at last, because lots of uops might be
		 * freed, which interferes with the temporary extraction from the eventq. */
		if (recover)
			p_recover(core, thread);
	}
}


void p_writeback()
{
	int core;
	p->stage = "writeback";
	FOREACH_CORE
		writeback_core(core);
}

