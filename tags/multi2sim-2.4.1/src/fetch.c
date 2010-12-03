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


static int can_fetch(int core, int thread)
{
	uint32_t phaddr, block;
	struct ctx_t *ctx = THREAD.ctx;

	/* Context must be running */
	if (!ctx || !ctx_get_status(ctx, ctx_running))
		return 0;
	
	/* Fetch stage stalled or context evict signal activated */
	if (THREAD.fetch_stall || ctx->dealloc_signal)
		return 0;
	
	/* Fetch queue must have not exceeded the limit of stored bytes
	 * to be able to store new macroinstructions. */
	if (THREAD.fetchq_occ >= fetchq_size)
		return 0;
	
	/* If the next fetch address belongs to a new block, cache system
	 * must be accessible to read it. */
	block = THREAD.fetch_neip & ~(THREAD.fetch_bsize - 1);
	if (block != THREAD.fetch_block) {
		phaddr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
		if (!cache_system_can_access(core, thread, cache_kind_inst,
			cache_access_kind_read, phaddr))
			return 0;
	}
	
	/* We can fetch */
	return 1;
}


/* Execute in the simulation kernel a macroinstruction.
 * Decode instruction 'isa_inst' and insert uops into the fetch queue.
 * As 'uop_decode' does, return the first uop from the fetched macroinst,
 * or the last control uop if there was any. */
static struct uop_t *fetch_inst(int core, int thread, int fetch_tcache)
{
	struct ctx_t *ctx = THREAD.ctx;
	struct list_t *fetchq = THREAD.fetchq;
	int count, newcount, i;
	struct uop_t *uop, *ret;

	/* Functional simulation */
	THREAD.fetch_eip = THREAD.fetch_neip;
	ctx_set_eip(ctx, THREAD.fetch_eip);
	ctx_execute_inst(ctx);
	THREAD.fetch_neip = THREAD.fetch_eip + isa_inst.size;

	/* Split macroinstruction into uops stored in list. */
	count = list_count(fetchq);
	ret = uop_decode(fetchq);
	newcount = list_count(fetchq);

	/* Check that at least one instruction was inserted */
	/*if (isa_inst.opcode && count == newcount && !ctx_get_status(ctx, ctx_specmode)) {
		fprintf(stderr, "isa_inst: ");
		x86_inst_dump(&isa_inst, stderr);
		fprintf(stderr, "\n");
		panic("no uop added to fetch queue");
	}*/

	/* Update inserted uop fields */
	for (i = count; i < newcount; i++) {
		uop = list_get(fetchq, i);
		assert(uop);
		uop->seq = ++p->seq;
		uop->mop_seq = p->seq - i + count;
		uop->mop_size = isa_inst.size;
		uop->mop_count = newcount - count;
		uop->mop_index = i - count;
		uop->ctx = ctx;
		uop->core = core;
		uop->thread = thread;
		uop->eip = THREAD.fetch_eip;
		uop->in_fetchq = 1;
		uop->fetch_tcache = fetch_tcache;
		uop->specmode = ctx_get_status(ctx, ctx_specmode);
		uop->fetch_access = THREAD.fetch_access;
		uop->neip = ctx->regs->eip;
		uop->pred_neip = THREAD.fetch_neip;
		uop->target_neip = isa_target;

		/* Count number of logical/physical input/output
		 * dependences. Physical odeps might be different than
		 * logical odeps. */
		rf_count_deps(uop);

		/* Memory access uops */
		if (uop->flags & FMEM) {
			uop->mem_vtladdr = ctx->mem->last_address;
			uop->mem_phaddr = mmu_translate(THREAD.ctx->mid, ctx->mem->last_address);
		}

		/* Store macroinstruction/uop names */
		uop_dump_buf(uop, uop->name, sizeof(uop->name));
		if (i == count)
			x86_inst_dump_buf(&isa_inst, uop->mop_name,
				sizeof(uop->mop_name));

		/* New uop */
		p->fetched++;
		THREAD.fetched++;
		if (fetch_tcache)
			THREAD.tcacheq_occ++;
	}

	/* Increase fetch queue occupancy if instruction does not come from
	 * trace cache, and return. */
	if (ret && !fetch_tcache)
		THREAD.fetchq_occ += ret->mop_size;
	return ret;
}


/* Try to fetch instruction from trace cache.
 * Return true if there was a hit and fetching succeeded. */
