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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_EMU_H
#define ARCH_COMMON_EMU_H

#include <lib/cpp/Timer.h>
#include <lib/esim/ESim.h>
#include <string>


namespace comm
{

class Emu
{
	// Name of emulator
	std::string name;
	
	// Timer keeping track of emulator activity
	misc::Timer timer;

protected:

	// Event-driven simulator
	esim::ESim *esim;

	// Number of emulated instructions
	long long instructions;

public:
	/// Constructor
	Emu(const std::string &name);

	/// Increment the number of emulated instructions
	void incInstructions() { ++instructions; }

	/// Return the number of emulated instructions
	long long getInstructions() { return instructions; }

	/// Start or resume the emulator timer
	void StartTimer() { timer.Start(); }

	/// Pause the emulator timer
	void StopTimer() { timer.Stop(); }

	/// Run one iteration of the emulation loop for the architecture. If
	/// there was an active emulation, the function returns \c true. This is
	/// a virtual abstract function. Every emulator derived from this class
	/// has to provide an implementation for it.
	virtual bool Run() = 0;

	/// Dump the statistics summary for the emulator. This is a virtual
	/// function that should be overridden by child classes. The function is
	/// not abstract: its body is still valid for class comm::Emu, and
	/// it can be invoked by the child class to print general statistics
	/// present for all emulators, such as instruction count.
	virtual void DumpSummary(std::ostream &os) const;
};


}  // namespace comm

#endif

