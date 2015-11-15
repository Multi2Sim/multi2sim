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

#include <arch/hsa/disassembler/BrigCodeEntry.h>

#include "LdInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

LdInstructionWorker::LdInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}

LdInstructionWorker::~LdInstructionWorker()
{
	// TODO Auto-generated destructor stub
}

template<typename T>
void LdInstructionWorker::Inst_LD_Aux(BrigCodeEntry *instruction)
{
	// Get address according the type of the operand
	unsigned address;
	operand_value_retriever->Retrieve(instruction, 1, &address);

	// Translate address to flat address
	address = work_item->getFlatAddress(instruction->getSegment(), address);

	// Read data
	unsigned vector_modifier = instruction->getVectorModifier();
	if (vector_modifier == 0) vector_modifier = 1;
	auto value = misc::new_unique_array<T>(vector_modifier);
	Emulator::getInstance()->getMemory()->Read(address,
			sizeof(T) * vector_modifier,
			(char *)value.get());

	// Move value from register or immediate into memory
	operand_value_writer->Write(instruction, 0, value.get());
}


void LdInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	switch (instruction->getType())
	{
	case BRIG_TYPE_U32:

		Inst_LD_Aux<unsigned int>(instruction);
		break;

	case BRIG_TYPE_S32:

		Inst_LD_Aux<int>(instruction);
		break;

	case BRIG_TYPE_F32:

		Inst_LD_Aux<float>(instruction);
		break;

	case BRIG_TYPE_U64:

		Inst_LD_Aux<unsigned long long>(instruction);
		break;

	case BRIG_TYPE_S64:

		Inst_LD_Aux<long long>(instruction);
		break;

	case BRIG_TYPE_F64:

		Inst_LD_Aux<double>(instruction);
		break;

	default:

		throw misc::Panic("Unimplemented type for Inst LD.");
		break;
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
