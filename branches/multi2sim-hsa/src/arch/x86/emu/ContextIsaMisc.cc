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

#include "Context.h"
#include "Emu.h"

#include <lib/cpp/Misc.h>


namespace x86
{

// Macros defined to prevent accidental use of functions that cause unsafe
// execution in speculative mode.
#undef assert
#define memory __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#define assert __COMPILATION_ERROR__


void Context::ExecuteInst_bound_r16_rm32()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_bound_r32_rm64()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_bsf_r32_rm32()
{
	unsigned int r32 = LoadR32();
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"bsf %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (r32)
		: "g" (r32), "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(r32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, 0, 0, UInstDepR32, UInstDepZps, 0, 0);
}


void Context::ExecuteInst_bsr_r32_rm32()
{
	unsigned int r32 = LoadR32();
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"bsr %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (r32), "m" (rm32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(r32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, 0, 0, UInstDepR32, UInstDepZps, 0, 0);
}


void Context::ExecuteInst_bswap_ir32()
{
	unsigned int ir32 = LoadIR32();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"mov %1, %%eax\n\t"
		"bswap %%eax\n\t"
		"mov %%eax, %0\n\t"
		: "=g" (ir32)
		: "g" (ir32)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreIR32(ir32);

	newUInst(UInstShift, UInstDepIr32, 0, 0, UInstDepIr32, 0, 0, 0);
}


void Context::ExecuteInst_bt_rm32_r32()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ecx\n\t"
		"mov %1, %%eax\n\t"
		"bt %%ecx, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (rm32), "m" (r32), "g" (flags)
		: "eax", "ecx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, UInstDepR32, 0, UInstDepCf, 0, 0, 0);
}


void Context::ExecuteInst_bt_rm32_imm8()
{
	unsigned int rm32 = LoadRm32();
	unsigned int imm8 = inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ecx\n\t"
		"mov %1, %%eax\n\t"
		"bt %%ecx, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ecx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, 0, 0, UInstDepCf, 0, 0, 0);
}


void Context::ExecuteInst_bts_rm32_imm8()
{
	unsigned int rm32 = LoadRm32();
	unsigned int imm8 = inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %3, %%ecx\n\t"
		"mov %2, %%eax\n\t"
		"btsl %%ecx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ecx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, 0, 0, UInstDepRm32, UInstDepCf, 0, 0);
}


