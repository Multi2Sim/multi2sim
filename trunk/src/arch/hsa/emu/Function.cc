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

#include <cstring>

#include <lib/cpp/String.h>
#include <arch/hsa/asm/BrigCodeEntry.h>
#include <arch/hsa/asm/BrigOperandEntry.h>
#include <arch/hsa/asm/AsmService.h>

#include "Emu.h"
#include "AQLQueue.h"
#include "ProgramLoader.h"
#include "Function.h"
#include "StackFrame.h"

namespace HSA
{

Function::Function(const std::string& name) :
		name(name)
{
}


std::unique_ptr<BrigCodeEntry> Function::getFirstEntry() const
{
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	if(first_entry.get())
	{
		return binary->getCodeEntryByOffset(first_entry->getOffset());
	}
	return std::unique_ptr<BrigCodeEntry>(nullptr);
}


std::unique_ptr<BrigCodeEntry> Function::getLastEntry() const
{
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	if(last_entry.get())
	{
		return binary->getCodeEntryByOffset(last_entry->getOffset());
	}
	return std::unique_ptr<BrigCodeEntry>(nullptr);
}


void Function::addArgument(Variable *argument)
{

	// Check if argument is defined
	std::map<std::string, std::unique_ptr<Variable>>::iterator it
			= arg_info.find(argument->getName());
	if (it != arg_info.end())
	{
		throw Error(misc::fmt("Function argument %s redefined",
				name.c_str()));
	}

	// Insert argument into table
	arg_info.insert(std::make_pair(argument->getName(),
			std::unique_ptr<Variable>(argument)));

	// Increase arg_size
	arg_size += argument->getSize();
}


void Function::AddRegister(BrigRegisterKind kind, unsigned short number)
{
	// Get register's string representation
	std::string register_name = AsmService::RegisterToString(kind, number);

	// Check if the register has been added
	if (reg_info.find(register_name) != reg_info.end())
		throw misc::Panic("Duplicate register name");

	// Allocate size for the register
	reg_info.insert(std::make_pair(register_name, reg_size));
	switch(kind)
	{
	case BRIG_REGISTER_CONTROL:
		break;
	case BRIG_REGISTER_SINGLE:
		reg_size += 4;
		break;
	case BRIG_REGISTER_DOUBLE:
		reg_size += 8;
		break;
	case BRIG_REGISTER_QUAD:
		reg_size += 16;
		break;
	}

}


unsigned int Function::getRegisterOffset(const std::string &name) const
{
	auto it = reg_info.find(name);
	if (it == reg_info.end())
		throw misc::Panic(misc::fmt("Register %s not found",
				name.c_str()));

	return it->second;
}


void Function::AllocateRegister(unsigned int *max_reg)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		for(unsigned int j = 0; j < max_reg[i]; j++)
		{
			AddRegister((BrigRegisterKind)i, j);
		}
	}
}


void Function::PassByValue(StackFrame *caller_frame,
		StackFrame *callee_frame, BrigCodeEntry *call_inst)
{
	// Get arguments operands
	//BrigOperandArgumentList *out_args =
	//		(BrigOperandArgumentList *)call_inst->getOperand(0);
	auto in_args = call_inst->getOperand(2);

	// Get caller's argument scope and callee's function argument
	// scope
	VariableScope *callee_scope = callee_frame->getFunctionArguments();
	VariableScope *caller_scope = caller_frame->getArgumentScope();


	for (auto it = arg_info.begin(); it != arg_info.end(); it++)
	{
		// Get argument information from the function
		Variable *argument = it->second.get();

		// Insert argument into callee's function argument scope
		SegmentManager *callee_segment =
				callee_frame->getFuncArgSegment();
		callee_scope->DeclearVariable(argument->getName(),
				argument->getType(), argument->getDim(),
				callee_segment);

		// Copy argument's value
		if (argument->isInput())
		{
			// Get argument index
			unsigned int index = argument->getIndex();

			// Get the directive information and name
			auto variable_decl = in_args->getElement(index);
			std::string name_in_caller = variable_decl->getName();

			// Get buffer in caller;
			char *caller_buffer =
					caller_scope->getBuffer(name_in_caller);
			char *callee_buffer =
					callee_scope->getBuffer(argument->getName());

			// Copy memory
			BrigTypeX type = (BrigTypeX)argument->getType();
			unsigned short arg_size = AsmService::TypeToSize(type);
			memcpy(callee_buffer, caller_buffer, arg_size);
		}
	}
}


