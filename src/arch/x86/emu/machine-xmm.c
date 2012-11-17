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

#include <arch/common/arch.h>

#include "context.h"
#include "emu.h"
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


void x86_isa_addsd_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_add, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_addss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_add, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvtsd2ss_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_cvttsd2si_r32_xmmm64_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];
	unsigned int r32;

	x86_isa_load_xmmm64(ctx, xmm);

	__X86_ISA_ASM_START__
	asm volatile (
		"cvttsd2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	__X86_ISA_ASM_END__

	x86_isa_store_r32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm64, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_cvttss2si_r32_xmmm32_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];
	unsigned int r32;

	x86_isa_load_xmmm32(ctx, xmm);

	__X86_ISA_ASM_START__
	asm volatile (
		"cvttss2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	__X86_ISA_ASM_END__

	x86_isa_store_r32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_conv, x86_dep_xmmm32, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_divss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_div, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_ldmxcsr_m32_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_movapd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmmm128(ctx, xmm.as_uchar);
	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movapd_xmmm128_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_xmmm128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movaps_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmmm128(ctx, xmm.as_uchar);
	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movaps_xmmm128_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_xmmm128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movd_xmm_rm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	memset(xmm.as_uchar, 0, sizeof xmm);
	xmm.as_uint[0] = x86_isa_load_rm32(ctx);

	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_rm32, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movd_rm32_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_rm32(ctx, xmm.as_uint[0]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void x86_isa_movdqa_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];

	x86_isa_load_xmmm128(ctx, xmm);
	x86_isa_store_xmm(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movdqa_xmmm128_xmm_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];

	x86_isa_load_xmm(ctx, xmm);
	x86_isa_store_xmmm128(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movdqu_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];

	x86_isa_load_xmmm128(ctx, xmm);
	x86_isa_store_xmm(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movdqu_xmmm128_xmm_impl(struct x86_ctx_t *ctx)
{
	unsigned char xmm[16];

	x86_isa_load_xmm(ctx, xmm);
	x86_isa_store_xmmm128(ctx, xmm);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_movhpd_xmm_m64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	xmm.as_uint64[1] = x86_isa_load_m64(ctx);
	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movhpd_m64_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_m64(ctx, xmm.as_uint64[1]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movlhps_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm1;
	union x86_xmm_reg_t xmm2;

	x86_isa_load_xmm(ctx, xmm1.as_uchar);
	x86_isa_load_xmmm128(ctx, xmm2.as_uchar);
	xmm1.as_int64[1] = xmm2.as_uint64[0];
	x86_isa_store_xmm(ctx, xmm1.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movlpd_xmm_m64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	xmm.as_uint64[0] = x86_isa_load_m64(ctx);
	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movlpd_m64_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_m64(ctx, xmm.as_uint64[0]);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_mulsd_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_mult, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_mulss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_mult, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pmovmskb_r32_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t src;
	uint32_t r32;

	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_r32(ctx, r32);

	x86_uinst_new(ctx, x86_uinst_xmm_shift, x86_dep_xmmm128, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movntdq_m128_xmm_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_movq_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	unsigned char value[16];

	memset(value, 0, 16);
	x86_isa_load_xmmm64(ctx, value);
	x86_isa_store_xmm(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movq_xmmm64_xmm_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned char value[16];

	memset(value, 0, 16);
	if (ctx->inst.modrm_mod == 3)
		memcpy(&regs->xmm[ctx->inst.modrm_rm], value, 16);
	x86_isa_load_xmm(ctx, value);
	x86_isa_store_xmmm64(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movsd_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	unsigned char value[16];

	/* xmm <= m64: bits 127-64 of xmm set to 0.
	 * xmm <= xmm: bits 127-64 unmodified */
	if (ctx->inst.modrm_mod == 3)
		x86_isa_load_xmm(ctx, value);
	else
		memset(value, 0, 16);
	x86_isa_load_xmmm64(ctx, value);
	x86_isa_store_xmm(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm64, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movsd_xmmm64_xmm_impl(struct x86_ctx_t *ctx)
{
	unsigned char value[16];

	/* xmm <= xmm: bits 127-64 unmodified.
	 * m64 <= xmm: copy 64 bits to memory */
	x86_isa_load_xmm(ctx, value);
	x86_isa_store_xmmm64(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void x86_isa_movss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	unsigned char value[16];

	/* xmm <= m32: bits 127-32 of xmm set to 0.
	 * xmm <= xmm: bits 127-32 unmodified */
	if (ctx->inst.modrm_mod == 3)
		x86_isa_load_xmm(ctx, value);
	else
		memset(value, 0, 16);
	x86_isa_load_xmmm32(ctx, value);
	x86_isa_store_xmm(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm32, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movss_xmmm32_xmm_impl(struct x86_ctx_t *ctx)
{
	unsigned char value[16];

	/* xmm <= xmm: bits 127-32 unmodified.
	 * m32 <= xmm: copy 32 bits to memory */
	x86_isa_load_xmm(ctx, value);
	x86_isa_store_xmmm32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm32, 0, 0, 0);
}


void x86_isa_movups_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmmm128(ctx, xmm.as_uchar);
	x86_isa_store_xmm(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_movups_xmmm128_xmm_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t xmm;

	x86_isa_load_xmm(ctx, xmm.as_uchar);
	x86_isa_store_xmmm128(ctx, xmm.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm128, 0, 0, 0);
}


void x86_isa_orpd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_or, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_orps_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_or, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_xorpd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_xorps_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_paddd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_add, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_palignr_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pand_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pcmpeqb_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpeqw_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpeqd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pcmpistri_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrb_xmm_r32m8_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrd_xmm_rm32_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_punpcklbw_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_punpcklwd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_punpckldq_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_punpcklqdq_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pshufb_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pshufd_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	unsigned int src[4], dst[4];
	unsigned char imm = ctx->inst.imm.b;

	x86_isa_load_xmmm128(ctx, (unsigned char *) src);
	dst[0] = src[imm & 3];
	dst[1] = src[(imm >> 2) & 3];
	dst[2] = src[(imm >> 4) & 3];
	dst[3] = src[(imm >> 6) & 3];
	x86_isa_store_xmm(ctx, (unsigned char *) dst);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_pslldq_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_psrldq_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_stmxcsr_m32_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_psubb_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_psubw_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_psubd_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_ptest_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pxor_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_xor, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_shufps_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;
	unsigned char imm;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);
	imm = ctx->inst.imm.b;

	dest.as_int[0] = dest.as_int[imm & 3];
	dest.as_int[1] = dest.as_int[(imm >> 2) & 3];
	dest.as_int[2] = src.as_int[(imm >> 4) & 3];
	dest.as_int[3] = src.as_int[(imm >> 6) & 3];

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_sqrtss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_div, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_subsd_xmm_xmmm64_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm64(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_subss_xmm_xmmm32_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm32(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_fp_sub, x86_dep_xmmm32, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void x86_isa_unpcklps_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	union x86_xmm_reg_t dest;
	union x86_xmm_reg_t src;

	int spec_mode;

	x86_isa_load_xmm(ctx, dest.as_uchar);
	x86_isa_load_xmmm128(ctx, src.as_uchar);

	/* Prevent execution of the floating-point computation in speculative
	 * mode, since it may cause host exceptions for garbage input operands. */
	spec_mode = x86_ctx_get_status(ctx, x86_ctx_spec_mode);
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

	x86_isa_store_xmm(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_shuf, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}
