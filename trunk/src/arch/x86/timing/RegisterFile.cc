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

const int RegisterFile::MinINTSize = Uinst::DepIntCount + Uinst::MaxODeps;
const int RegisterFile::MinFPSize = Uinst::DepFpCount + Uinst::MaxODeps;
const int RegisterFile::MinXMMSize = Uinst::DepXmmCount + Uinst::MaxODeps;

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


RegisterFile::RegisterFile(Thread *thread) :
		thread(thread)
{
	// Core that the register file belongs to
	core = thread->getCore();

	// Integer register file
	int_phy_reg_count = int_local_size;
	int_phy_reg = misc::new_unique_array<PhysicalRegister>(int_local_size);

	// Free list
	int_free_phy_reg_count = int_local_size;
	int_free_phy_reg = misc::new_unique_array<int>(int_local_size);
	for (int physical_register = 0; physical_register < int_local_size; physical_register++)
		int_free_phy_reg[physical_register] = physical_register;

	// Floating-point register file
	fp_phy_reg_count = fp_local_size;
	fp_phy_reg = misc::new_unique_array<PhysicalRegister>(fp_local_size);

	// Free list
	fp_free_phy_reg_count = fp_local_size;
	fp_free_phy_reg = misc::new_unique_array<int>(fp_local_size);

	// Initialize free list
	for (int physical_register = 0; physical_register < fp_local_size; physical_register++)
		fp_free_phy_reg[physical_register] = physical_register;

	// XMM register file
	xmm_phy_reg_count = xmm_local_size;
	xmm_phy_reg = misc::new_unique_array<PhysicalRegister>(xmm_local_size);

	// Free list
	xmm_free_phy_reg_count = xmm_local_size;
	xmm_free_phy_reg = misc::new_unique_array<int>(xmm_local_size);

	// Initialize free list
	for (int physical_register = 0; physical_register < xmm_local_size; physical_register++)
		xmm_free_phy_reg[physical_register] = physical_register;
	
	// Initial mappings for the integer register file. Map each logical
	// register to a new physical register, and map all flags to the first
	// allocated physical register.
	int flag_physical_register = -1;
	for (int dep = 0; dep < Uinst::DepIntCount; dep++)
	{
		int physical_register;
		int dependency = dep + Uinst::DepIntFirst;
		if (dependency >= Uinst::DepFlagFirst &&
				dependency <= Uinst::DepFlagLast)
		{
			assert(flag_physical_register >= 0);
			physical_register = flag_physical_register;
		}
		else
		{
			physical_register = RequestIntRegister();
			flag_physical_register = physical_register;
		}
		int_phy_reg[physical_register].busy++;
		int_rat[dep] = physical_register;
	}

	// Initial mapping for floating-point registers.
	for (int dep = 0; dep < Uinst::DepFpCount; dep++)
	{
		int physical_register = RequestFPRegister();
		fp_phy_reg[physical_register].busy++;
		fp_rat[dep] = physical_register;
	}

	// Initial mapping for xmm registers.
	for (int dep = 0; dep < Uinst::DepXmmCount; dep++)
	{
		int physical_register = RequestXMMRegister();
		xmm_phy_reg[physical_register].busy++;
		xmm_rat[dep] = physical_register;
	}
}


void RegisterFile::ParseConfiguration(misc::IniFile *ini_file)
{
	std::string section = "Queues";
	kind = (Kind) ini_file->ReadEnum(section, "RfKind",
			KindMap, KindPrivate);
	int_size = ini_file->ReadInt(section, "RfIntSize", 80);
	fp_size = ini_file->ReadInt(section, "RfFpSize", 40);
	xmm_size = ini_file->ReadInt(section, "RfXmmSize", 40);

	int num_threads = ini_file->ReadInt("General", "Threads", 1);

	if (int_size < MinINTSize)
		throw Error(misc::fmt("rf_int_size must be at least %d", MinINTSize));
	if (fp_size < MinFPSize)
		throw Error(misc::fmt("rf_fp_size must be at least %d", MinFPSize));
	if (xmm_size < MinXMMSize)
		throw Error(misc::fmt("rf_xmm_size must be at least %d", MinXMMSize));

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


int RegisterFile::RequestIntRegister()
{
	// Local variable
	int physical_register;

	// Obtain a register from the free list
	assert(int_free_phy_reg_count > 0);
	physical_register = int_free_phy_reg[int_free_phy_reg_count - 1];
	int_free_phy_reg_count--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!int_phy_reg[physical_register].busy);
	assert(!int_phy_reg[physical_register].pending);
	return physical_register;
}