static int fetch_thread_tcache(int core, int thread)
{
	struct uop_t *uop;
	uint32_t eip_branch;  /* next branch address */
	int mpred, hit, mop_count, i;
	uint32_t *mop_array, neip;

	/* No trace cache, no space in the trace cache queue. */
	if (!tcache_present)
		return 0;
	if (THREAD.tcacheq_occ >= tcache_queue_size)
		return 0;
	
	/* Access BTB, branch predictor, and trace cache */
	eip_branch = bpred_btb_next_branch(THREAD.bpred,
		THREAD.fetch_neip, THREAD.fetch_bsize);
	mpred = eip_branch ? bpred_lookup_multiple(THREAD.bpred,
		eip_branch, tcache_branch_max) : 0;
	hit = tcache_lookup(THREAD.tcache, THREAD.fetch_neip, mpred,
		&mop_count, &mop_array, &neip);
	if (!hit)
		return 0;
	
	/* Fetch instruction in trace cache line. */
	for (i = 0; i < mop_count; i++) {
		
		/* If instruction caused context to suspend or finish */
		if (!ctx_get_status(THREAD.ctx, ctx_running))
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
		if (uop->flags & FCTRL) {
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
	struct ctx_t *ctx = THREAD.ctx;
	struct uop_t *uop;
	uint32_t block, phaddr, target;
	int taken;

	/* Try to fetch from trace cache first */
	if (fetch_thread_tcache(core, thread))
		return;

	/* If new block to fetch is not the same as the previously fetched (and stored)
	 * block, access the instruction cache. */
	block = THREAD.fetch_neip & ~(THREAD.fetch_bsize - 1);
	if (block != THREAD.fetch_block) {
		phaddr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
		THREAD.fetch_block = block;
		THREAD.fetch_access = cache_system_read(core, thread,
			cache_kind_inst, phaddr, NULL, NULL);
		THREAD.btb_reads++;
	}

	/* Fetch all instructions within the block up to the first predict-taken branch. */
	while ((THREAD.fetch_neip & ~(THREAD.fetch_bsize - 1)) == block) {
		
		/* If instruction caused context to suspend or finish */
		if (!ctx_get_status(ctx, ctx_running))
			break;
		
		/* Insert macroinstruction into the fetch queue. Since the macroinstruction
		 * information is only available at this point, we use it to decode
		 * instruction now and insert uops into the fetchq. However, the fetchq
		 * occupancy is increased with as many bytes as macroinst size. */
		uop = fetch_inst(core, thread, 0);
		if (!isa_inst.size)  /* isa_inst invalid - no forward progress in loop */
			break;
		if (!uop)  /* no uop was produced by this macroinst */
			continue;

		/* Instruction detected as branches by the BTB are checked for branch
		 * direction in the branch predictor. If they are predicted taken,
		 * stop fetching from this block and set new fetch address. */
		if (uop->flags & FCTRL) {
			target = bpred_btb_lookup(THREAD.bpred, uop);
			taken = target && bpred_lookup(THREAD.bpred, uop);
			if (taken) {
				THREAD.fetch_neip = target;
				uop->pred_neip = target;
				break;
			}
		}
	}
}




static void fetch_core(int core)
{
	int thread, new;
	int must_switch;

	switch (p_fetch_kind) {

	/* Fetch from all threads */
	case p_fetch_kind_shared:
		FOREACH_THREAD
			if (can_fetch(core, thread))
				fetch_thread(core, thread);
		break;

	case p_fetch_kind_timeslice:
		FOREACH_THREAD {
			CORE.fetch_current = (CORE.fetch_current + 1) % p_threads;
			if (can_fetch(core, CORE.fetch_current)) {
				fetch_thread(core, CORE.fetch_current);
				break;
			}
		}
		break;
	
	case p_fetch_kind_switchonevent:
		
		/* Check for context switch */
		thread = CORE.fetch_current;
		must_switch = !ctx_get_status(THREAD.ctx, ctx_running);
		if (sim_cycle - CORE.fetch_switch > p_thread_quantum ||  /* Quantum expired */
			eventq_longlat(core, thread) ||  /* Long latency instruction */
			must_switch)  /* Current context is suspended */
		{
			/* Find a new thread to switch to */
			for (new = (thread + 1) % p_threads; new != thread;
				new = (new + 1) % p_threads)
			{
				/* Do not choose it if it is not eligible for fetching */
				if (!can_fetch(core, new))
					continue;
					
				/* Choose it if we need to switch */
				if (must_switch)
					break;

				/* Do not choose it if it is unfair */
				if (ITHREAD(new).committed > THREAD.committed + 100000)
					continue;

				/* Choose it if it is not stalled */
				if (!eventq_longlat(core, new))
					break;
			}
				
			/* if thread switch successful */
			if (new != thread) {
				CORE.fetch_current = new;
				CORE.fetch_switch = sim_cycle;
				ITHREAD(new).fetch_stall = p_thread_switch_penalty;
			}
		}

		/* Fetch */
		if (can_fetch(core, CORE.fetch_current))
			fetch_thread(core, CORE.fetch_current);
		break;
	}
}


void p_fetch()
{
	int core;
	p->stage = "fetch";
	FOREACH_CORE
		fetch_core(core);
}

