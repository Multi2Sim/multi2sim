/*
 *  Multi2Sim
 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/String.h>

#include "Driver.h"


namespace Kepler
{


// Initialize table of ABI call names
const char *Driver::call_name[CallCodeCount] =
{
	"Invalid",  // For code 0
#define DEFCALL(name, code) #name,
#include "Driver.def"
#undef DEFCALL
};


// Initialize table of ABI call functions
const Driver::CallFn Driver::call_fn[CallCodeCount] =
{
	nullptr,  // For code 0
#define DEFCALL(name, code) &Driver::Call##name,
#include "Driver.def"
#undef DEFCALL
};


// Debug file name, as set by user
std::string Driver::debug_file;

// Singleton instance
std::unique_ptr<Driver> Driver::instance;

// Debugger
misc::Debug Driver::debug;


Driver *Driver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Driver());
	return instance.get();
}


Driver::Driver() : comm::Driver("Kepler",
		"/dev/kepler")
{
	// Welcome message from driver
	debug << "Driver initialized\n";
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
	command_line->setCategory("Kepler");

	// Debug information for driver
	command_line->RegisterString("--kpl-debug-driver <file>", debug_file,
			"Dump debug information for the Kepler driver, "
			"including all ABI calls coming from the runtime.");
}


void Driver::ProcessOptions()
{
	debug.setPath(debug_file);
	debug.setPrefix("[Kepler driver]");
}
	
	
Module *Driver::addModule(const std::string &cubin_path)
{
	// The module ID is its position in the list
	int id = modules.size();

	// Create module and add it to the list of modules
	modules.emplace_back(new Module(id, cubin_path));
	return modules.back().get();
}


}  // namepsace Kepler
