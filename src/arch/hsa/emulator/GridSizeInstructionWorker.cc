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

#include "GridSizeInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

GridSizeInstructionWorker::GridSizeInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


GridSizeInstructionWorker::~GridSizeInstructionWorker()
{
}


void GridSizeInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	unsigned int dim_number;
	unsigned int size;
	operand_value_retriever->Retrieve(instruction, 1, &dim_number);

	switch(dim_number)
	{
	case 0:

		size = work_item->getGrid()->getGridSizeX();
		break;

	case 1:

		size = work_item->getGrid()->getGridSizeY();
		break;

	case 2:

		size = work_item->getGrid()->getGridSizeZ();
		break;

	default:

		throw Error(misc::fmt("Invaid dim_number %d.\n", dim_number));
	}

	operand_value_writer->Write(instruction, 0, &size);
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
