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

#include "ExecutionUnit.h"

namespace SI
{

// Forward declarations
class ComputeUnit;


/// Class representing the SIMD unit of a compute unit
class SimdUnit : public ExecutionUnit
{
	// Variable number of decoded Uops
	std::deque<std::unique_ptr<Uop>> decode_buffer;

	// Variable number of execution instructions
	std::deque<std::unique_ptr<Uop>> exec_buffer;

public:
	//
	// Static fields
	//

	/// Maximum number of instructions processed per cycle
	static int width;

	/// Number of lanes per SIMD.  This must divide the wavefront size
	/// (64) evenly.
	static int num_simd_lanes;

	/// Size of the buffer holding issued instructions
	static int issue_buffer_size;

	/// Latency of the decode stage in number of cycles
	static int decode_latency;

	/// Size of the buffer holding decoded instructions
	static int decode_buffer_size;

	/// Number of cycles it takes to read operands from the register
	/// files, execute the SIMD ALU operation, and write the results
	/// out to the register file for a single subwavefront.  It makes
	/// sense to combine all three stages since the wavefront is pipelined
	/// across all of them and can therefore be in different stages
	/// at the same time
	static int read_exec_write_latency;

	/// Size of the execution buffer in instructions
	static int exec_buffer_size;

	/// Size of the buffer holding instructions that have began the
	/// read-exec-write stages.
	static int read_exec_write_buffer_size;

	// Statistics
	long long num_instructions;




	//
	// Class members
	//


	/// Constructor
	SimdUnit(ComputeUnit *compute_unit) : ExecutionUnit(compute_unit)
	{
	}

	/// Run the actions occurring in one cycle
	void Run();

	/// Return whether there is room in the issue buffer of the SIMD
	/// unit to absorb a new instruction.
	bool canIssue() const override
	{
		return getIssueBufferOccupancy() < issue_buffer_size;
	}

	/// Return whether the given uop is a SIMD instruction.
	bool isValidUop(Uop *uop) const override;

	/// Issue the given instruction into the SIMD unit.
	void Issue(std::unique_ptr<Uop> uop) override;

	/// Complete the instruction
	void Complete();

	/// Execute stage of the execution pipeline
	void Execute();

	/// Decode stage of the execution pipeline
	void Decode();

};

}

#endif

