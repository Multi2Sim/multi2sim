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

#ifndef ARCH_SOUTHERN_ISLANDS_DRIVER_DRIVER_H
#define ARCH_SOUTHERN_ISLANDS_DRIVER_DRIVER_H

#include <arch/common/Driver.h>
#include <arch/southern-islands/disassembler/Disassembler.h>
#include <arch/southern-islands/emulator/NDRange.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include "Program.h"
#include "Kernel.h"

namespace SI
{

// Forward Declarations
class Program;


/// Southern Islands driver
class Driver : public comm::Driver
{
	// Debug file name, as set by user
	static std::string debug_file;
	
	// Unique instance of singleton
	static std::unique_ptr<Driver> instance;

	// Primary list of Programs
	std::vector<std::unique_ptr<Program>> programs;

	// Primary list of Kernels
	std::vector<std::unique_ptr<Kernel>> kernels;

	// Indicates whether memory is fused or not
	bool fused = false;

	// Enumeration with all ABI call codes. Each entry of Driver.def will
	// expand into an assignment. For example, entry
	//
	//	DEFCALL(Init, 1)
	//
	// expands to
	//
	//	CallCodeInit = 1
	//
	// A last element 'CallCodeCount' is equal to one unit higher than the
	// latest ABI call found in the file.
	enum
	{
#define DEFCALL(name, code) CallCode##name = code,
#include "Driver.def"
#undef DEFCALL
		CallCodeCount
	};

	// ABI call functions. Each entry in Driver.def will expand into a
	// function prototype. For example, entry
	//
	//	DEFCALL(Init, 1)
	//
	// expands to
	//
	//	void CallInit(mem::Memory *memory, unsigned args_ptr);
	//
#define DEFCALL(name, code) \
	int Call##name(comm::Context *context, \
			mem::Memory *memory, \
			unsigned args_ptr);
#include "Driver.def"
#undef DEFCALL

	// System call names
	static const char *call_name[CallCodeCount];

	// Prototype of a member function executing an ABI call
	typedef int (Driver::*CallFn)(comm::Context *context,
			mem::Memory *memory,
			unsigned args_ptr);

	// Table of ABI call execution functions
	static const CallFn call_fn[CallCodeCount];

	// Add program
	Program *AddProgram(int program_id);

	// Add kernel
	Kernel *AddKernel(int kernel_id, const std::string &func, Program *program);
	
public:

	//
	// Error class
	//

	/// Error related with the Southern Islands driver
	class Error : public misc::Error
	{
	public:

		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Southern Islands driver");
		}
	};




	//
	// Static fields
	//
	
	/// Maximum work goup buffer size
	static const unsigned MaxWorkGroupBufferSize = 1024 * 1024;
	
	/// Debugger
	static misc::Debug debug;

	/// Obtain instance of the singleton
	static Driver *getInstance();

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();




	//
	// Member functions
	//

	/// Constructor
	Driver() : comm::Driver("Southern Islands",
			"/dev/southern-islands")
	{
	}

	/// Invoke an ABI call. See documentation for comm::Driver::Call for
	/// details on the meaning of the arguments.
	int Call(comm::Context *context,
			mem::Memory *memory,
			int code,
			unsigned args_ptr);
	
	/// Get reference to the main program list
	std::vector<std::unique_ptr<Program>> &getPrograms() { return programs; }

	/// Get count of programs in list
	int getProgramCount() const { return programs.size(); }

	/// Get program by its ID
	Program *getProgramById(unsigned id)
	{
		assert(id < programs.size());
		return programs[id].get();
	}

	/// Get count of kernels in list
	int getKernelCount() const { return kernels.size(); }

	/// Get kernel by its Id
	Kernel *getKernelById(unsigned id)
	{
		assert(id < kernels.size());
		return kernels[id].get();
	}
};


}  // namespace SI

#endif

