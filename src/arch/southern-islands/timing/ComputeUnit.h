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
class Timing;
class WorkGroup;
class Gpu;



/// Class representing one compute unit in the GPU device.
class ComputeUnit
{
	// Fetch an instruction from the given wavefront pool
	void Fetch(FetchBuffer *fetch_buffer, WavefrontPool *wavefront_pool);

	// Issue an instruction from the given fetch buffer into the
	// appropriate execution unit.
	void Issue(FetchBuffer *fetch_buffer);

	// Issue a set of instructions from the given fetch buffer into the
	// given execution unit.
	void IssueToExecutionUnit(FetchBuffer *fetch_buffer,
			ExecutionUnit *execution_unit);

	// Update the visualization states for non-issued instructions
	void UpdateFetchVisualization(FetchBuffer *fetch_buffer);

	// Associated timing simulator, saved for performance
	Timing *timing = nullptr;

	// Associated GPU
	Gpu *gpu;

	// Index of the compute unit in the GPU device, initialized in the
	// constructor.
	int index;

	// List of work-groups currently mapped to the compute unit
	std::vector<WorkGroup *> work_groups;

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

	// Associated LDS module
	std::unique_ptr<mem::Module> lds_module;

	// Counter of identifiers assigned to uops in this compute unit
	long long uop_id_counter = 0;

public:

	//
	// Static fields
	//
	
	/// Number of wavefront pools per compute unit, configured by the user
	static int num_wavefront_pools;

	/// Fetch latency in cycles
	static int fetch_latency;

	/// Number of instructions fetched per cycle
	static int fetch_width;

	/// Maximum capacity of fetch buffer in number of instructions
	static int fetch_buffer_size;

	/// Issue latency in cycles
	static int issue_latency;

	/// Maximum capacity of issue buffer in number of instructions
	static int issue_width;

	/// Maximum number of instructions issued in each cycle of each type
	/// (vector, scalar, branch, ...)
	static int max_instructions_issued_per_type;

	/// The maximum number of work_groups in a wavefront pool
	static int max_work_groups_per_wavefront_pool;

	/// The maximum number of wavefronts in a wavefront pool
	static int max_wavefronts_per_wavefront_pool; 

	// The total size of the Lds module
	static int lds_size;
	
	// The allocation size of the Lds module
	static int lds_alloc_size;
	
	// The latency of the Lds module
	static int lds_latency;

	// The block size for the Lds memory module
	static int lds_block_size;
	
	// The number of ports of the Lds module
	static int lds_num_ports; 


	//
	// Class members
	//

	/// Constructor
	ComputeUnit(int index, Gpu *gpu);

	/// Advance compute unit state by one cycle
	void Run();

	/// Return the index of this compute unit in the GPU
	int getIndex() const { return index; }

	/// Return a new unique sequential identifier for the next uop in the
	/// compute unit.
	long long getUopId() { return ++uop_id_counter; }

	/// Return a pointer to the associated GPU
	Gpu *getGpu() { return gpu; }

	/// Return the associated timing simulator
	Timing *getTiming() const { return timing; }
	
	/// Map a work group to the compute unit
	void MapWorkGroup(WorkGroup *work_group);

	/// Unmap a work group from the compute unit
	void UnmapWorkGroup(WorkGroup *work_group);

	/// Add a work group pointer to the work_groups list
	void AddWorkGroup(WorkGroup *work_group);

	/// Remove a work group pointer from the work_groups list
	void RemoveWorkGroup(WorkGroup *work_group);

	/// Return the associated LDS module
	mem::Module *getLdsModule() const { return lds_module.get(); }

	/// Cache used for vector data
	mem::Module *vector_cache = nullptr;

	/// Cache used for scalar data
	mem::Module *scalar_cache = nullptr;

	/// Iterator of the compute unit location in the available compute 
	/// units list
	std::list<ComputeUnit *>::iterator available_compute_units_iterator;

	/// Flag to indicate if the compute unit is currently available or not
	bool in_available_compute_units = false;




	//
	// Statistics
	//

	// Number of total instructions
	long long num_total_instructions = 0;

	// Number of issued branch instructions
	long long num_branch_instructions = 0;

	// Number of issued scalar memory instructions
	long long num_scalar_memory_instructions = 0;

	// Number of issued scalar ALU instructions
	long long num_scalar_alu_instructions = 0;

	// Number of issued SIMD instructions
	long long num_simd_instructions = 0;

	// Number of issued vector memory instructions
	long long num_vector_memory_instructions = 0;

	// Number of issued LDS instructions
	long long num_lds_instructions = 0;

	// Number of scalar registers being read from
	long long num_sreg_reads = 0;
	
	// Number of scalar registers being written to
	long long num_sreg_writes = 0;
	
	// Number of vector registers being read from
	long long num_vreg_reads = 0;
	
	// Number of vectorr registers being written to
	long long num_vreg_writes = 0;

	// Number of total mapped work groups for the compute unit
	long long num_mapped_work_groups = 0;
};

}

#endif

