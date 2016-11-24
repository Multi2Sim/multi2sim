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



#define CF regs.getFlag(Instruction::FlagCF)
#define ZF regs.getFlag(Instruction::FlagZF)
#define SF regs.getFlag(Instruction::FlagSF)
#define OF regs.getFlag(Instruction::FlagOF)
#define PF regs.getFlag(Instruction::FlagPF)


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
	newUinst(Uinst::OpcodeMove, \
			idep1, \
			idep2, \
			0, \
			Uinst::DepRm8, \
			0, \
			0, \
			0); \
}


#define op_jcc_rel8(cc, idep1, idep2) \
void Context::ExecuteInst_j##cc##_rel8() \
{ \
	target_eip = regs.getEip() + (char) inst.getImmByte(); \
	if (cc_##cc) \
		regs.setEip(target_eip); \
	newUinst(Uinst::OpcodeBranch, \
			idep1, \
			idep2, \
			0, \
			0, \
			0, \
			0, \
			0); \
}


#define op_jcc_rel32(cc, idep1, idep2) \
void Context::ExecuteInst_j##cc##_rel32() \
{ \
	target_eip = regs.getEip() + inst.getImmDWord(); \
	if (cc_##cc) \
		regs.setEip(target_eip); \
	newUinst(Uinst::OpcodeBranch, \
			idep1, \
			idep2, \
			0, \
			0, \
			0, \
			0, \
			0); \
}


#define op_cmov_r16_rm16(cc, idep1, idep2) \
void Context::ExecuteInst_cmov##cc##_r16_rm16() \
{ \
	if (cc_##cc) \
		StoreR16(LoadRm16()); \
	newUinst(Uinst::OpcodeMove, \
			idep1, \
			idep2, \
			Uinst::DepRm16, \
			Uinst::DepR16, \
			0, \
			0, \
			0); \
}


#define op_cmov_r32_rm32(cc, idep1, idep2) \
void Context::ExecuteInst_cmov##cc##_r32_rm32() \
{ \
	if (cc_##cc) \
		StoreR32(LoadRm32()); \
	newUinst(Uinst::OpcodeMove, \
			idep1, \
			idep2, \
			Uinst::DepRm32, \
			Uinst::DepR32, \
			0, \
			0, \
			0); \
}


#define op_cc_all(ccop) \
	op_##ccop(a, Uinst::DepCf, Uinst::DepZps) \
	op_##ccop(ae, Uinst::DepCf, 0) \
	op_##ccop(b, Uinst::DepCf, 0) \
	op_##ccop(be, Uinst::DepCf, Uinst::DepZps) \
	op_##ccop(e, Uinst::DepZps, 0) \
	op_##ccop(g, Uinst::DepZps, Uinst::DepOf) \
	op_##ccop(ge, Uinst::DepZps, Uinst::DepOf) \
	op_##ccop(l, Uinst::DepZps, Uinst::DepOf) \
	op_##ccop(le, Uinst::DepZps, Uinst::DepOf) \
	op_##ccop(ne, Uinst::DepZps, 0) \
	op_##ccop(no, Uinst::DepOf, 0) \
	op_##ccop(np, Uinst::DepZps, 0) \
	op_##ccop(ns, Uinst::DepZps, 0) \
	op_##ccop(o, Uinst::DepOf, 0) \
	op_##ccop(p, Uinst::DepZps, 0) \
	op_##ccop(s, Uinst::DepZps, 0)


op_cc_all(setcc)
op_cc_all(jcc_rel8)
op_cc_all(jcc_rel32)
op_cc_all(cmov_r16_rm16)
op_cc_all(cmov_r32_rm32)


void Context::ExecuteInst_jecxz_rel8()
{
	target_eip = regs.getEip() + inst.getImmByte();
	if (!regs.Read(Instruction::RegEcx))
		regs.setEip(target_eip);
	newUinst(Uinst::OpcodeBranch, Uinst::DepEcx, 0, 0, 0, 0, 0, 0);
}


void Context::ExecuteInst_jcxz_rel8()
{
	target_eip = regs.getEip() + inst.getImmByte();
	if (!regs.Read(Instruction::RegCx))
		regs.setEip(target_eip);
	newUinst(Uinst::OpcodeBranch, Uinst::DepEcx, 0, 0, 0, 0, 0, 0);
}


}  // namespace x86

