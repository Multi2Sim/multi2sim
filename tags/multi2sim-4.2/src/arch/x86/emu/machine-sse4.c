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


#include "isa.h"
#include "machine.h"
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


#ifndef HAVE_SSE4
static char *x86_isa_err_sse4 =
	"\tThis version of Multi2Sim has been built on a machine without support\n"
	"\tfor SSE4 x86 instructions. While the implementation of this\n"
	"\tinstruction is supported, it was disabled for compatibility with your\n"
	"\tprocessor.\n";
#endif


void x86_isa_pcmpeqq_xmm_xmmm128_impl(X86Context *ctx)
{
#ifdef HAVE_SSE4
	union x86_inst_xmm_reg_t dest;
	union x86_inst_xmm_reg_t src;

	X86ContextLoadXMM(ctx, dest.as_uchar);
	X86ContextLoadXMMM128(ctx, src.as_uchar);

	__X86_ISA_ASM_START__
	asm volatile (
		"movdqu %1, %%xmm0\n\t"
		"movdqu %0, %%xmm1\n\t"
		"pcmpeqq %%xmm0, %%xmm1\n\t"
		"movdqu %%xmm1, %0\n\t"
		: "=m" (dest)
		: "m" (src)
		: "xmm0", "xmm1"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreXMM(ctx, dest.as_uchar);

	x86_uinst_new(ctx, x86_uinst_xmm_comp, x86_dep_xmmm128, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);

#else
	X86ContextError(ctx, "%s: SSE4 instruction not supported.\n%s",
			__FUNCTION__, x86_isa_err_sse4);
#endif
}


void x86_isa_pcmpistri_xmm_xmmm128_imm8_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrb_xmm_r32m8_imm8_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrd_xmm_rm32_imm8_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_ptest_xmm_xmmm128_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


