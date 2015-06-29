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

#include <arch/southern-islands/disassembler/Inst.h>


namespace SI
{

// Forward declarations
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

	// Associated instruction
	Inst instruction;

	// Associated wavefront, assigned in constructor
	Wavefront *wavefront;

public:

	/// Constructor
	Uop(Wavefront *wavefront);

	/// Return the unique identifier assigned in sequential order to the
	/// uop when it was created.
	long long getId() const { return id; }

	/// Return the instruction associated with the uop
	Inst *getInstruction() { return &instruction; }

	/// Return the associated wavefront
	Wavefront *getWavefront() { return wavefront; }

	/// Cycle in which the uop is first ready after fetch
	long long fetch_ready = 0;
};

}

#endif

