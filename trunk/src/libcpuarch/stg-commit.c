/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpuarch.h>

static char *err_commit_stall =
	"\tThe CPU commit stage has not received any instruction for 1M\n"
	"\tcycles. Most likely, this means that a deadlock condition\n"
	"\toccurred in the management of some modeled structure (network,\n"
	"\tcache system, core queues, etc.).\n";


static int can_commit_thread(int core, int thread)
{
	struct uop_t *uop;
	struct ctx_t *ctx = THREAD.ctx;

	/* Sanity check - If the context is running, we assume that something is
	 * going wrong if more than 1M cycles go by without committing an inst. */
	if (!ctx || !ctx_get_status(ctx, ctx_running))
		THREAD.last_commit_cycle = cpu->cycle;
	if (cpu->cycle - THREAD.last_commit_cycle > 1000000)
	{
		fprintf(stderr, "core-thread %d-%d: commit stall.\n%s",
			core, thread, err_commit_stall);
		ke_sim_finish = ke_sim_finish_max_time; //////////////
	}

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
	if (uop->uinst->opcode == x86_uinst_store)
	{
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
	while (quant && can_commit_thread(core, thread))
	{
		/* Get instruction at the head of the ROB */
		uop = rob_head(core, thread);
		assert(uop_exists(uop));
		assert(uop->core == core);
		assert(uop->thread == thread);
		assert(!recover);
		
		/* Mispredicted branch */
		if (cpu_recover_kind == cpu_recover_kind_commit &&
			(uop->flags & X86_UINST_CTRL) && uop->neip != uop->pred_neip)
			recover = 1;
	
		/* Free physical registers */
		assert(!uop->specmode);
		rf_commit(uop);
		
		/* Branches update branch predictor and btb */
		if (uop->flags & X86_UINST_CTRL)
		{
			bpred_update(THREAD.bpred, uop);
			bpred_btb_update(THREAD.bpred, uop);
			THREAD.btb_writes++;
		}

		/* Trace cache */
		if (trace_cache_present)
			trace_cache_new_uop(THREAD.trace_cache, uop);
			
		/* Statistics */
		THREAD.last_commit_cycle = cpu->cycle;
		THREAD.committed[uop->uinst->opcode]++;
		CORE.committed[uop->uinst->opcode]++;
		cpu->committed[uop->uinst->opcode]++;
		cpu->inst++;
		ctx->inst_count++;
		if (uop->fetch_trace_cache)
			THREAD.trace_cache->committed++;
		if (uop->flags & X86_UINST_CTRL)
		{
			THREAD.branches++;
			CORE.branches++;
			cpu->branches++;
			if (uop->neip != uop->pred_neip)
			{
				THREAD.mispred++;
				CORE.mispred++;
				cpu->mispred++;
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
		if (recover)
		{
			cpu_recover(core, thread);
			fu_release(core);
		}
	}

	/* If context eviction signal is activated and pipeline is empty,
	 * deallocate context. */
	if (ctx->dealloc_signal && cpu_pipeline_empty(core, thread))
		cpu_unmap_context(core, thread);
}


void commit_core(int core)
{
	int pass, quant, new;

	/* Commit stage for core */
	switch (cpu_commit_kind)
	{

	case cpu_commit_kind_shared:
		pass = cpu_threads;
		quant = cpu_commit_width;
		while (quant && pass) {
			CORE.commit_current = (CORE.commit_current + 1) % cpu_threads;
			if (can_commit_thread(core, CORE.commit_current))
			{
				commit_thread(core, CORE.commit_current, 1);
				quant--;
				pass = cpu_threads;
			}
			else
				pass--;
		}
		break;
	
	case cpu_commit_kind_timeslice:
	
		/* look for a not empty VB */
		new = (CORE.commit_current + 1) % cpu_threads;
		while (new != CORE.commit_current && !can_commit_thread(core, new))
			new = (new + 1) % cpu_threads;
		
		/* commit new thread */
		CORE.commit_current = new;
		commit_thread(core, new, cpu_commit_width);
		break;
	
	}
}


void cpu_commit()
{
	int core;
	cpu->stage = "commit";
	FOREACH_CORE
		commit_core(core);
}
