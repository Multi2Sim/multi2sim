/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu(dudlykoo@gmail.com)
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
#include <iostream>
#include <vector>

#include <lib/cpp/Environment.h>
#include <lib/cpp/Misc.cc>

#include "Context.h"
#include "Emu.h"

namespace MIPS
{
//
// Private functions
//


void Context::UpdateState(unsigned state)
{
	// If the difference between the old and new state lies in other
	// states other than 'ContextSpecMode', a reschedule is marked. */
	unsigned diff = this->state ^ state;
	if (diff & ~ContextSpecMode)
		emu->setScheduleSignal();

	// Update state
	this->state = state;
	if (this->state & ContextFinished)
		this->state = ContextFinished
		| (state & ContextAlloc)
		| (state & ContextMapped);
	if (this->state & ContextZombie)
		this->state = ContextZombie
		| (state & ContextAlloc)
		| (state & ContextMapped);
	if (!(this->state & ContextSuspended) &&
			!(this->state & ContextFinished) &&
			!(this->state & ContextZombie) &&
			!(this->state & ContextLocked))
		this->state |= ContextRunning;
	else
		this->state &= ~ContextRunning;

	// Update presence of context in emulator lists depending on its state
	emu->UpdateContextInList(ContextListRunning, this, this->state & ContextRunning);
	emu->UpdateContextInList(ContextListZombie, this, this->state & ContextZombie);
	emu->UpdateContextInList(ContextListFinished, this, this->state & ContextFinished);
	emu->UpdateContextInList(ContextListSuspended, this, this->state & ContextSuspended);
}


Context::Context()
{
	// Save emulator instance
	emu = Emu::getInstance();

	// Initialize
	pid = emu->getPid();
	sched_policy = SCHED_RR;
	sched_priority = 1;  // Lowest priority

	wakeup_fn = nullptr;
	can_wakeup_fn = nullptr;

	// Presence in context lists
	for (int i = 0; i < ContextListCount; i++)
		context_list_present[i] = false;
}


Context::~Context()
{
	// Debug
	emu->context_debug << "Context " << pid << " destroyed\n";
}


void Context::Load(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// String in 'args' must contain at least one non-empty element
	if (!args.size() || args[0].empty())
		misc::panic("%s: function invoked with no program name, or with an "
				"empty program.", __FUNCTION__);

	// Program must not have been loaded before
	if (loader.get() || memory.get())
		misc::panic("%s: program '%s' has already been loaded in a "
				"previous call to this function.",
				__FUNCTION__, args[0].c_str());

	// Create new memory image
	assert(!memory.get());
	memory.reset(new mem::Memory());
	address_space_index = emu->getAddressSpaceIndex();

	// Create signal handler table
	signal_handler_table.reset(new SignalHandlerTable());

	// Create speculative memory, and link it with the real memory
	spec_mem.reset(new mem::SpecMem(memory.get()));

	// Create file descriptor table
	file_table.reset(new comm::FileTable());

	// Create file descriptor table
	file_table.reset(new comm::FileTable());

	// Create new loader info
	assert(!loader.get());
	loader.reset(new Loader());
	loader->exe = misc::getFullPath(args[0], cwd);
	loader->args = args;
	loader->cwd = cwd.empty() ? misc::getCwd() : cwd;
	loader->stdin_file_name = stdin_file_name;
	loader->stdout_file_name = stdout_file_name;

	// Add environment variables
	misc::Environment *environment = misc::Environment::getInstance();
	for (auto variable : environment->getVariables())
		loader->env.emplace_back(variable);
	for (auto &var : env)
		loader->env.emplace_back(var);

	// Create call stack
	call_stack.reset(new comm::CallStack(loader->exe));

	// Load the binary
	LoadBinary();
}


void Context::Suspend(CanWakeupFn can_wakeup_fn, WakeupFn wakeup_fn,
		ContextState wakeup_state)
{
	// Checks
	assert(!getState(ContextSuspended));
	assert(!this->can_wakeup_fn);
	assert(!this->wakeup_fn);

	// Store callbacks and data
	this->can_wakeup_fn = can_wakeup_fn;
	this->wakeup_fn = wakeup_fn;
	this->wakeup_state = wakeup_state;

	// Suspend context
	setState(ContextSuspended);
	setState(ContextCallback);
	setState(wakeup_state);
	emu->ProcessEventsSchedule();
}


bool Context::CanWakeup()
{
	// Checks
	assert(getState(ContextCallback));
	assert(getState(ContextSuspended));
	assert(this->can_wakeup_fn);

	// Invoke callback
	return (this->*can_wakeup_fn)();
}


void Context::Wakeup()
{
	// Checks
	assert(getState(ContextCallback));
	assert(getState(ContextSuspended));
	assert(this->wakeup_fn);

	// Wakeup context
	(this->*wakeup_fn)();
	clearState(ContextCallback);
	clearState(ContextSuspended);
	clearState(wakeup_state);

	// Reset callbacks and free data
	can_wakeup_fn = nullptr;
	wakeup_fn = nullptr;
}


void Context::Execute()
{
	// Memory permissions should not be checked if the context is executing in
	// speculative mode. This will prevent guest segmentation faults to occur.
	bool spec_mode = getState(ContextSpecMode);
	if (spec_mode)
		memory->setSafe(false);
	else
		memory->setSafeDefault();

	// read 4 bytes mips instruction from memory into buffer
	char buffer[4];
	char *buffer_ptr = memory->getBuffer(regs.getPC(), 4,
				mem::Memory::AccessExec);
	if (!buffer_ptr)
	{
		// Disable safe mode. If a part of the 4 read bytes does not
		// belong to the actual instruction, and they lie on a page with
		// no permissions, this would generate an undesired protection
		// fault.
		memory->setSafe(false);
		buffer_ptr = buffer;
		memory->Access(regs.getPC(), 4, buffer_ptr,
				mem::Memory::AccessExec);
	}

	// Return to default safe mode
	memory->setSafeDefault();

	// Disassemble
	inst.Decode(regs.getPC(),buffer_ptr);

	// Set last, current, and target instruction addresses
	regs.setPC(next_ip);
	next_ip = n_next_ip;
	n_next_ip += 4;

	// Reset effective address
	effective_address = 0;

	// Call instruction emulation function
	if (inst.getOpcode())
	{
		ExecuteInstFn fn = execute_inst_fn[inst.getOpcode()];
		(this->*fn)();
	}

	// Stats
	emu->incInstructions();
}


void Context::FinishGroup(int exit_code)
{
	// Make call on group parent only
	if (group_parent)
	{
		assert(!group_parent->group_parent);
		group_parent->FinishGroup(exit_code);
		return;
	}

	// Context already finished
	if (getState(ContextFinished) || getState(ContextZombie))
		return;

	// Finish all contexts in the group
	for (auto &context : emu->getContexts())
	{
		if (context->group_parent != this && context.get() != this)
			continue;

		if (context.get() == this)
			context->setState(context->parent ? ContextZombie : ContextFinished);
		else
			context->setState(ContextFinished);
		context->exit_code = exit_code;
	}

	// Process events
	emu->ProcessEventsSchedule();
}


void Context::Finish(int exit_code)
{
	// Context already finished
	if (getState(ContextFinished) || getState(ContextZombie))
		return;

	// Finish context
	setState(parent ? ContextZombie : ContextFinished);
	this->exit_code = exit_code;
	emu->ProcessEventsSchedule();
}

}  // namespace MIPS

