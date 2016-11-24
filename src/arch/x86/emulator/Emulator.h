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

#ifndef ARCH_X86_EMULATOR_EMULATOR_H
#define ARCH_X86_EMULATOR_EMULATOR_H

#include <pthread.h>

#include <arch/common/Arch.h>
#include <arch/common/Emulator.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>

#include "Context.h"


namespace x86
{

class Disassembler;
class Context;

/// x86 exception
class Error : public misc::Error
{
public:

	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		/// Add module prefix
		AppendPrefix("x86");
	}
};


/// x86 emulator
class Emulator : public comm::Emulator
{
	//
	// Static fields
	//

	// Debugger files
	static std::string call_debug_file;
	static std::string context_debug_file;
	static std::string isa_debug_file;
	static std::string loader_debug_file;
	static std::string syscall_debug_file;

	// Maximum number of instructions
	static long long max_instructions;

	// Unique instance of singleton
	static std::unique_ptr<Emulator> instance;




	//
	// Class members
	//

	// Primary list of contexts
	std::list<std::unique_ptr<Context>> contexts;

	// List of running contexts
	std::list<Context *> running_contexts;

	// List of suspended contexts
	std::list<Context *> suspended_contexts;

	// List of finished contexts
	std::list<Context *> finished_contexts;

	// List of zombie contexts
	std::list<Context *> zombie_contexts;

	// Schedule next call to Emu::ProcessEvents(). The call will only be
	// effective if 'process_events_force' is set. This flag should be
	// accessed thread-safely locking the mutex.
	bool process_events_force = false;
	
	// Process ID to be assigned next. Process IDs are assigned in
	// increasing order, using function Emu::getPid()
	int pid = 100;
	
	// Emulator mutex, used to access shared variables between main program
	// and child host threads.
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	// Counter of times that a context has been suspended in a futex. Used
	// for FIFO wakeups.
	long long futex_sleep_count = 0;


public:

	//
	// Static fields
	//

	/// Get instance of singleton
	static Emulator *getInstance();

	/// Destroy the singleton if it existed
	static void Destroy() { instance = nullptr; }

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Return the maximum number of instructions, as set up by the user
	static long long getMaxInstructions() { return max_instructions; }

	/// Debugger for function calls
	static misc::Debug call_debug;

	/// Debugger for x86 contexts
	static misc::Debug context_debug;

	/// Debugger for x86 ISA emulation
	static misc::Debug isa_debug;

	/// Debugger for program loader
	static misc::Debug loader_debug;

	/// Debugger for system calls
	static misc::Debug syscall_debug;




	//
	// Class members
	//

	/// Constructor
	Emulator() : comm::Emulator("x86") { }

	/// Create a new context associated with the emulator. The context is
	/// inserted in the main emulator context list. Its state is set to
	/// ContextRunning, and it is inserted into the emulator list of running
	/// contexts.
	Context *newContext();

	/// Return a context given its pid, or null if no contexts exists with
	/// that pid.
	Context *getContext(int pid);

	/// Remove a context from all context lists and free it
	void FreeContext(Context *context);

