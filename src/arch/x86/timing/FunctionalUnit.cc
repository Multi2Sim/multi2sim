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

#include "FunctionalUnit.h"
#include "Timing.h"

namespace x86
{

std::string FunctionalUnit::name[TypeCount] =
{
		// Invalid
		"<invalid>",

		// Integer register operation
		"IntAdd",
		"IntMult",
		"IntDiv",

		// Logic operation
		"EffAddr",
		"Logic",

		// Floating point register operation
		"FloatSimple",
		"FloatAdd",
		"FloatComp",
		"FloatMult",
		"FloatDiv",
		"FloatComplex",

		// XMM register operation
		"XMMIntAdd",
		"XMMIntMult",
		"XMMIntDiv",
		"XMMLogic",
		"XMMFloatAdd",
		"XMMFloatComp",
		"XMMFloatMult",
		"XMMFloatDiv",
		"XMMFloatConv",
		"XMMFloatComplex"
};

FunctionalUnit::ReservationPool FunctionalUnit::reservation_pool[TypeCount] =
{
		{ 0, 0, 0 },  // Unused

		{ 3, 1, 1 },  // IntAdd
		{ 1, 3, 1 },  // IntMult
		{ 1, 14, 11 },  // IntDiv

		{ 3, 2, 2 },  // Effaddr
		{ 3, 1, 1 },  // Logic

		{ 1, 2, 1 },  // FloatSimple
		{ 1, 3, 1 },  // FloatAdd
		{ 1, 3, 1 },  // FloatComp
		{ 1, 5, 1 },  // FloatMult
		{ 1, 12, 5 },  // FloatDiv
		{ 1, 22, 14 },  // FloatComplex

		{ 1, 1, 1 },  // XmmIntAdd
		{ 1, 3, 1 },  // XmmIntMult
		{ 1, 14, 11 },  // XmmIntDiv

		{ 1, 1, 1 },  // XmmLogic

		{ 1, 3, 1 },  // XmmFloatAdd
		{ 1, 3, 1 },  // XmmFloatComp
		{ 1, 5, 1 },  // XmmFloatMult
		{ 1, 12, 6 },  // XmmFloatDiv
		{ 1, 3, 1 },  // XmmFloatConv
		{ 1, 22, 14 }  // XmmFloatComplex
};

FunctionalUnit::Type FunctionalUnit::type_table[UInstOpcodeCount] =
{
		TypeNone,  // UInstNop

		TypeNone,  // UInstMove
		TypeIntAdd,  // UInstAdd
		TypeIntAdd,  // UInstSub
		TypeIntMult,  // UInstMult
		TypeIntDiv,  // UInstDiv
		TypeEffaddr,  // UInstEffaddr

		TypeLogic,  // UInstAnd
		TypeLogic,  // UInstOr
		TypeLogic,  // UInstXor
		TypeLogic,  // UInstNot
		TypeLogic,  // UInstShift
		TypeLogic,  // UInstSign

		TypeNone,  // UInstFpMove
		TypeFloatSimple,  // UInstFpSign
		TypeFloatSimple,  // UInstFpRound

		TypeFloatAdd,  // UInstFpAdd
		TypeFloatAdd,  // UInstFpSub
		TypeFloatComp,  // UInstFpComp
		TypeFloatMult,  // UInstFpMult
		TypeFloatDiv,  // UInstFpDiv

		TypeFloatComplex,  // UInstFpExp
		TypeFloatComplex,  // UInstFpLog
		TypeFloatComplex,  // UInstFpSin
		TypeFloatComplex,  // UInstFpCos
		TypeFloatComplex,  // UInstFpSincos
		TypeFloatComplex,  // UInstFpTan
		TypeFloatComplex,  // UInstFpAtan
		TypeFloatComplex,  // UInstFpSqrt
};


void FunctionalUnit::ParseConfiguration(misc::IniFile *ini_file)
{
	// Section
	std::string section = "FunctionalUnits";

	// Get configuration parameter
	for (int i = 1; i < TypeCount; i++)
	{
		// Get corresponding pool entry
		ReservationPool *pool_ptr = &reservation_pool[i];
		std::string value = "";

		// Get functional units count
		value = name[i] + ".Count";
		pool_ptr->count = ini_file->ReadInt(section, value, pool_ptr->count);

		// Get operation latency
		value = name[i] + ".OpLat";
		pool_ptr->operation_latency = ini_file->ReadInt(section, value,
				pool_ptr->operation_latency);

		// Get issue latency
		value = name[i] + ".IssueLat";
		pool_ptr->issue_latency = ini_file->ReadInt(section, value,
				pool_ptr->issue_latency);
	}
}


int FunctionalUnit::Reserve(Uop *uop)
{
	// Get the functional unit type required by the uop.
	// If the uop does not require a functional unit, return
	// 1 cycle latency.
	Type type = type_table[uop->getUinst()->getOpcode()];
	if (type == TypeNone)
		return 1;

	// First time uop tries to reserve functional unit
	if (!uop->getFirstCycleTryReserve() == 0)
		uop->setFirstCycleTryReserve(Timing::getInstance()->getCycle());

	// Find a free functional unit
	assert(type > TypeNone && type < TypeCount);
	assert(reservation_pool[type].count <= MaxFunctionalUnitReservation);
	for (int i = 0; i < reservation_pool[type].count; i++)
	{
		if (cycle_when_free[type][i] <= Timing::getInstance()->getCycle())
		{
			// Make sure the latency exist
			assert(reservation_pool[type].issue_latency > 0);
			assert(reservation_pool[type].operation_latency > 0);

			// Calculate the cycle count when functional unit is free
			cycle_when_free[type][i] = Timing::getInstance()->getCycle()
					+ reservation_pool[type].issue_latency;

			// Increment the access count
			accesses[type]++;

			// Calculate the wait time in cycle
			waiting_time[type] += Timing::getInstance()->getCycle() - uop->getFirstCycleTryReserve();

			// Return the operation latency and indication functional unit is reserved
			return reservation_pool[type].operation_latency;
		}
	}

	// No free functional unit was found
	denied[type]++;
	return 0;
}


void FunctionalUnit::ReleaseAll()
{
	// Clear the free cycle count for each functional unit
	for (int i = 0; i < TypeCount; i++)
		for (int j = 0; j < reservation_pool[i].count; j++)
			cycle_when_free[i][j] = 0;
}

}
