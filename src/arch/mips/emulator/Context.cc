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
#include <poll.h>
#include <vector>

#include <lib/cpp/Environment.h>
#include <lib/cpp/Misc.cc>

#include "Context.h"
#include "Emulator.h"


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
		emulator->setScheduleSignal();

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
	emulator->UpdateContextInList(ContextListRunning, this, this->state & ContextRunning);
	emulator->UpdateContextInList(ContextListZombie, this, this->state & ContextZombie);
	emulator->UpdateContextInList(ContextListFinished, this, this->state & ContextFinished);
	emulator->UpdateContextInList(ContextListSuspended, this, this->state & ContextSuspended);
}

std::string Context::OpenProcSelfMaps()
{
	// Create temporary file
	int fd;
	FILE *f = NULL;
	char path[256];
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		throw misc::Panic("Cannot create temporary file");

	// Get the first page
	unsigned end = 0;
	for (;;)
	{
		// Get start of next range
		mem::Memory::Page *page = memory->getNextPage(end);
		if (!page)
			break;
		unsigned start = page->getTag();
		end = page->getTag();
		int perm = page->getPerm() & (mem::Memory::AccessRead |
				mem::Memory::AccessWrite |
				mem::Memory::AccessExec);

		// Get end of range
		for (;;)
		{
			page = memory->getPage(end + mem::Memory::PageSize);
			if (!page)
				break;
			int page_perm = page->getPerm() &
					(mem::Memory::AccessRead |
							mem::Memory::AccessWrite |
							mem::Memory::AccessExec);
			if (page_perm != perm)
				break;
			end += mem::Memory::PageSize;
			perm = page_perm;
		}

		// Dump range
		fprintf(f, "%08x-%08x %c%c%c%c 00000000 00:00\n", start,
				end + mem::Memory::PageSize,
				perm & mem::Memory::AccessRead ? 'r' : '-',
						perm & mem::Memory::AccessWrite ? 'w' : '-',
								perm & mem::Memory::AccessExec ? 'x' : '-',
										'p');
	}

	// Close file
	fclose(f);
	return path;
}

Context::Context()
{
	// Save emulator instance
	emulator = Emulator::getInstance();

	// Initialize
	pid = emulator->getPid();
	sched_policy = SCHED_RR;
	sched_priority = 1;  // Lowest priority

	wakeup_fn = nullptr;
	can_wakeup_fn = nullptr;

	// Presence in context lists
	for (int i = 0; i < ContextListCount; i++)
		context_list_present[i] = false;

	// Debug
	Emulator::context_debug << "Context " << pid << " created\n";
}


Context::~Context()
{
	// Debug
	emulator->context_debug << "Context " << pid << " destroyed\n";
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
	address_space_index = emulator->getAddressSpaceIndex();

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


void Context::HostThreadSuspend()
{
	// Get current time
	esim::Engine *esim = esim::Engine::getInstance();
	long long now = esim->getRealTime();

	// Detach this thread - we don't want the parent to have to join it to
	// release its resources. The thread termination can be observed by
	// atomically checking the 'host_thread_suspend_active' field of the
	// context.
	pthread_detach(pthread_self());

	// Suspended in system call 'nanosleep'
	if (getState(ContextNanosleep))
	{
		// Calculate remaining sleep time in microseconds
		long long timeout = syscall_nanosleep_wakeup_time > now ?
				syscall_nanosleep_wakeup_time - now : 0;
		usleep(timeout);

	}

	// Suspended in system call 'read'
	if (getState(ContextRead))
	{
		// Get file descriptor
		comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_read_fd);
		if (!desc)
			throw misc::Panic(misc::fmt("Invalid file descriptor "
					"(%d)", syscall_read_fd));

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = POLLIN;
		int err = poll(&host_fds, 1, -1);
		if (err < 0)
			throw misc::Panic("Unexpected error in host 'poll'");
	}

	// Suspended in system call 'write'
	if (getState(ContextWrite))
	{
		// Get file descriptor
		comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_write_fd);
		if (!desc)
			throw misc::Panic(misc::fmt("Invalid file descriptor "
					"(%d)", syscall_write_fd));

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = POLLOUT;
		int err = poll(&host_fds, 1, -1);
		if (err < 0)
			throw misc::Panic("Unexpected error in host 'poll'");
	}

	// Suspended in system call 'poll'
	if (getState(ContextPoll))
	{
		// Get file descriptor
		comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_poll_fd);
		if (!desc)
			throw misc::Panic(misc::fmt("Invalid file descriptor "
					"(%d)", syscall_poll_fd));

		// Calculate timeout for host call in milliseconds from now
		int timeout;
		if (!syscall_poll_time)
			timeout = -1;
		else if (syscall_poll_time < now)
			timeout = 0;
		else
			timeout = (syscall_poll_time - now) / 1000;

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = ((syscall_poll_events & 4) ? POLLOUT : 0) |
				((syscall_poll_events & 1) ? POLLIN : 0);
		int err = poll(&host_fds, 1, timeout);
		if (err < 0)
			throw misc::Panic("Unexpected error in host 'poll'");
	}

	// Event occurred - thread finishes
	emulator->LockMutex();
	emulator->ProcessEventsScheduleUnsafe();
	host_thread_suspend_active = false;
	emulator->UnlockMutex();
}


void Context::HostThreadSuspendCancelUnsafe()
{
	if (host_thread_suspend_active)
	{
		if (pthread_cancel(host_thread_suspend))
			throw misc::Panic(misc::fmt("[Context %d] Error "
					"canceling host thread", pid));
		host_thread_suspend_active = false;
	}
	emulator->ProcessEventsScheduleUnsafe();
}


void Context::HostThreadSuspendCancel()
{
	emulator->LockMutex();
	HostThreadSuspendCancelUnsafe();
	emulator->UnlockMutex();
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
	emulator->ProcessEventsSchedule();
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

	// set PC to the next instruction pointer
	regs.setPC(next_ip);

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
	inst.Decode(regs.getPC(), buffer_ptr);

	// Debug
	if (emulator->isa_debug)
	{
		ELFReader::Symbol *symbol = (loader->binary)->getSymbolByAddress(regs.getPC());
		std::string symbol_string = symbol->getName();
		if ((regs.getPC() - previous_ip) != 4)
			emulator->isa_debug << misc::fmt("\nIN %s\n", symbol_string.c_str());

		emulator->isa_debug << misc::fmt("%d %8lld %x: ", pid,
				emulator->getNumInstructions(),
				regs.getPC());
		inst.Dump(emulator->isa_debug.operator std::ostream &());
		emulator->isa_debug << misc::fmt("\n");
	}

	// Set last, current, and target instruction addresses
	previous_ip = regs.getPC();
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
	emulator->incNumInstructions();
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
	for (auto &context : emulator->getContexts())
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
	emulator->ProcessEventsSchedule();
}


void Context::Finish(int exit_code)
{
	// Context already finished
	if (getState(ContextFinished) || getState(ContextZombie))
		return;

	// Finish context
	setState(parent ? ContextZombie : ContextFinished);
	this->exit_code = exit_code;
	emulator->ProcessEventsSchedule();
}

}  // namespace MIPS

