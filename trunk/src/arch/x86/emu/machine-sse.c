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


void x86_isa_addps_xmm_xmmm128_impl(X86Context *ctx)
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
			"addps %%xmm0, %%xmm1\n\t"
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


void x86_isa_addss_xmm_xmmm32_impl(X86Context *ctx)
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
			"addss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_add, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_andps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_and, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
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

void x86_isa_cmppd_xmm_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;
	int imm8 = ctx->inst.imm.b;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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
			X86ContextError(ctx, "%s: invalid value for 'imm8'",
				__FUNCTION__);
		}
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cmpps_xmm_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	int spec_mode;
	int imm8 = ctx->inst.imm.b;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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
			X86ContextError(ctx, "%s: invalid value for 'imm8'",
				__FUNCTION__);
		}
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);
	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtsi2ss_xmm_rm32_impl(X86Context *ctx)
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
			"cvtsi2ss %%eax, %%xmm0\n\t"
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


void x86_isa_cvttss2si_r32_xmmm32_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;
	unsigned int r32;

	int spec_mode;

	X86ContextLoadXMMM32(ctx, xmm.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
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

	X86ContextStoreR32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm32, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_divps_xmm_xmmm128_impl(X86Context *ctx)
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
			"divps %%xmm0, %%xmm1\n\t"
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


void x86_isa_divss_xmm_xmmm32_impl(X86Context *ctx)
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
			"divss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_div, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_ldmxcsr_m32_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_maxps_xmm_xmmm128_impl(X86Context *ctx)
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
			"maxps %%xmm0, %%xmm1\n\t"
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


void x86_isa_maxss_xmm_xmmm32_impl(X86Context *ctx)
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
			"maxss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_minps_xmm_xmmm128_impl(X86Context *ctx)
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
			"minps %%xmm0, %%xmm1\n\t"
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


void x86_isa_minss_xmm_xmmm32_impl(X86Context *ctx)
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
			"minss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movaps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMMM128(ctx, xmm.as_uchar);
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movaps_xmmm128_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreXMMM128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movhlps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm1;
	union x86_inst_xmm_reg_t xmm2;

	X86ContextLoadXMM(ctx, xmm1.as_uchar);
	X86ContextLoadXMMM128(ctx, xmm2.as_uchar);
	xmm1.as_uint64[0] = xmm2.as_uint64[1];
	X86ContextStoreXMM(ctx, xmm1.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movlhps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm1;
	union x86_inst_xmm_reg_t xmm2;

	X86ContextLoadXMM(ctx, xmm1.as_uchar);
	X86ContextLoadXMMM128(ctx, xmm2.as_uchar);
	xmm1.as_uint64[1] = xmm2.as_uint64[0];
	X86ContextStoreXMM(ctx, xmm1.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movss_xmm_xmmm32_impl(X86Context *ctx)
{
	unsigned char value[16];

	/* xmm <= m32: bits 127-32 of xmm set to 0.
	 * xmm <= xmm: bits 127-32 unmodified */
	if (ctx->inst.modrm_mod == 3)
		X86ContextLoadXMM(ctx, value);
	else
		memset(value, 0, 16);
	X86ContextLoadXMMM32(ctx, value);
	X86ContextStoreXMM(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm32, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movss_xmmm32_xmm_impl(X86Context *ctx)
{
	unsigned char value[16];

	/* xmm <= xmm: bits 127-32 unmodified.
	 * m32 <= xmm: copy 32 bits to memory */
	X86ContextLoadXMM(ctx, value);
	X86ContextStoreXMMM32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm32, 0, 0, 0);
}


void x86_isa_movups_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMMM128(ctx, xmm.as_uchar);
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movups_xmmm128_xmm_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;

	X86ContextLoadXMM(ctx, xmm.as_uchar);
	X86ContextStoreXMMM128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_mulps_xmm_xmmm128_impl(X86Context *ctx)
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
			"mulps %%xmm0, %%xmm1\n\t"
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


void x86_isa_mulss_xmm_xmmm32_impl(X86Context *ctx)
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
			"mulss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_mult, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_orps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_or, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pextrw_r32_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;
	unsigned char imm8;
	unsigned int result;

	/* Get 'xmm' source */
	X86ContextLoadXMMM128(ctx, xmm.as_uchar);

	/* Take the 3 LSB of 'imm8' */
	imm8 = ctx->inst.imm.b & 0x7;

	/* Select 2-byte word from 'xmm' pointed to by 'imm8',
	 * and place it in the 2 LSBytes of 'result' */
	result = xmm.as_ushort[imm8];
	X86ContextStoreR32(ctx, result);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_pinsrw_xmm_r32m16_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t xmm;
	unsigned short r32m16;
	unsigned char imm8;
	
	X86ContextLoadXMM(ctx, xmm.as_uchar);
	r32m16 = X86ContextLoadR32M16(ctx);
	imm8 = ctx->inst.imm.b & 0x7;

	/* Update word in 'xmm' */
	xmm.as_ushort[imm8] = r32m16;
	X86ContextStoreXMM(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmm, x86_dep_rm32, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pmovmskb_r32_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t src;
	unsigned int r32;

	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreR32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_shufps_xmm_xmmm128_imm8_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;
	unsigned char imm;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);
	imm = ctx->inst.imm.b;

	dest.as_int[0] = dest.as_int[imm & 3];
	dest.as_int[1] = dest.as_int[(imm >> 2) & 3];
	dest.as_int[2] = src.as_int[(imm >> 4) & 3];
	dest.as_int[3] = src.as_int[(imm >> 6) & 3];

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_sqrtps_xmm_xmmm128_impl(X86Context *ctx)
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
			"sqrtps %%xmm0, %%xmm1\n\t"
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


void x86_isa_sqrtss_xmm_xmmm32_impl(X86Context *ctx)
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
			"sqrtss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sqrt, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_stmxcsr_m32_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_subps_xmm_xmmm128_impl(X86Context *ctx)
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
			"subps %%xmm0, %%xmm1\n\t"
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


void x86_isa_subss_xmm_xmmm32_impl(X86Context *ctx)
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
			"subss %%xmm0, %%xmm1\n\t"
			"movdqu %%xmm1, %0\n\t"
			: "=m" (dest)
			: "m" (src)
			: "xmm0", "xmm1"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_ucomisd_xmm_xmmm64_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;
	
	struct x86_regs_t *regs = ctx->regs;
	unsigned long eflags = regs->eflags;

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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	regs->eflags = eflags;

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
}


void x86_isa_ucomiss_xmm_xmmm32_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;
	
	struct x86_regs_t *regs = ctx->regs;
	unsigned long eflags = regs->eflags;

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

	X86ContextStoreXMM(ctx, dest.as_uchar);
	regs->eflags = eflags;

	x86_uinst_new(ctx, x86_uinst_xmm_fp_comp, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
}


void x86_isa_unpckhps_xmm_xmmm128_impl(X86Context *ctx)
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
			"unpckhps %%xmm0, %%xmm1\n\t"
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


void x86_isa_unpcklps_xmm_xmmm128_impl(X86Context *ctx)
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
			"unpcklps %%xmm0, %%xmm1\n\t"
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


void x86_isa_xorps_xmm_xmmm128_impl(X86Context *ctx)
{
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

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

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


