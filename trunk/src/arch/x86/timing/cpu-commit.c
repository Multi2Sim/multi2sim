/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>

#include <arch/common/arch.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>

#include "bpred.h"
#include "cpu.h"
#include "fu.h"
#include "reg-file.h"
#include "rob.h"
#include "trace-cache.h"


static char *err_x86_cpu_commit_stall =
	"\tThe CPU commit stage has not received any instruction for 1M\n"
	"\tcycles. Most likely, this means that a deadlock condition\n"
	"\toccurred in the management of some modeled structure (network,\n"
	"\tcache system, core queues, etc.).\n";


static int x86_cpu_can_commit_thread(int core, int thread)
{
	struct x86_uop_t *uop;
	struct x86_ctx_t *ctx = X86_THREAD.ctx;

	/* Sanity check - If the context is running, we assume that something is
	 * going wrong if more than 1M cycles go by without committing an inst. */
	if (!ctx || !x86_ctx_get_state(ctx, x86_ctx_running))
		X86_THREAD.last_commit_cycle = arch_x86->cycle;
	if (arch_x86->cycle - X86_THREAD.last_commit_cycle > 1000000)
	{
		warning("core-thread %d-%d: simulation ended due to commit stall.\n%s",
			core, thread, err_x86_cpu_commit_stall);
		esim_finish = esim_finish_stall;
	}

	/* If there is no instruction in the ROB, or the instruction is not
	 * located at the ROB head in shared approaches, end. */
	if (!x86_rob_can_dequeue(core, thread))
		return 0;

	/* Get instruction from ROB head */
	uop = x86_rob_head(core, thread);
	assert(x86_uop_exists(uop));
	assert(uop->core == core && uop->thread == thread);

	/* Stores must be ready. Update here 'uop->ready' flag for efficiency,
	 * if the call to 'x86_reg_file_ready' shows input registers to be ready. */
	if (uop->uinst->opcode == x86_uinst_store)
	{
		if (!uop->ready && x86_reg_file_ready(uop))
			uop->ready = 1;
		return uop->ready;
	}
	
	/* Instructions other than stores must be completed. */
	return uop->completed;
}


static void x86_cpu_commit_thread(int core, int thread, int quant)
{
	struct x86_ctx_t *ctx = X86_THREAD.ctx;
	struct x86_uop_t *uop;
	int recover = 0;

	/* Commit stage for thread */
	assert(ctx);
	while (quant && x86_cpu_can_commit_thread(core, thread))
	{
		/* Get instruction at the head of the ROB */
		uop = x86_rob_head(core, thread);
		assert(x86_uop_exists(uop));
		assert(uop->core == core);
		assert(uop->thread == thread);
		assert(!recover);
		
		/* Mispredicted branch */
		if (x86_cpu_recover_kind == x86_cpu_recover_kind_commit &&
			(uop->flags & X86_UINST_CTRL) && uop->neip != uop->pred_neip)
			recover = 1;
	
		/* Free physical registers */
		assert(!uop->specmode);
		x86_reg_file_commit(uop);
		
		/* Branches update branch predictor and btb */
		if (uop->flags & X86_UINST_CTRL)
		{
			x86_bpred_update(X86_THREAD.bpred, uop);
			x86_bpred_btb_update(X86_THREAD.bpred, uop);
			X86_THREAD.btb_writes++;
		}

		/* Trace cache */
		if (x86_trace_cache_present)
			x86_trace_cache_new_uop(X86_THREAD.trace_cache, uop);
			
		/* Statistics */
		X86_THREAD.last_commit_cycle = arch_x86->cycle;
		X86_THREAD.num_committed_uinst_array[uop->uinst->opcode]++;
		X86_CORE.num_committed_uinst_array[uop->uinst->opcode]++;
		x86_cpu->num_committed_uinst_array[uop->uinst->opcode]++;
		x86_cpu->num_committed_uinst++;
		ctx->inst_count++;
		if (uop->trace_cache)
			X86_THREAD.trace_cache->num_committed_uinst++;
		if (!uop->mop_index)
			x86_cpu->num_committed_inst++;
		if (uop->flags & X86_UINST_CTRL)
		{
			X86_THREAD.num_branch_uinst++;
			X86_CORE.num_branch_uinst++;
			x86_cpu->num_branch_uinst++;
			if (uop->neip != uop->pred_neip)
			{
				X86_THREAD.num_mispred_branch_uinst++;
				X86_CORE.num_mispred_branch_uinst++;
				x86_cpu->num_mispred_branch_uinst++;
			}
		}

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"co\"\n",
				uop->id_in_core, uop->core);
			x86_cpu_uop_trace_list_add(uop);
		}

		/* Retire instruction */
		x86_rob_remove_head(core, thread);
		X86_CORE.rob_reads++;
		X86_THREAD.rob_reads++;
		quant--;

		/* Recover. Functional units are cleared when processor
		 * recovers at commit. */
		if (recover)
		{
			x86_cpu_recover(core, thread);
			x86_fu_release(core);
		}
	}

	/* If context eviction signal is activated and pipeline is empty,
	 * deallocate context. */
	if (ctx->evict_signal && x86_cpu_pipeline_empty(core, thread))
		x86_cpu_evict_context(core, thread);
}


static void x86_cpu_commit_core(int core)
{
	int pass;
	int quant;
	int new;

	/* Commit stage for core */
	switch (x86_cpu_commit_kind)
	{

	case x86_cpu_commit_kind_shared:
		pass = x86_cpu_num_threads;
		quant = x86_cpu_commit_width;
		while (quant && pass)
		{
			X86_CORE.commit_current = (X86_CORE.commit_current + 1) % x86_cpu_num_threads;
			if (x86_cpu_can_commit_thread(core, X86_CORE.commit_current))
			{
				x86_cpu_commit_thread(core, X86_CORE.commit_current, 1);
				quant--;
				pass = x86_cpu_num_threads;
			}
			else
				pass--;
		}
		break;
	
	case x86_cpu_commit_kind_timeslice:
	
		/* look for a not empty VB */
		new = (X86_CORE.commit_current + 1) % x86_cpu_num_threads;
		while (new != X86_CORE.commit_current && !x86_cpu_can_commit_thread(core, new))
			new = (new + 1) % x86_cpu_num_threads;
		
		/* commit new thread */
		X86_CORE.commit_current = new;
		x86_cpu_commit_thread(core, new, x86_cpu_commit_width);
		break;
	
	}
}


void x86_cpu_commit()
{
	int core;

	x86_cpu->stage = "commit";
	X86_CORE_FOR_EACH
		x86_cpu_commit_core(core);
}
