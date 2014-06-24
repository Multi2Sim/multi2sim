/*  Multi2Sim
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

#ifndef ARCH_KEPLER_DRIVER_DRIVER_H
#define ARCH_KEPLER_DRIVER_DRIVER_H

#include <arch/common/Driver.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>


namespace Kepler
{

// Forward Declarations
class Program;


// Exception thrown by driver errors
class Error : public misc::Error
{
public:

	/// Constructor
	Error(const std::string &message) :
			misc::Error("Kepler driver", message)
	{
	}
};


/// Kepler driver
class Driver : public comm::Driver
{
	// Debug file name, as set by user
	static std::string debug_file;

	// Unique instance of singleton
	static std::unique_ptr<Driver> instance;

	// Version numbers
	static const int version_major;
	static const int version_minor;

	// Singletons have private constructors
	Driver();

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
#define DEFCALL(name, code) int Call##name(mem::Memory *memory, \
		unsigned args_ptr);
#include "Driver.def"
#undef DEFCALL

	// System call names
	static const char *call_name[CallCodeCount];

	// Prototype of a member function executing an ABI call
	typedef int (Driver::*CallFn)(mem::Memory *memory, unsigned args_ptr);

	// Table of ABI call execution functions
	static const CallFn call_fn[CallCodeCount];

public:

	/// Obtain instance of the singleton
	static Driver *getInstance();

	/// Invoke an ABI call. See documentation for comm::Driver::Call for
	/// details on the meaning of the arguments.
	int Call(int code, mem::Memory *memory, unsigned args_ptr);

	/// Debugger
	static misc::Debug debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};


}  // namespace SI

#endif