int RegisterFile::RequestFPRegister()
{
	// Local variable
	int physical_register;

	// Obtain a register from the free list
	assert(fp_free_phy_reg_count > 0);
	physical_register = fp_free_phy_reg[fp_free_phy_reg_count - 1];
	fp_free_phy_reg_count--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!fp_phy_reg[physical_register].busy);
	assert(!fp_phy_reg[physical_register].pending);
	return physical_register;
}


int RegisterFile::RequestXMMRegister()
{
	// Local variable
	int physical_register;

	// Obtain a register from the free list
	assert(xmm_free_phy_reg_count > 0);
	physical_register = xmm_free_phy_reg[xmm_free_phy_reg_count - 1];
	xmm_free_phy_reg_count--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!xmm_phy_reg[physical_register].busy);
	assert(!xmm_phy_reg[physical_register].pending);
	return physical_register;
}


bool RegisterFile::canRename(Uop *uop)
{
	// Detect negative cases
	if (kind == KindPrivate)
	{
		// Not enough integer registers
		if (thread->getNumIntegerRegistersOccupied() +
				uop->getPhyIntOdepCount() > int_local_size)
			return false;

		// Not enough floating-point registers
		if (thread->getNumFloatPointRegistersOccupied() +
				uop->getPhyFpOdepCount() > fp_local_size)
			return false;

		// Not enough XMM registers
		if (thread->getNumXmmRegistersOccupied() +
				uop->getPhyXmmOdepCount() > xmm_local_size)
			return false;
	}
	else
	{
		// Not enough integer registers
		if (core->getNumIntegerRegistersOccupied() +
				uop->getPhyIntOdepCount() > int_local_size)
			return false;

		// Not enough floating-point registers
		if (core->getNumFloatPointRegistersOccupied() +
				uop->getPhyFpOdepCount() > fp_local_size)
			return false;

		// Not enough XMM registers
		if (core->getNumXmmRegistersOccupied() +
				uop->getPhyXmmOdepCount() > xmm_local_size)
			return false;
	}

	// Uop can be renamed
	return true;
}


