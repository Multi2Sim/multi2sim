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
#include "Emu.h"

namespace HSA
{

StackFrame::StackFrame(Function *function)
{
	// Set function that this frame is working on
	this->function = function;

	// Set the program counter to be pointing to the first entry of the
	// function
	pc = function->getEntryPoint();

	// Set next directive to be processed to the first in function directive
	next_dir = function->getFirstInFunctionDirective();

	// Allocate register space
	char *register_space = new char[function->getRegisterSize()];
	this->register_storage.reset(register_space);

	// Initialize argument scopes
	function_arguments.reset(new VariableScope());
	variable_scope.reset(new VariableScope());
}


StackFrame::~StackFrame()
{}

void StackFrame::setPc(char *pc)
{
	// Otherwise move pc;
	this->pc = pc;
}


void StackFrame::Dump(std::ostream &os = std::cout) const
{
	os << "***** Stack frame *****\n";

	// Dump function name
	os << misc::fmt("  Function: %s, \n", function->getName().c_str());

	// Dump program counter and current instruction
	os << misc::fmt("  Program counter: 0x%llx, ", (unsigned long long)pc);
	os << BrigInstEntry(pc, ProgramLoader::getInstance()->getBinary());
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
	function_arguments->Dump(os);
	os << "  ***** ******** ********* *****\n\n";

	// If in argument scope, dump argument scope
	os << "  ***** Argument scope *****\n";
	if (argument_scope.get())
		argument_scope->Dump(os);
	os << "  ***** ******** ***** *****\n\n";

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
			os << "ture";
		else
			os << "false";
		break;

	case 's':

		os << misc::fmt("%d, %d, %f",
				getRegisterValue<unsigned short>(name),
				getRegisterValue<short>(name),
				getRegisterValue<float>(name));
		break;

	case 'd':

		os << misc::fmt("%d, %d, %f",
				getRegisterValue<unsigned int>(name),
				getRegisterValue<int>(name),
				getRegisterValue<double>(name));
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


unsigned int StackFrame::getCodeOffset() const
{
	BrigSection *code_section =
		ProgramLoader::getInstance()->getBinary()->getBrigSection(BrigSectionCode);
	char *code_buffer = (char *)code_section->getBuffer();
	return pc - code_buffer;
}

}  // namespace HSA

