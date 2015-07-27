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

#include <lib/cpp/Misc.h>


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



#ifndef HAVE_SSE4
static char *x86_isa_err_sse4 =
	"\tThis version of Multi2Sim has been built on a machine without support\n"
	"\tfor SSE4 x86 instructions. While the implementation of this\n"
	"\tinstruction is supported, it was disabled for compatibility with your\n"
	"\tprocessor.\n";
#endif


void Context::ExecuteInst_pcmpeqq_xmm_xmmm128()
{
#ifdef HAVE_SSE4
	XmmValue dest;
	XmmValue src;

	LoadXmm(dest);
	LoadXmmM128(src);

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

	StoreXmm(dest);

	newUinst(Uinst::OpcodeXmmComp,
			Uinst::DepXmmm128,
			Uinst::DepXmm,
			0,
			Uinst::DepXmm,
			0,
			0,
			0);

#else
	throw misc::Panic(x86_isa_err_sse4);
#endif
}

void Context::ExecuteInst_pcmpistri_xmm_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pinsrb_xmm_r32m8_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pinsrd_xmm_rm32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ptest_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}



}  // namespace x86

