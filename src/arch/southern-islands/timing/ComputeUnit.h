/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H

#include <list>

#include <memory/Module.h>

#include "BranchUnit.h"
#include "FetchBuffer.h"
#include "LdsUnit.h"
#include "SimdUnit.h"
#include "ScalarUnit.h"
#include "VectorMemoryUnit.h"
#include "WavefrontPool.h"


namespace SI
{

// Forward declarations
class WorkGroup;


/// Class representing one compute unit in the GPU device.
class ComputeUnit
{
	//
	// Static fields
	//
	
	// Number of wavefront pools per compute unit, configured by the user
	static int num_wavefront_pools;




	//
	// Class members
	//

	// Index of the compute unit in the GPU device, initialized in the
	// constructor.
	int index;

	// List of work-groups currently mapped to the compute unit
	std::list<WorkGroup *> work_groups;

	// Variable number of wavefront pools
	std::vector<std::unique_ptr<WavefrontPool>> wavefront_pools;

	// Variable number of fetch buffers
	std::vector<std::unique_ptr<FetchBuffer>> fetch_buffers;

	// Variable number of SIMD units
	std::vector<std::unique_ptr<SimdUnit>> simd_units;

	// One instance of the scalar unit
	ScalarUnit scalar_unit;

	// One instance of the branch unit
	BranchUnit branch_unit;

	// One instance of the Local Data Share (LDS) unit
	LdsUnit lds_unit;

	// One instance of the vector memory unit
	VectorMemoryUnit vector_memory_unit;

public:

	/// Constructor
	ComputeUnit(int index);

	/// Advance compute unit state by one cycle
	void Run();

	/// Return the index of this compute unit in the GPU
	int getIndex() const { return index; }

	/// Cache used for vector data
	mem::Module *vector_cache = nullptr;

	/// Cache used for scalar data
	mem::Module *scalar_cache = nullptr;
};

}

#endif

