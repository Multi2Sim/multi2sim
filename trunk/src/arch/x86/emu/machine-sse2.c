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

#include <string.h>


#include "context.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"
#include "uinst.h"


/* Macros defined to prevent accidental use of 'mem_<xx>' instructions */
#define mem_access __COMPILATION_ERROR__
#define mem_read __COMPILATION_ERROR__
#define mem_write __COMPILATION_ERROR__
#define mem_zero __COMPILATION_ERROR__
#define mem_read_string __COMPILATION_ERROR__
#define mem_write_string __COMPILATION_ERROR__
#define mem_get_buffer __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#ifdef assert
#undef assert
#endif
#define assert __COMPILATION_ERROR__


void x86_isa_addpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_addsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_add, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_andpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_and, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtdq2pd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtdq2ps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtps2pd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtsi2sd_xmm_rm32_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	unsigned int src;

	int spec_mode;

	src = X86ContextLoadRm32(ctx);
	X86ContextLoadXMM(ctx, dest.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmm, x86_dep_rm32, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvttsd2si_r32_xmmm64_impl(X86Context *ctx)
{
	unsigned char xmm[16];
	unsigned int r32;

	X86ContextLoadXMMM64(ctx, xmm);

	__X86_ISA_ASM_START__
	asm volatile (
		"cvttsd2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm64, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_cvtsd2si_r32_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t src;
	unsigned int r32;

	int spec_mode;

	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreR32(ctx, r32);
	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm64, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_cvtsd2ss_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtpd2ps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtss2sd_xmm_xmmm32_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvttpd2dq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvttps2dq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_divpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_div, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_divsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_div, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_maxpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_maxsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_minpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_minsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movapd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMMM128(ctx, xmm.as_uchar);
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movapd_xmmm128_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreXMMM128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movd_xmm_rm32_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	memset(xmm.as_uchar, 0, sizeof xmm);
	xmm.as_uint[0] = X86ContextLoadRm32(ctx);

	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_rm32, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movd_rm32_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreRm32(ctx, xmm.as_uint[0]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void x86_isa_movdqa_xmm_xmmm128_impl(X86Context *ctx)
{
	unsigned char xmm[16];

	X86ContextLoadXMMM128(ctx, xmm);
	X86ContextStoreXMM(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movdqa_xmmm128_xmm_impl(X86Context *ctx)
{
	unsigned char xmm[16];

	X86ContextLoadXMM(ctx, xmm);
	X86ContextStoreXMMM128(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movdqu_xmm_xmmm128_impl(X86Context *ctx)
{
	unsigned char xmm[16];

	X86ContextLoadXMMM128(ctx, xmm);
	X86ContextStoreXMM(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movdqu_xmmm128_xmm_impl(X86Context *ctx)
{
	unsigned char xmm[16];

	X86ContextLoadXMM(ctx, xmm);
	X86ContextStoreXMMM128(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movhpd_xmm_m64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	xmm.as_uint64[1] = X86ContextLoadM64(ctx);
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movhpd_m64_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreM64(ctx, xmm.as_uint64[1]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movlpd_xmm_m64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	xmm.as_uint64[0] = X86ContextLoadM64(ctx);
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movlpd_m64_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreM64(ctx, xmm.as_uint64[0]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movmskpd_r32_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t src;
	unsigned int r32;

	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreR32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movmskps_r32_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t src;
	unsigned int r32;

	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreR32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movntdq_m128_xmm_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_movq_xmm_xmmm64_impl(X86Context *ctx)
{
	unsigned char value[16];

	memset(value, 0, 16);
	X86ContextLoadXMMM64(ctx, value);
	X86ContextStoreXMM(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movq_xmmm64_xmm_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned char value[16];

	memset(value, 0, 16);
	if (ctx->inst.modrm_mod == 3)
		memcpy(&regs->xmm[ctx->inst.modrm_rm], value, 16);
	X86ContextLoadXMM(ctx, value);
	X86ContextStoreXMMM64(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movsd_xmm_xmmm64_impl(X86Context *ctx)
{
	unsigned char value[16];

	/* xmm <= m64: bits 127-64 of xmm set to 0.
	 * xmm <= xmm: bits 127-64 unmodified */
	if (ctx->inst.modrm_mod == 3)
		X86ContextLoadXMM(ctx, value);
	else
		memset(value, 0, 16);
	X86ContextLoadXMMM64(ctx, value);
	X86ContextStoreXMM(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movsd_xmmm64_xmm_impl(X86Context *ctx)
{
	unsigned char value[16];

	/* xmm <= xmm: bits 127-64 unmodified.
	 * m64 <= xmm: copy 64 bits to memory */
	X86ContextLoadXMM(ctx, value);
	X86ContextStoreXMMM64(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_mulpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_mult, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_mulsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_mult, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_orpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_or, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_paddb_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_paddw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_paddd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_paddq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pand_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_and, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pandn_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_nand, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpeqb_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpeqw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpeqd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpgtb_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpgtw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpgtd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_por_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_or, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pshufd_xmm_xmmm128_imm8_impl(X86Context *ctx)
{
	unsigned int src[4], dst[4];
	unsigned char imm = ctx->inst.imm.b;

	X86ContextLoadXMMM128(ctx, (unsigned char *) src);
	dst[0] = src[imm & 3];
	dst[1] = src[(imm >> 2) & 3];
	dst[2] = src[(imm >> 4) & 3];
	dst[3] = src[(imm >> 6) & 3];
	X86ContextStoreXMM(ctx, (unsigned char *) dst);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pslldq_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int count;
	int i;

	X86ContextLoadXMMM128(ctx, src.as_uchar);
	count = ctx->inst.imm.b;
	if (count > 16)
		count = 16;

	memset(dest.as_uchar, 0, sizeof dest);
	for (i = 0; i < 16 - count; i++)
		dest.as_uchar[i + count] = src.as_uchar[i];


	X86ContextStoreXMMM128(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_pslld_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_pslld_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psllw_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psllw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psllq_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psllq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psraw_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psraw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psrlw_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psrad_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psrad_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psrlw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psrld_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psrld_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psrlq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psrlq_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMMM128(ctx, dest.as_uchar);
	memset(src.as_uchar, 0, sizeof src);
	src.as_uchar[0] = ctx->inst.imm.b;

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

	X86ContextStoreXMMM128(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psrldq_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int count;
	int i;

	X86ContextLoadXMMM128(ctx, src.as_uchar);
	count = ctx->inst.imm.b;
	if (count > 16)
		count = 16;

	memset(dest.as_uchar, 0, sizeof dest);
	for (i = 0; i < 16 - count; i++)
		dest.as_uchar[i] = src.as_uchar[i + count];


	X86ContextStoreXMMM128(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_psubb_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psubw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_sub, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psubd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_sub, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_psubq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_sub, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_punpcklbw_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_punpcklwd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_punpckldq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_punpcklqdq_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pxor_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_sqrtpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sqrt, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_sqrtsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sqrt, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_subpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_subsd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_unpckhpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_unpcklpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_xorpd_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


