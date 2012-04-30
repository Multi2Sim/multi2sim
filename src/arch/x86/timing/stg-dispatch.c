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


/* Return the reason why a thread cannot be dispatched. If it can,
 * return x86_dispatch_stall_used. */
static enum x86_dispatch_stall_t x86_cpu_can_dispatch_thread(int core, int thread)
{
	struct list_t *uopq = X86_THREAD.uop_queue;
	struct x86_uop_t *uop;

	/* Uop queue empty. */
	uop = list_get(uopq, 0);
	if (!uop)
		return !X86_THREAD.ctx || !x86_ctx_get_status(X86_THREAD.ctx, x86_ctx_running) ?
			x86_dispatch_stall_ctx : x86_dispatch_stall_uop_queue;

	/* If iq/lq/sq/rob full, done */
	if (!x86_rob_can_enqueue(uop))
		return x86_dispatch_stall_rob;
	if (!(uop->flags & X86_UINST_MEM) && !x86_iq_can_insert(uop))
		return x86_dispatch_stall_iq;
	if ((uop->flags & X86_UINST_MEM) && !x86_lsq_can_insert(uop))
		return x86_dispatch_stall_lsq;
	if (!x86_reg_file_can_rename(uop))
		return x86_dispatch_stall_rename;
	
	return x86_dispatch_stall_used;
}


static int x86_cpu_dispatch_thread(int core, int thread, int quant)
{
	struct x86_uop_t *uop;
	enum x86_dispatch_stall_t stall;

	while (quant)
	{
		/* Check if we can decode */
		stall = x86_cpu_can_dispatch_thread(core, thread);
		if (stall != x86_dispatch_stall_used)
		{
			X86_CORE.dispatch_stall[stall] += quant;
			break;
		}
	
		/* Get entry from uop queue */
		uop = list_remove_at(X86_THREAD.uop_queue, 0);
		assert(x86_uop_exists(uop));
		uop->in_uop_queue = 0;
		
		/* Rename */
		x86_reg_file_rename(uop);
		
		/* Insert in ROB */
		x86_rob_enqueue(uop);
		X86_CORE.rob_writes++;
		X86_THREAD.rob_writes++;
		
		/* Non memory instruction into IQ */
		if (!(uop->flags & X86_UINST_MEM))
		{
			x86_iq_insert(uop);
			X86_CORE.iq_writes++;
			X86_THREAD.iq_writes++;
		}
		
		/* Memory instructions into the LSQ */
		if (uop->flags & X86_UINST_MEM)
		{
			x86_lsq_insert(uop);
			X86_CORE.lsq_writes++;
			X86_THREAD.lsq_writes++;
		}
		
		/* Another instruction dispatched */
		X86_CORE.dispatch_stall[uop->specmode ? x86_dispatch_stall_spec : x86_dispatch_stall_used]++;
		X86_THREAD.dispatched[uop->uinst->opcode]++;
		X86_CORE.dispatched[uop->uinst->opcode]++;
		x86_cpu->dispatched[uop->uinst->opcode]++;
		quant--;

		/* Pipeline debug */
		esim_debug("uop action=\"create\", core=%d, seq=%llu, name=\"%s\","
			" mop_name=\"%s\", mop_count=%d, mop_index=%d, spec=%u,"
			" stg_dispatch=1, in_rob=%u, in_iq=%u, in_lsq=%u\n",
			uop->core, uop->id_in_core, uop->name,
			uop->mop_name, uop->mop_count, uop->mop_index, uop->specmode,
			!!uop->in_rob, !!uop->in_iq, uop->in_lq || uop->in_sq);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"di\"\n",
			uop->id_in_core, uop->core);

	}

	return quant;
}


static void x86_cpu_dispatch_core(int core)
{
	int skip = x86_cpu_num_threads;
	int quant = x86_cpu_dispatch_width;
	int remain;

	switch (x86_cpu_dispatch_kind)
	{

	case x86_cpu_dispatch_kind_shared:
		
		do {
			X86_CORE.dispatch_current = (X86_CORE.dispatch_current + 1) % x86_cpu_num_threads;
			remain = x86_cpu_dispatch_thread(core, X86_CORE.dispatch_current, 1);
			skip = remain ? skip - 1 : x86_cpu_num_threads;
			quant = remain ? quant : quant - 1;
		} while (quant && skip);
		break;
	
	case x86_cpu_dispatch_kind_timeslice:
		
		do {
			X86_CORE.dispatch_current = (X86_CORE.dispatch_current + 1) % x86_cpu_num_threads;
			skip--;
		} while (skip && x86_cpu_can_dispatch_thread(core, X86_CORE.dispatch_current) != x86_dispatch_stall_used);
		x86_cpu_dispatch_thread(core, X86_CORE.dispatch_current, quant);
		break;
	}
}


void x86_cpu_dispatch()
{
	int core;
	x86_cpu->stage = "dispatch";
	X86_CORE_FOR_EACH
		x86_cpu_dispatch_core(core);
}
