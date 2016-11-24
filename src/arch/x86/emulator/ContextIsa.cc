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

#include <cstdarg>
#include <cstring>

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emulator.h"


namespace x86
{

Context::ExecuteInstFn Context::execute_inst_fn[Instruction::OpcodeCount] =
{
		nullptr  // For Inst::OpcodeNone
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) \
		 , &Context::ExecuteInst_##name
#include <arch/x86/disassembler/Instruction.def>
#undef DEFINST
};


void Context::MemoryRead(unsigned int address, int size, void *buffer)
{
	// Speculative mode read
	if (getState(StateSpecMode))
	{
		spec_mem->Read(address, size, (char *) buffer);
		return;
	}

	// Read in regular mode
	memory->Read(address, size, (char *) buffer);
}


void Context::MemoryWrite(unsigned int address, int size, void *buffer)
{
	// Speculative mode write
	if (getState(StateSpecMode))
	{
		spec_mem->Write(address, size, (char *) buffer);
		return;
	}

	// Write in regular mode
	memory->Write(address, size, (char *) buffer);
}



// Macros defined to prevent accidental use of functions that cause unsafe
// execution in speculative mode.
#undef assert
#define memory __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#define assert __COMPILATION_ERROR__


unsigned char Context::LoadRm8()
{
	unsigned char value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + Instruction::RegAl);

	MemoryRead(getEffectiveAddress(), 1, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=0x%x", last_effective_address, value);
	return value;
}

unsigned short Context::LoadRm16()
{
	unsigned short value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + Instruction::RegAx);

	MemoryRead(getEffectiveAddress(), 2, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=0x%x", last_effective_address, value);
	return value;
}

unsigned int Context::LoadRm32()
{
	unsigned int value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + Instruction::RegEax);

	MemoryRead(getEffectiveAddress(), 4, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=0x%x", last_effective_address, value);
	return value;
}

unsigned short Context::LoadR32M16()
{
	unsigned short value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + Instruction::RegEax);

	MemoryRead(getEffectiveAddress(), 2, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=0x%x", last_effective_address, value);
	return value;
}

unsigned long long Context::LoadM64()
{
	unsigned long long value;

	MemoryRead(getEffectiveAddress(), 8, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=0x%llx", last_effective_address, value);
	return value;
}

void Context::StoreRm8(unsigned char value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + Instruction::RegAl, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 1, &value);
	emulator->isa_debug << misc::fmt("  [0x%x] <- 0x%x", last_effective_address, value);
}

void Context::StoreRm16(unsigned short value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + Instruction::RegAx, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 2, &value);
	emulator->isa_debug << misc::fmt("  [0x%x] <- 0x%x", last_effective_address, value);
}

void Context::StoreRm32(unsigned int value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + Instruction::RegEax, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 4, &value);
	emulator->isa_debug << misc::fmt("  [0x%x] <- 0x%x", last_effective_address, value);
}

void Context::StoreM64(unsigned long long value)
{
	MemoryWrite(getEffectiveAddress(), 8, &value);
	emulator->isa_debug << misc::fmt("  [0x%x] <- 0x%llx", last_effective_address, value);
}

unsigned Context::getLinearAddress(unsigned offset)
{
	// No segment override
	if (!inst.getSegment())
		return offset;
	
	// Segment override
	if (inst.getSegment() != Instruction::RegGs)
	{
		throw misc::Panic("Unimplemented segment override");
		return 0;
	}

	// GLibc segment at TLS entry 6
	if (regs.Read(Instruction::RegGs) != 0x33)
	{
		throw misc::Panic(misc::fmt("Linear address for gs = 0x%x",
				regs.Read(Instruction::RegGs)));
		return 0;
	}

	if (!glibc_segment_base)
	{
		throw misc::Panic(misc::fmt("GLibc segment not set "
				"in linear address calculation"));
		return 0;
	}

	// Return address
	return glibc_segment_base + offset;
}


unsigned Context::getEffectiveAddress()
{
	// Check 'modrm_mod' field
	if (inst.getModRmMod() == 3)
		throw Error("Invalid value for 'modrm_mod'");

	// Address
	unsigned address = regs.Read(inst.getEaBase()) +
			regs.Read(inst.getEaIndex()) * inst.getEaScale() +
			inst.getDisp();
	
	// Add segment base
	address = getLinearAddress(address);

	// Record effective address in context. This address is used later in
	// the generation of micro-instructions. We need to record it to avoid
	// calling this function again later, since the source register used to
	// calculate the effective address can be overwritten after the
	// instruction emulation.
	last_effective_address = address;

	return address;
}

