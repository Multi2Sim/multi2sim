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


void Context::ExecuteInst_bound_r16_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bound_r32_rm64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bsf_r32_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bsr_r32_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bswap_ir32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bt_rm32_r32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bt_rm32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_bts_rm32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_call_rel32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_call_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cbw()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cdq()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cld()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cmpxchg_rm32_r32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cmpxchg8b_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cpuid()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_cwde()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_dec_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_dec_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_dec_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_dec_ir16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_dec_ir32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_div_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_div_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_hlt()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_idiv_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_imul_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_imul_r16_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_imul_r32_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_imul_r32_rm32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_imul_r32_rm32_imm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_inc_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_inc_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_inc_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_inc_ir16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_inc_ir32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_int_3()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_int_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_into()
{
	__UNIMPLEMENTED__
}


void Context::ExecuteInst_jmp_rel8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_jmp_rel32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_jmp_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_lea_r32_m()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_leave()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_lfence()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm8_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_r8_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm8_r8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm16_r16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm32_r32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_r16_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_r32_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_al_moffs8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_ax_moffs16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_eax_moffs32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_moffs8_al()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_moffs16_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_moffs32_eax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_ir8_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_ir16_imm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_ir32_imm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm16_imm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm32_imm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm16_sreg()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_rm32_sreg()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_sreg_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mov_sreg_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movsx_r16_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movsx_r32_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movsx_r32_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movzx_r16_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movzx_r32_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_movzx_r32_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_mul_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_neg_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_neg_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_nop()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_nop_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_nop_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_not_rm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_not_rm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_not_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_imm8_al()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_imm8_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_imm8_eax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_dx_al()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_dx_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_out_dx_eax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pause()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pop_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pop_ir32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_popf()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_prefetcht0_m8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_prefetcht1()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_prefetcht2()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_prefetchnta()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_push_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_push_imm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_push_rm32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_push_ir32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_pushf()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_rdtsc()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ret()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_repz_ret()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ret_imm16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sahf()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_sfence()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shld_rm16_r16_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shld_rm16_r16_cl()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shld_rm32_r32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shld_rm32_r32_cl()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shrd_rm32_r32_imm8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_shrd_rm32_r32_cl()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_std()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xadd_rm8_r8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xadd_rm32_r32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xchg_ir16_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xchg_ir32_eax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xchg_rm8_r8()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xchg_rm16_r16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_xchg_rm32_r32()
{
	__UNIMPLEMENTED__
}

}  // namespace x86

