/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpukernel.h>


#define op_xxx_rm8_1_impl(xxx, idep) \
void op_##xxx##_rm8_1_impl() \
{ \
	uint8_t rm8 = isa_load_rm8(); \
	uint8_t count = 1; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	isa_store_rm8(rm8); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm8, 0, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm8_cl_impl(xxx, idep) \
void op_##xxx##_rm8_cl_impl() \
{ \
	uint8_t rm8 = isa_load_rm8(); \
	uint8_t count = isa_load_reg(x86_reg_cl); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	isa_store_rm8(rm8); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm8, x86_dep_ecx, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm8_imm8_impl(xxx, idep) \
void op_##xxx##_rm8_imm8_impl() \
{ \
	uint8_t rm8 = isa_load_rm8(); \
	uint8_t count = isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	isa_store_rm8(rm8); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm8, 0, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_1_impl(xxx, idep) \
void op_##xxx##_rm16_1_impl() \
{ \
	uint16_t rm16 = isa_load_rm16(); \
	uint8_t count = 1; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm16, 0, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_cl_impl(xxx, idep) \
void op_##xxx##_rm16_cl_impl() \
{ \
	uint16_t rm16 = isa_load_rm16(); \
	uint8_t count = isa_load_reg(x86_reg_cl); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm16, x86_dep_ecx, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_imm8_impl(xxx, idep) \
void op_##xxx##_rm16_imm8_impl() \
{ \
	uint16_t rm16 = isa_load_rm16(); \
	uint8_t count = isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm16, 0, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_1_impl(xxx, idep) \
void op_##xxx##_rm32_1_impl() \
{ \
	uint32_t rm32 = isa_load_rm32(); \
	uint8_t count = 1; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm32, 0, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_cl_impl(xxx, idep) \
void op_##xxx##_rm32_cl_impl() \
{ \
	uint32_t rm32 = isa_load_rm32(); \
	uint8_t count = isa_load_reg(x86_reg_cl); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm32, x86_dep_ecx, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_imm8_impl(xxx, idep) \
void op_##xxx##_rm32_imm8_impl() \
{ \
	uint32_t rm32 = isa_load_rm32(); \
	uint8_t count = isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"pushf\n\t" \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		"popf\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
	x86_uinst_new(x86_uinst_shift, idep, x86_dep_rm32, 0, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
}


/* Define all possible combinations of 8, 16 and 32 bit destination registers
 * and 1, cl or imm8 source operand. */
#define op_xxx_all(xxx, idep) \
	op_xxx_rm8_1_impl(xxx, idep) \
	op_xxx_rm8_cl_impl(xxx, idep) \
	op_xxx_rm8_imm8_impl(xxx, idep) \
	op_xxx_rm16_1_impl(xxx, idep) \
	op_xxx_rm16_cl_impl(xxx, idep) \
	op_xxx_rm16_imm8_impl(xxx, idep) \
	op_xxx_rm32_1_impl(xxx, idep) \
	op_xxx_rm32_cl_impl(xxx, idep) \
	op_xxx_rm32_imm8_impl(xxx, idep)


/* Implementation for rotation and shift instructions */
op_xxx_all(rcl, x86_dep_cf)
op_xxx_all(rcr, x86_dep_cf)
op_xxx_all(rol, 0)
op_xxx_all(ror, 0)

op_xxx_all(sar, 0)
op_xxx_all(shl, 0)
op_xxx_all(shr, 0)

