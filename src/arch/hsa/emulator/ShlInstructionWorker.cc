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

#include "ShlInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

ShlInstructionWorker::ShlInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


ShlInstructionWorker::~ShlInstructionWorker()
{
}


template<typename T>
void ShlInstructionWorker::Inst_SHL_Aux(BrigCodeEntry *instruction)
{
	// Perform action
	T src0;
	unsigned int src1;
	operand_value_retriever->Retrieve(instruction, 1, &src0);
	operand_value_retriever->Retrieve(instruction, 2, &src1);
	T des = src0 << src1;
	operand_value_writer->Write(instruction, 0, &des);
}


void ShlInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	switch (instruction->getType())
	{
	case BRIG_TYPE_S32:

		Inst_SHL_Aux<int>(instruction);
		break;

	case BRIG_TYPE_S64:

		Inst_SHL_Aux<long long>(instruction);
		break;

	case BRIG_TYPE_U32:

		Inst_SHL_Aux<unsigned int>(instruction);
		break;

	case BRIG_TYPE_U64:

		Inst_SHL_Aux<unsigned long long>(instruction);
		break;

	default:

		throw misc::Error("Illegal type.");
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
