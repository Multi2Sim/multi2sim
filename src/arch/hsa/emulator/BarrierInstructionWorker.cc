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

#include "BarrierInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

BarrierInstructionWorker::BarrierInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		HsaInstructionWorker(work_item, stack_frame)
{
}


BarrierInstructionWorker::~BarrierInstructionWorker()
{
}


void BarrierInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	// Suspend the work item
	work_item->setStatus(WorkItem::WorkItemStatusSuspend);

	// Notify the work group
	work_item->getWorkGroup()->HitBarrier(
			work_item->getAbsoluteFlattenedId());

	// Move PC forward
	work_item->MovePcForwardByOne();
}

}  // namespace HSA
