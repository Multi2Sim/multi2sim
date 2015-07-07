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

#include "VectorMemoryUnit.h"
#include "ComputeUnit.h"


namespace SI
{

int VectorMemoryUnit::issue_buffer_size = 1;
int VectorMemoryUnit::decode_latency = 1;
int VectorMemoryUnit::decode_buffer_size = 1;
int VectorMemoryUnit::read_latency = 1;
int VectorMemoryUnit::read_buffer_size = 1;
int VectorMemoryUnit::write_latency = 1;
int VectorMemoryUnit::write_buffer_size = 1;


void VectorMemoryUnit::Run()
{
}

bool VectorMemoryUnit::isValidUop(Uop *uop) const
{
	// Get instruction
	Instruction *instruction = uop->getInstruction();

	// Determine if vector memory instruction
	if (instruction->getFormat() != Instruction::FormatMTBUF &&
			instruction->getFormat() != Instruction::FormatMUBUF)
		return false;

	return true;
}

void VectorMemoryUnit::Issue(std::shared_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();

	// One more instruction of this kind
	compute_unit->num_vector_memory_instructions++;
	uop->getWavefrontPoolEntry()->lgkm_cnt++;

	// Issue it
	ExecutionUnit::Issue(uop);
}


}

