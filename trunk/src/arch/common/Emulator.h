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

#ifndef ARCH_COMMON_EMULATOR_H
#define ARCH_COMMON_EMULATOR_H

#include <cstdlib>
#include <string>

#include <lib/cpp/Error.h>
#include <lib/cpp/Timer.h>
#include <lib/esim/Engine.h>
#include <memory/Mmu.h>


namespace comm
{

class Emulator
{
	// Name of emulator
	std::string name;
	
	// Timer keeping track of emulator activity
	misc::Timer timer;

	// Each emulator has its own MMU. An MMU provides a separate physical
	// address space that the timing simulator uses to access a subset of
	// the memory hierarchy. The reason why the MMU is introduced at the
	// emulation level is because the creation of contexts deal with whether
	// virtual memory spaces are reused or re-created across contexts.
	//
	// NOTE: This approach has to be changed or extended when implementing
	// fused memory systems across architectures.
	mem::Mmu mmu;

protected:

	/// Event-driven simulation engine
	esim::Engine *esim;

	/// Number of emulated instructions
	long long num_instructions = 0;

public:
	/// Constructor
	Emulator(const std::string &name);

	/// Return the emulator name
	const std::string &getName() const { return name; }

	/// Return the MMU associated with this emulator. NOTE: When introducing
	/// fused memory systems in future versions, an MMU will not be
	/// necessarily associated to each emulator.
	mem::Mmu *getMmu() { return &mmu; }

	/// Increment the number of emulated instructions
	void incNumInstructions() { ++num_instructions; }

	/// Return the number of emulated instructions
	long long getNumInstructions() const { return num_instructions; }

	/// Start or resume the emulator timer
	void StartTimer() { timer.Start(); }

	/// Pause the emulator timer
	void StopTimer() { timer.Stop(); }

	/// Return the time in microseconds that this emulator has been running
	/// so far.
	long long getTimerValue() const { return timer.getValue(); }

	/// Create a CPU context and load a program from the specified command
	/// line. The context state is left in a state ready to start running
	/// the first x86 ISA instruction at the program entry. This function
	/// should be overridden by all CPU architectures.
	///
	/// \param args
	///	Command line to be used, where the first argument contains the
	///	path to the executable ELF file. If the executable is given as a
	///	relative path, it is considered relative to argument \a cwd, or
	///	to the current directoy if \a cwd is empty.
	///
	/// \param env
	///	Array of environment variables. The environment variables
	///	actually loaded in the program is the vector of existing
	///	environment variables in the M2S process, together with any
	///	extra variable contained in this array.
	///
	/// \param cwd
	///	Initial current working directory for the context. Relative
	///	paths used by the context will be relative to this directory.
	///
	/// \param stdin_file_name
	///	File to redirect the standard input, or empty string for no
	///	redirection. If this is a relative path, it is interpreted as
	///	relative to \a cwd, or to the current directory if \a cwd is
	///	empty.
	///
	/// \param stdout_file_name
	///	File to redirect the standard output and standard error output,
	///	or empty string for no redirection. If this is a relative path,
	///	it is interpreted as relative to \a cwd, or to the current
	///	directoy if \a cwd is empty.
	virtual void LoadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env = { },
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "")
	{
		throw misc::Panic("Unimplemented");
	}

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

