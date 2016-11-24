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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_EXECUTION_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_EXECUTION_UNIT_H

#include <deque>
#include <memory>

#include "Uop.h"


namespace SI
{

// Forward declarations
class ComputeUnit;


/// Abstract base class representing an execution unit where the front-end can
/// issue instructions. Derived classes are SimdUnit, ScalarUnit, ...
class ExecutionUnit
{
	// Compute unit that it belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Number of instructions issued to this execution unit
	long long num_instructions = 0;

protected:

	// Issue buffer absorbing instructions from the front end
	std::deque<std::unique_ptr<Uop>> issue_buffer;

public:

	/// Constructor
	ExecutionUnit(ComputeUnit *compute_unit) : compute_unit(compute_unit)
	{
	}

	/// Run the actions occurring in one cycle. This is a pure virtual
	/// function that every execution unit must implement.
	virtual void Run() = 0;

	/// Return whether the given uop is accepted by the execution unit,
	/// based on the type of instruction that it contains. This is a pure
	/// virtual function that every execution unit must implement.
	virtual bool isValidUop(Uop *uop) const = 0;

	/// Return whether the execution unit can absorb one more instruction.
	/// This is a pure virtual function that every execution unit must
	/// implement.
	virtual bool canIssue() const = 0;

	/// Issue the given uop into the execution unit. Child classes can
	/// override this function to extend its behavior, but should invoke the
	/// parent class function, too.
	virtual void Issue(std::unique_ptr<Uop> uop);

	/// Return the number of instructions currently present in the issue
	/// buffer.
	int getIssueBufferOccupancy() const { return issue_buffer.size(); }

	/// Return the number of instructions issued into the execution unit.
	long long getNumInstructions() const { return num_instructions; }

	/// Return the compute unit that this execution unit belongs to.
	ComputeUnit *getComputeUnit() const { return compute_unit; }
};

}

#endif

