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

	/* Process SQ */
	lnlist_head(sq);
	while (!lnlist_eol(sq) && quant) {
		
		/* Get store */
		store = lnlist_get(sq);
		assert(store->flags & FSTORE);

		/* Check that it can issue */
		if (store->in_rob)
			break;
		if (!cache_system_can_access(core, thread, cache_kind_data,
			cache_access_kind_write, store->mem_phaddr))
			break;

		/* Store can be issued. */
		sq_remove(core, thread);
		cache_system_write(core, thread, cache_kind_data,
			store->mem_phaddr, CORE.eventq, store);

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_eventq" to
		 * prevent the uop from being freed. */
		store->in_eventq = 1;
		store->issued = 1;
		store->issue_when = sim_cycle;
	
		/* Instruction issued */
		CORE.issued[store->uop]++;
		CORE.lsq_reads++;
		CORE.rf_int_reads += store->ph_int_idep_count;
		CORE.rf_fp_reads += store->ph_fp_idep_count;
		THREAD.issued[store->uop]++;
		THREAD.lsq_reads++;
		THREAD.rf_int_reads += store->ph_int_idep_count;
		THREAD.rf_fp_reads += store->ph_fp_idep_count;
		p->issued[store->uop]++;
		quant--;
		
		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_issue=1, in_lsq=0, issued=1\n",
			store->core, (long long unsigned) store->di_seq);
	}
	return quant;
}


static int issue_lq(int core, int thread, int quant)
{
	struct lnlist_t *lq = THREAD.lq;
	struct uop_t *load;

	/* Debug */
	if (esim_debug_file)
		uop_lnlist_check_if_ready(lq);
	
	/* Process lq */
	lnlist_head(lq);
	while (!lnlist_eol(lq) && quant) {
		
		/* Get element from LQ.
		 * If it is not ready, go to the next one */
		load = lnlist_get(lq);
		if (!load->ready && !rf_ready(load)) {
			lnlist_next(lq);
			continue;
		}
		load->ready = 1;
		if (!cache_system_can_access(core, thread, cache_kind_data,
			cache_access_kind_read, load->mem_phaddr))
		{
			lnlist_next(lq);
			continue;
		}

		/* Load can be issued. Remove it from lq.
		 * Access data tlb and cache. */
		lq_remove(core, thread);
		cache_system_read(core, thread, cache_kind_data,
			load->mem_phaddr, CORE.eventq, load);

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_eventq" to
		 * prevent the uop from being freed. */
		load->in_eventq = 1;
		load->issued = 1;
		load->issue_when = sim_cycle;
		
		/* Instruction issued */
		CORE.issued[load->uop]++;
		CORE.lsq_reads++;
		CORE.rf_int_reads += load->ph_int_idep_count;
		CORE.rf_fp_reads += load->ph_fp_idep_count;
		THREAD.issued[load->uop]++;
		THREAD.lsq_reads++;
		THREAD.rf_int_reads += load->ph_int_idep_count;
		THREAD.rf_fp_reads += load->ph_fp_idep_count;
		p->issued[load->uop]++;
		quant--;
		
		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_issue=1, in_lsq=0, issued=1\n",
			load->core, (long long unsigned) load->di_seq);
	}
	
	return quant;
}


static int issue_iq(int core, int thread, int quant)
{
	struct lnlist_t *iq = THREAD.iq;
	struct uop_t *uop;
	int lat;

	/* Debug */
	if (esim_debug_file)
		uop_lnlist_check_if_ready(iq);
	
	/* Find instruction to issue */
	lnlist_head(iq);
	while (!lnlist_eol(iq) && quant) {
		
		/* Get element from IQ */
		uop = lnlist_get(iq);
		assert(uop_exists(uop));
		assert(!(uop->flags & FMEM));
		if (!uop->ready && !rf_ready(uop)) {
			lnlist_next(iq);
			continue;
		}
		uop->ready = 1;  /* avoid next call to 'rf_ready' */
		
		/* If inst does not require fu, one cycle latency.
		 * Otherwise, try to reserve the corresponding fu. */
		if (!uop->fu_class) {
			lat = 1;
		} else {
			lat = fu_reserve(uop);
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
		uop->issued = 1;
		uop->issue_when = sim_cycle;
		uop->when = sim_cycle + lat;
		eventq_insert(CORE.eventq, uop);
		
		/* Instruction issued */
		CORE.issued[uop->uop]++;
		CORE.iq_reads++;
		CORE.rf_int_reads += uop->ph_int_idep_count;
		CORE.rf_fp_reads += uop->ph_fp_idep_count;
		THREAD.issued[uop->uop]++;
		THREAD.iq_reads++;
		THREAD.rf_int_reads += uop->ph_int_idep_count;
		THREAD.rf_fp_reads += uop->ph_fp_idep_count;
		p->issued[uop->uop]++;
		quant--;

		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_issue=1, in_iq=0, issued=1\n",
			uop->core, (long long unsigned) uop->di_seq);
	}
	
	return quant;
}


static int issue_thread_lsq(int core, int thread, int quant)
{
	quant = issue_lq(core, thread, quant);
	quant = issue_sq(core, thread, quant);
	return quant;
}


static int issue_thread_iq(int core, int thread, int quant)
{
	quant = issue_iq(core, thread, quant);
	return quant;
}


void issue_core(int core)
{
	int skip, quant;

	switch (p_issue_kind) {
	
	case p_issue_kind_shared:
		
		/* Issue LSQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);

		/* Issue IQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);
		
		break;
	
	case p_issue_kind_timeslice:
		
		/* Issue LSQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant == p_issue_width);

		/* Issue IQs */
		quant = p_issue_width;
		skip = p_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % p_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
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

