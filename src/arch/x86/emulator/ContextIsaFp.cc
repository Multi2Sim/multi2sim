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
#include "Emulator.h"


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


void Context::ExecuteInst_f2xm1()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"f2xm1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpExp,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fabs()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fabs\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSign,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fadd_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"flds %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpAdd,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fadd_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpAdd,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fadd_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpAdd,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fadd_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpAdd,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_faddp_sti_st0()
{
	ExecuteInst_fadd_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fchs()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fchs\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSign,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovb_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (regs.getFlag(Instruction::FlagCF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepCf,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmove_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (regs.getFlag(Instruction::FlagZF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepZps,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovbe_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (regs.getFlag(Instruction::FlagCF) || regs.getFlag(Instruction::FlagZF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepCf,
			Uinst::DepZps,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovu_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (regs.getFlag(Instruction::FlagPF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepZps,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovnb_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (!regs.getFlag(Instruction::FlagCF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepCf,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovne_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (!regs.getFlag(Instruction::FlagZF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepZps,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovnbe_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (!regs.getFlag(Instruction::FlagCF) && !regs.getFlag(Instruction::FlagZF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepCf,
			Uinst::DepZps,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcmovnu_st0_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	if (!regs.getFlag(Instruction::FlagPF))
		StoreFpu(0, sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			Uinst::DepZps,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcom_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	LoadFpu(0, st0);
	m32 = LoadFloat();

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fcomps %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcom_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	LoadFpu(0, st0);
	m64 = LoadDouble();

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fcompl %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcom_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcomp_m32()
{
	ExecuteInst_fcom_m32();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcomp_m64()
{
	ExecuteInst_fcom_m64();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcomp_sti()
{
	ExecuteInst_fcom_sti();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcompp()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);
	PopFpu(nullptr);
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop, 0, 0, 0, 0, 0, 0, 0);
	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcomi_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned long flags = regs.getEflags();

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	__X86_CONTEXT_FP_END__

	regs.setEflags(flags);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepZps,
			Uinst::DepCf,
			0,
			0);
}


void Context::ExecuteInst_fcomip_st0_sti()
{
	ExecuteInst_fcomi_st0_sti();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fucomi_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned long flags = regs.getEflags();

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	__X86_CONTEXT_FP_END__

	regs.setEflags(flags);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepZps,
			Uinst::DepCf,
			0,
			0);
}


void Context::ExecuteInst_fucomip_st0_sti()
{
	ExecuteInst_fucomi_st0_sti();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fcos()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fcos\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpCos,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdiv_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"flds %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdiv_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdiv_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdiv_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivp_sti_st0()
{
	ExecuteInst_fdiv_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivr_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"flds %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m32), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivr_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldl %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m64), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivr_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeDiv,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivr_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeDiv,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fdivrp_sti_st0()
{
	ExecuteInst_fdivr_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fild_m16()
{
	short m16;
	unsigned char e[10];

	MemoryRead(getEffectiveAddress(), 2, &m16);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"filds %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m16)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem16, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fild_m32()
{
	int m32;
	unsigned char e[10];

	MemoryRead(getEffectiveAddress(), 4, &m32);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fildl %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m32)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem32, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fild_m64()
{
	long long m64;
	unsigned char e[10];

	MemoryRead(getEffectiveAddress(), 8, &m64);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fildq %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m64)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem64, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fist_m16()
{
	short m16;
	unsigned char e[10];

	LoadFpu(0, e);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fistps %0\n\t"
		: "=m" (m16)
		: "m" (*e)
	);
	__X86_CONTEXT_FP_END__

	MemoryWrite(getEffectiveAddress(), 2, &m16);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem16,
			0,
			0,
			0);
}


void Context::ExecuteInst_fist_m32()
{
	int m32;
	unsigned char e[10];

	LoadFpu(0, e);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fistpl %0\n\t"
		: "=m" (m32)
		: "m" (*e)
	);
	__X86_CONTEXT_FP_END__

	MemoryWrite(getEffectiveAddress(), 4, &m32);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem32,
			0,
			0,
			0);
}


void Context::ExecuteInst_fist_m64()
{
	long long m64;
	unsigned char e[10];

	LoadFpu(0, e);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fistpq %0\n\t"
		: "=m" (m64)
		: "m" (*e)
	);
	__X86_CONTEXT_FP_END__

	MemoryWrite(getEffectiveAddress(), 8, &m64);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem64,
			0,
			0,
			0);
}


void Context::ExecuteInst_fistp_m16()
{
	ExecuteInst_fist_m16();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fistp_m32()
{
	ExecuteInst_fist_m32();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fistp_m64()
{
	ExecuteInst_fist_m64();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fld1()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fld1\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldl2e()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldl2e\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldl2t()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldl2t\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldpi()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldpi\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldlg2()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldlg2\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldln2()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldln2\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fldz()
{
	unsigned char v[10];

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldz\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_CONTEXT_FP_END__

	PushFpu(v);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, 0, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fld_m32()
{
	unsigned char e[10];
	float m32;

	m32 = LoadFloat();
	Extended::FloatToExtended(m32, e);
	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem32, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fld_m64()
{
	unsigned char e[10];
	double m64;

	m64 = LoadDouble();
	Extended::DoubleToExtended(m64, e);
	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem64, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fld_m80()
{
	unsigned char e[10];

	LoadExtended(e);
	PushFpu(e);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpMove, Uinst::DepMem80, 0, 0, Uinst::DepSt0, 0, 0, 0);
}


void Context::ExecuteInst_fld_sti()
{
	unsigned char sti[10];
	LoadFpu(inst.getOpIndex(), sti);
	PushFpu(sti);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSti,
			0,
			0,
			Uinst::DepFpaux,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPush, 0, 0, 0, 0, 0, 0, 0);
	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepFpaux,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fldcw_m16()
{
	unsigned int addr;
	unsigned short value;
	int spec_mode;

	addr = getEffectiveAddress();
	MemoryRead(addr, 2, &value);

	// Mask all floating-point exception on wrong path
	spec_mode = getState(StateSpecMode);
	if (spec_mode)
		value |= 0x3f;

	// Set value
	regs.setFpuCtrl(value);
	emulator->isa_debug << misc::fmt(" fpcw<=0x%x", value);

	// Micro-instructions
	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepMem16,
			0,
			0,
			Uinst::DepFpcw,
			0,
			0,
			0);
}


void Context::ExecuteInst_fmul_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"flds %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpMult,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fmul_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpMult,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fmul_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpMult,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fmul_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpMult,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fmulp_sti_st0()
{
	ExecuteInst_fmul_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fnstcw_m16()
{
	ExecuteInst_fstcw_m16();
}


void Context::ExecuteInst_fnstsw_ax()
{
	unsigned short status = LoadFpuStatus();
	regs.Write(Instruction::RegAx, status);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepFpst,
			0,
			0,
			Uinst::DepEax,
			0,
			0,
			0);
}


void Context::ExecuteInst_fpatan()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fpatan\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(1, st1);
	PopFpu(nullptr);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpAtan,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt1,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_fprem()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fprem\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fprem1()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fprem1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpDiv,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fptan()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fptan\n\t"
		"fnstsw %%ax\n\t"
		"fstp %%st(0)\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);

	newUinst(Uinst::OpcodeFpTan,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			0);

	ExecuteInst_fld1();
	StoreFpuCode(status);
}


void Context::ExecuteInst_frndint()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"frndint\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpRound,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fscale()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fscale\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpExp,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsin()
{
	unsigned char st0[10];
	short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fsin\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSin,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsincos()
{
	unsigned char st0[10], vsin[10], vcos[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fsin\n\t"
		"fstpt %0\n\t"
		"fldt %3\n\t"
		"fcos\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %1\n\t"
		"mov %%ax, %2\n\t"
		: "=m" (*vsin), "=m" (*vcos), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, vsin);
	PushFpu(vcos);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpPush,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpSincos, Uinst::DepSt1, 0, 0, Uinst::DepSt0, Uinst::DepSt1, 0, 0);
}


void Context::ExecuteInst_fsqrt()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fsqrt\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSqrt,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fst_m32()
{
	unsigned char st0[10];
	float m32;

	LoadFpu(0, st0);
	m32 = Extended::ExtendedToFloat(st0);
	StoreFloat(m32);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem32,
			0,
			0,
			0);
}


void Context::ExecuteInst_fst_m64()
{
	unsigned char st0[10];
	double m64;

	LoadFpu(0, st0);
	m64 = Extended::ExtendedToDouble(st0);
	StoreDouble(m64);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem64,
			0,
			0,
			0);
}


void Context::ExecuteInst_fst_sti()
{
	unsigned char st0[10];

	LoadFpu(0, st0);
	StoreFpu(inst.getOpIndex(), st0);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fstp_m32()
{
	ExecuteInst_fst_m32();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fstp_m64()
{
	ExecuteInst_fst_m64();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fstp_m80()
{
	unsigned char m80[10];

	PopFpu(m80);
	StoreExtended(m80);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepMem80,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_fstp_sti()
{
	ExecuteInst_fst_sti();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fstsw_ax()
{
	unsigned short status = LoadFpuStatus();
	regs.Write(Instruction::RegAx, status);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepFpst,
			0,
			0,
			Uinst::DepEax,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsub_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"flds %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsub_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsub_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsub_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubp_sti_st0()
{
	ExecuteInst_fsub_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubr_m32()
{
	unsigned char st0[10];
	float m32;
	unsigned short status;

	m32 = LoadFloat();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"flds %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m32), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepMem32,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubr_m64()
{
	unsigned char st0[10];
	double m64;
	unsigned short status;

	m64 = LoadDouble();
	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldl %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m64), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepMem64,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubr_st0_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(0, st0);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubr_sti_st0()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(inst.getOpIndex(), sti);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpSub,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSti,
			0,
			0,
			0);
}


void Context::ExecuteInst_fsubrp_sti_st0()
{
	ExecuteInst_fsubr_sti_st0();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fstcw_m16()
{
	unsigned short value = regs.getFpuCtrl();

	// Store value of FP control word
	unsigned address = getEffectiveAddress();
	MemoryWrite(address, 2, &value);
	emulator->isa_debug << misc::fmt(" [0x%x]<=0x%x", address, value);

	// Micro-instructions
	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepFpcw,
			0,
			0,
			Uinst::DepMem32,
			0,
			0,
			0);
}


void Context::ExecuteInst_ftst()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"ftst\n\t"
		"fnstsw %%ax\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fucom_sti()
{
	unsigned char st0[10], sti[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fucomp_sti()
{
	ExecuteInst_fucom_sti();
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fucompp()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);
	PopFpu(nullptr);
	PopFpu(nullptr);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop, 0, 0, 0, 0, 0, 0, 0);
	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}


void Context::ExecuteInst_fxam()
{
	unsigned char st0[10];
	unsigned short status;

	LoadFpu(0, st0);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %1\n\t"
		"fxam\n\t"
		"fstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		"fstp %%st(0)\n\t"
		: "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpComp,
			Uinst::DepSt0,
			0,
			0,
			Uinst::DepFpst,
			0,
			0,
			0);
}


void Context::ExecuteInst_fxch_sti()
{
	unsigned char st0[10], sti[10];
	LoadFpu(0, st0);
	LoadFpu(inst.getOpIndex(), sti);
	StoreFpu(0, sti);
	StoreFpu(inst.getOpIndex(), st0);

	newUinst(Uinst::OpcodeFpMove,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			Uinst::DepSt0,
			Uinst::DepSti,
			0,
			0);
}


void Context::ExecuteInst_fyl2x()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fyl2x\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(1, st1);
	PopFpu(nullptr);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpLog,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt1,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop, 0, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_fyl2xp1()
{
	unsigned char st0[10], st1[10];
	unsigned short status;

	LoadFpu(0, st0);
	LoadFpu(1, st1);

	__X86_CONTEXT_FP_BEGIN__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fyl2xp1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_CONTEXT_FP_END__

	StoreFpu(1, st1);
	PopFpu(nullptr);
	StoreFpuCode(status);

	newUinst(Uinst::OpcodeFpLog,
			Uinst::DepSt0,
			Uinst::DepSt1,
			0,
			Uinst::DepSt1,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeFpPop,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
}

}  // namespace x86

