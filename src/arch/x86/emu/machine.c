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

#include <arch/common/arch.h>
#include <lib/util/misc.h>

#include "context.h"
#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"
#include "syscall.h"
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


void x86_isa_bound_r16_rm32_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_bound_r32_rm64_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_bsf_r32_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"bsf %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (r32)
		: "g" (r32), "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_r32, x86_dep_zps, 0, 0);
}


void x86_isa_bsr_r32_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"bsr %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (r32), "m" (rm32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_r32, x86_dep_zps, 0, 0);
}


void x86_isa_bswap_ir32_impl(X86Context *ctx)
{
	unsigned int ir32 = X86ContextLoadIR32(ctx);

	__X86_ISA_ASM_START__
	asm volatile (
		"mov %1, %%eax\n\t"
		"bswap %%eax\n\t"
		"mov %%eax, %0\n\t"
		: "=g" (ir32)
		: "g" (ir32)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreIR32(ctx, ir32);

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_ir32, 0, 0, x86_dep_ir32, 0, 0, 0);
}


void x86_isa_bt_rm32_r32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ecx\n\t"
		"mov %1, %%eax\n\t"
		"bt %%ecx, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (rm32), "m" (r32), "g" (flags)
		: "eax", "ecx"
	);
	__X86_ISA_ASM_END__

	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_cf, 0, 0, 0);
}


void x86_isa_bt_rm32_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int imm8 = ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ecx\n\t"
		"mov %1, %%eax\n\t"
		"bt %%ecx, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ecx"
	);
	__X86_ISA_ASM_END__

	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_cf, 0, 0, 0);
}


void x86_isa_bts_rm32_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int imm8 = ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %3, %%ecx\n\t"
		"mov %2, %%eax\n\t"
		"btsl %%ecx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ecx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_cf, 0, 0);
}


void x86_isa_call_rel32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	regs->esp -= 4;
	X86ContextMemWrite(ctx, regs->esp, 4, &regs->eip);
	ctx->target_eip = regs->eip + ctx->inst.imm.d;
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_call, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_call_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	ctx->target_eip = X86ContextLoadRm32(ctx);
	regs->esp -= 4;
	X86ContextMemWrite(ctx, regs->esp, 4, &regs->eip);
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_call, x86_dep_rm32, 0, 0, 0, 0, 0, 0);
}


void x86_isa_cbw_impl(X86Context *ctx)
{
	unsigned short ax = (char) X86ContextLoadReg(ctx, x86_inst_reg_al);
	X86ContextStoreReg(ctx, x86_inst_reg_ax, ax);

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_cdq_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	int signed_eax = regs->eax;

	regs->edx = signed_eax < 0 ? -1 : 0;

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_edx, 0, 0, 0);
}


void x86_isa_cld_impl(X86Context *ctx)
{
	X86ContextClearFlag(ctx, x86_inst_flag_df);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, 0, x86_dep_df, 0, 0);
}


void x86_isa_cmpxchg_rm32_r32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int eax = regs->eax;
	unsigned long flags = regs->eflags;
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %4, %%eax\n\t"
		"mov %5, %%ebx\n\t"
		"mov %6, %%ecx\n\t"
		"cmpxchg %%ecx, %%ebx\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		"mov %%eax, %1\n\t"
		"mov %%ebx, %2\n\t"
		: "=g" (flags), "=m" (eax), "=m" (rm32)
		: "g" (flags), "m" (eax), "m" (rm32), "m" (r32)
		: "eax", "ebx", "ecx"
	);
	__X86_ISA_ASM_END__

	regs->eflags = flags;
	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);
	X86ContextStoreRm32(ctx, rm32);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_eax, x86_dep_rm32, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_zps, x86_dep_r32, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_cf, x86_dep_rm32, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_cmpxchg8b_m64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int eax, ebx, ecx, edx;
	unsigned long long edx_eax, m64;

	eax = regs->eax;
	ebx = regs->ebx;
	ecx = regs->ecx;
	edx = regs->edx;
	edx_eax = ((unsigned long long) edx << 32) | eax;
	m64 = X86ContextLoadM64(ctx);

	if (edx_eax == m64)
	{
		X86ContextSetFlag(ctx, x86_inst_flag_zf);
		m64 = ((unsigned long long) ecx << 32) | ebx;
		X86ContextStoreM64(ctx, m64);
	}
	else
	{
		X86ContextClearFlag(ctx, x86_inst_flag_zf);
		regs->edx = m64 >> 32;
		regs->eax = m64;
	}

	x86_uinst_new_mem(ctx, x86_uinst_load, X86ContextEffectiveAddress(ctx), 8, 0, 0, 0,
		x86_dep_aux, 0, 0, 0);  /* Load m64 */
	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_edx, x86_dep_eax, x86_dep_aux,
		x86_dep_zps, 0, 0, 0);  /* Compare edx-eax with m64 */
	x86_uinst_new_mem(ctx, x86_uinst_store, X86ContextEffectiveAddress(ctx), 8, x86_dep_zps, x86_dep_ecx, x86_dep_ebx,
		x86_dep_mem64, 0, 0, 0);  /* Conditionally store m64 */
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_zps, 0, 0,
		x86_dep_edx, x86_dep_eax, 0, 0);  /* Conditionaly store edx-eax */
}


