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
	int thread;

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
		uop->in_eventq = FALSE;
		thread = uop->thread;
		
		/* Loads and stores: even if they appear as completed in the event queue,
		 * they are not until the data witness says so. */
		if ((uop->flags & FMEM) && !uop->data_witness) {
			uop->when = sim_cycle + 1;
			eventq_insert(CORE.eventq, uop);
			uop->in_eventq = TRUE;
			continue;
		}

		/* Completed loads and pdg fetch policy */
		/* Load finished memory access */
		if (p_fetch_policy == p_fetch_policy_pdg) {
			if ((uop->flags & FLOAD) && sim_cycle - uop->issue_when >= 5)
				uop->lmpred_actual_miss = TRUE;
			if (uop->lmpred_miss) {
				THREAD.lmpred_misses--;
				uop->lmpred_miss = FALSE;
				assert(THREAD.lmpred_misses >= 0);
			}
			if (uop->lmpred_actual_miss)
				THREAD.lmpred[uop->lmpred_idx] = 0;
			else
				THREAD.lmpred[uop->lmpred_idx] =
					MIN(THREAD.lmpred[uop->lmpred_idx] + 1, 3);
		}


		/* If it is a misspredicted branch and user selected the
		 * recovery process to be performed in writeback, do so */
		if (uop->mispred && p_recover_kind == p_recover_kind_writeback)
			p_recover(uop);

		/* Writeback */
		uop->completed = TRUE;
		phregs_write(uop);
		ptrace_new_stage(uop, ptrace_writeback);
		uop_free_if_not_queued(uop);
	}
}


void p_writeback()
{
	int core;
	p->stage = "writeback";
	FOREACH_CORE
		writeback_core(core);
}

