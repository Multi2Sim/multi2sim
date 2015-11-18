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

#ifndef ARCH_HSA_DRIVER_DRIVER_H
#define ARCH_HSA_DRIVER_DRIVER_H

#include <arch/common/Driver.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>
#include <memory/Memory.h>

#include "SignalManager.h"
#include "../../../../runtime/include/hsa.h"
#include "../../../../runtime/include/hsa_ext_finalize.h"
#include "DriverCallHandler.h"

namespace HSA
{
class StackFrame;
class Component;
class WorkItem;

class Driver: public comm::Driver
{
	// Debug file name, as set by user
	static std::string debug_file;

	// Maps from the function name in HSAIL to call number
	static misc::StringMap function_name_to_call_map;

	// The container of all signals
	std::unique_ptr<SignalManager> signal_manager;

	/// Constructor
	Driver();

	// Unique instance of singleton
	static std::unique_ptr<Driver> instance;

	// Enumeration with all ABI call codes. Each entry of Driver.def will
	// expand into an assignment. For example, entry
	//
	//	DEFCALL(Init, 1, &hsa_init)
	//
	// expands to
	//
	//	CallCodeInit = 1
	//
	// A last element 'CallCodeCount' is equal to one unit higher than the
	// latest ABI call found in the file.
	enum
	{
		CallInvalid,
#define DEFCALL(name, code, func) CallCode##name,
#include "Driver.def"
#undef DEFCALL
		CallCodeCount
	};

	// ABI call functions. Each entry in Driver.def will expand into a
	// function prototype. For example, entry
	//
	//	DEFCALL(Init, 1, &hsa_init)
	//
	// expands to
	//
	//	void CallInit(mem::Memory *memory, unsigned args_ptr);
	//
#define DEFCALL(name, code, func) \
	int Call##name(comm::Context *context, \
			mem::Memory *memory, \
			unsigned args_ptr);
#include "Driver.def"
#undef DEFCALL

	// ABI call names
	static const char *call_name[CallCodeCount];

	// Prototype of a member function executing an ABI call
	typedef int (Driver::*CallFn)(comm::Context *context,
			mem::Memory *memory,
			unsigned args_ptr);

	// Table of ABI call execution functions
	static const CallFn call_fn[CallCodeCount];

	// Retrieve the value at a certain memory space
	template <typename T>
	static T getArgumentValue(int offset, mem::Memory *memory,
			unsigned args_ptr)
	{
		// Read from memory
		T value;
		memory->Read(args_ptr + offset, sizeof(T), (char *)&value);
		return value;
	}

	// Set the the value at a certain memory space
	template<typename T>
	static void setArgumentValue(T value, int offset, mem::Memory *memory,
			unsigned args_ptr)
	{
		memory->Write(args_ptr+offset, sizeof(T), (char *)&value);
	}

	// Get driver call handler by call number
	std::unique_ptr<DriverCallHandler> GetDriverCallHandler(int code);

public:

	/// Obtain instance of the singleton
	static Driver *getInstance();

	/// Debugger
	static misc::Debug debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Invoke an ABI call. See documentation for comm::Driver::Call for
	/// details on the meaning of the arguments.
	int Call(comm::Context *context,
			mem::Memory *memory,
			int code,
			unsigned args_ptr);

	/// Destructor
	virtual ~Driver();

	/// Get the signal manager
	SignalManager *getSignalManager() const { return signal_manager.get(); }
};

} /* namespace HSA */

#endif 
