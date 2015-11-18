/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <cstring>

#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/emulator/Emulator.h>
#include <arch/hsa/emulator/StackFrame.h>
#include <arch/hsa/emulator/WorkItem.h>

#include "Driver.h"
#include "HsaExecutable.h"
#include "SignalDestroyHandler.h"


namespace HSA
{

// Initialize table of ABI call names
const char *Driver::call_name[CallCodeCount] =
{
	"Invalid",  // For code 0
#define DEFCALL(name, code, func) #name,
#include "Driver.def"
#undef DEFCALL
};


// Initialize table of ABI call functions
const Driver::CallFn Driver::call_fn[CallCodeCount] =
{
	nullptr,  // For code 0
#define DEFCALL(name, code, func) &Driver::Call##name,
#include "Driver.def"
#undef DEFCALL
};


// Initialize the map from function name to driver call number
misc::StringMap Driver::function_name_to_call_map = 
{
#define STR(x) #x
#define DEFCALL(name, code, func) {STR(func), code},
#include "Driver.def"
#undef DEFCALL
#undef STR
};


// Debug file name, as set by user
std::string Driver::debug_file;

// Singleton instance
std::unique_ptr<Driver> Driver::instance;

// Debugger
misc::Debug Driver::debug;


Driver::Driver() : comm::Driver("HSA", "/dev/hsa")
{
	signal_manager.reset(new SignalManager());
}


void Driver::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option '--hsa-debug-driver <file>'
	command_line->RegisterString("--hsa-debug-driver <file>", debug_file,
			"Dump debug information for the HSA driver, "
			"including all ABI calls coming from the runtime.");
}


void Driver::ProcessOptions()
{
	debug.setPath(debug_file);
	//debug.setPrefix("[HSA driver]");
}


Driver *Driver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Driver());
	return instance.get();
}


std::unique_ptr<DriverCallHandler> Driver::GetDriverCallHandler(int code)
{
	switch(code)
	{
	case CallCodeSignalDestroy:

		return misc::new_unique<SignalDestroyHandler>(
				signal_manager.get());
		break;

	default:

		return std::unique_ptr<DriverCallHandler>(nullptr);
		break;
	}
}


int Driver::Call(comm::Context *context,
		mem::Memory *memory,
		int code,
		unsigned args_ptr)
{
	auto driver_call_handler = GetDriverCallHandler(code);
	if (driver_call_handler.get())
	{
		driver_call_handler->Process(memory, args_ptr);
		return 0;
	}

	// Check valid call
	if (code < 0 || code >= CallCodeCount || !call_fn[code])
	{
		throw misc::Panic(misc::fmt("Invalid call code (%d)\n", code));
		return -1;
	}

	// Debug
	debug << misc::fmt("ABI call '%s'\n", call_name[code]);

	// Invoke call
	CallFn fn = call_fn[code];
	return (this->*fn)(context, memory, args_ptr);
}


Driver::~Driver()
{
}

}  // namespace HSA
