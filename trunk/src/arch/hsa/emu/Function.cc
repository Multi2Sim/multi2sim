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

#include <lib/cpp/String.h>
#include <arch/hsa/asm/BrigEntry.h>
#include <arch/hsa/asm/AsmService.h>

#include "Emu.h"
#include "AQLQueue.h"
#include "Function.h"

namespace HSA
{

Function::Function(const std::string& name) :
		name(name)
{
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


unsigned int Function::getRegisterOffset(BrigRegisterKind kind,
		unsigned short number) const
{
	unsigned int offset = 0;
	switch(kind)
	{
	case BRIG_REGISTER_CONTROL:
		return 0;
		break;
	case BRIG_REGISTER_SINGLE:
		return number;
		break;
	case BRIG_REGISTER_DOUBLE:
		offset += max_reg[BRIG_REGISTER_SINGLE];
		return offset + 2 * number;
		break;
	case BRIG_REGISTER_QUAD:
		offset += max_reg[BRIG_REGISTER_QUAD];
		offset += max_reg[2] * 2;
		return offset + 2 * number;
		break;
	}
	return 0;
}

/*
void Function::addRegister(BrigRegisterKind kind, unsigned short number)
{
	// Validate the name of the register
	int size = getRegisterSizeByName(kind);

	// Skip C registers
	// if (size == 1)
	//	return;

	// Check if the register exists
	int offset = getRegisterOffset(kind, number);
	if (offset >= 0)
		return;

	// Check if enough space is available
	if (reg_size + size > 512)
		throw Error(misc::fmt("No enough space to allocated register %s"
				"in function %s.",
				name.c_str(), this->name.c_str()));

	// Insert the register information
	reg_info.insert(std::make_pair(name, reg_size));
	reg_size += size;
}
*/


void Function::AllocateRegister(unsigned int *max_reg)
{
	reg_size += max_reg[BRIG_REGISTER_SINGLE] * 1;
	reg_size += max_reg[BRIG_REGISTER_DOUBLE] * 2;
	reg_size += max_reg[BRIG_REGISTER_QUAD] * 4;
	for (unsigned int i = 0; i < 4; i++)
	{
		this->max_reg[i] = max_reg[i];
	}
}


/*
void Function::PassByValue(VariableScope *caller_scope,
			VariableScope *callee_scope, BrigInstEntry *call_inst)
{
	// Get arguments operands
	//BrigOperandArgumentList *out_args =
	//		(BrigOperandArgumentList *)call_inst->getOperand(0);
	BrigOperandArgumentList *in_args =
			(BrigOperandArgumentList *)call_inst->getOperand(2);

	// Get the binary
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();

	for (auto it = arg_info.begin(); it != arg_info.end(); it++)
	{
		// Get argument information from the function
		Variable *argument = it->second.get();

		// Insert argument into callee's function argument scope
		callee_scope->DeclearVariable(argument->getName(),
				argument->getSize(), argument->getType());

		// Copy argument's value
		if (argument->isInput())
		{
			// Get argument index
			unsigned int index = argument->getIndex();

			// Get the directive information and name
			BrigDirectiveVariable *variable_dir =
					(BrigDirectiveVariable *)
					BrigDirEntry::GetDirByOffset(binary,
							in_args->elements[index]);
			std::string name_in_caller =
					BrigStrEntry::GetStringByOffset(binary,
					variable_dir->name);

			// Get buffer in caller;
			char *caller_buffer =
					caller_scope->getBuffer(name_in_caller);
			char *callee_buffer =
					callee_scope->getBuffer(argument->getName());

			// Copy memory
			memcpy(callee_buffer, caller_buffer,
					BrigEntry::type2size(
							argument->getType()));

		}
	}
}
*/

/*
void Function::PassBackByValue(VariableScope *caller_scope,
			VariableScope *callee_scope, BrigInstEntry *call_inst)
{
	// Get the binary
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();

	// Get arguments operands
	BrigOperandArgumentList *out_args =
			(BrigOperandArgumentList *)call_inst->getOperand(0);

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
			BrigDirectiveVariable *variable_dir =
					(BrigDirectiveVariable *)
					BrigDirEntry::GetDirByOffset(binary,
							out_args->elements[index]);
			std::string name_in_caller =
					BrigStrEntry::GetStringByOffset(binary,
					variable_dir->name);

			// Get buffer in caller;
			char *caller_buffer =
					caller_scope->getBuffer(name_in_caller);
			char *callee_buffer =
					callee_scope->getBuffer(argument->getName());

			// Copy memory
			memcpy(caller_buffer, callee_buffer,
					BrigEntry::type2size(
							argument->getType()));
		}

	}

}
*/


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
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < max_reg[i]; j++)
		{
			os << misc::fmt("\tregister %s%d, offset %d\n",
					AsmService::RegisterKindToString((BrigRegisterKind)i).c_str(),
					j,
					getRegisterOffset((BrigRegisterKind)i, j));
		}
	}
	os << misc::fmt("\tRegister size allocated %d bytes\n", reg_size);
	os << misc::fmt("\t*********************\n\n");
}

}  // namespace HSA
