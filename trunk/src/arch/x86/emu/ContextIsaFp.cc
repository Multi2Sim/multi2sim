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



void Context::ExecuteInst_f2xm1()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fabs()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fadd_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fadd_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fadd_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fadd_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_faddp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fchs()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovb_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmove_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovbe_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovu_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovnb_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovne_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovnbe_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcmovnu_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcom_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcom_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcom_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcomp_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcomp_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcomp_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcompp()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcomi_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcomip_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fucomi_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fucomip_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fcos()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdiv_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdiv_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdiv_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdiv_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivr_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivr_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivr_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivr_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fdivrp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fild_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fild_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fild_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fist_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fist_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fistp_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fistp_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fistp_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fld1()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldl2e()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldl2t()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldpi()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldlg2()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldln2()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldz()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fld_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fld_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fld_m80()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fld_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fldcw_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fmul_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fmul_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fmul_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fmul_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fmulp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fnstcw_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fnstsw_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fpatan()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fprem()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fprem1()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fptan()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_frndint()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fscale()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsin()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsincos()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsqrt()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fst_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fst_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fst_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstp_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstp_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstp_m80()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstp_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstsw_ax()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsub_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsub_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsub_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsub_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubr_m32()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubr_m64()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubr_st0_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubr_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fsubrp_sti_st0()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fstcw_m16()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_ftst()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fucom_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fucomp_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fucompp()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fxam()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fxch_sti()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fyl2x()
{
	__UNIMPLEMENTED__
}

void Context::ExecuteInst_fyl2xp1()
{
	__UNIMPLEMENTED__
}

}  // namespace x86

