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


/* Return the reason why a thread cannot be dispatched. If it can,
 * return di_stall_used. */
static enum di_stall_enum can_dispatch_thread(int core, int thread)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct uop_t *uop;

	/* Fetch queue must have an instruction ready to be dequeued. */
	uop = list_get(fetchq, 0);
	if (!uop)
		return !THREAD.ctx || !ctx_get_status(THREAD.ctx, ctx_running) ?
			di_stall_ctx : di_stall_fetchq;
	if (cache_system_pending_access(uop->core, uop->thread, cache_kind_inst, uop->fetch_access))
		return di_stall_fetchq;

	/* If iq/lq/sq/rob full, done */
	if (!rob_can_enqueue(uop))
		return di_stall_rob;
	if (!(uop->flags & FMEM) && !iq_can_insert(uop))
		return di_stall_iq;
	if ((uop->flags & FLOAD) && !lq_can_insert(uop))
		return di_stall_lq;
	if ((uop->flags & FSTORE) && !sq_can_insert(uop))
		return di_stall_sq;
	if (!phregs_can_rename(uop))
		return di_stall_rename;
	
	return di_stall_used;
}


static int dispatch_thread(int core, int thread, int quant)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct uop_t *uop;
	enum di_stall_enum stall;

	while (quant) {
		
		/* Check if we can decode */
		stall = can_dispatch_thread(core, thread);
		if (stall != di_stall_used) {
			p->di_stall[stall] += quant;
			break;
		}
	
		/* Get entry from fetch queue */
		uop = list_remove_at(fetchq, 0);
		assert(!list_error(fetchq));
		assert(uop_exists(uop));
		uop->in_fetchq = FALSE;
		
		/* Rename */
		phregs_rename(uop);
		
		/* Insert in ROB */
		rob_enqueue(uop);
		
		/* Non memory instruction into IQ */
		if (!(uop->flags & FMEM))
			iq_insert(uop);
		
		/* Loads are inserted into the LQ */
		if ((uop->flags & FLOAD))
			lq_insert(uop);
		
		/* Stores are inserted into the SQ */
		if ((uop->flags & FSTORE))
			sq_insert(uop);

		/* Another instruction dispatched */
		ptrace_new_stage(uop, ptrace_dispatch);
		p->di_stall[uop->specmode ? di_stall_spec : di_stall_used]++;
		p->dispatched++;
		THREAD.dispatched++;
		quant--;
	}

	return quant;
}

void dispatch_core(int core)
{
	int skip = p_threads;
	int quant = p_dispatch_width;
	int remain;

	switch (p_dispatch_kind) {

	case p_dispatch_kind_shared:
		
		do {
			CORE.dispatch_current = (CORE.dispatch_current + 1) % p_threads;
			remain = dispatch_thread(core, CORE.dispatch_current, 1);
			skip = remain ? skip - 1 : p_threads;
			quant = remain ? quant : quant - 1;
		} while (quant && skip);
		break;
	
	case p_dispatch_kind_timeslice:
		
		do {
			CORE.dispatch_current = (CORE.dispatch_current + 1) % p_threads;
			skip--;
		} while (skip && can_dispatch_thread(core, CORE.dispatch_current) != di_stall_used);
		dispatch_thread(core, CORE.dispatch_current, quant);
		break;
	}
}


void p_dispatch()
{
	int core;
	p->stage = "dispatch";
	FOREACH_CORE
		dispatch_core(core);
}