	/// Create a context and load a program. See comm::Emu::Load() for
	/// details on the meaning of each argument.
	void LoadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env = {},
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "");

	/// Return a unique process ID. Contexts can call this function when
	/// created to obtain their unique identifier.
	int getPid() { return pid++; }

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
	// List of all contexts
	//

	/// Return an iterator to the first element of the context list
	std::list<std::unique_ptr<Context>>::iterator getContextsBegin()
	{
		return contexts.begin();
	}

	/// Return a past-the-end iterator to the context list
	std::list<std::unique_ptr<Context>>::iterator getContextsEnd()
	{
		return contexts.end();
	}

	/// Return the number of contexts
	int getNumContexts() const
	{
		return contexts.size();
	}



	//
	// List of running contexts
	//

	/// Add a context to the list of running contexts. The context must
	/// not be present in said list.
	void InsertInRunningContexts(Context *context);

	/// Remove a context from the list of running contexts. The context
	/// must be present in said list.
	void RemoveFromRunningContexts(Context *context);

	/// Update the presence of a context in the list of running contexts,
	/// regardless of its previous presence in it.
	void UpdateRunningContexts(Context *context, bool present);

	/// Return an iterator to the first element of the running context list
	std::list<Context *>::iterator getRunningContextsBegin()
	{
		return running_contexts.begin();
	}

	/// Return a past-the-end iterator to the running context list
	std::list<Context *>::iterator getRunningContextsEnd()
	{
		return running_contexts.end();
	}

	/// Return the number of running contexts
	int getNumRunningContexts() const
	{
		return running_contexts.size();
	}




	//
	// List of suspended contexts
	//

	/// Add a context to the list of suspended contexts. The context must
	/// not be present in said list.
	void InsertInSuspendedContexts(Context *context);

	/// Remove a context from the list of suspended contexts. The context
	/// must be present in said list.
	void RemoveFromSuspendedContexts(Context *context);

	/// Update the presence of a context in the list of suspended contexts,
	/// regardless of its previous presence in it.
	void UpdateSuspendedContexts(Context *context, bool present);

	/// Return an iterator to the first element of the suspended context list
	std::list<Context *>::iterator getSuspendedContextsBegin()
	{
		return suspended_contexts.begin();
	}

	/// Return a past-the-end iterator to the suspended context list
	std::list<Context *>::iterator getSuspendedContextsEnd()
	{
		return suspended_contexts.end();
	}

	/// Return the number of suspended contexts
	int getNumSuspendedContexts() const
	{
		return suspended_contexts.size();
	}




	//
	// List of finished contexts
	//

	/// Add a context to the list of finished contexts. The context must
	/// not be present in said list.
	void InsertInFinishedContexts(Context *context);

	/// Remove a context from the list of finished contexts. The context
	/// must be present in said list.
	void RemoveFromFinishedContexts(Context *context);

	/// Update the presence of a context in the list of finished contexts,
	/// regardless of its previous presence in it.
	void UpdateFinishedContexts(Context *context, bool present);

	/// Return an iterator to the first element of the finished context list
	std::list<Context *>::iterator getFinishedContextsBegin()
	{
		return finished_contexts.begin();
	}

	/// Return a past-the-end iterator to the finished context list
	std::list<Context *>::iterator getFinishedContextsEnd()
	{
		return finished_contexts.end();
	}

	/// Return the number of finished contexts
	int getNumFinishedContexts() const
	{
		return finished_contexts.size();
	}




	//
	// List of zombie contexts
	//

	/// Add a context to the list of zombie contexts. The context must
	/// not be present in said list.
	void InsertInZombieContexts(Context *context);

	/// Remove a context from the list of zombie contexts. The context
	/// must be present in said list.
	void RemoveFromZombieContexts(Context *context);

	/// Update the presence of a context in the list of zombie contexts,
	/// regardless of its previous presence in it.
	void UpdateZombieContexts(Context *context, bool present);

	/// Return an iterator to the first element of the zombie context list
	std::list<Context *>::iterator getZombieContextsBegin()
	{
		return zombie_contexts.begin();
	}

	/// Return a past-the-end iterator to the zombie context list
	std::list<Context *>::iterator getZombieContextsEnd()
	{
		return zombie_contexts.end();
	}

	/// Return the number of zombie contexts
	int getNumZombieContexts() const
	{
		return zombie_contexts.size();
	}




	//
	// Public fields
	//
	
	/// Signals a call to the scheduler Timing::Schedule() in the
	/// beginning of next cycle. This flag is set any time a context changes
	/// its state in any bit other than StateSpecMode. It can be set
	/// anywhere in the code by directly assigning a value of true. E.g.:
	/// when a system call is executed to change the context's affinity.
	bool schedule_signal = false;

};


}  // namespace x86

#endif

