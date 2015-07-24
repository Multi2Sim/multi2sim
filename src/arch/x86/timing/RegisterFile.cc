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

const int RegisterFile::MinIntegerSize = Uinst::DepIntCount + Uinst::MaxODeps;
const int RegisterFile::MinFloatingPointSize = Uinst::DepFpCount + Uinst::MaxODeps;
const int RegisterFile::MinXmmSize = Uinst::DepXmmCount + Uinst::MaxODeps;

misc::StringMap RegisterFile::KindMap =
{
	{"Shared", KindShared},
	{"Private", KindPrivate}
};

RegisterFile::Kind RegisterFile::kind;
int RegisterFile::integer_size;
int RegisterFile::floating_point_size;
int RegisterFile::xmm_size;
int RegisterFile::integer_local_size;
int RegisterFile::floating_point_local_size;
int RegisterFile::xmm_local_size;


RegisterFile::RegisterFile(Thread *thread) :
		thread(thread)
{
	// Core that the register file belongs to
	core = thread->getCore();

	// Integer register file
	integer_registers = misc::new_unique_array<PhysicalRegister>(integer_local_size);

	// Free list
	num_free_integer_registers = integer_local_size;
	free_integer_registers = misc::new_unique_array<int>(integer_local_size);
	for (int physical_register = 0;
			physical_register < integer_local_size;
			physical_register++)
		free_integer_registers[physical_register] = physical_register;

	// Floating-point register file
	floating_point_registers = misc::new_unique_array<PhysicalRegister>(floating_point_local_size);

	// Free list
	num_free_floating_point_registers = floating_point_local_size;
	free_floating_point_registers = misc::new_unique_array<int>(floating_point_local_size);

	// Initialize free list
	for (int physical_register = 0;
			physical_register < floating_point_local_size;
			physical_register++)
		free_floating_point_registers[physical_register] = physical_register;

	// XMM register file
	xmm_registers = misc::new_unique_array<PhysicalRegister>(xmm_local_size);

	// Free list
	num_free_xmm_registers = xmm_local_size;
	free_xmm_registers = misc::new_unique_array<int>(xmm_local_size);

	// Initialize free list
	for (int physical_register = 0;
			physical_register < xmm_local_size;
			physical_register++)
		free_xmm_registers[physical_register] = physical_register;
	
	// Initial mappings for the integer register file. Map each logical
	// register to a new physical register, and map all flags to the first
	// allocated physical register.
	int flag_physical_register = -1;
	for (int dep = 0; dep < Uinst::DepIntCount; dep++)
	{
		int physical_register;
		int dependency = dep + Uinst::DepIntFirst;
		if (Uinst::isFlagDependency(dependency))
		{
			assert(flag_physical_register >= 0);
			physical_register = flag_physical_register;
		}
		else
		{
			physical_register = RequestIntegerRegister();
			flag_physical_register = physical_register;
		}
		integer_registers[physical_register].busy++;
		integer_rat[dep] = physical_register;
	}

	// Initial mapping for floating-point registers.
	for (int dep = 0; dep < Uinst::DepFpCount; dep++)
	{
		int physical_register = RequestFloatingPointRegister();
		floating_point_registers[physical_register].busy++;
		floating_point_rat[dep] = physical_register;
	}

	// Initial mapping for xmm registers.
	for (int dep = 0; dep < Uinst::DepXmmCount; dep++)
	{
		int physical_register = RequestXmmRegister();
		xmm_registers[physical_register].busy++;
		xmm_rat[dep] = physical_register;
	}
}


void RegisterFile::ParseConfiguration(misc::IniFile *ini_file)
{
	// Section [Queues]
	std::string section = "Queues";

	// Read parameters
	kind = (Kind) ini_file->ReadEnum(section, "RfKind",
			KindMap, KindPrivate);
	integer_size = ini_file->ReadInt(section, "RfIntSize", 80);
	floating_point_size = ini_file->ReadInt(section, "RfFpSize", 40);
	xmm_size = ini_file->ReadInt(section, "RfXmmSize", 40);

	// Get number of threads
	int num_threads = ini_file->ReadInt("General", "Threads", 1);

	// Check valid sizes
	if (integer_size < MinIntegerSize)
		throw Error(misc::fmt("rf_int_size must be at least %d", MinIntegerSize));
	if (floating_point_size < MinFloatingPointSize)
		throw Error(misc::fmt("rf_fp_size must be at least %d", MinFloatingPointSize));
	if (xmm_size < MinXmmSize)
		throw Error(misc::fmt("rf_xmm_size must be at least %d", MinXmmSize));

	// Calculate local sizes based on private/shared register file
	if (kind == KindPrivate)
	{
		integer_local_size = integer_size;
		floating_point_local_size = floating_point_size;
		xmm_local_size = xmm_size;
	}
	else
	{
		integer_local_size = integer_size * num_threads;
		floating_point_local_size = floating_point_size * num_threads;
		xmm_local_size = xmm_size * num_threads;
	}
}


