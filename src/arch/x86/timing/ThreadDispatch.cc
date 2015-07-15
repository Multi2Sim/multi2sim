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

Thread::DispatchStall Thread::canDispatch()
{
	// Uop queue is empty
	if (uop_queue.size() == 0)
		return !context || !context->getState(Context::StateRunning) ?
				DispatchStallContext :
				DispatchStallUopQueue;
	
	// Reorder buffer is full
	if (!canInsertInReorderBuffer())
		return DispatchStallReorderBuffer;

	// Instruction queue is full
	Uop *uop = uop_queue.front().get();
	if (!(uop->getFlags() & Uinst::FlagMem) && !canInsertInInstructionQueue())
		return DispatchStallInstructionQueue;

	// Load-store queue is full
	if ((uop->getFlags() & Uinst::FlagMem) && !canInsertInLoadStoreQueue())
		return DispatchStallLoadStoreQueue;

	// No physical registers available
	if (!register_file->canRename(uop))
		return DispatchStallRename;

	// Dispatch slot available
	return DispatchStallUsed;
}


int Thread::Dispatch(int quantum)
{
#if 0
	// Repeat while there is quantum left
	while (quantum)
	{
		/* Check if we can decode */
		stall = X86ThreadCanDispatch(self);
		if (stall != x86_dispatch_stall_used)
		{
			core->dispatch_stall[stall] += quantum;
			break;
		}
	
		/* Get entry from uop queue */
		uop = list_remove_at(self->uop_queue, 0);
		assert(x86_uop_exists(uop));
		uop->in_uop_queue = 0;
		
		/* Rename */
		X86ThreadRenameUop(self, uop);
		
		/* Insert in ROB */
		X86CoreEnqueueInROB(core, uop);
		core->rob_writes++;
		self->rob_writes++;
		
		/* Non memory instruction into IQ */
		if (!(uop->flags & X86_UINST_MEM))
		{
			X86ThreadInsertInIQ(self, uop);
			core->iq_writes++;
			self->iq_writes++;
		}
		
		/* Memory instructions into the LSQ */
		if (uop->flags & X86_UINST_MEM)
		{
			X86ThreadInsertInLSQ(self, uop);
			core->lsq_writes++;
			self->lsq_writes++;
		}
		
		/* Statistics */
		core->dispatch_stall[uop->specmode ? x86_dispatch_stall_spec : x86_dispatch_stall_used]++;
		self->num_dispatched_uinst_array[uop->uinst->opcode]++;
		core->num_dispatched_uinst_array[uop->uinst->opcode]++;
		cpu->num_dispatched_uinst_array[uop->uinst->opcode]++;
		if (uop->trace_cache)
			self->trace_cache->num_dispatched_uinst++;
		
		/* Another instruction dispatched, update quantum. */
		quantum--;

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"di\"\n",
			uop->id_in_core, core->id);

	}

	return quantum;
#endif
	return 0;
}

}

