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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_GPU_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_GPU_H

#include <vector>

#include <lib/cpp/Misc.h>

#include "ComputeUnit.h"


namespace SI
{


/// Class representing a Southern Islands GPU device.
class Gpu
{
	//
	// Configuration
	//

	// Number of compute units
	static int num_compute_units;




	//
	// Class members
	//

	// Vector of compute units
	std::vector<std::unique_ptr<ComputeUnit>> compute_units;

public:

	//
	// Static members
	//

	/// Return the number of compute units, as set up by the user
	static int getNumComputeUnits() { return num_compute_units; }




	//
	// Class members
	//

	/// Constructor
	Gpu();

	/// Return the compute unit with the given index.
	ComputeUnit *getComputeUnit(int index) const
	{
		assert(misc::inRange(index, 0, compute_units.size() - 1));
		return compute_units[index].get();
	}

	/// Return an iterator to the first compute unit
	std::vector<std::unique_ptr<ComputeUnit>>::iterator getComputeUnitsBegin()
	{
		return compute_units.begin();
	}

	/// Return a past-the-end iterator to the list of compute units
	std::vector<std::unique_ptr<ComputeUnit>>::iterator getComputeUnitsEnd()
	{
		return compute_units.end();
	}

	/// Advance one cycle in the GPU state
	void Run();
};

}

#endif