int RegisterFile::RequestIntegerRegister()
{
	// Obtain a register from the free list
	assert(num_free_integer_registers > 0);
	int physical_register = free_integer_registers[num_free_integer_registers - 1];
	num_free_integer_registers--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!integer_registers[physical_register].busy);
	assert(!integer_registers[physical_register].pending);
	return physical_register;
}


int RegisterFile::RequestFloatingPointRegister()
{
	// Obtain a register from the free list
	assert(num_free_floating_point_registers > 0);
	int physical_register = free_floating_point_registers[num_free_floating_point_registers - 1];
	num_free_floating_point_registers--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!floating_point_registers[physical_register].busy);
	assert(!floating_point_registers[physical_register].pending);
	return physical_register;
}


int RegisterFile::RequestXmmRegister()
{
	// Obtain a register from the free list
	assert(num_free_xmm_registers > 0);
	int physical_register = free_xmm_registers[num_free_xmm_registers - 1];
	num_free_xmm_registers--;
	core->incNumIntegerRegistersOccupied();
	thread->incNumIntegerRegistersOccupied();
	assert(!xmm_registers[physical_register].busy);
	assert(!xmm_registers[physical_register].pending);
	return physical_register;
}


bool RegisterFile::canRename(Uop *uop)
{
	// Detect negative cases
	if (kind == KindPrivate)
	{
		// Not enough integer registers
		if (thread->getNumIntegerRegistersOccupied()
				+ uop->getNumIntegerOutputs()
				> integer_local_size)
			return false;

		// Not enough floating-point registers
		if (thread->getNumFloatPointRegistersOccupied()
				+ uop->getNumFloatingPointOutputs()
				> floating_point_local_size)
			return false;

		// Not enough XMM registers
		if (thread->getNumXmmRegistersOccupied()
				+ uop->getNumXmmOutputs()
				> xmm_local_size)
			return false;
	}
	else
	{
		// Not enough integer registers
		if (core->getNumIntegerRegistersOccupied()
				+ uop->getNumIntegerOutputs()
				> integer_local_size)
			return false;

		// Not enough floating-point registers
		if (core->getNumFloatPointRegistersOccupied()
				+ uop->getNumFloatingPointOutputs()
				> floating_point_local_size)
			return false;

		// Not enough XMM registers
		if (core->getNumXmmRegistersOccupied()
				+ uop->getNumXmmOutputs()
				> xmm_local_size)
			return false;
	}

	// Uop can be renamed
	return true;
}


