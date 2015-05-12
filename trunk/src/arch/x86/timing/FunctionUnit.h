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

#ifndef X86_ARCH_TIMING_FUNCTION_UNIT_H
#define X86_ARCH_TIMING_FUNCTION_UNIT_H

#include <lib/cpp/IniFile.h>

#include "Uop.h"

namespace x86
{

// Global varable
const int max_function_unit_reservation = 10;

class FunctionUnit
{
public:
	enum Class
	{
		ClassNone = 0,
		ClassIntAdd,
		ClassIntMult,
		ClassIntDiv,
		ClassEffaddr,
		ClassLogic,
		ClassFloatSimple,
		ClassFloatAdd,
		ClassFloatComp,
		ClassFloatMult,
		ClassFloatDiv,
		ClassFloatComplex,
		ClassXmmIntAdd,
		ClassXmmIntMult,
		ClassXmmIntDiv,
		ClassXmmLogic,
		ClassXmmFloatAdd,
		ClassXmmFloatCompare,
		ClassXmmFloatMult,
		ClassXmmFloatDiv,
		ClassXmmFloatConv,
		ClassXmmFloatComplex,
		ClassCount
	};

private:

	// Function unit parameter
	long long cycle_when_free[ClassCount][max_function_unit_reservation];
	long long accesses[ClassCount];
	long long denied[ClassCount];
	long long waiting_time[ClassCount];

	// Structure of function unit reservation pool
	struct ReservationPool
	{
		int count;
		int operation_laterncy;
		int issue_laterncy;
	};

	// static member
	static std::string name[ClassCount];
	static ReservationPool reservation_pool[ClassCount];
	static Class class_table[UInstOpcodeCount];

public:
	/// Read function unit configuration from configuration file
	static void ParseConfiguration(const std::string &section,
			misc::IniFile &config);

	/// Dump configuration
	void DumpConfig(std::ostream &os = std::cout);
};

}

#endif // X86_ARCH_TIMING_FUNCTION_UNIT_H
