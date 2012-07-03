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

#include <x86-emu.h>


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


#define CF x86_isa_get_flag(ctx, x86_flag_cf)
#define ZF x86_isa_get_flag(ctx, x86_flag_zf)
#define SF x86_isa_get_flag(ctx, x86_flag_sf)
#define OF x86_isa_get_flag(ctx, x86_flag_of)
#define PF x86_isa_get_flag(ctx, x86_flag_pf)


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
void x86_isa_set##cc##_rm8_impl(struct x86_ctx_t *ctx) \
{ \
	if (cc_##cc) \
		x86_isa_store_rm8(ctx, 1); \
	else \
		x86_isa_store_rm8(ctx, 0); \
	x86_uinst_new(ctx, x86_uinst_move, idep1, idep2, 0, x86_dep_rm8, 0, 0, 0); \
}


#define op_jcc_rel8(cc, idep1, idep2) \
void x86_isa_j##cc##_rel8_impl(struct x86_ctx_t *ctx) \
{ \
	x86_isa_target = x86_isa_regs->eip + (int8_t) x86_isa_inst.imm.b; \
	if (cc_##cc) \
		x86_isa_regs->eip = x86_isa_target; \
	x86_uinst_new(ctx, x86_uinst_branch, idep1, idep2, 0, 0, 0, 0, 0); \
}


#define op_jcc_rel32(cc, idep1, idep2) \
void x86_isa_j##cc##_rel32_impl(struct x86_ctx_t *ctx) \
{ \
	x86_isa_target = x86_isa_regs->eip + x86_isa_inst.imm.d; \
	if (cc_##cc) \
		x86_isa_regs->eip = x86_isa_target; \
	x86_uinst_new(ctx, x86_uinst_branch, idep1, idep2, 0, 0, 0, 0, 0); \
}


#define op_cmov_r16_rm16(cc, idep1, idep2) \
void x86_isa_cmov##cc##_r16_rm16_impl(struct x86_ctx_t *ctx) \
{ \
	if (cc_##cc) \
		x86_isa_store_r16(ctx, x86_isa_load_rm16(ctx)); \
	x86_uinst_new(ctx, x86_uinst_move, idep1, idep2, x86_dep_rm16, x86_dep_r16, 0, 0, 0); \
}


#define op_cmov_r32_rm32(cc, idep1, idep2) \
void x86_isa_cmov##cc##_r32_rm32_impl(struct x86_ctx_t *ctx) \
{ \
	if (cc_##cc) \
		x86_isa_store_r32(ctx, x86_isa_load_rm32(ctx)); \
	x86_uinst_new(ctx, x86_uinst_move, idep1, idep2, x86_dep_rm32, x86_dep_r32, 0, 0, 0); \
}


#define op_cc_all(ccop) \
	op_##ccop(a, x86_dep_cf, x86_dep_zps) \
	op_##ccop(ae, x86_dep_cf, 0) \
	op_##ccop(b, x86_dep_cf, 0) \
	op_##ccop(be, x86_dep_cf, x86_dep_zps) \
	op_##ccop(e, x86_dep_zps, 0) \
	op_##ccop(g, x86_dep_zps, x86_dep_of) \
	op_##ccop(ge, x86_dep_zps, x86_dep_of) \
	op_##ccop(l, x86_dep_zps, x86_dep_of) \
	op_##ccop(le, x86_dep_zps, x86_dep_of) \
	op_##ccop(ne, x86_dep_zps, 0) \
	op_##ccop(no, x86_dep_of, 0) \
	op_##ccop(np, x86_dep_zps, 0) \
	op_##ccop(ns, x86_dep_zps, 0) \
	op_##ccop(o, x86_dep_of, 0) \
	op_##ccop(p, x86_dep_zps, 0) \
	op_##ccop(s, x86_dep_zps, 0)


op_cc_all(setcc)
op_cc_all(jcc_rel8)
op_cc_all(jcc_rel32)
op_cc_all(cmov_r16_rm16)
op_cc_all(cmov_r32_rm32)


void x86_isa_jecxz_rel8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_target = x86_isa_regs->eip + x86_isa_inst.imm.b;
	if (!x86_isa_load_reg(ctx, x86_reg_ecx))
		x86_isa_regs->eip = x86_isa_target;
	x86_uinst_new(ctx, x86_uinst_branch, x86_dep_ecx, 0, 0, 0, 0, 0, 0);
}


void x86_isa_jcxz_rel8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_target = x86_isa_regs->eip + x86_isa_inst.imm.b;
	if (!x86_isa_load_reg(ctx, x86_reg_cx))
		x86_isa_regs->eip = x86_isa_target;
	x86_uinst_new(ctx, x86_uinst_branch, x86_dep_ecx, 0, 0, 0, 0, 0, 0);
}

