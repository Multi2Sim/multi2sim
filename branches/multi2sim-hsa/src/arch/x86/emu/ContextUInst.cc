/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "UInst.h"


namespace x86
{

int Context::getMemoryDepSize(UInst *uinst, int index, UInstDep &std_dep)
{
	// Get dependence
	assert(misc::inRange(index, 0, UInstMaxDeps - 1));
	UInstDep dep = uinst->getDep(index);
	std_dep = dep;

	// Check type
	switch (dep)
	{

	case UInstDepRm8:
	case UInstDepRm16:
	case UInstDepRm32:

		// The 'modrm_mod' field of the instruction bytes indicates
		// whether it's actually a memory dependence or a register
		if (inst.getModRmMod() == 3)
			return 0;

		std_dep = UInstDepData;
		return 1 << (dep - UInstDepRm8);

	case UInstDepMem8:
	case UInstDepMem16:
	case UInstDepMem32:
	case UInstDepMem64:
	case UInstDepMem128:

		std_dep = UInstDepData;
		return 1 << (dep - UInstDepMem8);

	case UInstDepMem80:

		std_dep = UInstDepData;
		return 10;

	case UInstDepXmmm32:
	case UInstDepXmmm64:
	case UInstDepXmmm128:

		// The 'modrm_mod' field indicates whether it's actually a memory
		// dependence or a register.
		if (inst.getModRmMod() == 3)
			return 0;

		std_dep = UInstDepXmmData;
		return 1 << (dep - UInstDepXmmm32 + 2);

	default:
		return 0;
	}
}

void Context::EmitUInstEffectiveAddress(UInst *uinst, int index)
{
	// Check if it is a memory dependence
	UInstDep mem_std_dep;
	if (!getMemoryDepSize(uinst, index, mem_std_dep))
		return;

	// Label occurrence of effective address computation
	uinst_effaddr_emitted = true;
	
	// Emit micro-instruction
	UInst *new_uinst = new UInst(UInstEffaddr);
	new_uinst->setIDep(0, inst.getSegment() ?
			inst.getSegment() - InstRegEs + UInstDepEs
			: UInstDepNone);
	new_uinst->setIDep(1, inst.getEaBase() ?
			inst.getEaBase() - InstRegEax + UInstDepEax
			: UInstDepNone);
	new_uinst->setIDep(2, inst.getEaIndex() ?
			inst.getEaIndex() - InstRegEax + UInstDepEax
			: UInstDepNone);
	new_uinst->setODep(0, UInstDepEa);
	uinst_list.emplace_back(new_uinst);
}

void Context::ParseUInstDep(UInst *uinst, int index)
{
	// Regular dependence */
	assert(misc::inRange(index, 0, UInstMaxDeps - 1));
	UInstDep dep = uinst->getDep(index);
	if (UInst::isValidDep(dep))
		return;

	// Instruction dependent
	switch (dep)
	{

	case UInstDepNone:

		break;

	case UInstDepEaseg:

		uinst->setDep(index, inst.getSegment() ?
				inst.getSegment() - InstRegEs + UInstDepEs
				: UInstDepNone);
		break;

	case UInstDepEabas:

		uinst->setDep(index, inst.getEaBase() ?
				inst.getEaBase() - InstRegEax + UInstDepEax
				: UInstDepNone);
		break;

	case UInstDepEaidx:

		uinst->setDep(index, inst.getEaIndex() ?
				inst.getEaIndex() - InstRegEax + UInstDepEax
				: UInstDepNone);
		break;

	// If we reached this point, 'rmXXX' dependences are actually registers.
	// Otherwise, they would have been handled by ParseUInstIDep() or
	// ParseUInstODep() functions.
	case UInstDepRm8:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, inst.getModRmRm() < 4 ?
				UInstDepEax + inst.getModRmRm()
				: UInstDepEax + inst.getModRmRm() - 4);
		break;

	case UInstDepRm16:
	case UInstDepRm32:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, UInstDepEax + inst.getModRmRm());
		break;

	case UInstDepR8:

		uinst->setDep(index, inst.getModRmReg() < 4 ?
				UInstDepEax + inst.getModRmReg()
				: UInstDepEax + inst.getModRmReg() - 4);
		break;

	case UInstDepR16:
	case UInstDepR32:

		uinst->setDep(index, UInstDepEax + inst.getModRmReg());
		break;

	case UInstDepIr8:

