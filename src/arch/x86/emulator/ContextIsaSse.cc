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

#define __UNIMPLEMENTED__ throw misc::Panic(misc::fmt("Unimplemented instruction %s", __FUNCTION__));

void Context::ExecuteInst_addps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	this->LoadXmm(dest);
	this->LoadXmmM128(src);

	int spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"addps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	this->StoreXmm(dest);

	this->newUinst(Uinst::OpcodeFpAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_addss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"addss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpAdd,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_andps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"andps %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmAnd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


#define X86_ISA_CMPP_ASM(__inst) \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"movdqu %1, %%xmm0\n\t" \
		"movdqu %0, %%xmm1\n\t" \
		__inst " %%xmm0, %%xmm1\n\t" \
		"movdqu %%xmm1, %0\n\t" \
		: "=m" (dest) \
		: "m" (src) \
		: "xmm0", "xmm1" \
	); \
	__X86_ISA_ASM_END__

void Context::ExecuteInst_cmppd_xmm_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;
	int imm8 = inst.getImmByte();

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		switch (imm8)
		{
		case 0: X86_ISA_CMPP_ASM("cmpeqpd"); break;
		case 1: X86_ISA_CMPP_ASM("cmpltpd"); break;
		case 2: X86_ISA_CMPP_ASM("cmplepd"); break;
		case 3: X86_ISA_CMPP_ASM("cmpunordpd"); break;
		case 4: X86_ISA_CMPP_ASM("cmpneqpd"); break;
		case 5: X86_ISA_CMPP_ASM("cmpnltpd"); break;
		case 6: X86_ISA_CMPP_ASM("cmpnlepd"); break;
		case 7: X86_ISA_CMPP_ASM("cmpordpd"); break;
		default:
			throw misc::Error(misc::fmt(
					"%s: invalid value for 'imm8'",
					__FUNCTION__));
		}
	}

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cmpps_xmm_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;
	int imm8 = inst.getImmByte();

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		switch (imm8)
		{
		case 0: X86_ISA_CMPP_ASM("cmpeqps"); break;
		case 1: X86_ISA_CMPP_ASM("cmpltps"); break;
		case 2: X86_ISA_CMPP_ASM("cmpleps"); break;
		case 3: X86_ISA_CMPP_ASM("cmpunordps"); break;
		case 4: X86_ISA_CMPP_ASM("cmpneqps"); break;
		case 5: X86_ISA_CMPP_ASM("cmpnltps"); break;
		case 6: X86_ISA_CMPP_ASM("cmpnleps"); break;
		case 7: X86_ISA_CMPP_ASM("cmpordps"); break;
		default:
			throw misc::Error(misc::fmt(
					"%s: invalid value for 'imm8'",
					__FUNCTION__));
		}
	}

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtsi2ss_xmm_rm32()
{
	XmmValue dest;
	unsigned int src;

	int spec_mode;

	src = LoadRm32();
	LoadXmm(dest);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %0, %%xmm0\n\t"
			"mov %1, %%eax\n\t"
			"cvtsi2ss %%eax, %%xmm0\n\t"
			"movdqu %%xmm0, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "eax"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmm,
			Uinst::DepRm32,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvttss2si_r32_xmmm32()
{
	XmmValue xmm;
	unsigned int r32;

	int spec_mode;

	LoadXmmM32(xmm);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"cvttss2si %1, %%eax\n\t"
			"mov %%eax, %0"
			: "=m" (r32)
			: "m" (xmm)
			: "eax"
		);
		__X86_ISA_ASM_END__
	}

	StoreR32(r32);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm32,
			0,
			0,
			Uinst::DepR32,
			0,
			0,
			0);
}


void Context::ExecuteInst_divps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"divps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpDiv,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_divss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"divss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpDiv,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_ldmxcsr_m32()
{
	__UNIMPLEMENTED__
}


