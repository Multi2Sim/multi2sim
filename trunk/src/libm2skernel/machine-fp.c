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

#define DUMP_ABORT printf("\n%lld\n", (long long) isa_inst_count), abort()


void op_f2xm1_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"f2xm1\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fabs_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fabs\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fadd_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fld %2\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m32)
	);
	isa_store_fpu(0, st0);
}


void op_fadd_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fldl %2\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m64)
	);
	isa_store_fpu(0, st0);
}


void op_fadd_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(0, st0);
}


void op_fadd_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_faddp_sti_st0_impl() {
	op_fadd_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_fchs_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fchs\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fcmovb_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (isa_get_flag(flag_cf))
		isa_store_fpu(0, sti);
}


void op_fcmove_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (isa_get_flag(flag_zf))
		isa_store_fpu(0, sti);
}


void op_fcmovbe_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (isa_get_flag(flag_cf) || isa_get_flag(flag_zf))
		isa_store_fpu(0, sti);
}


void op_fcmovu_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (isa_get_flag(flag_pf))
		isa_store_fpu(0, sti);
}


void op_fcmovnb_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (!isa_get_flag(flag_cf))
		isa_store_fpu(0, sti);
}


void op_fcmovne_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (!isa_get_flag(flag_zf))
		isa_store_fpu(0, sti);
}


void op_fcmovnbe_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (!isa_get_flag(flag_cf) && !isa_get_flag(flag_zf))
		isa_store_fpu(0, sti);
}


void op_fcmovnu_st0_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	if (!isa_get_flag(flag_pf))
		isa_store_fpu(0, sti);
}


void op_fcom_m32_impl() {
	uint8_t st0[10];
	float m32;
	uint16_t status;

	isa_load_fpu(0, st0);
	m32 = isa_load_float();
	asm volatile (
		"fldt %1\n\t"
		"fcomps %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fcom_m64_impl() {
	uint8_t st0[10];
	double m64;
	uint16_t status;

	isa_load_fpu(0, st0);
	m64 = isa_load_double();
	asm volatile (
		"fldt %1\n\t"
		"fcompl %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fcom_sti_impl() {
	uint8_t st0[10], sti[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fcomp_m32_impl() {
	op_fcom_m32_impl();
	isa_pop_fpu(NULL);
}


void op_fcomp_m64_impl() {
	op_fcom_m64_impl();
	isa_pop_fpu(NULL);
}


void op_fcomp_sti_impl() {
	op_fcom_sti_impl();
	isa_pop_fpu(NULL);
}


void op_fcompp_impl() {
	uint8_t st0[10], st1[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	isa_store_fpu_code(status);
	isa_pop_fpu(NULL);
	isa_pop_fpu(NULL);
}


void op_fcomi_st0_sti_impl() {
	uint8_t st0[10], sti[10];
	unsigned long flags = isa_regs->eflags;

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"push %3 ; popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf ; pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	isa_regs->eflags = flags;
}


void op_fcomip_st0_sti_impl() {
	op_fcomi_st0_sti_impl();
	isa_pop_fpu(NULL);
}


void op_fucomi_st0_sti_impl() {
	uint8_t st0[10], sti[10];
	unsigned long flags = isa_regs->eflags;

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"push %3 ; popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf ; pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	isa_regs->eflags = flags;
}


void op_fucomip_st0_sti_impl() {
	op_fucomi_st0_sti_impl();
	isa_pop_fpu(NULL);
}


void op_fcos_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fcos\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fdiv_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fld %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m32)
	);
	isa_store_fpu(0, st0);
}


void op_fdiv_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fldl %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m64)
	);
	isa_store_fpu(0, st0);
}


void op_fdiv_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(0, st0);
}


