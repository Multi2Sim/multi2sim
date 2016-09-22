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

#include <arch/kepler/emulator/Grid.h>

#include "Gpu.h"
#include "Timing.h"


namespace Kepler
{

// Static variables
int GPU::num_sms = 14;
int GPU::num_registers = 65536;
int GPU::shared_memory_size = 16384;
long long GPU::max_cycles = 0;

GPU::GPU()
{
	// Create MMU
	mmu = misc::new_unique<mem::Mmu>("Kepler");

	// Create Streaming Multiprocessor
	sms.reserve(num_sms);
	for (int i = 0; i < num_sms; i++)
	{
		// Add new SM to the overall list of SMs
		sms.emplace_back(misc::new_unique<SM>(i, this));

		// Add SMs to the list of available SMs
		SM *sm = sms.back().get();
		InsertInAvailableSMs(sm);
	}
}


SM *GPU::getAvailableSM()
{
	if (available_sms.empty())
		return nullptr;
	else
		return available_sms.front();
}


void GPU::InsertInAvailableSMs(SM *sm)
{
	// Sanity
	assert(!sm->in_available_sms);

	// Insert SM
	sm->in_available_sms = true;
	sm->available_sms_iterator = available_sms.insert(available_sms.end(), sm);
}


void GPU::RemoveFromAvailableSMs(SM *sm)
{
	// Sanity
	assert(sm->in_available_sms);

	// Remove context
	available_sms.erase(sm->available_sms_iterator);
	sm->in_available_sms = false;
	sm->available_sms_iterator = available_sms.end();
}


void GPU::MapGrid(Grid *grid)
{
	// Check that at least one block can be allocated per warp pool
	GetNumBlocksAllowedPerSM(grid->getThreadBlockSize(),
			grid->getNumRegistersPerThread(), grid->getSharedMemorySize());

	// Make sure the number of blocks per SM is non-zero
	if (!num_blocks_allowed_per_sm)
	{
		throw Timing::Error(misc::fmt("block resources cannot be allocated to an"
			"SM.\n\tAn SM in the GPU has a limit in "
			"number of warps, number\n\tof registers, and "
			"amount of shared memory. If the block size\n"
			"\texceeds any of these limits, the grid cannot "
			"be executed.\n"));
	}
}


void GPU::GetNumBlocksAllowedPerSM(int threads_per_block,
		int registers_per_thread, int shared_memory_size_per_block)
{
	// Get maximum number of blocks per SM as limited by the maximum number of
	// warps, given the number of warps per block in the grid
	assert(warp_size > 0);

	int warps_per_block = (threads_per_block + warp_size - 1) / warp_size;
	int max_blocks_limited_by_max_warps = SM::max_warps_per_sm /
			warps_per_block;

	// Get maximum number of blocks per SM as limited by the number of available
	// registers, given the number of registers used per thread
	int registers_per_block = registers_per_thread * threads_per_block;
	int max_blocks_limited_by_num_registers =
			num_registers / registers_per_block;

	/// Get maximum number of blocks per SM as limited by the amount of
	/// available shared memory, given the shared memory used by each blocks in
	/// the grid

	int max_blocks_limited_by_shared_memory;
	if (shared_memory_size_per_block != 0)
		max_blocks_limited_by_shared_memory =
			shared_memory_size / shared_memory_size_per_block;
	else
		max_blocks_limited_by_shared_memory = SM::max_blocks_per_sm;

	// Based on the limits above, calculate the actual limit of blocks per SM
	num_blocks_allowed_per_sm = SM::max_blocks_per_sm;
	num_blocks_allowed_per_sm = std::min(num_blocks_allowed_per_sm,
			max_blocks_limited_by_max_warps);
	num_blocks_allowed_per_sm = std::min(num_blocks_allowed_per_sm,
			max_blocks_limited_by_num_registers);
	num_blocks_allowed_per_sm = std::min(num_blocks_allowed_per_sm,
			max_blocks_limited_by_shared_memory);
}


void GPU::Run()
{
	// Advance on cycle in each SM
	for (auto &sm : sms)
		sm->Run();
}

}
