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

#include "SimdUnit.h"
#include "ComputeUnit.h"


namespace SI
{

int SimdUnit::num_simd_lanes = 16;
int SimdUnit::issue_buffer_size = 1;
int SimdUnit::decode_latency = 1;
int SimdUnit::decode_buffer_size = 1;
int SimdUnit::read_exec_write_latency = 8;
int SimdUnit::read_exec_write_buffer_size = 2;


void SimdUnit::Run()
{
}

bool SimdUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();

	// Determine if simd instruction
	if (instruction->getFormat() != Instruction::FormatVOP2 &&
			instruction->getFormat() != Instruction::FormatVOP1 &&
			instruction->getFormat() != Instruction::FormatVOPC &&
			instruction->getFormat() != Instruction::FormatVOP3a &&
			instruction->getFormat() != Instruction::FormatVOP3b)
		return false;

	return true;
}

void SimdUnit::Issue(std::shared_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	compute_unit->num_simd_instructions++;

	// Issue it
	ExecutionUnit::Issue(uop);
}

}

