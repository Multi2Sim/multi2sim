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

#include <cstring>

#include <arch/hsa/asm/AsmService.h>

#include "ProgramLoader.h"
#include "WorkGroup.h"
#include "WorkItem.h"
#include "Grid.h"
#include "SegmentManager.h"

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

	// Set work group information
	group_size_x = packet->getWorkGroupSizeX();
	group_size_y = packet->getWorkGroupSizeY();
	group_size_z = packet->getWorkGroupSizeZ();
	group_size = group_size_x * group_size_y * group_size_z;

	// Set root function information
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	auto function = binary->getCodeEntryByOffset(packet->getKernalObjectAddress());
	std::string function_name = function->getName();
	root_function = ProgramLoader::getInstance()->getFunction(
			function_name);
	kernel_args = packet->getKernargAddress();

	// Create kernel argument
	mem::Memory *memory = Emu::getInstance()->getMemory();
	kernarg_segment.reset(new SegmentManager(memory,
			root_function->getArgumentSize()));
	kernel_arguments.reset(new VariableScope());
	BrigCodeEntry *function_directive =
			root_function->getFunctionDirective();
	auto arg_entry = function_directive->Next();
	for (int i = 0; i < function_directive->getInArgCount(); i++)
	{
		std::string name = arg_entry->getName();
		unsigned inseg_address = kernel_arguments->DeclearVariable(name,
				arg_entry->getType(), arg_entry->getDim(),
				kernarg_segment.get());
		unsigned size = AsmService::TypeToSize(arg_entry->getType());

		// Get the buffer from both host and guest
		char *buffer = kernel_arguments->getBuffer(name);
		char *host_buffer = memory->getBuffer(kernel_args + inseg_address,
				size, mem::Memory::AccessRead);
		memcpy(buffer, host_buffer, size);

		// Move arg_entry forward
		arg_entry = arg_entry->Next();
	}


	// Create work items
	for (unsigned int i = 0; i < grid_size; i++)
	{
		unsigned int z = i / group_size_x / group_size_y;
		unsigned int y = i % (group_size_x *group_size_y) / group_size_x;
		unsigned int x = i % (group_size_x * group_size_y) % group_size_x;
		deployWorkItem(x, y, z, packet->getPrivateSegmentSizeBytes(),
				packet->getGroupSegmentSizeBytes());
	}
}


Grid::~Grid()
{
}


bool Grid::Execute()
{
	bool on_going = false;

	for (auto it = workgroups.begin(); it != workgroups.end(); it++)
	{
		if (it->second->Execute())
			on_going = true;
	}

	return on_going;
}


void Grid::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("***** %dD Grid (%d x %d x %d) *****\n",
			dimension, getGridSizeX(),
			getGridSizeY(), getGridSizeZ());

	// Dump kernel arguments
	os << misc::fmt("\n\t***** Arguments *****\n");
	kernel_arguments->Dump(os, 2);
	os << misc::fmt("\n\t***** ********* *****\n");

	// Dump work items
	for (auto it = workgroups.begin(); it != workgroups.end(); it++)
	{
		os << *(it->second.get());
	}

	os << "***** **** *****\n";
}


void Grid::deployWorkItem(unsigned int abs_id_x,
			unsigned int abs_id_y,
			unsigned int abs_id_z,
			unsigned private_segment_size,
			unsigned group_segment_size)
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
		createWorkGroup(group_id_x, group_id_y, group_id_z,
				group_segment_size);
		it = workgroups.find(group_flattened_id);
	}
	WorkGroup *work_group = it->second.get();

	// Create work item
	auto work_item = misc::new_unique<WorkItem>(work_group,
			private_segment_size,
			abs_id_x, abs_id_y, abs_id_z,
			root_function);

	// Add created work item into work group
	work_group->addWorkItem(std::move(work_item));
}


void Grid::createWorkGroup(unsigned int id_x, unsigned int id_y,
			unsigned int id_z, unsigned group_segment_size)
{
	auto work_group = misc::new_unique<WorkGroup>(this, group_segment_size,
			id_x, id_y, id_z);
	unsigned int flattened_id = work_group->getGroupFlattenedId();
	workgroups.insert(std::make_pair(flattened_id,
			std::move(work_group)));
}


}  // namespace HSA
