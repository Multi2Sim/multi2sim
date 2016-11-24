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

#include "Alu.h"
#include "Timing.h"

namespace x86
{

int Alu::configuration[FunctionalUnit::TypeCount][3] =
{
	{ 0, 0, 0 },  // Unused

	{ 3, 1, 1 },  // IntAdd
	{ 1, 3, 1 },  // IntMult
	{ 1, 14, 11 },  // IntDiv

	{ 3, 2, 2 },  // EffAddr
	{ 3, 1, 1 },  // Logic

	{ 1, 2, 1 },  // FloatSimple
	{ 1, 3, 1 },  // FloatAdd
	{ 1, 3, 1 },  // FloatCompare
	{ 1, 5, 1 },  // FloatMult
	{ 1, 12, 5 },  // FloatDiv
	{ 1, 22, 14 },  // FloatComplex

	{ 1, 1, 1 },  // XmmIntAdd
	{ 1, 3, 1 },  // XmmIntMult
	{ 1, 14, 11 },  // XmmIntDiv

	{ 1, 1, 1 },  // XmmLogic

	{ 1, 3, 1 },  // XmmFloatAdd
	{ 1, 3, 1 },  // XmmFloatCompare
	{ 1, 5, 1 },  // XmmFloatMult
	{ 1, 12, 6 },  // XmmFloatDiv
	{ 1, 3, 1 },  // XmmFloatConv
	{ 1, 22, 14 }  // XmmFloatComplex
};

const FunctionalUnit::Type Alu::type_table[Uinst::OpcodeCount] =
{
	FunctionalUnit::TypeNone,  // UInstNop

	FunctionalUnit::TypeNone,  // UInstMove
	FunctionalUnit::TypeIntAdd,  // UInstAdd
	FunctionalUnit::TypeIntAdd,  // UInstSub
	FunctionalUnit::TypeIntMult,  // UInstMult
	FunctionalUnit::TypeIntDiv,  // UInstDiv
	FunctionalUnit::TypeEffAddr,  // UInstEffAddr

	FunctionalUnit::TypeLogic,  // UInstAnd
	FunctionalUnit::TypeLogic,  // UInstOr
	FunctionalUnit::TypeLogic,  // UInstXor
	FunctionalUnit::TypeLogic,  // UInstNot
	FunctionalUnit::TypeLogic,  // UInstShift
	FunctionalUnit::TypeLogic,  // UInstSign

	FunctionalUnit::TypeNone,  // UInstFpMove
	FunctionalUnit::TypeFloatSimple,  // UInstFpSign
	FunctionalUnit::TypeFloatSimple,  // UInstFpRound

	FunctionalUnit::TypeFloatAdd,  // UInstFpAdd
	FunctionalUnit::TypeFloatAdd,  // UInstFpSub
	FunctionalUnit::TypeFloatCompare,  // UInstFpComp
	FunctionalUnit::TypeFloatMult,  // UInstFpMult
	FunctionalUnit::TypeFloatDiv,  // UInstFpDiv

	FunctionalUnit::TypeFloatComplex,  // UInstFpExp
	FunctionalUnit::TypeFloatComplex,  // UInstFpLog
	FunctionalUnit::TypeFloatComplex,  // UInstFpSin
	FunctionalUnit::TypeFloatComplex,  // UInstFpCos
	FunctionalUnit::TypeFloatComplex,  // UInstFpSincos
	FunctionalUnit::TypeFloatComplex,  // UInstFpTan
	FunctionalUnit::TypeFloatComplex,  // UInstFpAtan
	FunctionalUnit::TypeFloatComplex,  // UInstFpSqrt
};


void Alu::ParseConfiguration(misc::IniFile *ini_file)
{
	// Section
	std::string section = "FunctionalUnits";

	// Get configuration parameter
	for (int i = 1; i < FunctionalUnit::TypeCount; i++)
	{
		// Functional unit name
		std::string name = FunctionalUnit::type_map[i];
		assert(!name.empty());

		// Number of instances
		configuration[i][0] = ini_file->ReadInt(section,
				name + ".Count",
				configuration[i][0]);

		// Get total operation latency
		configuration[i][1] = ini_file->ReadInt(section,
				name + ".OpLat",
				configuration[i][1]);

		// Get issue latency
		configuration[i][2] = ini_file->ReadInt(section,
				name + ".IssueLat",
				configuration[i][2]);
	}
}


void Alu::DumpConfiguration(std::ostream &os)
{
	// Title
	os << "[ Config.FunctionalUnits ]\n";

	// Traverse functional units
	for (int i = 1; i < FunctionalUnit::TypeCount; i++)
	{
		// Functional unit name
		std::string name = FunctionalUnit::type_map[i];
		assert(!name.empty());

		// Print configuration
		os << misc::fmt("%s.Count = %d\n", name.c_str(), configuration[i][0]);
		os << misc::fmt("%s.OpLat = %d\n", name.c_str(), configuration[i][1]);
		os << misc::fmt("%s.IssueLat = %d\n", name.c_str(), configuration[i][2]);
	}

	// Done
	os << '\n';
}


Alu::Alu()
{
	// Reserve functional unit vector entries
	functional_units.resize(FunctionalUnit::TypeCount);

	// Create functional units
	for (int type = 1; type < FunctionalUnit::TypeCount; type++)
		functional_units[type] = misc::new_unique<FunctionalUnit>(
				(FunctionalUnit::Type) type,
				FunctionalUnit::type_map[type],
				configuration[type][0],
				configuration[type][1],
				configuration[type][2]);
}


int Alu::Reserve(Uop *uop)
{
	// Current cycle
	Timing *timing = Timing::getInstance();
	long long cycle = timing->getCycle();

	// Record the first attempt of the uop to reserve a functional unit
	if (!uop->first_alu_cycle)
		uop->first_alu_cycle = cycle;

	// Get the functional unit type required by the uop. If the uop does not
	// require a functional unit, return 1 cycle latency.
	FunctionalUnit::Type type = type_table[uop->getUinst()->getOpcode()];
	if (type == FunctionalUnit::TypeNone)
		return 1;

	// Obtain functional unit of the required kind
	assert(type > FunctionalUnit::TypeNone &&
			type < FunctionalUnit::TypeCount);
	FunctionalUnit *functional_unit = functional_units[type].get();
	assert(functional_unit);

	// Reserve functional unit
	return functional_unit->Reserve(uop);
}


void Alu::ReleaseAll()
{
	// Release all functional units
	for (int type = 1; type < FunctionalUnit::TypeCount; type++)
		functional_units[type]->Release();
}


void Alu::DumpReport(std::ostream &os) const
{
	// Header
	os << "; Functional unit pool\n";
	os << ";    Accesses - Number of uops issued to a f.u.\n";
	os << ";    Denied - Number of requests denied due to busy f.u.\n";
	os << ";    WaitingTime - Average number of waiting cycles to reserve f.u.\n";

	// Traverse functional units
	for (int type = 1; type < FunctionalUnit::TypeCount; type++)
	{
		FunctionalUnit *functional_unit = functional_units[type].get();
		os << misc::fmt("fu.%s.Accesses = %lld\n",
				functional_unit->getName().c_str(),
				functional_unit->getNumAccesses());
		os << misc::fmt("fu.%s.Denied = %lld\n",
				functional_unit->getName().c_str(),
				functional_unit->getNumDeniedAccesses());
		os << misc::fmt("fu.%s.WaitingTime = %.4g\n",
				functional_unit->getName().c_str(),
				functional_unit->getNumAccesses() ?
				(double) functional_unit->getWaitingTime()
				/ functional_unit->getNumAccesses()
				: 0.0);

	}

	// Done
	os << '\n';
}


}

