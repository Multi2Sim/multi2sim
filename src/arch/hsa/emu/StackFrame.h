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

#include "Function.h"


namespace HSA
{

class StackFrame
{
	// The function this stack frame associate with
	Function *function;

	// Pointer to the instruction to be executed
	char *pc;

	// Argument storage
	// FIXME: Move argument_storage to guest memory
	std::unique_ptr<char> argument_storage;

	// Register storage
	// FIXME: Move register_storgae to guest memory
	std::unique_ptr<char> register_storage;

public:

	/// Constructor
	StackFrame(Function *function);

	/// Destructor
	~StackFrame();

	/// Return the function
	Function *getFunction() const { return function; }

	/// Return the program counter
	char *getPc() const { return pc; }

	/// Set the program counter
	void setPc(char *pc)
	{
		// FIXME: check if the pc is in valid region
		this->pc = pc;
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
		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Cast corresponding pointer to certain type
		Type *pointer = (Type *)(this->register_storage.get() + offset);

		// Return the value of the register
		return *pointer;
	}

	// Set a registers value
	template <typename Type>
	void setRegisterValue(const std::string &name, Type value)
	{
		// Get the offset of the register
		unsigned int offset = function->getRegisterOffset(name);

		// Cast corresponding pointer to certain type
		Type *pointer = (Type *)(this->register_storage.get() + offset);

		// Set the value of the register
		*pointer = value;
	}

};

}  // namespace HSA

#endif

