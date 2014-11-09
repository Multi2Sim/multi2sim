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

#include "DriverCallbackInfo.h"
#include "runtime.h"

namespace HSA
{
class StackFrame;
class Component;
class WorkItem;

class Driver: public comm::Driver
{
	// Debug file name, as set by user
	static std::string debug_file;

	/// Constructor
	Driver() : comm::Driver("HSA", "/dev/hsa"){};

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
#define DEFCALL(name, code, func) CallCode##name = code,
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
#define DEFCALL(name, code, func) int Call##name(mem::Memory *memory, \
		unsigned args_ptr);
#include "Driver.def"
#undef DEFCALL

	// Stores the argument received when agent_iterate is called, when
	// next_agent function is called, reuse the arguments
	mem::Memory *agent_iterator_memory;
	unsigned agent_iterator_args_ptr;

	// ABI call names
	static const char *call_name[CallCodeCount];

	// Prototype of a member function executing an ABI call
	typedef int (Driver::*CallFn)(mem::Memory *memory, unsigned args_ptr);

	// Table of ABI call execution functions
	static const CallFn call_fn[CallCodeCount];

	// Retrieve the value at a certain memory space
	template <typename T>
	static T getArgumentValue(int offset, mem::Memory *memory, unsigned args_ptr)
	{
		// Get the pointer to that argument 
		T *buffer = (T *)memory->getBuffer(args_ptr + offset, sizeof(T), 
				mem::Memory::AccessRead);

		// Dereference the pointer and return the value
		return (*buffer);	
	}

	// Set the the value at a certain memory space
	template<typename T>
	static void setArgumentValue(T value, int offset, mem::Memory *memory,
			unsigned args_ptr)
	{
		// Get the pointer to the argument
		T *buffer = (T *)memory->getBuffer(args_ptr + offset, sizeof(T), 
				mem::Memory::AccessWrite);

		// Set the value of the argument
		*buffer = value;
	}

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

	/// Used as callback function to iterate next device
	static int IterateAgentNext(DriverCallbackInfo *args);

	/// Redirect the execution to the callback function
	void StartAgentIterateCallback(WorkItem *workitem,
			unsigned callback_address,
			unsigned long long component_handler,
			unsigned long long data_address,
			unsigned args_ptr);




	//
	// Interceptor related functions
	//

private:

	// Maps from the function name in HSAIL to call number
	static misc::StringMap function_name_to_call_map;

	// Copy the arguments into a certain place in memory
	void SerializeArguments(char *arg_buffer, StackFrame *stack_top);

	// The iterator pointing to the components
	std::map<unsigned long long, std::unique_ptr<Component>>::iterator
			component_iterator;

	// Allocate memory to hold serialized arguments, returns the address
	// pointing to the beginning of the argument memory space.,
	unsigned PassArgumentsInByValue(const std::string &function_name,
			StackFrame *stack_top);

	// Copy the argument value back from the callee's buffer to caller's
	// buffer
	void PassBackByValue(unsigned arg_address,
			StackFrame *stack_top);

public:
	
	/// Prototype for callback functions;
	typedef int (Driver::*CallbackFn)(mem::Memory *memory, 
			unsigned args_ptr);

	/// Try to intercept the execution of a HSAIL function
	///
	/// \param function_name 
	///	The name of the function to be intercepted
	///
	/// \param stack_top
	///	The stack frame where the interception take place
	///
	/// \return 
	///	Determine if the function is intercepted
	bool Intercept(const std::string &function_name, 
			StackFrame *stack_top);

	/// Exit the intercepted context by passing the returning value back to 
	/// the stack frame, freeing the memory allocated for the serialized 
	/// argument and move the host pc forward.
	void ExitInterceptedEnvironment(unsigned arg_address, 
			StackFrame *stack_top);

};

} /* namespace HSA */

#endif 