void RegisterFile::Rename(Uop *uop)
{
	// Local variable
	int logical_register, stack_reg, physical_register, ophy_reg;

	// Checks FIXME
	//// assert(uop->thread == self);

	// Update floating-point top of stack
	if (uop->getUinst()->getOpcode() == Uinst::OpcodeFpPop)
	{
		// Pop floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 1) % 8;
	}
	else if (uop->getUinst()->getOpcode() == Uinst::OpcodeFpPush)
	{
		// Push floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 7) % 8;
	}

	// Rename input int/FP/XMM registers
	for (int dep = 0; dep < Uinst::MaxIDeps; dep++)
	{
		logical_register = (int) uop->getUinst()->getIDep(dep);
		if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast)
		{
			physical_register = int_rat[logical_register - Uinst::DepIntFirst];
			uop->setPhyRegIdep(dep, physical_register);
			thread->incRatIntReads();
		}
		else if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast)
		{
			// Convert to top-of-stack relative
			stack_reg = (logical_register - Uinst::DepFpFirst + fp_top_of_stack) % 8
					+ Uinst::DepFpFirst;
			assert(stack_reg >= Uinst::DepFpFirst && stack_reg <= Uinst::DepFpLast);

			// Rename it.
			physical_register = fp_rat[stack_reg - Uinst::DepFpFirst];
			uop->setPhyRegIdep(dep, physical_register);
			thread->incRatFpReads();
		}
		else if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast)
		{
			physical_register = xmm_rat[logical_register - Uinst::DepXmmFirst];
			uop->setPhyRegIdep(dep, physical_register);
			thread->incRatXmmReads();
		}
		else
		{
			uop->setPhyRegIdep(dep, -1);
		}
	}

	// Rename output int/FP/XMM registers (not flags)
	int flag_physical_register = -1;
	int flag_count = 0;
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		logical_register = (int)uop->getUinst()->getODep(dep);
		if (logical_register >= Uinst::DepFlagFirst && logical_register <= Uinst::DepFlagLast)
		{
			// Record a new flag
			flag_count++;
		}
		else if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast)
		{
			// Reclaim a free integer register
			physical_register = RequestIntRegister();
			int_phy_reg[physical_register].busy++;
			int_phy_reg[physical_register].pending = 1;
			ophy_reg = int_rat[logical_register - Uinst::DepIntFirst];
			if (flag_physical_register < 0)
				flag_physical_register = physical_register;

			// Allocate it
			uop->setPhyRegOdep(dep, physical_register);
			uop->setPhyRegOOdep(dep, ophy_reg);
			int_rat[logical_register - Uinst::DepIntFirst] = physical_register;
			thread->incRatIntWrites();
		}
		else if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast)
		{
			// Convert to top-of-stack relative
			stack_reg = (logical_register - Uinst::DepFpFirst + fp_top_of_stack) % 8
					+ Uinst::DepFpFirst;
			assert(stack_reg >= Uinst::DepFpFirst && stack_reg <= Uinst::DepFpLast);

			// Reclaim a free FP register
			physical_register = RequestFPRegister();
			fp_phy_reg[physical_register].busy++;
			fp_phy_reg[physical_register].pending = 1;
			ophy_reg = fp_rat[stack_reg - Uinst::DepFpFirst];

			// Allocate it
			uop->setPhyRegOdep(dep, physical_register);
			uop->setPhyRegOOdep(dep, ophy_reg);
			fp_rat[stack_reg - Uinst::DepFpFirst] = physical_register;
			thread->incRatFpWrites();
		}
		else if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast)
		{
			// Reclaim a free xmm register
			physical_register = RequestXMMRegister();
			xmm_phy_reg[physical_register].busy++;
			xmm_phy_reg[physical_register].pending = 1;
			ophy_reg = xmm_rat[logical_register - Uinst::DepXmmFirst];

			// Allocate it
			uop->setPhyRegOdep(dep, physical_register);
			uop->setPhyRegOOdep(dep, ophy_reg);
			xmm_rat[logical_register - Uinst::DepXmmFirst] = physical_register;
			thread->incRatXmmWrites();
		}
		else
		{
			// Not a valid output dependence
			uop->setPhyRegOdep(dep, -1);
			uop->setPhyRegOOdep(dep, -1);
		}
	}

	// Rename flags
	if (flag_count > 0) {
		if (flag_physical_register < 0)
			flag_physical_register = RequestIntRegister();
		for (int dep = 0; dep < Uinst::MaxODeps; dep++)
		{
			logical_register = (int)uop->getUinst()->getODep(dep);
			if (!(logical_register >= Uinst::DepFlagFirst && logical_register <= Uinst::DepFlagLast))
				continue;
			int_phy_reg[flag_physical_register].busy++;
			int_phy_reg[flag_physical_register].pending = 1;
			ophy_reg = int_rat[logical_register - Uinst::DepIntFirst];
			uop->setPhyRegOdep(dep, flag_physical_register);
			uop->setPhyRegOOdep(dep, ophy_reg);
			int_rat[logical_register - Uinst::DepFlagFirst] = flag_physical_register;
		}
	}
}


bool RegisterFile::isUopReady(Uop *uop)
{
	for (int dep = 0; dep < Uinst::MaxIDeps; dep++)
	{
		int logical_register = uop->getUinst()->getIDep(dep);
		int physical_register = uop->getPhyRegIdep(dep);
		if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast
				&& int_phy_reg[physical_register].pending)
			return false;
		if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast
				&& fp_phy_reg[physical_register].pending)
			return false;
		if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast
				&& xmm_phy_reg[physical_register].pending)
			return false;
	}
	return true;
}


void RegisterFile::WriteUop(Uop *uop)
{
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		int logical_register = uop->getUinst()->getODep(dep);
		int physical_register = uop->getPhyRegOdep(dep);
		if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast)
			int_phy_reg[physical_register].pending = 0;
		else if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast)
			fp_phy_reg[physical_register].pending = 0;
		else if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast)
			xmm_phy_reg[physical_register].pending = 0;
	}
}


