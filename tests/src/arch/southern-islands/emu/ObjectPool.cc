/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/cpp/Misc.h>
#include <gtest/gtest.h>

#include "ObjectPool.h"

namespace SI
{

ObjectPool::ObjectPool()
{
	// Get disassembler and emulator singletons
	as = Disassembler::getInstance();
	emulator = Emulator::getInstance();

	// Allocate NDRange
	ndrange = misc::new_unique<NDRange>();
	
	// Set local size, global size, and work dimension
	int work_dim = 1;
	unsigned global_size[1] = {1};
	unsigned local_size[1] = {1};
	ndrange->SetupSize((unsigned *) &global_size, 
			(unsigned * ) &local_size, work_dim);

	// Allocate Work Group, Wavefront, and Work Item
	work_group = misc::new_unique<WorkGroup>(ndrange.get(), 0);
	wavefront = misc::new_unique<Wavefront>(work_group.get(), 0);
	work_item = misc::new_unique<WorkItem>(wavefront.get(), 0);
	
	// Assign work item to a work group
	work_item->setWorkGroup(work_group.get());

	// Create a new Instruction
	inst = misc::new_unique<Instruction>();
}


}

