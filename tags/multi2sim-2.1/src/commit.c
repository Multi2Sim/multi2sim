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


static int can_commit_uop_rob(struct uop_t *uop)
{
	if (!(uop->flags & FMEM) || (uop->flags & FLOAD))
		return uop->completed;
	if (uop->flags & FSTORE)
		return uop->ready;
	panic("can_commit_uop_rob: shouln't get here");
	return TRUE;
}


static int can_commit_uop_vb(struct uop_t *uop)
{
	if ((uop->flags & FCTRL))
		return uop->completed;
	if (uop->uop == uop_effaddr && p_effaddr)
		return uop->completed;
	return TRUE;
}


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
	 * located at the ROB head in shared approeaches, end. */
	if (!rob_can_dequeue(core, thread))
		return FALSE;

	/* Get instruction from ROB head */
	uop = rob_head(core, thread);
	assert(uop_exists(uop));
	assert(uop->core == core && uop->thread == thread);

	/* Return */
	return p_arch == p_arch_rob ? can_commit_uop_rob(uop) :
		can_commit_uop_vb(uop);
}


static void commit_thread(int core, int thread, int quant)
{
	struct uop_t *uop;
	
	while (quant && can_commit_thread(core, thread)) {
		
		/* Get instruction at the head of the ROB */
		uop = rob_head(core, thread);
		assert(uop_exists(uop));
		assert(uop->core == core);
		assert(uop->thread == thread);
		
		/* First instruction in specmode; recover.
		 * When we do recover at commit, we can stop functional units. */
		if (uop->specmode && p_recover_kind == p_recover_kind_commit) {
			p_recover(core, thread);
			fu_release(CORE.fu);
			continue;
		}
		
		/* Free physical registers */
		assert(!uop->specmode);
		phregs_commit(uop);
		
		/* Branches update branch predictor */
		if ((uop->flags & FCTRL))
			bpred_update(THREAD.bpred, uop);
			
		/* Pipeline trace */
		ptrace_new_stage(uop, ptrace_commit);
		ptrace_end_uop(uop);

		/* Stats */
		THREAD.last_commit_cycle = sim_cycle;
		THREAD.committed++;
		p->committed++;
		
		/* Retire instruction */
		rob_remove_head(core, thread);
		quant--;
	}
}


void commit_core(int core)
{
	int pass, quant, new;

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