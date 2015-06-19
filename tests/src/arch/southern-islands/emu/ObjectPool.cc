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

#include <arch/southern-islands/emu/NDRange.h>
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/WorkItem.h>
#include <arch/southern-islands/asm/Asm.h>
#include <arch/southern-islands/asm/Inst.h>
#include <gtest/gtest.h>

#include "ObjectPool.h"

namespace SI
{

NDRange *ObjectPool::SetNDRangeSize()
{
	// Set local size, global size, and work dimension
	int work_dim = 1;
	unsigned global_size[1] = {1};
	unsigned local_size[1] = {1};
	ndrange.SetupSize((unsigned *) &global_size, 
			(unsigned * ) &local_size, work_dim);

	// Return 
	return &ndrange;
}

ObjectPool::ObjectPool() : as(Asm::getInstance()), emu(Emu::getInstance()),
		ndrange(emu), work_group(SetNDRangeSize(), 0), 
		wavefront(&work_group, 0), work_item(&wavefront, 0), 
		inst(as)
{
	// Assign work item to a work group
	work_item.setWorkGroup(&work_group);
}


}