void op_fdiv_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_fdivp_sti_st0_impl() {
	op_fdiv_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_fdivr_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fld %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (m32), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fdivr_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldl %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (m64), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fdivr_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fdivr_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_fdivrp_sti_st0_impl() {
	op_fdivr_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_fild_m16_impl() {
	int16_t m16;
	uint8_t e[10];
	mem_read(isa_mem, isa_effective_address(), 2, &m16);
	asm volatile ("filds %1; fstpt %0\n\t"
		: "=m" (*e) : "m" (m16));
	isa_push_fpu(e);
}


void op_fild_m32_impl() {
	int32_t m32;
	uint8_t e[10];
	mem_read(isa_mem, isa_effective_address(), 4, &m32);
	asm volatile ("fildl %1; fstpt %0\n\t"
		: "=m" (*e) : "m" (m32));
	isa_push_fpu(e);
}


void op_fild_m64_impl() {
	int64_t m64;
	uint8_t e[10];
	mem_read(isa_mem, isa_effective_address(), 8, &m64);
	asm volatile ("fildq %1; fstpt %0\n\t"
		: "=m" (*e) : "m" (m64));
	isa_push_fpu(e);
}


void op_fist_m16_impl() {
	int16_t m16;
	uint8_t e[10];
	isa_load_fpu(0, e);
	asm volatile ("fldt %1; fistps %0\n\t"
		: "=m" (m16) : "m" (*e));
	mem_write(isa_mem, isa_effective_address(), 2, &m16);
}


void op_fist_m32_impl() {
	int32_t m32;
	uint8_t e[10];
	isa_load_fpu(0, e);
	asm volatile ("fldt %1; fistpl %0\n\t"
		: "=m" (m32) : "m" (*e));
	mem_write(isa_mem, isa_effective_address(), 4, &m32);
}


void op_fist_m64_impl() {
	int64_t m64;
	uint8_t e[10];
	isa_load_fpu(0, e);
	asm volatile ("fldt %1; fistpq %0\n\t"
		: "=m" (m64) : "m" (*e));
	mem_write(isa_mem, isa_effective_address(), 8, &m64);
}


void op_fistp_m16_impl() {
	op_fist_m16_impl();
	isa_pop_fpu(NULL);
}


void op_fistp_m32_impl() {
	op_fist_m32_impl();
	isa_pop_fpu(NULL);
}


void op_fistp_m64_impl() {
	op_fist_m64_impl();
	isa_pop_fpu(NULL);
}


void op_fld1_impl() {
	uint8_t v[10];
	asm volatile ("fld1; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldl2e_impl() {
	uint8_t v[10];
	asm volatile ("fldl2e; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldl2t_impl() {
	uint8_t v[10];
	asm volatile ("fldl2t; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldpi_impl() {
	uint8_t v[10];
	asm volatile ("fldpi; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldlg2_impl() {
	uint8_t v[10];
	asm volatile ("fldlg2; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldln2_impl() {
	uint8_t v[10];
	asm volatile ("fldln2; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldz_impl() {
	uint8_t v[10];
	asm volatile ("fldz; fstpt %0\n\t" : "=m" (*v));
	isa_push_fpu(v);
}


void op_fldcw_m16_impl() {
	uint32_t addr = isa_effective_address();
	uint16_t value;
	mem_read(isa_mem, addr, 2, &value);
	isa_regs->fpu_ctrl = value;
	asm volatile (
		"fldcw %0\n\t"
		:
		: "m" (value)
	);
}


void op_fld_m32_impl() {
	uint8_t e[10];
	float m32;
	m32 = isa_load_float();
	isa_float_to_extended(m32, e);
	isa_push_fpu(e);
}


void op_fld_m64_impl() {
	uint8_t e[10];
	double m64;
	m64 = isa_load_double();
	isa_double_to_extended(m64, e);
	isa_push_fpu(e);
}


void op_fld_m80_impl() {
	uint8_t e[10];
	isa_load_extended(e);
	isa_push_fpu(e);
}


void op_fld_sti_impl() {
	uint8_t sti[10];
	isa_load_fpu(isa_inst.opindex, sti);
	isa_push_fpu(sti);
}


void op_fmul_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fld %2\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m32)
	);
	isa_store_fpu(0, st0);
}


void op_fmul_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fldl %2\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m64)
	);
	isa_store_fpu(0, st0);
}


void op_fmul_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(0, st0);
}


void op_fmul_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_fmulp_sti_st0_impl() {
	op_fmul_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_fstcw_m16_impl() {
	uint16_t value = isa_regs->fpu_ctrl;
	asm volatile (
		"fnstcw %0\n\t"
		:
		: "m" (value)
	);
	mem_write(isa_mem, isa_effective_address(), 2, &value);
}


void op_fnstcw_m16_impl() {
	op_fstcw_m16_impl();
}


void op_fnstsw_ax_impl() {
	uint16_t status = isa_load_fpu_status();
	isa_store_reg(reg_ax, status);
}


void op_fprem_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fprem\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(0, st0);
}


void op_fprem1_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fprem1\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(0, st0);
}


void op_fptan_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fptan\n\t"
		"fstp %%st(0)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
	op_fld1_impl();
}


void op_fpatan_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fpatan\n\t"
		"fstpt %0\n\t"
		: "=m" (*st1)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(1, st1);
	isa_pop_fpu(NULL);
}


void op_frndint_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"frndint\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fscale_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fscale\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(0, st0);
}


