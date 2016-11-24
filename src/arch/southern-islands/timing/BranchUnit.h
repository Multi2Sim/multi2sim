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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_BRANCH_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_BRANCH_UNIT_H

#include <memory>

#include "ExecutionUnit.h"
#include "Uop.h"


namespace SI
{

// Forward declarations
class ComputeUnit;


/// Class representing the branch unit of a compute unit
class BranchUnit : public ExecutionUnit
{
	// Variable number of decoded Uops
	std::deque<std::unique_ptr<Uop>> decode_buffer;

	// Variable number of register read instructions
	std::deque<std::unique_ptr<Uop>> read_buffer;

	// Variable number of execution instructions
	std::deque<std::unique_ptr<Uop>> exec_buffer;

	// Variable number of register instructions
	std::deque<std::unique_ptr<Uop>> write_buffer;

public:

	//
	// Static fields
	//




	/// Maximum number of instructions processed per cycle
	static int width;

	/// Size of the issue buffer in number of instructions
	static int issue_buffer_size;

	/// Latency of the decode stage in number of cycles
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

	/// Size of the write buffer in number of instructions
	static int write_buffer_size;




	//
	// Class members
	//

	/// Constructor
	BranchUnit(ComputeUnit *compute_unit) :
			ExecutionUnit(compute_unit)
	{
	}

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
	
	/// Run the actions occurring in one cycle
	void Run();
	



	//
	// Statistics
	//

	// Number of branch instructions
	long long num_instructions;
	
	/// Return whether there is room in the issue buffer of the branch
	/// unit to absorb a new instruction.
	bool canIssue() const override
	{
		return getIssueBufferOccupancy() < issue_buffer_size;
	}
	
	/// Return whether the given uop is a branch instruction.
	bool isValidUop(Uop *uop) const override;

	/// Issue the given instruction into the branch unit.
	void Issue(std::unique_ptr<Uop> uop) override;

	/// Return the current size of the issue buffer
	unsigned getIssueBufferSize() { return issue_buffer.size(); };
	
	/// Return the current size of the decode buffer
	unsigned getDecodeBufferSize() { return decode_buffer.size(); };
	
	/// Return the current size of the read buffer
	unsigned getReadBufferSize() { return read_buffer.size(); };
	
	/// Return the current size of the exec buffer
	unsigned getExecBufferSize() { return exec_buffer.size(); };
	
	/// Return the current size of the write buffer
	unsigned getWriteBufferSize() { return write_buffer.size(); };
};

}

#endif

