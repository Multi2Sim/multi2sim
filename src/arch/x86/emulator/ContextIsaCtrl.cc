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



#define CF regs.getFlag(InstFlagCF)
#define ZF regs.getFlag(InstFlagZF)
#define SF regs.getFlag(InstFlagSF)
#define OF regs.getFlag(InstFlagOF)
#define PF regs.getFlag(InstFlagPF)


#define cc_a	(!CF && !ZF)
#define cc_ae	(!CF)
#define cc_b	(CF)
#define cc_be	(CF || ZF)
#define cc_e	(ZF)
#define cc_g	(!ZF && SF == OF)
#define cc_ge	(SF == OF)
#define cc_l	(SF != OF)
#define cc_le	(ZF || SF != OF)
#define cc_ne	(!ZF)
#define cc_no	(!OF)
#define cc_np	(!PF)
#define cc_ns	(!SF)
#define cc_o	(OF)
#define cc_p	(PF)
#define cc_s	(SF)


#define op_setcc(cc, idep1, idep2) \
void Context::ExecuteInst_set##cc##_rm8() \
{ \
	if (cc_##cc) \
		StoreRm8(1); \
	else \
		StoreRm8(0); \
	newUInst(UInstMove, idep1, idep2, 0, UInstDepRm8, 0, 0, 0); \
}


#define op_jcc_rel8(cc, idep1, idep2) \
void Context::ExecuteInst_j##cc##_rel8() \
{ \
	target_eip = regs.getEip() + (char) inst.getImmByte(); \
	if (cc_##cc) \
		regs.setEip(target_eip); \
	newUInst(UInstBranch, idep1, idep2, 0, 0, 0, 0, 0); \
}


#define op_jcc_rel32(cc, idep1, idep2) \
void Context::ExecuteInst_j##cc##_rel32() \
{ \
	target_eip = regs.getEip() + inst.getImmDWord(); \
	if (cc_##cc) \
		regs.setEip(target_eip); \
	newUInst(UInstBranch, idep1, idep2, 0, 0, 0, 0, 0); \
}


#define op_cmov_r16_rm16(cc, idep1, idep2) \
void Context::ExecuteInst_cmov##cc##_r16_rm16() \
{ \
	if (cc_##cc) \
		StoreR16(LoadRm16()); \
	newUInst(UInstMove, idep1, idep2, UInstDepRm16, UInstDepR16, 0, 0, 0); \
}


#define op_cmov_r32_rm32(cc, idep1, idep2) \
void Context::ExecuteInst_cmov##cc##_r32_rm32() \
{ \
	if (cc_##cc) \
		StoreR32(LoadRm32()); \
	newUInst(UInstMove, idep1, idep2, UInstDepRm32, UInstDepR32, 0, 0, 0); \
}


#define op_cc_all(ccop) \
	op_##ccop(a, UInstDepCf, UInstDepZps) \
	op_##ccop(ae, UInstDepCf, 0) \
	op_##ccop(b, UInstDepCf, 0) \
	op_##ccop(be, UInstDepCf, UInstDepZps) \
	op_##ccop(e, UInstDepZps, 0) \
	op_##ccop(g, UInstDepZps, UInstDepOf) \
	op_##ccop(ge, UInstDepZps, UInstDepOf) \
	op_##ccop(l, UInstDepZps, UInstDepOf) \
	op_##ccop(le, UInstDepZps, UInstDepOf) \
	op_##ccop(ne, UInstDepZps, 0) \
	op_##ccop(no, UInstDepOf, 0) \
	op_##ccop(np, UInstDepZps, 0) \
	op_##ccop(ns, UInstDepZps, 0) \
	op_##ccop(o, UInstDepOf, 0) \
	op_##ccop(p, UInstDepZps, 0) \
	op_##ccop(s, UInstDepZps, 0)


op_cc_all(setcc)
op_cc_all(jcc_rel8)
op_cc_all(jcc_rel32)
op_cc_all(cmov_r16_rm16)
op_cc_all(cmov_r32_rm32)


void Context::ExecuteInst_jecxz_rel8()
{
	target_eip = regs.getEip() + inst.getImmByte();
	if (!regs.Read(InstRegEcx))
		regs.setEip(target_eip);
	newUInst(UInstBranch, UInstDepEcx, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_jcxz_rel8()
{
	target_eip = regs.getEip() + inst.getImmByte();
	if (!regs.Read(InstRegCx))
		regs.setEip(target_eip);
	newUInst(UInstBranch, UInstDepEcx, 0, 0, 0, 0, 0, 0);
}


}  // namespace x86

