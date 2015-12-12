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

#include "WorkItemAbsIdInstructionWorker.h"
#include "WorkItem.h"

namespace HSA 
{

WorkItemAbsIdInstructionWorker::WorkItemAbsIdInstructionWorker(
		WorkItem *work_item, StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame) 
{
}	


void WorkItemAbsIdInstructionWorker::RetrieveOperandValue()
{
	operand_value_retriever->Retrieve(instruction, 1, &dimension);
}


void WorkItemAbsIdInstructionWorker::RetrieveAbsoluteId()
{
	switch(dimension)
	{
	case 0:

		absolute_id = work_item->getAbsoluteIdX();
		break;

	case 1:

		absolute_id = work_item->getAbsoluteIdY();
		break;

	case 2:

		absolute_id = work_item->getAbsoluteIdZ();
		break;

	default:

		throw misc::Error("Trying to getting work item absolute id "
				"other than x, y and z axis.");
	}
}


void WorkItemAbsIdInstructionWorker::WriteResultBack()
{
	operand_value_writer->Write(instruction, 0, &absolute_id);
	work_item->MovePcForwardByOne();
}


void WorkItemAbsIdInstructionWorker::Execute(BrigCodeEntry *instruction) 
{
	this->instruction = instruction;
	RetrieveOperandValue();
	RetrieveAbsoluteId();
	WriteResultBack();
}

}


