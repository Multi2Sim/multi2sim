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

#include "StInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

StInstructionWorker::StInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


StInstructionWorker::~StInstructionWorker()
{
}


template<typename T>
void StInstructionWorker::Inst_ST_Aux(BrigCodeEntry *instruction)
{
	// Get address to store
	unsigned address;
	operand_value_retriever->Retrieve(instruction, 1, &address);

	// Translate address to flat address
	address = work_item->getFlatAddress(instruction->getSegment(), address);

	// Move value from register or immediate into memory
	T src0;
	operand_value_retriever->Retrieve(instruction, 0, &src0);
	mem::Memory *memory = Emulator::getInstance()->getMemory();
	memory->Write(address, sizeof(T), (char *)&src0);
}


void StInstructionWorker::Execute(BrigCodeEntry *instruction) {
	// Get type
	switch (instruction->getType())
	{
	case BRIG_TYPE_U8:
	case BRIG_TYPE_S8:

		Inst_ST_Aux<unsigned char>(instruction);
		break;

	case BRIG_TYPE_U16:
	case BRIG_TYPE_S16:

		Inst_ST_Aux<unsigned short>(instruction);
		break;

	case BRIG_TYPE_U32:
	case BRIG_TYPE_S32:

		Inst_ST_Aux<unsigned int>(instruction);
		break;

	case BRIG_TYPE_F32:
		Inst_ST_Aux<float>(instruction);
		break;

	case BRIG_TYPE_U64:
	case BRIG_TYPE_S64:

		Inst_ST_Aux<unsigned long long>(instruction);
		break;

	case BRIG_TYPE_F64:
		Inst_ST_Aux<double>(instruction);
		break;

	default:
		throw misc::Panic("Type is not supported");
		break;
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
