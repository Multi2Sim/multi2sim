/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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


namespace ARM
{

void Context::RunSignalHandler(int sig)
{
	// Debug
	assert(misc::inRange(sig, 1, 64));
	//Emu::syscall_debug << "context " << pid << " executes signal "
			//"handler for signal " << sig << '\n';

	// Signal SIGCHLD ignored if no signal handler installed
	SignalHandler *signal_handler = signal_handler_table->
			getSignalHandler(sig);
	if (sig == SIGCHLD && !signal_handler->getHandler())
		return;

	// Save a copy of the register file
	signal_mask_table.setRegs(regs);


}

void Context::CheckSignalHandlerIntr()
{
	// Context cannot be running a signal handler. A signal must be pending
	// and unblocked.
	assert(!getState(ContextStateHandler));
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
		// FIXME
		//unsigned char buf[2];
		//regs.decPC(2);
		//memory->Read(regs.getPC(), 2, (char *) buf);
		//assert(buf[0] == 0xcd && buf[1] == 0x80);  // 'int 0x80'
	}
	else
	{
		// FIXME
		// Otherwise, return -EINTR
		//regs.setRegister(7, -EINTR);
	}

	// Run the signal handler
	RunSignalHandler(sig);
	signal_mask_table.getPending().Delete(sig);
}

void Context::CheckSignalHandler()
{
	// If context is already running a signal handler, do nothing.
	if (getState(ContextStateHandler))
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

}