void RegisterFile::UndoUop(Uop *uop)
{
	// Undo mappings in reverse order, in case an instruction has a
	// duplicated output dependence.
	assert(uop->speculative_mode);
	for (int dep = Uinst::MaxODeps - 1; dep >= 0; dep--)
	{
		int logical_register = uop->getUinst()->getODep(dep);
		int physical_register = uop->getPhyRegOdep(dep);
		int ophy_reg = uop->getPhyRegOOdep(dep);
		if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast)
		{
			// Decrease busy counter and free if 0.
			assert(int_phy_reg[physical_register].busy > 0);
			assert(!int_phy_reg[physical_register].pending);
			int_phy_reg[physical_register].busy--;
			if (!int_phy_reg[physical_register].busy)
			{
				assert(int_free_phy_reg_count < int_local_size);
				assert(core->getNumIntegerRegistersOccupied() > 0 && thread->getNumIntegerRegistersOccupied() > 0);
				int_free_phy_reg[int_free_phy_reg_count] = physical_register;
				int_free_phy_reg_count++;
				core->decNumIntegerRegistersOccupied();
				thread->decNumIntegerRegistersOccupied();
			}

			// Return to previous mapping
			int_rat[logical_register - Uinst::DepIntFirst] = ophy_reg;
			assert(int_phy_reg[ophy_reg].busy);
		}
		else if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast)
		{
			// Convert to top-of-stack relative
			int stack_reg = (logical_register - Uinst::DepFpFirst
					+ fp_top_of_stack) % 8
					+ Uinst::DepFpFirst;
			assert(stack_reg >= Uinst::DepFpFirst &&
					stack_reg <= Uinst::DepFpLast);

			// Decrease busy counter and free if 0.
			assert(fp_phy_reg[physical_register].busy > 0);
			assert(!fp_phy_reg[physical_register].pending);
			fp_phy_reg[physical_register].busy--;
			if (!fp_phy_reg[physical_register].busy)
			{
				assert(fp_free_phy_reg_count < fp_local_size);
				assert(core->getNumFloatPointRegistersOccupied() > 0 && thread->getNumFloatPointRegistersOccupied() > 0);
				fp_free_phy_reg[fp_free_phy_reg_count] = physical_register;
				fp_free_phy_reg_count++;
				core->decNumFloatPointRegistersOccupied();
				thread->decNumFloatPointRegistersOccupied();
			}

			// Return to previous mapping
			fp_rat[stack_reg - Uinst::DepFpFirst] = ophy_reg;
			assert(fp_phy_reg[ophy_reg].busy);
		}
		else if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast)
		{
			// Decrease busy counter and free if 0.
			assert(xmm_phy_reg[physical_register].busy > 0);
			assert(!xmm_phy_reg[physical_register].pending);
			xmm_phy_reg[physical_register].busy--;
			if (!xmm_phy_reg[physical_register].busy)
			{
				assert(xmm_free_phy_reg_count < xmm_local_size);
				assert(core->getNumXmmRegistersOccupied() > 0 && thread->getNumXmmRegistersOccupied() > 0);
				xmm_free_phy_reg[xmm_free_phy_reg_count] = physical_register;
				xmm_free_phy_reg_count++;
				core->decNumXmmRegistersOccupied();
				thread->decNumXmmRegistersOccupied();
			}

			// Return to previous mapping
			xmm_rat[logical_register - Uinst::DepXmmFirst] = ophy_reg;
			assert(xmm_phy_reg[ophy_reg].busy);
		}
		else
		{
			// Not a valid dependence.
			assert(physical_register == -1);
			assert(ophy_reg == -1);
		}
	}

	// Undo modification in floating-point top of stack
	if (uop->getUinst()->getOpcode() == Uinst::OpcodeFpPop)
	{
		// Inverse-pop floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 7) % 8;
	}
	else if (uop->getUinst()->getOpcode() == Uinst::OpcodeFpPush)
	{
		// Inverse-push floating-point stack
		fp_top_of_stack = (fp_top_of_stack + 1) % 8;
	}
}