void RegisterFile::Rename(Uop *uop)
{
	// Update floating-point top of stack
	if (uop->getOpcode() == Uinst::OpcodeFpPop)
	{
		// Pop floating-point stack
		floating_point_top = (floating_point_top + 1) % 8;
	}
	else if (uop->getOpcode() == Uinst::OpcodeFpPush)
	{
		// Push floating-point stack
		floating_point_top = (floating_point_top + 7) % 8;
	}

	// Rename input int/FP/XMM registers
	for (int dep = 0; dep < Uinst::MaxIDeps; dep++)
	{
		int logical_register = uop->getUinst()->getIDep(dep);
		if (Uinst::isIntegerDependency(logical_register))
		{
			int physical_register = integer_rat[logical_register - Uinst::DepIntFirst];
			uop->setInput(dep, physical_register);
			thread->incRatIntReads();
		}
		else if (Uinst::isFloatingPointDependency(logical_register))
		{
			// Convert to top-of-stack relative
			int stack_register = (logical_register
					- Uinst::DepFpFirst + floating_point_top)
					% 8 + Uinst::DepFpFirst;
			assert(stack_register >= Uinst::DepFpFirst
					&& stack_register <= Uinst::DepFpLast);

			// Rename it.
			int physical_register = floating_point_rat[stack_register - Uinst::DepFpFirst];
			uop->setInput(dep, physical_register);
			thread->incRatFpReads();
		}
		else if (Uinst::isXmmDependency(logical_register))
		{
			int physical_register = xmm_rat[logical_register - Uinst::DepXmmFirst];
			uop->setInput(dep, physical_register);
			thread->incRatXmmReads();
		}
		else
		{
			uop->setInput(dep, -1);
		}
	}

	// Rename output int/FP/XMM registers (not flags)
	int flag_physical_register = -1;
	int flag_count = 0;
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		int logical_register = uop->getUinst()->getODep(dep);
		if (Uinst::isFlagDependency(logical_register))
		{
			// Record a new flag
			flag_count++;
		}
		else if (Uinst::isIntegerDependency(logical_register))
		{
			// Request a free integer register
			int physical_register = RequestIntegerRegister();
			integer_registers[physical_register].busy++;
			integer_registers[physical_register].pending = 1;
			int old_physical_register = integer_rat[logical_register - Uinst::DepIntFirst];
			if (flag_physical_register < 0)
				flag_physical_register = physical_register;

			// Allocate it
			uop->setOutput(dep, physical_register);
			uop->setOldOutput(dep, old_physical_register);
			integer_rat[logical_register - Uinst::DepIntFirst] = physical_register;
			thread->incRatIntWrites();
		}
		else if (Uinst::isFloatingPointDependency(logical_register))
		{
			// Convert to top-of-stack relative
			int stack_register = (logical_register
					- Uinst::DepFpFirst + floating_point_top)
					% 8 + Uinst::DepFpFirst;
			assert(stack_register >= Uinst::DepFpFirst
					&& stack_register <= Uinst::DepFpLast);

			// Request a free floating-point register
			int physical_register = RequestFloatingPointRegister();
			floating_point_registers[physical_register].busy++;
			floating_point_registers[physical_register].pending = 1;
			int old_physical_register = floating_point_rat[stack_register - Uinst::DepFpFirst];

			// Allocate it
			uop->setOutput(dep, physical_register);
			uop->setOldOutput(dep, old_physical_register);
			floating_point_rat[stack_register - Uinst::DepFpFirst] = physical_register;
			thread->incRatFpWrites();
		}
		else if (Uinst::isXmmDependency(logical_register))
		{
			// Request a free XMM register
			int physical_register = RequestXmmRegister();
			xmm_registers[physical_register].busy++;
			xmm_registers[physical_register].pending = 1;
			int old_physical_register = xmm_rat[logical_register - Uinst::DepXmmFirst];

			// Allocate it
			uop->setOutput(dep, physical_register);
			uop->setOldOutput(dep, old_physical_register);
			xmm_rat[logical_register - Uinst::DepXmmFirst] = physical_register;
			thread->incRatXmmWrites();
		}
		else
		{
			// Not a valid output dependence
			uop->setOutput(dep, -1);
			uop->setOldOutput(dep, -1);
		}
	}

	// Rename flags
	if (flag_count > 0)
	{
		// Request flag register
		if (flag_physical_register < 0)
			flag_physical_register = RequestIntegerRegister();

		// Traverse dependencies
		for (int dep = 0; dep < Uinst::MaxODeps; dep++)
		{
			int logical_register = uop->getUinst()->getODep(dep);
			if (!(logical_register >= Uinst::DepFlagFirst
					&& logical_register <= Uinst::DepFlagLast))
				continue;
			integer_registers[flag_physical_register].busy++;
			integer_registers[flag_physical_register].pending = 1;
			int old_physical_register = integer_rat[logical_register - Uinst::DepIntFirst];
			uop->setOutput(dep, flag_physical_register);
			uop->setOldOutput(dep, old_physical_register);
			integer_rat[logical_register - Uinst::DepFlagFirst] = flag_physical_register;
		}
	}
}


bool RegisterFile::isUopReady(Uop *uop)
{
	// If uop is marked as ready, it means that we verified that it is
	// ready before. The uop ready state can never change from true to
	// false.
	if (uop->ready)
		return true;
	
	// Traverse dependencies
	for (int dep = 0; dep < Uinst::MaxIDeps; dep++)
	{
		// Get dependencies
		int logical_register = uop->getUinst()->getIDep(dep);
		int physical_register = uop->getInput(dep);

		// Integer dependency
		if (Uinst::isIntegerDependency(logical_register)
				&& integer_registers[physical_register].pending)
			return false;

		// Floating-point dependency
		if (Uinst::isFloatingPointDependency(logical_register)
				&& floating_point_registers[physical_register].pending)
			return false;

		// XMM dependency
		if (Uinst::isXmmDependency(logical_register)
				&& xmm_registers[physical_register].pending)
			return false;
	}

	// At this point, we found that the uop is ready. Save this information
	// in the 'ready' field of the uop to avoid having to check in the
	// future.
	uop->ready = true;
	return true;
}