		uinst->setDep(index, inst.getOpIndex() < 4 ?
				UInstDepEax + inst.getOpIndex()
				: UInstDepEax + inst.getOpIndex() - 4);
		break;

	case UInstDepIr16:
	case UInstDepIr32:

		uinst->setDep(index, UInstDepEax + inst.getOpIndex());
		break;

	case UInstDepSreg:

		uinst->setDep(index, UInstDepEs + inst.getModRmReg());
		break;

	case UInstDepSti:

		uinst->setDep(index, UInstDepSt0 + inst.getOpIndex());
		break;

	case UInstDepXmmm32:
	case UInstDepXmmm64:
	case UInstDepXmmm128:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, UInstDepXmm0 + inst.getModRmRm());
		break;

	case UInstDepXmm:

		uinst->setDep(index, UInstDepXmm0 + inst.getModRmReg());
		break;

	default:

		misc::panic("%s: unknown dep: %d\n", __FUNCTION__, dep);
	}
}


void Context::ParseUInstIDep(UInst *uinst, int index)
{
	// Get dependence
	assert(misc::inRange(index, 0, UInstMaxIDeps - 1));
	UInstDep dep = uinst->getIDep(index);

	// No action if no dependence
	if (dep == UInstDepNone)
		return;

	// Memory dependence
	UInstDep mem_std_dep;
	int mem_dep_size = getMemoryDepSize(uinst, index, mem_std_dep);
	if (mem_dep_size)
	{
		// If uinst is 'move', just convert it into a 'load'.
		// Replace 'rmXXX' by 'ea' dependence
		if (uinst->getOpcode() == UInstMove)
		{
			uinst->setOpcode(UInstLoad);
			uinst->setDep(index, UInstDepEa);
			uinst->setMemoryAccess(last_effective_address, mem_dep_size);
			return;
		}

		// Load
		UInst *new_uinst = new UInst(UInstLoad);
		new_uinst->setIDep(0, UInstDepEa);
		new_uinst->setODep(0, mem_std_dep);
		new_uinst->setMemoryAccess(last_effective_address, mem_dep_size);
		uinst_list.emplace_back(new_uinst);

		// Input dependence of instruction is converted into UInstDepData
		uinst->setDep(index, mem_std_dep);
		return;
	}

	// Regular dependence
	ParseUInstDep(uinst, index);
}

void Context::ParseUInstODep(UInst *uinst, int index)
{
	// Convert index into global dependence index
	assert(misc::inRange(index, 0, UInstMaxODeps - 1));
	index += UInstMaxIDeps;

	// Nothing is dependency is empty
	UInstDep dep = uinst->getDep(index);
	if (!dep)
		return;

	// Memory dependence
	UInstDep mem_std_dep;
	int mem_dep_size = getMemoryDepSize(uinst, index, mem_std_dep);
	if (mem_dep_size)
	{
		// If uinst is 'move', just convert it into a 'store'
		if (uinst->getOpcode() == UInstMove)
		{
			// Try to add 'ea' as an input dependence
			if (uinst->addIDep(UInstDepEa))
			{
				uinst->setOpcode(UInstStore);
				uinst->setDep(index, UInstDepNone);
				uinst->setMemoryAccess(last_effective_address,
						mem_dep_size);
				return;
			}
		}

		// Store
		UInst *new_uinst = new UInst(UInstStore);
		new_uinst->setIDep(0, UInstDepEa);
		new_uinst->setIDep(1, mem_std_dep);
		new_uinst->setMemoryAccess(last_effective_address, mem_dep_size);
		uinst_list.emplace_back(new_uinst);

		// Output dependence of instruction is UInstDepData
		uinst->setDep(index, mem_std_dep);
		return;
	}

	// Regular dependence
	ParseUInstDep(uinst, index);

}

void Context::ProcessNewUInst(UInst *uinst)
{
	// Emit effective address computation if needed.
	for (int i = 0; !uinst_effaddr_emitted && i < UInstMaxDeps; i++)
		EmitUInstEffectiveAddress(uinst, i);
	
	// Parse input dependences
	for (int i = 0; i < UInstMaxIDeps; i++)
		ParseUInstIDep(uinst, i);
	
	// Add micro-instruction to list
	uinst_list.emplace_back(uinst);
	
	// Parse output dependences
	for (int i = 0; i < UInstMaxODeps; i++)
		ParseUInstODep(uinst, i);
}


}  // namespace x86

