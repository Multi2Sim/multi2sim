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
#include <memory/Memory.h>

#include "Context.h"
#include "Emulator.h"
#include "Signal.h"


namespace x86
{


void Context::RunSignalHandler(int sig)
{
	// Debug
	assert(misc::inRange(sig, 1, 64));
	Emulator::syscall_debug << getName() << " executes signal "
			"handler for signal " << sig << '\n';

	// Signal SIGCHLD ignored if no signal handler installed
	SignalHandler *signal_handler = signal_handler_table->
			getSignalHandler(sig);
	if (sig == SIGCHLD && !signal_handler->getHandler())
		return;

	// Save a copy of the register file
	signal_mask_table.setRegs(regs);

	// Create a memory page with execution permission, and copy return code
	// on it.
	signal_mask_table.setRetCodePtr(memory->MapSpace(
			mem::Memory::PageSize, mem::Memory::PageSize));
	memory->Map(signal_mask_table.getRetCodePtr(), mem::Memory::PageSize,
			mem::Memory::AccessExec | mem::Memory::AccessInit);
	Emulator::syscall_debug << "  return code of signal handler allocated at "
			<< misc::fmt("0x%x\n",
			signal_mask_table.getRetCodePtr());
	memory->Init(signal_mask_table.getRetCodePtr(), sizeof signal_ret_code,
			signal_ret_code);

	// Initialize stack frame
	SignalFrame signal_frame;
	signal_frame.ret_code_ptr = signal_mask_table.getRetCodePtr();
	signal_frame.sig = sig;
	signal_frame.gs = regs.getGs();
	signal_frame.fs = regs.getFs();
	signal_frame.es = regs.getEs();
	signal_frame.ds = regs.getDs();
	signal_frame.edi = regs.getEdi();
	signal_frame.esi = regs.getEsi();
	signal_frame.ebp = regs.getEbp();
	signal_frame.esp = regs.getEsp();
	signal_frame.ebx = regs.getEbx();
	signal_frame.edx = regs.getEdx();
	signal_frame.ecx = regs.getEcx();
	signal_frame.eax = regs.getEax();
	signal_frame.trapno = 0;
	signal_frame.err = 0;
	signal_frame.eip = regs.getEip();
	signal_frame.cs = regs.getCs();
	signal_frame.eflags = regs.getEflags();
	signal_frame.esp_at_signal = regs.getEsp();
	signal_frame.ss = regs.getSs();
	signal_frame.pfpstate = 0;
	signal_frame.oldmask = 0;
	signal_frame.cr2 = 0;

	// Push signal frame
	regs.decEsp(sizeof signal_frame);
	memory->Write(regs.getEsp(), sizeof signal_frame,
			(char *) &signal_frame);
	
	// The program will continue now executing the signal handler.
	// In the current implementation, we do not allow other signals to
	// interrupt the signal handler, so we notify it in the context status.
	if (getState(StateHandler))
		misc::fatal("%s: already running a handler", __FUNCTION__);
	setState(StateHandler);

	// Set eip to run handler
	unsigned handler = signal_handler->getHandler();
	if (!handler)
		misc::fatal("%s: invalid signal handler", __FUNCTION__);
	regs.setEip(handler);
}


void Context::ReturnFromSignalHandler()
{
	// Change context status
	if (!getState(StateHandler))
		misc::fatal("%s: not handling a signal", __FUNCTION__);
	clearState(StateHandler);

	// Free signal frame
	memory->Unmap(signal_mask_table.getRetCodePtr(), mem::Memory::PageSize);
	Emulator::syscall_debug << "  signal handler return code at " <<
			misc::fmt("0x%x", signal_mask_table.getRetCodePtr())
			<< " deallocated\n";

	// Restore saved register file and free backup
	regs = signal_mask_table.getRegs();
	signal_mask_table.freeRegs();
}


void Context::CheckSignalHandlerIntr()
{
	// Context cannot be running a signal handler. A signal must be pending
	// and unblocked.
	assert(!getState(StateHandler));
	assert((signal_mask_table.getPending().getBitmap() &
			~signal_mask_table.getBlocked().getBitmap()).Any());

	// Get signal number
	int sig;
	for (sig = 1; sig <= 64; sig++)
		if (signal_mask_table.getPending().isMember(sig) &&
				!signal_mask_table.getBlocked().isMember(sig))
			break;
	assert(sig <= 64);

	// If signal handling uses 'SA_RESTART' flag, set return address to
	// system call.
	SignalHandler *signal_handler = signal_handler_table->
			getSignalHandler(sig);
	if (signal_handler->getFlags() & 0x10000000u)
	{
		unsigned char buf[2];
		regs.decEip(2);
		memory->Read(regs.getEip(), 2, (char *) buf);
		assert(buf[0] == 0xcd && buf[1] == 0x80);  // 'int 0x80'
	}
	else
	{
		// Otherwise, return -EINTR
		regs.setEax(-EINTR);
	}

	// Run the signal handler
	RunSignalHandler(sig);
	signal_mask_table.getPending().Delete(sig);
}


void Context::CheckSignalHandler()
{
	// If context is already running a signal handler, do nothing.
	if (getState(StateHandler))
		return;
	
	// If there is no pending unblocked signal, do nothing.
	if ((signal_mask_table.getPending().getBitmap() &
			~signal_mask_table.getBlocked().getBitmap()).None())
		return;
	
	/* There is some unblocked pending signal, prepare signal handler to
	 * be executed. */
	for (int sig = 1; sig <= 64; sig++)
	{
		if (signal_mask_table.getPending().isMember(sig) &&
				!signal_mask_table.getBlocked().isMember(sig))
		{
			RunSignalHandler(sig);
			signal_mask_table.getPending().Delete(sig);
			break;
		}
	}
}

}  // namespace x86

