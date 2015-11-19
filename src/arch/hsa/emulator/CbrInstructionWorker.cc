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

#include "CbrInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

CbrInstructionWorker::CbrInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
	// TODO Auto-generated constructor stub

}


CbrInstructionWorker::~CbrInstructionWorker()
{
	// TODO Auto-generated destructor stub
}


void CbrInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Retrieve condition
	unsigned char condition;
	operand_value_retriever->Retrieve(instruction, 0, &condition);

	// Jump if condition is true
	if (condition){
		// Retrieve 1st operand
		auto operand1 = instruction->getOperand(1);
		if (operand1->getKind() == BRIG_KIND_OPERAND_CODE_REF)
		{
			auto label = operand1->getRef();

			// Redirect pc to a certain label
			stack_frame->setPc(std::move(label));
			return;
		}else{
			throw misc::Panic("Unsupported operand type for CBR.");
		}
	}

	// Move PC forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
