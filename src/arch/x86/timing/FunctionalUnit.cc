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

const int FunctionalUnit::MaxInstances;

const misc::StringMap FunctionalUnit::type_map =
{
	{ "None", TypeNone },
	{ "IntAdd", TypeIntAdd },
	{ "IntMult", TypeIntMult },
	{ "IntDiv", TypeIntDiv },
	{ "EffAddr", TypeEffAddr },
	{ "Logic", TypeLogic },
	{ "FloatSimple", TypeFloatSimple },
	{ "FloatAdd", TypeFloatAdd },
	{ "FloatCompare", TypeFloatCompare },
	{ "FloatMult", TypeFloatMult },
	{ "FloatDiv", TypeFloatDiv },
	{ "FloatComplex", TypeFloatComplex },
	{ "XmmIntAdd", TypeXmmIntAdd },
	{ "XmmIntMult", TypeXmmIntMult },
	{ "XmmIntDiv", TypeXmmIntDiv },
	{ "XmmLogic", TypeXmmLogic },
	{ "XmmFloatAdd", TypeXmmFloatAdd },
	{ "XmmFloatCompare", TypeXmmFloatCompare },
	{ "XmmFloatMult", TypeXmmFloatMult },
	{ "XmmFloatDiv", TypeXmmFloatDiv },
	{ "XmmFloatConv", TypeXmmFloatConv },
	{ "XmmFloatComplex", TypeXmmFloatComplex }
};


int FunctionalUnit::Reserve(Uop *uop)
{
	// Current cycle
	Timing *timing = Timing::getInstance();
	long long cycle = timing->getCycle();

	// Find a free functional unit
	assert(num_instances <= MaxInstances);
	for (int i = 0; i < num_instances; i++)
	{
		// Skip if instance is busy
		if (cycle < cycle_free[i])
			continue;

		// Reserve instance
		assert(operation_latency > 0);
		assert(issue_latency > 0);
		cycle_free[i] = cycle + issue_latency;

		// Stats
		num_accesses++;
		waiting_time += cycle - uop->first_alu_cycle;

		// Return the total operation latency
		return operation_latency;
	}

	// No free instance found
	num_denied_accesses++;
	return 0;
}


void FunctionalUnit::Release()
{
	assert(num_instances <= MaxInstances);
	for (int i = 0; i < num_instances; i++)
		cycle_free[i] = 0;
}

}

