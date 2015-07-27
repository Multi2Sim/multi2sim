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


void Context::ExecuteInst_addpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"addpd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_addsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"addsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpAdd,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_andpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"andpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_cvtdq2pd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtdq2pd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtdq2ps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtdq2ps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtps2pd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtps2pd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtsi2sd_xmm_rm32()
{
	XmmValue dest;
	unsigned int src;

	int spec_mode;

	src = LoadRm32();
	LoadXmm(dest);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %0, %%xmm0\n\t"
			"mov %1, %%eax\n\t"
			"cvtsi2sd %%eax, %%xmm0\n\t"
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


void Context::ExecuteInst_cvttsd2si_r32_xmmm64()
{
	XmmValue xmm;
	unsigned int r32;

	LoadXmmM64(xmm);

	__X86_ISA_ASM_START__
	asm volatile (
		"cvttsd2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (xmm)
		: "eax"
	);
	__X86_ISA_ASM_END__

	StoreR32(r32);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm64,
			0,
			0,
			Uinst::DepR32,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtsd2si_r32_xmmm64()
{
	XmmValue src;
	unsigned int r32;

	int spec_mode;

	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"cvtsd2si %%xmm0, %%eax\n\t"
			"mov %%eax, %0"
			: "=m" (r32)
			: "m" (src)
			: "xmm0", "eax"
		);
		__X86_ISA_ASM_END__
	}

	StoreR32(r32);
	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm64,
			0,
			0,
			Uinst::DepR32,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtsd2ss_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtsd2ss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtpd2ps_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtpd2ps %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvtss2sd_xmm_xmmm32()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM32(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvtss2sd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm32,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvttpd2dq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvttpd2dq %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_cvttps2dq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"cvttps2dq %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmConv,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_divpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"divpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_divsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"divsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpDiv,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_maxpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"maxpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_maxsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"maxsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_minpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"minpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_minsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"minsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpComp,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movapd_xmm_xmmm128()
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


void Context::ExecuteInst_movapd_xmmm128_xmm()
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


void Context::ExecuteInst_movd_xmm_rm32()
{
	XmmValue xmm;


	xmm.setWithMemset(0, 0, 16);
	xmm.setAsUInt(0, LoadRm32());

	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepRm32,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movd_rm32_xmm()
{
	XmmValue xmm;

	LoadXmm(xmm);
	StoreRm32(xmm.getAsUInt(0));

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepRm32,
			0,
			0,
			0);
}


void Context::ExecuteInst_movdqa_xmm_xmmm128()
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


void Context::ExecuteInst_movdqa_xmmm128_xmm()
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


void Context::ExecuteInst_movdqu_xmm_xmmm128()
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


void Context::ExecuteInst_movdqu_xmmm128_xmm()
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


void Context::ExecuteInst_movhpd_xmm_m64()
{
	XmmValue xmm;

	LoadXmm(xmm);
	xmm.setAsUInt64(1, LoadM64());
	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movhpd_m64_xmm()
{
	XmmValue xmm;

	LoadXmm(xmm);
	StoreM64(xmm.getAsUInt64(1));

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm64,
			0,
			0,
			0);
}


void Context::ExecuteInst_movlpd_xmm_m64()
{
	XmmValue xmm;

	LoadXmm(xmm);
	xmm.setAsUInt64(0, LoadM64());
	StoreXmm(xmm);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movlpd_m64_xmm()
{
	XmmValue xmm;

	LoadXmm(xmm);
	StoreM64(xmm.getAsUInt64(0));

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm64,
			0,
			0,
			0);
}


void Context::ExecuteInst_movmskpd_r32_xmmm128()
{
	XmmValue src;
	unsigned int r32;

	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movmskpd %%xmm0, %%eax\n\t"
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


void Context::ExecuteInst_movmskps_r32_xmmm128()
{
	XmmValue src;
	unsigned int r32;

	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movmskps %%xmm0, %%eax\n\t"
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


void Context::ExecuteInst_movntdq_m128_xmm()
{
	__UNIMPLEMENTED__
}


void Context::ExecuteInst_movq_xmm_xmmm64()
{
	XmmValue value;

	value.setWithMemset(0, 0, 16);
	LoadXmmM64(value);
	StoreXmm(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm64,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movq_xmmm64_xmm()
{
	Regs &regs = getRegs();
	XmmValue value;

	value.setWithMemset(0, 0, 16);
	if (inst.getModRmMod() == 3)
		regs.getXMM(inst.getModRmRm()).setWithMemcpy(&value);
	LoadXmm(value);
	StoreXmmM64(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm64,
			0,
			0,
			0);
}


void Context::ExecuteInst_movsd_xmm_xmmm64()
{
	XmmValue value;

	/* xmm <= m64: bits 127-64 of xmm set to 0.
	 * xmm <= xmm: bits 127-64 unmodified */
	if (inst.getModRmMod() == 3)
		LoadXmm(value);
	else
		value.setWithMemset(0, 0, 16);
	LoadXmmM64(value);
	StoreXmm(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmmm64,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_movsd_xmmm64_xmm()
{
	XmmValue value;

	/* xmm <= xmm: bits 127-64 unmodified.
	 * m64 <= xmm: copy 64 bits to memory */
	LoadXmm(value);
	StoreXmmM64(value);

	newUinst(Uinst::OpcodeXmmMove,
			Uinst::DepXmm,
			0,
			0,
			Uinst::DepXmmm64,
			0,
			0,
			0);
}


void Context::ExecuteInst_mulpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"mulpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_mulsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"mulsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpMult,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_orpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"orpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_paddb_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"paddb %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_paddw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"paddw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_paddd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"paddd %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_paddq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"paddq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmAdd,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pand_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pand %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_pandn_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pandn %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmNand,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpeqb_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpeqb %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpeqw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpeqw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpeqd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpeqd %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpgtb_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpgtb %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpgtw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpgtw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pcmpgtd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpgtd %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_por_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"por %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_pshufd_xmm_xmmm128_imm8()
{
	XmmValue src, dest;
	unsigned char imm = inst.getImmByte();

	LoadXmmM128(src);
	dest.setAsUChar(0, src.getAsUChar(imm & 3));
	dest.setAsUChar(1, src.getAsUChar((imm >> 2) & 3));
	dest.setAsUChar(2, src.getAsUChar((imm >> 4) & 3));
	dest.setAsUChar(3, src.getAsUChar((imm >> 6) & 3));
	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShuf,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_pslldq_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	int count;
	int i;

	LoadXmmM128(src);
	count = inst.getImmByte();
	if (count > 16)
		count = 16;

	dest.setWithMemset(0, 0, 16);
	for (i = 0; i < 16 - count; i++)
		dest.setAsUChar(i+count, src.getAsUChar(i));


	StoreXmmM128(dest);
	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_pslld_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pslld %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_pslld_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pslld %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psllw_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psllw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psllw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psllw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psllq_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psllq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psllq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psllq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psraw_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psraw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psraw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psraw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrlw_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrlw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrad_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrad %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrad_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrad %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrlw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrlw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrld_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrld %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrld_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(0, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrld %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrlq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrlq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrlq_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	LoadXmmM128(dest);
	src.setWithMemset(0, 0, 16);
	src.setAsUChar(1, inst.getImmByte());

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psrlq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmmM128(dest);

	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psrldq_xmmm128_imm8()
{
	XmmValue dest;
	XmmValue src;

	int count;
	int i;

	LoadXmmM128(src);
	count = inst.getImmByte();
	if (count > 16)
		count = 16;

	dest.setWithMemset(0, 0, 16);
	for (i = 0; i < 16 - count; i++)
		dest.setAsUChar(i, src.getAsUChar(i+count));


	StoreXmmM128(dest);
	newUinst(Uinst::OpcodeXmmShift,
			Uinst::DepXmmm128,
			0,
			0,
			Uinst::DepXmmm128,
			0,
			0,
			0);
}


void Context::ExecuteInst_psubb_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psubb %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

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


void Context::ExecuteInst_psubw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psubw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmSub,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psubd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psubd %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmSub,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_psubq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"psubq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	StoreXmm(dest);
	newUinst(Uinst::OpcodeXmmSub,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_punpcklbw_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"punpcklbw %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

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


void Context::ExecuteInst_punpcklwd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"punpcklwd %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

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


void Context::ExecuteInst_punpckldq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"punpckldq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

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


void Context::ExecuteInst_punpcklqdq_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"punpcklqdq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

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


void Context::ExecuteInst_pxor_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pxor %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_sqrtpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"sqrtpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_sqrtsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"sqrtsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSqrt,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_subpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"subpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_subsd_xmm_xmmm64()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM64(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"subsd %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmFpSub,
			Uinst::DepXmmm64,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);
}


void Context::ExecuteInst_unpckhpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"unpckhpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_unpcklpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	int spec_mode;

	LoadXmm(dest);
	LoadXmmM128(src);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = getState(StateSpecMode);
	if (!spec_mode)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"movdqu %1, %%xmm0\n\t"
			"movdqu %0, %%xmm1\n\t"
			"unpcklpd %%xmm0, %%xmm1\n\t"
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


void Context::ExecuteInst_xorpd_xmm_xmmm128()
{
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"xorpd %%xmm0, %%xmm1\n\t"
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

