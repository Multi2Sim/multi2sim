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


static int can_fetch(int core, int thread)
{
	struct x86_ctx_t *ctx = THREAD.ctx;

	uint32_t phy_addr;
	uint32_t block;

	/* Context must be running */
	if (!ctx || !x86_ctx_get_status(ctx, x86_ctx_running))
		return 0;
	
	/* Fetch stalled or context evict signal activated */
	if (THREAD.fetch_stall_until >= cpu->cycle || ctx->dealloc_signal)
		return 0;
	
	/* Fetch queue must have not exceeded the limit of stored bytes
	 * to be able to store new macro-instructions. */
	if (THREAD.fetchq_occ >= fetchq_size)
		return 0;
	
	/* If the next fetch address belongs to a new block, cache system
	 * must be accessible to read it. */
	block = THREAD.fetch_neip & ~(THREAD.inst_mod->block_size - 1);
	if (block != THREAD.fetch_block)
	{
		phy_addr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
		if (!mod_can_access(THREAD.inst_mod, phy_addr))
			return 0;
	}
	
	/* We can fetch */
	return 1;
}


/* Execute in the simulation kernel a macro-instruction and create uops.
 * If any of the uops is a control uop, this uop will be the return value of
 * the function. Otherwise, the first decoded uop is returned. */
static struct uop_t *fetch_inst(int core, int thread, int fetch_trace_cache)
{
	struct x86_ctx_t *ctx = THREAD.ctx;

	struct uop_t *uop;
	struct uop_t *ret_uop;

	struct x86_uinst_t *uinst;
	int uinst_count;
	int uinst_index;

	/* Functional simulation */
	THREAD.fetch_eip = THREAD.fetch_neip;
	x86_ctx_set_eip(ctx, THREAD.fetch_eip);
	x86_ctx_execute_inst(ctx);
	THREAD.fetch_neip = THREAD.fetch_eip + isa_inst.size;

	/* Micro-instructions created by the x86 instructions can be found now
	 * in 'x86_uinst_list'. */
	uinst_count = list_count(x86_uinst_list);
	uinst_index = 0;
	ret_uop = NULL;
	while (list_count(x86_uinst_list))
	{
		/* Get uinst from head of list */
		uinst = list_remove_at(x86_uinst_list, 0);

		/* Create uop */
		uop = uop_create();
		uop->uinst = uinst;
		assert(uinst->opcode > 0 && uinst->opcode < x86_uinst_opcode_count);
		uop->flags = x86_uinst_info[uinst->opcode].flags;
		uop->seq = ++cpu->seq;

		uop->ctx = ctx;
		uop->core = core;
		uop->thread = thread;

		uop->mop_count = uinst_count;
		uop->mop_size = isa_inst.size;
		uop->mop_seq = uop->seq - uinst_index;
		uop->mop_index = uinst_index;

		uop->eip = THREAD.fetch_eip;
		uop->in_fetchq = 1;
		uop->fetch_trace_cache = fetch_trace_cache;
		uop->specmode = x86_ctx_get_status(ctx, x86_ctx_specmode);
		uop->fetch_address = THREAD.fetch_address;
		uop->fetch_access = THREAD.fetch_access;
		uop->neip = ctx->regs->eip;
		uop->pred_neip = THREAD.fetch_neip;
		uop->target_neip = isa_target;

		/* Process uop dependences and classify them in integer, floating-point,
		 * flags, etc. */
		rf_count_deps(uop);

		/* Calculate physical address of a memory access */
		if (uop->flags & X86_UINST_MEM)
			uop->phy_addr = mmu_translate(THREAD.ctx->mid,
				uinst->address);

		/* Store x86 macro-instruction and uinst names. This is costly,
		 * do it only if debug is activated. */
		if (esim_debug_file)
		{
			x86_uinst_dump_buf(uinst, uop->name, sizeof(uop->name));
			if (!uinst_index)
				x86_inst_dump_buf(&isa_inst, uop->mop_name,
						sizeof(uop->mop_name));
		}

		/* Select as returned uop */
		if (!ret_uop || (uop->flags & X86_UINST_CTRL))
			ret_uop = uop;

		/* Insert into fetch queue */
		list_add(THREAD.fetchq, uop);
		cpu->fetched++;
		THREAD.fetched++;
		if (fetch_trace_cache)
			THREAD.trace_cache_queue_occ++;

		/* Next uinst */
		uinst_index++;
	}

