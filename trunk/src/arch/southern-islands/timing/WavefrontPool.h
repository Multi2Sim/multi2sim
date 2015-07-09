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


/// Wavefront pool entry
class WavefrontPoolEntry
{
	// Wavefront pool that it belongs to
	WavefrontPool *wavefront_pool;

	// Wavefront that belongs to this entry
	Wavefront *wavefront;

public:

	/// Constructor
	WavefrontPoolEntry(WavefrontPool *wavefront_pool) :
			wavefront_pool(wavefront_pool)
	{
	}

	/// Return the wavefront pool that it belongs to
	WavefrontPool *getWavefrontPool() const { return wavefront_pool; }

	/// Return the associated wavefront
	Wavefront *getWavefront() const { return wavefront; }

	/// Number of outstanding vector memory accesses
	int vm_cnt = 0;

	/// Number of outstanding export memory accesses
	int exp_cnt = 0;

	/// Number of outstanding LDS, GLDS, or constant memory accesses
	int lgkm_cnt = 0;

	// Flags updated during wavefront execution
	bool ready = false;
	bool ready_next_cycle = false;
	bool wavefront_completed = false;
	bool wait_for_barrier = false;
	bool wavefront_finished = false;
};


/// Class representing the wavefront pool in the compute unit front-end
class WavefrontPool
{
	// Compute unit that it belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Number of instructions
	long long num_instructions = 0;

	// Wavefront pool entries that belong to this pool
	std::vector<std::unique_ptr<WavefrontPoolEntry>> wavefront_pool_entries;
public:

	/// Constructor
	WavefrontPool(ComputeUnit *compute_unit) : compute_unit(compute_unit)
	{
	}

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

