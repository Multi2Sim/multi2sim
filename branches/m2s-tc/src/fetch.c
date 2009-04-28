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

	/* Context must be running */
	if (!THREAD.ctx || !ctx_get_status(THREAD.ctx, ctx_running))
		return FALSE;
	
	/* Fetch stage must not be stalled */
	if (THREAD.fetch_stall)
		return FALSE;
	
	/* Fetch queue must have space */
	if (!fetchq_can_insert(core, thread))
		return FALSE;
	
	/* Fetch policy - Predictive Data Gating */
	if (p_fetch_policy == p_fetch_policy_pdg && THREAD.lmpred_misses > PDG_THRESHOLD)
		return FALSE;

	/* If the next fetch address belongs to a new block, cache system
	 * must be accessible to read it. */
	block = THREAD.fetch_neip & ~(cache_block_size - 1);
	if (block != THREAD.fetch_block) {
		phaddr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
		if (!cache_system_can_access(core, thread, cache_kind_inst, phaddr))
			return FALSE;
	}
	
	/* We can fetch */
	return TRUE;
}


/* Take instruction 'isa_inst' and insert uops into the fetch queue. Their access
 * ID is 'fetch_access'. Return the number of fetched uops. */
static int fetch_inst(int core, int thread)
{
	struct list_t *fetchq = THREAD.fetchq;
	struct ctx_t *ctx = THREAD.ctx;
	int count, i, uop_count;
	struct uop_t *uop;

	/* Split fetched instruction into uops */
	uop_count = 0;
	count = list_count(fetchq);
	uop_decode(&isa_inst, fetchq);

	/* Check that at least one instruction was inserted */
	if (isa_inst.opcode && count == list_count(fetchq) && !ctx_get_status(ctx, ctx_specmode)) {
		fprintf(stderr, "isa_inst: ");
		x86_inst_dump(&isa_inst, stderr);
		fprintf(stderr, "\n");
		panic("no uop added to fetch queue");
	}

	/* Decode rest of instructions */
	for (i = count; i < list_count(fetchq); i++) {
		uop = list_get(fetchq, i);
		uop->ctx = ctx;
		uop->core = core;
		uop->thread = thread;
		uop->seq = ++p->seq;
		uop->eip = THREAD.fetch_eip;
		uop->size = isa_inst.size;
		uop->in_fetchq = TRUE;
		uop->specmode = ctx_get_status(ctx, ctx_specmode);

		/* If uop comes from trace cache, it is ready. Otherwise, wait for
		 * instruction cache to finish access. */
		if (THREAD.fetch_tc_uops) {
			uop->fetch_access = 0;
			THREAD.fetch_tc_uops--;
			p->tc_fetched++;
		} else {
			uop->fetch_access = THREAD.fetch_access;
		}

		/* For control instructions */
		if (uop->flags & FCTRL) {
			uop->neip = ctx->regs->eip;
			uop->pred_neip = bpred_lookup(uop);
			uop->mispred = uop->pred_neip != uop->neip && !uop->specmode;
			uop->taken = uop->neip != uop->eip + isa_inst.size;
			THREAD.fetch_neip = uop->pred_neip;
			if (!uop->specmode) {
				p->branches++;
				if (uop->taken)
					p->taken_branches++;
				if (uop->mispred)
					p->mispred++;
			}
		}

		/* Memory access uops */
		if (uop->flags & FMEM) {
			uop->mem_vtladdr = ctx->mem->last_address;
			uop->mem_phaddr = mmu_translate(THREAD.ctx->mid, ctx->mem->last_address);
		}

		/* Loads for pdg fetch policy */
		if (p_fetch_policy == p_fetch_policy_pdg && (uop->flags & FLOAD)) {
			uop->lmpred_idx = uop->eip % PDG_LMPRED_SIZE;
			uop->lmpred_miss = THREAD.lmpred[uop->lmpred_idx] < 2;
			if (uop->lmpred_miss)
				THREAD.lmpred_misses++;
		}

		/* New uop */
		uop_count++;
		ptrace_new_uop(uop);
		ptrace_new_stage(uop, ptrace_fetch);
		p->fetched++;
		THREAD.fetched++;
	}

	return uop_count;
}


static int fetch_thread(int core, int thread, int quant)
{
	struct ctx_t *ctx = THREAD.ctx;
	uint32_t phaddr, block, pattern;
	int tc_uops, uop_count;

	/* Fetch instructions from a thread's instruction cache until one of these
	 * conditions is fulfilled:
	 *   - 'quant' uops have been fetched.
	 *   - there is a taken branch with a target outside the fetched block
	 * If there is a hit in the trace cache, fetch the whole trace cache line. */

	/* Look up trace cache */
	pattern = bpred_get_trace(core, thread, THREAD.fetch_neip, tc_branches);
	tc_uops = tc_find_trace(core, thread, THREAD.fetch_neip, pattern);
	THREAD.fetch_tc_uops = MAX(THREAD.fetch_tc_uops, tc_uops);

	/* Fetch instructions */
	while (quant || THREAD.fetch_tc_uops) {

		/* Check if we can fetch */
		if (!can_fetch(core, thread))
			break;

		/* Access instruction cache */
		block = THREAD.fetch_neip & ~(cache_block_size - 1);
		if (block != THREAD.fetch_block) {
			phaddr = mmu_translate(THREAD.ctx->mid, THREAD.fetch_neip);
			THREAD.fetch_block = block;
			THREAD.fetch_access = cache_system_read(core, thread,
				cache_kind_inst, phaddr, NULL);
		}

		/* Functional simulation */
		THREAD.fetch_eip = THREAD.fetch_neip;
		ctx_set_eip(ctx, THREAD.fetch_eip);
		ctx_execute_inst(ctx);
		THREAD.fetch_neip = THREAD.fetch_eip + isa_inst.size;

		/* New context spawned */
		if (ke->context_count > p->context_map_count)
			p_context_map_update();

		/* Insert uops into fetch queue */
		uop_count = fetch_inst(core, thread);
		uop_count = MAX(uop_count, 1);
		quant = MAX(0, quant - uop_count);

	}

	/* Return the unused fetch slots */
	return quant;
}


void fetch_core(int core)
{
	int thread;
	FOREACH_THREAD
		fetch_thread(core, thread, p_fetch_width);
}


void p_fetch()
{
	int core;
	p->stage = "fetch";
	FOREACH_CORE
		fetch_core(core);
}
