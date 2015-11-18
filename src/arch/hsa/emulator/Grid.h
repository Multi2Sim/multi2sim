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

#ifndef ARCH_HSA_EMULATOR_GRID_H
#define ARCH_HSA_EMULATOR_GRID_H

#include <map>

// #include "WorkGroup.h"
#include "AQLPacket.h"
#include "Component.h"


namespace HSA
{

class WorkGroup;
class Component;
class Function;
class SegmentManager;
class Variable;
class SignalManager;

// A grid is an instance of a kernel execution, equivalent to NDRange in OpenCL
class Grid
{
	// Component it belongs to
	Component *component;

	// The packet that launches this kernel
	AQLDispatchPacket *packet;

	// The signal manager
	SignalManager *signal_manager;

	// Dimension
	unsigned short dimension;

	// Grid size x, y and z
	unsigned int grid_size_x;
	unsigned int grid_size_y;
	unsigned int grid_size_z;

	// Grid size in number of work items
	unsigned int grid_size;

	// Work group size
	unsigned int group_size_x;
	unsigned int group_size_y;
	unsigned int group_size_z;

	// Work group size, number of work items in a work group
	unsigned int group_size;

	// Root function to execute
	Function *root_function;

	// Pointer (in guest memory) to kernel arguments
	unsigned long long kernel_args;

	// Segment manager for kernel args
	std::unique_ptr<SegmentManager> kernarg_segment;

	// Kernal arguments
	std::map<std::string, std::unique_ptr<Variable>> kernel_arguments;

	// List of work groups, maps work group flattened absolute id
	std::map<unsigned int, std::unique_ptr<WorkGroup>> workgroups;

	// Deploy tasks on work item
	void deployWorkItem(unsigned int abs_id_x,
	 		unsigned int abs_id_y,
	 		unsigned int abs_id_z,
	 		unsigned private_segment_size,
	 		unsigned group_segment_size);

	// Create work group
	void createWorkGroup(unsigned int id_x, unsigned int id_y,
			unsigned int id_z, unsigned group_segment_size);

public:

	/// Constructor
	///
	/// \param packet
	/// 	The packet that contains the information about the launch of
	///	this kernel
	Grid(Component* component, AQLDispatchPacket *packet);

	/// Destructor
	~Grid();

	/// Execute instructions in this grid
	///
	/// \return
	///	False, if this grid has finished its execution
	bool Execute();

	/// Dump grid formation
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Grid &grid)
	{
		grid.Dump(os);
		return os;
	}

	/// Return grid dimension
	unsigned short getDimension() const { return dimension; }

	/// Return grid size x
	unsigned int getGridSizeX() const { return grid_size_x; }

	/// Return grid size y
	unsigned int getGridSizeY() const { return grid_size_y; }

	/// Return grid size z
	unsigned int getGridSizeZ() const { return grid_size_z; }

	/// Return work group size x
	unsigned int getGroupSizeX() const { return group_size_x; }

	/// Return work group size y
	unsigned int getGroupSizeY() const { return group_size_y; }

	/// Return work group size z
	unsigned int getGroupSizeZ() const { return group_size_z; }

	/// Return the pointer to the component
	Component *getComponent() const { return component; }

	/// Return the kernel segment manager
	SegmentManager *getKernargSegment() const 
	{ 
		return kernarg_segment.get(); 
	}

	/// Return the kernel argument variable by the name. If the name is
	/// not found, return nullptr;
	Variable *getKernelArgument(const std::string &name)
	{
		auto it = kernel_arguments.find(name);
		if (it == kernel_arguments.end())
			return nullptr;
		return it->second.get();
	}
};

}  // namespace HSA

#endif 
