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
#include <lib/cpp/Error.h>
#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/disassembler/Brig.h>
#include <arch/hsa/disassembler/BrigCodeEntry.h>

#include "LdaInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

LdaInstructionWorker::LdaInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


LdaInstructionWorker::~LdaInstructionWorker()
{
}


template<typename T>
void LdaInstructionWorker::Inst_LDA_Aux(BrigCodeEntry *instruction)
{
	// Retrieve operand
	auto address_operand = instruction->getOperand(1);
	auto symbol = address_operand->getSymbol();
	std::string name = symbol->getName();

	// Get offset
	uint64_t offset = address_operand->getOffset();

	// Declare addres
	unsigned address;

	// Get the symbol from stack top
	Variable *variable = stack_frame->getSymbol(name);
	if (!variable)
		throw misc::Error(misc::fmt("Variable %s not defined.\n",
					name.c_str()));
	address = variable->getAddress();

	// Add offset
	address += offset;

	// Save result
	operand_value_writer->Write(instruction, 0, (T*)&address);

	// Return
	return;
}


void LdaInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	switch(instruction->getType())
	{
	case BRIG_TYPE_U32:

		Inst_LDA_Aux<unsigned int>(instruction);
		break;

	case BRIG_TYPE_U64:

		Inst_LDA_Aux<unsigned long long>(instruction);
		break;

	default:

		throw misc::Error("Unsupported type for instruction LDA.");
		break;
	}

	// Move PC forward.
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
