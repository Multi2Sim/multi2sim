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


static int can_commit_thread(int core, int thread)
{
	struct uop_t *uop;
	struct ctx_t *ctx = THREAD.ctx;

	/* Sanity check - If the context is running, we assume that something is
	 * going wrong if more than 1M cycles go by without committing an inst. */
	if (!ctx || !ctx_get_status(ctx, ctx_running))
		THREAD.last_commit_cycle = sim_cycle;
	if (sim_cycle - THREAD.last_commit_cycle > 1000000)
		panic("c%dt%d: no inst committed in 1M cycles", core, thread);

	/* If there is no instruction in the ROB, or the instruction is not
	 * located at the ROB head in shared approaches, end. */
	if (!rob_can_dequeue(core, thread))
		return 0;

	/* Get instruction from ROB head */
	uop = rob_head(core, thread);
	assert(uop_exists(uop));
	assert(uop->core == core && uop->thread == thread);

	/* Stores must be ready. Update here 'uop->ready' flag for efficiency,
	 * if the call to 'rf_ready' shows input registers to be ready. */
	if (uop->flags & FSTORE) {
		if (!uop->ready && rf_ready(uop))
			uop->ready = 1;
		return uop->ready;
	}
	
	/* Instructions other than stores must be completed. */
	return uop->completed;
}


static void commit_thread(int core, int thread, int quant)
{
	struct ctx_t *ctx = THREAD.ctx;
	struct uop_t *uop;
	int recover = 0;

	/* Update pipeline debugger with ready stores */
	if (esim_debug_file)
		uop_lnlist_check_if_ready(THREAD.sq);
	
	/* Commit stage for thread */
	assert(ctx);
	while (quant && can_commit_thread(core, thread)) {
		
		/* Get instruction at the head of the ROB */
		uop = rob_head(core, thread);
		assert(uop_exists(uop));
		assert(uop->core == core);
		assert(uop->thread == thread);
		assert(!recover);
		
		/* Mispredicted branch */
		if (p_recover_kind == p_recover_kind_commit &&
			(uop->flags & FCTRL) && uop->neip != uop->pred_neip)
			recover = 1;
	
		/* Free physical registers */
		assert(!uop->specmode);
		rf_commit(uop);
		
		/* Branches update branch predictor and btb */
		if (uop->flags & FCTRL) {
			bpred_update(THREAD.bpred, uop);
			bpred_btb_update(THREAD.bpred, uop);
			THREAD.btb_writes++;
		}

		/* Trace cache */
		if (tcache_present)
			tcache_new_uop(THREAD.tcache, uop);
			
		/* Stats */
		THREAD.last_commit_cycle = sim_cycle;
		THREAD.committed[uop->uop]++;
		CORE.committed[uop->uop]++;
		p->committed[uop->uop]++;
		sim_inst++;
		if (uop->fetch_tcache)
			THREAD.tcache->committed++;
		if (uop->flags & FCTRL) {
			THREAD.branches++;
			CORE.branches++;
			p->branches++;
			if (uop->neip != uop->pred_neip) {
				THREAD.mispred++;
				CORE.mispred++;
				p->mispred++;
			}
		}

		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu, stg_commit=1\n",
			uop->core, (long long unsigned) uop->di_seq);
		esim_debug("uop action=\"destroy\", core=%d, seq=%llu\n",
			uop->core, (long long unsigned) uop->di_seq);
		
		/* Retire instruction */
		rob_remove_head(core, thread);
		CORE.rob_reads++;
		THREAD.rob_reads++;
		quant--;

		/* Recover. Functional units are cleared when processor
		 * recovers at commit. */
		if (recover) {
			p_recover(core, thread);
			fu_release(core);
		}
	}

	/* If context eviction signal is activated and pipeline is empty,
	 * deallocate context. */
	if (ctx->dealloc_signal && p_pipeline_empty(core, thread))
		p_unmap_context(core, thread);
}


void commit_core(int core)
{
	int pass, quant, new;

	/* Commit stage for core */
	switch (p_commit_kind) {

	case p_commit_kind_shared:
		pass = p_threads;
		quant = p_commit_width;
		while (quant && pass) {
			CORE.commit_current = (CORE.commit_current + 1) % p_threads;
			if (can_commit_thread(core, CORE.commit_current)) {
				commit_thread(core, CORE.commit_current, 1);
				quant--;
				pass = p_threads;
			} else
				pass--;
		}
		break;
	
	case p_commit_kind_timeslice:
	
		/* look for a not empty VB */
		new = (CORE.commit_current + 1) % p_threads;
		while (new != CORE.commit_current && !can_commit_thread(core, new))
			new = (new + 1) % p_threads;
		
		/* commit new thread */
		CORE.commit_current = new;
		commit_thread(core, new, p_commit_width);
		break;
	
	}
}


void p_commit()
{
	int core;
	p->stage = "commit";
	FOREACH_CORE
		commit_core(core);
}
