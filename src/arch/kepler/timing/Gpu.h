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

#ifndef ARCH_KEPLER_TIMING_GPU_H
#define ARCH_KEPLER_TIMING_GPU_H

#include<vector>

#include <lib/cpp/Misc.h>
#include <memory/Mmu.h>

#include "SM.h"


namespace Kepler
{

class Grid;

/// Class representing a Kepler GPU device.
class GPU
{

public:

private:

	//
	// Class members
	//

	// MMU used by this GPU
	std::unique_ptr<mem::Mmu> mmu;

	// Vector of streaming multiprocessor
	std::vector<std::unique_ptr<SM>> sms;

	// List of available streaming multiprocessor
	std::list<SM *> available_sms;

	/// Number of blocks allowed in a warp pool
	int num_blocks_allowed_per_warp_pool = 0;

	/// Number of blocks allowed in a streaming multiprocessor
	int num_blocks_allowed_per_sm = 0;

public:

	//
	// Static members
	//


	// Maximum number of cycles to simulate
	static long long max_cycles;

	// Number of streaming multiprocessors
	static int num_sms;




	//
	// Configuration
	//

	// Register allocation size
	static unsigned register_allocation_size;

	// Number of registers per SM
	static int num_registers;

	// Size of shared memory in Bytes
	static int shared_memory_size;




	//
	// Class members
	//

	/// Last cycle when uop completed execution
	long long last_complete_cycle = 0 ;

	/// Constructor
	GPU();

	/// Return the iterator of an available streaming multiproessor. If no sm
	/// are available, a nullptr is returned.
	SM *getAvailableSM();

	/// Insert the given SM in the list of available units. The SM must no be
	/// currently present in the list.
	void InsertInAvailableSMs (SM * sm);

	/// Remove the SM from the list of available units. The SM must be present
	/// in the list.
	void RemoveFromAvailableSMs (SM *sm);

	/// Return the SM with a given index
	SM *getSM(int index) const
	{
		assert(misc::inRange(index, 0, sms.size()-1));
		return sms[index].get();
	}

	/// Return the number of blocks allower per sm
	int getNumBlocksAllowedPerSM() const { return num_blocks_allowed_per_sm; }

	/// Return the associated MMU
	mem::Mmu *getMmu() const { return mmu.get(); }

	/// Map a grid to the GPU object
	void MapGrid(Grid *grid);

	/// Calculate the number of allowed blocks per SM
	void GetNumBlocksAllowedPerSM( int threads_per_block,
					int registers_per_thread, int shared_memory_size_per_block);

	/// Return an iterator to the first SM
	std::vector<std::unique_ptr<SM>>::iterator getSMsBegin()
	{
		return sms.begin();
	}

	/// Return a past-the-end iterator to the vector of SMs
	std::vector<std::unique_ptr<SM>>::iterator getSMsEnd()
	{
		return sms.end();
	}

	/// Return an iterator to the first available SM
	std::list<SM *>::iterator getAvailableSMsBegin()
	{
		return available_sms.begin();
	}

	/// Return a past-the-end iterator to the list of SMs
	std::list<SM *>::iterator getAvailabelSMsEnd()
	{
		return available_sms.end();
	}

	/// Return the number of available SMs
	unsigned getNumAvailableSMs()
	{
		return available_sms.size();
	}

	/// Advance on cycle in the GPU state
	void Run();

	/// Add an SM to the list of available SMs
	SM *AddSM(SM *sm);
};

}

#endif
