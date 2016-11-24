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
	unsigned char al = regs.Read(Instruction::RegAl); \
	unsigned char imm8 = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		regs.Write(Instruction::RegAl, al); \
		newUinst(uinst, \
				Uinst::DepEax, \
				cin_dep, \
				0, \
				Uinst::DepEax, \
				Uinst::DepZps, \
				Uinst::DepCf, \
				Uinst::DepOf); \
	} else { \
		newUinst(uinst, \
				Uinst::DepEax, \
				cin_dep, \
				0, \
				Uinst::DepZps, \
				Uinst::DepCf, \
				Uinst::DepOf, \
				0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_ax_imm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_ax_imm16() \
{ \
	unsigned short ax = regs.Read(Instruction::RegAx); \
	unsigned short imm16 = inst.getImmWord(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		regs.Write(Instruction::RegAx, ax); \
		newUinst(uinst, Uinst::DepEax, cin_dep, 0, Uinst::DepEax, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepEax, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_eax_imm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_eax_imm32() \
{ \
	unsigned int eax = regs.Read(Instruction::RegEax); \
	unsigned int imm32 = inst.getImmDWord(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		regs.Write(Instruction::RegEax, eax); \
		newUinst(uinst, Uinst::DepEax, cin_dep, 0, Uinst::DepEax, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepEax, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm8_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm8_imm8() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char imm8 = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm8, cin_dep, 0, Uinst::DepRm8, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm8, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_imm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_imm16() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short imm16 = inst.getImmWord(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm16, cin_dep, 0, Uinst::DepRm16, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm16, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_imm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_imm32() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int imm32 = inst.getImmDWord(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm32, cin_dep, 0, Uinst::DepRm32, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm32, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_imm8() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short imm8 = (char) inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm16, cin_dep, 0, Uinst::DepRm16, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm16, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_imm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_imm8() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int imm8 = (char) inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm32, cin_dep, 0, Uinst::DepRm32, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm32, cin_dep, 0, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm8_r8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm8_r8() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char r8 = LoadR8(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm8, Uinst::DepR8, cin_dep, Uinst::DepRm8, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm8, Uinst::DepR8, cin_dep, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm16_r16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm16_r16() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned short r16 = LoadR16(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm16, Uinst::DepR16, cin_dep, Uinst::DepRm16, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepRm16, Uinst::DepR16, cin_dep, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_rm32_r32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_rm32_r32() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned int r32 = LoadR32(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepRm32, Uinst::DepR32, cin_dep, Uinst::DepRm32, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else  { \
		newUinst(uinst, Uinst::DepRm32, Uinst::DepR32, cin_dep, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r8_rm8(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r8_rm8() \
{ \
	unsigned char r8 = LoadR8(); \
	unsigned char rm8 = LoadRm8(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepR8, Uinst::DepRm8, cin_dep, Uinst::DepR8, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepR8, Uinst::DepRm8, cin_dep, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r16_rm16(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r16_rm16() \
{ \
	unsigned short r16 = LoadR16(); \
	unsigned short rm16 = LoadRm16(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, Uinst::DepR16, Uinst::DepRm16, cin_dep, Uinst::DepR16, \
				Uinst::DepZps, Uinst::DepCf, Uinst::DepOf); \
	} else { \
		newUinst(uinst, Uinst::DepR16, Uinst::DepRm16, cin_dep, Uinst::DepZps, \
				Uinst::DepCf, Uinst::DepOf, 0); \
	} \
	regs.setEflags(flags); \
}


#define op_stdop_r32_rm32(stdop, wb, cin, uinst) \
void Context::ExecuteInst_##stdop##_r32_rm32() \
{ \
	unsigned int r32 = LoadR32(); \
	unsigned int rm32 = LoadRm32(); \
	unsigned long flags = regs.getEflags(); \
	Uinst::Dep cin_dep = cin ? Uinst::DepCf : Uinst::DepNone; \
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
		newUinst(uinst, \
				Uinst::DepR32, \
				Uinst::DepRm32, \
				cin_dep, \
				Uinst::DepR32, \
				Uinst::DepZps, \
				Uinst::DepCf, \
				Uinst::DepOf); \
	} else { \
		newUinst(uinst, \
				Uinst::DepR32, \
				Uinst::DepRm32, \
				cin_dep, \
				Uinst::DepZps, \
				Uinst::DepCf, \
				Uinst::DepOf, \
				0); \
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
op_stdop_all(adc, 1, 1, Uinst::OpcodeAdd)
op_stdop_all(add, 1, 0, Uinst::OpcodeAdd)
op_stdop_all(and, 1, 0, Uinst::OpcodeAnd)
op_stdop_all(cmp, 0, 0, Uinst::OpcodeSub)
op_stdop_all(or, 1, 0, Uinst::OpcodeOr)
op_stdop_all(sbb, 1, 1, Uinst::OpcodeSub)
op_stdop_all(sub, 1, 0, Uinst::OpcodeSub)
op_stdop_all(test, 0, 0, Uinst::OpcodeAnd)
op_stdop_all(xor, 1, 0, Uinst::OpcodeXor)


}  // namespace x86

