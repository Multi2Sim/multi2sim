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


#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <mem-system/mmu.h>
#include <mem-system/module.h>

#include "core.h"
#include "cpu.h"
#include "event-queue.h"
#include "fu.h"
#include "inst-queue.h"
#include "issue.h"
#include "load-store-queue.h"
#include "reg-file.h"
#include "thread.h"
#include "trace-cache.h"


/*
 * Class 'X86Thread'
 */

static int X86ThreadIssueSQ(X86Thread *self, int quantum)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct x86_uop_t *store;
	struct linked_list_t *sq = self->sq;
	struct mod_client_info_t *client_info;

	/* Process SQ */
	linked_list_head(sq);
	while (!linked_list_is_end(sq) && quantum)
	{
		/* Get store */
		store = linked_list_get(sq);
		assert(store->uinst->opcode == x86_uinst_store);

		/* Only committed stores issue */
		if (store->in_rob)
			break;

		/* Check that memory system entry is ready */
		if (!mod_can_access(self->data_mod, store->phy_addr))
			break;

		/* Remove store from store queue */
		X86ThreadRemoveFromSQ(self);

		/* create and fill the mod_client_info_t object */
		client_info = mod_client_info_create(self->data_mod);
		client_info->prefetcher_eip = store->eip;

		/* Issue store */
		mod_access(self->data_mod, mod_access_store,
		       store->phy_addr, NULL, core->event_queue, store, client_info);

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		store->in_event_queue = 1;
		store->issued = 1;
		store->issue_when = asTiming(cpu)->cycle;
	
		/* Statistics */
		core->num_issued_uinst_array[store->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += store->ph_int_idep_count;
		core->reg_file_fp_reads += store->ph_fp_idep_count;
		self->num_issued_uinst_array[store->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += store->ph_int_idep_count;
		self->reg_file_fp_reads += store->ph_fp_idep_count;
		cpu->num_issued_uinst_array[store->uinst->opcode]++;
		if (store->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction, update quantum. */
		quantum--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(store->phy_addr, mmu_access_write);
	}
	return quantum;
}


static int X86ThreadIssueLQ(X86Thread *self, int quant)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct linked_list_t *lq = self->lq;
	struct x86_uop_t *load;
	struct mod_client_info_t *client_info;

	/* Process lq */
	linked_list_head(lq);
	while (!linked_list_is_end(lq) && quant)
	{
		/* Get element from load queue. If it is not ready, go to the next one */
		load = linked_list_get(lq);
		if (!load->ready && !X86ThreadIsUopReady(self, load))
		{
			linked_list_next(lq);
			continue;
		}
		load->ready = 1;

		/* Check that memory system is accessible */
		if (!mod_can_access(self->data_mod, load->phy_addr))
		{
			linked_list_next(lq);
			continue;
		}

		/* Remove from load queue */
		assert(load->uinst->opcode == x86_uinst_load);
		X86ThreadRemoveFromLQ(self);

		/* create and fill the mod_client_info_t object */
		client_info = mod_client_info_create(self->data_mod);
		client_info->prefetcher_eip = load->eip;

		/* Access memory system */
		mod_access(self->data_mod, mod_access_load,
			load->phy_addr, NULL, core->event_queue, load, client_info);

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		load->in_event_queue = 1;
		load->issued = 1;
		load->issue_when = asTiming(cpu)->cycle;
		
		/* Statistics */
		core->num_issued_uinst_array[load->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += load->ph_int_idep_count;
		core->reg_file_fp_reads += load->ph_fp_idep_count;
		self->num_issued_uinst_array[load->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += load->ph_int_idep_count;
		self->reg_file_fp_reads += load->ph_fp_idep_count;
		cpu->num_issued_uinst_array[load->uinst->opcode]++;
		if (load->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quant--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(load->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			load->id_in_core, core->id);
	}
	
	return quant;
}


static int X86ThreadIssuePreQ(X86Thread *self, int quantum)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct linked_list_t *preq = self->preq;
	struct x86_uop_t *prefetch;

	/* Process preq */
	linked_list_head(preq);
	while (!linked_list_is_end(preq) && quantum)
	{
		/* Get element from prefetch queue. If it is not ready, go to the next one */
		prefetch = linked_list_get(preq);
		if (!prefetch->ready && !X86ThreadIsUopReady(self, prefetch))
		{
			linked_list_next(preq);
			continue;
		}

		/* 
		 * Make sure its not been prefetched recently. This is just to avoid unnecessary
		 * memory traffic. Even though the cache will realise a "hit" on redundant 
		 * prefetches, its still helpful to avoid going to the memory (cache). 
		 */
		if (prefetch_history_is_redundant(core->prefetch_history,
							   self->data_mod, prefetch->phy_addr))
		{
			/* remove from queue. do not prefetch. */
			assert(prefetch->uinst->opcode == x86_uinst_prefetch);
			X86ThreadRemovePreQ(self);
			prefetch->completed = 1;
			x86_uop_free_if_not_queued(prefetch);
			continue;
		}

		prefetch->ready = 1;

		/* Check that memory system is accessible */
		if (!mod_can_access(self->data_mod, prefetch->phy_addr))
		{
			linked_list_next(preq);
			continue;
		}

		/* Remove from prefetch queue */
		assert(prefetch->uinst->opcode == x86_uinst_prefetch);
		X86ThreadRemovePreQ(self);

		/* Access memory system */
		mod_access(self->data_mod, mod_access_prefetch,
			prefetch->phy_addr, NULL, core->event_queue, prefetch, NULL);

		/* Record prefetched address */
		prefetch_history_record(core->prefetch_history, prefetch->phy_addr);

		/* The cache system will place the prefetch at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		prefetch->in_event_queue = 1;
		prefetch->issued = 1;
		prefetch->issue_when = asTiming(cpu)->cycle;
		
		/* Statistics */
		core->num_issued_uinst_array[prefetch->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += prefetch->ph_int_idep_count;
		core->reg_file_fp_reads += prefetch->ph_fp_idep_count;
		self->num_issued_uinst_array[prefetch->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += prefetch->ph_int_idep_count;
		self->reg_file_fp_reads += prefetch->ph_fp_idep_count;
		cpu->num_issued_uinst_array[prefetch->uinst->opcode]++;
		if (prefetch->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quantum--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(prefetch->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			prefetch->id_in_core, core->id);
	}
	
	return quantum;
}


static int X86ThreadIssueIQ(X86Thread *self, int quant)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct linked_list_t *iq = self->iq;
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
		if (!uop->ready && !X86ThreadIsUopReady(self, uop))
		{
			linked_list_next(iq);
			continue;
		}
		uop->ready = 1;  /* avoid next call to 'X86ThreadIsUopReady' */
		
		/* Run the instruction in its corresponding functional unit.
		 * If the instruction does not require a functional unit, 'X86CoreReserveFunctionalUnit'
		 * returns 1 cycle latency. If there is no functional unit available,
		 * 'X86CoreReserveFunctionalUnit' returns 0. */
		lat = X86CoreReserveFunctionalUnit(core, uop);
		if (!lat)
		{
			linked_list_next(iq);
			continue;
		}
		
		/* Instruction was issued to the corresponding fu.
		 * Remove it from IQ */
		X86ThreadRemoveFromIQ(self);
		
		/* Schedule inst in Event Queue */
		assert(!uop->in_event_queue);
		assert(lat > 0);
		uop->issued = 1;
		uop->issue_when = asTiming(cpu)->cycle;
		uop->when = asTiming(cpu)->cycle + lat;
		X86CoreInsertInEventQueue(core, uop);
		
		/* Statistics */
		core->num_issued_uinst_array[uop->uinst->opcode]++;
		core->iq_reads++;
		core->reg_file_int_reads += uop->ph_int_idep_count;
		core->reg_file_fp_reads += uop->ph_fp_idep_count;
		self->num_issued_uinst_array[uop->uinst->opcode]++;
		self->iq_reads++;
		self->reg_file_int_reads += uop->ph_int_idep_count;
		self->reg_file_fp_reads += uop->ph_fp_idep_count;
		cpu->num_issued_uinst_array[uop->uinst->opcode]++;
		if (uop->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quant--;

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			uop->id_in_core, core->id);
	}
	
	return quant;
}


static int X86ThreadIssueLSQ(X86Thread *self, int quantum)
{
	quantum = X86ThreadIssueLQ(self, quantum);
	quantum = X86ThreadIssueSQ(self, quantum);
	quantum = X86ThreadIssuePreQ(self, quantum);

	return quantum;
}





/*
 * Class 'X86Core'
 */

static void X86CoreIssue(X86Core *self)
{
	X86Thread *thread;

	int skip;
	int quantum;

	switch (x86_cpu_issue_kind)
	{
	
	case x86_cpu_issue_kind_shared:
	{
		/* Issue LSQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueLSQ(thread, quantum);
			skip--;
		} while (skip && quantum);

		/* Issue IQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueIQ(thread, quantum);
			skip--;
		} while (skip && quantum);
		
		break;
	}
	
	case x86_cpu_issue_kind_timeslice:
	{
		/* Issue LSQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueLSQ(thread, quantum);
			skip--;
		} while (skip && quantum == x86_cpu_issue_width);

		/* Issue IQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueIQ(thread, quantum);
			skip--;
		} while (skip && quantum == x86_cpu_issue_width);

		break;
	}

	default:
		panic("%s: invalid issue kind", __FUNCTION__);
	}
}




/*
 * Class 'X86Cpu'
 */

void X86CpuIssue(X86Cpu *self)
{
	int i;

	self->stage = "issue";
	for (i = 0; i < x86_cpu_num_cores; i++)
		X86CoreIssue(self->cores[i]);
}
