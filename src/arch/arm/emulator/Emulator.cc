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

#include "Context.h"
#include "Emulator.h"


namespace ARM
{


//
// Configuration variables
//

// Debug files
std::string Emulator::loader_debug_file;
std::string Emulator::isa_debug_file;
std::string Emulator::context_debug_file;
std::string Emulator::syscall_debug_file;

// Maximum number of instructions
long long Emulator::max_instructions;



//
// Static variables
//

// Emulator singleton
std::unique_ptr<Emulator> Emulator::instance;


// Debuggers
misc::Debug Emulator::context_debug;
misc::Debug Emulator::isa_debug;
misc::Debug Emulator::loader_debug;
misc::Debug Emulator::syscall_debug;



//
// Functions
//

void Emulator::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("ARM");

	// Option --arm-debug-ctx <file>
	command_line->RegisterString("--arm-debug-ctx <file>", context_debug_file,
			"Dump debug information related with context creation, "
			"destruction, allocation, or state change.");

	// Option --arm-debug-isa <file>
	command_line->RegisterString("--arm-debug-isa <file>", isa_debug_file,
			"Debug information for dynamic execution of ARM "
			"instructions. Updates on the processor state can be "
			"analyzed using this information.");

	// Option --arm-debug-loader <file>
	command_line->RegisterString("--arm-debug-loader <file>", loader_debug_file,
			"Dump debug information extending the analysis of the "
			"ELF program binary. This information shows which ELF "
			"sections and symbols are loaded to the initial program "
			"memory image.");

	// Option --arm-debug-syscall <file>
	command_line->RegisterString("--arm-debug-syscall <file>", syscall_debug_file,
			"Debug information for system calls performed by a ARM "
			"program, including system call code, arguments, and "
			"return value.");
}


void Emulator::ProcessOptions()
{
	// Debuggers
	context_debug.setPath(context_debug_file);
	isa_debug.setPath(isa_debug_file);
	loader_debug.setPath(loader_debug_file);
	syscall_debug.setPath(syscall_debug_file);
}


Emulator::Emulator() : comm::Emulator("ARM")
{
	// Initialize
	pid = 100;
	process_events_force = false;
	schedule_signal = false;
	futex_sleep_count = 0;
	address_space_index = 0;
}

void Emulator::AddContextToList(ContextListType type, Context *context)
{
	// Nothing if already present
	if (context->context_list_present[type])
		return;

	// Add context
	context->context_list_present[type] = true;
	context_list[type].push_back(context);
	auto iter = context_list[type].end();
	context->context_list_iter[type] = --iter;
}

void Emulator::RemoveContextFromList(ContextListType type, Context *context)
{
	// Nothing if not present
	if (!context->context_list_present[type])
		return;

	// Remove context
	context->context_list_present[type] = false;
	auto iter = context->context_list_iter[type];
	context_list[type].erase(iter);
}

void Emulator::UpdateContextInList(ContextListType type, Context *context,
			bool present)
{
	if (present && !context->context_list_present[type])
		AddContextToList(type, context);
	else if (!present && context->context_list_present[type])
		RemoveContextFromList(type, context);
}

Emulator *Emulator::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Emulator());
	return instance.get();
}


Context *Emulator::newContext()
{
	// Create context and add to context list
	Context *context = new Context();
	contexts.emplace_back(context);

	// Save position in context list
	auto iter = contexts.end();
	context->contexts_iter = --iter;

	// Set the context in running state. This call will add it automatically
	// to the emulator list of running contexts.
	context->setState(ContextStateRunning);

	// Return
	return context;
}


void Emulator::LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// Create new context
	Context *context = newContext();

	context->Load(args,
			env,
			cwd,
			stdin_file_name,
			stdout_file_name);
}


void Emulator::freeContext(Context *context)
{
	// Remove context from all context lists
	for (int i = 0; i < ContextListCount; i++)
		RemoveContextFromList((ContextListType) i, context);

	// Remove from main context list. This will invoke the context
	// destructor and free it.
	contexts.erase(context->contexts_iter);
}

void Emulator::ProcessEventsSchedule()
{
	LockMutex();
	process_events_force = true;
	UnlockMutex();
}

void Emulator::ProcessEvents()
{
	// Check if events need actually be checked.
	LockMutex();
	if (!process_events_force)
	{
		UnlockMutex();
		return;
	}

	// By default, no subsequent call to ProcessEvents() is assumed
	process_events_force = false;


	//
	// LOOP 1
	// Look at the list of suspended contexts and try to find
	// one that needs to be waken up.
	//
	auto &suspended_context_list = getContextList(ContextListSuspended);
	auto iter_next = suspended_context_list.end();
	for (auto iter = suspended_context_list.begin();
				iter != suspended_context_list.end();
				iter = iter_next)
	{
		// Save iterator to next element here, since the context can be
		// removed from the suspended list if waken up.
		iter_next = iter;
		++iter_next;
		Context *context = *iter;
		assert(context->getState(ContextStateSuspended));
		assert(context->context_list_iter[ContextListSuspended] == iter);
		assert(context->context_list_present[ContextListSuspended]);

		// Context suspended in a system call using a custom wake up
		// check call-back function. NOTE: this is a new mechanism. It'd
		// be nice if all other system calls started using it. It is
		// nicer, since it allows for a check of wake up conditions
		// together with the system call itself, without having
		// distributed code for the implementation of a system call
		// (e.g. 'read').
		if (context->getState(ContextStateCallback) && context->CanWakeup())
		{
			context->Wakeup();
			continue;
		}
	}

	//
	// LOOP 3
	// Process pending signals in running contexts to launch signal handlers
	//
	for (Context *context : context_list[ContextListRunning])
		context->CheckSignalHandler();

	// Unlock
	UnlockMutex();
}


bool Emulator::Run()
{
	// Stop if there is no more contexts
	if (!contexts.size())
		return false;

	// Stop if maximum number of CPU instructions exceeded
	//if (max_instructions && instructions >= max_instructions)
	//	esim->Finish("ARMMaxInst");

	// Stop if any previous reason met
	if (esim->hasFinished())
		return true;

	// Run an instruction from every running context. During execution, a
	// context can remove itself from the running list, so we need to save
	// the iterator to the next element before executing.
	auto end = context_list[ContextListRunning].end();
	for (auto it = context_list[ContextListRunning].begin(); it != end; )
	{
		// Save position of next context
		auto next = it;
		++next;

		// Run one iteration
		Context *context = *it;
		context->Execute();

		// Move to saved next context
		it = next;
	}

	// Free finished contexts
	while (context_list[ContextListFinished].size())
		freeContext(context_list[ContextListFinished].front());

	// Process list of suspended contexts
	ProcessEvents();

	// Still running
	return true;
}

} // namespace ARM