void RegisterFile::WriteUop(Uop *uop)
{
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		int logical_register = uop->getUinst()->getODep(dep);
		int physical_register = uop->getOutput(dep);
		if (Uinst::isIntegerDependency(logical_register))
			integer_registers[physical_register].pending = 0;
		else if (Uinst::isFloatingPointDependency(logical_register))
			floating_point_registers[physical_register].pending = 0;
		else if (Uinst::isXmmDependency(logical_register))
			xmm_registers[physical_register].pending = 0;
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
		int physical_register = uop->getOutput(dep);
		int old_physical_register = uop->getOldOutput(dep);
		if (Uinst::isIntegerDependency(logical_register))
		{
			// Decrease busy counter and free if 0.
			assert(integer_registers[physical_register].busy > 0);
			assert(!integer_registers[physical_register].pending);
			integer_registers[physical_register].busy--;
			if (!integer_registers[physical_register].busy)
			{
				assert(num_free_integer_registers < integer_local_size);
				assert(core->getNumIntegerRegistersOccupied() > 0
						&& thread->getNumIntegerRegistersOccupied() > 0);
				free_integer_registers[num_free_integer_registers] = physical_register;
				num_free_integer_registers++;
				core->decNumIntegerRegistersOccupied();
				thread->decNumIntegerRegistersOccupied();
			}

			// Return to previous mapping
			integer_rat[logical_register - Uinst::DepIntFirst] = old_physical_register;
			assert(integer_registers[old_physical_register].busy);
		}
		else if (Uinst::isFloatingPointDependency(logical_register))
		{
			// Convert to top-of-stack relative
			int stack_register = (logical_register - Uinst::DepFpFirst
					+ floating_point_top) % 8
					+ Uinst::DepFpFirst;
			assert(stack_register >= Uinst::DepFpFirst &&
					stack_register <= Uinst::DepFpLast);

			// Decrease busy counter and free if 0.
			assert(floating_point_registers[physical_register].busy > 0);
			assert(!floating_point_registers[physical_register].pending);
			floating_point_registers[physical_register].busy--;
			if (!floating_point_registers[physical_register].busy)
			{
				assert(num_free_floating_point_registers < floating_point_local_size);
				assert(core->getNumFloatPointRegistersOccupied() > 0
						&& thread->getNumFloatPointRegistersOccupied() > 0);
				free_floating_point_registers[num_free_floating_point_registers] = physical_register;
				num_free_floating_point_registers++;
				core->decNumFloatPointRegistersOccupied();
				thread->decNumFloatPointRegistersOccupied();
			}

			// Return to previous mapping
			floating_point_rat[stack_register - Uinst::DepFpFirst] = old_physical_register;
			assert(floating_point_registers[old_physical_register].busy);
		}
		else if (Uinst::isXmmDependency(logical_register))
		{
			// Decrease busy counter and free if 0.
			assert(xmm_registers[physical_register].busy > 0);
			assert(!xmm_registers[physical_register].pending);
			xmm_registers[physical_register].busy--;
			if (!xmm_registers[physical_register].busy)
			{
				assert(num_free_xmm_registers < xmm_local_size);
				assert(core->getNumXmmRegistersOccupied() > 0
						&& thread->getNumXmmRegistersOccupied() > 0);
				free_xmm_registers[num_free_xmm_registers] = physical_register;
				num_free_xmm_registers++;
				core->decNumXmmRegistersOccupied();
				thread->decNumXmmRegistersOccupied();
			}

			// Return to previous mapping
			xmm_rat[logical_register - Uinst::DepXmmFirst] = old_physical_register;
			assert(xmm_registers[old_physical_register].busy);
		}
		else
		{
			// Not a valid dependence.
			assert(physical_register == -1);
			assert(old_physical_register == -1);
		}
	}

	// Undo modification in floating-point top of stack
	if (uop->getOpcode() == Uinst::OpcodeFpPop)
	{
		// Inverse-pop floating-point stack
		floating_point_top = (floating_point_top + 7) % 8;
	}
	else if (uop->getOpcode() == Uinst::OpcodeFpPush)
	{
		// Inverse-push floating-point stack
		floating_point_top = (floating_point_top + 1) % 8;
	}
}


