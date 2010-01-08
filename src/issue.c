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

static int issue_sq(int core, int thread, int quant)
{
	struct uop_t *store;
	struct lnlist_t *sq = THREAD.sq;

	lnlist_head(sq);
	while (!lnlist_eol(sq) && quant) {
		
		/* Get store */
		store = lnlist_get(sq);
		assert(store->flags & FSTORE);

		/* Check that it can issue */
		if (!store->ready && !phregs_ready(store))
			break;
		store->ready = 1;
		if (store->in_rob)
			break;
		if (!cache_system_can_access(core, thread, cache_kind_data, store->mem_phaddr))
			break;

		/* Access */
		cache_system_write(core, thread, cache_kind_data,
			store->mem_phaddr, &store->data_witness);

		/* Schedule inst in Event Queue */
		assert(!store->in_eventq);
		store->issued = TRUE;
		store->issue_when = sim_cycle;
		store->when = sim_cycle + 1;
		eventq_insert(CORE.eventq, store);
		store->in_eventq = TRUE;
		
		/* Remove it from SQ */
		sq_remove(core, thread);

		/* Instruction issued */
		ptrace_new_stage(store, ptrace_memory);
		THREAD.issued++;
		p->issued++;
		quant--;
	}
	return quant;
}


static int issue_lq(int core, int thread, int quant)
{
	struct lnlist_t *lq = THREAD.lq;
	struct uop_t *load;
	
	/* Find instruction to issue */
	lnlist_head(lq);
	while (!lnlist_eol(lq) && quant) {
		
		/* Get element from LQ.
		 * If it is not ready, go to the next one */
		load = lnlist_get(lq);
		if (!load->ready && !phregs_ready(load)) {
			lnlist_next(lq);
			continue;
		}
		load->ready = 1;
		if (!cache_system_can_access(core, thread, cache_kind_data, load->mem_phaddr)) {
			lnlist_next(lq);
			continue;
		}
	
		/* Ok, we can issue. Access the data TLB and cache */
		cache_system_read(core, thread, cache_kind_data,
			load->mem_phaddr, &load->data_witness);
		
		/* Remove 'load' from LQ */
		lq_remove(core, thread);
		
		/* Schedule 'load' in Event Queue for next cycle.
		 * Writeback stage will check if it is really completed. */
		assert(!load->in_eventq);
		load->issued = TRUE;
		load->issue_when = sim_cycle;
		load->when = sim_cycle + 1;
		eventq_insert(CORE.eventq, load);
		load->in_eventq = TRUE;
		
		/* Instruction issued */
		ptrace_new_stage(load, ptrace_memory);
		THREAD.issued++;
		p->issued++;
		quant--;
	}
	
	return quant;
}


static int issue_iq(int core, int thread, int quant)
{
	struct lnlist_t *iq = THREAD.iq;
	struct uop_t *uop;
	int lat;
	
	/* Find instruction to issue */
	lnlist_head(iq);
	while (!lnlist_eol(iq) && quant) {
		
		/* Get element from IQ */
		uop = lnlist_get(iq);
		assert(uop_exists(uop));
		assert(!(uop->flags & FMEM));
		if (!uop->ready && !phregs_ready(uop)) {
			lnlist_next(iq);
			continue;
		}
		uop->ready = 1;  /* avoid next call to 'phregs_ready' */
		
		/* If inst does not require fu, one cycle latency.
		 * Otherwise, try to reserve the corresponding fu. */
		if (!uop->fu_class) {
			lat = 1;
		} else {
			lat = fu_reserve(CORE.fu, uop->fu_class);
			if (!lat) {
				lnlist_next(iq);
				continue;
			}
		}
		
		/* Instruction was issued to the corresponding fu.
		 * Remove it from IQ */
		iq_remove(core, thread);
		
		/* Schedule inst in Event Queue */
		assert(!uop->in_eventq);
		assert(lat > 0);
		uop->issued = TRUE;
		uop->issue_when = sim_cycle;
		uop->when = sim_cycle + lat;
		eventq_insert(CORE.eventq, uop);
		
		/* Instruction issued */
		ptrace_new_stage(uop, ptrace_execution);
		THREAD.issued++;
		p->issued++;
		quant--;
	}
	
	return quant;
}


static int issue_thread_iq(int core, int thread, int quant)
{
	quant = issue_sq(core, thread, quant);
	quant = issue_lq(core, thread, quant);
	return quant;
}


static int issue_thread_lsq(int core, int thread, int quant)
{
	quant = issue_iq(core, thread, quant);
	return quant;
}


void issue_core(int core)
{
	int skip, quant;

	switch (p_issue_kind) {
	
	case p_issue_kind_shared:
		
		/* Issue IQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);

		/* Issue LSQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);
		break;
	
	case p_issue_kind_timeslice:
		
		/* Issue IQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant == p_issue_width);

		/* Issue LSQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant == p_issue_width);
		break;
	}
}


void p_issue()
{
	int core;
	p->stage = "issue";
	FOREACH_CORE
		issue_core(core);
}

