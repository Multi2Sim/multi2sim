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

#include <iostream>
#include <poll.h>
#include <vector>
#include <cstring>
#include <unistd.h>

#include <lib/cpp/Environment.h>
#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emulator.h"


namespace ARM
{

void Context::UpdateState(unsigned state)
{
	// If the difference between the old and new state lies in other
	// states other than 'ContextSpecMode', a reschedule is marked. */
	unsigned diff = this->state ^ state;
	if (diff & ~ContextStateSpecMode)
		emulator->setScheduleSignal();

	// Update state
	this->state = state;
	if (this->state & ContextStateFinished)
		this->state = ContextStateFinished
		| (state & ContextStateAlloc);
	if (this->state & ContextStateZombie)
		this->state = ContextStateZombie
		| (state & ContextStateAlloc);
	if (!(this->state & ContextStateSuspended) &&
			!(this->state & ContextStateFinished) &&
			!(this->state & ContextStateZombie) &&
			!(this->state & ContextStateLocked))
		this->state |= ContextStateRunning;
	else
		this->state &= ~ContextStateRunning;

	// Update presence of context in emulator lists depending on its state
	emulator->UpdateContextInList(ContextListRunning, this, this->state & ContextStateRunning);
	emulator->UpdateContextInList(ContextListZombie, this, this->state & ContextStateZombie);
	emulator->UpdateContextInList(ContextListFinished, this, this->state & ContextStateFinished);
	emulator->UpdateContextInList(ContextListSuspended, this, this->state & ContextStateSuspended);
	emulator->UpdateContextInList(ContextListAlloc, this, this->state & ContextListAlloc);
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
	state = 0;
	glibc_segment_base = 0;
	glibc_segment_limit = 0;
	pid = emulator->getPid();
	parent = nullptr;
	group_parent = nullptr;
	exit_signal = 0;
	exit_code = 0;
	clear_child_tid = 0;
	robust_list_head = 0;
	host_thread_suspend_active = false;
	host_thread_timer_active = false;
	sched_policy = SCHED_RR;
	sched_priority = 1;  // Lowest priority

	wakeup_fn = nullptr;
	can_wakeup_fn = nullptr;

	// Presence in context lists
	for (int i = 0; i < ContextListCount; i++)
		context_list_present[i] = false;

	// Debug
	emulator->context_debug << "Context " << pid << " created\n";
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
	//FIXME signal_handler_table.reset(new SignalHandlerTable());

	// Create speculative memory, and link it with the real memory
	spec_mem.reset(new mem::SpecMem(memory.get()));

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

	// Create Arm-Thumb Symbol List
	for (int i = 0; i < loader->binary->getNumSymbols(); i++)
	{
		if (!(loader->binary->getSymbol(i)->getName().compare(0, 2, "$a"))
				|| !(loader->binary->getSymbol(i)->getName().compare(0, 2, "$t")))
		{
			ELFReader::Symbol *symbolPtr = loader->binary->getSymbol(i);
			thumb_symbol_list.push_back(symbolPtr);
		}
	}
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
	if (getState(ContextStateNanosleep))
	{
		// Calculate remaining sleep time in microseconds
		long long timeout = syscall_nanosleep_wakeup_time > now ?
				syscall_nanosleep_wakeup_time - now : 0;
		usleep(timeout);

	}

	// Suspended in system call 'read'
	if (getState(ContextStateRead))
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
	if (getState(ContextStateWrite))
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
	if (getState(ContextStatePoll))
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
	assert(!getState(ContextStateSuspended));
	assert(!this->can_wakeup_fn);
	assert(!this->wakeup_fn);

	// Store callbacks and data
	this->can_wakeup_fn = can_wakeup_fn;
	this->wakeup_fn = wakeup_fn;
	this->wakeup_state = wakeup_state;

	// Suspend context
	setState(ContextStateSuspended);
	setState(ContextStateCallback);
	setState(wakeup_state);
	emulator->ProcessEventsSchedule();
}


bool Context::CanWakeup()
{
	// Checks
	assert(getState(ContextStateCallback));
	assert(getState(ContextStateSuspended));
	assert(this->can_wakeup_fn);

	// Invoke callback
	return (this->*can_wakeup_fn)();
}


void Context::Wakeup()
{
	// Checks
	assert(getState(ContextStateCallback));
	assert(getState(ContextStateSuspended));
	assert(this->wakeup_fn);

	// Wakeup context
	(this->*wakeup_fn)();
	clearState(ContextStateCallback);
	clearState(ContextStateSuspended);
	clearState(wakeup_state);

	// Reset callbacks and free data
	can_wakeup_fn = nullptr;
	wakeup_fn = nullptr;
}


void Context::Execute()
{
	// Memory permissions should not be checked if the context is executing in
	// speculative mode. This will prevent guest segmentation faults to occur.
	bool spec_mode = getState(ContextStateSpecMode);
	if (spec_mode)
		memory->setSafe(false);
	else
		memory->setSafeDefault();

	// Change the related register according to the fault ID
	unsigned int helper_id = CheckKuserHelper();
	if (helper_id != 0)
	{
		regs.setPC(regs.getLR() + 4);

		if(helper_id == 0xffff0fe0)
		{
			// Get TLS value
			regs.setRegister(0, regs.getCP15().c13_tls3);
		}
		else if(helper_id == 0xffff0fc0)
		{
			regs.setRegister(3, 0);
		}
		else if(helper_id == 0xffff0fa0)
		{

		}
	}

	// Get ARM operating mode
	ContextMode arm_mode = OperateMode(regs.getPC() - 2);

	// Change PC according to the thumb field in CPSR
	if (arm_mode == ContextModeArm)
	{
		if(regs.getCPSR().thumb != 0)
		{
			regs.getCPSR().thumb = 0;
			regs.incPC(2);
		}
		else
		{
			regs.getCPSR().thumb = 0;
		}
	}
	else if (arm_mode == ContextModeThumb)
	{
		if (regs.getCPSR().thumb == 0)
		{
			regs.getCPSR().thumb = 1;
			regs.decPC(2);
		}
		else
		{
			regs.getCPSR().thumb = 1;
		}
	}

	// Get buffer according to the Program Counter
	char *buffer_ptr;
	if (regs.getCPSR().thumb != 0)
		buffer_ptr = memory->getBuffer((regs.getPC() - 2), 2,
					mem::Memory::AccessExec);
	else
		buffer_ptr = memory->getBuffer((regs.getPC() - 4), 4,
					mem::Memory::AccessExec);

	// Return to default safe mode
	memory->setSafeDefault();

	// Disassemble
	if (regs.getCPSR().thumb != 0)
	{
		if (IsThumb32(buffer_ptr))
		{
			regs.incPC(2);
			buffer_ptr = memory->getBuffer((regs.getPC() - 4), 4,
					mem::Memory::AccessExec);
			inst.Thumb32Decode(buffer_ptr, (regs.getPC() - 4));
			setInstType(ContextInstTypeThumb32);
			if (inst.getThumb32Opcode() == Instruction::Thumb32OpcodeInvalid)
				throw misc::Panic(misc::fmt("0x%x: not supported arm instruction (%02x %02x %02x %02x...)",
					(regs.getPC() - 4), buffer_ptr[0], buffer_ptr[1], buffer_ptr[2], buffer_ptr[3]));
		}
		else
		{
			inst.Thumb16Decode(buffer_ptr, (regs.getPC() - 2));
			setInstType(ContextInstTypeThumb16);
		}
	}
	else
	{
		inst.Decode((regs.getPC() - 4), buffer_ptr);
		setInstType(ContextInstTypeArm32);
		if (inst.getOpcode() == Instruction::OpcodeInvalid)
			throw misc::Panic(misc::fmt("0x%x: not supported arm instruction (%02x %02x %02x %02x...)",
					(regs.getPC() - 4), buffer_ptr[0], buffer_ptr[1], buffer_ptr[2], buffer_ptr[3]));
	}

	// Execute instruction
	if(iteq_inst_num != 0 && iteq_block_flag != 0)
	{
		//TODO
	}
	else
	{
		ExecuteInst();
	}

	// Stats
	emulator->incNumInstructions();
}


void Context::ExecuteInst()
{
	// Set last, current, and target instruction addresses
	last_ip = current_ip;
	if(regs.getCPSR().thumb != 0)
	{
		// Thumb2 Mode
		if(getInstType() == ContextInstTypeThumb16)
		{
			current_ip = regs.getPC() - 2;
		}
		else if (getInstType() == ContextInstTypeThumb32)
		{
			current_ip = regs.getPC() - 4;
		}
		else
		{
			throw misc::Panic(misc::fmt("%d : ARM Wrong Instruction Type \n", getInstType()));
		}
	}
	else
	{
		// ARM mode
		current_ip = regs.getPC() - 4;
	}
	target_ip = 0;

	// Debug
	if (emulator->isa_debug)
	{
		emulator->isa_debug << misc::fmt("%d %8lld %x: ", pid,
				emulator->getNumInstructions(), current_ip);
		if (regs.getCPSR().thumb != 0)
		{
			if(getInstType() == ContextInstTypeThumb32)
			{
				inst.Thumb32Dump(emulator->isa_debug.operator std::ostream &());
			}
			else if (getInstType() == ContextInstTypeThumb16)
			{
				inst.Thumb16Dump(emulator->isa_debug.operator std::ostream &());
			}
		}
		else
		{
			inst.Dump(emulator->isa_debug.operator std::ostream &());
			emulator->isa_debug << misc::fmt("  (%d bytes)", 4);
		}
		//emu->isa_debug << misc::fmt("\n");
	}

	// Call instruction emulation function
	switch (getInstType())
	{
	case ContextInstTypeArm32:

		regs.incPC(4);
		if (inst.getOpcode())
		{
			ExecuteInstFn fn = execute_inst_fn[inst.getOpcode()];
			(this->*fn)();
		}
		break;

	case ContextInstTypeThumb16:

		regs.incPC(2);
		if (inst.getThumb16Opcode())
		{
			ExecuteInstThumb16Fn fn = execute_inst_thumb16_fn[inst.getThumb16Opcode()];
			(this->*fn)();
		}
		break;

	case ContextInstTypeThumb32:

		regs.incPC(2);
		if (inst.getThumb32Opcode())
		{
			ExecuteInstThumb32Fn fn = execute_inst_thumb32_fn[inst.getThumb32Opcode()];
			(this->*fn)();
		}
		break;

	default:
		throw misc::Panic(misc::fmt("Invalid instruction type"));
	}

	// debug
	emulator->isa_debug << misc::fmt("\n");
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
	if (getState(ContextStateFinished) || getState(ContextStateZombie))
		return;

	// Finish all contexts in the group
	for (auto &context : emulator->getContexts())
	{
		if (context->group_parent != this && context.get() != this)
			continue;

		if (context.get() == this)
			context->setState(context->parent ? ContextStateZombie : ContextStateFinished);
		else
			context->setState(ContextStateFinished);
		context->exit_code = exit_code;
	}

	// Process events
	emulator->ProcessEventsSchedule();
}


void Context::Finish(int exit_code)
{
	// Context already finished
	if (getState(ContextStateFinished) || getState(ContextStateZombie))
		return;

	// Finish context
	setState(parent ? ContextStateZombie : ContextStateFinished);
	this->exit_code = exit_code;
	emulator->ProcessEventsSchedule();
}


ContextMode Context::OperateMode(unsigned int addr)
{
	ELFReader::Symbol *symbol;
	ContextMode mode;

	// Locate the symbol tag in the sorted symbol list by input address
	unsigned int tag_index;
	for (unsigned int i = 0; i < thumb_symbol_list.size(); ++i)
	{
		symbol = thumb_symbol_list[i];
		if(symbol->getValue() > addr)
		{
			tag_index = i - 1;
			break;
		}
	}

	// Get the ARM operating mode based on the symbol
	if (tag_index < 0 || tag_index >= thumb_symbol_list.size())
	{
		symbol = nullptr;
	}
	else
	{
		symbol = thumb_symbol_list[tag_index];
	}
	if(symbol)
	{
		if(!(symbol->getName().compare(0, 2, "$a")))
		{
			mode = ContextModeArm;
		}
		else if(!(symbol->getName().compare(0, 2, "$t")))
		{
			mode = ContextModeThumb;
		}
	}
	else
	{
		mode = ContextModeArm;
	}
	return mode;
}


unsigned int Context::CheckKuserHelper()
{
	//FIXME
	unsigned int ret_val = 0;
	switch (regs.getPC())
	{
	case 0xffff0fe0 + 4:

		emulator->isa_debug <<
			misc::fmt("  TLS Helper handled\n Helper location : 0x%x\n pc restored at : 0x%x\n\n",
				regs.getPC(), regs.getLR());
		ret_val = 0xffff0fe0;
		break;

	case 0xffff0fc0 + 4:

		emulator->isa_debug <<
			misc::fmt("  Fault handled\n Fault location : 0x%x\n pc restored at : 0x%x\n\n",
				regs.getPC(), regs.getLR());
		ret_val = 0xffff0fc0;
		break;

	case 0xffff0fa0 + 4:

		emulator->isa_debug <<
			misc::fmt("  Fault handled\n Fault location : 0x%x\n pc restored at : 0x%x\n\n",
				regs.getPC(), regs.getLR());
		ret_val = 0xffff0fa0;
		break;

	default:

		ret_val = 0;
		break;
	}
	return (ret_val);
}


bool Context::IsThumb32(const char *inst_ptr)
{
	// Get the char information from the address
	unsigned char bytes[4];
	for (unsigned int byte_index = 0; byte_index < 4; ++byte_index)
		bytes[byte_index] = *(inst_ptr + byte_index);

	// Get the argument that used to check thumb 32
	unsigned int arg1 = (bytes[1] & 0xf8) >> 3;
	if ((arg1 == 0x1d) || (arg1 == 0x1e) || (arg1 == 0x1f))
	{
		return true;
	}
	else
	{
		return false;
	}
}

}  // namespace ARM
