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


#include <arch/x86/emu/context.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/misc.h>

#include "core.h"
#include "cpu.h"
#include "event-queue.h"
#include "fetch-queue.h"
#include "inst-queue.h"
#include "load-store-queue.h"
#include "recover.h"
#include "reg-file.h"
#include "rob.h"
#include "thread.h"
#include "trace-cache.h"
#include "uop-queue.h"


/*
 * Class 'X86Thread'
 */

void X86ThreadRecover(X86Thread *self)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct x86_uop_t *uop;

	/* Remove instructions of this thread in fetch queue, uop queue,
	 * instruction queue, store queue, load queue, and event queue. */
	X86ThreadRecoverFetchQueue(self);
	X86ThreadRecoverUopQueue(self);
	X86ThreadRecoverIQ(self);
	X86ThreadRecoverLSQ(self);
	X86ThreadRecoverEventQueue(self);

	/* Remove instructions from ROB, restoring the state of the
	 * physical register file. */
	for (;;)
	{
		/* Get instruction */
		uop = X86ThreadGetROBTail(self);
		if (!uop)
			break;

		/* If we already removed all speculative instructions,
		 * the work is finished */
		assert(uop->thread == self);
		if (!uop->specmode)
			break;
		
		/* Statistics */
		if (uop->trace_cache)
			self->trace_cache->num_squashed_uinst++;
		self->num_squashed_uinst++;
		core->num_squashed_uinst++;
		cpu->num_squashed_uinst++;
		
		/* Undo map */
		if (!uop->completed)
			X86ThreadWriteUop(self, uop);
		X86ThreadUndoUop(self, uop);

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"sq\"\n",
				uop->id_in_core, core->id);
			X86CpuAddToTraceList(cpu, uop);
		}

		/* Remove entry in ROB */
		X86ThreadRemoveROBTail(self);
	}

	/* Check state of fetch stage and mapped context, if still any */
	if (self->ctx)
	{
		/* If we actually fetched wrong instructions, recover emulator */
		if (X86ContextGetState(self->ctx, X86ContextSpecMode))
			X86ContextRecover(self->ctx);
	
		/* Stall fetch and set eip to fetch. */
		self->fetch_stall_until = MAX(self->fetch_stall_until,
				asTiming(cpu)->cycle + x86_cpu_recover_penalty - 1);
		self->fetch_neip = self->ctx->regs->eip;
	}
}