void Context::ExecuteInst_maxps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"maxps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_maxss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"maxss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_minps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"minps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_minss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"minss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movaps_xmm_xmmm128()
{
	XmmValue xmm;

	LoadXmmM128(xmm);
	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movaps_xmmm128_xmm()
{
	XmmValue xmm;

	LoadXmm(xmm);
	StoreXmmM128(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_movhlps_xmm_xmmm128()
{
	XmmValue xmm1;
	XmmValue xmm2;

	LoadXmm(xmm1);
	LoadXmmM128(xmm2);
	xmm1.setAsUInt64(0, xmm2.getAsUInt64(1));
	StoreXmm(xmm1);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movlhps_xmm_xmmm128()
{
	XmmValue xmm1;
	XmmValue xmm2;

	LoadXmm(xmm1);
	LoadXmmM128(xmm2);
	xmm1.setAsUInt64(1, xmm2.getAsUInt64(0));
	StoreXmm(xmm1);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movss_xmm_xmmm32()
{
	XmmValue value;

	/* xmm <= m32: bits 127-32 of xmm set to 0.
	 * xmm <= xmm: bits 127-32 unmodified */
	if (inst.getModRmMod() == 3)
		LoadXmm(value);
	else
		value.setWithMemset(0, 0, 16);
	LoadXmmM32(value);
	StoreXmm(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm32,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movss_xmmm32_xmm()
{
	XmmValue value;

	/* xmm <= xmm: bits 127-32 unmodified.
	 * m32 <= xmm: copy 32 bits to memory */
	LoadXmm(value);
	StoreXmmM32(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm32,
			0,
			0,
			0);
}


void Context::ExecuteInst_movups_xmm_xmmm128()
{
	XmmValue xmm;

	LoadXmmM128(xmm);
	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movups_xmmm128_xmm()
{
	XmmValue xmm;

	LoadXmm(xmm);
	StoreXmmM128(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_mulps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"mulps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpMult,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_mulss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"mulss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpMult,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_orps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"orps %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmOr,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pextrw_r32_xmmm128_imm8()
{
	XmmValue xmm;
	unsigned char imm8;
	unsigned int result;

	/* Get 'xmm' source */
	LoadXmmM128(xmm);

	/* Take the 3 LSB of 'imm8' */
	imm8 = inst.getImmByte() & 0x7;

	/* Select 2-byte word from 'xmm' pointed to by 'imm8',
	 * and place it in the 2 LSBytes of 'result' */
	result = xmm.getAsUShort(imm8);
	StoreR32(result);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepR32,
			0,
			0,
			0);
}


void Context::ExecuteInst_pinsrw_xmm_r32m16_imm8()
{
	XmmValue xmm;
	unsigned short r32m16;
	unsigned char imm8;

	LoadXmm(xmm);
	r32m16 = LoadR32M16();
	imm8 = inst.getImmByte() & 0x7;

	/* Update word in 'xmm' */
	xmm.setAsUShort(imm8, r32m16);
	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmm,
			Uinst::DepRm32,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pmovmskb_r32_xmmm128()
{
	XmmValue src;
	unsigned int r32;

	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"pmovmskb %%xmm0, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (src)
		: "xmm0", "eax"
	);
	__X86_ISA_ASM_END__

	StoreR32(r32);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepR32,
			0,
			0,
			0);
}


void Context::ExecuteInst_shufps_xmm_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;
	unsigned char imm;

	LoadXmm(dest);
	LoadXmmM128(src);
	imm = inst.getImmByte();

	dest.setAsInt(0, dest.getAsInt(imm & 3));
	dest.setAsInt(1, dest.getAsInt((imm >> 2) & 3));
	dest.setAsInt(2, src.getAsInt((imm >> 4) & 3));
	dest.setAsInt(3, src.getAsInt((imm >> 6) & 3));

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShuf,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_sqrtps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"sqrtps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSqrt,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_sqrtss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"sqrtss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSqrt,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_stmxcsr_m32()
{
	__UNIMPLEMENTED__
}


void Context::ExecuteInst_subps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"subps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSub,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_subss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"subss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSub,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_ucomisd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	unsigned long eflags = regs.getEflags();

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"push %3\n\t"
			"popf\n\t"
			"movdqu %2, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"ucomisd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			"pushf\n\t"
			"pop %1\n\t"
			: "=m" (dest), "=g" (eflags)
			: "m" (src), "g" (eflags)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);
	regs.setEflags(eflags);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepZps,
			Uinst::DepCf,
			Uinst::DepOf,
			0);
}


void Context::ExecuteInst_ucomiss_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	unsigned long eflags = regs.getEflags();

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"push %3\n\t"
			"popf\n\t"
			"movdqu %2, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"ucomiss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			"pushf\n\t"
			"pop %1\n\t"
			: "=m" (dest), "=g" (eflags)
			: "m" (src), "g" (eflags)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);
	regs.setEflags(eflags);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepZps,
			Uinst::DepCf,
			Uinst::DepOf,
			0);
}


void Context::ExecuteInst_unpckhps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"unpckhps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShuf,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_unpcklps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = this->getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"unpcklps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShuf,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_xorps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"xorps %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmXor,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}

}  // namespace x86

