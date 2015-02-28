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

#include "WorkGroup.h"
#include "SegmentManager.h"

namespace HSA
{

WorkGroup::WorkGroup(Grid *grid,
		unsigned group_segment_size,
		unsigned int group_id_x,
		unsigned int group_id_y,
		unsigned int group_id_z)
{
	// Get grid object
	this->grid = grid;

	// Set group id
	this->group_id_x = group_id_x;
	this->group_id_y = group_id_y;
	this->group_id_z = group_id_z;

	// Set the group segment memory manager
	mem::Memory *memory = Emu::getInstance()->getMemory();
	group_segment.reset(new SegmentManager(memory, group_segment_size));
}


bool WorkGroup::Execute()
{
	bool on_going = false;
	for (auto it = wavefronts.begin(); it != wavefronts.end(); it++)
	{
		if (it->second->Execute())
			on_going = true;
	}
	return on_going;
}

void WorkGroup::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("  ***** %dD Work group (%d, %d, %d) ****\n",
			getGrid()->getDimension(), getGroupIdX(),
			getGroupIdY(), getGroupIdZ());
	os << misc::fmt("  ***** Group size (%d x %d x %d) *****\n",
			getGrid()->getGroupSizeX(),
			getGrid()->getGroupSizeY(),
			getGrid()->getGroupSizeZ());
	for (auto it = wavefronts.begin(); it != wavefronts.end(); it++)
	{
		os << *(it->second.get());
	}
	os << "  ***** ***** ***** ************** *****\n";
}


void WorkGroup::addWorkItem(std::unique_ptr<WorkItem> work_item)
{
	// Get work item id and wave front id it should belongs to
	unsigned int workitem_flattened_id = work_item->getFlattenedId();
	unsigned int wavefront_id = workitem_flattened_id /
			getGrid()->getComponent()->getWavesize();

	// Check if wavefront is created
	auto it = wavefronts.find(wavefront_id);
	Wavefront *wavefront;
	if (it == wavefronts.end())
	{
		wavefront = new Wavefront(this);
		wavefronts.insert(std::make_pair(wavefront_id,
				std::unique_ptr<Wavefront>(wavefront)));
	}
	else
	{
		wavefront = it->second.get();
	}


	// Insert the work item into the wave front
	wavefront->addWorkItem(std::move(work_item));
}


unsigned int WorkGroup::getGroupFlattenedId()
{
	return group_id_x +
			group_id_y * grid->getGroupSizeX() +
			group_id_z * grid->getGroupSizeX() *
			grid->getGroupSizeZ();
}

}

