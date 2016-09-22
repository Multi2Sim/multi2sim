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

#ifndef ARCH_KEPLER_TIMING_WARPPOOL_H
#define ARCH_KEPLER_TIMING_WARPPOOL_H


namespace Kepler
{

// Forward declarations
class SM;
class WarpPool;
class Warp;
class ThreadBlock;
class Uop;


/// Warp pool entry
class WarpPoolEntry
{
	// ID of entry in the warp pool
	int id_in_warp_pool;

	// Warp pool that it belongs to
	WarpPool * warp_pool;

	// Warp that belongs to this entry
	Warp *warp = nullptr;

	// Uop currently associated with the entry
	Uop *uop = nullptr;

public:

	/// Constructor
	WarpPoolEntry(int id, WarpPool *warp_pool) :
		id_in_warp_pool(id),
		warp_pool(warp_pool)
	{
	}

	/// Clear the internal counters of the warp
	void Clear();


	//
	// Getters
	//

	/// Get the ID of the entry in the warp pool
	int getIdInWarpPool() { return id_in_warp_pool; }

	/// Return the warp pool that it belongs to
	WarpPool *getWarpPool() const { return warp_pool; }

	/// Return the associated warp
	Warp *getWarp() const { return warp; }




	//
	// Setters
	//

	/// Set the associated warp
	void setWarp(Warp *warp) { this->warp = warp; }




	//
	// Counters
	//

	/// Number of outstanding memory access
	int memory_count = 0;




	//
	// Flags updated during warp execution
	//

	/// Valid if warp assigned to entry
	bool valid = false;

	/// Indicates whether the warp is ready
	bool ready = false;

	/// Indicates whether the warp will be ready in the next cycle
	bool ready_next_cycle = false;

	/// Indicates whether is waiting at a barrier
	bool wait_for_barrier = false;

	/// Indicates whether the warp has finished yet
	bool warp_finished = false;

	/// Indicates whether the warp needs to wait for a memory access
	bool mem_wait = false;
};


// Class representing the warp pool in the SM front end
class WarpPool
{
	// Global identifier for the warp pool, assigned in constructor
	int id;

	// Compute unit that it belongs to, assigned in constructor;
	SM *sm;

	// Number of instructions
	long long num_instructions = 0;

	// Number of warps associated with the warp pool
	int num_warps = 0;

	// Warp pool entries that belong to this pool
	std::vector<std::unique_ptr<WarpPoolEntry>> warp_pool_entries;

public:

	/// Constructor
	WarpPool(int id, SM *sm);

	/// Return the identifier for this warp pool
	int getId() const { return id; }

	/// Return number of warps reside in the warp pool
	int getNumWarps() const { return num_warps; }

	/// Map warp to the warp pool
	void MapWarp(Warp *warp);

	/// Unmap warp from the warp pool
	void UnmapWarp(Warp *warp);

	/// Find first available warp pool entry index
	int FindFirstAvailableEntryIndex();

	/// Return an iterator to the first warp pool entry
	/// in warp_pool_entries
	std::vector<std::unique_ptr<WarpPoolEntry>>::iterator begin()
	{
		return warp_pool_entries.begin();
	}

	/// Return a past-the-end iterator to warp_pool_entries
	std::vector<std::unique_ptr<WarpPoolEntry>>::iterator end()
	{
		return warp_pool_entries.end();
	}

	/// Return the associated SM
	SM *getSM() const { return sm; }
};

}

#endif
