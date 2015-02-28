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

#ifndef ARCH_HSA_EMU_STACKFRAME_H
#define ARCH_HSA_EMU_STACKFRAME_H

#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/driver/DriverCallbackInfo.h>

#include "Function.h"
#include "VariableScope.h"

namespace HSA
{
class WorkItem;
class Driver;

class StackFrame
{
	// The function this stack frame associate with
	Function *function;

	// The work item that this stack frame belongs to
	WorkItem *work_item;

	// Pointer to the instruction to be executed
	std::unique_ptr<BrigCodeEntry> pc;

	// Function input and output arguments
	std::unique_ptr<VariableScope> function_arguments;

	// The function arg segment memory manager
	 std::unique_ptr<SegmentManager> func_arg_segment;

	// Arguments scope, surrounded by {} in current function. Since
	// argument scope cannot be nested, when we start a new one, the old
	// one must have already been released.
	std::unique_ptr<VariableScope> argument_scope;

	// All variables declared in private, group and global segment
	std::unique_ptr<VariableScope> variable_scope;

 	// The arg segment memory manager
 	std::unique_ptr<SegmentManager> arg_segment;

	// Register storage
	std::unique_ptr<char> register_storage;

	// C registers, use a 8 bit char for each 1 bit boolean value
	unsigned char c_registers[8];

public:

	/// Prototype for return callback functions
	typedef int (*CallbackFn)(DriverCallbackInfo *info);

private:
	// If driver_function is set, when the stack frame return, call this
	// function
	CallbackFn return_callback = nullptr;

	// The information provided for the return callback function
	std::unique_ptr<DriverCallbackInfo> callback_info;

public:

	/// Constructor
	StackFrame(Function *function, WorkItem *work_item);

	/// Destructor
	~StackFrame();

	/// Return the function
	Function *getFunction() const { return function; }

	/// Return the program counter
	BrigCodeEntry *getPc() const { return pc.get(); }

	/// Set the program counter
	void setPc(std::unique_ptr<BrigCodeEntry> pc);

	/// Set return callback function
	void setReturnCallback(CallbackFn callback,
			std::unique_ptr<DriverCallbackInfo> info)
	{
		this->return_callback = callback;
		this->callback_info = std::move(info);
	}

	/// Dump stack frame information
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const StackFrame &frame)
	{
		frame.Dump(os);
		return os;
	}

	/// Dump the information of a register by name
	void DumpRegister(const std::string &name, std::ostream &os) const;

	/// Return register value
	template <typename Type>
	Type getRegisterValue(const std::string &name) const
	{
		// Do special action for c registers
		if (name[1] == 'c')
		{
			unsigned int index = name[2] - '0';
			return c_registers[index];
		}

		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Cast corresponding pointer to certain type
		Type *pointer = (Type *)(this->register_storage.get() + offset);

		// Return the value of the register
		return *pointer;
	}

	/// Set a registers value
	template <typename Type>
	void setRegisterValue(const std::string &name, Type value)
	{
		// Do special action for c registers
		if (name[1] == 'c')
		{
			unsigned int index = name[2] - '0';
			c_registers[index] = value;
			return;
		}

		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Cast corresponding pointer to certain type
		Type *pointer = (Type *)(this->register_storage.get() + offset);

		// Set the value of the register
		*pointer = value;
	}

	/// Start an argument scope, when a '{' appears. Requires the size to
	/// be allocated for the argument segment
	void StartArgumentScope(unsigned size);

	/// Release an argument scope, when we find a '}'
	void  CloseArgumentScope();

	/// Create an argument in the argument scope
//	void CreateArgument(const std::string &name, BrigTypeX type)
//	{
//		if (argument_scope.get())
//		{
//			argument_scope->DeclearVariable(name, type,
//					arg_segment.get());
//		}
//	};

	/// Return current work item
	WorkItem *getWorkItem() const { return work_item; }

	/// Return argument scope
	VariableScope *getArgumentScope() const { return argument_scope.get(); }

	/// Return function argument scope
	VariableScope *getFunctionArguments() const
	{
		return function_arguments.get();
	}

	/// Return the argument segment memory manager
	SegmentManager *getArgSegment() const { return arg_segment.get(); }

	/// Return the function argument segment memory manager
	SegmentManager *getFuncArgSegment() const { return func_arg_segment.get(); }

	/// Return variable scope
	VariableScope *getVariableScope() const { return variable_scope.get(); }

	/// Get the return callback function
	CallbackFn getReturnCallback() const 
	{
		return return_callback;
	}

	/// Return the pointers to the callback arguments
	DriverCallbackInfo* getReturnCallbackInfo() const
	{
		return callback_info.get();
	}

};

}  // namespace HSA

#endif

