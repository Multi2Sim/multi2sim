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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_WAVEFRONT_POOL_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_WAVEFRONT_POOL_H


namespace SI
{

// Forward declarations
class ComputeUnit;
class WavefrontPool;
class Wavefront;
class WorkGroup;
class Uop;


/// Wavefront pool entry
class WavefrontPoolEntry
{
	// ID of entry in the wavefront pool
	int id_in_wavefront_pool;

	// Wavefront pool that it belongs to
	WavefrontPool *wavefront_pool;

	// Wavefront that belongs to this entry
	Wavefront *wavefront = nullptr;
	
	// Uop currently associated with the entry
	Uop *uop = nullptr;
	
public:

	/// Constructor
	WavefrontPoolEntry(int id, WavefrontPool *wavefront_pool) :
		id_in_wavefront_pool(id), 
		wavefront_pool(wavefront_pool)
	{
	}
	
	/// Clear the internal counters of the wavefront pool entry
	void Clear();


	//
	// Getters
	//

	/// Get the ID of the entry in the wavefront pool
	int getIdInWavefrontPool() { return id_in_wavefront_pool; }

	/// Return the wavefront pool that it belongs to
	WavefrontPool *getWavefrontPool() const { return wavefront_pool; }

	/// Return the associated wavefront
	Wavefront *getWavefront() const { return wavefront; }



	//
	// Setters
	//

	/// Set the associated wavefront
	void setWavefront(Wavefront *wavefront) { this->wavefront = wavefront; }

	


	//
	// Counters
	// 

	/// Number of outstanding vector memory accesses
	int vm_cnt = 0;

	/// Number of outstanding export memory accesses
	int exp_cnt = 0;

	/// Number of outstanding LDS, GLDS, or constant memory accesses
	int lgkm_cnt = 0;



	
	// 
	// Flags updated during wavefront execution
	//

	/// Valid if wavefront assigned to entry
	bool valid = false; 
	
	/// Indicates whether the wavefront is ready now
	bool ready = false;
	
	/// Indicates whether the wavefront will be ready in the next cycle
	bool ready_next_cycle = false;
	
	/// Indicates whether the wavefront is waiting at a barrier
	bool wait_for_barrier = false;
	
	/// Indicates whether the wavefront has finished yet
	bool wavefront_finished = false;

	/// Indicates whether the wavefront needs to wait for a memory access
	bool mem_wait = false;
};


/// Class representing the wavefront pool in the compute unit front-end
class WavefrontPool
{
	// Global identifier for the wavefront pool, assigned in constructor
	int id;

	// Compute unit that it belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Number of instructions
	long long num_instructions = 0;

	// Number of wavefronts associated with the wavefront pool
	int num_wavefronts = 0;

	// Wavefront pool entries that belong to this pool
	std::vector<std::unique_ptr<WavefrontPoolEntry>> wavefront_pool_entries;

public:

	/// Constructor
	WavefrontPool(int id, ComputeUnit *compute_unit);

	/// Return the identifier for this wavefront pool
	int getId() const { return id; }

	/// Map wavefronts to the wavefront pool
	void MapWavefronts(WorkGroup *work_group);
	
	/// Unmap wavefronts from the wavefront pool
	void UnmapWavefronts(WorkGroup *work_group);

	/// Return an iterator to the first wavefront pool entry
	/// in wavefront_pool_entries
	std::vector<std::unique_ptr<WavefrontPoolEntry>>::iterator begin()
	{
		return wavefront_pool_entries.begin();
	}

	/// Return a past-the-end iterator to wavefront_pool_entries
	std::vector<std::unique_ptr<WavefrontPoolEntry>>::iterator end()
	{
		return wavefront_pool_entries.end();
	}

	/// Return the associated compute unit
	ComputeUnit *getComputeUnit() const { return compute_unit; }
};

}

#endif

