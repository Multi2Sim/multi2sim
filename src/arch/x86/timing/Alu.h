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

#ifndef ARCH_X86_TIMING_ALU_H
#define ARCH_X86_TIMING_ALU_H

#include <arch/x86/emulator/Uinst.h>
#include <lib/cpp/IniFile.h>

#include "FunctionalUnit.h"


namespace x86
{

// Forward declarations
class Timing;
class Uop;


/// Class representing the arithmetic-logic unit
class Alu
{
	//
	// Static fields
	//

	// Table indexed by a micro-instruction opcode returning the type
	// of functional unit required by that micro-instruction.
	static const FunctionalUnit::Type type_table[Uinst::OpcodeCount];

	// Configuration of each functional unit, given as three integers:
	//	0 -> Number of instances
	//	1 -> Total operation latency
	//	2 -> Issue latency
	static int configuration[FunctionalUnit::TypeCount][3];




	//
	// Class members
	//

	// Vector of functional units, indexed by a functional unit type
	std::vector<std::unique_ptr<FunctionalUnit>> functional_units;

public:

	//
	// Static functions
	//

	/// Read functional unit configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Dump configuration
	static void DumpConfiguration(std::ostream &os = std::cout);



	
	//
	// Class members
	//

	/// Constructor
	Alu();

	/// Reserve the functional unit required by the uop. The return value is
	/// the functional unit latency, or 0 if it could not be reserved. If
	/// the uop does not require any functional unit, the function returns
	/// a latency of 1 cycle.
	int Reserve(Uop *uop);

	/// Release all functional units
	void ReleaseAll();

	/// Dump report for functional units.
	void DumpReport(std::ostream &os = std::cout) const;
	



	//
	// Getters
	//

	// Get the number of instance based on given type count
	static int getAluCount(int type_count) { return configuration[type_count][0]; }

	// Get the operation latency based on given type count
	static int getAluOperationLatency(int type_count) { return configuration[type_count][1]; }

	// Get the issue latency based on given type count
	static int getAluIssueLatency(int type_count) { return configuration[type_count][2]; }

};

}

#endif

