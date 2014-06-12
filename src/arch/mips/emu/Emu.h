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
#include <arch/common/Arch.h>
#include <arch/common/Emu.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include "Context.h"

namespace MIPS
{

//class Context;


class EmuConfig : public misc::CommandLineConfig
{
	// Maximum number of instructions
	long long max_instructions;

	// Simulation kind
	comm::ArchSimKind sim_kind;

	// Process prefetch instructions
	bool process_prefetch_hints;

public:

	/// Initialization of default command-line options
	EmuConfig();

	/// Register command-line options related with the MIPS emulator
	void Register(misc::CommandLine &command_line);

	/// Return maximum number of instructions
	long long getMaxInstructions() { return max_instructions; }
};


class Emu : public comm::Emu
{
	// Unique instance of x86 emulator
	static std::unique_ptr<Emu> instance;

	// See setScheduleSignal()
	bool schedule_signal;

	// Primary list of contexts
	std::list<std::unique_ptr<Context>> contexts;

	// Secondary lists of contexts. Contexts in different states
	// are added/removed from this lists as their state gets updated.
	std::list<Context *> context_list[ContextListCount];

	// Index of virtual memory space assigned to new contexts. A new ID
	// can be retrieved in increasing order by using function
	// Emu::getAddressSpaceIndex()
	int address_space_index;

	// Process ID to be assigned next. Process IDs are assigned in
	// increasing order, using function Emu::getPid()
	int pid;

	// Schedule next call to Emu::ProcessEvents(). The call will only be
	// effective if 'process_events_force' is set. This flag should be
	// accessed thread-safely locking the mutex.
	bool process_events_force;

	// Counter of times that a context has been suspended in a futex. Used
	// for FIFO wakeups.
	long long futex_sleep_count;

public:
	///Constructor and Desctructor
	Emu();
	~Emu();

	/// Signals a call to the scheduler Timing::Schedule() in the
	/// beginning of next cycle. This flag is set any time a context changes
	/// its state in any bit other than ContextSpecMode. It can be set
	/// anywhere in the code by directly assigning a value to 1. E.g.:
	/// when a system call is executed to change the context's affinity.
	void setScheduleSignal() { schedule_signal = true; }


	/// Create a new context associated with the emulator. The context is
	/// inserted in the main emulator context list. Its state is set to
	/// ContextRunning, and it is inserted into the emulator list of running
	/// contexts.
	Context *newContext();

	/// The mips emulator is a singleton class. The only possible instance of
	/// it will be allocated the first time this function is invoked.
	static Emu *getInstance();

	/// Remove a context from all context lists and free it
	void freeContext(Context *context);

	/// Remove a context from a context list if present
	void RemoveContextFromList(ContextListType type, Context *context);

	/// Return a unique increasing ID for a virtual memory space for
	/// contexts.
	int getAddressSpaceIndex() { return address_space_index++; }

	/// Debugger for program loader
	static misc::Debug loader_debug;

	/// Run one iteration of the emulation loop.
	/// \return This function \c true if the iteration had a useful
	/// emulation, and \c false if all contexts finished execution.
	bool Run();

	//
	// Debuggers and configuration
	//

	/// Configuration for MIPS emulator
	static EmuConfig config;

	/// Debugger for x86 contexts
	static misc::Debug context_debug;
};
}
