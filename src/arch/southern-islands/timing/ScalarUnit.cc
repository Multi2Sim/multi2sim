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

#include "ComputeUnit.h"
#include "ScalarUnit.h"


namespace SI
{

int ScalarUnit::width = 1;
int ScalarUnit::issue_buffer_size = 4;
int ScalarUnit::decode_latency = 1;
int ScalarUnit::decode_buffer_size = 1;
int ScalarUnit::read_latency = 1;
int ScalarUnit::read_buffer_size = 1;
int ScalarUnit::exec_latency = 4;
int ScalarUnit::exec_buffer_size = 32;
int ScalarUnit::write_latency = 1;
int ScalarUnit::write_buffer_size = 1;


void ScalarUnit::Run()
{
}


bool ScalarUnit::isValidUop(Uop *uop) const
{
	Instruction *instruction = uop->getInstruction();
	if (instruction->getFormat() != Instruction::FormatSOPP &&
			instruction->getFormat() != Instruction::FormatSOP1 &&
			instruction->getFormat() != Instruction::FormatSOP2 &&
			instruction->getFormat() != Instruction::FormatSOPC &&
			instruction->getFormat() != Instruction::FormatSOPK &&
			instruction->getFormat() != Instruction::FormatSMRD)
		return false;
	
	if (instruction->getFormat() == Instruction::FormatSOPP && 
			instruction->getBytes()->sopp.op > 1 && 
			instruction->getBytes()->sopp.op < 10)
		return false;

	return true;
}


void ScalarUnit::Issue(std::shared_ptr<Uop> uop)
{
	// One more instruction of this kind
	ComputeUnit *compute_unit = getComputeUnit();
	if (uop->getInstruction()->getFormat() == Instruction::FormatSMRD)
	{
		compute_unit->num_scalar_memory_instructions++;
		uop->getWavefrontPoolEntry()->lgkm_cnt++;
	}
	else
	{
		// Scalar ALU instructions must complete before the next
		// instruction can be fetched.
		compute_unit->num_scalar_alu_instructions++;
	}

	// Issue it
	ExecutionUnit::Issue(uop);
}

}

