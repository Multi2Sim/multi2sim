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
#include "Uinst.h"


namespace x86
{

int Context::getMemoryDepSize(Uinst *uinst, int index, Uinst::Dep &std_dep)
{
	// Get dependence
	assert(misc::inRange(index, 0, Uinst::MaxDeps - 1));
	Uinst::Dep dep = uinst->getDep(index);
	std_dep = dep;

	// Check type
	switch (dep)
	{

	case Uinst::DepRm8:
	case Uinst::DepRm16:
	case Uinst::DepRm32:

		// The 'modrm_mod' field of the instruction bytes indicates
		// whether it's actually a memory dependence or a register
		if (inst.getModRmMod() == 3)
			return 0;

		std_dep = Uinst::DepData;
		return 1 << (dep - Uinst::DepRm8);

	case Uinst::DepMem8:
	case Uinst::DepMem16:
	case Uinst::DepMem32:
	case Uinst::DepMem64:
	case Uinst::DepMem128:

		std_dep = Uinst::DepData;
		return 1 << (dep - Uinst::DepMem8);

	case Uinst::DepMem80:

		std_dep = Uinst::DepData;
		return 10;

	case Uinst::DepXmmm32:
	case Uinst::DepXmmm64:
	case Uinst::DepXmmm128:

		// The 'modrm_mod' field indicates whether it's actually a memory
		// dependence or a register.
		if (inst.getModRmMod() == 3)
			return 0;

		std_dep = Uinst::DepXmmData;
		return 1 << (dep - Uinst::DepXmmm32 + 2);

	default:
		return 0;
	}
}


void Context::EmitUinstEffectiveAddress(Uinst *uinst, int index)
{
	// Check if it is a memory dependence
	Uinst::Dep mem_std_dep;
	if (!getMemoryDepSize(uinst, index, mem_std_dep))
		return;

	// Label occurrence of effective address computation
	uinst_effaddr_emitted = true;

	// Create micro-instruction
	uinsts.emplace_back(misc::new_unique<Uinst>(Uinst::OpcodeEffaddr));
	Uinst *new_uinst = uinsts.back().get();
	
	// Emit micro-instruction
	new_uinst->setIDep(0, inst.getSegment() ?
			inst.getSegment() - Instruction::RegEs + Uinst::DepEs
			: Uinst::DepNone);
	new_uinst->setIDep(1, inst.getEaBase() ?
			inst.getEaBase() - Instruction::RegEax + Uinst::DepEax
			: Uinst::DepNone);
	new_uinst->setIDep(2, inst.getEaIndex() ?
			inst.getEaIndex() - Instruction::RegEax + Uinst::DepEax
			: Uinst::DepNone);
	new_uinst->setODep(0, Uinst::DepEa);
}


void Context::ParseUinstDep(Uinst *uinst, int index)
{
	// Regular dependence
	assert(misc::inRange(index, 0, Uinst::MaxDeps - 1));
	Uinst::Dep dep = uinst->getDep(index);
	if (Uinst::isValidDependency(dep))
		return;

	// Instruction dependent
	switch (dep)
	{

	case Uinst::DepNone:

		break;

	case Uinst::DepEaseg:

		uinst->setDep(index, inst.getSegment() ?
				inst.getSegment() - Instruction::RegEs + Uinst::DepEs
				: Uinst::DepNone);
		break;

	case Uinst::DepEabas:

		uinst->setDep(index, inst.getEaBase() ?
				inst.getEaBase() - Instruction::RegEax + Uinst::DepEax
				: Uinst::DepNone);
		break;

	case Uinst::DepEaidx:

		uinst->setDep(index, inst.getEaIndex() ?
				inst.getEaIndex() - Instruction::RegEax + Uinst::DepEax
				: Uinst::DepNone);
		break;

	// If we reached this point, 'rmXXX' dependences are actually registers.
	// Otherwise, they would have been handled by ParseUInstIDep() or
	// ParseUInstODep() functions.
	case Uinst::DepRm8:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, inst.getModRmRm() < 4 ?
				Uinst::DepEax + inst.getModRmRm()
				: Uinst::DepEax + inst.getModRmRm() - 4);
		break;

	case Uinst::DepRm16:
	case Uinst::DepRm32:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, Uinst::DepEax + inst.getModRmRm());
		break;

	case Uinst::DepR8:

		uinst->setDep(index, inst.getModRmReg() < 4 ?
				Uinst::DepEax + inst.getModRmReg()
				: Uinst::DepEax + inst.getModRmReg() - 4);
		break;

	case Uinst::DepR16:
	case Uinst::DepR32:

		uinst->setDep(index, Uinst::DepEax + inst.getModRmReg());
		break;

	case Uinst::DepIr8:

		uinst->setDep(index, inst.getOpIndex() < 4 ?
				Uinst::DepEax + inst.getOpIndex()
				: Uinst::DepEax + inst.getOpIndex() - 4);
		break;

	case Uinst::DepIr16:
	case Uinst::DepIr32:

		uinst->setDep(index, Uinst::DepEax + inst.getOpIndex());
		break;

	case Uinst::DepSreg:

