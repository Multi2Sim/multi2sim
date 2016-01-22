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

#ifndef ARCH_X86_TIMING_FUNCTIONAL_UNIT_H
#define ARCH_X86_TIMING_FUNCTIONAL_UNIT_H

#include <lib/cpp/String.h>


namespace x86
{

// Forward declarations
class Uop;


/// Class representing a functional unit within the ALU. One instance of this
/// class represents all actual hardware instances of functional units of the
/// same kind.
class FunctionalUnit
{
public:

	/// Maximum number of instances of a functional unit of the same kind.
	/// This determines the maximum number that can be configured by the
	/// user.
	static const int MaxInstances = 10;

	/// Functional unit type
	enum Type
	{
		TypeNone = 0,
		TypeIntAdd,
		TypeIntMult,
		TypeIntDiv,
		TypeEffAddr,
		TypeLogic,
		TypeFloatSimple,
		TypeFloatAdd,
		TypeFloatCompare,
		TypeFloatMult,
		TypeFloatDiv,
		TypeFloatComplex,
		TypeXmmIntAdd,
		TypeXmmIntMult,
		TypeXmmIntDiv,
		TypeXmmLogic,
		TypeXmmFloatAdd,
		TypeXmmFloatCompare,
		TypeXmmFloatMult,
		TypeXmmFloatDiv,
		TypeXmmFloatConv,
		TypeXmmFloatComplex,
		TypeCount
	};

	/// String map for values of type FunctionalUnit::Type
	static const misc::StringMap type_map;

private:

	// Function unit type, initialized in constructor
	Type type;

	// Functional unit name, initialized in constructor
	std::string name;

	// Number of instances of this functional unit, initialized in
	// constructor
	int num_instances;

	// Number of cycles since an operation begins until it completes,
	// initialized in constructor
	int operation_latency;

	// Number of cycles since an operation begins until the next operation
	// can begin, initialized in constructor
	int issue_latency;

	// Cycle when each instance of the functional unit is free
	long long cycle_free[MaxInstances] = {};



	//
	// Statistics
	//

	// Number of accesses to this functional unit
	long long num_accesses = 0;

	// Number of times that this functional unit was denied
	long long num_denied_accesses = 0;

	// Waiting time of functional unit
	long long waiting_time = 0;

public:
	
	//
	// Class members
	//

	/// Constructor
	FunctionalUnit(Type type,
			const std::string &name,
			int num_instances,
			int operation_latency,
			int issue_latency) :
			type(type),
			name(name),
			num_instances(num_instances),
			operation_latency(operation_latency),
			issue_latency(issue_latency)
	{
	}

	/// Return the functional unit name.
	const std::string &getName() const { return name; }

	/// Reserve an instance of this functional unit for the given uop. The
	/// uop must belong to this functional unit. The function returns the
	/// functional unit latency, or 0 if no free instance was found.
	int Reserve(Uop *uop);

	/// Release all instances of this functional unit
	void Release();



	//
	// Statistics
	//

	/// Return the total number of a ccesses
	long long getNumAccesses() const { return num_accesses; }

	/// Return the total number of denied accesses
	long long getNumDeniedAccesses() const { return num_denied_accesses; }

	/// Return the total waiting time
	long long getWaitingTime() const { return waiting_time; }
};

}

#endif

