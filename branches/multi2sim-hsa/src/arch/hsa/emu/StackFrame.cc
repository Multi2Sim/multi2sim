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

#include "StackFrame.h"
#include "WorkItem.h"
#include "Emu.h"
#include "SegmentManager.h"

namespace HSA
{

StackFrame::StackFrame(Function *function, WorkItem *work_item)
{
	// Set function that this frame is working on
	this->function = function;

	// Set work item
	this->work_item = work_item;

	// Set the program counter to be pointing to the first entry of the
	// function
	pc = function->getFirstEntry();

	// Allocate register space
	this->register_storage.reset(new char[function->getRegisterSize()]);

	// Initialize argument scopes
	function_arguments.reset(new VariableScope());
	variable_scope.reset(new VariableScope());

	// Initialize function argument segment
	mem::Memory *memory = Emu::getInstance()->getMemory();
	func_arg_segment.reset(new SegmentManager(memory,
			function->getArgumentSize()));
}


StackFrame::~StackFrame()
{
	// Remove scopes
	argument_scope.release();
	variable_scope.release();
	function_arguments.release();

	func_arg_segment.release();
	if (arg_segment.get())
	{
		arg_segment.release();
	}
}


void StackFrame::setPc(std::unique_ptr<BrigCodeEntry> pc)
{
	this->pc = std::move(pc);
}


void StackFrame::StartArgumentScope(unsigned size)
{
	argument_scope.reset(new VariableScope());
	mem::Memory *memory = Emu::getInstance()->getMemory();
	arg_segment.reset(new SegmentManager(memory, size));
};


void  StackFrame::CloseArgumentScope()
{
	argument_scope.reset(nullptr);
	arg_segment.reset(nullptr);
};


void StackFrame::Dump(std::ostream &os = std::cout) const
{
	os << "***** Stack frame *****\n";

	// Dump function name
	os << misc::fmt("  Function: %s, \n", function->getName().c_str());

	// Dump program counter and current instruction
	os << misc::fmt("  Program counter (offset in code section): 0x%x, ",
			pc->getOffset());
	pc->Dump(os);
	os << "\n";

	// Dump Register status
	os << "  ***** Registers *****\n";
	std::map<std::string, unsigned int> reg_info =
			function->getRegisterInformation();
	for (auto it = reg_info.begin(); it != reg_info.end(); it++)
	{
		os << "    ";
		DumpRegister(it->first, os);
	}
	os << "  ***** ********* *****\n\n";

	// Dump function arguments
	os << "  ***** Function arguments *****\n";
	function_arguments->Dump(os, 4);
	os << "  ***** ******** ********* *****\n\n";

	// If in argument scope, dump argument scope
	os << "  ***** Argument scope *****\n";
	if (argument_scope.get())
		argument_scope->Dump(os, 4);
	os << "  ***** ******** ***** *****\n\n";

	// Variable scope, 4
	os << "  ***** Variables *****\n";
	variable_scope->Dump(os, 4);
	os << "  ***** ********* *****\n\n";

	// Dump back trace information
	work_item->Backtrace(os);


	os << "***** ***** ***** *****\n";
}


void StackFrame::DumpRegister(const std::string &name,
		std::ostream &os = std::cout) const
{
	os << misc::fmt("%s: ", name.c_str());
	switch (name[1])
	{
	case 'c':

		if (getRegisterValue<unsigned char>(name))
			os << "true";
		else
			os << "false";
		break;

	case 's':

		os << misc::fmt("%u, %d, %f, 0x%08x",
				getRegisterValue<unsigned int>(name),
				getRegisterValue<int>(name),
				getRegisterValue<float>(name),
				getRegisterValue<unsigned int>(name));

		break;

	case 'd':

		os << misc::fmt("%llu, %lld, %f, 0x%016llx",
				getRegisterValue<unsigned long long>(name),
				getRegisterValue<long long>(name),
				getRegisterValue<double>(name),
				getRegisterValue<unsigned long long>(name));
		break;

	case 'q':

	{
		unsigned int offset =
				function->getRegisterOffset(name);
		for (int i=0; i<16; i++)
		{
			if (i > 0)
			{
				os << ", ";
			}
			os << misc::fmt("0x%x",
					*(this->register_storage.get()
							+ offset + i));
		}
		break;
	}

	default:

		throw misc::Panic(misc::fmt(
				"Unknown register name %s.",
				name.c_str()));
		break;
	}
	os << "\n";
}

}  // namespace HSA

