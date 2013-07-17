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


#include "context.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"
#include "uinst.h"


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


#define op_xxx_rm8_1_impl(xxx, idep) \
void x86_isa_##xxx##_rm8_1_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned char rm8 = X86ContextLoadRm8(ctx); \
	unsigned char count = 1; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm8(ctx, rm8); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm8, 0, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm8_cl_impl(xxx, idep) \
void x86_isa_##xxx##_rm8_cl_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned char rm8 = X86ContextLoadRm8(ctx); \
	unsigned char count = X86ContextLoadReg(ctx, x86_inst_reg_cl); \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm8(ctx, rm8); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm8, x86_dep_ecx, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm8_imm8_impl(xxx, idep) \
void x86_isa_##xxx##_rm8_imm8_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned char rm8 = X86ContextLoadRm8(ctx); \
	unsigned char count = ctx->inst.imm.b; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%al\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%al\n\t" \
		"mov %%al, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm8) \
		: "m" (rm8), "m" (count), "g" (flags) \
		: "al", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm8(ctx, rm8); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm8, 0, x86_dep_rm8, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_1_impl(xxx, idep) \
void x86_isa_##xxx##_rm16_1_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned short rm16 = X86ContextLoadRm16(ctx); \
	unsigned char count = 1; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm16(ctx, rm16); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm16, 0, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_cl_impl(xxx, idep) \
void x86_isa_##xxx##_rm16_cl_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned short rm16 = X86ContextLoadRm16(ctx); \
	unsigned char count = X86ContextLoadReg(ctx, x86_inst_reg_cl); \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm16(ctx, rm16); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm16, x86_dep_ecx, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm16_imm8_impl(xxx, idep) \
void x86_isa_##xxx##_rm16_imm8_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned short rm16 = X86ContextLoadRm16(ctx); \
	unsigned char count = ctx->inst.imm.b; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%ax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%ax\n\t" \
		"mov %%ax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm16) \
		: "m" (rm16), "m" (count), "g" (flags) \
		: "ax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm16(ctx, rm16); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm16, 0, x86_dep_rm16, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_1_impl(xxx, idep) \
void x86_isa_##xxx##_rm32_1_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned int rm32 = X86ContextLoadRm32(ctx); \
	unsigned char count = 1; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm32(ctx, rm32); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm32, 0, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_cl_impl(xxx, idep) \
void x86_isa_##xxx##_rm32_cl_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned int rm32 = X86ContextLoadRm32(ctx); \
	unsigned char count = X86ContextLoadReg(ctx, x86_inst_reg_cl); \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm32(ctx, rm32); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm32, x86_dep_ecx, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
}


#define op_xxx_rm32_imm8_impl(xxx, idep) \
void x86_isa_##xxx##_rm32_imm8_impl(X86Context *ctx) \
{ \
	struct x86_regs_t *regs = ctx->regs; \
	unsigned int rm32 = X86ContextLoadRm32(ctx); \
	unsigned char count = ctx->inst.imm.b; \
	unsigned long flags = regs->eflags; \
	__X86_ISA_ASM_START__ \
	asm volatile ( \
		"push %4\n\t" \
		"popf\n\t" \
		"mov %2, %%eax\n\t" \
		"mov %3, %%cl\n\t" \
		#xxx " %%cl, %%eax\n\t" \
		"mov %%eax, %1\n\t" \
		"pushf\n\t" \
		"pop %0\n\t" \
		: "=g" (flags), "=m" (rm32) \
		: "m" (rm32), "m" (count), "g" (flags) \
		: "eax", "cl" \
	); \
	__X86_ISA_ASM_END__ \
	X86ContextStoreRm32(ctx, rm32); \
	regs->eflags = flags; \
	x86_uinst_new(ctx, x86_uinst_shift, idep, x86_dep_rm32, 0, x86_dep_rm32, x86_dep_cf, x86_dep_of, 0); \
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

