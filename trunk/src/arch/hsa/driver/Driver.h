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
#include <arch/hsa/emu/Emu.h>

#include "runtime.h"

namespace HSA
{

class Driver: public comm::Driver
{
	// Debug file name, as set by user
	static std::string debug_file;

	/// Constructor
	Driver() : comm::Driver("HSA", "/dev/hsa")
	{};

	// Unique instance of singleton
	static std::unique_ptr<Driver> instance;

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
#define DEFCALL(name, code, func) CallCode##name = code,
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
#define DEFCALL(name, code, func) int Call##name(mem::Memory *memory, \
		unsigned args_ptr);
#include "Driver.def"
#undef DEFCALL

	// ABI call names
	static const char *call_name[CallCodeCount];

	// Prototype of a member function executing an ABI call
	typedef int (Driver::*CallFn)(mem::Memory *memory, unsigned args_ptr);

	// Table of ABI call execution functions
	static const CallFn call_fn[CallCodeCount];

	// The iterator pointing to the components
	std::map<unsigned long long, std::unique_ptr<Component>>::iterator
			component_iterator;

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
	int Call(int code, mem::Memory *memory, unsigned args_ptr);

	/// Destructor
	~Driver();
};

} /* namespace HSA */

#endif 
