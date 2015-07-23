/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include "Thread.h"


namespace x86
{

int Thread::IssueLoadQueue(int quantum)
{
	// List iterators
	auto it = load_queue.begin();
	auto e = load_queue.end();

	// Traverse list
	while (it != e && quantum > 0)
	{
		// Get the uop
		std::shared_ptr<Uop> uop = *it;

		// If the uop is not ready, skip it
		if (!register_file->isUopReady(uop.get()))
		{
			++it;
			continue;
		}

		// Check that memory system is accessible
		if (!data_module->canAccess(uop->physical_address))
		{
			++it;
			continue;
		}
	}
#if 0
	/* Process lq */
	linked_list_head(lq);
	while (!linked_list_is_end(lq) && quant)
	{
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
		MMUAccessPage(cpu->mmu, load->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n",
			load->id_in_core, core->id);
	}
#endif

	// Return remaining quantum
	return quantum;
}


int Thread::IssueStoreQueue(int quantum)
{
	// Return remaining quantum
	return quantum;
}


int Thread::IssueLoadStoreQueue(int quantum)
{
	// Give priority to loads versus stores
	quantum = IssueLoadQueue(quantum);
	quantum = IssueStoreQueue(quantum);

	// Return remaining unused quantum
	return quantum;
}


int Thread::IssueInstructionQueue(int quantum)
{
	// Return remaining unused quantum
	return quantum;
}

}

