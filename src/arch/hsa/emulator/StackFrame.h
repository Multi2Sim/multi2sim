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

#ifndef ARCH_HSA_EMULATOR_STACKFRAME_H
#define ARCH_HSA_EMULATOR_STACKFRAME_H

#include <cstring>

#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/disassembler/AsmService.h>

#include "Function.h"

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
	std::map<std::string, std::unique_ptr<Variable>> function_arguments;

	// The function arg segment memory manager. The owner ship is kept
	// by the caller frame
	SegmentManager *function_argument_segment;

	// Arguments scope, surrounded by {} in current function. Since
	// argument scope cannot be nested, when we start a new one, the old
	// one must have already been released.
	std::map<std::string, std::unique_ptr<Variable>> argument_scope;

	// The arg segment memory manager
 	std::unique_ptr<SegmentManager> argument_segment;

	// All variables declared in private, group and global segment
	std::map<std::string, std::unique_ptr<Variable>> variables;

 	// Register storage
	std::unique_ptr<char[]> register_storage;

	// C registers, use a 8 bit char for each 1 bit boolean value
	unsigned char c_registers[8];

public:

	/// Constructor
	///
	/// \param function 
	/// 	The function for this stack frame
	///
	/// \param work_item
	/// 	The work item that own this stack frame
	///
	/// \param function_argument_section
	/// 	The new HSA spec requires that the callee frame uses the 
	/// 	same arguments with the caller's argument frame. It is no 
	///	longer passed by value. Therefore, the function argument 
	/// 	segment is set by the caller. The caller with also set the 
	///	layout of this segment for the callee, by adding variables 
	/// 	in member variable \a function_arguments.
	StackFrame(Function *function, WorkItem *work_item, 
			SegmentManager *function_argument_segment);

	/// Destructor
	~StackFrame();

	/// Return the function
	Function *getFunction() const { return function; }

	/// Return the program counter
	BrigCodeEntry *getPc() const { return pc.get(); }

	/// Set the program counter
	void setPc(std::unique_ptr<BrigCodeEntry> pc);

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
	void getRegisterValue(const std::string &name, void *buffer) const
	{
		// Do special action for c registers
		if (name[1] == 'c')
		{
			unsigned int index = name[2] - '0';
			*(unsigned char *)buffer = c_registers[index];
			return;
		}

		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Get the size of the register
		unsigned size = AsmService::getSizeInByteByRegisterName(name);

		// Copy the value of the register
		memcpy(buffer, register_storage.get() + offset, size);

		// Return the value of the register
		return;
	}

	/// Set a registers value
	void setRegisterValue(const std::string &name, void *value)
	{
		// Do special action for c registers
		if (name[1] == 'c')
		{
			unsigned int index = name[2] - '0';
			c_registers[index] = *(unsigned char *)value;
			return;
		}

		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Get the size of the register
		unsigned size = AsmService::getSizeInByteByRegisterName(name);

		// Copy the value to the register
		memcpy(register_storage.get() + offset, value, size);

		// Set the value of the register
		return;
	}

	/// Start an argument scope, when a '{' appears. Requires the size to
	/// be allocated for the argument segment
	void StartArgumentScope(unsigned size);

	/// Release an argument scope, when we find a '}'
	void CloseArgumentScope();

	/// Return current work item
	WorkItem *getWorkItem() const { return work_item; }

	/// Return the argument segment memory manager
	SegmentManager *getArgumentSegment() const 
	{ 
		return argument_segment.get(); 
	}

	/// Return the function argument segment memory manager
	SegmentManager *getFunctionArgumentSegment() const 
	{ 
		return function_argument_segment; 
	}

	/// Adding an argument in the function argument list. This API is 
	/// opened for the caller to set the memory layout for the function
	/// argument segment. In the caller function, it will use bracket to 
	/// create an argument segment and define a few variables there. 
	/// Those variables is going to be the input and output argument 
	/// for the callee function. Because the formal name and the actuall
	/// name can be different and the sequence of declaring those 
	/// variables are different, when the function is involked, the 
	/// simulator should know the connection between the formal and 
	/// the actual argument. The value of the actual argument is stored
	/// in the memory that is managed by the argument segment manager. 
	/// Passing the arguments is just passing a referece of the segment 
	/// manager, since the callee will be able to access that part of 
	/// memory. However, the simulator would have to modify the memory
	/// layout, so that the callee knows the right offset.
	void addFunctionArguments(std::unique_ptr<Variable> argument)
	{
		function_arguments.emplace(argument->getName(), 
				std::move(argument));
	}

	/// Add an variable to the the variable list
	void addVariable(std::unique_ptr<Variable> variable)
	{
		variables.emplace(variable->getName(), 
				std::move(variable));
	}

	/// Return an variable by the name of the variable. If the name is 
	/// not found, return nullptr
	Variable *getVariable(const std::string &name) 
	{
		auto it = variables.find(name);
		if (it == variables.end()) 
			return nullptr;
		return it->second.get();
	}

	/// Return an argument in the argument scope. If the name is not found
	/// return nullptr
	Variable *getArgument(const std::string &name)
	{
		auto it = argument_scope.find(name);
		if (it == argument_scope.end())
			return nullptr;
		return it->second.get();
	}

	/// Return an argument in the function arguments. If the name is not 
	/// found, return nullptr
	Variable *getFunctionArgument(const std::string &name) 
	{
		auto it = function_arguments.find(name);
		if(it == function_arguments.end())
			return nullptr;
		return it->second.get();
	}

	/// This function trys to get a symbol in the stack frame. It would 
	/// first try to search in the argument scope, and variables and 
	/// finally function arguments. If the name is net defined in 
	//. this stack frame, nullptr will be returned.
	Variable *getSymbol(const std::string &name);

};

}  // namespace HSA

#endif

