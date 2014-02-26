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



void Context::ExecuteInst_addps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}
void Context::ExecuteInst_addss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_andps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cmppd_xmm_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cmpps_xmm_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvtsi2ss_xmm_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cvttss2si_r32_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_divps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_divss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ldmxcsr_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_maxps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_maxss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_minps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_minss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movaps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movaps_xmmm128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movhlps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movlhps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movss_xmmm32_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movups_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movups_xmmm128_xmm()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mulps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mulss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_orps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pextrw_r32_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pinsrw_xmm_r32m16_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pmovmskb_r32_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shufps_xmm_xmmm128_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sqrtps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sqrtss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_stmxcsr_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_subps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_subss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ucomisd_xmm_xmmm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ucomiss_xmm_xmmm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_unpckhps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_unpcklps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xorps_xmm_xmmm128()
{
	__UNIMPLEMENTED__
}


}  // namespace x86

