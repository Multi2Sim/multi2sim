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
	struct list_t *uopq = THREAD.uopq;
	struct uop_t *uop;

	/* Uop queue empty. */
	uop = list_get(uopq, 0);
	if (!uop)
		return !THREAD.ctx || !ctx_get_status(THREAD.ctx, ctx_running) ?
			di_stall_ctx : di_stall_uopq;

	/* If iq/lq/sq/rob full, done */
	if (!rob_can_enqueue(uop))
		return di_stall_rob;
	if (!(uop->flags & FMEM) && !iq_can_insert(uop))
		return di_stall_iq;
	if ((uop->flags & FMEM) && !lsq_can_insert(uop))
		return di_stall_lsq;
	if (!rf_can_rename(uop))
		return di_stall_rename;
	
	return di_stall_used;
}


static int dispatch_thread(int core, int thread, int quant)
{
	struct uop_t *uop;
	enum di_stall_enum stall;

	while (quant) {
		
		/* Check if we can decode */
		stall = can_dispatch_thread(core, thread);
		if (stall != di_stall_used) {
			CORE.di_stall[stall] += quant;
			break;
		}
	
		/* Get entry from uop queue */
		uop = list_remove_at(THREAD.uopq, 0);
		assert(uop_exists(uop));
		uop->in_uopq = 0;
		
		/* Rename */
		rf_rename(uop);
		
		/* Insert in ROB */
		rob_enqueue(uop);
		CORE.rob_writes++;
		THREAD.rob_writes++;
		
		/* Non memory instruction into IQ */
		if (!(uop->flags & FMEM)) {
			iq_insert(uop);
			CORE.iq_writes++;
			THREAD.iq_writes++;
		}
		
		/* Memory instructions into the LSQ */
		if (uop->flags & FMEM) {
			lsq_insert(uop);
			CORE.lsq_writes++;
			THREAD.lsq_writes++;
		}
		
		/* Another instruction dispatched */
		uop->di_seq = ++CORE.di_seq;
		CORE.di_stall[uop->specmode ? di_stall_spec : di_stall_used]++;
		THREAD.dispatched[uop->uop]++;
		CORE.dispatched[uop->uop]++;
		p->dispatched[uop->uop]++;
		quant--;

		/* Pipeline debug */
		esim_debug("uop action=\"create\", core=%d, seq=%llu, name=\"%s\","
			" mop_name=\"%s\", mop_count=%d, mop_index=%d, spec=%u,"
			" stg_dispatch=1, in_rob=%u, in_iq=%u, in_lsq=%u\n",
			uop->core, (long long unsigned) uop->di_seq, uop->name,
			uop->mop_name, uop->mop_count, uop->mop_index, uop->specmode,
			!!uop->in_rob, !!uop->in_iq, uop->in_lq || uop->in_sq);
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

