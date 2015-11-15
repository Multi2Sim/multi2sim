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

#include "CvtInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

CvtInstructionWorker::CvtInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


CvtInstructionWorker::~CvtInstructionWorker()
{
}


template<typename SrcType, typename DstType>
void CvtInstructionWorker::Inst_CVT_chop_Aux(BrigCodeEntry *instruction)
{
	SrcType src;
	operand_value_retriever->Retrieve(instruction, 1, &src);
	DstType dst = 0;

	// Use memcpy to chop
	memcpy(&dst, &src, sizeof(DstType));
	operand_value_writer->Write(instruction, 0, &dst);
}


template<typename SrcType, typename DstType>
void CvtInstructionWorker::Inst_CVT_zext_Aux(BrigCodeEntry *instruction)
{
	SrcType src;
	operand_value_retriever->Retrieve(instruction, 1, &src);

	// Force cast
	DstType dst = (DstType)src;
	operand_value_writer->Write(instruction, 0, &dst);
}


template<typename SrcType, typename DstType>
void CvtInstructionWorker::Inst_CVT_sext_Aux(BrigCodeEntry *instruction)
{
	SrcType src;
	operand_value_retriever->Retrieve(instruction, 1, &src);

	// Force cast
	DstType dst = (DstType)src;
	operand_value_writer->Write(instruction, 0, &dst);
}


template<typename SrcType, typename DstType>
void CvtInstructionWorker::Inst_CVT_u2f_Aux(BrigCodeEntry *instruction)
{
	SrcType src;
	operand_value_retriever->Retrieve(instruction, 1, &src);

	// Force cast
	DstType dst = (DstType)src;
	operand_value_writer->Write(instruction, 0, &dst);
}


void CvtInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Get src type and dst type
	BrigType src_type = instruction->getSourceType();
	BrigType dst_type = instruction->getType();

	if (src_type == BRIG_TYPE_U16 && dst_type == BRIG_TYPE_U32)
	{
		Inst_CVT_zext_Aux<unsigned short, unsigned int>(instruction);
	}
	else if (src_type == BRIG_TYPE_U32 && dst_type == BRIG_TYPE_U64)
	{
		Inst_CVT_zext_Aux<unsigned int, unsigned long long>(instruction);
	}
	else if (src_type == BRIG_TYPE_U64 && dst_type == BRIG_TYPE_U32)
	{
		Inst_CVT_chop_Aux<unsigned long long, unsigned int>(instruction);
	}
	else if (src_type == BRIG_TYPE_U32 && dst_type == BRIG_TYPE_U8)
	{
		Inst_CVT_chop_Aux<unsigned int, unsigned char>(instruction);
	}
	else if (src_type == BRIG_TYPE_U64 && dst_type == BRIG_TYPE_U8)
	{
		Inst_CVT_chop_Aux<unsigned long long, unsigned char>(instruction);
	}
	else if(src_type == BRIG_TYPE_U64 && dst_type == BRIG_TYPE_F32)
	{
		Inst_CVT_u2f_Aux<unsigned long long, float>(instruction);
	}
	else if (src_type == BRIG_TYPE_S32 && dst_type == BRIG_TYPE_S64)
	{
		Inst_CVT_sext_Aux<int, long long>(instruction);
	}
	else
	{
		throw misc::Panic(misc::fmt("Conversion between %s and %s "
				"is not supported\n",
				AsmService::TypeToString(src_type).c_str(),
				AsmService::TypeToString(dst_type).c_str()));
	}

	// Move PC forward
	work_item->MovePcForwardByOne();
}

}

