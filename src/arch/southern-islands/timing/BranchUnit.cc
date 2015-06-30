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

#include "BranchUnit.h"
#include "Timing.h"


namespace SI
{

int BranchUnit::issue_buffer_size = 1;
int BranchUnit::decode_latency = 1;
int BranchUnit::decode_buffer_size = 1;
int BranchUnit::read_latency = 1;
int BranchUnit::read_buffer_size = 1;
int BranchUnit::exec_latency = 16;
int BranchUnit::exec_buffer_size = 16;
int BranchUnit::write_latency = 1;
int BranchUnit::write_buffer_size = 1;


void BranchUnit::Run()
{
}


bool BranchUnit::isValidUop(Uop *uop) const
{
	Instruction *instruction = uop->getInstruction();
	return instruction->getFormat() != Instruction::FormatSOPP &&
			instruction->getBytes()->sopp.op > 1 &&
			instruction->getBytes()->sopp.op < 10;
}

}

