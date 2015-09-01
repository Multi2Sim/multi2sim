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

#ifndef ARCH_HSA_EMULATOR_FUNCTION_H
#define ARCH_HSA_EMULATOR_FUNCTION_H

#include <map>
#include <memory>
#include <string>

#include <arch/hsa/disassembler/BrigCodeEntry.h>

#include "Variable.h"


namespace HSA
{

class StackFrame;
class HsaExecutable;

/// A function encapsulates information about a HSAIL function
class Function
{
	// name of the function
	std::string name;

	// The executable that this function belongs to
	HsaExecutable *executable;

	// The module that this function belongs to
	BrigFile *binary;

	// The first code entry in the function
	std::unique_ptr<BrigCodeEntry> first_entry;

	// Pointer to the last instuction in the function, when pc is move to
	// a point beyond this, return the function
	std::unique_ptr<BrigCodeEntry> last_entry;

	// The directive where the function is declared
	std::unique_ptr<BrigCodeEntry> function_directive;

	/// Dump argument related information
	void DumpArgumentInfo(std::ostream &os) const;

	/// Dump register related information
	void DumpRegisterInfo(std::ostream &os) const;




	//
	// Fields related with arguments
	//

	// Argument size. 
	// When stack frame initialize, allocate the size of memory
	unsigned int argument_size = 0;

	// Map the name with the information of the argument
	std::map<std::string, std::unique_ptr<Variable>> arguments;




	//
	// Fields related with registers
	//

	// Allocated register size
	unsigned int register_size = 0;

	// Maps register
	std::map<std::string, unsigned int> register_info;

	// Add register information into table
	void addRegister(BrigRegisterKind kind, unsigned short number);

public:

	/// Constructor
	Function(const std::string &name);

	/// Destructor
	~Function(){};

	/// Returns function name
	std::string getName() const { return name; }


	/// Set first code entry
	void setFirstEntry(std::unique_ptr<BrigCodeEntry> first_entry)
	{
		this->first_entry = std::move(first_entry);
	}

	/// Return pointer to entry point
	std::unique_ptr<BrigCodeEntry> getFirstEntry() const;

	/// Set the last entry
	void setLastEntry(std::unique_ptr<BrigCodeEntry> last_entry)
	{
		this->last_entry = std::move(last_entry);
	}

	/// Return pointer to the last entry
	std::unique_ptr<BrigCodeEntry> getLastEntry() const;

	/// Set the directive
	void setFunctionDirective(std::unique_ptr<BrigCodeEntry> directive)
	{
		this->function_directive = std::move(directive);
	}

	/// Return the pointer to the directive
	BrigCodeEntry *getFunctionDirective() const
	{
		return function_directive.get();
	}

	/// Add an argument information in argument table
	void addArgument(std::unique_ptr<Variable> argument);

	/// Return the memory size required to hold the arguments
	unsigned getArgumentSize() const { return argument_size; }

	/// Return the number of arguments
	unsigned getArgumentCount() const { return arguments.size(); }

	/// Allocate register
	void AllocateRegister(unsigned int *max_reg);

	/// Return the offset of an register. If the register does not exist,
	/// return -1.
	unsigned int getRegisterOffset(const std::string &name) const;

	/// Return the size of register required
	unsigned int getRegisterSize() const { return register_size; }

	/// Return the begin iterator of the register
	std::map<std::string, unsigned>::iterator getRegisterBegin() 
	{
		return register_info.begin();
	}

	// Return the end iterator of the register
	std::map<std::string, unsigned>:: iterator getRegisterEnd()
	{
		return register_info.end();
	}

	/*
	/// Copy variable information and value from caller's argument scope
	/// to callee's argument scope
	void PassByValue(StackFrame *caller_frame,
			StackFrame *callee_frame, BrigCodeEntry *call_inst);

	/// Copy return value from the calle to caller
	void PassBackByValue(StackFrame *caller_frame,
			StackFrame *callee_frame, BrigCodeEntry *call_inst);
	*/

	/// Dump function information for debug propose
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Function &function)
	{
		function.Dump(os);
		return os;
	}

	/// Set executable
	void setExecutable(HsaExecutable *executable)
	{
		this->executable = executable;
	}

	/// Get executable
	HsaExecutable *getExecutable()
	{
		return executable;;
	}

	/// Set module
	void setModule(BrigFile *binary)
	{
		this->binary = binary;
	}

	/// Get module
	BrigFile *getModule()
	{
		return binary;
	}

};

}  // namespace HSA

#endif
