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

#include "Grid.h"

namespace HSA
{

Grid::Grid(Component *component, AQLDispatchPacket *packet)
{
	// Set component
	this->component = component;

	// Set grid information
	dimension = packet->getDimension();
	grid_size_x = packet->getGridSizeX();
	grid_size_y = packet->getGridSizeY();
	grid_size_z = packet->getGridSizeZ();
	grid_size = grid_size_x * grid_size_y * grid_size_z;
	std::cout << misc::fmt("Deploying grid size %d (%d x %d x %d).\n",
			grid_size, grid_size_x, grid_size_y, grid_size_z);


	// Set work group information
	group_size_x = packet->getWorkGroupSizeX();
	group_size_y = packet->getWorkGroupSizeY();
	group_size_z = packet->getWorkGroupSizeZ();
	group_size = group_size_x * group_size_y * group_size_z;

	// Set root function information
	this->root_function = (Function *)packet->getKernalObjectAddress();
	this->kernel_args = packet->getKernargAddress();

	// Create work items
	for (unsigned int i; i < grid_size; i++)
	{
		unsigned int z = i / group_size_x / group_size_y;
		unsigned int y = i % (group_size_x *group_size_y) / group_size_x;
		unsigned int x = i % (group_size_x * group_size_y) % group_size_x;

		std::cout << misc::fmt("About to create work item (%d, %d, "
				"%d)\n", x, y, z);

		this->deployWorkItem(x, y, z);
	}

}


Grid::~Grid()
{
}


bool Grid::Execute()
{
	return false;
}


void Grid::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("***** %dD Grid (%d x %d x %d) *****\n",
			dimension, getGridSizeX(),
			getGridSizeY(), getGridSizeZ());
	for (auto it = workgroups.begin(); it != workgroups.end(); it++)
	{
		os << *(it->second.get());
	}
	os << "***** **** *****\n";
}


void Grid::deployWorkItem(unsigned int abs_id_x,
			unsigned int abs_id_y,
			unsigned int abs_id_z)
{
	// Get work group id
	unsigned int group_id_x = abs_id_x / group_size_x;
	unsigned int group_id_y = abs_id_y / group_size_y;
	unsigned int group_id_z = abs_id_z / group_size_z;
	unsigned int group_flattened_id = group_id_x +
			group_id_y * group_size_x +
			group_id_z * group_size_x * group_size_y;

	// Check if the work group exist. If not, create it.
	auto it = workgroups.find(group_flattened_id);
	if (it == workgroups.end())
	{
		createWorkGroup(group_id_x, group_id_y, group_id_z);
		it = workgroups.find(group_flattened_id);
	}
	WorkGroup *work_group = it->second.get();

	// Create work item
	WorkItem *work_item = new WorkItem(work_group,
			abs_id_x, abs_id_y, abs_id_z,
			root_function, kernel_args);

	// Add created work item into work group
	work_group->addWorkItem(work_item);
}


void Grid::createWorkGroup(unsigned int id_x, unsigned int id_y,
			unsigned int id_z)
{
	WorkGroup *work_group = new WorkGroup(this, id_x, id_y, id_z);
	unsigned int flattened_id = work_group->getGroupFlattenedId();
	workgroups.insert(std::make_pair(flattened_id,
			std::unique_ptr<WorkGroup>(work_group)));
}

}  // namespace HSA
