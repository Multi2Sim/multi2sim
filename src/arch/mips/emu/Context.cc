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

#include <lib/cpp/Misc.cc>

#include "Context.h"
#include "Emu.h"

// Variable used in function Context::loadProgram()
extern char **environ;

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
}

Context::Context()
{
	// Save emulator instance
	emu = Emu::getInstance();


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
	file_table.reset(new FileTable());

	// Create file descriptor table
	file_table.reset(new FileTable());

	// Create new loader info
	assert(!loader.get());
	loader.reset(new Loader());
	loader->args = args;
	loader->cwd = cwd;
	loader->stdin_file_name = stdin_file_name;
	loader->stdout_file_name = stdout_file_name;

	// Add environment variables
	for (int i = 0; environ[i]; i++)
		loader->env.emplace_back(environ[i]);
	for (auto &var : env)
		loader->env.emplace_back(var);

	// Load the binary
	LoadBinary();
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

	/// read 4 bytes mips instruction from memory into buffer
	char buffer[4];
	char *buffer_ptr = memory->getBuffer(regs.getPC(), 4,
				mem::MemoryAccessExec);
	if (!buffer_ptr)
	{
		// Disable safe mode. If a part of the 4 read bytes does not
		// belong to the actual instruction, and they lie on a page with
		// no permissions, this would generate an undesired protection
		// fault.
		memory->setSafe(false);
		buffer_ptr = buffer;
		memory->Access(regs.getPC(), 4, buffer_ptr,
			mem::MemoryAccessExec);
	}

	// Return to default safe mode
	memory->setSafeDefault();

	// Disassemble
	inst->Decode(regs.getPC(),buffer_ptr);

	// Set last, current, and target instruction addresses
	last_eip = current_eip;
	current_eip = regs.getPC();
	target_eip = 0;

	// Reset effective address
	effective_address = 0;

	// Advance Program Counter to the size of instruction (4 bytes)
	regs.incPC(4);

	// Call instruction emulation function
	if(inst->GetOpcode())
	{
		//ExecuteInstFn fn = execute_inst_fn[inst->GetOpcode()];
		//(this->*fn)();
	}

	// Stats
	emu->incInstructions();
}


}  // namespace MIPS