void x86_isa_cpuid_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int info = 0;

	switch (regs->eax)
	{

	case 0x0:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x2);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0x756e6547);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0x6c65746e);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0x49656e69);
		break;

	case 0x1:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x00000f29);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0x0102080b);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0x00004400);

		/* EDX register returns CPU features information. */
		info = SETBITVALUE32(info, 31, 1);  /* PBE - Pend Brk En */
		info = SETBITVALUE32(info, 29, 1);  /* TM - Therm Monitor */
		info = SETBITVALUE32(info, 28, 1);  /* HTT - Hyper-threading Tech. */
		info = SETBITVALUE32(info, 27, 1);  /* SS - Self snoop */
		info = SETBITVALUE32(info, 26, 0);  /* SSE2 - SSE2 Extensions */
		info = SETBITVALUE32(info, 25, 0);  /* SSE - SSE Extensions */
		info = SETBITVALUE32(info, 24, 1);  /* FXSR - FXSAVE/FXRSTOR */
		info = SETBITVALUE32(info, 23, 0);  /* MMX - MMX Technology */
		info = SETBITVALUE32(info, 22, 1);  /* ACPI - Thermal Monitor and Clock Ctrl */
		info = SETBITVALUE32(info, 21, 1);  /* DS - Debug Store */
		info = SETBITVALUE32(info, 19, 1);  /* CLFSH - CFLUSH instruction */
		info = SETBITVALUE32(info, 18, 0);  /* PSN - Processor Serial Number */
		info = SETBITVALUE32(info, 17, 1);  /* PSE - Page size extension */
		info = SETBITVALUE32(info, 16, 1);  /* PAT - Page Attribute Table */
		info = SETBITVALUE32(info, 15, 1);  /* CMOV - Conditional Move/Compare Instruction */
		info = SETBITVALUE32(info, 14, 1);  /* MCA - Machine Check Architecture */
		info = SETBITVALUE32(info, 13, 1);  /* PGE - PTE Global bit */
		info = SETBITVALUE32(info, 12, 1);  /* MTRR - Memory Type Range Registers */
		info = SETBITVALUE32(info, 11, 1);  /* SEP - SYSENTER and SYSEXIT */
		info = SETBITVALUE32(info, 9, 1);  /* APIC - APIC on Chip */
		info = SETBITVALUE32(info, 8, 1);  /* CX8 - CMPXCHG8B inst. */
		info = SETBITVALUE32(info, 7, 1);  /* MCE - Machine Check Exception */
		info = SETBITVALUE32(info, 6, 1);  /* PAE - Physical Address Extensions */
		info = SETBITVALUE32(info, 5, 1);  /* MSR - RDMSR and WRMSR Support */
		info = SETBITVALUE32(info, 4, 1);  /* TSC - Time Stamp Counter */
		info = SETBITVALUE32(info, 3, 1);  /* PSE - Page Size Extensions */
		info = SETBITVALUE32(info, 2, 1);  /* DE - Debugging Extensions */
		info = SETBITVALUE32(info, 1, 1);  /* VME - Virtual-8086 Mode Enhancement */
		info = SETBITVALUE32(info, 0, 1);  /* FPU - x87 FPU on Chip */

		X86ContextStoreReg(ctx, x86_inst_reg_edx, info);
		break;

	case 0x2:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0);
		break;

	case 0x80000000:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x80000004);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0);
		break;

	case 0x80000001:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0);
		break;

	case 0x80000002:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x20202020);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0x20202020);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0x20202020);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0x20202020);
		break;

	case 0x80000003:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x6e492020);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0x286c6574);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0x58202952);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0x286e6f65);
		break;

	case 0x80000004:

		X86ContextStoreReg(ctx, x86_inst_reg_eax, 0x20294d54);
		X86ContextStoreReg(ctx, x86_inst_reg_ebx, 0x20555043);
		X86ContextStoreReg(ctx, x86_inst_reg_ecx, 0x30382e32);
		X86ContextStoreReg(ctx, x86_inst_reg_edx, 0x7a4847);
		break;

	default:

		X86ContextError(ctx, "inst 'cpuid' not implemented for eax=0x%x", regs->eax);
	}

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_eax, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_ebx, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_ecx, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_edx, 0, 0, 0);
}


