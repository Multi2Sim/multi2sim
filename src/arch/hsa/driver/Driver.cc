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

#include "Driver.h"


namespace HSA
{

//
// Class 'DriverConfig'
//

void DriverConfig::Register(misc::CommandLine &command_line)
{
	// Option --hsa-driver-debug <file>
	command_line.RegisterString("--hsa-driver-debug <file>", debug_file,
			"Dump debug information related with HSA driver ABI "
			"calls and their arguments.");
	
}


void DriverConfig::Process()
{
	Driver::debug.setPath(debug_file);
}





//
// Class 'Driver'
//

// Initialize table of ABI call names
const char *Driver::call_name[CallCodeCount] =
{
	"Invalid",  // For code 0
#define DEFCALL(name, code) #name,
#include "Driver.dat"
#undef DEFCALL
};


// Initialize table of ABI call functions
const Driver::CallFn Driver::call_fn[CallCodeCount] =
{
	nullptr,  // For code 0
#define DEFCALL(name, code) &Driver::Call##name,
#include "Driver.dat"
#undef DEFCALL
};


// Configuration
DriverConfig Driver::config;

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
	

int Driver::Call(int code, mem::Memory *memory, unsigned args_ptr)
{
	// Check valid call
	if (code < 0 || code >= CallCodeCount || !call_fn[code])
	{
		debug << "Invalid call code (" << code << ")\n";
		return -1;
	}

	// Debug
	debug << "ABI call '" << call_name[code] << "\n";

	// Invoke call
	CallFn fn = call_fn[code];
	return (this->*fn)(memory, args_ptr);
}


}  // namepsace HSA