void Function::PassBackByValue(StackFrame *caller_frame,
		StackFrame *callee_frame, BrigCodeEntry *call_inst)
{
	// Get arguments operands
	auto out_args = call_inst->getOperand(0);

	// Get caller's argument scope and callee's function argument
	// scope
	VariableScope *callee_scope = callee_frame->getFunctionArguments();
	VariableScope *caller_scope = caller_frame->getArgumentScope();

	// Traverse all arguments
	for (auto it = arg_info.begin(); it != arg_info.end(); it++)
	{
		// Get argument information from the function
		Variable *argument = it->second.get();

		// Only process output argument
		if (!argument->isInput())
		{
			// Get argument index
			unsigned int index = argument->getIndex();

			// Get the directive information and name
			auto variable_decl = out_args->getElement(index);
			std::string name_in_caller = variable_decl->getName();

			// Get buffer in caller;
			char *caller_buffer =
					caller_scope->getBuffer(name_in_caller);
			char *callee_buffer =
					callee_scope->getBuffer(argument->getName());
			assert(caller_buffer && callee_buffer);

			// Copy memory
			BrigTypeX type = (BrigTypeX)argument->getType();
			unsigned short arg_size = AsmService::TypeToSize(type);
			memcpy(caller_buffer, callee_buffer, arg_size);
		}

	}

}



void Function::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n****************************************"
			"***************************************\n");

	// Dump function information
	os << misc::fmt("\tFunction name: %s.\n", name.c_str());

	// Dump argument related information
	DumpArgumentInfo(os);

	// Dump register related information
	DumpRegisterInfo(os);

	// Dump first and last entry
	if (getFunctionDirective()->getCodeBlockEntryCount() > 0)
	{
		// Dump first entry
		os << "\tFirst entry: ";
		getFirstEntry()->Dump(os);

		// Dump last entry
		os << "\tLast entry: ";
		getLastEntry()->Dump(os);
	}
	else
	{
		os << "\tThis function has no instruction nor directive\n";
	}

	os << misc::fmt("****************************************"
			"***************************************\n");
}


void Function::DumpArgumentInfo(std::ostream &os = std::cout) const
{

	// Dump the argument information
	os << misc::fmt("\n\t***** Arguments *****\n");
	std::map<std::string, std::unique_ptr<Variable>>::const_iterator it;
	for (it = arg_info.begin(); it != arg_info.end(); it++)
	{
		os << "\t";
		if (it->second->isInput())
			os << "Input ";
		else
			os << "Output ";
		it->second->Dump(os);
		os << "\n";
	}
	os << misc::fmt("\tArgument size allocated %d bytes\n", arg_size);
	os << misc::fmt("\t*********************\n\n");

}


void Function::DumpRegisterInfo(std::ostream &os = std::cout) const
{
	// Dump the argument information
	os << misc::fmt("\n\t***** Registers *****\n");
	for (auto it = reg_info.begin(); it != reg_info.end(); it++)
	{
		os << misc::fmt("\tregister %s, offset %d\n",
				it->first.c_str(),
				getRegisterOffset(it->first));
	}
	os << misc::fmt("\tRegister size allocated %d bytes\n", reg_size);
	os << misc::fmt("\t*********************\n\n");
}

}  // namespace HSA