void Context::ExecuteInst_call_rel32()
{
	// Emulate
	regs.decEsp(4);
	unsigned eip = regs.getEip();
	MemoryWrite(regs.getEsp(), 4, &eip);
	target_eip = regs.getEip() + inst.getImmDWord();
	regs.setEip(target_eip);

	// Call stack
	if (call_stack != nullptr)
		call_stack->Call(target_eip, regs.getEsp());

	// Micro-instructions
	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, 0, 0, 0, 0, 0, 0);
	newUInst(UInstCall, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_call_rm32()
{
	// Emulate
	target_eip = LoadRm32();
	regs.decEsp(4);
	unsigned eip = regs.getEip();
	MemoryWrite(regs.getEsp(), 4, &eip);
	regs.setEip(target_eip);

	// Call stack
	if (call_stack != nullptr)
		call_stack->Call(target_eip, regs.getEsp());

	// Micro-instructions
	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, 0, 0, 0, 0, 0, 0);
	newUInst(UInstCall, UInstDepRm32, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_cbw()
{
	unsigned short ax = (char) regs.Read(InstRegAl);
	regs.Write(InstRegAx, ax);

	newUInst(UInstSign, UInstDepEax, 0, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_cdq()
{
	int signed_eax = regs.getEax();

	regs.setEdx(signed_eax < 0 ? -1 : 0);

	newUInst(UInstSign, UInstDepEax, 0, 0, UInstDepEdx, 0, 0, 0);
}


void Context::ExecuteInst_cld()
{
	regs.clearFlag(InstFlagDF);

	newUInst(UInstMove, 0, 0, 0, 0, UInstDepDf, 0, 0);
}


void Context::ExecuteInst_cmpxchg_rm32_r32()
{
	unsigned int eax = regs.getEax();
	unsigned long flags = regs.getEflags();
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %4, %%eax\n\t"
		"mov %5, %%ebx\n\t"
		"mov %6, %%ecx\n\t"
		"cmpxchg %%ecx, %%ebx\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		"mov %%eax, %1\n\t"
		"mov %%ebx, %2\n\t"
		: "=g" (flags), "=m" (eax), "=m" (rm32)
		: "g" (flags), "m" (eax), "m" (rm32), "m" (r32)
		: "eax", "ebx", "ecx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.setEflags(flags);
	regs.Write(InstRegEax, eax);
	StoreRm32(rm32);

	newUInst(UInstSub, UInstDepEax, UInstDepRm32, 0, UInstDepZps, UInstDepCf, UInstDepOf, 0);
	newUInst(UInstMove, UInstDepZps, UInstDepR32, 0, UInstDepRm32, 0, 0, 0);
	newUInst(UInstMove, UInstDepCf, UInstDepRm32, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_cmpxchg8b_m64()
{
	unsigned int eax, ebx, ecx, edx;
	unsigned long long edx_eax, m64;

	eax = regs.getEax();
	ebx = regs.getEbx();
	ecx = regs.getEcx();
	edx = regs.getEdx();
	edx_eax = ((unsigned long long) edx << 32) | eax;
	m64 = LoadM64();

	if (edx_eax == m64)
	{
		regs.setFlag(InstFlagZF);
		m64 = ((unsigned long long) ecx << 32) | ebx;
		StoreM64(m64);
	}
	else
	{
		regs.clearFlag(InstFlagZF);
		regs.setEdx(m64 >> 32);
		regs.setEax(m64);
	}

	newMemoryUInst(UInstLoad, getEffectiveAddress(), 8, 0, 0, 0,
		UInstDepAux, 0, 0, 0);  // Load m64
	newUInst(UInstSub, UInstDepEdx, UInstDepEax, UInstDepAux,
		UInstDepZps, 0, 0, 0);  // Compare edx-eax with m64
	newMemoryUInst(UInstStore, getEffectiveAddress(), 8, UInstDepZps, UInstDepEcx, UInstDepEbx,
		UInstDepMem64, 0, 0, 0);  // Conditionally store m64
	newUInst(UInstMove, UInstDepZps, 0, 0,
		UInstDepEdx, UInstDepEax, 0, 0);  // Conditionaly store edx-eax
}


void Context::ExecuteInst_cpuid()
{
	unsigned int info = 0;

	switch (regs.getEax())
	{

	case 0x0:

		regs.Write(InstRegEax, 0x2);
		regs.Write(InstRegEbx, 0x756e6547);
		regs.Write(InstRegEcx, 0x6c65746e);
		regs.Write(InstRegEdx, 0x49656e69);
		break;

	case 0x1:

		regs.Write(InstRegEax, 0x00000f29);
		regs.Write(InstRegEbx, 0x0102080b);
		regs.Write(InstRegEcx, 0x00004400);

		// EDX register returns CPU features information.
		info = misc::setBit32(info, 31, 1);  // PBE - Pend Brk En
		info = misc::setBit32(info, 29, 1);  // TM - Therm Monitor
		info = misc::setBit32(info, 28, 1);  // HTT - Hyper-threading Tech.
		info = misc::setBit32(info, 27, 1);  // SS - Self snoop
		info = misc::setBit32(info, 26, 0);  // SSE2 - SSE2 Extensions
		info = misc::setBit32(info, 25, 0);  // SSE - SSE Extensions
		info = misc::setBit32(info, 24, 1);  // FXSR - FXSAVE/FXRSTOR
		info = misc::setBit32(info, 23, 0);  // MMX - MMX Technology
		info = misc::setBit32(info, 22, 1);  // ACPI - Thermal Monitor and Clock Ctrl
		info = misc::setBit32(info, 21, 1);  // DS - Debug Store
		info = misc::setBit32(info, 19, 1);  // CLFSH - CFLUSH instruction
		info = misc::setBit32(info, 18, 0);  // PSN - Processor Serial Number
		info = misc::setBit32(info, 17, 1);  // PSE - Page size extension
		info = misc::setBit32(info, 16, 1);  // PAT - Page Attribute Table
		info = misc::setBit32(info, 15, 1);  // CMOV - Conditional Move/Compare Instruction
		info = misc::setBit32(info, 14, 1);  // MCA - Machine Check Architecture
		info = misc::setBit32(info, 13, 1);  // PGE - PTE Global bit
		info = misc::setBit32(info, 12, 1);  // MTRR - Memory Type Range Registers
		info = misc::setBit32(info, 11, 1);  // SEP - SYSENTER and SYSEXIT
		info = misc::setBit32(info, 9, 1);  // APIC - APIC on Chip
		info = misc::setBit32(info, 8, 1);  // CX8 - CMPXCHG8B inst.
		info = misc::setBit32(info, 7, 1);  // MCE - Machine Check Exception
		info = misc::setBit32(info, 6, 1);  // PAE - Physical Address Extensions
		info = misc::setBit32(info, 5, 1);  // MSR - RDMSR and WRMSR Support
		info = misc::setBit32(info, 4, 1);  // TSC - Time Stamp Counter
		info = misc::setBit32(info, 3, 1);  // PSE - Page Size Extensions
		info = misc::setBit32(info, 2, 1);  // DE - Debugging Extensions
		info = misc::setBit32(info, 1, 1);  // VME - Virtual-8086 Mode Enhancement
		info = misc::setBit32(info, 0, 1);  // FPU - x87 FPU on Chip

		regs.Write(InstRegEdx, info);
		break;

	case 0x2:

		regs.Write(InstRegEax, 0);
		regs.Write(InstRegEbx, 0);
		regs.Write(InstRegEcx, 0);
		regs.Write(InstRegEdx, 0);
		break;

	case 0x80000000:

		regs.Write(InstRegEax, 0x80000004);
		regs.Write(InstRegEbx, 0);
		regs.Write(InstRegEcx, 0);
		regs.Write(InstRegEdx, 0);
		break;

	case 0x80000001:

		regs.Write(InstRegEax, 0);
		regs.Write(InstRegEbx, 0);
		regs.Write(InstRegEcx, 0);
		regs.Write(InstRegEdx, 0);
		break;

	case 0x80000002:

		regs.Write(InstRegEax, 0x20202020);
		regs.Write(InstRegEbx, 0x20202020);
		regs.Write(InstRegEcx, 0x20202020);
		regs.Write(InstRegEdx, 0x20202020);
		break;

	case 0x80000003:

		regs.Write(InstRegEax, 0x6e492020);
		regs.Write(InstRegEbx, 0x286c6574);
		regs.Write(InstRegEcx, 0x58202952);
		regs.Write(InstRegEdx, 0x286e6f65);
		break;

	case 0x80000004:

		regs.Write(InstRegEax, 0x20294d54);
		regs.Write(InstRegEbx, 0x20555043);
		regs.Write(InstRegEcx, 0x30382e32);
		regs.Write(InstRegEdx, 0x7a4847);
		break;

	default:

		throw misc::Panic(misc::fmt("Unimplemented for eax = 0x%x",
				regs.getEax()));
	}

	newUInst(UInstMove, 0, 0, 0, UInstDepEax, 0, 0, 0);
	newUInst(UInstMove, 0, 0, 0, UInstDepEbx, 0, 0, 0);
	newUInst(UInstMove, 0, 0, 0, UInstDepEcx, 0, 0, 0);
	newUInst(UInstMove, 0, 0, 0, UInstDepEdx, 0, 0, 0);
}


void Context::ExecuteInst_cwde()
{
	unsigned int eax = (short) regs.Read(InstRegAx);
	regs.Write(InstRegEax, eax);

	newUInst(UInstSign, UInstDepEax, 0, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_dec_rm8()
{
	unsigned char rm8 = LoadRm8();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"dec %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm8(rm8);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepRm8, 0, 0, UInstDepRm8, UInstDepZps, UInstDepOf, 0);
}


void Context::ExecuteInst_dec_rm16()
{
	unsigned short rm16 = LoadRm16();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"dec %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm16)
		: "g" (rm16), "g" (flags)
		: "ax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm16(rm16);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepRm16, 0, 0, UInstDepRm16, UInstDepZps, UInstDepOf, 0);
}


void Context::ExecuteInst_dec_rm32()
{
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"dec %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepRm32, 0, 0, UInstDepRm32, UInstDepZps, UInstDepOf, 0);
}


void Context::ExecuteInst_dec_ir16()
{
	unsigned short ir16 = LoadIR16();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"dec %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir16)
		: "g" (ir16), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreIR16(ir16);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepIr16, 0, 0, UInstDepIr16, UInstDepZps, UInstDepOf, 0);
}


void Context::ExecuteInst_dec_ir32()
{
	unsigned int ir32 = LoadIR32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"dec %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir32)
		: "g" (ir32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreIR32(ir32);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepIr32, 0, 0, UInstDepIr32, UInstDepZps, UInstDepOf, 0);
}


void Context::ExecuteInst_div_rm8()
{
	int skip_emulation;
	int spec_mode;

	unsigned short ax = regs.Read(InstRegAx);
	unsigned char rm8 = LoadRm8();

	if (!rm8)
		throw Error("Division by 0");

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xff. */
	spec_mode = getState(StateSpecMode);
	skip_emulation = spec_mode && ax > 0xff;

	// Emulate
	if (!skip_emulation)
	{
		__X86_CONTEXT_SAVE_FLAGS__
		asm volatile (
			"mov %1, %%ax\n\t"
			"mov %2, %%bl\n\t"
			"div %%bl\n\t"
			"mov %%ax, %0\n\t"
			: "=m" (ax)
			: "m" (ax), "m" (rm8)
			: "ax", "bl"
		);
		__X86_CONTEXT_RESTORE_FLAGS__
	}

	regs.Write(InstRegAx, ax);

	newUInst(UInstDiv, UInstDepEax, UInstDepRm8, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_div_rm32()
{
	int spec_mode;
	int skip_emulation;

	unsigned int eax = regs.getEax();
	unsigned int edx = regs.getEdx();
	unsigned int rm32 = LoadRm32();

	if (!rm32)
		throw Error("Division by 0");

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xffffffff. */
	spec_mode = getState(StateSpecMode);
	skip_emulation = spec_mode && edx;

	// Emulate
	if (!skip_emulation)
	{
		__X86_CONTEXT_SAVE_FLAGS__
		asm volatile (
			"mov %2, %%eax\n\t"
			"mov %3, %%edx\n\t"
			"mov %4, %%ebx\n\t"
			"div %%ebx\n\t"
			"mov %%eax, %0\n\t"
			"mov %%edx, %1\n\t"
			: "=m" (eax), "=m" (edx)
			: "m" (eax), "m" (edx), "m" (rm32)
			: "eax", "edx", "ebx"
		);
		__X86_CONTEXT_RESTORE_FLAGS__
	}

	regs.Write(InstRegEax, eax);
	regs.Write(InstRegEdx, edx);

	newUInst(UInstDiv, UInstDepEdx, UInstDepEax, UInstDepRm32, UInstDepEax, UInstDepEdx, 0, 0);
}


void Context::ExecuteInst_hlt()
{
	throw Error("Instruction 'hlt' executed");
}


void Context::ExecuteInst_idiv_rm32()
{
	int spec_mode;
	int skip_emulation;

	unsigned int eax = regs.getEax();
	unsigned int edx = regs.getEdx();
	unsigned int rm32 = LoadRm32();

	if (!rm32)
		throw Error("Division by 0");

	// Avoid emulation in speculative mode if it could cause a divide exception
	skip_emulation = 0;
	spec_mode = getState(StateSpecMode);
	if (spec_mode)
	{
		long long edx_eax = ((unsigned long long) edx << 32) | eax;
		if (edx_eax > (long long) 0x7fffffffll
				|| edx_eax < (long long) 0xffffffff80000000ll)
			skip_emulation = 1;
	}

	if (!skip_emulation)
	{
		__X86_CONTEXT_SAVE_FLAGS__
		asm volatile (
			"mov %2, %%eax\n\t"
			"mov %3, %%edx\n\t"
			"mov %4, %%ebx\n\t"
			"idiv %%ebx\n\t"
			"mov %%eax, %0\n\t"
			"mov %%edx, %1\n\t"
			: "=m" (eax), "=m" (edx)
			: "m" (eax), "m" (edx), "m" (rm32)
			: "eax", "edx", "ebx"
		);
		__X86_CONTEXT_RESTORE_FLAGS__
	}

	regs.Write(InstRegEax, eax);
	regs.Write(InstRegEdx, edx);

	newUInst(UInstDiv, UInstDepRm32, UInstDepEax, 0, UInstDepEax, UInstDepEdx, 0, 0);
}


void Context::ExecuteInst_imul_rm32()
{
	unsigned int eax = regs.Read(InstRegEax);
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();
	unsigned int edx;

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %3, %%eax\n\t"
		"mov %4, %%edx\n\t"
		"imul %%edx\n\t"
		"mov %%edx, %1\n\t"
		"mov %%eax, %2\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (edx), "=m" (eax)
		: "m" (eax), "m" (rm32), "g" (flags)
		: "eax", "edx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.Write(InstRegEax, eax);
	regs.Write(InstRegEdx, edx);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepRm32, UInstDepEax, 0, UInstDepEax, UInstDepEdx, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_imul_r16_rm16()
{
	unsigned int r16 = LoadR16();
	unsigned int rm16 = LoadRm16();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"mov %3, %%bx\n\t"
		"imul %%bx, %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r16)
		: "m" (r16), "m" (rm16), "g" (flags)
		: "ax", "bx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR16(r16);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepR16, UInstDepRm16, 0, UInstDepR16, 0, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_imul_r32_rm32()
{
	unsigned int r32 = LoadR32();
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (r32), "m" (rm32), "g" (flags)
		: "eax", "ebx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(r32);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepR32, UInstDepRm32, 0, UInstDepR32, 0, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_imul_r32_rm32_imm8()
{
	unsigned int r32;
	unsigned int rm32 = LoadRm32();
	unsigned int imm8 = (char) inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ebx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(r32);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepRm32, 0, 0, UInstDepR32, 0, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_imul_r32_rm32_imm32()
{
	unsigned int r32;
	unsigned int rm32 = LoadRm32();
	unsigned int imm32 = inst.getImmDWord();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (rm32), "m" (imm32), "g" (flags)
		: "eax", "ebx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(r32);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepRm32, 0, 0, UInstDepR32, 0, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_inc_rm8()
{
	unsigned char rm8 = LoadRm8();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"inc %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm8(rm8);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepRm8, 0, 0, UInstDepRm8, 0, UInstDepZps, UInstDepOf);
}


void Context::ExecuteInst_inc_rm16()
{
	unsigned short rm16 = LoadRm16();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"inc %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm16)
		: "g" (rm16), "g" (flags)
		: "ax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm16(rm16);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepRm16, 0, 0, UInstDepRm16, 0, UInstDepZps, UInstDepOf);
}


void Context::ExecuteInst_inc_rm32()
{
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"inc %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepRm32, 0, 0, UInstDepRm32, 0, UInstDepZps, UInstDepOf);
}


void Context::ExecuteInst_inc_ir16()
{
	unsigned short ir16 = LoadIR16();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"inc %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir16)
		: "g" (ir16), "g" (flags)
		: "ax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreIR16(ir16);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepIr16, 0, 0, UInstDepIr16, 0, UInstDepZps, UInstDepOf);
}


void Context::ExecuteInst_inc_ir32()
{
	unsigned int ir32 = LoadIR32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"inc %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir32)
		: "g" (ir32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreIR32(ir32);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepIr32, 0, 0, UInstDepIr32, 0, UInstDepZps, UInstDepOf);
}


void Context::ExecuteInst_int_3()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_int_imm8()
{
	int spec_mode;
	unsigned int num;

	// Interrupt code
	num = (unsigned char) inst.getImmByte();
	if (num != 0x80)
		throw misc::Panic("Software interrupt not supported for "
				"code other than 0x80");
	
	// Debug
	int code = regs.getEax();
	if (misc::inRange(code, 0, SyscallCodeCount))
		emu->isa_debug << " syscall '" << syscall_name[code] << "'";

	// Do system call if not in speculative mode
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
		ExecuteSyscall();

	newUInst(UInstSyscall, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_into()
{
	throw misc::Panic("Unimplemented instruction");
}



void Context::ExecuteInst_jmp_rel8()
{
	target_eip = regs.getEip() + (char) inst.getImmByte();
	regs.setEip(target_eip);

	newUInst(UInstJump, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_jmp_rel32()
{
	target_eip = regs.getEip() + inst.getImmDWord();
	regs.setEip(target_eip);

	newUInst(UInstJump, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_jmp_rm32()
{
	target_eip = LoadRm32();
	regs.setEip(target_eip);

	newUInst(UInstJump, UInstDepRm32, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_lea_r32_m()
{
	unsigned int value = getEffectiveAddress();

	if (inst.getSegment())
		throw misc::Panic("Unsupported segment");

	StoreR32(value);

	newUInst(UInstEffaddr, UInstDepEaseg, UInstDepEabas, UInstDepEaidx, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_leave()
{
	unsigned int value;
	regs.setEsp(regs.getEbp());

	if (inst.getSegment())
		throw misc::Panic("Unsupported segment");

	MemoryRead(regs.getEsp(), 4, &value);
	regs.incEsp(4);
	regs.Write(InstRegEbp, value);

	newUInst(UInstMove, UInstDepEbp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4, 4, UInstDepAux, 0, 0, UInstDepEbp, 0, 0, 0);
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
}


void Context::ExecuteInst_lfence()
{
}


void Context::ExecuteInst_mov_rm8_imm8()
{
	unsigned char value = inst.getImmByte();
	StoreRm8(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepRm8, 0, 0, 0);
}


void Context::ExecuteInst_mov_r8_rm8()
{
	unsigned char value = LoadRm8();
	StoreR8(value);

	newUInst(UInstMove, UInstDepRm8, 0, 0, UInstDepR8, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm8_r8()
{
	unsigned char value = LoadR8();
	StoreRm8(value);

	newUInst(UInstMove, UInstDepR8, 0, 0, UInstDepRm8, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm16_r16()
{
	unsigned short value = LoadR16();
	StoreRm16(value);

	newUInst(UInstMove, UInstDepR16, 0, 0, UInstDepRm16, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm32_r32()
{
	unsigned int value = LoadR32();
	StoreRm32(value);

	newUInst(UInstMove, UInstDepR32, 0, 0, UInstDepRm32, 0, 0, 0);
}


void Context::ExecuteInst_mov_r16_rm16()
{
	unsigned short value = LoadRm16();
	StoreR16(value);

	newUInst(UInstMove, UInstDepRm16, 0, 0, UInstDepR16, 0, 0, 0);
}


void Context::ExecuteInst_mov_r32_rm32()
{
	unsigned int value = LoadRm32();
	StoreR32(value);

	newUInst(UInstMove, UInstDepRm32, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_mov_al_moffs8()
{
	unsigned char value;

	MemoryRead(getMoffsAddress(), 1, &value);
	regs.Write(InstRegAl, value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, getMoffsAddress(), 1, UInstDepAux, 0, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_mov_ax_moffs16()
{
	unsigned short value;

	MemoryRead(getMoffsAddress(), 2, &value);
	regs.Write(InstRegAx, value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, getMoffsAddress(), 2, UInstDepAux, 0, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_mov_eax_moffs32()
{
	unsigned int value;

	MemoryRead(getMoffsAddress(), 4, &value);
	regs.Write(InstRegEax, value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, getMoffsAddress(), 4, UInstDepAux, 0, 0, UInstDepEax, 0, 0, 0);
}


void Context::ExecuteInst_mov_moffs8_al()
{
	unsigned char value = regs.Read(InstRegAl);
	MemoryWrite(getMoffsAddress(), 1, &value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, getMoffsAddress(), 1, UInstDepAux, UInstDepEax, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_mov_moffs16_ax()
{
	unsigned short value = regs.Read(InstRegAx);
	MemoryWrite(getMoffsAddress(), 2, &value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, getMoffsAddress(), 2, UInstDepAux, UInstDepEax, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_mov_moffs32_eax()
{
	unsigned int value = regs.Read(InstRegEax);
	MemoryWrite(getMoffsAddress(), 4, &value);

	newUInst(UInstEffaddr, 0, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, getMoffsAddress(), 4, UInstDepAux, UInstDepEax, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_mov_ir8_imm8()
{
	unsigned char value = inst.getImmByte();
	StoreIR8(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepIr8, 0, 0, 0);
}


void Context::ExecuteInst_mov_ir16_imm16()
{
	unsigned short value = inst.getImmWord();
	StoreIR16(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepIr16, 0, 0, 0);
}


void Context::ExecuteInst_mov_ir32_imm32()
{
	unsigned int value = inst.getImmDWord();
	StoreIR32(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepIr32, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm16_imm16()
{
	unsigned short value = inst.getImmWord();
	StoreRm16(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepRm16, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm32_imm32()
{
	unsigned int value = inst.getImmDWord();
	StoreRm32(value);

	newUInst(UInstMove, 0, 0, 0, UInstDepRm32, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm16_sreg()
{
	unsigned short value = LoadSReg();
	if (inst.getModRmReg() != 5)
		throw misc::Panic("Not supported for sreg != gs");
	StoreRm16(value);

	newUInst(UInstMove, UInstDepSreg, 0, 0, UInstDepRm16, 0, 0, 0);
}


void Context::ExecuteInst_mov_rm32_sreg()
{
	ExecuteInst_mov_rm16_sreg();
}


void Context::ExecuteInst_mov_sreg_rm16()
{
	unsigned short value = LoadRm16();
	if (inst.getModRmReg() != 5)
		throw misc::Panic("Not supported for sreg != gs");
	StoreSReg(value);

	newUInst(UInstMove, UInstDepRm16, 0, 0, UInstDepSreg, 0, 0, 0);
}


void Context::ExecuteInst_mov_sreg_rm32()
{
	ExecuteInst_mov_sreg_rm16();
}


void Context::ExecuteInst_movsx_r16_rm8()
{
	unsigned short value = (char) LoadRm8();
	StoreR16(value);

	newUInst(UInstSign, UInstDepRm8, 0, 0, UInstDepR16, 0, 0, 0);
}


void Context::ExecuteInst_movsx_r32_rm8()
{
	unsigned int value = (char) LoadRm8();
	StoreR32(value);

	newUInst(UInstSign, UInstDepRm8, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_movsx_r32_rm16()
{
	unsigned int value = (short) LoadRm16();
	StoreR32(value);

	newUInst(UInstSign, UInstDepRm16, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_movzx_r16_rm8()
{
	unsigned char value = LoadRm8();
	StoreR16(value);

	newUInst(UInstMove, UInstDepRm8, 0, 0, UInstDepR16, 0, 0, 0);
}


void Context::ExecuteInst_movzx_r32_rm8()
{
	unsigned char value = LoadRm8();
	StoreR32(value);

	newUInst(UInstMove, UInstDepRm8, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_movzx_r32_rm16()
{
	unsigned short value = LoadRm16();
	StoreR32(value);

	newUInst(UInstMove, UInstDepRm16, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_mul_rm32()
{
	unsigned int eax = regs.Read(InstRegEax);
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();
	unsigned int edx;

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %3, %%eax\n\t"
		"mov %4, %%edx\n\t"
		"mul %%edx\n\t"
		"mov %%edx, %1\n\t"
		"mov %%eax, %2\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (edx), "=m" (eax)
		: "m" (eax), "m" (rm32), "g" (flags)
		: "eax", "edx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.Write(InstRegEax, eax);
	regs.Write(InstRegEdx, edx);
	regs.setEflags(flags);

	newUInst(UInstMult, UInstDepRm32, UInstDepEax, 0, UInstDepEdx, UInstDepEax, UInstDepOf, UInstDepCf);
}


void Context::ExecuteInst_neg_rm8()
{
	unsigned char rm8 = LoadRm8();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"neg %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm8(rm8);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepRm8, 0, 0, UInstDepRm8, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_neg_rm32()
{
	unsigned int rm32 = LoadRm32();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"neg %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstSub, UInstDepRm32, 0, 0, UInstDepRm32, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_nop()
{
}


void Context::ExecuteInst_nop_rm16()
{
}


void Context::ExecuteInst_nop_rm32()
{
}


void Context::ExecuteInst_not_rm8()
{
	unsigned char value = LoadRm8();

	value = ~value;
	StoreRm8(value);

	newUInst(UInstNot, UInstDepRm8, 0, 0, UInstDepRm8, 0, 0, 0);
}


void Context::ExecuteInst_not_rm16()
{
	unsigned short value = LoadRm16();
	value = ~value;
	StoreRm16(value);

	newUInst(UInstNot, UInstDepRm8, 0, 0, UInstDepRm16, 0, 0, 0);
}


void Context::ExecuteInst_not_rm32()
{
	unsigned int value = LoadRm32();
	value = ~value;
	StoreRm32(value);

	newUInst(UInstNot, UInstDepRm8, 0, 0, UInstDepRm32, 0, 0, 0);
}


void Context::ExecuteInst_out_imm8_al()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_out_imm8_ax()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_out_imm8_eax()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_out_dx_al()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_out_dx_ax()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_out_dx_eax()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_pause()
{
}


void Context::ExecuteInst_pop_rm32()
{
	unsigned int value;

	MemoryRead(regs.getEsp(), 4, &value);
	regs.incEsp(4);
	StoreRm32(value);

	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4, 4, UInstDepAux, 0, 0, UInstDepRm32, 0, 0, 0);
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
}


void Context::ExecuteInst_pop_ir32()
{
	unsigned int value;

	if (inst.getSegment())
		throw misc::Panic("Unsupported segment");

	MemoryRead(regs.getEsp(), 4, &value);
	regs.incEsp(4);
	StoreIR32(value);

	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4, 4, UInstDepAux, 0, 0, UInstDepIr32, 0, 0, 0);
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
}


void Context::ExecuteInst_popf()
{
	unsigned eflags;
	MemoryRead(regs.getEsp(), 4, &eflags);
	regs.incEsp(4);

	// Prevent TF from being set. A program should never do this, but it could
	// happen during speculative execution (case reported by Multi2Sim user).
	// The next instruction that is emulated in speculative mode could cause the
	// host to push this value of 'eflags', causing a TRAP in the host code.
	eflags &= ~(1 << 8);

	// Set the new value for flags
	regs.setEflags(eflags);

	// Micro-instructions
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4, 4, UInstDepAux, 0, 0, UInstDepZps, UInstDepCf, UInstDepOf, 0);
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
}


void Context::ExecuteInst_prefetcht0_m8()
{
	unsigned int eff_addr;

	// Prefetching makes sense only in a detailed simulation
	if (Emu::getSimKind() != comm::Arch::SimDetailed)
		return;

	// Not if prefetch is disabled
	if (!Emu::getProcessPrefetchHints())
		return;

	eff_addr = getEffectiveAddress();
	newUInst(UInstEffaddr, UInstDepEaseg, UInstDepEabas, UInstDepEaidx, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstPrefetch, eff_addr, 1, UInstDepAux, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_prefetcht1()
{
}


void Context::ExecuteInst_prefetcht2()
{
}


void Context::ExecuteInst_prefetchnta()
{
}


void Context::ExecuteInst_push_imm8()
{
	unsigned int value = (char) inst.getImmByte();

	regs.Write(InstRegEsp, regs.getEsp() - 4);
	MemoryWrite(regs.getEsp(), 4, &value);

	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_push_imm32()
{
	unsigned int value = inst.getImmDWord();

	regs.Write(InstRegEsp, regs.getEsp() - 4);
	MemoryWrite(regs.getEsp(), 4, &value);

	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_push_rm32()
{
	unsigned int value = LoadRm32();

	regs.Write(InstRegEsp, regs.getEsp() - 4);
	MemoryWrite(regs.getEsp(), 4, &value);

	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, UInstDepRm32, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_push_ir32()
{
	unsigned int value = LoadIR32();

	regs.Write(InstRegEsp, regs.getEsp() - 4);
	MemoryWrite(regs.getEsp(), 4, &value);

	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, UInstDepIr32, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_pushf()
{
	unsigned eflags = regs.getEflags();
	regs.Write(InstRegEsp, regs.getEsp() - 4);
	MemoryWrite(regs.getEsp(), 4, &eflags);

	newUInst(UInstSub, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newUInst(UInstMove, UInstDepZps, UInstDepCf, UInstDepOf, UInstDepAux2, 0, 0, 0);
	newMemoryUInst(UInstStore, regs.getEsp(), 4, UInstDepAux, UInstDepAux2, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_rdtsc()
{
	unsigned int eax, edx;

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"rdtsc\n\t"
		"mov %%eax, %0\n\t"
		"mov %%edx, %1\n\t"
		: "=g" (eax), "=g" (edx)
		:
		: "eax", "edx"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.Write(InstRegEdx, edx);
	regs.Write(InstRegEax, eax);

	newUInst(UInstMove, 0, 0, 0, UInstDepEax, UInstDepEdx, 0, 0);
}


void Context::ExecuteInst_ret()
{
	if (inst.getSegment())
		throw misc::Panic("Unsupported segment");

	// Emulate
	MemoryRead(regs.getEsp(), 4, &target_eip);
	regs.incEsp(4);
	regs.setEip(target_eip);

	// Call stack
	if (call_stack != nullptr)
		call_stack->Return(regs.getEip(), regs.getEsp());

	// Micro-instrutcions
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4, 4, UInstDepAux, 0, 0, UInstDepAux, 0, 0, 0);  // pop aux
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);  // add esp, 4
	newUInst(UInstRet, UInstDepAux, 0, 0, 0, 0, 0, 0);  // jmp aux
}


void Context::ExecuteInst_repz_ret()
{
	ExecuteInst_ret();
}


void Context::ExecuteInst_ret_imm16()
{
	if (inst.getSegment())
		throw misc::Panic("Unsupported segment");

	// Emulate
	MemoryRead(regs.getEsp(), 4, &target_eip);
	unsigned short pop = inst.getImmWord();
	regs.incEsp(4 + pop);
	regs.setEip(target_eip);

	// Call stack
	if (call_stack != nullptr)
		call_stack->Return(regs.getEip(), regs.getEsp());

	// Micro-instructions
	newUInst(UInstEffaddr, UInstDepEsp, 0, 0, UInstDepAux, 0, 0, 0);
	newMemoryUInst(UInstLoad, regs.getEsp() - 4 - pop, 4, UInstDepAux, 0, 0, UInstDepAux, 0, 0, 0);  // pop aux
	newUInst(UInstAdd, UInstDepEsp, 0, 0, UInstDepEsp, 0, 0, 0);  // add esp, 4
	newUInst(UInstRet, UInstDepAux, 0, 0, 0, 0, 0, 0);  // jmp aux
}


void Context::ExecuteInst_sahf()
{
	regs.setEflags(regs.getEflags() & ~0xff);
	regs.setEflags(regs.getEflags() | regs.Read(InstRegAh));
	regs.setEflags(regs.getEflags() & ~0x28);
	regs.setEflags(regs.getEflags() | 0x2);

	newUInst(UInstMove, UInstDepEax, 0, 0, 0, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_sfence()
{
}


void Context::ExecuteInst_shld_rm16_r16_imm8()
{
	unsigned short rm16 = LoadRm16();
	unsigned short r16 = LoadR16();
	unsigned char imm8 = inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%bx\n\t"
		"mov %2, %%ax\n\t"
		"shld %%cl, %%bx, %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm16)
		: "m" (rm16), "m" (r16), "m" (imm8), "g" (flags)
		: "ax", "bx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm16(rm16);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm16, UInstDepR16, 0, UInstDepRm16, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_shld_rm16_r16_cl()
{
	unsigned short rm16 = LoadRm16();
	unsigned short r16 = LoadR16();
	unsigned char cl = regs.Read(InstRegCl);
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%bx\n\t"
		"mov %2, %%ax\n\t"
		"shld %%cl, %%bx, %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm16)
		: "m" (rm16), "m" (r16), "m" (cl), "g" (flags)
		: "ax", "bx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm16(rm16);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm16, UInstDepR16, UInstDepEcx,
		UInstDepRm16, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_shld_rm32_r32_imm8()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned char imm8 = inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shld %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (imm8), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, UInstDepR32, 0, UInstDepRm32, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_shld_rm32_r32_cl()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned char cl = regs.Read(InstRegCl);
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shld %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (cl), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, UInstDepR32, UInstDepEcx,
		UInstDepRm32, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_shrd_rm32_r32_imm8()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned char imm8 = inst.getImmByte();
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shrd %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (imm8), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, UInstDepR32, 0, UInstDepRm32, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_shrd_rm32_r32_cl()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned char cl = regs.Read(InstRegCl);
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shrd %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (cl), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreRm32(rm32);
	regs.setEflags(flags);

	newUInst(UInstShift, UInstDepRm32, UInstDepR32, UInstDepEcx,
		UInstDepRm32, UInstDepZps, UInstDepCf, UInstDepOf);
}


void Context::ExecuteInst_std()
{
	regs.setFlag(InstFlagDF);

	newUInst(UInstMove, 0, 0, 0, 0, UInstDepDf, 0, 0);
}


void Context::ExecuteInst_xadd_rm8_r8()
{
	unsigned char rm8 = LoadRm8();
	unsigned char r8 = LoadR8();
	unsigned char sum;
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"add %3, %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (sum)
		: "m" (rm8), "m" (r8), "g" (flags)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR8(rm8);
	StoreRm8(sum);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepRm8, UInstDepR8, 0, UInstDepAux, UInstDepZps, UInstDepCf, UInstDepOf);
	newUInst(UInstMove, UInstDepAux, 0, 0, UInstDepRm8, 0, 0, 0);
	newUInst(UInstMove, UInstDepAux, 0, 0, UInstDepR8, 0, 0, 0);
}


void Context::ExecuteInst_xadd_rm32_r32()
{
	unsigned int rm32 = LoadRm32();
	unsigned int r32 = LoadR32();
	unsigned int sum;
	unsigned long flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (sum)
		: "m" (rm32), "m" (r32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	StoreR32(rm32);
	StoreRm32(sum);
	regs.setEflags(flags);

	newUInst(UInstAdd, UInstDepRm32, UInstDepR32, 0, UInstDepAux, UInstDepZps, UInstDepCf, UInstDepOf);
	newUInst(UInstMove, UInstDepAux, 0, 0, UInstDepRm32, 0, 0, 0);
	newUInst(UInstMove, UInstDepAux, 0, 0, UInstDepR32, 0, 0, 0);
}


void Context::ExecuteInst_xchg_ir16_ax()
{
	unsigned short ax, ir16;

	ax = regs.Read(InstRegAx);
	ir16 = LoadIR16();
	regs.Write(InstRegAx, ir16);
	StoreIR16(ax);

	newUInst(UInstMove, UInstDepIr16, UInstDepEax, 0, UInstDepIr16, UInstDepEax, 0, 0);
}


void Context::ExecuteInst_xchg_ir32_eax()
{
	unsigned int eax, ir32;

	eax = regs.Read(InstRegEax);
	ir32 = LoadIR32();
	regs.Write(InstRegEax, ir32);
	StoreIR32(eax);

	newUInst(UInstMove, UInstDepIr32, UInstDepEax, 0, UInstDepIr32, UInstDepEax, 0, 0);
}


void Context::ExecuteInst_xchg_rm8_r8()
{
	unsigned char rm8;
	unsigned char r8;

	rm8 = LoadRm8();
	r8 = LoadR8();
	StoreRm8(r8);
	StoreR8(rm8);

	newUInst(UInstMove, UInstDepRm8, UInstDepR8, 0, UInstDepRm8, UInstDepR8, 0, 0);
}


void Context::ExecuteInst_xchg_rm16_r16()
{
	unsigned short rm16;
	unsigned short r16;

	rm16 = LoadRm16();
	r16 = LoadR16();
	StoreRm16(r16);
	StoreR16(rm16);

	newUInst(UInstMove, UInstDepRm16, UInstDepR16, 0, UInstDepRm16, UInstDepR16, 0, 0);
}


void Context::ExecuteInst_xchg_rm32_r32()
{
	unsigned int rm32;
	unsigned int r32;

	rm32 = LoadRm32();
	r32 = LoadR32();
	StoreRm32(r32);
	StoreR32(rm32);

	newUInst(UInstMove, UInstDepRm32, UInstDepR32, 0, UInstDepRm32, UInstDepR32, 0, 0);
}



}  // namespace x86