void op_fsin_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fsin\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fsincos_impl() {
	uint8_t st0[10], vsin[10], vcos[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %2\n\t"
		"fsin\n\t"
		"fstpt %0\n\t"
		"fldt %2\n\t"
		"fcos\n\t"
		"fstpt %1\n\t"
		: "=m" (*vsin), "=m" (*vcos)
		: "m" (*st0)
	);
	isa_store_fpu(0, vsin);
	isa_push_fpu(vcos);
}


void op_fsqrt_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fsqrt\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fst_m32_impl() {
	uint8_t st0[10];
	float m32;
	isa_load_fpu(0, st0);
	m32 = isa_extended_to_float(st0);
	isa_store_float(m32);
}


void op_fst_m64_impl() {
	uint8_t st0[10];
	double m64;
	isa_load_fpu(0, st0);
	m64 = isa_extended_to_double(st0);
	isa_store_double(m64);
}


void op_fst_sti_impl() {
	uint8_t st0[10];
	isa_load_fpu(0, st0);
	isa_store_fpu(isa_inst.opindex, st0);
}


void op_fstp_m32_impl() {
	op_fst_m32_impl();
	isa_pop_fpu(NULL);
}


void op_fstp_m64_impl() {
	op_fst_m64_impl();
	isa_pop_fpu(NULL);
}


void op_fstp_m80_impl() {
	uint8_t m80[10];
	isa_pop_fpu(m80);
	isa_store_extended(m80);
}


void op_fstp_sti_impl() {
	op_fst_sti_impl();
	isa_pop_fpu(NULL);
}


void op_fstsw_ax_impl() {
	uint16_t status = isa_load_fpu_status();
	isa_store_reg(reg_ax, status);
}


void op_fsub_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fld %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m32)
	);
	isa_store_fpu(0, st0);
}


void op_fsub_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fldl %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (m64)
	);
	isa_store_fpu(0, st0);
}


void op_fsub_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(0, st0);
}


void op_fsub_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_fsubp_sti_st0_impl() {
	op_fsub_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_fsubr_m32_impl() {
	uint8_t st0[10];
	float m32;

	m32 = isa_load_float();
	isa_load_fpu(0, st0);
	asm volatile (
		"fld %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (m32), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fsubr_m64_impl() {
	uint8_t st0[10];
	double m64;

	m64 = isa_load_double();
	isa_load_fpu(0, st0);
	asm volatile (
		"fldl %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (m64), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fsubr_st0_sti_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*st0)
		: "m" (*sti), "m" (*st0)
	);
	isa_store_fpu(0, st0);
}


void op_fsubr_sti_st0_impl() {
	uint8_t st0[10], sti[10];

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %1\n\t"
		"fldt %2\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fstpt %0\n\t"
		: "=m" (*sti)
		: "m" (*st0), "m" (*sti)
	);
	isa_store_fpu(isa_inst.opindex, sti);
}


void op_fsubrp_sti_st0_impl() {
	op_fsubr_sti_st0_impl();
	isa_pop_fpu(NULL);
}


void op_ftst_impl()
{
	uint8_t st0[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"ftst\n\t"
		"fnstsw %%ax\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fucom_sti_impl() {
	uint8_t st0[10], sti[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fucomp_sti_impl() {
	op_fucom_sti_impl();
	isa_pop_fpu(NULL);
}


void op_fucompp_impl() {
	uint8_t st0[10], st1[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucompp\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	isa_store_fpu_code(status);
	isa_pop_fpu(NULL);
	isa_pop_fpu(NULL);
}


void op_fxam_impl() {
	uint8_t st0[10];
	uint16_t status;

	isa_load_fpu(0, st0);
	asm volatile (
		"fldt %1\n\t"
		"fxam\n\t"
		"fstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		"fstp %%st(0)\n\t"
		: "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	isa_store_fpu_code(status);
}


void op_fxch_sti_impl() {
	uint8_t st0[10], sti[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(isa_inst.opindex, sti);
	isa_store_fpu(0, sti);
	isa_store_fpu(isa_inst.opindex, st0);
}


void op_fyl2x_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fyl2x\n\t"
		"fstpt %0\n\t"
		: "=m" (*st1)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(1, st1);
	isa_pop_fpu(NULL);
}


void op_fyl2xp1_impl() {
	uint8_t st0[10], st1[10];
	isa_load_fpu(0, st0);
	isa_load_fpu(1, st1);
	asm volatile (
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fyl2xp1\n\t"
		"fstpt %0\n\t"
		: "=m" (*st1)
		: "m" (*st0), "m" (*st1)
	);
	isa_store_fpu(1, st1);
	isa_pop_fpu(NULL);
}



