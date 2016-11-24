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

#include <lib/cpp/Misc.h>

#include <arch/southern-islands/emulator/NDRange.h>

#include "Driver.h"


namespace SI
{

// Forward declarations                                                          
class Disassembler; 


const char *Driver::call_name[CallCodeCount] =
{
	"Invalid",  // For code 0
#define DEFCALL(name, code) #name,
#include "Driver.def"
#undef DEFCALL
};


const Driver::CallFn Driver::call_fn[CallCodeCount] =
{
	nullptr,  // For code 0
#define DEFCALL(name, code) &Driver::Call##name,
#include "Driver.def"
#undef DEFCALL
};


std::string Driver::debug_file;

std::unique_ptr<Driver> Driver::instance;

misc::Debug Driver::debug;


Driver *Driver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance = misc::new_unique<Driver>();
	return instance.get();
}
	

int Driver::Call(comm::Context *context,
		mem::Memory *memory,
		int code,
		unsigned args_ptr)
{
	// Check valid call
	if (code < 0 || code >= CallCodeCount || !call_fn[code])
	{
		debug << misc::fmt("Invalid call code (%d)\n", code);
		return -1;
	}

	// Debug
	debug << misc::fmt("ABI call '%s'\n", call_name[code]);

	// Invoke call
	CallFn fn = call_fn[code];
	return (this->*fn)(context, memory, args_ptr);
}


void Driver::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Southern Islands");

	// Option '--si-debug-driver <file>'
	command_line->RegisterString("--si-debug-driver <file>", debug_file,
			"Dump debug information for the Southern Islands driver, "
			"including all ABI calls coming from the runtime.");	
}


void Driver::ProcessOptions()
{
	debug.setPath(debug_file);
	debug.setPrefix("[Southern Islands driver]");
}


Program *Driver::AddProgram(int program_id)
{
	// Create new program and insert it to program list
	programs.emplace_back(misc::new_unique<Program>(program_id));

	// Return
	return programs.back().get();
}


Kernel *Driver::AddKernel(int kernel_id, const std::string &func, Program *program)
{
	// Create new kernel and insert it to program list
	kernels.emplace_back(misc::new_unique<Kernel>(kernel_id, func, program));

	// Return
	return kernels.back().get();
}


}  // namepsace SI

