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


static int issue_sq(int core, int thread, int quant)
{
	struct uop_t *store;
	struct linked_list_t *sq = THREAD.sq;

	/* Process SQ */
	linked_list_head(sq);
	while (!linked_list_is_end(sq) && quant)
	{
		/* Get store */
		store = linked_list_get(sq);
		assert(store->uinst->opcode == x86_uinst_store);

		/* Only committed stores issue */
		if (store->in_rob)
			break;

		/* Check that memory system entry is ready */
		if (!mod_can_access(THREAD.data_mod, store->phy_addr))
			break;

		/* Remove store from store queue */
		sq_remove(core, thread);

		/* Issue store */
		mod_access(THREAD.data_mod, mod_entry_cpu, mod_access_write,
			store->phy_addr, NULL, CORE.eventq, store);

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_eventq" to
		 * prevent the uop from being freed. */
		store->in_eventq = 1;
		store->issued = 1;
		store->issue_when = cpu->cycle;
	
		/* Instruction issued */
		CORE.issued[store->uinst->opcode]++;
		CORE.lsq_reads++;
		CORE.rf_int_reads += store->ph_int_idep_count;
		CORE.rf_fp_reads += store->ph_fp_idep_count;
		THREAD.issued[store->uinst->opcode]++;
		THREAD.lsq_reads++;
		THREAD.rf_int_reads += store->ph_int_idep_count;
		THREAD.rf_fp_reads += store->ph_fp_idep_count;
		cpu->issued[store->uinst->opcode]++;
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(store->phy_addr, mmu_access_write);

		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_issue=1, in_lsq=0, issued=1\n",
			store->core, (long long unsigned) store->di_seq);
	}
	return quant;
}


static int issue_lq(int core, int thread, int quant)
{
	struct linked_list_t *lq = THREAD.lq;
	struct uop_t *load;

	/* Debug */
	if (esim_debug_file)
		uop_lnlist_check_if_ready(lq);
	
	/* Process lq */
	linked_list_head(lq);
	while (!linked_list_is_end(lq) && quant)
	{
		/* Get element from load queue. If it is not ready, go to the next one */
		load = linked_list_get(lq);
		if (!load->ready && !rf_ready(load))
		{
			linked_list_next(lq);
			continue;
		}
		load->ready = 1;

		/* Check that memory system is accessible */
		if (!mod_can_access(THREAD.data_mod, load->phy_addr))
		{
			linked_list_next(lq);
			continue;
		}

		/* Remove from load queue */
		assert(load->uinst->opcode == x86_uinst_load);
		lq_remove(core, thread);

		/* Access memory system */
		mod_access(THREAD.data_mod, mod_entry_cpu, mod_access_read,
			load->phy_addr, NULL, CORE.eventq, load);

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_eventq" to
		 * prevent the uop from being freed. */
		load->in_eventq = 1;
		load->issued = 1;
		load->issue_when = cpu->cycle;
		
		/* Instruction issued */
		CORE.issued[load->uinst->opcode]++;
		CORE.lsq_reads++;
		CORE.rf_int_reads += load->ph_int_idep_count;
		CORE.rf_fp_reads += load->ph_fp_idep_count;
		THREAD.issued[load->uinst->opcode]++;
		THREAD.lsq_reads++;
		THREAD.rf_int_reads += load->ph_int_idep_count;
		THREAD.rf_fp_reads += load->ph_fp_idep_count;
		cpu->issued[load->uinst->opcode]++;
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(load->phy_addr, mmu_access_read);

		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_issue=1, in_lsq=0, issued=1\n",
			load->core, (long long unsigned) load->di_seq);
	}
	
	return quant;
}


static int issue_iq(int core, int thread, int quant)
{
	struct linked_list_t *iq = THREAD.iq;
	struct uop_t *uop;
	int lat;

	/* Debug */
	if (esim_debug_file)
		uop_lnlist_check_if_ready(iq);
	
	/* Find instruction to issue */
	linked_list_head(iq);
	while (!linked_list_is_end(iq) && quant) {
		
		/* Get element from IQ */
		uop = linked_list_get(iq);
		assert(uop_exists(uop));
		assert(!(uop->flags & X86_UINST_MEM));
		if (!uop->ready && !rf_ready(uop)) {
			linked_list_next(iq);
			continue;
		}
		uop->ready = 1;  /* avoid next call to 'rf_ready' */
		
		/* Run the instruction in its corresponding functional unit.
		 * If the instruction does not require a functional unit, 'fu_reserve'
		 * returns 1 cycle latency. If there is no functional unit available,
		 * 'fu_reserve' returns 0. */
		lat = fu_reserve(uop);
		if (!lat) {
			linked_list_next(iq);
			continue;
		}
		
		/* Instruction was issued to the corresponding fu.
		 * Remove it from IQ */
		iq_remove(core, thread);
		
		/* Schedule inst in Event Queue */
		assert(!uop->in_eventq);
		assert(lat > 0);
		uop->issued = 1;
		uop->issue_when = cpu->cycle;
		uop->when = cpu->cycle + lat;
		eventq_insert(CORE.eventq, uop);
		
		/* Instruction issued */
		CORE.issued[uop->uinst->opcode]++;
		CORE.iq_reads++;
		CORE.rf_int_reads += uop->ph_int_idep_count;
		CORE.rf_fp_reads += uop->ph_fp_idep_count;
		THREAD.issued[uop->uinst->opcode]++;
		THREAD.iq_reads++;
		THREAD.rf_int_reads += uop->ph_int_idep_count;
		THREAD.rf_fp_reads += uop->ph_fp_idep_count;
		cpu->issued[uop->uinst->opcode]++;
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

	switch (cpu_issue_kind) {
	
	case cpu_issue_kind_shared:
		
		/* Issue LSQs */
		quant = cpu_issue_width;
		skip = cpu_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % cpu_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);

		/* Issue IQs */
		quant = cpu_issue_width;
		skip = cpu_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % cpu_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant);
		
		break;
	
	case cpu_issue_kind_timeslice:
		
		/* Issue LSQs */
		quant = cpu_issue_width;
		skip = cpu_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % cpu_threads;
			quant = issue_thread_lsq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant == cpu_issue_width);

		/* Issue IQs */
		quant = cpu_issue_width;
		skip = cpu_threads;
		do {
			CORE.issue_current = (CORE.issue_current + 1) % cpu_threads;
			quant = issue_thread_iq(core, CORE.issue_current, quant);
			skip--;
		} while (skip && quant == cpu_issue_width);

		break;
	}
}


void cpu_issue()
{
	int core;
	cpu->stage = "issue";
	FOREACH_CORE
		issue_core(core);
}

