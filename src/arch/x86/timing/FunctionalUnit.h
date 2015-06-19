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

#ifndef ARCH_X86_TIMING_FUNCTIONAL_UNIT_H
#define ARCH_X86_TIMING_FUNCTIONAL_UNIT_H

#include <lib/cpp/IniFile.h>

#include "Uop.h"

namespace x86
{

// Forward declaration
class Timing;

class FunctionalUnit
{
public:

	/// Maximum number of reserved functional unit
	static const int MaxFunctionalUnitReservation = 10;

	/// Functional unit type
	enum Type
	{
		TypeNone = 0,
		TypeIntAdd,
		TypeIntMult,
		TypeIntDiv,
		TypeEffaddr,
		TypeLogic,
		TypeFloatSimple,
		TypeFloatAdd,
		TypeFloatComp,
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

	/// Structure of functional unit reservation pool
	struct ReservationPool
	{
		int count;
		int operation_latency;
		int issue_latency;
	};

private:

	//
	// Functional unit parameters
	//

	// Cycle count when functional unit is free
	long long cycle_when_free[TypeCount][MaxFunctionalUnitReservation] = { };

	// Access count of functional unit
	long long accesses[TypeCount] = { };

	// Denied count of functional unit
	long long denied[TypeCount] = { };

	// Waiting time of functional unit
	long long waiting_time[TypeCount] = { };




	//
	// static members
	//

	// The name of the functional unit
	static std::string name[TypeCount];

	// Reservation pool
	static ReservationPool reservation_pool[TypeCount];

	// Functional Unit type table
	static Type type_table[UInstOpcodeCount];

public:

	/// Read functional unit configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Dump configuration
	void DumpConfiguration(std::ostream &os = std::cout);

	/// Reserve the functional unit required by the uop.
	/// The return value is the functional unit latency, or 0 if it could not
	/// be reserved.
	int Reserve(Uop *uop);

	/// Release all functional units
	void ReleaseAll();
};

}

#endif // ARCH_X86_TIMING_FUNCTIONAL_UNIT_H
