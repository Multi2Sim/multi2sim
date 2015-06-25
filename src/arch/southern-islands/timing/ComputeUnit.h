/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_COMPUTE_UNIT_H

#include <memory/Module.h>


namespace SI
{

class ComputeUnit
{
	// Index of the compute unit in the GPU device, initialized in the
	// constructor.
	int index;

public:

	/// Constructor
	ComputeUnit(int index) : index(index)
	{
	}

	/// Advance compute unit state by one cycle
	void Run();

	/// Return the index of this compute unit in the GPU
	int getIndex() const { return index; }

	/// Cache used for vector data
	mem::Module *vector_cache = nullptr;

	/// Cache used for scalar data
	mem::Module *scalar_cache = nullptr;
};

}

#endif