unsigned Context::getMoffsAddress()
{
	// Immediate value as effective address
	unsigned address = inst.getImmDWord();

	// Add segment base
	address = getLinearAddress(address);

	return address;
}

void Context::LoadFpu(int index, unsigned char *value)
{
	if (!misc::inRange(index, 0, 7))
		throw Error("Invalid value for 'index'");

	int eff_index = (regs.getFpuTop() + index) % 8;
	if (!regs.isFpuValid(eff_index))
		throw Error("Invalid FPU stack entry");

	memcpy(value, regs.getFpuValue(eff_index), 10);
	if (emulator->isa_debug)
		emulator->isa_debug << misc::fmt("  st(%d)=%g", index,
				Extended::ExtendedToDouble(value));
}


void Context::StoreFpu(int index, unsigned char *value)
{
	// Check valid index
	if (!misc::inRange(index, 0, 7))
		throw Error("Invalid value for 'index'");

	// Get index
	index = (regs.getFpuTop() + index) % 8;
	if (!regs.isFpuValid(index))
		throw Error("Invalid FPU stack entry");

	// Store value
	memcpy(regs.getFpuValue(index), value, 10);
	if (emulator->isa_debug)
		emulator->isa_debug << misc::fmt("  st(%d)<=%g", index,
				Extended::ExtendedToDouble(value));
}


void Context::PushFpu(unsigned char *value)
{
	// Debug
	if (emulator->isa_debug)
		emulator->isa_debug << misc::fmt("  st(0)<=%g (pushed)",
				Extended::ExtendedToDouble(value));

	// Get stack top
	regs.decFpuTop();
	if (regs.isFpuValid(regs.getFpuTop()))
		throw misc::Panic("Unexpected valid FPU entry");

	regs.setFpuValid(regs.getFpuTop());
	memcpy(regs.getFpuValue(regs.getFpuTop()), value, 10);
}


void Context::PopFpu(unsigned char *value)
{
	// Check valid entry
	if (!regs.isFpuValid(regs.getFpuTop()))
		throw misc::Panic("Unexpected invalid FPU entry");

	// Copy value
	if (value)
		memcpy(value, regs.getFpuValue(regs.getFpuTop()), 10);
	
	// Debug
	if (emulator->isa_debug)
		emulator->isa_debug << misc::fmt("  st(0)=%g (popped)",
				Extended::ExtendedToDouble(regs.
				getFpuValue(regs.getFpuTop())));

	// Pop
	regs.setFpuValid(regs.getFpuTop(), false);
	regs.incFpuTop();
}


double Context::LoadDouble()
{
	double value;
	MemoryRead(getEffectiveAddress(), 8, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=%g", getEffectiveAddress(), value);
	return value;
}


void Context::LoadExtended(unsigned char *value)
{
	MemoryRead(getEffectiveAddress(), 10, value);
}


void Context::StoreExtended(unsigned char *value)
{
	MemoryWrite(getEffectiveAddress(), 10, value);
}


void Context::StoreDouble(double value)
{
	MemoryWrite(getEffectiveAddress(), 8, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]<=%g", getEffectiveAddress(), value);
}


float Context::LoadFloat()
{
	float value;

	MemoryRead(getEffectiveAddress(), 4, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]=%g", getEffectiveAddress(),
			(double) value);

	return value;
}


void Context::StoreFloat(float value)
{
	MemoryWrite(getEffectiveAddress(), 4, &value);
	emulator->isa_debug << misc::fmt("  [0x%x]<=%g", getEffectiveAddress(),
			(double) value);
}


void Context::StoreFpuCode(unsigned short status)
{
	regs.setFpuCode(0);
	if (misc::getBit32(status, 14))
		regs.setFpuCode(regs.getFpuCode() | 8);
	regs.setFpuCode(regs.getFpuCode() | ((status >> 8) & 7));
}


unsigned short Context::LoadFpuStatus()
{
	unsigned short status = 0;
	
	if (!misc::inRange(regs.getFpuTop(), 0, 7))
		throw misc::Panic("Invalid FPU stack top");

	status |= regs.getFpuTop() << 11;
	if (misc::getBit32(regs.getFpuCode(), 3))
		status |= 0x4000;
	status |= (regs.getFpuCode() & 7) << 8;
	return status;
}


}  // namespace x86