void RegisterFile::CommitUop(Uop *uop)
{
	assert(!uop->speculative_mode);
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		int logical_register = (int)uop->getUinst()->getODep(dep);
		int physical_register = uop->getOutput(dep);
		int old_physical_register = uop->getOldOutput(dep);

		if (Uinst::isIntegerDependency(logical_register))
		{
			// Decrease counter of previous mapping and free if 0.
			assert(integer_registers[old_physical_register].busy > 0);
			integer_registers[old_physical_register].busy--;
			if (!integer_registers[old_physical_register].busy)
			{
				assert(!integer_registers[old_physical_register].pending);
				assert(num_free_integer_registers < integer_local_size);
				assert(core->getNumIntegerRegistersOccupied() > 0
						&& thread->getNumIntegerRegistersOccupied() > 0);
				free_integer_registers[num_free_integer_registers] = old_physical_register;
				num_free_integer_registers++;
				core->decNumIntegerRegistersOccupied();
				thread->decNumIntegerRegistersOccupied();
			}
		}
		else if (Uinst::isFloatingPointDependency(logical_register))
		{
			// Decrease counter of previous mapping and free if 0.
			assert(floating_point_registers[old_physical_register].busy > 0);
			floating_point_registers[old_physical_register].busy--;
			if (!floating_point_registers[old_physical_register].busy)
			{
				assert(!floating_point_registers[old_physical_register].pending);
				assert(num_free_floating_point_registers < floating_point_local_size);
				assert(core->getNumFloatPointRegistersOccupied() > 0
						&& thread->getNumFloatPointRegistersOccupied() > 0);
				free_floating_point_registers[num_free_floating_point_registers] = old_physical_register;
				num_free_floating_point_registers++;
				core->decNumFloatPointRegistersOccupied();
				thread->decNumFloatPointRegistersOccupied();
			}
		}
		else if (Uinst::isXmmDependency(logical_register))
		{
			// Decrease counter of previous mapping and free if 0.
			assert(xmm_registers[old_physical_register].busy > 0);
			xmm_registers[old_physical_register].busy--;
			if (!xmm_registers[old_physical_register].busy)
			{
				assert(!xmm_registers[old_physical_register].pending);
				assert(num_free_xmm_registers < xmm_local_size);
				assert(core->getNumXmmRegistersOccupied() > 0
						&& thread->getNumXmmRegistersOccupied() > 0);
				free_xmm_registers[num_free_xmm_registers] = old_physical_register;
				num_free_xmm_registers++;
				core->decNumXmmRegistersOccupied();
				thread->decNumXmmRegistersOccupied();
			}
		}
		else
		{
			// Not a valid dependence.
			assert(physical_register == -1);
			assert(old_physical_register == -1);
		}
	}
}


void RegisterFile::CheckRegisterFile()
{
	// Check that all registers in the free list are actually free.
	for (int i = 0; i < num_free_integer_registers; i++)
	{
		int physical_register = free_integer_registers[i];
		assert(!integer_registers[physical_register].busy);
		assert(!integer_registers[physical_register].pending);
	}
	for (int i = 0; i < num_free_floating_point_registers; i++)
	{
		int physical_register = free_floating_point_registers[i];
		assert(!floating_point_registers[physical_register].busy);
		assert(!floating_point_registers[physical_register].pending);
	}
	for (int i = 0; i < num_free_xmm_registers; i++)
	{
		int physical_register = free_xmm_registers[i];
		assert(!xmm_registers[physical_register].busy);
		assert(!xmm_registers[physical_register].pending);
	}

	// Check that all mapped integer registers are busy
	for (int logical_register = Uinst::DepIntFirst;
			logical_register <= Uinst::DepIntLast;
			logical_register++)
	{
		int physical_register = integer_rat[logical_register - Uinst::DepIntFirst];
		assert(integer_registers[physical_register].busy);
	}

	// Floating-point registers
	for (int logical_register = Uinst::DepFpFirst;
			logical_register <= Uinst::DepFpLast;
			logical_register++)
	{
		int physical_register = floating_point_rat[logical_register - Uinst::DepFpFirst];
		assert(floating_point_registers[physical_register].busy);
	}

	// XMM registers
	for (int logical_register = Uinst::DepXmmFirst;
			logical_register <= Uinst::DepXmmLast;
			logical_register++)
	{
		int physical_register = xmm_rat[logical_register - Uinst::DepXmmFirst];
		assert(xmm_registers[physical_register].busy);
	}
}


}
