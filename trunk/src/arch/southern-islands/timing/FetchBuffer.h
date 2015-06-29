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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_FETCH_BUFFER_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_FETCH_BUFFER_H

#include <memory>
#include <list>

#include "Uop.h"


namespace SI
{

// Forward declarations
class ComputeUnit;


/// Class representing a fetch buffer in the compute unit front-end
class FetchBuffer
{
	// Compute unit that it belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Buffer of instructions
	std::list<std::shared_ptr<Uop>> buffer;

public:
	
	//
	// Static fields
	//

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




	//
	// Class members
	//

	/// Constructor
	FetchBuffer(ComputeUnit *compute_unit) : compute_unit(compute_unit)
	{
	}

	/// Return an iterator to the first uop in the fetch buffer
	std::list<std::shared_ptr<Uop>>::iterator begin()
	{
		return buffer.begin();
	}

	/// Return a past-the-end iterator to the fetch buffer
	std::list<std::shared_ptr<Uop>>::iterator end()
	{
		return buffer.end();
	}

	/// Remove the uop pointed to by the given iterator, and return a
	/// shared pointer reference to the removed entry.
	std::shared_ptr<Uop> Remove(std::list<std::shared_ptr<Uop>>::iterator it);
};

}

#endif

