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

#include "XorInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

XorInstructionWorker::XorInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


XorInstructionWorker::~XorInstructionWorker()
{
}


template<typename T>
void XorInstructionWorker::Inst_XOR_Aux(BrigCodeEntry *instruction)
{
	// Perform action
	T src0;
	T src1;
	operand_value_retriever->Retrieve(instruction, 1, &src0);
	operand_value_retriever->Retrieve(instruction, 2, &src1);
	T des = src0 ^ src1;
	operand_value_writer->Write(instruction, 0, &des);
}


void XorInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Do different action according to the kind of the inst
	switch (instruction->getType())
	{
	case BRIG_TYPE_B1:

		Inst_XOR_Aux<uint8_t>(instruction);
		break;

	case BRIG_TYPE_B32:

		Inst_XOR_Aux<unsigned int>(instruction);
		break;

	case BRIG_TYPE_B64:

		Inst_XOR_Aux<unsigned long long>(instruction);
		break;

	default:

		throw Error("Illegal type.");
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