void RegisterFile::CommitUop(Uop *uop)
{
	assert(!uop->speculative_mode);
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		int logical_register = (int)uop->getUinst()->getODep(dep);
		int physical_register = uop->getPhyRegOdep(dep);
		int ophy_reg = uop->getPhyRegOOdep(dep);

		if (logical_register >= Uinst::DepIntFirst &&
				logical_register <= Uinst::DepIntLast)
		{
			// Decrease counter of previous mapping and free if 0.
			assert(int_phy_reg[ophy_reg].busy > 0);
			int_phy_reg[ophy_reg].busy--;
			if (!int_phy_reg[ophy_reg].busy)
			{
				assert(!int_phy_reg[ophy_reg].pending);
				assert(int_free_phy_reg_count < int_local_size);
				assert(core->getNumIntegerRegistersOccupied() > 0 && thread->getNumIntegerRegistersOccupied() > 0);
				int_free_phy_reg[int_free_phy_reg_count] = ophy_reg;
				int_free_phy_reg_count++;
				core->decNumIntegerRegistersOccupied();
				thread->decNumIntegerRegistersOccupied();
			}
		}
		else if (logical_register >= Uinst::DepFpFirst &&
				logical_register <= Uinst::DepFpLast)
		{
			// Decrease counter of previous mapping and free if 0.
			assert(fp_phy_reg[ophy_reg].busy > 0);
			fp_phy_reg[ophy_reg].busy--;
			if (!fp_phy_reg[ophy_reg].busy)
			{
				assert(!fp_phy_reg[ophy_reg].pending);
				assert(fp_free_phy_reg_count < fp_local_size);
				assert(core->getNumFloatPointRegistersOccupied() > 0 && thread->getNumFloatPointRegistersOccupied() > 0);
				fp_free_phy_reg[fp_free_phy_reg_count] = ophy_reg;
				fp_free_phy_reg_count++;
				core->decNumFloatPointRegistersOccupied();
				thread->decNumFloatPointRegistersOccupied();
			}
		}
		else if (logical_register >= Uinst::DepXmmFirst &&
				logical_register <= Uinst::DepXmmLast)
		{
			// Decrease counter of previous mapping and free if 0.
			assert(xmm_phy_reg[ophy_reg].busy > 0);
			xmm_phy_reg[ophy_reg].busy--;
			if (!xmm_phy_reg[ophy_reg].busy)
			{
				assert(!xmm_phy_reg[ophy_reg].pending);
				assert(xmm_free_phy_reg_count < xmm_local_size);
				assert(core->getNumXmmRegistersOccupied() > 0 && thread->getNumXmmRegistersOccupied() > 0);
				xmm_free_phy_reg[xmm_free_phy_reg_count] = ophy_reg;
				xmm_free_phy_reg_count++;
				core->decNumXmmRegistersOccupied();
				thread->decNumXmmRegistersOccupied();
			}
		}
		else
		{
			// Not a valid dependence.
			assert(physical_register == -1);
			assert(ophy_reg == -1);
		}
	}
}


void RegisterFile::CheckRegisterFile()
{
	// Check that all registers in the free list are actually free.
	int physical_register;
	for (int i = 0; i < int_free_phy_reg_count; i++)
	{
		physical_register = int_free_phy_reg[i];
		assert(!int_phy_reg[physical_register].busy);
		assert(!int_phy_reg[physical_register].pending);
	}
	for (int i = 0; i < fp_free_phy_reg_count; i++)
	{
		physical_register = fp_free_phy_reg[i];
		assert(!fp_phy_reg[physical_register].busy);
		assert(!fp_phy_reg[physical_register].pending);
	}
	for (int i = 0; i < xmm_free_phy_reg_count; i++)
	{
		physical_register = xmm_free_phy_reg[i];
		assert(!xmm_phy_reg[physical_register].busy);
		assert(!xmm_phy_reg[physical_register].pending);
	}

	// Check that all mapped registers are busy
	for (int logical_register = Uinst::DepIntFirst; logical_register <= Uinst::DepIntLast; logical_register++)
	{
		physical_register = int_rat[logical_register - Uinst::DepIntFirst];
		assert(int_phy_reg[physical_register].busy);
	}
	for (int logical_register = Uinst::DepFpFirst; logical_register <= Uinst::DepFpLast; logical_register++)
	{
		physical_register = fp_rat[logical_register - Uinst::DepFpFirst];
		assert(fp_phy_reg[physical_register].busy);
	}
	for (int logical_register = Uinst::DepXmmFirst; logical_register <= Uinst::DepXmmLast; logical_register++)
	{
		physical_register = xmm_rat[logical_register - Uinst::DepXmmFirst];
		assert(xmm_phy_reg[physical_register].busy);
	}

	// Check that all destination and previous destination
	// registers of instructions in the rob are busy
	// ROB related FIXME
	/*
	for (int i = 0; i < self->rob_count; i++)
	{
		uop = X86GetROBEntry(self, i);
		assert(uop);
		for (int dep = 0; dep < UInstMaxODeps; dep++)
		{
			logical_register = uop->uinst->odep[dep];
			physical_register = uop->ph_odep[dep];
			ophy_reg = uop->ph_oodep[dep];
			if (logical_register >= Uinst::DepIntFirst && logical_register <= Uinst::DepIntLast)
			{
				assert(int_phy_reg[physical_register].busy);
				assert(int_phy_reg[ophy_reg].busy);
			}
			else if (logical_register >= Uinst::DepFpFirst && logical_register <= Uinst::DepFpLast)
			{
				assert(fp_phy_reg[physical_register].busy);
				assert(fp_phy_reg[ophy_reg].busy);
			}
			else if (logical_register >= Uinst::DepXmmFirst && logical_register <= Uinst::DepXmmLast)
			{
				assert(xmm_phy_reg[physical_register].busy);
				assert(xmm_phy_reg[ophy_reg].busy);
			}
			else
			{
				assert(physical_register == -1);
				assert(ophy_reg == -1);
			}
		}
	}
	*/
}


}
