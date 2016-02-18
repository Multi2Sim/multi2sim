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
#include <cinttypes>

#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/driver/HsaExecutable.h>
#include <arch/hsa/driver/HsaExecutableSymbol.h>
#include <arch/hsa/driver/SignalManager.h>

#include "WorkGroup.h"
#include "WorkItem.h"
#include "Grid.h"
#include "SegmentManager.h"

namespace HSA
{

Grid::Grid(Component *component, AQLDispatchPacket *packet)
{
	// Set packet
	this->packet = packet;

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

	// Get kernel object
	HsaExecutableSymbol *kernel_object = (HsaExecutableSymbol *)
			packet->getKernalObjectAddress();
	std::string function_name = kernel_object->getDirective()->getName();
	root_function = kernel_object->getExecutable()->
			getFunction(function_name);
	kernel_args = packet->getKernargAddress();

	// Create and copy kernel argument
	mem::Memory *memory = Emulator::getInstance()->getMemory();
	kernarg_segment = misc::new_unique<SegmentManager>(memory, 
			root_function->getArgumentSize());
	kernel_arguments.clear();
	BrigCodeEntry *function_directive =
			root_function->getFunctionDirective();
	auto argument_entry = function_directive->Next();
	unsigned input_argument_offset = 0;
	for (int i = 0; i < function_directive->getInArgCount(); i++)
	{
		// Allocate the memory in the kernarg segment
		std::string name = argument_entry->getName();
		unsigned argument_size = AsmService::TypeToSize(
				argument_entry->getType());
		unsigned dim = argument_entry->getDim();
		if (dim == 0) dim = 1;
		unsigned address = kernarg_segment->Allocate(
				argument_size * dim);

		// Add the argument information to the kernel arguments list
		auto argument = misc::new_unique<Variable>(name, 
				argument_entry->getType(), dim, 
				address, BRIG_SEGMENT_KERNARG,false);
		kernel_arguments.emplace(name, std::move(argument));

		// Copy argument
		auto host_buffer = misc::new_unique_array<char>(
						argument_size * dim);
		memory->Read(kernel_args + input_argument_offset, 
				argument_size * dim, 
				host_buffer.get());
		unsigned flat_address = kernarg_segment->getFlatAddress(
				address);
		memory->Write(flat_address, argument_size * dim, 
				host_buffer.get());

		// Move arg_entry forward
		argument_entry = argument_entry->Next();

		// Set the input argument offset forward
		input_argument_offset += argument_size * dim;
	}


	// Create work items
	for (unsigned int i = 0; i < grid_size; i++)
	{
		unsigned int z = i / grid_size_x / grid_size_y;
		unsigned int y = i % (grid_size_x * grid_size_y) / grid_size_x;
		unsigned int x = i % (grid_size_x * grid_size_y) % grid_size_x;
		deployWorkItem(x, y, z, packet->getPrivateSegmentSizeBytes(),
				packet->getGroupSegmentSizeBytes());
	}

	// Retrieve signal manager
	signal_manager = Driver::getInstance()->getSignalManager();
}


Grid::~Grid()
{
}


bool Grid::Execute()
{
	bool active = false;
	auto it = workgroups.begin();
	while (it != workgroups.end())
	{

		if (it->second->Execute())
		{
			active = true;
			it++;
		}
		else
		{
			it = workgroups.erase(it);
		}
	}

	// Send completion signal when finished execution
	if (!active)
	{
		uint64_t completion_signal = packet->getCompletionSignal();
		int64_t signal_value = signal_manager->GetValue(
				completion_signal);
		Emulator::isa_debug << misc::fmt("Kernel execution finished, "
				"reducing completion signal from %" PRId64 " to %" PRId64 "\n",
				signal_value, signal_value - 1);
		signal_value--;
		signal_manager->ChangeValue(completion_signal, signal_value);
	}

	return active;
}


void Grid::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("***** %dD Grid (%d x %d x %d) *****\n",
			dimension, getGridSizeX(),
			getGridSizeY(), getGridSizeZ());

	// Dump kernel arguments
	os << misc::fmt("\n\t***** Arguments *****\n\n");
	// kernel_arguments->Dump(os, 2);
	os << misc::fmt("\n\t***** ********* *****\n\n");

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
	auto work_item = misc::new_unique<WorkItem>();
	work_item->Initialize(work_group,
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
