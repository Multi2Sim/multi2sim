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


#define op_stdop_al_imm8(stdop, wb) void op_##stdop##_al_imm8_impl() { \
	uint8_t al = isa_load_reg(reg_al); \
	uint8_t imm8 = isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (al) \
		: "m" (al), "m" (imm8), "g" (flags) \
		: "al" \
	); \
	if (wb) \
		isa_store_reg(reg_al, al); \
	isa_regs->eflags = flags; \
}


#define op_stdop_ax_imm16(stdop, wb) void op_##stdop##_ax_imm16_impl() { \
	uint16_t ax = isa_load_reg(reg_ax); \
	uint16_t imm16 = isa_inst.imm.w; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (ax) \
		: "m" (ax), "m" (imm16), "g" (flags) \
		: "ax" \
	); \
	if (wb) \
		isa_store_reg(reg_ax, ax); \
	isa_regs->eflags = flags; \
}


#define op_stdop_eax_imm32(stdop, wb) void op_##stdop##_eax_imm32_impl() { \
	uint32_t eax = isa_load_reg(reg_eax); \
	uint32_t imm32 = isa_inst.imm.d; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (eax) \
		: "m" (eax), "m" (imm32), "g" (flags) \
		: "eax" \
	); \
	if (wb) \
		isa_store_reg(reg_eax, eax); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm8_imm8(stdop, wb) void op_##stdop##_rm8_imm8_impl() { \
	uint8_t rm8 = isa_load_rm8(); \
	uint8_t imm8 = isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (imm8), "g" (flags) \
		: "al" \
	); \
	if (wb) \
		isa_store_rm8(rm8); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm16_imm16(stdop, wb) void op_##stdop##_rm16_imm16_impl() { \
	uint16_t rm16 = isa_load_rm16(); \
	uint16_t imm16 = isa_inst.imm.w; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (imm16), "g" (flags) \
		: "ax" \
	); \
	if (wb) \
		isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm32_imm32(stdop, wb) void op_##stdop##_rm32_imm32_impl() { \
	uint32_t rm32 = isa_load_rm32(); \
	uint32_t imm32 = isa_inst.imm.d; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (imm32), "g" (flags) \
		: "eax" \
	); \
	if (wb) \
		isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm16_imm8(stdop, wb) void op_##stdop##_rm16_imm8_impl() { \
	uint16_t rm16 = isa_load_rm16(); \
	uint16_t imm8 = (int8_t) isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (imm8), "g" (flags) \
		: "ax" \
	); \
	if (wb) \
		isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm32_imm8(stdop, wb) void op_##stdop##_rm32_imm8_impl() { \
	uint32_t rm32 = isa_load_rm32(); \
	uint32_t imm8 = (int8_t) isa_inst.imm.b; \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (imm8), "g" (flags) \
		: "eax" \
	); \
	if (wb) \
		isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm8_r8(stdop, wb) void op_##stdop##_rm8_r8_impl() { \
	uint8_t rm8 = isa_load_rm8(); \
	uint8_t r8 = isa_load_r8(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (r8), "g" (flags) \
		: "al" \
	); \
	if (wb) \
		isa_store_rm8(rm8); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm16_r16(stdop, wb) void op_##stdop##_rm16_r16_impl() { \
	uint16_t rm16 = isa_load_rm16(); \
	uint16_t r16 = isa_load_r16(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (r16), "g" (flags) \
		: "ax" \
	); \
	if (wb) \
		isa_store_rm16(rm16); \
	isa_regs->eflags = flags; \
}


#define op_stdop_rm32_r32(stdop, wb) void op_##stdop##_rm32_r32_impl() { \
	uint32_t rm32 = isa_load_rm32(); \
	uint32_t r32 = isa_load_r32(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (r32), "g" (flags) \
		: "eax" \
	); \
	if (wb) \
		isa_store_rm32(rm32); \
	isa_regs->eflags = flags; \
}


#define op_stdop_r8_rm8(stdop, wb) void op_##stdop##_r8_rm8_impl() { \
	uint8_t r8 = isa_load_r8(); \
	uint8_t rm8 = isa_load_rm8(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%al\n\t" \
		#stdop " %3, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (r8) \
		: "m" (r8), "m" (rm8), "g" (flags) \
		: "al" \
	); \
	if (wb) \
		isa_store_r8(r8); \
	isa_regs->eflags = flags; \
}


#define op_stdop_r16_rm16(stdop, wb) void op_##stdop##_r16_rm16_impl() { \
	uint16_t r16 = isa_load_r16(); \
	uint16_t rm16 = isa_load_rm16(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%ax\n\t" \
		#stdop " %3, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (r16) \
		: "m" (r16), "m" (rm16), "g" (flags) \
		: "ax" \
	); \
	if (wb) \
		isa_store_r16(r16); \
	isa_regs->eflags = flags; \
}


#define op_stdop_r32_rm32(stdop, wb) void op_##stdop##_r32_rm32_impl() { \
	uint32_t r32 = isa_load_r32(); \
	uint32_t rm32 = isa_load_rm32(); \
	unsigned long flags = isa_regs->eflags; \
	asm volatile ( \
		"push %4 ; popf\n\t" \
		"mov %2, %%eax\n\t" \
		#stdop " %3, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf ; pop %0\n\t" \
		: "=g" (flags), "=m" (r32) \
		: "m" (r32), "m" (rm32), "g" (flags) \
		: "eax" \
	); \
	if (wb) \
		isa_store_r32(r32); \
	isa_regs->eflags = flags; \
}


#define op_stdop_all(stdop, wb) \
	op_stdop_al_imm8(stdop, wb) \
	op_stdop_ax_imm16(stdop, wb) \
	op_stdop_eax_imm32(stdop, wb) \
	op_stdop_rm8_imm8(stdop, wb) \
	op_stdop_rm16_imm16(stdop, wb) \
	op_stdop_rm32_imm32(stdop, wb) \
	op_stdop_rm16_imm8(stdop, wb) \
	op_stdop_rm32_imm8(stdop, wb) \
	op_stdop_rm8_r8(stdop, wb) \
	op_stdop_rm16_r16(stdop, wb) \
	op_stdop_rm32_r32(stdop, wb) \
	op_stdop_r8_rm8(stdop, wb) \
	op_stdop_r16_rm16(stdop, wb) \
	op_stdop_r32_rm32(stdop, wb)


op_stdop_all(adc, 1)
op_stdop_all(add, 1)
op_stdop_all(and, 1)
op_stdop_all(cmp, 0)
op_stdop_all(or, 1)
op_stdop_all(sbb, 1)
op_stdop_all(sub, 1)
op_stdop_all(test, 0)
op_stdop_all(xor, 1)
