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


void p_recover(int core, int thread)
{
	struct uop_t *uop;

	/* Remove instructions of this thread in fetchq, uopq, iq, sq, lq and eventq. */
	fetchq_recover(core, thread);
	uopq_recover(core, thread);
	iq_recover(core, thread);
	lsq_recover(core, thread);
	eventq_recover(core, thread);

	/* Remove instructions from ROB, restoring the state of the
	 * physical register file. */
	for (;;) {
		
		/* Get instruction */
		uop = rob_tail(core, thread);
		if (!uop)
			break;

		/* If we already removed all speculative instructions,
		 * the work is finished */
		assert(uop->core == core);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		
		/* Stats */
		if (uop->fetch_tcache)
			THREAD.tcache->squashed++;
		THREAD.squashed++;
		CORE.squashed++;
		p->squashed++;
		
		/* Undo map */
		if (!uop->completed)
			rf_write(uop);
		rf_undo(uop);

		/* Debug */
		esim_debug("uop action=\"squash\", core=%d, seq=%llu\n",
			uop->core, (long long unsigned) uop->di_seq);
 
		/* Remove entry in ROB */
		rob_remove_tail(core, thread);
	}

	/* If we actually fetched wrong instructions, recover kernel */
	if (ctx_get_status(THREAD.ctx, ctx_specmode))
		ctx_recover(THREAD.ctx);
	
	/* Stall fetch and set eip to fetch. */
	THREAD.fetch_stall = MAX(THREAD.fetch_stall, p_recover_penalty);
	THREAD.fetch_neip = THREAD.ctx->regs->eip;
}

