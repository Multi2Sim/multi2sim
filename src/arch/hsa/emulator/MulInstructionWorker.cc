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

#include "MulInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

MulInstructionWorker::MulInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


MulInstructionWorker::~MulInstructionWorker()
{
}


template<typename T>
void MulInstructionWorker::Inst_MUL_Aux(BrigCodeEntry *instruction)
{
	T src0;
	T src1;
	operand_value_retriever->Retrieve(instruction, 1, &src0);
	operand_value_retriever->Retrieve(instruction, 2, &src1);
	T des = src0 * src1;
	operand_value_writer->Write(instruction, 0, &des);
}


void MulInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Do different action accoding to the kind of the inst
	if (instruction->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (instruction->getType())
		{
		case BRIG_TYPE_S32:

			Inst_MUL_Aux<int>(instruction);
			break;

		case BRIG_TYPE_S64:

			Inst_MUL_Aux<long long>(instruction);
			break;

		case BRIG_TYPE_U32:

			Inst_MUL_Aux<unsigned int>(instruction);
			break;

		case BRIG_TYPE_U64:

			Inst_MUL_Aux<unsigned long long>(instruction);
			break;

		default:

			throw Error("Illegal type.");
		}
	}
	else if (instruction->getKind() == BRIG_KIND_INST_MOD)
	{
		switch (instruction->getType())
		{
		case BRIG_TYPE_F32:

			Inst_MUL_Aux<float>(instruction);
			break;

		case BRIG_TYPE_F64:

			Inst_MUL_Aux<double>(instruction);
			break;

		default:

			throw Error("Illegal type.");
		}
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
