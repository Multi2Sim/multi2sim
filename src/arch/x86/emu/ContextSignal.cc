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

#include <csignal>

#include <lib/cpp/Misc.h>
#include <mem-system/Memory.h>

#include "Context.h"
#include "Emu.h"
#include "Signal.h"

using namespace Memory;
using namespace misc;


namespace x86
{


#if 0
void Context::RunSignalHandler(int sig)
{
	// Debug
	assert(InRange(sig, 1, 64));
	Emu::syscall_debug << "context " << pid << " executes signal "
			"handler for signal " << sig << '\n';

	// Signal SIGCHLD ignored if no signal handler installed
	SignalHandler *signal_handler = signal_handler_table->
			getSignalHandler(sig - 1);
	if (sig == SIGCHLD && !signal_handler->getHandler())
		return;

	// Save a copy of the register file
	signal_mask_table.setRegs(regs);

	// Create a memory page with execution permission, and copy return code
	// on it.
	signal_mask_table.setRetCodePtr(memory->MapSpace(
			MemoryPageSize, MemoryPageSize));
	memory->Map(signal_mask_table.getRetCodePtr(), MemoryPageSize,
			MemoryAccessExec | MemoryAccessInit);
	Emu::syscall_debug << "  return code of signal handler allocated at "
			<< StringFmt("0x%x\n",
			signal_mask_table.getRetCodePtr());
	memory->Init(signal_mask_table.getRetCodePtr(), sizeof signal_ret_code,
			signal_ret_code);

	// Initialize stack frame
	SignalFrame signal_frame;
	signal_frame.ret_code_ptr = signal_mask_table->getRetCodePtr();
	signal_frame.sig = sig;
	signal_frame.gs = ctx->regs->gs;
	signal_frame.fs = ctx->regs->fs;
	signal_frame.es = ctx->regs->es;
	signal_frame.ds = ctx->regs->ds;
	signal_frame.edi = ctx->regs->edi;
	signal_frame.esi = ctx->regs->esi;
	signal_frame.ebp = ctx->regs->ebp;
	signal_frame.esp = ctx->regs->esp;
	signal_frame.ebx = ctx->regs->ebx;
	signal_frame.edx = ctx->regs->edx;
	signal_frame.ecx = ctx->regs->ecx;
	signal_frame.eax = ctx->regs->eax;
	signal_frame.trapno = 0;
	signal_frame.err = 0;
	signal_frame.eip = ctx->regs->eip;
	signal_frame.cs = ctx->regs->cs;
	signal_frame.eflags = ctx->regs->eflags;
	signal_frame.esp_at_signal = ctx->regs->esp;
	signal_frame.ss = ctx->regs->ss;
	signal_frame.pfpstate = 0;
	signal_frame.oldmask = 0;
	signal_frame.cr2 = 0;

	// Push signal frame
	ctx->regs->esp -= sizeof(signal_frame);
	mem_write(ctx->mem, ctx->regs->esp, sizeof(signal_frame), &signal_frame);
	
	/* The program will continue now executing the signal handler.
	 * In the current implementation, we do not allow other signals to
	 * interrupt the signal handler, so we notify it in the context status. */
	if (X86ContextGetState(ctx, X86ContextHandler))
		fatal("signal_handler_run: already running a handler");
	X86ContextSetState(ctx, X86ContextHandler);

	// Set eip to run handler
	unsigned handler = ctx->signal_handler_table->sigaction[sig - 1].handler;
	if (!handler)
		fatal("%s: invalid signal handler", __FUNCTION__);
	ctx->regs->eip = handler;
}


void X86ContextReturnFromSignalHandler(X86Context *ctx)
{
	// Change context status
	if (!X86ContextGetState(ctx, X86ContextHandler))
		fatal("signal_handler_return: not handling a signal");
	X86ContextClearState(ctx, X86ContextHandler);

	// Free signal frame
	mem_unmap(ctx->mem, ctx->signal_mask_table->pretcode, MEM_PAGE_SIZE);
	x86_sys_debug("  signal handler return code at 0x%x deallocated\n",
		ctx->signal_mask_table->pretcode);

	// Restore saved register file and free backup
	x86_regs_copy(ctx->regs, ctx->signal_mask_table->regs);
	x86_regs_free(ctx->signal_mask_table->regs);
}


void X86ContextCheckSignalHandlerIntr(X86Context *ctx)
{
	int sig;

	// Context cannot be running a signal handler
	// A signal must be pending and unblocked
	assert(!X86ContextGetState(ctx, X86ContextHandler));
	assert(ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked);

	// Get signal number
	for (sig = 1; sig <= 64; sig++)
		if (x86_sigset_member(&ctx->signal_mask_table->pending, sig) &&
			!x86_sigset_member(&ctx->signal_mask_table->blocked, sig))
			break;
	assert(sig <= 64);

	/* If signal handling uses 'SA_RESTART' flag, set return address to
	 * system call. */
	if (ctx->signal_handler_table->sigaction[sig - 1].flags & 0x10000000u)
	{
		unsigned char buf[2];

		ctx->regs->eip -= 2;
		mem_read(ctx->mem, ctx->regs->eip, 2, buf);
		assert(buf[0] == 0xcd && buf[1] == 0x80);  // 'int 0x80'
	}
	else
	{
		
		// Otherwise, return -EINTR
		ctx->regs->eax = -EINTR;
	}

	// Run the signal handler
	X86ContextRunSignalHandler(ctx, sig);
	x86_sigset_del(&ctx->signal_mask_table->pending, sig);

}


void X86ContextCheckSignalHandler(X86Context *ctx)
{
	int sig;

	// If context is already running a signal handler, do nothing.
	if (X86ContextGetState(ctx, X86ContextHandler))
		return;
	
	// If there is no pending unblocked signal, do nothing.
	if (!(ctx->signal_mask_table->pending & ~ctx->signal_mask_table->blocked))
		return;
	
	/* There is some unblocked pending signal, prepare signal handler to
	 * be executed. */
	for (sig = 1; sig <= 64; sig++)
	{
		if (x86_sigset_member(&ctx->signal_mask_table->pending, sig) &&
			!x86_sigset_member(&ctx->signal_mask_table->blocked, sig))
		{
			X86ContextRunSignalHandler(ctx, sig);
			x86_sigset_del(&ctx->signal_mask_table->pending, sig);
			break;
		}
	}
}
#endif

}  // namespace x86