	/* Increase fetch queue occupancy if instruction does not come from
	 * trace cache, and return. */
	if (ret_uop && !fetch_trace_cache)
		THREAD.fetchq_occ += ret_uop->mop_size;
	return ret_uop;
}


/* Try to fetch instruction from trace cache.
 * Return true if there was a hit and fetching succeeded. */
static int fetch_thread_trace_cache(int core, int thread)
{
	struct uop_t *uop;
	uint32_t eip_branch;  /* next branch address */
	int mpred, hit, mop_count, i;
	uint32_t *mop_array, neip;

	/* No trace cache, no space in the trace cache queue. */
	if (!trace_cache_present)
		return 0;
	if (THREAD.trace_cache_queue_occ >= trace_cache_queue_size)
		return 0;
	
	/* Access BTB, branch predictor, and trace cache */
	eip_branch = bpred_btb_next_branch(THREAD.bpred,
		THREAD.fetch_neip, THREAD.inst_mod->block_size);
	mpred = eip_branch ? bpred_lookup_multiple(THREAD.bpred,
		eip_branch, trace_cache_branch_max) : 0;
	hit = trace_cache_lookup(THREAD.trace_cache, THREAD.fetch_neip, mpred,
		&mop_count, &mop_array, &neip);
	if (!hit)
		return 0;
	
	/* Fetch instruction in trace cache line. */
	for (i = 0; i < mop_count; i++)
	{
		/* If instruction caused context to suspend or finish */
		if (!x86_ctx_get_status(THREAD.ctx, x86_ctx_running))
			break;
		
		/* Insert decoded uops into the trace cache queue. In the simulation,
		 * the uop is inserted into the fetch queue, but its occupancy is not
		 * increased. */
		THREAD.fetch_neip = mop_array[i];
		uop = fetch_inst(core, thread, 1);
		if (!uop)  /* no uop was produced by this macroinst */
			continue;

		/* If instruction is a branch, access branch predictor just in order
		 * to have the necessary information to update it at commit. */
		if (uop->flags & X86_UINST_CTRL)
		{
			bpred_lookup(THREAD.bpred, uop);
			uop->pred_neip = i == mop_count - 1 ? neip :
				mop_array[i + 1];
		}
	}

	/* Set next fetch address as returned by the trace cache, and exit. */
	THREAD.fetch_neip = neip;
	return 1;
}


