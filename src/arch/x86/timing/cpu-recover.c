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
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/misc.h>

#include "cpu.h"
#include "event-queue.h"
#include "fetch-queue.h"
#include "inst-queue.h"
#include "load-store-queue.h"
#include "reg-file.h"
#include "rob.h"
#include "trace-cache.h"
#include "uop-queue.h"


void x86_cpu_recover(int core, int thread)
{
	struct x86_uop_t *uop;

	/* Remove instructions of this thread in fetch queue, uop queue,
	 * instruction queue, store queue, load queue, and event queue. */
	x86_fetch_queue_recover(core, thread);
	x86_uop_queue_recover(core, thread);
	x86_iq_recover(core, thread);
	x86_lsq_recover(core, thread);
	x86_event_queue_recover(core, thread);

	/* Remove instructions from ROB, restoring the state of the
	 * physical register file. */
	for (;;)
	{
		/* Get instruction */
		uop = x86_rob_tail(core, thread);
		if (!uop)
			break;

		/* If we already removed all speculative instructions,
		 * the work is finished */
		assert(uop->core == core);
		assert(uop->thread == thread);
		if (!uop->specmode)
			break;
		
		/* Statistics */
		if (uop->trace_cache)
			X86_THREAD.trace_cache->num_squashed_uinst++;
		X86_THREAD.num_squashed_uinst++;
		X86_CORE.num_squashed_uinst++;
		x86_cpu->num_squashed_uinst++;
		
		/* Undo map */
		if (!uop->completed)
			x86_reg_file_write(uop);
		x86_reg_file_undo(uop);

		/* Trace */
		if (x86_tracing())
		{
			x86_trace("x86.inst id=%lld core=%d stg=\"sq\"\n",
				uop->id_in_core, uop->core);
			x86_cpu_uop_trace_list_add(uop);
		}

		/* Remove entry in ROB */
		x86_rob_remove_tail(core, thread);
	}

	/* If we actually fetched wrong instructions, recover emulator */
	if (x86_ctx_get_state(X86_THREAD.ctx, x86_ctx_spec_mode))
		x86_ctx_recover(X86_THREAD.ctx);
	
	/* Stall fetch and set eip to fetch. */
	X86_THREAD.fetch_stall_until = MAX(X86_THREAD.fetch_stall_until, arch_x86->cycle + x86_cpu_recover_penalty - 1);
	X86_THREAD.fetch_neip = X86_THREAD.ctx->regs->eip;
}

