/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "m2skernel.h"


#define CF isa_get_flag(flag_cf)
#define ZF isa_get_flag(flag_zf)
#define SF isa_get_flag(flag_sf)
#define OF isa_get_flag(flag_of)
#define PF isa_get_flag(flag_pf)


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


#define op_setcc(cc) void op_set##cc##_rm8_impl() { \
	if (cc_##cc) isa_store_rm8(1); \
	else isa_store_rm8(0); }

#define op_jcc_rel8(cc) void op_j##cc##_rel8_impl() { \
	isa_target = isa_regs->eip + (int8_t) isa_inst.imm.b; \
	if (cc_##cc) isa_regs->eip = isa_target; }

#define op_jcc_rel32(cc) void op_j##cc##_rel32_impl() { \
	isa_target = isa_regs->eip + isa_inst.imm.d; \
	if (cc_##cc) isa_regs->eip = isa_target; }

#define op_cmov_r16_rm16(cc) void op_cmov##cc##_r16_rm16_impl() { \
	if (cc_##cc) isa_store_r16(isa_load_rm16()); }

#define op_cmov_r32_rm32(cc) void op_cmov##cc##_r32_rm32_impl() { \
	if (cc_##cc) isa_store_r32(isa_load_rm32()); }

#define op_cc_all(ccop) \
	op_##ccop(a) \
	op_##ccop(ae) \
	op_##ccop(b) \
	op_##ccop(be) \
	op_##ccop(e) \
	op_##ccop(g) \
	op_##ccop(ge) \
	op_##ccop(l) \
	op_##ccop(le) \
	op_##ccop(ne) \
	op_##ccop(no) \
	op_##ccop(np) \
	op_##ccop(ns) \
	op_##ccop(o) \
	op_##ccop(p) \
	op_##ccop(s)

op_cc_all(setcc)
op_cc_all(jcc_rel8)
op_cc_all(jcc_rel32)
op_cc_all(cmov_r16_rm16)
op_cc_all(cmov_r32_rm32)


void op_jecxz_rel8_impl() {
	isa_target = isa_regs->eip + isa_inst.imm.b;
	if (!isa_load_reg(reg_ecx))
		isa_regs->eip = isa_target;
}

void op_jcxz_rel8_impl() {
	isa_target = isa_regs->eip + isa_inst.imm.b;
	if (!isa_load_reg(reg_cx))
		isa_regs->eip = isa_target;
}

