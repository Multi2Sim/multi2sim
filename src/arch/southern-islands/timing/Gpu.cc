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

#include <arch/southern-islands/emulator/Emulator.h>
#include <arch/southern-islands/emulator/NDRange.h>

#include "Gpu.h"
#include "Timing.h"


namespace SI
{

// Static variables
int Gpu::num_compute_units = 32;
unsigned Gpu::register_allocation_size = 32;
int Gpu::num_scalar_registers = 2048;
int Gpu::num_vector_registers = 65536;
int Gpu::lds_allocation_size = 64; 
int Gpu::lds_size = 65536;
long long Gpu::max_cycles = 0;

// String map of the argument's access type                                      
const misc::StringMap Gpu::register_allocation_granularity_map =                                
{                                                                                
	{ "Wavefront", RegisterAllocationWavefront },
	{ "WorkGroup", RegisterAllocationWorkGroup }
}; 

Gpu::Gpu()
{
	// Create MMU
	mmu = misc::new_unique<mem::Mmu>("Southern Islands");

	// Create compute units
	compute_units.reserve(num_compute_units);
	for (int i = 0; i < num_compute_units; i++)
	{
		// Add new compute unit to the overall list of compute units
		compute_units.emplace_back(misc::new_unique<ComputeUnit>(i, this));
		
		// Add compute units to the list of available compute units
		ComputeUnit *compute_unit = compute_units.back().get();
		InsertInAvailableComputeUnits(compute_unit);
	}
}


ComputeUnit *Gpu::getAvailableComputeUnit()
{
	if (available_compute_units.empty())
		return nullptr;
	else
		return available_compute_units.front();
}


void Gpu::InsertInAvailableComputeUnits(ComputeUnit *compute_unit)
{
	// Sanity
	assert(!compute_unit->in_available_compute_units);
	
	// Insert compute_unit
	compute_unit->in_available_compute_units = true;
	compute_unit->available_compute_units_iterator = 
			available_compute_units.insert(
			available_compute_units.end(), 
			compute_unit);
}


void Gpu::RemoveFromAvailableComputeUnits(ComputeUnit *compute_unit)
{
	// Sanity
	assert(compute_unit->in_available_compute_units);
	
	// Remove context
	available_compute_units.erase(
			compute_unit->available_compute_units_iterator);
	compute_unit->in_available_compute_units = false;
	compute_unit->available_compute_units_iterator = 
			available_compute_units.end();
}


void Gpu::MapNDRange(NDRange *ndrange)
{
	// Check that at least one work-group can be allocated per 
	// wavefront pool
	Gpu::CalcGetWorkGroupsPerWavefrontPool(ndrange->getLocalSize1D(),
			ndrange->getNumVgprUsed(),
			ndrange->getLocalMemTop());

	// Make sure the number of work groups per wavefront pool is non-zero
	if (!work_groups_per_wavefront_pool)
	{
		throw Timing::Error(misc::fmt("work-group resources cannot be allocated to a compute "
			"unit.\n\tA compute unit in the GPU has a limit in "
			"number of wavefronts, number\n\tof registers, and "
			"amount of local memory. If the work-group size\n"
			"\texceeds any of these limits, the ND-Range cannot "
			"be executed.\n"));
	}

	// Calculate limit of work groups per compute unit
	work_groups_per_compute_unit = work_groups_per_wavefront_pool *
			ComputeUnit::num_wavefront_pools;
	assert(work_groups_per_wavefront_pool <=
			ComputeUnit::max_work_groups_per_wavefront_pool);
	// Debug info
	Emulator::scheduler_debug << misc::fmt("NDRange %d calculations:\n"
			"\t%d work group per wavefront pool\n"
			"\t%d work group slot per compute unit\n",
			ndrange->getId(),
			work_groups_per_wavefront_pool,
			work_groups_per_compute_unit);
	
}


void Gpu::CalcGetWorkGroupsPerWavefrontPool(int work_items_per_work_group, 
		int registers_per_work_item, int local_memory_per_work_group)
{
	// Get maximum number of work-groups per SIMD as limited by the 
	// maximum number of wavefronts, given the number of wavefronts per 
	// work-group in the NDRange
	assert(WorkGroup::WavefrontSize > 0);
	int wavefronts_per_work_group = (work_items_per_work_group + 
			WorkGroup::WavefrontSize - 1) / 
			WorkGroup::WavefrontSize;
	int max_work_groups_limited_by_max_wavefronts = 
			ComputeUnit::max_wavefronts_per_wavefront_pool /
			wavefronts_per_work_group;

	// Get maximum number of work-groups per SIMD as limited by the number 
	// of available registers, given the number of registers used per 
	// work-item.
	int registers_per_work_group;
	if (register_allocation_granularity == RegisterAllocationWavefront)
	{
		registers_per_work_group = misc::RoundUp(
				registers_per_work_item *
				WorkGroup::WavefrontSize, 
				register_allocation_size) * 
				wavefronts_per_work_group;
	}
	else
	{
		registers_per_work_group = misc::RoundUp(
				registers_per_work_item *
				work_items_per_work_group, 
				register_allocation_size);
	}
	
	// FIXME need to account for scalar registers
	int max_work_groups_limited_by_num_registers = 
			registers_per_work_group ?
			num_vector_registers / registers_per_work_group :
			ComputeUnit::max_work_groups_per_wavefront_pool;

	// Get maximum number of work-groups per SIMD as limited by the 
	// amount of available local memory, given the local memory used 
	// by each work-group in the NDRange
	local_memory_per_work_group = misc::RoundUp(local_memory_per_work_group, 
			lds_allocation_size);
	int max_work_groups_limited_by_local_memory = 
			local_memory_per_work_group ?
			lds_size / local_memory_per_work_group :
			ComputeUnit::max_work_groups_per_wavefront_pool;

	// Based on the limits above, calculate the actual limit of work-groups 
	// per SIMD.
	work_groups_per_wavefront_pool = 
			ComputeUnit::max_work_groups_per_wavefront_pool;
	work_groups_per_wavefront_pool = std::min(work_groups_per_wavefront_pool,
			max_work_groups_limited_by_max_wavefronts);
	work_groups_per_wavefront_pool= std::min(work_groups_per_wavefront_pool, 
			max_work_groups_limited_by_num_registers);
	work_groups_per_wavefront_pool = std::min(work_groups_per_wavefront_pool, 
			max_work_groups_limited_by_local_memory);
}


void Gpu::Run()
{
	// Advance one cycle in each compute unit
	for (auto &compute_unit : compute_units)
		compute_unit->Run();
}

}

