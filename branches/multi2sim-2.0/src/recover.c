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
	struct ctx_t *ctx = THREAD.ctx;
	struct uop_t *uop;

	/* Remove instructions of this thread in fetchq, iq, sq, lq and eventq. */
	fetchq_recover(core, thread);
	iq_recover(core, thread);
	lq_recover(core, thread);
	sq_recover(core, thread);
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
		
		/* Do we have to decrement pending_readers? */
		if (!uop->issued)
			phregs_read(uop);
		
		/* Do we have to mark destination physical registers as
		 * completed? */
		if (!uop->completed)
			phregs_write(uop);
		
		/* Undo map and remove entry in ROB */
		phregs_undo(uop);
		uop_pdg_recover(uop);
		ptrace_end_uop(uop);
		rob_remove_tail(core, thread);
	}

	/* If we actually fetched wrong instructions, recover kernel */
	if (ctx_get_status(ctx, ctx_specmode))
		ctx_recover(ctx);

	/* Stall fetch and set eip to fetch. */
	THREAD.fetch_stall = MAX(THREAD.fetch_stall, p_recover_penalty);
	THREAD.fetch_neip = ctx->regs->eip;
}