static void fetch_thread(int core, int thread)
{
	struct x86_ctx_t *ctx = THREAD.ctx;
	struct uop_t *uop;

	uint32_t phy_addr;
	uint32_t block;
	uint32_t target;

	int taken;

	/* Try to fetch from trace cache first */
	if (fetch_thread_trace_cache(core, thread))
		return;
	
	/* If new block to fetch is not the same as the previously fetched (and stored)
	 * block, access the instruction cache. */
	block = THREAD.fetch_neip & ~(THREAD.inst_mod->block_size - 1);
	if (block != THREAD.fetch_block)
	{
		phy_addr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
		THREAD.fetch_block = block;
		THREAD.fetch_address = phy_addr;
		THREAD.fetch_access = mod_access(THREAD.inst_mod, mod_entry_cpu,
			mod_access_read, phy_addr, NULL, NULL, NULL);
		THREAD.btb_reads++;

		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(phy_addr, mmu_access_execute);
	}

	/* Fetch all instructions within the block up to the first predict-taken branch. */
	while ((THREAD.fetch_neip & ~(THREAD.inst_mod->block_size - 1)) == block)
	{
		/* If instruction caused context to suspend or finish */
		if (!x86_ctx_get_status(ctx, x86_ctx_running))
			break;
	
		/* If fetch queue full, stop fetching */
		if (THREAD.fetchq_occ >= fetchq_size)
			break;
		
		/* Insert macro-instruction into the fetch queue. Since the macro-instruction
		 * information is only available at this point, we use it to decode
		 * instruction now and insert uops into the fetch queue. However, the
		 * fetch queue occupancy is increased with the macro-instruction size. */
		uop = fetch_inst(core, thread, 0);
		if (!isa_inst.size)  /* isa_inst invalid - no forward progress in loop */
			break;
		if (!uop)  /* no uop was produced by this macro-instruction */
			continue;

		/* Instruction detected as branches by the BTB are checked for branch
		 * direction in the branch predictor. If they are predicted taken,
		 * stop fetching from this block and set new fetch address. */
		if (uop->flags & X86_UINST_CTRL)
		{
			target = bpred_btb_lookup(THREAD.bpred, uop);
			taken = target && bpred_lookup(THREAD.bpred, uop);
			if (taken)
			{
				THREAD.fetch_neip = target;
				uop->pred_neip = target;
				break;
			}
		}
	}
}




static void fetch_core(int core)
{
	int thread;

	switch (cpu_fetch_kind)
	{

	case cpu_fetch_kind_shared:
	{
		/* Fetch from all threads */
		FOREACH_THREAD
		{
			if (can_fetch(core, thread))
				fetch_thread(core, thread);
		}
		break;
	}

	case cpu_fetch_kind_timeslice:
	{
		/* Round-robin fetch */
		FOREACH_THREAD
		{
			CORE.fetch_current = (CORE.fetch_current + 1) % cpu_threads;
			if (can_fetch(core, CORE.fetch_current))
			{
				fetch_thread(core, CORE.fetch_current);
				break;
			}
		}
		break;
	}
	
	case cpu_fetch_kind_switchonevent:
	{
		int must_switch;
		int new;

		/* If current thread is stalled, it means that we just switched to it.
		 * No fetching and no switching either. */
		thread = CORE.fetch_current;
		if (THREAD.fetch_stall_until >= cpu->cycle)
			break;

		/* Switch thread if:
		 * - Quantum expired for current thread.
		 * - Long latency instruction is in progress. */
		must_switch = !can_fetch(core, thread);
		must_switch = must_switch || cpu->cycle - CORE.fetch_switch_when >
			cpu_thread_quantum + cpu_thread_switch_penalty;
		must_switch = must_switch ||
			eventq_longlat(core, thread);

		/* Switch thread */
		if (must_switch)
		{
			/* Find a new thread to switch to */
			for (new = (thread + 1) % cpu_threads; new != thread;
				new = (new + 1) % cpu_threads)
			{
				/* Do not choose it if it is not eligible for fetching */
				if (!can_fetch(core, new))
					continue;
					
				/* Choose it if we need to switch */
				if (must_switch)
					break;

				/* Do not choose it if it is unfair.
				 * FIXME: more meaningful fairness policy needed here. */
				if (ITHREAD(new).committed > THREAD.committed + 100000)
					continue;

				/* Choose it if it is not stalled */
				if (!eventq_longlat(core, new))
					break;
			}
				
			/* Thread switch successful? */
			if (new != thread)
			{
				CORE.fetch_current = new;
				CORE.fetch_switch_when = cpu->cycle;
				ITHREAD(new).fetch_stall_until = cpu->cycle + cpu_thread_switch_penalty - 1;
			}
		}

		/* Fetch */
		if (can_fetch(core, CORE.fetch_current))
			fetch_thread(core, CORE.fetch_current);
		break;
	}

	default:
		
		panic("%s: wrong fetch policy", __FUNCTION__);
	}
}


void cpu_fetch()
{
	int core;
	cpu->stage = "fetch";
	FOREACH_CORE
		fetch_core(core);
}

