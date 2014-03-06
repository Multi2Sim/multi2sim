/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_X86_EMU_EMU_H
#define ARCH_X86_EMU_EMU_H

#include <pthread.h>

#include <arch/common/Arch.h>
#include <arch/common/Emu.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include "Context.h"


namespace x86
{

class Asm;
class Context;


/// x86 Emulator configuration
class EmuConfig : public misc::CommandLineConfig
{
	// Debugger files
	std::string call_debug_file;
	std::string context_debug_file;
	std::string cuda_debug_file;
	std::string glut_debug_file;
	std::string isa_debug_file;
	std::string loader_debug_file;
	std::string opencl_debug_file;
	std::string opengl_debug_file;
	std::string syscall_debug_file;

	// Maximum number of instructions
	long long max_instructions;

	// Simulation kind
	comm::ArchSimKind sim_kind;

	// Process prefetch instructions
	bool process_prefetch_hints;

public:

	/// Initialization of default command-line options
	EmuConfig();

	/// Register command-line options related with the x86 emulator
	void Register(misc::CommandLine &command_line);

	/// Process command-line options related with the x86 emualtor
	void Process();

	/// Return maximum number of instructions
	long long getMaxInstructions() { return max_instructions; }

	/// Return the type of simulation
	comm::ArchSimKind getSimKind() { return sim_kind; }

	/// Return whether to process prefetch hints
	bool getProcessPrefetchHints() { return process_prefetch_hints; }
};


/// x86 emulator
class Emu : public comm::Emu
{
	// Unique instance of x86 emulator
	static std::unique_ptr<Emu> instance;

	// Primary list of contexts
	std::list<std::unique_ptr<Context>> contexts;

	// Secondary lists of contexts. Contexts in different states
	// are added/removed from this lists as their state gets updated.
	std::list<Context *> context_list[ContextListCount];

	// See setScheduleSignal()
	bool schedule_signal;

	// Private constructor. The only possible instance of the x86 emulator
	// can be obtained with a call to getInstance()
	Emu();
	
	// Schedule next call to Emu::ProcessEvents(). The call will only be
	// effective if 'process_events_force' is set. This flag should be
	// accessed thread-safely locking the mutex.
	bool process_events_force;
	
	// Process ID to be assigned next. Process IDs are assigned in
	// increasing order, using function Emu::getPid()
	int pid;
	
	// Index of virtual memory space assigned to new contexts. A new ID
	// can be retrieved in increasing order by using function
	// Emu::getAddressSpaceIndex()
	int address_space_index;
	
	// Emulator mutex, used to access shared variables between main program
	// and child host threads.
	pthread_mutex_t mutex;

	// Counter of times that a context has been suspended in a futex. Used
	// for FIFO wakeups.
	long long futex_sleep_count;


public:

	/// The x86 emulator is a singleton class. The only possible instance of
	/// it will be allocated the first time this function is invoked.
	static Emu *getInstance();

	/// Create a new context associated with the emulator. The context is
	/// inserted in the main emulator context list. Its state is set to
	/// ContextRunning, and it is inserted into the emulator list of running
	/// contexts.
	Context *newContext();

	/// Return a context given its pid, or null if no contexts exists with
	/// that pid.
	Context *getContext(int pid);

	/// Remove a context from all context lists and free it
	void freeContext(Context *context);

	/// Add a context to a context list if it is not present already
	void AddContextToList(ContextListType type, Context *context);

	/// Remove a context from a context list if present
	void RemoveContextFromList(ContextListType type, Context *context);

	/// Update the presence of a context in a list depending on the value
	/// passed in argument \a present. If \c true, the context will be added
	/// to the list (if not present already. If \c false, it will be removed
	/// (if still present).
	void UpdateContextInList(ContextListType type, Context *context,
			bool present);

	/// Return a constant reference to a list of contexts
	const std::list<Context *> &getContextList(ContextListType type) const {
			return context_list[type]; }

	/// Signals a call to the scheduler Timing::Schedule() in the
	/// beginning of next cycle. This flag is set any time a context changes
	/// its state in any bit other than ContextSpecMode. It can be set
	/// anywhere in the code by directly assigning a value to 1. E.g.:
	/// when a system call is executed to change the context's affinity.
	void setScheduleSignal() { schedule_signal = true; }

	/// Return a unique process ID. Contexts can call this function when
	/// created to obtain their unique identifier.
	int getPid() { return pid++; }

	/// Return a unique increasing ID for a virtual memory space for
	/// contexts.
	int getAddressSpaceIndex() { return address_space_index++; }

	/// Get reference to the main context list
	std::list<std::unique_ptr<Context>> &getContexts() { return contexts; }

	/// Lock the emulator mutex
	void LockMutex() { pthread_mutex_lock(&mutex); }

	/// Unlock the emulator mutex
	void UnlockMutex() { pthread_mutex_unlock(&mutex); }

	// Check for events detected in spawned host threads, such as waking up
	// contexts or sending signals. The list is only effectively processed
	// if events have been scheduled to get processed with a previous call
	// to ProcessEventsSchedule().
	void ProcessEvents();

	/// Increment an internal counter for futex identifiers, and return its
	/// new value. This function is used to assign futex identifiers used as
	/// event timestamps.
	long long incFutexSleepCount() { return ++futex_sleep_count; }

	/// Schedule a call to ProcessEvents(). This call internally locks the
	/// emulator mutex.
	void ProcessEventsSchedule();
	
	/// Schedule next call to ProcessEvents(). The emulator mutex must be
	/// locked before invoking this function.
	void ProcessEventsScheduleUnsafe() { process_events_force = true; }

	/// Run one iteration of the emulation loop.
	/// \return This function \c true if the iteration had a useful
	/// emulation, and \c false if all contexts finished execution.
	bool Run();


	//
	// Debuggers and configuration
	//

	/// Debugger for function calls
	static misc::Debug call_debug;

	/// Debugger for x86 contexts
	static misc::Debug context_debug;

	/// Debugger for CUDA driver
	static misc::Debug cuda_debug;

	/// Debugger for GLUT driver
	static misc::Debug glut_debug;
	
	/// Debugger for x86 ISA emulation
	static misc::Debug isa_debug;

	/// Debugger for program loader
	static misc::Debug loader_debug;

	/// Debugger for OpenCL driver
	static misc::Debug opencl_debug;

	/// Debugger for OpenGL driver
	static misc::Debug opengl_debug;
	
	/// Debugger for system calls
	static misc::Debug syscall_debug;

	/// Configuration for x86 emulator
	static EmuConfig config;
};


}  // namespace x86

#endif