void x86_isa_cwde_impl(X86Context *ctx)
{
	unsigned int eax = (short) X86ContextLoadReg(ctx, x86_inst_reg_ax);
	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_dec_rm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned char rm8 = X86ContextLoadRm8(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"dec %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm8(ctx, rm8);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_rm8, 0, 0, x86_dep_rm8, x86_dep_zps, x86_dep_of, 0);
}


void x86_isa_dec_rm16_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short rm16 = X86ContextLoadRm16(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"dec %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm16)
		: "g" (rm16), "g" (flags)
		: "ax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm16(ctx, rm16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_rm16, 0, 0, x86_dep_rm16, x86_dep_zps, x86_dep_of, 0);
}


void x86_isa_dec_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"dec %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_zps, x86_dep_of, 0);
}


void x86_isa_dec_ir16_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short ir16 = X86ContextLoadIR16(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"dec %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir16)
		: "g" (ir16), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreIR16(ctx, ir16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_ir16, 0, 0, x86_dep_ir16, x86_dep_zps, x86_dep_of, 0);
}


void x86_isa_dec_ir32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int ir32 = X86ContextLoadIR32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"dec %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir32)
		: "g" (ir32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreIR32(ctx, ir32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_ir32, 0, 0, x86_dep_ir32, x86_dep_zps, x86_dep_of, 0);
}


void x86_isa_div_rm8_impl(X86Context *ctx)
{
	int skip_emulation;
	int spec_mode;

	unsigned short ax = X86ContextLoadReg(ctx, x86_inst_reg_ax);
	unsigned char rm8 = X86ContextLoadRm8(ctx);

	if (!rm8) {
		X86ContextError(ctx, "%s: division by 0", __FUNCTION__);
		return;
	}

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xff. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
	skip_emulation = spec_mode && ax > 0xff;

	/* Emulate */
	if (!skip_emulation)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"mov %1, %%ax\n\t"
			"mov %2, %%bl\n\t"
			"div %%bl\n\t"
			"mov %%ax, %0\n\t"
			: "=m" (ax)
			: "m" (ax), "m" (rm8)
			: "ax", "bl"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreReg(ctx, x86_inst_reg_ax, ax);

	x86_uinst_new(ctx, x86_uinst_div, x86_dep_eax, x86_dep_rm8, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_div_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int spec_mode;
	int skip_emulation;

	unsigned int eax = regs->eax;
	unsigned int edx = regs->edx;
	unsigned int rm32 = X86ContextLoadRm32(ctx);

	if (!rm32) {
		X86ContextError(ctx, "%s: division by 0", __FUNCTION__);
		return;
	}

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xffffffff. */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
	skip_emulation = spec_mode && edx;

	/* Emulate */
	if (!skip_emulation)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"mov %2, %%eax\n\t"
			"mov %3, %%edx\n\t"
			"mov %4, %%ebx\n\t"
			"div %%ebx\n\t"
			"mov %%eax, %0\n\t"
			"mov %%edx, %1\n\t"
			: "=m" (eax), "=m" (edx)
			: "m" (eax), "m" (edx), "m" (rm32)
			: "eax", "edx", "ebx"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);
	X86ContextStoreReg(ctx, x86_inst_reg_edx, edx);

	x86_uinst_new(ctx, x86_uinst_div, x86_dep_edx, x86_dep_eax, x86_dep_rm32, x86_dep_eax, x86_dep_edx, 0, 0);
}


void x86_isa_hlt_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: 'hlt' instruction", __FUNCTION__);
}


void x86_isa_idiv_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int spec_mode;
	int skip_emulation;

	unsigned int eax = regs->eax;
	unsigned int edx = regs->edx;
	unsigned int rm32 = X86ContextLoadRm32(ctx);

	if (!rm32)
	{
		X86ContextError(ctx, "%s: division by 0", __FUNCTION__);
		return;
	}

	/* Avoid emulation in speculative mode if it could cause a divide exception */
	skip_emulation = 0;
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
	if (spec_mode)
	{
		long long edx_eax = ((unsigned long long) edx << 32) | eax;
		if (edx_eax > 0x7fffffffll || edx_eax < 0xffffffff80000000ll)
			skip_emulation = 1;
	}

	if (!skip_emulation)
	{
		__X86_ISA_ASM_START__
		asm volatile (
			"mov %2, %%eax\n\t"
			"mov %3, %%edx\n\t"
			"mov %4, %%ebx\n\t"
			"idiv %%ebx\n\t"
			"mov %%eax, %0\n\t"
			"mov %%edx, %1\n\t"
			: "=m" (eax), "=m" (edx)
			: "m" (eax), "m" (edx), "m" (rm32)
			: "eax", "edx", "ebx"
		);
		__X86_ISA_ASM_END__
	}

	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);
	X86ContextStoreReg(ctx, x86_inst_reg_edx, edx);

	x86_uinst_new(ctx, x86_uinst_div, x86_dep_rm32, x86_dep_eax, 0, x86_dep_eax, x86_dep_edx, 0, 0);
}


