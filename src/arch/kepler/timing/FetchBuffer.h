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

#ifndef ARCH_KEPLER_TIMING_FETCH_BUFFER_H
#define ARCH_KEPLER_TIMING_FETCH_BUFFER_H

#include<memory>
#include<list>

#include "Uop.h"


namespace Kepler
{

// Forward declarations
class SM;


/// Class representing a fetch buffer in the sm
class FetchBuffer
{
	// Global fetch buffer identifier, assigned in constructor
	int id;

	// SM that it belongs to, assigned in constructor
	SM *sm;

	// Buffer of instructions
	std::vector<std::unique_ptr<Uop>> buffer;

	// The Fetch buffer entry which indicates whether the entry has a valid
	// instructions. The size of this buffer is max_warp_per_warp * 2 because
	// two instruction entries per warp
	std::vector<bool> valid_buffer;

	// Branch pending bit. This bit indicates whether there is a branch
	// instruction in flight
	std::vector<bool> branch_pending_buffer;

	// Position of last fetched warp in the fetch buffer
	int last_fetched_warp_index;

	// Position of last dispatched warp in the fetch buffer
	int last_dispatched_warp_index;

	// Flag indicates whether the buffer has space to fetch. 0 means has space.
	// 1 means there are still fetched instruction are not dispatched
	bool valid;

public:
	//
	// Static fields
	//

	/// Number of instructions per entry
	static const int num_instructions_per_entry = 2;

	/// Constructor
	FetchBuffer(int id, SM *sm);

	/// Return the number of uops in the fetch buffer
	int getSize() { return buffer.size(); }

	/// Return the identifier for this fetch buffer
	int getId() const { return id; }

	/// Return the id of last fetched warp index in fetch buffer
	int getLastFetchedWarpIndex() const { return last_fetched_warp_index; }

	/// Return the id of last dispatched warp index in fetch buffer
	int getLastDispatchedWarpIndex() const
	{
		return last_dispatched_warp_index;
	}

	/// Return the
	bool getBranchPendingBit(int index) const
	{
		return branch_pending_buffer[index];
	}

	/// Check if the given slot in the entry is valid(two slots per entry)
	bool IsValidEntry(int index) const
	{
		return (valid_buffer[index]);
	}

	/// Return an iterator to the first uop in the fetch buffer
	std::vector<std::unique_ptr<Uop>>::iterator begin()
	{
		return buffer.begin();
	}

	/// Return a past-the-end iterator to the fetch buffer
	std::vector<std::unique_ptr<Uop>>::iterator end()
	{
		return buffer.end();
	}

	/// Remove the uop pointed to by the given iterator
	void Remove(std::vector<std::unique_ptr<Uop>>::iterator it);

	/// Add new uops to fetch buffer and update flags and other buffers two
	/// new uops per fetch
	void AddToFetchBuffer(int index, std::unique_ptr<Kepler::Uop> uop);

	/// Remove the uop from fetch buffer and update flags and other buffers by
	/// the given entry index
	void FlushFetchBufferEntrySet(int entry_index);

	void setLastFatchedWarpIndex(int index)
	{
		last_fetched_warp_index = index;
 	}

	void setLastDispatchedWarpIndex(int index)
	{
		last_dispatched_warp_index = index;
	}

	void setBranchPendingBit(int index, bool value)
	{
		branch_pending_buffer[index] = value;
	}
};

}

#endif
