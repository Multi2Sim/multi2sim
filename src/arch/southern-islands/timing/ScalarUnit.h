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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_SCALAR_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_SCALAR_UNIT_H

#include "ExecutionUnit.h"


namespace SI
{

// Forward declarations
class ComputeUnit;


/// Class representing the scalar unit of a compute unit
class ScalarUnit : public ExecutionUnit
{
	// Variable number of decoded Uops
	std::deque<std::unique_ptr<Uop>> decode_buffer;

	// Variable number of register read instructions
	std::deque<std::unique_ptr<Uop>> read_buffer;

	// Variable number of execution instructions
	std::deque<std::unique_ptr<Uop>> exec_buffer;

	// Variable number of register instructions
	std::deque<std::unique_ptr<Uop>> write_buffer;

	// Variable number of pending memory accesses
	std::deque<std::unique_ptr<Uop>> inflight_buffer;

public:
	//
	// Static fields
	//

	/// Maximum number of instructions processed per cycle
	static int width;

	/// Size of the issue buffer in number of instructions
	static int issue_buffer_size;

	/// Decode latency in number of cycles
	static int decode_latency;

	/// Size of the decode buffer in number of instructions
	static int decode_buffer_size;

	/// Latency of the read stage in number of cycles
	static int read_latency;

	/// Size of the read buffer in number of instructions
	static int read_buffer_size;

	/// Latency of the execution stage in number of cycles
	static int exec_latency;

	/// Size of the execution buffer in number of instructions
	static int exec_buffer_size;

	/// Latency of the write stage in number of cycles
	static int write_latency;

	/// Size of the write buffer in number of cycles
	static int write_buffer_size;




	//
	// Class members
	//

	/// Constructor
	ScalarUnit(ComputeUnit *compute_unit) :
			ExecutionUnit(compute_unit)
	{
	}

	/// Run the actions occurring in one cycle
	void Run();
	
	/// Return whether there is room in the issue buffer of the scalar
	/// unit to absorb a new instruction.
	bool canIssue() const override
	{
		return getIssueBufferOccupancy() < issue_buffer_size;
	}

	/// Return whether the given uop is a scalar instruction.
	bool isValidUop(Uop *uop) const override;
	
	/// Issue the given instruction into the scalar unit.
	void Issue(std::unique_ptr<Uop> uop) override;

	/// Complete the instruction
	void Complete();

	/// Write stage of the execution pipeline.
	void Write();

	/// Execute stage of the execution pipeline.
	void Execute();

	/// Read stage of the execution pipeline.
	void Read();

	/// Decode stage of the execution pipeline.
	void Decode();

	// Statistics
	long long num_instructions;

	/// Remove the uop pointed to by the given iterator, and return a
	/// shared pointer reference to the removed entry.
	std::shared_ptr<Uop> Remove(std::deque<std::shared_ptr<Uop>>::iterator it);
};

}

#endif

