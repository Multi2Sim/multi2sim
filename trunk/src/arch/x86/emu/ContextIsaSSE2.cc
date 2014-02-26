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


#define __UNIMPLEMENTED__ IsaError("%s: unimplemented instruction", \
		__FUNCTION__);


void Context::ExecuteInst_addpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_addsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_andpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtdq2pd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtdq2ps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtps2pd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtsi2sd_xmm_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvttsd2si_r32_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtsd2si_r32_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtsd2ss_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtpd2ps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtss2sd_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvttpd2dq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvttps2dq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_divpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_divsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_maxpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_maxsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_minpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_minsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movapd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movapd_xmmm128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movd_xmm_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movd_rm32_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movdqa_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movdqa_xmmm128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movdqu_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movdqu_xmmm128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movhpd_xmm_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movhpd_m64_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movlpd_xmm_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movlpd_m64_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movmskpd_r32_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movmskps_r32_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movntdq_m128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movq_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movq_xmmm64_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movsd_xmmm64_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mulpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mulsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_orpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_paddb_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_paddw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_paddd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_paddq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pand_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pandn_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpeqb_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpeqw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpeqd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpgtb_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpgtw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pcmpgtd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_por_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pshufd_xmm_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pslldq_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pslld_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pslld_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psllw_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psllw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psllq_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psllq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psraw_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psraw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrlw_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrad_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrad_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrlw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrld_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrld_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrlq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrlq_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psrldq_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psubb_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psubw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psubd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_psubq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_punpcklbw_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_punpcklwd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_punpckldq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_punpcklqdq_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pxor_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sqrtpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sqrtsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_subpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_subsd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_unpckhpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_unpcklpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xorpd_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}



}  // namespace x86

