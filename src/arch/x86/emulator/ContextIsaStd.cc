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


#define op_stdop_al_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_al_imm8() \
{ \
	unsigned char al = regs.Read(InstRegAl); \
	unsigned char imm8 = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (al) \
		: "m" (al), "m" (imm8), "g" (flags) \
		: "al" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		regs.Write(InstRegAl, al); \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepEax, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_ax_imm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_ax_imm16() \
{ \
	unsigned short ax = regs.Read(InstRegAx); \
	unsigned short imm16 = inst.getImmWord(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (ax) \
		: "m" (ax), "m" (imm16), "g" (flags) \
		: "ax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		regs.Write(InstRegAx, ax); \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepEax, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_eax_imm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_eax_imm32() \
{ \
	unsigned int eax = regs.Read(InstRegEax); \
	unsigned int imm32 = inst.getImmDWord(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (eax) \
		: "m" (eax), "m" (imm32), "g" (flags) \
		: "eax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		regs.Write(InstRegEax, eax); \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepEax, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepEax, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm8_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm8_imm8() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char imm8 = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (imm8), "g" (flags) \
		: "al" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm8(rm8); \
		newUInst(uinst, UInstDepRm8, cin_dep, 0, UInstDepRm8, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm8, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_imm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_imm16() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short imm16 = inst.getImmWord(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (imm16), "g" (flags) \
		: "ax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm16(rm16); \
		newUInst(uinst, UInstDepRm16, cin_dep, 0, UInstDepRm16, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm16, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_imm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_imm32() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int imm32 = inst.getImmDWord(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (imm32), "g" (flags) \
		: "eax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm32(rm32); \
		newUInst(uinst, UInstDepRm32, cin_dep, 0, UInstDepRm32, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm32, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_imm8() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short imm8 = (char) inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (imm8), "g" (flags) \
		: "ax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm16(rm16); \
		newUInst(uinst, UInstDepRm16, cin_dep, 0, UInstDepRm16, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm16, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_imm8() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int imm8 = (char) inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (imm8), "g" (flags) \
		: "eax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm32(rm32); \
		newUInst(uinst, UInstDepRm32, cin_dep, 0, UInstDepRm32, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm32, cin_dep, 0, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm8_r8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm8_r8() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char r8 = LoadR8(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (r8), "g" (flags) \
		: "al" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm8(rm8); \
		newUInst(uinst, UInstDepRm8, UInstDepR8, cin_dep, UInstDepRm8, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm8, UInstDepR8, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_r16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_r16() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short r16 = LoadR16(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (r16), "g" (flags) \
		: "ax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm16(rm16); \
		newUInst(uinst, UInstDepRm16, UInstDepR16, cin_dep, UInstDepRm16, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepRm16, UInstDepR16, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_r32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_r32() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int r32 = LoadR32(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (r32), "g" (flags) \
		: "eax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreRm32(rm32); \
		newUInst(uinst, UInstDepRm32, UInstDepR32, cin_dep, UInstDepRm32, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else  { \
		newUInst(uinst, UInstDepRm32, UInstDepR32, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r8_rm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r8_rm8() \
{ \
	unsigned char r8 = LoadR8(); \
	unsigned char rm8 = LoadRm8(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (r8) \
		: "m" (r8), "m" (rm8), "g" (flags) \
		: "al" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreR8(r8); \
		newUInst(uinst, UInstDepR8, UInstDepRm8, cin_dep, UInstDepR8, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepR8, UInstDepRm8, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r16_rm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r16_rm16() \
{ \
	unsigned short r16 = LoadR16(); \
	unsigned short rm16 = LoadRm16(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (r16) \
		: "m" (r16), "m" (rm16), "g" (flags) \
		: "ax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreR16(r16); \
		newUInst(uinst, UInstDepR16, UInstDepRm16, cin_dep, UInstDepR16, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepR16, UInstDepRm16, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r32_rm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r32_rm32() \
{ \
	unsigned int r32 = LoadR32(); \
	unsigned int rm32 = LoadRm32(); \
	unsigned long flags = regs.getEflags(); \
	UInstDep cin_dep = cin ? UInstDepCf : UInstDepNone; \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (r32) \
		: "m" (r32), "m" (rm32), "g" (flags) \
		: "eax" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	if (wb) { \
		StoreR32(r32); \
		newUInst(uinst, UInstDepR32, UInstDepRm32, cin_dep, UInstDepR32, \
				UInstDepZps, UInstDepCf, UInstDepOf); \
	} else { \
		newUInst(uinst, UInstDepR32, UInstDepRm32, cin_dep, UInstDepZps, \
				UInstDepCf, UInstDepOf, 0); \
	} \
	regs.setEflags(flags); \
}


// Macro to generate all 14 variants of a standard arithmetic instruction.
//
// The 'wb' (write-back) argument should be set if the instruction saves its
// output to the destination operand (e.g., 'add' does, but 'cmp' does not).
//
// The 'cin' (carry-in) argument should be set for those instructions that
// have flag CF as an additional input dependence ('adc' and 'sbb').

#define op_stdop_all(stdop, wb, cin, uinst) \
	op_stdop_al_imm8(stdop, wb, cin, uinst) \
	op_stdop_ax_imm16(stdop, wb, cin, uinst) \
	op_stdop_eax_imm32(stdop, wb, cin, uinst) \
	op_stdop_rm8_imm8(stdop, wb, cin, uinst) \
	op_stdop_rm16_imm16(stdop, wb, cin, uinst) \
	op_stdop_rm32_imm32(stdop, wb, cin, uinst) \
	op_stdop_rm16_imm8(stdop, wb, cin, uinst) \
	op_stdop_rm32_imm8(stdop, wb, cin, uinst) \
	op_stdop_rm8_r8(stdop, wb, cin, uinst) \
	op_stdop_rm16_r16(stdop, wb, cin, uinst) \
	op_stdop_rm32_r32(stdop, wb, cin, uinst) \
	op_stdop_r8_rm8(stdop, wb, cin, uinst) \
	op_stdop_r16_rm16(stdop, wb, cin, uinst) \
	op_stdop_r32_rm32(stdop, wb, cin, uinst)


/* Generate all standard arithmetic instructions. */
op_stdop_all(adc, 1, 1, UInstAdd)
op_stdop_all(add, 1, 0, UInstAdd)
op_stdop_all(and, 1, 0, UInstAnd)
op_stdop_all(cmp, 0, 0, UInstSub)
op_stdop_all(or, 1, 0, UInstOr)
op_stdop_all(sbb, 1, 1, UInstSub)
op_stdop_all(sub, 1, 0, UInstSub)
op_stdop_all(test, 0, 0, UInstAnd)
op_stdop_all(xor, 1, 0, UInstXor)


}  // namespace x86

