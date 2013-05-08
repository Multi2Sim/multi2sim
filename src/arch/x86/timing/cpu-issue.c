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
#include <arch/x86/emu/emu.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <mem-system/mmu.h>
#include <mem-system/module.h>
#include <mem-system/prefetch-history.h>

#include "cpu.h"
#include "event-queue.h"
#include "fu.h"
#include "inst-queue.h"
#include "load-store-queue.h"
#include "reg-file.h"
#include "trace-cache.h"


static int x86_cpu_issue_sq(int core, int thread, int quant)
{
	struct x86_uop_t *store;
	struct linked_list_t *sq = X86_THREAD.sq;
	struct mod_client_info_t *client_info;

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

		/* create and fill the mod_client_info_t object */
		client_info = mod_client_info_create(X86_THREAD.data_mod);
		client_info->prefetcher_eip = store->eip;

		/* Issue store */
		mod_access(X86_THREAD.data_mod, mod_access_store,
		       store->phy_addr, NULL, X86_CORE.event_queue, store, client_info);

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		store->in_event_queue = 1;
		store->issued = 1;
		store->issue_when = arch_x86->cycle;
	
		/* Statistics */
		X86_CORE.num_issued_uinst_array[store->uinst->opcode]++;
		X86_CORE.lsq_reads++;
		X86_CORE.reg_file_int_reads += store->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += store->ph_fp_idep_count;
		X86_THREAD.num_issued_uinst_array[store->uinst->opcode]++;
		X86_THREAD.lsq_reads++;
		X86_THREAD.reg_file_int_reads += store->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += store->ph_fp_idep_count;
		x86_cpu->num_issued_uinst_array[store->uinst->opcode]++;
		if (store->trace_cache)
			X86_THREAD.trace_cache->num_issued_uinst++;

		/* One more instruction, update quantum. */
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
	struct mod_client_info_t *client_info;

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

		/* create and fill the mod_client_info_t object */
		client_info = mod_client_info_create(X86_THREAD.data_mod);
		client_info->prefetcher_eip = load->eip;

		/* Access memory system */
		mod_access(X86_THREAD.data_mod, mod_access_load,
			load->phy_addr, NULL, X86_CORE.event_queue, load, client_info);

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		load->in_event_queue = 1;
		load->issued = 1;
		load->issue_when = arch_x86->cycle;
		
		/* Statistics */
		X86_CORE.num_issued_uinst_array[load->uinst->opcode]++;
		X86_CORE.lsq_reads++;
		X86_CORE.reg_file_int_reads += load->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += load->ph_fp_idep_count;
		X86_THREAD.num_issued_uinst_array[load->uinst->opcode]++;
		X86_THREAD.lsq_reads++;
		X86_THREAD.reg_file_int_reads += load->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += load->ph_fp_idep_count;
		x86_cpu->num_issued_uinst_array[load->uinst->opcode]++;
		if (load->trace_cache)
			X86_THREAD.trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
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

static int x86_cpu_issue_preq(int core, int thread, int quant)
{
	struct linked_list_t *preq = X86_THREAD.preq;
	struct x86_uop_t *prefetch;

	/* Process preq */
	linked_list_head(preq);
	while (!linked_list_is_end(preq) && quant)
	{
		/* Get element from prefetch queue. If it is not ready, go to the next one */
		prefetch = linked_list_get(preq);
		if (!prefetch->ready && !x86_reg_file_ready(prefetch))
		{
			linked_list_next(preq);
			continue;
		}

		/* 
		 * Make sure its not been prefetched recently. This is just to avoid unnecessary
		 * memory traffic. Even though the cache will realise a "hit" on redundant 
		 * prefetches, its still helpful to avoid going to the memory (cache). 
		 */
		if (prefetch_history_is_redundant(X86_CORE.prefetch_history,
							   X86_THREAD.data_mod, prefetch->phy_addr))
		{
			/* remove from queue. do not prefetch. */
			assert(prefetch->uinst->opcode == x86_uinst_prefetch);
			x86_preq_remove(core, thread);
			prefetch->completed = 1;
			x86_uop_free_if_not_queued(prefetch);
			continue;
		}

		prefetch->ready = 1;

		/* Check that memory system is accessible */
		if (!mod_can_access(X86_THREAD.data_mod, prefetch->phy_addr))
		{
			linked_list_next(preq);
			continue;
		}

		/* Remove from prefetch queue */
		assert(prefetch->uinst->opcode == x86_uinst_prefetch);
		x86_preq_remove(core, thread);

		/* Access memory system */
		mod_access(X86_THREAD.data_mod, mod_access_prefetch,
			prefetch->phy_addr, NULL, X86_CORE.event_queue, prefetch, NULL);

		/* Record prefetched address */
		prefetch_history_record(X86_CORE.prefetch_history, prefetch->phy_addr);

		/* The cache system will place the prefetch at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		prefetch->in_event_queue = 1;
		prefetch->issued = 1;
		prefetch->issue_when = arch_x86->cycle;
		
		/* Statistics */
		X86_CORE.num_issued_uinst_array[prefetch->uinst->opcode]++;
		X86_CORE.lsq_reads++;
		X86_CORE.reg_file_int_reads += prefetch->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += prefetch->ph_fp_idep_count;
		X86_THREAD.num_issued_uinst_array[prefetch->uinst->opcode]++;
		X86_THREAD.lsq_reads++;
		X86_THREAD.reg_file_int_reads += prefetch->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += prefetch->ph_fp_idep_count;
		x86_cpu->num_issued_uinst_array[prefetch->uinst->opcode]++;
		if (prefetch->trace_cache)
			X86_THREAD.trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(prefetch->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			prefetch->id_in_core, prefetch->core);
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
		uop->issue_when = arch_x86->cycle;
		uop->when = arch_x86->cycle + lat;
		x86_event_queue_insert(X86_CORE.event_queue, uop);
		
		/* Statistics */
		X86_CORE.num_issued_uinst_array[uop->uinst->opcode]++;
		X86_CORE.iq_reads++;
		X86_CORE.reg_file_int_reads += uop->ph_int_idep_count;
		X86_CORE.reg_file_fp_reads += uop->ph_fp_idep_count;
		X86_THREAD.num_issued_uinst_array[uop->uinst->opcode]++;
		X86_THREAD.iq_reads++;
		X86_THREAD.reg_file_int_reads += uop->ph_int_idep_count;
		X86_THREAD.reg_file_fp_reads += uop->ph_fp_idep_count;
		x86_cpu->num_issued_uinst_array[uop->uinst->opcode]++;
		if (uop->trace_cache)
			X86_THREAD.trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
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
	quant = x86_cpu_issue_preq(core, thread, quant);

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
