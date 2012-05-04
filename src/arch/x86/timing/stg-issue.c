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

#include <x86-timing.h>


static int x86_cpu_issue_sq(int core, int thread, int quant)
{
	struct x86_uop_t *store;
	struct linked_list_t *sq = X86_THREAD.sq;

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
		if (!mod_can_access(X86_THREAD.data_mod, store->phy_addr))
			break;

		/* Remove store from store queue */
		x86_sq_remove(core, thread);

		/* Issue store */
		mod_access(X86_THREAD.data_mod, mod_entry_cpu, mod_access_write,
			store->phy_addr, NULL, X86_CORE.event_queue, store);

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		store->in_event_queue = 1;
		store->issued = 1;
		store->issue_when = x86_cpu->cycle;
	
		/* Instruction issued */
		X86_CORE.issued[store->uinst->opcode]++;
		X86_CORE.lsq_reads++;
		X86_CORE.reg_file_int_reads += store->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += store->ph_fp_idep_count;
		X86_THREAD.issued[store->uinst->opcode]++;
		X86_THREAD.lsq_reads++;
		X86_THREAD.reg_file_int_reads += store->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += store->ph_fp_idep_count;
		x86_cpu->issued[store->uinst->opcode]++;
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(store->phy_addr, mmu_access_write);
	}
	return quant;
}


static int x86_cpu_issue_lq(int core, int thread, int quant)
{
	struct linked_list_t *lq = X86_THREAD.lq;
	struct x86_uop_t *load;

	/* Process lq */
	linked_list_head(lq);
	while (!linked_list_is_end(lq) && quant)
	{
		/* Get element from load queue. If it is not ready, go to the next one */
		load = linked_list_get(lq);
		if (!load->ready && !x86_reg_file_ready(load))
		{
			linked_list_next(lq);
			continue;
		}
		load->ready = 1;

		/* Check that memory system is accessible */
		if (!mod_can_access(X86_THREAD.data_mod, load->phy_addr))
		{
			linked_list_next(lq);
			continue;
		}

		/* Remove from load queue */
		assert(load->uinst->opcode == x86_uinst_load);
		x86_lq_remove(core, thread);

		/* Access memory system */
		mod_access(X86_THREAD.data_mod, mod_entry_cpu, mod_access_read,
			load->phy_addr, NULL, X86_CORE.event_queue, load);

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		load->in_event_queue = 1;
		load->issued = 1;
		load->issue_when = x86_cpu->cycle;
		
		/* Instruction issued */
		X86_CORE.issued[load->uinst->opcode]++;
		X86_CORE.lsq_reads++;
		X86_CORE.reg_file_int_reads += load->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += load->ph_fp_idep_count;
		X86_THREAD.issued[load->uinst->opcode]++;
		X86_THREAD.lsq_reads++;
		X86_THREAD.reg_file_int_reads += load->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += load->ph_fp_idep_count;
		x86_cpu->issued[load->uinst->opcode]++;
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(load->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			load->id_in_core, load->core);
	}
	
	return quant;
}


static int x86_cpu_issue_iq(int core, int thread, int quant)
{
	struct linked_list_t *iq = X86_THREAD.iq;
	struct x86_uop_t *uop;
	int lat;

	/* Find instruction to issue */
	linked_list_head(iq);
	while (!linked_list_is_end(iq) && quant)
	{
		/* Get element from IQ */
		uop = linked_list_get(iq);
		assert(x86_uop_exists(uop));
		assert(!(uop->flags & X86_UINST_MEM));
		if (!uop->ready && !x86_reg_file_ready(uop))
		{
			linked_list_next(iq);
			continue;
		}
		uop->ready = 1;  /* avoid next call to 'x86_reg_file_ready' */
		
		/* Run the instruction in its corresponding functional unit.
		 * If the instruction does not require a functional unit, 'x86_fu_reserve'
		 * returns 1 cycle latency. If there is no functional unit available,
		 * 'x86_fu_reserve' returns 0. */
		lat = x86_fu_reserve(uop);
		if (!lat)
		{
			linked_list_next(iq);
			continue;
		}
		
		/* Instruction was issued to the corresponding fu.
		 * Remove it from IQ */
		x86_iq_remove(core, thread);
		
		/* Schedule inst in Event Queue */
		assert(!uop->in_event_queue);
		assert(lat > 0);
		uop->issued = 1;
		uop->issue_when = x86_cpu->cycle;
		uop->when = x86_cpu->cycle + lat;
		x86_event_queue_insert(X86_CORE.event_queue, uop);
		
		/* Instruction issued */
		X86_CORE.issued[uop->uinst->opcode]++;
		X86_CORE.iq_reads++;
		X86_CORE.reg_file_int_reads += uop->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += uop->ph_fp_idep_count;
		X86_THREAD.issued[uop->uinst->opcode]++;
		X86_THREAD.iq_reads++;
		X86_THREAD.reg_file_int_reads += uop->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += uop->ph_fp_idep_count;
		x86_cpu->issued[uop->uinst->opcode]++;
		quant--;

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			uop->id_in_core, uop->core);
	}
	
	return quant;
}


static int x86_cpu_issue_thread_lsq(int core, int thread, int quant)
{
	quant = x86_cpu_issue_lq(core, thread, quant);
	quant = x86_cpu_issue_sq(core, thread, quant);
	return quant;
}


static int x86_cpu_issue_thread_iq(int core, int thread, int quant)
{
	quant = x86_cpu_issue_iq(core, thread, quant);
	return quant;
}


static void x86_cpu_issue_core(int core)
{
	int skip, quant;

	switch (x86_cpu_issue_kind)
	{
	
	case x86_cpu_issue_kind_shared:
	{
		/* Issue LSQs */
		quant = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do {
			X86_CORE.issue_current = (X86_CORE.issue_current + 1) % x86_cpu_num_threads;
			quant = x86_cpu_issue_thread_lsq(core, X86_CORE.issue_current, quant);
			skip--;
		} while (skip && quant);

		/* Issue IQs */
		quant = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do {
			X86_CORE.issue_current = (X86_CORE.issue_current + 1) % x86_cpu_num_threads;
			quant = x86_cpu_issue_thread_iq(core, X86_CORE.issue_current, quant);
			skip--;
		} while (skip && quant);
		
		break;
	}
	
	case x86_cpu_issue_kind_timeslice:
	{
		/* Issue LSQs */
		quant = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do {
			X86_CORE.issue_current = (X86_CORE.issue_current + 1) % x86_cpu_num_threads;
			quant = x86_cpu_issue_thread_lsq(core, X86_CORE.issue_current, quant);
			skip--;
		} while (skip && quant == x86_cpu_issue_width);

		/* Issue IQs */
		quant = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do {
			X86_CORE.issue_current = (X86_CORE.issue_current + 1) % x86_cpu_num_threads;
			quant = x86_cpu_issue_thread_iq(core, X86_CORE.issue_current, quant);
			skip--;
		} while (skip && quant == x86_cpu_issue_width);

		break;
	}

	default:
		panic("%s: invalid issue kind", __FUNCTION__);
	}
}


void x86_cpu_issue()
{
	int core;

	x86_cpu->stage = "issue";
	X86_CORE_FOR_EACH
		x86_cpu_issue_core(core);
}