void x86_isa_imul_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int eax = X86ContextLoadReg(ctx, x86_inst_reg_eax);
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;
	unsigned int edx;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %3, %%eax\n\t"
		"mov %4, %%edx\n\t"
		"imul %%edx\n\t"
		"mov %%edx, %1\n\t"
		"mov %%eax, %2\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (edx), "=m" (eax)
		: "m" (eax), "m" (rm32), "g" (flags)
		: "eax", "edx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);
	X86ContextStoreReg(ctx, x86_inst_reg_edx, edx);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_mult, x86_dep_rm32, x86_dep_eax, 0, x86_dep_eax, x86_dep_edx, x86_dep_cf, x86_dep_of);
}


void x86_isa_imul_r32_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (r32), "m" (rm32), "g" (flags)
		: "eax", "ebx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_mult, x86_dep_r32, x86_dep_rm32, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void x86_isa_imul_r32_rm32_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int r32;
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int imm8 = (char) ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (rm32), "m" (imm8), "g" (flags)
		: "eax", "ebx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_mult, x86_dep_rm32, 0, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void x86_isa_imul_r32_rm32_imm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int r32;
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int imm32 = ctx->inst.imm.d;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"mov %3, %%ebx\n\t"
		"imul %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (r32)
		: "m" (rm32), "m" (imm32), "g" (flags)
		: "eax", "ebx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, r32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_mult, x86_dep_rm32, 0, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void x86_isa_inc_rm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned char rm8 = X86ContextLoadRm8(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"inc %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm8(ctx, rm8);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_rm8, 0, 0, x86_dep_rm8, 0, x86_dep_zps, x86_dep_of);
}


void x86_isa_inc_rm16_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short rm16 = X86ContextLoadRm16(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"inc %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm16)
		: "g" (rm16), "g" (flags)
		: "ax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm16(ctx, rm16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_rm16, 0, 0, x86_dep_rm16, 0, x86_dep_zps, x86_dep_of);
}


void x86_isa_inc_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"inc %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_rm32, 0, 0, x86_dep_rm32, 0, x86_dep_zps, x86_dep_of);
}


void x86_isa_inc_ir16_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short ir16 = X86ContextLoadIR16(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%ax\n\t"
		"inc %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir16)
		: "g" (ir16), "g" (flags)
		: "ax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreIR16(ctx, ir16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_ir16, 0, 0, x86_dep_ir16, 0, x86_dep_zps, x86_dep_of);
}


void x86_isa_inc_ir32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int ir32 = X86ContextLoadIR32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"inc %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (ir32)
		: "g" (ir32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreIR32(ctx, ir32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_ir32, 0, 0, x86_dep_ir32, 0, x86_dep_zps, x86_dep_of);
}


void x86_isa_int_3_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_int_imm8_impl(X86Context *ctx)
{
	int spec_mode;
	unsigned int num;

	/* Interrupt code */
	num = (unsigned char) ctx->inst.imm.b;
	if (num != 0x80)
		X86ContextError(ctx, "%s: not supported for num != 0x80", __FUNCTION__);

	/* Do system call if not in speculative mode */
	spec_mode = X86ContextGetState(ctx, X86ContextSpecMode);
	if (!spec_mode)
		X86ContextSyscall(ctx);

	x86_uinst_new(ctx, x86_uinst_syscall, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_into_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}



void x86_isa_jmp_rel8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	ctx->target_eip = regs->eip + (char) ctx->inst.imm.b;
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_jump, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_jmp_rel32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	ctx->target_eip = regs->eip + ctx->inst.imm.d;
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_jump, 0, 0, 0, 0, 0, 0, 0);
}


void x86_isa_jmp_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	ctx->target_eip = X86ContextLoadRm32(ctx);
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_jump, x86_dep_rm32, 0, 0, 0, 0, 0, 0);
}


void x86_isa_lea_r32_m_impl(X86Context *ctx)
{
	unsigned int value = X86ContextEffectiveAddress(ctx);

	if (ctx->inst.segment)
	{
		X86ContextError(ctx, "%s: not supported for this segment", __FUNCTION__);
		return;
	}

	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_easeg, x86_dep_eabas, x86_dep_eaidx, x86_dep_r32, 0, 0, 0);
}


