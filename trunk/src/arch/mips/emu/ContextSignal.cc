/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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


namespace MIPS
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

void Context::CheckSignalHandler()
{
	// If context is already running a signal handler, do nothing.
	if (getState(ContextHandler))
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

} // namespace MIPS
