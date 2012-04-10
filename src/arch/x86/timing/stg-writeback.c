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


static void x86_cpu_writeback_core(int core)
{
	struct x86_uop_t *uop;

	int thread;
	int recover = 0;

	for (;;)
	{
		/* Pick element from the head of the event queue */
		linked_list_head(X86_CORE.event_queue);
		uop = linked_list_get(X86_CORE.event_queue);
		if (!uop)
			break;

		/* A memory uop placed in the event queue is always complete.
		 * Other uops are complete when uop->when is equals to current cycle. */
		if (uop->flags & X86_UINST_MEM)
			uop->when = x86_cpu->cycle;
		if (uop->when > x86_cpu->cycle)
			break;
		
		/* Check element integrity */
		assert(x86_uop_exists(uop));
		assert(uop->when == x86_cpu->cycle);
		assert(uop->core == core);
		assert(uop->ready);
		assert(!uop->completed);
		
		/* Extract element from event queue. */
		linked_list_remove(X86_CORE.event_queue);
		uop->in_event_queue = 0;
		thread = uop->thread;
		
		/* If a mispredicted branch is solved and recovery is configured to be
		 * performed at writeback, schedule it for the end of the iteration. */
		if (x86_cpu_recover_kind == x86_cpu_recover_kind_writeback &&
			(uop->flags & X86_UINST_CTRL) && !uop->specmode &&
			uop->neip != uop->pred_neip)
			recover = 1;

		/* Debug */
		esim_debug("uop action=\"update\", core=%d, seq=%llu,"
			" stg_writeback=1, completed=1\n",
			uop->core, (long long unsigned) uop->dispatch_seq);

		/* Writeback */
		uop->completed = 1;
		x86_reg_file_write(uop);
		X86_CORE.reg_file_int_writes += uop->ph_int_odep_count;
		X86_CORE.reg_file_fp_writes += uop->ph_fp_odep_count;
		X86_CORE.iq_wakeup_accesses++;
		X86_THREAD.reg_file_int_writes += uop->ph_int_odep_count;
		X86_THREAD.reg_file_fp_writes += uop->ph_fp_odep_count;
		X86_THREAD.iq_wakeup_accesses++;
		x86_uop_free_if_not_queued(uop);

		/* Recovery. This must be performed at last, because lots of uops might be
		 * freed, which interferes with the temporary extraction from the event_queue. */
		if (recover)
			x86_cpu_recover(core, thread);
	}
}


void x86_cpu_writeback()
{
	int core;

	x86_cpu->stage = "writeback";
	X86_CORE_FOR_EACH
		x86_cpu_writeback_core(core);
}