void x86_isa_leave_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int value;
	regs->esp = regs->ebp;

	if (ctx->inst.segment)
	{
		X86ContextError(ctx, "%s: not supported segment", __FUNCTION__);
		return;
	}

	X86ContextMemRead(ctx, regs->esp, 4, &value);
	regs->esp += 4;
	X86ContextStoreReg(ctx, x86_inst_reg_ebp, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_ebp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_ebp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void x86_isa_lfence_impl(X86Context *ctx)
{
}


void x86_isa_lock_xadd_rm8_r8_impl(X86Context *ctx)
{
	x86_isa_xadd_rm8_r8_impl(ctx);
}


void x86_isa_lock_xadd_rm32_r32_impl(X86Context *ctx)
{
	x86_isa_xadd_rm32_r32_impl(ctx);
}


void x86_isa_mov_rm8_imm8_impl(X86Context *ctx)
{
	unsigned char value = ctx->inst.imm.b;
	X86ContextStoreRm8(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void x86_isa_mov_r8_rm8_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadRm8(ctx);
	X86ContextStoreR8(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r8, 0, 0, 0);
}


void x86_isa_mov_rm8_r8_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadR8(ctx);
	X86ContextStoreRm8(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_r8, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void x86_isa_mov_rm16_r16_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadR16(ctx);
	X86ContextStoreRm16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_r16, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void x86_isa_mov_rm32_r32_impl(X86Context *ctx)
{
	unsigned int value = X86ContextLoadR32(ctx);
	X86ContextStoreRm32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_r32, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void x86_isa_mov_r16_rm16_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadRm16(ctx);
	X86ContextStoreR16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_r16, 0, 0, 0);
}


void x86_isa_mov_r32_rm32_impl(X86Context *ctx)
{
	unsigned int value = X86ContextLoadRm32(ctx);
	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm32, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_mov_al_moffs8_impl(X86Context *ctx)
{
	unsigned char value;

	X86ContextMemRead(ctx, X86ContextMoffsAddress(ctx), 1, &value);
	X86ContextStoreReg(ctx, x86_inst_reg_al, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, X86ContextMoffsAddress(ctx), 1, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_mov_ax_moffs16_impl(X86Context *ctx)
{
	unsigned short value;

	X86ContextMemRead(ctx, X86ContextMoffsAddress(ctx), 2, &value);
	X86ContextStoreReg(ctx, x86_inst_reg_ax, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, X86ContextMoffsAddress(ctx), 2, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_mov_eax_moffs32_impl(X86Context *ctx)
{
	unsigned int value;

	X86ContextMemRead(ctx, X86ContextMoffsAddress(ctx), 4, &value);
	X86ContextStoreReg(ctx, x86_inst_reg_eax, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, X86ContextMoffsAddress(ctx), 4, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void x86_isa_mov_moffs8_al_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadReg(ctx, x86_inst_reg_al);
	X86ContextMemWrite(ctx, X86ContextMoffsAddress(ctx), 1, &value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, X86ContextMoffsAddress(ctx), 1, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void x86_isa_mov_moffs16_ax_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadReg(ctx, x86_inst_reg_ax);
	X86ContextMemWrite(ctx, X86ContextMoffsAddress(ctx), 2, &value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, X86ContextMoffsAddress(ctx), 2, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void x86_isa_mov_moffs32_eax_impl(X86Context *ctx)
{
	unsigned int value = X86ContextLoadReg(ctx, x86_inst_reg_eax);
	X86ContextMemWrite(ctx, X86ContextMoffsAddress(ctx), 4, &value);

	x86_uinst_new(ctx, x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, X86ContextMoffsAddress(ctx), 4, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void x86_isa_mov_ir8_imm8_impl(X86Context *ctx)
{
	unsigned char value = ctx->inst.imm.b;
	X86ContextStoreIR8(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_ir8, 0, 0, 0);
}


void x86_isa_mov_ir16_imm16_impl(X86Context *ctx)
{
	unsigned short value = ctx->inst.imm.w;
	X86ContextStoreIR16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_ir16, 0, 0, 0);
}


void x86_isa_mov_ir32_imm32_impl(X86Context *ctx)
{
	unsigned int value = ctx->inst.imm.d;
	X86ContextStoreIR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_ir32, 0, 0, 0);
}


void x86_isa_mov_rm16_imm16_impl(X86Context *ctx)
{
	unsigned short value = ctx->inst.imm.w;
	X86ContextStoreRm16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void x86_isa_mov_rm32_imm32_impl(X86Context *ctx)
{
	unsigned int value = ctx->inst.imm.d;
	X86ContextStoreRm32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void x86_isa_mov_rm16_sreg_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadSReg(ctx);
	if (ctx->inst.reg != 5)
		X86ContextError(ctx, "%s: not supported for sreg != gs", __FUNCTION__);
	X86ContextStoreRm16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_sreg, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void x86_isa_mov_rm32_sreg_impl(X86Context *ctx)
{
	x86_isa_mov_rm16_sreg_impl(ctx);
}


void x86_isa_mov_sreg_rm16_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadRm16(ctx);
	if (ctx->inst.reg != 5)
		X86ContextError(ctx, "%s: not supported for sreg != gs", __FUNCTION__);
	X86ContextStoreSReg(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_sreg, 0, 0, 0);
}


void x86_isa_mov_sreg_rm32_impl(X86Context *ctx)
{
	x86_isa_mov_sreg_rm16_impl(ctx);
}


void x86_isa_movsx_r16_rm8_impl(X86Context *ctx)
{
	unsigned short value = (char) X86ContextLoadRm8(ctx);
	X86ContextStoreR16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_rm8, 0, 0, x86_dep_r16, 0, 0, 0);
}


void x86_isa_movsx_r32_rm8_impl(X86Context *ctx)
{
	unsigned int value = (char) X86ContextLoadRm8(ctx);
	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_rm8, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movsx_r32_rm16_impl(X86Context *ctx)
{
	unsigned int value = (short) X86ContextLoadRm16(ctx);
	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_sign, x86_dep_rm16, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movzx_r16_rm8_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadRm8(ctx);
	X86ContextStoreR16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r16, 0, 0, 0);
}


void x86_isa_movzx_r32_rm8_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadRm8(ctx);
	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_movzx_r32_rm16_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadRm16(ctx);
	X86ContextStoreR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_mul_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int eax = X86ContextLoadReg(ctx, x86_inst_reg_eax);
	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;
	unsigned int edx;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %3, %%eax\n\t"
		"mov %4, %%edx\n\t"
		"mul %%edx\n\t"
		"mov %%edx, %1\n\t"
		"mov %%eax, %2\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (edx), "=m" (eax)
		: "m" (eax), "m" (rm32), "g" (flags)
		: "eax", "edx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);
	X86ContextStoreReg(ctx, x86_inst_reg_edx, edx);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_mult, x86_dep_rm32, x86_dep_eax, 0, x86_dep_edx, x86_dep_eax, x86_dep_of, x86_dep_cf);
}


void x86_isa_neg_rm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned char rm8 = X86ContextLoadRm8(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"neg %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm8)
		: "g" (rm8), "g" (flags)
		: "al"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm8(ctx, rm8);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_rm8, 0, 0, x86_dep_rm8, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_neg_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"neg %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=g" (rm32)
		: "g" (rm32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_nop_impl(X86Context *ctx)
{
}


void x86_isa_nop_rm16_impl(X86Context *ctx)
{
}


void x86_isa_nop_rm32_impl(X86Context *ctx)
{
}


void x86_isa_not_rm8_impl(X86Context *ctx)
{
	unsigned char value = X86ContextLoadRm8(ctx);

	value = ~value;
	X86ContextStoreRm8(ctx, value);

	x86_uinst_new(ctx, x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void x86_isa_not_rm16_impl(X86Context *ctx)
{
	unsigned short value = X86ContextLoadRm16(ctx);
	value = ~value;
	X86ContextStoreRm16(ctx, value);

	x86_uinst_new(ctx, x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void x86_isa_not_rm32_impl(X86Context *ctx)
{
	unsigned int value = X86ContextLoadRm32(ctx);
	value = ~value;
	X86ContextStoreRm32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void x86_isa_out_imm8_al_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_out_imm8_ax_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_out_imm8_eax_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_out_dx_al_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_out_dx_ax_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_out_dx_eax_impl(X86Context *ctx)
{
	X86ContextError(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pause_impl(X86Context *ctx)
{
}


void x86_isa_pop_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value;

	X86ContextMemRead(ctx, regs->esp, 4, &value);
	regs->esp += 4;
	X86ContextStoreRm32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void x86_isa_pop_ir32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value;

	if (ctx->inst.segment)
	{
		X86ContextError(ctx, "%s: not supported segment", __FUNCTION__);
		return;
	}

	X86ContextMemRead(ctx, regs->esp, 4, &value);
	regs->esp += 4;
	X86ContextStoreIR32(ctx, value);

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_ir32, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void x86_isa_popf_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	X86ContextMemRead(ctx, regs->esp, 4, &regs->eflags);
	regs->esp += 4;

	/* Prevent TF from being set. A program should never do this, but it could
	 * happen during speculative execution (case reported by Multi2Sim user).
	 * The next instruction that is emulated in speculative mode could cause the
	 * host to push this value of 'eflags', causing a TRAP in the host code. */
	regs->eflags &= ~(1 << 8);

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void x86_isa_prefetcht0_m8_impl(X86Context *ctx)
{
	unsigned int eff_addr;

	/* prefetching makes sense only in a detailed simulation */
	if (arch_x86->sim_kind != arch_sim_kind_detailed)
		return;

	if (!x86_emu_process_prefetch_hints)
		return;

	eff_addr = X86ContextEffectiveAddress(ctx);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_easeg, x86_dep_eabas, x86_dep_eaidx, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_prefetch, eff_addr, 1, x86_dep_aux, 0, 0, 0, 0, 0, 0);
}


void x86_isa_prefetcht1_impl(X86Context *ctx)
{
}


void x86_isa_prefetcht2_impl(X86Context *ctx)
{
}


void x86_isa_prefetchnta_impl(X86Context *ctx)
{
}


void x86_isa_push_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value = (char) ctx->inst.imm.b;

	X86ContextStoreReg(ctx, x86_inst_reg_esp, regs->esp - 4);
	X86ContextMemWrite(ctx, regs->esp, 4, &value);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
}


void x86_isa_push_imm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value = ctx->inst.imm.d;

	X86ContextStoreReg(ctx, x86_inst_reg_esp, regs->esp - 4);
	X86ContextMemWrite(ctx, regs->esp, 4, &value);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
}


void x86_isa_push_rm32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value = X86ContextLoadRm32(ctx);

	X86ContextStoreReg(ctx, x86_inst_reg_esp, regs->esp - 4);
	X86ContextMemWrite(ctx, regs->esp, 4, &value);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, x86_dep_rm32, 0, 0, 0, 0, 0);
}


void x86_isa_push_ir32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned int value = X86ContextLoadIR32(ctx);

	X86ContextStoreReg(ctx, x86_inst_reg_esp, regs->esp - 4);
	X86ContextMemWrite(ctx, regs->esp, 4, &value);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, x86_dep_ir32, 0, 0, 0, 0, 0);
}


void x86_isa_pushf_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	X86ContextStoreReg(ctx, x86_inst_reg_esp, regs->esp - 4);
	X86ContextMemWrite(ctx, regs->esp, 4, &regs->eflags);

	x86_uinst_new(ctx, x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_zps, x86_dep_cf, x86_dep_of, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_store, regs->esp, 4, x86_dep_aux, x86_dep_aux2, 0, 0, 0, 0, 0);
}


void x86_isa_rdtsc_impl(X86Context *ctx)
{
	unsigned int eax, edx;

	__X86_ISA_ASM_START__
	asm volatile (
		"rdtsc\n\t"
		"mov %%eax, %0\n\t"
		"mov %%edx, %1\n\t"
		: "=g" (eax), "=g" (edx)
		:
		: "eax", "edx"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreReg(ctx, x86_inst_reg_edx, edx);
	X86ContextStoreReg(ctx, x86_inst_reg_eax, eax);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, x86_dep_eax, x86_dep_edx, 0, 0);
}


void x86_isa_ret_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	if (ctx->inst.segment)
	{
		X86ContextError(ctx, "%s: not supported segment", __FUNCTION__);
		return;
	}

	X86ContextMemRead(ctx, regs->esp, 4, &ctx->target_eip);
	regs->esp += 4;
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_aux, 0, 0, 0);  /* pop aux */
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);  /* add esp, 4 */
	x86_uinst_new(ctx, x86_uinst_ret, x86_dep_aux, 0, 0, 0, 0, 0, 0);  /* jmp aux */
}


void x86_isa_repz_ret_impl(X86Context *ctx)
{
	x86_isa_ret_impl(ctx);
}


void x86_isa_ret_imm16_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	unsigned short pop;

	if (ctx->inst.segment)
	{
		X86ContextError(ctx, "%s: not supported segment", __FUNCTION__);
		return;
	}

	X86ContextMemRead(ctx, regs->esp, 4, &ctx->target_eip);
	pop = ctx->inst.imm.w;
	regs->esp += 4 + pop;
	regs->eip = ctx->target_eip;

	x86_uinst_new(ctx, x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(ctx, x86_uinst_load, regs->esp - 4 - pop, 4, x86_dep_aux, 0, 0, x86_dep_aux, 0, 0, 0);  /* pop aux */
	x86_uinst_new(ctx, x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);  /* add esp, 4 */
	x86_uinst_new(ctx, x86_uinst_ret, x86_dep_aux, 0, 0, 0, 0, 0, 0);  /* jmp aux */
}


void x86_isa_sahf_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	regs->eflags &= ~0xff;
	regs->eflags |= X86ContextLoadReg(ctx, x86_inst_reg_ah);
	regs->eflags &= ~0x28;
	regs->eflags |= 0x2;

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_eax, 0, 0, 0, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_sfence_impl(X86Context *ctx)
{
}


void x86_isa_shld_rm16_r16_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short rm16 = X86ContextLoadRm16(ctx);
	unsigned short r16 = X86ContextLoadR16(ctx);
	unsigned char imm8 = ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%bx\n\t"
		"mov %2, %%ax\n\t"
		"shld %%cl, %%bx, %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm16)
		: "m" (rm16), "m" (r16), "m" (imm8), "g" (flags)
		: "ax", "bx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm16(ctx, rm16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm16, x86_dep_r16, 0, x86_dep_rm16, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_shld_rm16_r16_cl_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned short rm16 = X86ContextLoadRm16(ctx);
	unsigned short r16 = X86ContextLoadR16(ctx);
	unsigned char cl = X86ContextLoadReg(ctx, x86_inst_reg_cl);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%bx\n\t"
		"mov %2, %%ax\n\t"
		"shld %%cl, %%bx, %%ax\n\t"
		"mov %%ax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm16)
		: "m" (rm16), "m" (r16), "m" (cl), "g" (flags)
		: "ax", "bx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm16(ctx, rm16);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm16, x86_dep_r16, x86_dep_ecx,
		x86_dep_rm16, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_shld_rm32_r32_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned char imm8 = ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shld %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (imm8), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_shld_rm32_r32_cl_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned char cl = X86ContextLoadReg(ctx, x86_inst_reg_cl);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shld %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (cl), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, x86_dep_r32, x86_dep_ecx,
		x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_shrd_rm32_r32_imm8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned char imm8 = ctx->inst.imm.b;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shrd %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (imm8), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_shrd_rm32_r32_cl_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned char cl = X86ContextLoadReg(ctx, x86_inst_reg_cl);
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %4, %%cl\n\t"
		"mov %3, %%ebx\n\t"
		"mov %2, %%eax\n\t"
		"shrd %%cl, %%ebx, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rm32)
		: "m" (rm32), "m" (r32), "m" (cl), "g" (flags)
		: "eax", "ebx", "cl"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreRm32(ctx, rm32);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_shift, x86_dep_rm32, x86_dep_r32, x86_dep_ecx,
		x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void x86_isa_std_impl(X86Context *ctx)
{
	X86ContextSetFlag(ctx, x86_inst_flag_df);

	x86_uinst_new(ctx, x86_uinst_move, 0, 0, 0, 0, x86_dep_df, 0, 0);
}


void x86_isa_xadd_rm8_r8_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned char rm8 = X86ContextLoadRm8(ctx);
	unsigned char r8 = X86ContextLoadR8(ctx);
	unsigned char sum;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"add %3, %%al\n\t"
		"mov %%al, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (sum)
		: "m" (rm8), "m" (r8), "g" (flags)
		: "al"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR8(ctx, rm8);
	X86ContextStoreRm8(ctx, sum);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_rm8, x86_dep_r8, 0, x86_dep_aux, x86_dep_zps, x86_dep_cf, x86_dep_of);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_rm8, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_r8, 0, 0, 0);
}


void x86_isa_xadd_rm32_r32_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int rm32 = X86ContextLoadRm32(ctx);
	unsigned int r32 = X86ContextLoadR32(ctx);
	unsigned int sum;
	unsigned long flags = regs->eflags;

	__X86_ISA_ASM_START__
	asm volatile (
		"push %4\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %3, %%eax\n\t"
		"mov %%eax, %1\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (sum)
		: "m" (rm32), "m" (r32), "g" (flags)
		: "eax"
	);
	__X86_ISA_ASM_END__

	X86ContextStoreR32(ctx, rm32);
	X86ContextStoreRm32(ctx, sum);
	regs->eflags = flags;

	x86_uinst_new(ctx, x86_uinst_add, x86_dep_rm32, x86_dep_r32, 0, x86_dep_aux, x86_dep_zps, x86_dep_cf, x86_dep_of);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(ctx, x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_r32, 0, 0, 0);
}


void x86_isa_xchg_ir16_ax_impl(X86Context *ctx)
{
	unsigned short ax, ir16;

	ax = X86ContextLoadReg(ctx, x86_inst_reg_ax);
	ir16 = X86ContextLoadIR16(ctx);
	X86ContextStoreReg(ctx, x86_inst_reg_ax, ir16);
	X86ContextStoreIR16(ctx, ax);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_ir16, x86_dep_eax, 0, x86_dep_ir16, x86_dep_eax, 0, 0);
}


void x86_isa_xchg_ir32_eax_impl(X86Context *ctx)
{
	unsigned int eax, ir32;

	eax = X86ContextLoadReg(ctx, x86_inst_reg_eax);
	ir32 = X86ContextLoadIR32(ctx);
	X86ContextStoreReg(ctx, x86_inst_reg_eax, ir32);
	X86ContextStoreIR32(ctx, eax);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_ir32, x86_dep_eax, 0, x86_dep_ir32, x86_dep_eax, 0, 0);
}


void x86_isa_xchg_rm8_r8_impl(X86Context *ctx)
{
	unsigned char rm8;
	unsigned char r8;

	rm8 = X86ContextLoadRm8(ctx);
	r8 = X86ContextLoadR8(ctx);
	X86ContextStoreRm8(ctx, r8);
	X86ContextStoreR8(ctx, rm8);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm8, x86_dep_r8, 0, x86_dep_rm8, x86_dep_r8, 0, 0);
}


void x86_isa_xchg_rm16_r16_impl(X86Context *ctx)
{
	unsigned short rm16;
	unsigned short r16;

	rm16 = X86ContextLoadRm16(ctx);
	r16 = X86ContextLoadR16(ctx);
	X86ContextStoreRm16(ctx, r16);
	X86ContextStoreR16(ctx, rm16);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm16, x86_dep_r16, 0, x86_dep_rm16, x86_dep_r16, 0, 0);
}


void x86_isa_xchg_rm32_r32_impl(X86Context *ctx)
{
	unsigned int rm32;
	unsigned int r32;

	rm32 = X86ContextLoadRm32(ctx);
	r32 = X86ContextLoadR32(ctx);
	X86ContextStoreRm32(ctx, r32);
	X86ContextStoreR32(ctx, rm32);

	x86_uinst_new(ctx, x86_uinst_move, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_r32, 0, 0);
}

