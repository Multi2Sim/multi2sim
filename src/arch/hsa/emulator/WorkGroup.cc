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
	mem::Memory *memory = Emulator::getInstance()->getMemory();
	group_segment.reset(new SegmentManager(memory, group_segment_size));
}

WorkGroup::~WorkGroup()
{
}


bool WorkGroup::Execute()
{
	bool active = false;
	auto it = wavefronts.begin();
	while (it != wavefronts.end())
	{
		if (it->second->Execute())
		{
			active = true;
			it++;
		}
		else
		{
			it = wavefronts.erase(it);
		}
	}
	return active;
}


void WorkGroup::HitBarrier(unsigned int abs_flat_id)
{
	// Add an record to the set of suspended work items
	work_items_on_hold.insert(abs_flat_id);
		
	// Check if all the work item reaches the barrier
	if (work_items_on_hold.size() == num_work_items)
	{
		ActivateAllWorkItems();
		work_items_on_hold.clear();
	}
}


void WorkGroup::ActivateAllWorkItems()
{
	for (auto it = wavefronts.begin(); it != wavefronts.end(); it++)
	{
		it->second->ActivateAllWorkItems();
	}
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
		wavefronts.insert(std::make_pair(wavefront_id,
				misc::new_unique<Wavefront>(wavefront_id, this)
				));
		wavefront = wavefronts.at(wavefront_id).get();
	}
	else
	{
		wavefront = it->second.get();
	}

	// Current workgroup size
	unsigned int local_id_x = work_item->getLocalIdX();
	unsigned int local_id_y = work_item->getLocalIdY();
	unsigned int local_id_z = work_item->getLocalIdZ();
	if (local_id_x > current_group_size_x)
		current_group_size_x = local_id_x;
	if (local_id_y > current_group_size_y)
		current_group_size_y = local_id_y;
	if (local_id_z > current_group_size_z)
		current_group_size_z = local_id_z;

	// Insert the work item into the wave front
	wavefront->addWorkItem(std::move(work_item));

	// Increase the work item count by one
	num_work_items ++;
}


unsigned int WorkGroup::getGroupFlattenedId()
{
	return group_id_x +
			group_id_y * grid->getGroupSizeX() +
			group_id_z * grid->getGroupSizeX() *
			grid->getGroupSizeZ();
}


unsigned int WorkGroup::getCurrentWorkGroupSize(unsigned int dim)
{
	switch (dim)
	{
	case 0:
		return current_group_size_x;
	case 1:
		return current_group_size_y;
	case 2:
		return current_group_size_z;
	}
	return 0;
}

}

