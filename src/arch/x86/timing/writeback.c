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
#include <lib/util/linked-list.h>

#include "core.h"
#include "cpu.h"
#include "recover.h"
#include "reg-file.h"
#include "thread.h"
#include "writeback.h"



/*
 * Class 'X86Core'
 */

void X86CoreWriteback(X86Core *self)
{
	X86Cpu *cpu = self->cpu;
	X86Thread *thread;

	struct x86_uop_t *uop;

	int recover = 0;

	for (;;)
	{
		/* Pick element from the head of the event queue */
		linked_list_head(self->event_queue);
		uop = linked_list_get(self->event_queue);
		if (!uop)
			break;

		/* A memory uop placed in the event queue is always complete.
		 * Other uops are complete when uop->when is equals to current cycle. */
		if (uop->flags & X86_UINST_MEM)
			uop->when = asTiming(cpu)->cycle;
		if (uop->when > asTiming(cpu)->cycle)
			break;
		
		/* Check element integrity */
		assert(x86_uop_exists(uop));
		assert(uop->when == asTiming(cpu)->cycle);
		assert(uop->thread->core == self);
		assert(uop->ready);
		assert(!uop->completed);
		
		/* Extract element from event queue. */
		linked_list_remove(self->event_queue);
		uop->in_event_queue = 0;
		thread = uop->thread;
		
		/* If a mispredicted branch is solved and recovery is configured to be
		 * performed at writeback, schedule it for the end of the iteration. */
		if (x86_cpu_recover_kind == x86_cpu_recover_kind_writeback &&
			(uop->flags & X86_UINST_CTRL) && !uop->specmode &&
			uop->neip != uop->pred_neip)
			recover = 1;

		/* Trace. Prevent instructions that are not in the ROB from tracing.
		 * These can be either loads that were squashed, or stored that
		 * committed before issuing. */
		if (uop->in_rob)
			x86_trace("x86.inst id=%lld core=%d stg=\"wb\"\n",
				uop->id_in_core, self->id);

		/* Writeback */
		uop->completed = 1;
		X86ThreadWriteUop(thread, uop);
		self->reg_file_int_writes += uop->ph_int_odep_count;
		self->reg_file_fp_writes += uop->ph_fp_odep_count;
		self->iq_wakeup_accesses++;
		thread->reg_file_int_writes += uop->ph_int_odep_count;
		thread->reg_file_fp_writes += uop->ph_fp_odep_count;
		thread->iq_wakeup_accesses++;
		x86_uop_free_if_not_queued(uop);

		/* Recovery. This must be performed at last, because lots of uops might be
		 * freed, which interferes with the temporary extraction from the event_queue. */
		if (recover)
			X86ThreadRecover(thread);
	}
}




/*
 * Class 'X86Cpu'
 */

void X86CpuWriteback(X86Cpu *self)
{
	int i;

	self->stage = "writeback";
	for (i = 0; i < x86_cpu_num_cores; i++)
		X86CoreWriteback(self->cores[i]);
}

