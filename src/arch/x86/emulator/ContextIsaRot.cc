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




#define op_xxx_rm8_1_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm8_1() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char count = 1; \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm8(rm8); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm8, \
			0, \
			Uinst::DepRm8, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm8_cl_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm8_cl() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char count = regs.Read(Instruction::RegCl); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm8(rm8); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm8, \
			Uinst::DepEcx, \
			Uinst::DepRm8, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm8_imm8_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm8_imm8() \
{ \
	unsigned char rm8 = LoadRm8(); \
	unsigned char count = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm8(rm8); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm8, \
			0, \
			Uinst::DepRm8, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm16_1_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm16_1() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned char count = 1; \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm16(rm16); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm16, \
			0, \
			Uinst::DepRm16, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm16_cl_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm16_cl() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned char count = regs.Read(Instruction::RegCl); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm16(rm16); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm16, \
			Uinst::DepEcx, \
			Uinst::DepRm16, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm16_imm8_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm16_imm8() \
{ \
	unsigned short rm16 = LoadRm16(); \
	unsigned char count = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm16(rm16); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm16, \
			0, \
			Uinst::DepRm16, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm32_1_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm32_1() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned char count = 1; \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm32(rm32); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm32, \
			0, \
			Uinst::DepRm32, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm32_cl_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm32_cl() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned char count = regs.Read(Instruction::RegCl); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm32(rm32); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm32, \
			Uinst::DepEcx, \
			Uinst::DepRm32, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


#define op_xxx_rm32_imm8_impl(xxx, idep) \
void Context::ExecuteInst_##xxx##_rm32_imm8() \
{ \
	unsigned int rm32 = LoadRm32(); \
	unsigned char count = inst.getImmByte(); \
	unsigned long flags = regs.getEflags(); \
	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_CONTEXT_RESTORE_FLAGS__ \
	StoreRm32(rm32); \
	regs.setEflags(flags); \
	newUinst(Uinst::OpcodeShift, \
			idep, \
			Uinst::DepRm32, \
			0, \
			Uinst::DepRm32, \
			Uinst::DepCf, \
			Uinst::DepOf, \
			0); \
}


// Define all possible combinations of 8, 16 and 32 bit destination registers
// and 1, cl or imm8 source operand.
#define op_xxx_all(xxx, idep) \
	op_xxx_rm8_1_impl(xxx, idep) \
	op_xxx_rm8_cl_impl(xxx, idep) \
	op_xxx_rm8_imm8_impl(xxx, idep) \
	op_xxx_rm16_1_impl(xxx, idep) \
	op_xxx_rm16_cl_impl(xxx, idep) \
	op_xxx_rm16_imm8_impl(xxx, idep) \
	op_xxx_rm32_1_impl(xxx, idep) \
	op_xxx_rm32_cl_impl(xxx, idep) \
	op_xxx_rm32_imm8_impl(xxx, idep)


// Implementation for rotation and shift instructions
op_xxx_all(rcl, Uinst::DepCf)
op_xxx_all(rcr, Uinst::DepCf)
op_xxx_all(rol, 0)
op_xxx_all(ror, 0)

op_xxx_all(sar, 0)
op_xxx_all(shl, 0)
op_xxx_all(shr, 0)



}  // namespace x86

