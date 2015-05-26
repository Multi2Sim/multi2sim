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

#include "RegisterFile.h"
#include "Core.h"
#include "Thread.h"

namespace x86
{

const int RegisterFile::MinINTSize = UInstDepIntCount + UInstMaxODeps;
const int RegisterFile::MinFPSize = UInstDepFpCount + UInstMaxODeps;
const int RegisterFile::MinXMMSize = UInstDepXmmCount + UInstMaxODeps;

misc::StringMap RegisterFile::KindMap =
{
	{"Shared", KindShared},
	{"Private", KindPrivate}
};

RegisterFile::Kind RegisterFile::kind;
int RegisterFile::int_size;
int RegisterFile::fp_size;
int RegisterFile::xmm_size;
int RegisterFile::int_local_size;
int RegisterFile::fp_local_size;
int RegisterFile::xmm_local_size;


RegisterFile::RegisterFile(Core *core, Thread *thread)
	:
	core(core), thread(thread)
{
	// Integer register file
	int_phy_reg_count = int_local_size;
	int_phy_reg = misc::new_unique_array<PhysicalRegister>(int_local_size);

	// Free list
	int_free_phy_reg_count = int_local_size;
	int_free_phy_reg = misc::new_unique_array<int>(int_local_size);
	for (int phy_reg = 0; phy_reg < int_local_size; phy_reg++)
		int_free_phy_reg[phy_reg] = phy_reg;

	// Floating-point register file
	fp_phy_reg_count = fp_local_size;
	fp_phy_reg = misc::new_unique_array<PhysicalRegister>(fp_local_size);

	// Free list
	fp_free_phy_reg_count = fp_local_size;
	fp_free_phy_reg = misc::new_unique_array<int>(fp_local_size);

	// Initialize free list
	for (int phy_reg = 0; phy_reg < fp_local_size; phy_reg++)
		fp_free_phy_reg[phy_reg] = phy_reg;

	// XMM register file
	xmm_phy_reg_count = xmm_local_size;
	xmm_phy_reg = misc::new_unique_array<PhysicalRegister>(xmm_local_size);

	// Free list
	xmm_free_phy_reg_count = xmm_local_size;
	xmm_free_phy_reg = misc::new_unique_array<int>(xmm_local_size);

	// Initialize free list
	for (int phy_reg = 0; phy_reg < xmm_local_size; phy_reg++)
		xmm_free_phy_reg[phy_reg] = phy_reg;
}


void RegisterFile::ParseConfiguration(const std::string &section,
				misc::IniFile &config)
{
	kind = (Kind)config.ReadEnum(section, "RfKind",
			KindMap, KindPrivate);
	int_size = config.ReadInt(section, "RfIntSize", 80);
	fp_size = config.ReadInt(section, "RfFpSize", 40);
	xmm_size = config.ReadInt(section, "RfXmmSize", 40);

	int num_threads = config.ReadInt("General", "Threads", 1);

	if (int_size < MinINTSize)
		misc::fatal("rf_int_size must be at least %d", MinINTSize);
	if (fp_size < MinFPSize)
		misc::fatal("rf_fp_size must be at least %d", MinFPSize);
	if (xmm_size < MinXMMSize)
		misc::fatal("rf_xmm_size must be at least %d", MinXMMSize);

	if (kind == KindPrivate)
	{
		int_local_size = int_size;
		fp_local_size = fp_size;
		xmm_local_size = xmm_size;
	}
	else
	{
		int_local_size = int_size * num_threads;
		fp_local_size = fp_size * num_threads;
		xmm_local_size = xmm_size * num_threads;
	}
}


void RegisterFile::InitRegisterFile()
{
	// Local variable
	int phy_reg;
	int dependency;

	// Initial mapping for the integer register file.
	// Map each logical register to a new physical register,
	// and map all flags to the first allocated physical register.
	// FIXME Not consistent with M2S doc.
	int flag_phy_reg = -1;
	for (int dep = 0; dep < UInstDepIntCount; dep++)
	{
		dependency = dep + UInstDepIntFirst;
		if (dependency >= UInstDepFlagFirst && dependency <= UInstDepFlagLast)
		{
			assert(flag_phy_reg >= 0);
			phy_reg = flag_phy_reg;
		}
		else
		{
			phy_reg = RequestIntRegister();
			flag_phy_reg = phy_reg;
		}
		int_phy_reg[phy_reg].busy++;
		int_rat[dep] = phy_reg;
	}

	// Initial mapping for floating-point registers.
	for (int dep = 0; dep < UInstDepFpCount; dep++)
	{
		phy_reg = RequestFPRegister();
		fp_phy_reg[phy_reg].busy++;
		fp_rat[dep] = phy_reg;
	}

	// Initial mapping for xmm registers.
	for (int dep = 0; dep < UInstDepXmmCount; dep++)
	{
		phy_reg = RequestXMMRegister();
		xmm_phy_reg[phy_reg].busy++;
		xmm_rat[dep] = phy_reg;
	}
}


int RegisterFile::RequestIntRegister()
{
	// Local variable
	int phy_reg;

	// Obtain a register from the free list
	assert(int_free_phy_reg_count > 0);
	phy_reg = int_free_phy_reg[int_free_phy_reg_count - 1];
	int_free_phy_reg_count--;
	core->incRegFileIntCount();
	thread->incRegFileIntCount();
	assert(!int_phy_reg[phy_reg].busy);
	assert(!int_phy_reg[phy_reg].pending);
	return phy_reg;
}


int RegisterFile::RequestFPRegister()
{
	// Local variable
	int phy_reg;

	// Obtain a register from the free list
	assert(fp_free_phy_reg_count > 0);
	phy_reg = fp_free_phy_reg[fp_free_phy_reg_count - 1];
	fp_free_phy_reg_count--;
	core->incRegFileIntCount();
	thread->incRegFileIntCount();
	assert(!fp_phy_reg[phy_reg].busy);
	assert(!fp_phy_reg[phy_reg].pending);
	return phy_reg;
}


int RegisterFile::RequestXMMRegister()
{
	// Local variable
	int phy_reg;

	// Obtain a register from the free list
	assert(xmm_free_phy_reg_count > 0);
	phy_reg = xmm_free_phy_reg[xmm_free_phy_reg_count - 1];
	xmm_free_phy_reg_count--;
	core->incRegFileIntCount();
	thread->incRegFileIntCount();
	assert(!xmm_phy_reg[phy_reg].busy);
	assert(!xmm_phy_reg[phy_reg].pending);
	return phy_reg;
}


bool RegisterFile::CanRename(Uop &uop)
{
	// Detect negative cases. FIXME
	//assert(uop->thread == self);
	if (kind == KindPrivate)
	{
		if (thread->getRegFileIntCount() + uop.getPhyRegIntOdepCount() > int_local_size)
			return false;
		if (thread->getRegFileFpCount() + uop.getPhyRegFpOdepCount() > fp_local_size)
			return false;
		if (thread->getRegFileXmmCount() + uop.getPhyRegXmmOdepCount() > xmm_local_size)
			return false;
	}
	else
	{
		if (core->getRegFileIntCount() + uop.getPhyRegIntOdepCount() > int_local_size)
			return false;
		if (core->getRegFileFpCount() + uop.getPhyRegFpOdepCount() > fp_local_size)
			return false;
		if (core->getRegFileXmmCount() + uop.getPhyRegXmmOdepCount() > xmm_local_size)
			return false;
	}

	// Uop can be renamed.
	return true;
}


void RegisterFile::Rename(Uop &uop)
{
	// Local variable
	int local_reg, stack_reg, phy_reg, ophy_reg;

	// Checks FIXME
	//assert(uop->thread == self);

	// Update floating-point top of stack
	if (uop.getUinst()->getOpcode() == UInstFpPop)
	{
		// Pop floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 1) % 8;
	}
	else if (uop.getUinst()->getOpcode() == UInstFpPush)
	{
		// Push floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 7) % 8;
	}

	// Rename input int/FP/XMM registers
	for (int dep = 0; dep < UInstMaxIDeps; dep++)
	{
		local_reg = (int)uop.getUinst()->getIDep(dep);
		if (local_reg >= UInstDepIntFirst && local_reg <= UInstDepIntLast)
		{
			phy_reg = int_rat[local_reg - UInstDepIntFirst];
			uop.setPhyRegIdep(dep, phy_reg);
			thread->incRatIntReads();
		}
		else if (local_reg >= UInstDepFpFirst && local_reg <= UInstDepFpLast)
		{
			// Convert to top-of-stack relative
			stack_reg = (local_reg - UInstDepFpFirst + fp_top_of_stack) % 8
					+ UInstDepFpFirst;
			assert(stack_reg >= UInstDepFpFirst && stack_reg <= UInstDepFpLast);

			// Rename it.
			phy_reg = fp_rat[stack_reg - UInstDepFpFirst];
			uop.setPhyRegIdep(dep, phy_reg);
			thread->incRatFpReads();
		}
		else if (local_reg >= UInstDepXmmFirst && local_reg <= UInstDepXmmLast)
		{
			phy_reg = xmm_rat[local_reg - UInstDepXmmFirst];
			uop.setPhyRegIdep(dep, phy_reg);
			thread->incRatXmmReads();
		}
		else
		{
			uop.setPhyRegIdep(dep, -1);
		}
	}

	// Rename output int/FP/XMM registers (not flags)
	int flag_phy_reg = -1;
	int flag_count = 0;
	for (int dep = 0; dep < UInstMaxODeps; dep++)
	{
		local_reg = (int)uop.getUinst()->getODep(dep);
		if (local_reg >= UInstDepFlagFirst && local_reg <= UInstDepFlagLast)
		{
			// Record a new flag
			flag_count++;
		}
		else if (local_reg >= UInstDepIntFirst && local_reg <= UInstDepIntLast)
		{
			// Reclaim a free integer register
			phy_reg = RequestIntRegister();
			int_phy_reg[phy_reg].busy++;
			int_phy_reg[phy_reg].pending = 1;
			ophy_reg = int_rat[local_reg - UInstDepIntFirst];
			if (flag_phy_reg < 0)
				flag_phy_reg = phy_reg;

			// Allocate it
			uop.setPhyRegOdep(dep, phy_reg);
			uop.setPhyRegOOdep(dep, ophy_reg);
			int_rat[local_reg - UInstDepIntFirst] = phy_reg;
			thread->incRatIntWrites();
		}
		else if (local_reg >= UInstDepFpFirst && local_reg <= UInstDepFpLast)
		{
			// Convert to top-of-stack relative
			stack_reg = (local_reg - UInstDepFpFirst + fp_top_of_stack) % 8
					+ UInstDepFpFirst;
			assert(stack_reg >= UInstDepFpFirst && stack_reg <= UInstDepFpLast);

			// Reclaim a free FP register
			phy_reg = RequestFPRegister();
			fp_phy_reg[phy_reg].busy++;
			fp_phy_reg[phy_reg].pending = 1;
			ophy_reg = fp_rat[stack_reg - UInstDepFpFirst];

			// Allocate it
			uop.setPhyRegOdep(dep, phy_reg);
			uop.setPhyRegOOdep(dep, ophy_reg);
			fp_rat[stack_reg - UInstDepFpFirst] = phy_reg;
			thread->incRatFpWrites();
		}
		else if (local_reg >= UInstDepXmmFirst && local_reg <= UInstDepXmmLast)
		{
			// Reclaim a free xmm register
			phy_reg = RequestXMMRegister();
			xmm_phy_reg[phy_reg].busy++;
			xmm_phy_reg[phy_reg].pending = 1;
			ophy_reg = xmm_rat[local_reg - UInstDepXmmFirst];

			// Allocate it
			uop.setPhyRegOdep(dep, phy_reg);
			uop.setPhyRegOOdep(dep, ophy_reg);
			xmm_rat[local_reg - UInstDepXmmFirst] = phy_reg;
			thread->incRatXmmWrites();
		}
		else
		{
			// Not a valid output dependence
			uop.setPhyRegOdep(dep, -1);
			uop.setPhyRegOOdep(dep, -1);
		}
	}

	// Rename flags
	if (flag_count > 0) {
		if (flag_phy_reg < 0)
			flag_phy_reg = RequestIntRegister();
		for (int dep = 0; dep < UInstMaxODeps; dep++)
		{
			local_reg = (int)uop.getUinst()->getODep(dep);
			if (!(local_reg >= UInstDepFlagFirst && local_reg <= UInstDepFlagLast))
				continue;
			int_phy_reg[flag_phy_reg].busy++;
			int_phy_reg[flag_phy_reg].pending = 1;
			ophy_reg = int_rat[local_reg - UInstDepIntFirst];
			uop.setPhyRegOdep(dep, flag_phy_reg);
			uop.setPhyRegOOdep(dep, ophy_reg);
			int_rat[local_reg - UInstDepFlagFirst] = flag_phy_reg;
		}
	}
}

}
