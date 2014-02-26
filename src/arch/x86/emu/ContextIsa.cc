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

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"


namespace x86
{

void Context::IsaError(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	// No error shown on speculative mode
	if (state & ContextSpecMode)
		return;

	// Error
	fflush(NULL);
	fprintf(stderr, "\n\nfatal: x86 context %d at 0x%08x inst %lld: ",
			pid, current_eip, emu->getInstructions());
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	exit(1);
}

void Context::MemoryRead(unsigned int address, int size, void *buffer)
{
	// Speculative mode read
	/*if (self->state & X86ContextSpecMode)
	{
		spec_mem_read(self->spec_mem, addr, size, buf);
		return;
	}*/

	// Read in regular mode
	memory->Read(address, size, (char *) buffer);
}


void Context::MemoryWrite(unsigned int address, int size, void *buffer)
{
	// Speculative mode write
	/*if (self->state & X86ContextSpecMode)
	{
		spec_mem_write(self->spec_mem, addr, size, buf);
		return;
	}*/

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
		return regs.Read(inst.getModRmRm() + InstRegAl);

	MemoryRead(getEffectiveAddress(), 1, &value);
	emu->isa_debug << misc::fmt("  [0x%x]=0x%x", effective_address, value);
	return value;
}

unsigned short Context::LoadRm16()
{
	unsigned short value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + InstRegAx);

	MemoryRead(getEffectiveAddress(), 2, &value);
	emu->isa_debug << misc::fmt("  [0x%x]=0x%x", effective_address, value);
	return value;
}

unsigned int Context::LoadRm32()
{
	unsigned int value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + InstRegEax);

	MemoryRead(getEffectiveAddress(), 4, &value);
	emu->isa_debug << misc::fmt("  [0x%x]=0x%x", effective_address, value);
	return value;
}

unsigned short Context::LoadR32M16()
{
	unsigned short value;

	if (inst.getModRmMod() == 0x03)
		return regs.Read(inst.getModRmRm() + InstRegEax);

	MemoryRead(getEffectiveAddress(), 2, &value);
	emu->isa_debug << misc::fmt("  [0x%x]=0x%x", effective_address, value);
	return value;
}

unsigned long long Context::LoadM64()
{
	unsigned long long value;

	MemoryRead(getEffectiveAddress(), 8, &value);
	emu->isa_debug << misc::fmt("  [0x%x]=0x%llx", effective_address, value);
	return value;
}

void Context::StoreRm8(unsigned char value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + InstRegAl, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 1, &value);
	emu->isa_debug << misc::fmt("  [0x%x] <- 0x%x", effective_address, value);
}

void Context::StoreRm16(unsigned short value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + InstRegAx, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 2, &value);
	emu->isa_debug << misc::fmt("  [0x%x] <- 0x%x", effective_address, value);
}

void Context::StoreRm32(unsigned int value)
{
	if (inst.getModRmMod() == 0x03)
	{
		regs.Write(inst.getModRmRm() + InstRegEax, value);
		return;
	}
	MemoryWrite(getEffectiveAddress(), 4, &value);
	emu->isa_debug << misc::fmt("  [0x%x] <- 0x%x", effective_address, value);
}

void Context::StoreM64(unsigned long long value)
{
	MemoryWrite(getEffectiveAddress(), 8, &value);
	emu->isa_debug << misc::fmt("  [0x%x] <- 0x%llx", effective_address, value);
}

unsigned Context::getLinearAddress(unsigned offset)
{
	// No segment override
	if (!inst.getSegment())
		return offset;
	
	// Segment override
	if (inst.getSegment() != InstRegGs)
	{
		IsaError("segment override not supported");
		return 0;
	}

	// GLibc segment at TLS entry 6
	if (regs.Read(InstRegGs) != 0x33)
	{
		IsaError("isa_linear_address: gs = 0x%x", regs.Read(InstRegGs));
		return 0;
	}

	if (!glibc_segment_base)
	{
		IsaError("isa_linear_address: glibc segment not set");
		return 0;
	}

	// Return address
	return glibc_segment_base + offset;
}


unsigned Context::getEffectiveAddress()
{
	// Check 'modrm_mod' field
	if (inst.getModRmMod() == 3)
	{
		IsaError("%s: wrong value for 'modrm_mod'", __FUNCTION__);
		return 0;
	}

	// Address
	unsigned address = regs.Read(inst.getEaBase()) +
		regs.Read(inst.getEaIndex()) * inst.getEaScale() +
		inst.getDisp();
	
	// Add segment base
	address = getLinearAddress(address);

	// Record effective address in context. This address is used later in the
	// generation of micro-instructions. We need to record it to avoid calling this
	// function again later, since the source register used to calculate the effective
	// address can be overwritten after the instruction emulation.
	effective_address = address;

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



}  // namespace x86

