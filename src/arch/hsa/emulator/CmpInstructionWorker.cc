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

#include "CmpInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

CmpInstructionWorker::CmpInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}

CmpInstructionWorker::~CmpInstructionWorker()
{
}

void CmpInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	switch (instruction->getSourceType()){
	case BRIG_TYPE_B1:
	case BRIG_TYPE_B8:

		Inst_CMP_Aux<unsigned char, unsigned char>(instruction);
		break;

	case BRIG_TYPE_B16:
	case BRIG_TYPE_U16:

		Inst_CMP_Aux<unsigned short, unsigned char>(instruction);
		break;

	case BRIG_TYPE_B32:
	case BRIG_TYPE_U32:

		Inst_CMP_Aux<unsigned int, unsigned char>(instruction);
		break;

	case BRIG_TYPE_B64:
	case BRIG_TYPE_U64:

		Inst_CMP_Aux<unsigned long long, unsigned char>(instruction);
		break;

	case BRIG_TYPE_S32:

		Inst_CMP_Aux<int, unsigned char>(instruction);
		break;

	case BRIG_TYPE_S64:

		Inst_CMP_Aux<long long, unsigned char>(instruction);
		break;

	case BRIG_TYPE_F32:

		Inst_CMP_Aux<float, unsigned char>(instruction);
		break;

	case BRIG_TYPE_F64:

		Inst_CMP_Aux<double, unsigned char>(instruction);
		break;

	default:

		throw misc::Panic("Unsupported source type for CMP.");
		break;

	}
}

template<typename SrcType, typename DstType>
void CmpInstructionWorker::Inst_CMP_Aux(BrigCodeEntry *instruction)
{
	//Get source value
	SrcType src1;
	SrcType src2;
	DstType dst = 0;
	operand_value_retriever->Retrieve(instruction, 1, &src1);
	operand_value_retriever->Retrieve(instruction, 2, &src2);


	switch (instruction->getCompareOperation()){
	case BRIG_COMPARE_EQ:

		if (src1 == src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_NE:

		if (src1 != src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_LT:

		if (src1 < src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_LE:

		if (src1 <= src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_GT:

		if (src1 > src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_GE:

		if (src1 >= src2)
			dst = 1;
		else
			dst = 0;
		break;

	case BRIG_COMPARE_EQU:
	case BRIG_COMPARE_NEU:
	case BRIG_COMPARE_LTU:
	case BRIG_COMPARE_LEU:
	case BRIG_COMPARE_GTU:
	case BRIG_COMPARE_GEU:
	case BRIG_COMPARE_NUM:
	case BRIG_COMPARE_NAN:
	case BRIG_COMPARE_SEQ:
	case BRIG_COMPARE_SNE:
	case BRIG_COMPARE_SLT:
	case BRIG_COMPARE_SLE:
	case BRIG_COMPARE_SGT:
	case BRIG_COMPARE_SGE:
	case BRIG_COMPARE_SGEU:
	case BRIG_COMPARE_SEQU:
	case BRIG_COMPARE_SNEU:
	case BRIG_COMPARE_SLTU:
	case BRIG_COMPARE_SLEU:
	case BRIG_COMPARE_SNUM:
	case BRIG_COMPARE_SNAN:
	case BRIG_COMPARE_SGTU:
	default:
		throw misc::Panic("Unimplemented compare operation.");

	}

	// Store result value
	operand_value_writer->Write(instruction, 0, &dst);

	// Move PC forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
