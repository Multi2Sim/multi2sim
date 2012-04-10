/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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


void x86_isa_f2xm1_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"f2xm1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_exp, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fabs_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fabs\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sign, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fadd_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fld %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_add, x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fadd_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_add, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fadd_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_add, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fadd_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"faddp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_add, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_faddp_sti_st0_impl()
{
	x86_isa_fadd_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fchs_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fchs\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sign, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovb_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (x86_isa_get_flag(x86_flag_cf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_cf, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmove_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (x86_isa_get_flag(x86_flag_zf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_zps, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovbe_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (x86_isa_get_flag(x86_flag_cf) || x86_isa_get_flag(x86_flag_zf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_cf, x86_dep_zps, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovu_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (x86_isa_get_flag(x86_flag_pf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_zps, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovnb_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (!x86_isa_get_flag(x86_flag_cf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_cf, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovne_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (!x86_isa_get_flag(x86_flag_zf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_zps, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovnbe_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (!x86_isa_get_flag(x86_flag_cf) && !x86_isa_get_flag(x86_flag_zf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_cf, x86_dep_zps, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcmovnu_st0_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	if (!x86_isa_get_flag(x86_flag_pf))
		x86_isa_store_fpu(0, sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, x86_dep_zps, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fcom_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	m32 = x86_isa_load_float();

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %1\n\t"
		"fcomps %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_mem32, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fcom_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	m64 = x86_isa_load_double();

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %1\n\t"
		"fcompl %2\n\t"
		"fnstsw %%ax\n\t"
		"mov %%ax, %0\n\t"
		: "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_mem64, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fcom_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_sti, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fcomp_m32_impl()
{
	x86_isa_fcom_m32_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fcomp_m64_impl()
{
	x86_isa_fcom_m64_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fcomp_sti_impl()
{
	x86_isa_fcom_sti_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fcompp_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);
	x86_isa_pop_fpu(NULL);
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_st1, 0, x86_dep_fpst, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fcomi_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	unsigned long flags = x86_isa_regs->eflags;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fcomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_sti, 0, x86_dep_zps, x86_dep_cf, 0, 0);
}


void x86_isa_fcomip_st0_sti_impl()
{
	x86_isa_fcomi_st0_sti_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fucomi_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	unsigned long flags = x86_isa_regs->eflags;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"fldt %2\n\t"
		"fldt %1\n\t"
		"fucomip %%st(1), %%st\n\t"
		"fstp %%st(0)\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (*st0), "m" (*sti), "g" (flags)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_sti, 0, x86_dep_zps, x86_dep_cf, 0, 0);
}


void x86_isa_fucomip_st0_sti_impl()
{
	x86_isa_fucomi_st0_sti_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fcos_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fcos\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_cos, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdiv_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fld %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdiv_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdiv_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdiv_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fdivp_sti_st0_impl()
{
	x86_isa_fdiv_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fdivr_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fld %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m32), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdivr_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldl %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m64), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdivr_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_div, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fdivr_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fdivrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_div, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fdivrp_sti_st0_impl()
{
	x86_isa_fdivr_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fild_m16_impl()
{
	int16_t m16;
	uint8_t e[10];

	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 2, &m16);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"filds %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m16)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem16, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fild_m32_impl()
{
	int32_t m32;
	uint8_t e[10];

	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 4, &m32);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fildl %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m32)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem32, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fild_m64_impl()
{
	int64_t m64;
	uint8_t e[10];

	x86_isa_mem_read(x86_isa_mem, x86_isa_effective_address(), 8, &m64);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fildq %1\n\t"
		"fstpt %0\n\t"
		: "=m" (*e)
		: "m" (m64)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem64, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fist_m16_impl()
{
	int16_t m16;
	uint8_t e[10];

	x86_isa_load_fpu(0, e);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %1\n\t"
		"fistps %0\n\t"
		: "=m" (m16)
		: "m" (*e)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 2, &m16);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem16, 0, 0, 0);
}


void x86_isa_fist_m32_impl()
{
	int32_t m32;
	uint8_t e[10];

	x86_isa_load_fpu(0, e);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %1\n\t"
		"fistpl %0\n\t"
		: "=m" (m32)
		: "m" (*e)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 4, &m32);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem32, 0, 0, 0);
}


void x86_isa_fist_m64_impl()
{
	int64_t m64;
	uint8_t e[10];

	x86_isa_load_fpu(0, e);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %1\n\t"
		"fistpq %0\n\t"
		: "=m" (m64)
		: "m" (*e)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 8, &m64);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem64, 0, 0, 0);
}


void x86_isa_fistp_m16_impl()
{
	x86_isa_fist_m16_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fistp_m32_impl()
{
	x86_isa_fist_m32_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fistp_m64_impl()
{
	x86_isa_fist_m64_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fld1_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fld1\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldl2e_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldl2e\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldl2t_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldl2t\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldpi_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldpi\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldlg2_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldlg2\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldln2_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldln2\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldz_impl()
{
	uint8_t v[10];

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldz\n\t"
		"fstpt %0\n\t"
		: "=m" (*v)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_push_fpu(v);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, 0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fld_m32_impl()
{
	uint8_t e[10];
	float m32;

	m32 = x86_isa_load_float();
	x86_isa_float_to_extended(m32, e);
	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem32, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fld_m64_impl()
{
	uint8_t e[10];
	double m64;

	m64 = x86_isa_load_double();
	x86_isa_double_to_extended(m64, e);
	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem64, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fld_m80_impl()
{
	uint8_t e[10];
	x86_isa_load_extended(e);
	x86_isa_push_fpu(e);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem80, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fld_sti_impl()
{
	uint8_t sti[10];
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	x86_isa_push_fpu(sti);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_sti, 0, 0, x86_dep_fpaux, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_move, x86_dep_fpaux, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fldcw_m16_impl()
{
	uint32_t addr = x86_isa_effective_address();
	uint16_t value;

	x86_isa_mem_read(x86_isa_mem, addr, 2, &value);
	if(x86_isa_spec_mode) value |= 0x3f; /* mask all FP exceptions on wrong path */
	x86_isa_regs->fpu_ctrl = value;

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldcw %0\n\t"
		:
		: "m" (value)
	);
	__X86_ISA_FP_ASM_END__

	x86_uinst_new(x86_uinst_fp_move, x86_dep_mem16, 0, 0, x86_dep_fpcw, 0, 0, 0);
}


void x86_isa_fmul_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fld %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_mult, x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fmul_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_mult, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fmul_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_mult, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fmul_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fmulp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_mult, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fmulp_sti_st0_impl()
{
	x86_isa_fmul_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fnstcw_m16_impl()
{
	x86_isa_fstcw_m16_impl();
}


void x86_isa_fnstsw_ax_impl()
{
	uint16_t status = x86_isa_load_fpu_status();
	x86_isa_store_reg(x86_reg_ax, status);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_fpst, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_fpatan_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fpatan\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(1, st1);
	x86_isa_pop_fpu(NULL);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_atan, x86_dep_st0, x86_dep_st1, 0, x86_dep_st1, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fprem_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fprem\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_st1, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fprem1_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fprem1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_div, x86_dep_st0, x86_dep_st1, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fptan_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fptan\n\t"
		"fnstsw %%ax\n\t"
		"fstp %%st(0)\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);

	x86_uinst_new(x86_uinst_fp_tan, x86_dep_st0, 0, 0, x86_dep_st0, x86_dep_st1, 0, 0);

	x86_isa_fld1_impl();
	x86_isa_store_fpu_code(status);
}


void x86_isa_frndint_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"frndint\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_round, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fscale_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fscale\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"fstp %%st(0)\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_exp, x86_dep_st0, x86_dep_st1, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsin_impl()
{
	uint8_t st0[10];
	int16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fsin\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sin, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsincos_impl()
{
	uint8_t st0[10], vsin[10], vcos[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fsin\n\t"
		"fstpt %0\n\t"
		"fldt %3\n\t"
		"fcos\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %1\n\t"
		"mov %%ax, %2\n\t"
		: "=m" (*vsin), "=m" (*vcos), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, vsin);
	x86_isa_push_fpu(vcos);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_push, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_sincos, x86_dep_st1, 0, 0, x86_dep_st0, x86_dep_st1, 0, 0);
}


void x86_isa_fsqrt_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fsqrt\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sqrt, x86_dep_st0, 0, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fst_m32_impl()
{
	uint8_t st0[10];
	float m32;

	x86_isa_load_fpu(0, st0);
	m32 = x86_isa_extended_to_float(st0);
	x86_isa_store_float(m32);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem32, 0, 0, 0);
}


void x86_isa_fst_m64_impl()
{
	uint8_t st0[10];
	double m64;

	x86_isa_load_fpu(0, st0);
	m64 = x86_isa_extended_to_double(st0);
	x86_isa_store_double(m64);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem64, 0, 0, 0);
}


void x86_isa_fst_sti_impl()
{
	uint8_t st0[10];

	x86_isa_load_fpu(0, st0);
	x86_isa_store_fpu(x86_isa_inst.opindex, st0);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fstp_m32_impl()
{
	x86_isa_fst_m32_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fstp_m64_impl()
{
	x86_isa_fst_m64_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fstp_m80_impl()
{
	uint8_t m80[10];
	x86_isa_pop_fpu(m80);
	x86_isa_store_extended(m80);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, 0, 0, x86_dep_mem80, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fstp_sti_impl()
{
	x86_isa_fst_sti_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fstsw_ax_impl()
{
	uint16_t status = x86_isa_load_fpu_status();
	x86_isa_store_reg(x86_reg_ax, status);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_fpst, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_fsub_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fld %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m32)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub,	x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsub_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldl %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (m64)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsub_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsub_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fsubp_sti_st0_impl()
{
	x86_isa_fsub_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fsubr_m32_impl()
{
	uint8_t st0[10];
	float m32;
	uint16_t status;

	m32 = x86_isa_load_float();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fld %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m32), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_mem32, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsubr_m64_impl()
{
	uint8_t st0[10];
	double m64;
	uint16_t status;

	m64 = x86_isa_load_double();
	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldl %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (m64), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_mem64, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsubr_st0_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st0), "=g" (status)
		: "m" (*sti), "m" (*st0)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(0, st0);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, 0, 0, 0);
}


void x86_isa_fsubr_sti_st0_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %2\n\t"
		"fldt %3\n\t"
		"fsubrp %%st(0), %%st(1)\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*sti), "=g" (status)
		: "m" (*st0), "m" (*sti)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_sub, x86_dep_st0, x86_dep_sti, 0, x86_dep_sti, 0, 0, 0);
}


void x86_isa_fsubrp_sti_st0_impl()
{
	x86_isa_fsubr_sti_st0_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fstcw_m16_impl()
{
	uint16_t value = x86_isa_regs->fpu_ctrl;

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fnstcw %0\n\t"
		:
		: "m" (value)
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_mem_write(x86_isa_mem, x86_isa_effective_address(), 2, &value);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_fpcw, 0, 0, x86_dep_mem32, 0, 0, 0);
}


void x86_isa_ftst_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, 0, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fucom_sti_impl()
{
	uint8_t st0[10], sti[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_sti, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fucomp_sti_impl()
{
	x86_isa_fucom_sti_impl();
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fucompp_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);
	x86_isa_pop_fpu(NULL);
	x86_isa_pop_fpu(NULL);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, x86_dep_sti, 0, x86_dep_fpst, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fxam_impl()
{
	uint8_t st0[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);

	__X86_ISA_FP_ASM_START__
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
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_comp, x86_dep_st0, 0, 0, x86_dep_fpst, 0, 0, 0);
}


void x86_isa_fxch_sti_impl()
{
	uint8_t st0[10], sti[10];
	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(x86_isa_inst.opindex, sti);
	x86_isa_store_fpu(0, sti);
	x86_isa_store_fpu(x86_isa_inst.opindex, st0);

	x86_uinst_new(x86_uinst_fp_move, x86_dep_st0, x86_dep_sti, 0, x86_dep_st0, x86_dep_sti, 0, 0);
}


void x86_isa_fyl2x_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fyl2x\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(1, st1);
	x86_isa_pop_fpu(NULL);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_log, x86_dep_st0, x86_dep_st1, 0, x86_dep_st1, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_fyl2xp1_impl()
{
	uint8_t st0[10], st1[10];
	uint16_t status;

	x86_isa_load_fpu(0, st0);
	x86_isa_load_fpu(1, st1);

	__X86_ISA_FP_ASM_START__
	asm volatile (
		"fldt %3\n\t"
		"fldt %2\n\t"
		"fyl2xp1\n\t"
		"fnstsw %%ax\n\t"
		"fstpt %0\n\t"
		"mov %%ax, %1\n\t"
		: "=m" (*st1), "=g" (status)
		: "m" (*st0), "m" (*st1)
		: "ax"
	);
	__X86_ISA_FP_ASM_END__

	x86_isa_store_fpu(1, st1);
	x86_isa_pop_fpu(NULL);
	x86_isa_store_fpu_code(status);

	x86_uinst_new(x86_uinst_fp_log, x86_dep_st0, x86_dep_st1, 0, x86_dep_st1, 0, 0, 0);
	x86_uinst_new(x86_uinst_fp_pop, 0, 0, 0, 0, 0, 0, 0);
}
