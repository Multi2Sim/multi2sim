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


void WorkItemAbsIdInstructionWorker::Execute(BrigCodeEntry *instruction) 
{
	unsigned int dim;
	getOperandValue(instruction, 1, &dim);
	unsigned int abs_id;
	switch(dim)
	{
	case 0:

		abs_id = work_item->getAbsoluteIdX();
		break;

	case 1:

		abs_id = work_item->getAbsoluteIdY();
		break;

	case 2:

		abs_id = work_item->getAbsoluteIdZ();
		break;

	default:
		
		throw misc::Error("Trying to getting work item absolute id "
				"other than x, y and z axis.");
	}

	// Write the result back
	setOperandValue(instruction, 0, &abs_id);

	// Move pc to next instruction
	work_item->MovePcForwardByOne();
}

}


