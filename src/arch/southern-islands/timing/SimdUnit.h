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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_SIMD_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_SIMD_UNIT_H


namespace SI
{

// Forward declarations
class ComputeUnit;


/// Class representing the SIMD unit of a compute unit
class SimdUnit
{
	// Compute unit that it belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Number of instructions
	long long num_instructions = 0;

public:

	/// Constructor
	SimdUnit(ComputeUnit *compute_unit) : compute_unit(compute_unit)
	{
	}

	/// Run the actions occurring in one cycle
	void Run();
};

}

#endif

