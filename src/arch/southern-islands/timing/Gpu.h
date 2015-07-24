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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_GPU_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_GPU_H

#include <vector>

#include <lib/cpp/Misc.h>
#include <memory/MMU.h>

#include "ComputeUnit.h"


namespace SI
{

/// Class representing a Southern Islands GPU device.
class Gpu
{

public:

	/// Enumeration depciting the various allocation granularities
	enum RegisterAllocationGranularity
	{
		RegisterAllocationInvalid = 0,  /* For invalid user input */
		RegisterAllocationWavefront,
		RegisterAllocationWorkGroup
	};

	/// String map depciting the various allocation granularities
	static const misc::StringMap register_allocation_granularity_map;

private:

	//
	// Configuration
	//

	// Number of compute units
	static int num_compute_units;
	
	// Register allocation size
	static unsigned register_allocation_size;

	// Number of vector registers per compute unit
	static int num_vector_registers;
	
	// Allocation size of lds memory 
	static int lds_allocation_size; 

	// Size of lds memory
	static int lds_size;
	
	// MMU used by this GPU
	mem::MMU *mmu;




	//
	// Class members
	//

	// Vector of compute units
	std::vector<std::unique_ptr<ComputeUnit>> compute_units;

	// Granularity of the register allocation
	RegisterAllocationGranularity register_allocation_granularity = 
			RegisterAllocationInvalid;

	/// Number of work groups allowed in a wavefront pool
	int work_groups_per_wavefront_pool = 0;

	/// Number of work_groups allowed in a compute unit
	int work_groups_per_compute_unit = 0;
	
public:

	//
	// Static members
	//

	/// Return the number of compute units, as set up by the user
	static int getNumComputeUnits() { return num_compute_units; }




	//
	// Class members
	//

	/// Last cycle when uop completed execution
	long long last_complete_cycle;

	/// Constructor
	Gpu();

	/// Return the compute unit with the given index.
	ComputeUnit *getComputeUnit(int index) const
	{
		assert(misc::inRange(index, 0, compute_units.size() - 1));
		return compute_units[index].get();
	}

	/// return the number of work groups per compute unit
	int getWorkGroupsPerComputeUnit() const 
	{
		return work_groups_per_compute_unit; 
	}

	/// Return the associated MMU
	mem::MMU *getMmu() const { return mmu; }

	/// Map an NDRange to the GPP object
	void MapNDRange(NDRange *ndrange);
	
	// Calculate the number of allowed work groups per wavefront pool
	void CalcGetWorkGroupsPerWavefrontPool(
			int work_items_per_work_group, 
			int registers_per_work_item, 
			int local_memory_per_work_group);

	/// Return an iterator to the first compute unit
	std::vector<std::unique_ptr<ComputeUnit>>::iterator getComputeUnitsBegin()
	{
		return compute_units.begin();
	}

	/// Return a past-the-end iterator to the list of compute units
	std::vector<std::unique_ptr<ComputeUnit>>::iterator getComputeUnitsEnd()
	{
		return compute_units.end();
	}

	/// Advance one cycle in the GPU state
	void Run();
	
	/// Add a compute unit to the list of available compute units
	ComputeUnit *AddComputeUnit(ComputeUnit *compute_unit);
};

}

#endif

