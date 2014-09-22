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

#ifndef ARCH_HSA_EMU_RUNTIMEINTERCEPTOR_H
#define ARCH_HSA_EMU_RUNTIMEINTERCEPTOR_H

#include <lib/cpp/String.h>

#include "StackFrame.h"


namespace HSA
{

/// The Runtime Interceptor is a singleton that detects function calls. If
/// the name of function called equals to a runtime function, the interceptor
/// will redirect the execution of the function to the driver.
class RuntimeInterceptor
{
	// Unique instance
	static std::unique_ptr<RuntimeInterceptor> instance;

	// Maps from the function name to call number
	static misc::StringMap function_name_to_call_map;

	// Private constructor
	RuntimeInterceptor();

	// Allocate memory to hold serialized arguments, returns the address
	// pointing to the beginning of the argument memory space.,
	unsigned PassArgumentsInByValue(const std::string &function_name,
			StackFrame *stack_top);

	// Copy the argument value back from the callee's buffer to caller's
	// buffer
	void PassBackByValue(unsigned arg_address,
			StackFrame *stack_top);

public:

	/// Return the pointer to the instance, or throw a \c Panic if no
	/// program has been loaded
	static RuntimeInterceptor *getInstance();

	/// Try to intercept a function call
	///
	/// return
	///	If the function is intercepted, return true. Otherwise, return
	/// 	false.
	bool Intercept(const std::string &function_name,
			StackFrame *stack_top);

	/// Destructor
	~RuntimeInterceptor();
};

}  // namespace HSA

#endif
