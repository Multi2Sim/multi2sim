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

#include "FetchBuffer.h"
#include "SM.h"

namespace Kepler
{

FetchBuffer::FetchBuffer(int id, SM *sm):
		id(id),
		sm(sm)

	{
		// Initialize the uop buffer
		buffer.resize(sm->max_warps_per_warp_pool * num_instructions_per_entry);

		// Initialize the valid buffer
		valid_buffer.resize(sm->max_warps_per_warp_pool *
				num_instructions_per_entry, false);

		// Initialize branch pending buffer
		branch_pending_buffer.resize(sm->max_warps_per_warp_pool, false);

		// Initialize the last fetched warp index
		last_fetched_warp_index = -1;

		// Initialize the last dispatched warp index
		last_dispatched_warp_index = -1;

	}


void FetchBuffer::Remove(std::vector<std::unique_ptr<Uop>>::iterator it)
{
	assert(it != buffer.end());
	buffer.erase(it);
}


void FetchBuffer::AddToFetchBuffer(int index, std::unique_ptr<Kepler::Uop> uop)
{
	// Set the valid buffer entry
	valid_buffer[index] = true;

	// Set the uop buffer entry
	buffer[index] = std::move(uop);
}


void FetchBuffer::FlushFetchBufferEntrySet(int index)
{
	// Invalid the valid buffer entry
	valid_buffer[index] = false;
}
}
