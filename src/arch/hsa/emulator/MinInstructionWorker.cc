/*
 * MinInstructionWorker.cc
 *
 *  Created on: May 18, 2016
 *      Author: yifan
 */

#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>
#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/disassembler/Brig.h>
#include <arch/hsa/disassembler/BrigCodeEntry.h>

#include "MinInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

MinInstructionWorker::MinInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}

MinInstructionWorker::~MinInstructionWorker ()
{
}

template<typename T>
void MinInstructionWorker::Inst_Min_Aux(BrigCodeEntry *instruction)
{
	// Perform action
	T src0;
	T src1;
	operand_value_retriever->Retrieve(instruction, 1, &src0);
	operand_value_retriever->Retrieve(instruction, 2, &src1);
	T des = (src0 < src1) ? src0 : src1;
	operand_value_writer->Write(instruction, 0, &des);
}

void MinInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Do different action accoding to the kind of the inst
	if (instruction->getKind() == BRIG_KIND_INST_BASIC)
	{
		switch (instruction->getType())
		{
		case BRIG_TYPE_S32:

			Inst_Min_Aux<int>(instruction);
			break;

		case BRIG_TYPE_S64:

			Inst_Min_Aux<long long>(instruction);
			break;

		case BRIG_TYPE_U32:

			Inst_Min_Aux<unsigned int>(instruction);
			break;

		case BRIG_TYPE_U64:

			Inst_Min_Aux<unsigned long long>(instruction);
			break;

		case BRIG_TYPE_F32:

			Inst_Min_Aux<float>(instruction);
			break;

		case BRIG_TYPE_F64:

			Inst_Min_Aux<float>(instruction);
			break;

		default:

			throw Error("Illegal type.");
		}
	}
	else if (instruction->getKind() == BRIG_KIND_INST_MOD)
	{
		throw misc::Panic("Unimplemented Inst MIN, "
				"kind BRIG_KIND_INST_MOD.");
	}
	else
	{
		throw Error("Unsupported Inst kind.");
	}

	// Move the pc forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
