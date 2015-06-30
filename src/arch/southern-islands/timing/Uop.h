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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H

#include <arch/southern-islands/disassembler/Instruction.h>


namespace SI
{

// Forward declarations
class ComputeUnit;
class Wavefront;


/// Class representing an instruction flowing through the pipelines of the
/// GPU compute units.
class Uop
{
	//
	// Static fields
	//

	// Counter tracking the ID assigned to the last uop created
	static long long id_counter;




	//
	// Class members
	//

	// Unique identifier of the instruction, assigned when created
	long long id;

	// Unique identifier of the instruction in the wavefront that it
	// belongs to. This field is initialized in the constructor.
	long long id_in_wavefront;

	// Unitque identifier of the instruction in the compute unit that it
	// belongs to. This field is initialized in the constructor.
	long long id_in_compute_unit;

	// Associated instruction
	Instruction instruction;

	// Associated wavefront, assigned in constructor
	Wavefront *wavefront;

	// Compute unit that the uop belongs to, assigned in constructor
	ComputeUnit *compute_unit;

public:

	/// Constructor
	Uop(Wavefront *wavefront, ComputeUnit *compute_unit);

	/// Return the unique identifier assigned in sequential order to the
	/// uop when it was created.
	long long getId() const { return id; }

	/// Return a unique sequential identifier of the uop in the wavefront
	/// that it belongs to.
	long long getIdInWavefront() const { return id_in_wavefront; }

	/// Return a unique sequential identifier of the uop in the compute
	/// unit that it belongs to.
	long long getIdInComputeUnit() const { return id_in_compute_unit; }

	/// Return the instruction associated with the uop
	Instruction *getInstruction() { return &instruction; }

	/// Return the associated wavefront
	Wavefront *getWavefront() { return wavefront; }

	/// Return the associated compute unit
	ComputeUnit *getComputeUnit() { return compute_unit; }

	/// Cycle in which the uop is first ready after fetch
	long long fetch_ready = 0;

	/// Cycle in which the uop is first ready after being issued to its
	/// corresponding execution unit
	long long issue_ready = 0;
};

}

#endif