		uinst->setDep(index, Uinst::DepEs + inst.getModRmReg());
		break;

	case Uinst::DepSti:

		uinst->setDep(index, Uinst::DepSt0 + inst.getOpIndex());
		break;

	case Uinst::DepXmmm32:
	case Uinst::DepXmmm64:
	case Uinst::DepXmmm128:

		assert(inst.getModRmMod() == 3);
		uinst->setDep(index, Uinst::DepXmm0 + inst.getModRmRm());
		break;

	case Uinst::DepXmm:

		uinst->setDep(index, Uinst::DepXmm0 + inst.getModRmReg());
		break;

	default:

		misc::panic("%s: unknown dep: %d\n", __FUNCTION__, dep);
	}
}


void Context::ParseUinstIDep(Uinst *uinst, int index)
{
	// Get dependence
	assert(misc::inRange(index, 0, Uinst::MaxIDeps - 1));
	Uinst::Dep dep = uinst->getIDep(index);

	// No action if no dependence
	if (dep == Uinst::DepNone)
		return;

	// Memory dependence
	Uinst::Dep mem_std_dep;
	int mem_dep_size = getMemoryDepSize(uinst, index, mem_std_dep);
	if (mem_dep_size)
	{
		// If uinst is 'move', just convert it into a 'load'.
		// Replace 'rmXXX' by 'ea' dependence
		if (uinst->getOpcode() == Uinst::OpcodeMove)
		{
			uinst->setOpcode(Uinst::OpcodeLoad);
			uinst->setDep(index, Uinst::DepEa);
			uinst->setMemoryAccess(last_effective_address, mem_dep_size);
			return;
		}

		// Load
		uinsts.emplace_back(misc::new_unique<Uinst>(Uinst::OpcodeLoad));
		Uinst *new_uinst = uinsts.back().get();
		new_uinst->setIDep(0, Uinst::DepEa);
		new_uinst->setODep(0, mem_std_dep);
		new_uinst->setMemoryAccess(last_effective_address, mem_dep_size);

		// Input dependence of instruction is converted into Uinst::DepData
		uinst->setDep(index, mem_std_dep);
		return;
	}

	// Regular dependence
	ParseUinstDep(uinst, index);
}

void Context::ParseUinstODep(Uinst *uinst, int index)
{
	// Convert index into global dependence index
	assert(misc::inRange(index, 0, Uinst::MaxODeps - 1));
	index += Uinst::MaxIDeps;

	// Nothing is dependency is empty
	Uinst::Dep dep = uinst->getDep(index);
	if (!dep)
		return;

	// Memory dependence
	Uinst::Dep mem_std_dep;
	int mem_dep_size = getMemoryDepSize(uinst, index, mem_std_dep);
	if (mem_dep_size)
	{
		// If uinst is 'move', just convert it into a 'store'
		if (uinst->getOpcode() == Uinst::OpcodeMove)
		{
			// Try to add 'ea' as an input dependence
			if (uinst->addIDep(Uinst::DepEa))
			{
				uinst->setOpcode(Uinst::OpcodeStore);
				uinst->setDep(index, Uinst::DepNone);
				uinst->setMemoryAccess(last_effective_address,
						mem_dep_size);
				return;
			}
		}

		// Store
		uinsts.emplace_back(misc::new_unique<Uinst>(Uinst::OpcodeStore));
		Uinst *new_uinst = uinsts.back().get();
		new_uinst->setIDep(0, Uinst::DepEa);
		new_uinst->setIDep(1, mem_std_dep);
		new_uinst->setMemoryAccess(last_effective_address, mem_dep_size);

		// Output dependence of instruction is Uinst::DepData
		uinst->setDep(index, mem_std_dep);
		return;
	}

	// Regular dependence
	ParseUinstDep(uinst, index);

}


void Context::newMemoryUinst(
		Uinst::Opcode opcode,
		unsigned address,
		int size,
		int idep0,
		int idep1,
		int idep2,
		int odep0,
		int odep1,
		int odep2,
		int odep3)
{
	// Discard if we're in function simulation mode
	if (!uinst_active)
		return;

	// Create micro-instruction
	auto uinst = misc::new_shared<Uinst>(opcode);

	// Initialize
	uinst->setMemoryAccess(address, size);
	uinst->setIDep(0, idep0);
	uinst->setIDep(1, idep1);
	uinst->setIDep(2, idep2);
	uinst->setODep(0, odep0);
	uinst->setODep(1, odep1);
	uinst->setODep(2, odep2);
	uinst->setODep(3, odep3);

	// Emit effective address computation if needed.
	for (int i = 0; !uinst_effaddr_emitted && i < Uinst::MaxDeps; i++)
		EmitUinstEffectiveAddress(uinst.get(), i);
	
	// Parse input dependences
	for (int i = 0; i < Uinst::MaxIDeps; i++)
		ParseUinstIDep(uinst.get(), i);
	
	// Add micro-instruction to list
	uinsts.emplace_back(uinst);
	
	// Parse output dependences
	for (int i = 0; i < Uinst::MaxODeps; i++)
		ParseUinstODep(uinst.get(), i);
}

}

