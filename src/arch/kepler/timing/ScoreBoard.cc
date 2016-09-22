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

#include <cstring>

#include <arch/kepler/disassembler/Instruction.h>
#include <arch/kepler/emulator/Warp.h>

#include "ScoreBoard.h"
#include "SM.h"


namespace Kepler
{

ScoreBoard::ScoreBoard(int id, SM *sm):sm(sm), id(id)
{
	register_table.resize(sm->max_warps_per_warp_pool);
	predicate_table.resize(sm->max_warps_per_warp_pool);
	long_operation_register_table.resize(sm->max_warps_per_warp_pool);
}


void ScoreBoard::ReserveRegister(Warp *warp, unsigned register_index)
{
	int warp_pool_entry_index = warp->getWarpPoolEntryIndex();
	if (!(register_table[warp_pool_entry_index].find(register_index) ==
			register_table[warp_pool_entry_index].end()))
	{
		throw misc::Panic(misc::fmt("Kepler Scoreboard exception: trying to reserve a reserved"
				" register.\n SM ID = %d, Warp ID = %d, Register index = %u",
				sm->getId(), warp->getWarpPoolEntryIndex(), register_index));
	}

	register_table[warp_pool_entry_index].insert(register_index);
}


void ScoreBoard::ReservePredicate(Warp *warp, unsigned predicate_index)
{
	int warp_pool_entry_index = warp->getWarpPoolEntryIndex();
	if (!(predicate_table[warp_pool_entry_index].find(predicate_index) ==
			predicate_table[warp_pool_entry_index].end()))
	{
		throw misc::Panic(misc::fmt("Kepler Scoreboard exception: trying to reserve a reserved"
				"predicate register.\n SM ID = %d, Warp ID = %d, Register index"
				"= %d", sm->getId(), warp->getWarpPoolEntryIndex(),
				predicate_index));
	}

	predicate_table[warp_pool_entry_index].insert(predicate_index);
}


void ScoreBoard::ReserveRegisters(Warp *warp, Uop *uop)
{
	// Get instruction name
	Instruction::Opcode opcode = uop->getInstructionOpcode();

	/*
	if (warp->getId() == 1)
	{
		std::cout << " Scoreboard.cc in function reserve register "<< std::endl;
		std::cout << "Uop pc is" << uop->getInstructionPC() << std::endl;
		std::cout<<"Before reserve" << std::endl;
		for (auto it = register_table[warp->getWarpPoolEntryIndex()].begin();
			it != register_table[warp->getWarpPoolEntryIndex()].end(); ++it)
		std::cout << *it <<'\n';
	}
*/
	// Reserve registers
	for (unsigned i = 0; i < 4; i++)
	{
		int index = uop->getDestinationRegisterIndex(i);
		if (index > 0)
		{
			this->ReserveRegister(warp, index);
		}
	}
/*
	if (warp->getId() == 1)
	{
		std::cout << " Scoreboard.cc in function reserve register "<< std::endl;
		std::cout<<"After reserve" << std::endl;
		for (auto it = register_table[warp->getWarpPoolEntryIndex()].begin();
			it != register_table[warp->getWarpPoolEntryIndex()].end(); ++it)
		std::cout << *it <<'\n';
	}
*/
	// Reserve predicates
	for (unsigned i = 0; i < 2; i++)
	{
		int predicate_index = uop->getDestinationPredicateIndex(i);
		if (predicate_index > 0)
		{
			this->ReservePredicate(warp, predicate_index);
		}
	}

	// Track long operations  TODO only LD? or such as LDC are included
	if (opcode == Instruction::INST_LD)
	{
		for (unsigned i = 0; i < 4; i++)
		{
			int index = uop->getDestinationRegisterIndex(i);
			if (index > 0)
			{
				this->long_operation_register_table
				[warp->getWarpPoolEntryIndex()].insert(index);
			}
		}
	}
}


void ScoreBoard::ReleaseRegister(Warp *warp, unsigned register_index)
{
	int warp_pool_entry_index = warp->getWarpPoolEntryIndex();
	if (register_table[warp_pool_entry_index].find(register_index) ==
			register_table[warp_pool_entry_index].end())
		return;
	register_table[warp_pool_entry_index].erase(register_index);
}


void ScoreBoard::ReleasePredicate(Warp *warp, unsigned predicate_index)
{
	int warp_pool_entry_index = warp->getWarpPoolEntryIndex();
	if (predicate_table[warp_pool_entry_index].find(predicate_index) ==
			predicate_table[warp_pool_entry_index].end())
		return;
	predicate_table[warp_pool_entry_index].erase(predicate_index);
}


void ScoreBoard::ReleaseRegisters(Warp *warp, Uop *uop)
{
	// Get instruction opcode
	Instruction::Opcode opcode = uop->getInstructionOpcode();
/*
	if (warp->getId() == 1)
	{
		std::cout << " Scoreboard.cc in function release register before release" << std::endl;
		for (auto it = register_table[warp->getWarpPoolEntryIndex()].begin();
			it != register_table[warp->getWarpPoolEntryIndex()].end(); ++it)
		std::cout << *it <<'\n';
	}
*/
	for (unsigned i = 0; i < 4; i++)
	{
		int index = uop->getDestinationRegisterIndex(i);
		if (index > 0)
			this->ReleaseRegister(warp, index);
	}
/*
	if (warp->getId() == 1)
	{
		std::cout << " Scoreboard.cc in function release register after release" << std::endl;
		for (auto it = register_table[warp->getWarpPoolEntryIndex()].begin();
			it != register_table[warp->getWarpPoolEntryIndex()].end(); ++it)
		std::cout << *it <<'\n';
	}
*/
	// Release predicate
	for (unsigned i = 0; i < 2; i++)
	{
		int predicate_index = uop->getDestinationPredicateIndex(i);
		if (predicate_index > 0)
		{
			this->ReleasePredicate(warp, predicate_index);
		}
	}

	// Track long operations  TODO only LD? or such as LDC are included
	if (opcode == Instruction::INST_LD)
	{
		for (unsigned i = 0; i < 4; i++)
		{
			int index = uop->getDestinationRegisterIndex(i);
			if (index > 0)
			{
				this->long_operation_register_table
				[warp->getWarpPoolEntryIndex()].erase(index);
			}
		}
	}
}

bool ScoreBoard::CheckCollision(Warp *warp, Uop *uop)
{

	// Warp ID
	int warp_pool_entry_index = warp->getWarpPoolEntryIndex();

	// Get list of source and destination registers
	std::set<int> instruction_registers;

	// Get list of predicates
	std::set<int> instruction_predicates;

	std::set<int>::iterator index;

	for (int i = 0; i < 4; i++)
	{
		if (uop->getDestinationRegisterIndex(i) > 0)
			instruction_registers.insert
				(uop->getDestinationRegisterIndex(i));
		if (uop->getSourceRegisterIndex(i) > 0)
			instruction_registers.insert
				(uop->getSourceRegisterIndex(i));
		if (uop->getSourcePredicateIndex(i) > 0)
			instruction_predicates.insert
				(uop->getSourcePredicateIndex(i));
		if (i == 0 || i == 1)
		{
		 if (uop->getDestinationPredicateIndex(i) > 0)
			 instruction_predicates.insert
			 	 (uop->getDestinationPredicateIndex(i));
		}
	}

	// Check for collision
	for (index = instruction_registers.begin();
			index != instruction_registers.end(); index++)
	{
		if (register_table[warp_pool_entry_index].find(*index) !=
				register_table[warp_pool_entry_index].end())
			return true;
	}

	for (index = instruction_predicates.begin();
			index != instruction_predicates.end(); index++)
	{
		if (predicate_table[warp_pool_entry_index].find(*index) !=
				predicate_table[warp_pool_entry_index].end())
		return true;
	}

	return false;
}


} // namespace Kepler

