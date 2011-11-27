/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@ece.neu.edu)
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


void op_bound_r16_rm32_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_bound_r32_rm64_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_bsf_r32_rm32_impl()
{
	uint32_t r32 = isa_load_r32();
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(r32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_r32, x86_dep_zps, 0, 0);
}


void op_bsr_r32_rm32_impl()
{
	uint32_t r32 = isa_load_r32();
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(r32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_r32, x86_dep_zps, 0, 0);
}


void op_bswap_ir32_impl()
{
	uint32_t ir32 = isa_load_ir32();

	__ISA_ASM_START__
	asm volatile (
		"mov %1, %%eax\n\t"
		"bswap %%eax\n\t"
		"mov %%eax, %0\n\t"
		: "=g" (ir32)
		: "g" (ir32)
		: "eax"
	);
	__ISA_ASM_END__

	isa_store_ir32(ir32);

	x86_uinst_new(x86_uinst_shift, x86_dep_ir32, 0, 0, x86_dep_ir32, 0, 0, 0);
}


void op_bt_rm32_r32_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_cf, 0, 0, 0);
}


void op_bt_rm32_imm8_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t imm8 = isa_inst.imm.b;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_cf, 0, 0, 0);
}


void op_bts_rm32_imm8_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t imm8 = isa_inst.imm.b;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_cf, 0, 0);
}


void op_call_rel32_impl()
{
	isa_regs->esp -= 4;
	isa_mem_write(isa_mem, isa_regs->esp, 4, &isa_regs->eip);
	isa_target = isa_regs->eip + isa_inst.imm.d;
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_call, 0, 0, 0, 0, 0, 0, 0);
}


void op_call_rm32_impl() {
	isa_target = isa_load_rm32();
	isa_regs->esp -= 4;
	isa_mem_write(isa_mem, isa_regs->esp, 4, &isa_regs->eip);
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_call, x86_dep_rm32, 0, 0, 0, 0, 0, 0);
}


void op_cbw_impl()
{
	uint16_t ax = (int8_t) isa_load_reg(x86_reg_al);
	isa_store_reg(x86_reg_ax, ax);

	x86_uinst_new(x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_eax, 0, 0, 0);
}


void op_cdq_impl()
{
	int32_t signed_eax = isa_regs->eax;
	isa_regs->edx = signed_eax < 0 ? (int32_t) -1 : 0;

	x86_uinst_new(x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_edx, 0, 0, 0);
}


void op_cld_impl()
{
	isa_clear_flag(x86_flag_df);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, 0, x86_dep_df, 0, 0);
}


void op_cmpxchg_rm32_r32_impl()
{
	uint32_t eax = isa_regs->eax;
	unsigned long flags = isa_regs->eflags;
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_regs->eflags = flags;
	isa_store_reg(x86_reg_eax, eax);
	isa_store_rm32(rm32);

	x86_uinst_new(x86_uinst_sub, x86_dep_eax, x86_dep_rm32, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
	x86_uinst_new(x86_uinst_move, x86_dep_zps, x86_dep_r32, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, x86_dep_cf, x86_dep_rm32, 0, x86_dep_eax, 0, 0, 0);
}

void op_cmpxchg8b_m64_impl()
{
	uint32_t eax, ebx, ecx, edx;
	uint64_t edx_eax, m64;

	eax = isa_regs->eax;
	ebx = isa_regs->ebx;
	ecx = isa_regs->ecx;
	edx = isa_regs->edx;
	edx_eax = ((uint64_t) edx << 32) | eax;
	m64 = isa_load_m64();

	if (edx_eax == m64) {
		isa_set_flag(x86_flag_zf);
		m64 = ((uint64_t) ecx << 32) | ebx;
		isa_store_m64(m64);
	} else {
		isa_clear_flag(x86_flag_zf);
		isa_regs->edx = m64 >> 32;
		isa_regs->eax = m64;
	}

	x86_uinst_new_mem(x86_uinst_load, isa_effective_address(), 8, 0, 0, 0,
		x86_dep_aux, 0, 0, 0);  /* Load m64 */
	x86_uinst_new(x86_uinst_sub, x86_dep_edx, x86_dep_eax, x86_dep_aux,
		x86_dep_zps, 0, 0, 0);  /* Compare edx-eax with m64 */
	x86_uinst_new_mem(x86_uinst_store, isa_effective_address(), 8, x86_dep_zps, x86_dep_ecx, x86_dep_ebx,
		x86_dep_mem64, 0, 0, 0);  /* Conditionally store m64 */
	x86_uinst_new(x86_uinst_move, x86_dep_zps, 0, 0,
		x86_dep_edx, x86_dep_eax, 0, 0);  /* Conditionaly store edx-eax */
}


void op_cpuid_impl()
{
	uint32_t info = 0;

	switch (isa_regs->eax) {

	case 0x0:

		isa_store_reg(x86_reg_eax, 0x2);
		isa_store_reg(x86_reg_ebx, 0x756e6547);
		isa_store_reg(x86_reg_ecx, 0x6c65746e);
		isa_store_reg(x86_reg_edx, 0x49656e69);
		break;

	case 0x1:

		isa_store_reg(x86_reg_eax, 0x00000f29);
		isa_store_reg(x86_reg_ebx, 0x0102080b);
		isa_store_reg(x86_reg_ecx, 0x00004400);

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

		isa_store_reg(x86_reg_edx, info);
		break;

	case 0x2:

		isa_store_reg(x86_reg_eax, 0);
		isa_store_reg(x86_reg_ebx, 0);
		isa_store_reg(x86_reg_ecx, 0);
		isa_store_reg(x86_reg_edx, 0);
		break;

	case 0x80000000:

		isa_store_reg(x86_reg_eax, 0x80000004);
		isa_store_reg(x86_reg_ebx, 0);
		isa_store_reg(x86_reg_ecx, 0);
		isa_store_reg(x86_reg_edx, 0);
		break;

	case 0x80000001:

		isa_store_reg(x86_reg_eax, 0);
		isa_store_reg(x86_reg_ebx, 0);
		isa_store_reg(x86_reg_ecx, 0);
		isa_store_reg(x86_reg_edx, 0);
		break;

	case 0x80000002:

		isa_store_reg(x86_reg_eax, 0x20202020);
		isa_store_reg(x86_reg_ebx, 0x20202020);
		isa_store_reg(x86_reg_ecx, 0x20202020);
		isa_store_reg(x86_reg_edx, 0x20202020);
		break;

	case 0x80000003:

		isa_store_reg(x86_reg_eax, 0x6e492020);
		isa_store_reg(x86_reg_ebx, 0x286c6574);
		isa_store_reg(x86_reg_ecx, 0x58202952);
		isa_store_reg(x86_reg_edx, 0x286e6f65);
		break;

	case 0x80000004:

		isa_store_reg(x86_reg_eax, 0x20294d54);
		isa_store_reg(x86_reg_ebx, 0x20555043);
		isa_store_reg(x86_reg_ecx, 0x30382e32);
		isa_store_reg(x86_reg_edx, 0x7a4847);
		break;

	default:

		isa_error("inst 'cpuid' not implemented for eax=0x%x", isa_regs->eax);
	}

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_eax, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_ebx, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_ecx, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_edx, 0, 0, 0);
}


void op_cwde_impl()
{
	uint32_t eax = (int16_t) isa_load_reg(x86_reg_ax);
	isa_store_reg(x86_reg_eax, eax);

	x86_uinst_new(x86_uinst_sign, x86_dep_eax, 0, 0, x86_dep_eax, 0, 0, 0);
}


void op_dec_rm8_impl()
{
	uint8_t rm8 = isa_load_rm8();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm8(rm8);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_rm8, 0, 0, x86_dep_rm8, x86_dep_zps, x86_dep_of, 0);
}


void op_dec_rm32_impl()
{
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_zps, x86_dep_of, 0);
}


void op_dec_ir16_impl()
{
	uint16_t ir16 = isa_load_ir16();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_ir16(ir16);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_ir16, 0, 0, x86_dep_ir16, x86_dep_zps, x86_dep_of, 0);
}


void op_dec_ir32_impl()
{
	uint32_t ir32 = isa_load_ir32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_ir32(ir32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_ir32, 0, 0, x86_dep_ir32, x86_dep_zps, x86_dep_of, 0);
}


void op_div_rm8_impl()
{
	int skip_emulation = 0;

	uint16_t ax = isa_load_reg(x86_reg_ax);
	uint8_t rm8 = isa_load_rm8();

	if (!rm8) {
		isa_error("%s: division by 0", __FUNCTION__);
		return;
	}

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xff. */
	if (isa_spec_mode && ax > 0xff)
		skip_emulation = 1;

	/* Emulate */
	if (!skip_emulation)
	{
		__ISA_ASM_START__
		asm volatile (
			"mov %1, %%ax\n\t"
			"mov %2, %%bl\n\t"
			"div %%bl\n\t"
			"mov %%ax, %0\n\t"
			: "=m" (ax)
			: "m" (ax), "m" (rm8)
			: "ax", "bl"
		);
		__ISA_ASM_END__
	}

	isa_store_reg(x86_reg_ax, ax);

	x86_uinst_new(x86_uinst_div, x86_dep_eax, x86_dep_rm8, 0, x86_dep_eax, 0, 0, 0);
}


void op_div_rm32_impl()
{
	int skip_emulation = 0;

	uint32_t eax = isa_regs->eax;
	uint32_t edx = isa_regs->edx;
	uint32_t rm32 = isa_load_rm32();

	if (!rm32) {
		isa_error("%s: division by 0", __FUNCTION__);
		return;
	}

	/* A devide exception would occur in the host process if the 'div' instruction
	 * in the assembly code below generates a result greater than 0xffffffff. */
	if (isa_spec_mode && edx)
		skip_emulation = 1;

	/* Emulate */
	if (!skip_emulation)
	{
		__ISA_ASM_START__
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
		__ISA_ASM_END__
	}

	isa_store_reg(x86_reg_eax, eax);
	isa_store_reg(x86_reg_edx, edx);

	x86_uinst_new(x86_uinst_div, x86_dep_edx, x86_dep_eax, x86_dep_rm32, x86_dep_eax, x86_dep_edx, 0, 0);
}


void op_hlt_impl()
{
	isa_error("%s: 'hlt' instruction", __FUNCTION__);
}


void op_idiv_rm32_impl()
{
	int skip_emulation = 0;

	uint32_t eax = isa_regs->eax;
	uint32_t edx = isa_regs->edx;
	uint32_t rm32 = isa_load_rm32();

	if (!rm32) {
		isa_error("%s: division by 0", __FUNCTION__);
		return;
	}

	/* Avoid emulation in speculative mode if it could cause a divide exception */
	if (isa_spec_mode) {
		int64_t edx_eax = ((uint64_t) edx << 32) | eax;
		if (edx_eax > 0x7fffffffll || edx_eax < 0xffffffff80000000ll)
			skip_emulation = 1;
	}

	if (!skip_emulation)
	{
		__ISA_ASM_START__
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
		__ISA_ASM_END__
	}

	isa_store_reg(x86_reg_eax, eax);
	isa_store_reg(x86_reg_edx, edx);

	x86_uinst_new(x86_uinst_div, x86_dep_rm32, x86_dep_eax, 0, x86_dep_eax, x86_dep_edx, 0, 0);
}


void op_imul_rm32_impl()
{
	uint32_t eax = isa_load_reg(x86_reg_eax);
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;
	uint32_t edx;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_reg(x86_reg_eax, eax);
	isa_store_reg(x86_reg_edx, edx);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_mult, x86_dep_rm32, x86_dep_eax, 0, x86_dep_eax, x86_dep_edx, x86_dep_cf, x86_dep_of);
}


void op_imul_r32_rm32_impl()
{
	uint32_t r32 = isa_load_r32();
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(r32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_mult, x86_dep_r32, x86_dep_rm32, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void op_imul_r32_rm32_imm8_impl()
{
	uint32_t r32;
	uint32_t rm32 = isa_load_rm32();
	uint32_t imm8 = (int8_t) isa_inst.imm.b;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(r32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_mult, x86_dep_rm32, 0, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void op_imul_r32_rm32_imm32_impl()
{
	uint32_t r32;
	uint32_t rm32 = isa_load_rm32();
	uint32_t imm32 = isa_inst.imm.d;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(r32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_mult, x86_dep_rm32, 0, 0, x86_dep_r32, 0, x86_dep_cf, x86_dep_of);
}


void op_inc_rm8_impl()
{
	uint8_t rm8 = isa_load_rm8();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm8(rm8);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_rm8, 0, 0, x86_dep_rm8, 0, x86_dep_zps, x86_dep_of);
}


void op_inc_rm32_impl()
{
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_rm32, 0, 0, x86_dep_rm32, 0, x86_dep_zps, x86_dep_of);
}


void op_inc_ir16_impl()
{
	uint16_t ir16 = isa_load_ir16();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_ir16(ir16);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_ir16, 0, 0, x86_dep_ir16, 0, x86_dep_zps, x86_dep_of);
}


void op_inc_ir32_impl()
{
	uint32_t ir32 = isa_load_ir32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_ir32(ir32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_ir32, 0, 0, x86_dep_ir32, 0, x86_dep_zps, x86_dep_of);
}


void op_int_3_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_int_imm8_impl()
{
	uint32_t num;

	/* Interrupt code */
	num = (uint8_t) isa_inst.imm.b;
	if (num != 0x80)
		isa_error("%s: not supported for num != 0x80", __FUNCTION__);

	/* Do system call if not in speculative mode */
	if (!isa_spec_mode)
		syscall_do();

	x86_uinst_new(x86_uinst_syscall, 0, 0, 0, 0, 0, 0, 0);
}


void op_into_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}



void op_jmp_rel8_impl()
{
	isa_target = isa_regs->eip + (int8_t) isa_inst.imm.b;
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_jump, 0, 0, 0, 0, 0, 0, 0);
}


void op_jmp_rel32_impl()
{
	isa_target = isa_regs->eip + isa_inst.imm.d;
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_jump, 0, 0, 0, 0, 0, 0, 0);
}


void op_jmp_rm32_impl()
{
	isa_target = isa_load_rm32();
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_jump, x86_dep_rm32, 0, 0, 0, 0, 0, 0);
}


void op_lea_r32_m_impl()
{
	uint32_t value = isa_effective_address();

	if (isa_inst.segment) {
		isa_error("%s: not supported for this segment", __FUNCTION__);
		return;
	}

	isa_store_r32(value);

	x86_uinst_new(x86_uinst_effaddr, x86_dep_easeg, x86_dep_eabas, x86_dep_eaidx, x86_dep_r32, 0, 0, 0);
}


void op_leave_impl()
{
	uint32_t value;
	isa_regs->esp = isa_regs->ebp;

	if (isa_inst.segment) {
		isa_error("%s: not supported segment", __FUNCTION__);
		return;
	}

	isa_mem_read(isa_mem, isa_regs->esp, 4, &value);
	isa_regs->esp += 4;
	isa_store_reg(x86_reg_ebp, value);

	x86_uinst_new(x86_uinst_move, x86_dep_ebp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_ebp, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void op_lfence_impl()
{
}


void op_lock_xadd_rm8_r8_impl()
{
	op_xadd_rm8_r8_impl();
}


void op_lock_xadd_rm32_r32_impl()
{
	op_xadd_rm32_r32_impl();
}


void op_mov_rm8_imm8_impl()
{
	uint8_t value = isa_inst.imm.b;
	isa_store_rm8(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void op_mov_r8_rm8_impl()
{
	uint8_t value = isa_load_rm8();
	isa_store_r8(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r8, 0, 0, 0);
}


void op_mov_rm8_r8_impl()
{
	uint8_t value = isa_load_r8();
	isa_store_rm8(value);

	x86_uinst_new(x86_uinst_move, x86_dep_r8, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void op_mov_rm16_r16_impl()
{
	uint16_t value = isa_load_r16();
	isa_store_rm16(value);

	x86_uinst_new(x86_uinst_move, x86_dep_r16, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void op_mov_rm32_r32_impl()
{
	uint32_t value = isa_load_r32();
	isa_store_rm32(value);

	x86_uinst_new(x86_uinst_move, x86_dep_r32, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void op_mov_r16_rm16_impl()
{
	uint16_t value = isa_load_rm16();
	isa_store_r16(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_r16, 0, 0, 0);
}


void op_mov_r32_rm32_impl()
{
	uint32_t value = isa_load_rm32();
	isa_store_r32(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm32, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_mov_al_moffs8_impl()
{
	uint8_t value;

	isa_mem_read(isa_mem, isa_moffs_address(), 1, &value);
	isa_store_reg(x86_reg_al, value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_moffs_address(), 1, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void op_mov_ax_moffs16_impl()
{
	uint16_t value;

	isa_mem_read(isa_mem, isa_moffs_address(), 2, &value);
	isa_store_reg(x86_reg_ax, value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_moffs_address(), 2, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void op_mov_eax_moffs32_impl()
{
	uint32_t value;

	isa_mem_read(isa_mem, isa_moffs_address(), 4, &value);
	isa_store_reg(x86_reg_eax, value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_moffs_address(), 4, x86_dep_aux, 0, 0, x86_dep_eax, 0, 0, 0);
}


void op_mov_moffs8_al_impl()
{
	uint8_t value = isa_load_reg(x86_reg_al);
	isa_mem_write(isa_mem, isa_moffs_address(), 1, &value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_moffs_address(), 1, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void op_mov_moffs16_ax_impl()
{
	uint16_t value = isa_load_reg(x86_reg_ax);
	isa_mem_write(isa_mem, isa_moffs_address(), 2, &value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_moffs_address(), 2, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void op_mov_moffs32_eax_impl()
{
	uint32_t value = isa_load_reg(x86_reg_eax);
	isa_mem_write(isa_mem, isa_moffs_address(), 4, &value);

	x86_uinst_new(x86_uinst_effaddr, 0, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_moffs_address(), 4, x86_dep_aux, x86_dep_eax, 0, 0, 0, 0, 0);
}


void op_mov_ir8_imm8_impl()
{
	uint8_t value = isa_inst.imm.b;
	isa_store_ir8(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_ir8, 0, 0, 0);
}


void op_mov_ir16_imm16_impl()
{
	uint16_t value = isa_inst.imm.w;
	isa_store_ir16(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_ir16, 0, 0, 0);
}


void op_mov_ir32_imm32_impl()
{
	uint32_t value = isa_inst.imm.d;
	isa_store_ir32(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_ir32, 0, 0, 0);
}


void op_mov_rm16_imm16_impl()
{
	uint16_t value = isa_inst.imm.w;
	isa_store_rm16(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void op_mov_rm32_imm32_impl()
{
	uint32_t value = isa_inst.imm.d;
	isa_store_rm32(value);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void op_mov_rm16_sreg_impl()
{
	uint16_t value = isa_load_sreg();
	if (isa_inst.reg != 5)
		isa_error("%s: not supported for sreg != gs", __FUNCTION__);
	isa_store_rm16(value);

	x86_uinst_new(x86_uinst_move, x86_dep_sreg, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void op_mov_rm32_sreg_impl()
{
	op_mov_rm16_sreg_impl();
}


void op_mov_sreg_rm16_impl()
{
	uint16_t value = isa_load_rm16();
	if (isa_inst.reg != 5)
		isa_error("%s: not supported for sreg != gs", __FUNCTION__);
	isa_store_sreg(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_sreg, 0, 0, 0);
}


void op_mov_sreg_rm32_impl()
{
	op_mov_sreg_rm16_impl();
}


void op_movsx_r16_rm8_impl()
{
	uint16_t value = (int8_t) isa_load_rm8();
	isa_store_r16(value);

	x86_uinst_new(x86_uinst_sign, x86_dep_rm8, 0, 0, x86_dep_r16, 0, 0, 0);
}


void op_movsx_r32_rm8_impl()
{
	uint32_t value = (int8_t) isa_load_rm8();
	isa_store_r32(value);

	x86_uinst_new(x86_uinst_sign, x86_dep_rm8, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_movsx_r32_rm16_impl()
{
	uint32_t value = (int16_t) isa_load_rm16();
	isa_store_r32(value);

	x86_uinst_new(x86_uinst_sign, x86_dep_rm16, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_movzx_r16_rm8_impl()
{
	uint8_t value = isa_load_rm8();
	isa_store_r16(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r16, 0, 0, 0);
}


void op_movzx_r32_rm8_impl()
{
	uint8_t value = isa_load_rm8();
	isa_store_r32(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm8, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_movzx_r32_rm16_impl()
{
	uint16_t value = isa_load_rm16();
	isa_store_r32(value);

	x86_uinst_new(x86_uinst_move, x86_dep_rm16, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_mul_rm32_impl()
{
	uint32_t eax = isa_load_reg(x86_reg_eax);
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;
	uint32_t edx;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_reg(x86_reg_eax, eax);
	isa_store_reg(x86_reg_edx, edx);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_mult, x86_dep_rm32, x86_dep_eax, 0, x86_dep_edx, x86_dep_eax, x86_dep_of, x86_dep_cf);
}


void op_neg_rm8_impl()
{
	uint8_t rm8 = isa_load_rm8();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm8(rm8);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_rm8, 0, 0, x86_dep_rm8, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_neg_rm32_impl()
{
	uint32_t rm32 = isa_load_rm32();
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_sub, x86_dep_rm32, 0, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_nop_impl() {
}


void op_nop_rm16_impl() {
}


void op_nop_rm32_impl() {
}


void op_not_rm8_impl()
{
	uint8_t value = isa_load_rm8();
	value = ~value;
	isa_store_rm8(value);

	x86_uinst_new(x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm8, 0, 0, 0);
}


void op_not_rm16_impl()
{
	uint16_t value = isa_load_rm16();
	value = ~value;
	isa_store_rm16(value);

	x86_uinst_new(x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm16, 0, 0, 0);
}


void op_not_rm32_impl()
{
	uint32_t value = isa_load_rm32();
	value = ~value;
	isa_store_rm32(value);

	x86_uinst_new(x86_uinst_not, x86_dep_rm8, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void op_out_imm8_al_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_out_imm8_ax_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_out_imm8_eax_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_out_dx_al_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_out_dx_ax_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_out_dx_eax_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pause_impl()
{
}


void op_pop_rm32_impl()
{
	uint32_t value;

	isa_mem_read(isa_mem, isa_regs->esp, 4, &value);
	isa_regs->esp += 4;
	isa_store_rm32(value);

	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void op_pop_ir32_impl()
{
	uint32_t value;

	if (isa_inst.segment) {
		isa_error("%s: not supported segment", __FUNCTION__);
		return;
	}

	isa_mem_read(isa_mem, isa_regs->esp, 4, &value);
	isa_regs->esp += 4;
	isa_store_ir32(value);

	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_ir32, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void op_popf_impl()
{
	isa_mem_read(isa_mem, isa_regs->esp, 4, &isa_regs->eflags);
	isa_regs->esp += 4;

	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_zps, x86_dep_cf, x86_dep_of, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
}


void op_prefetcht0_impl() {
}


void op_prefetcht1_impl() {
}


void op_prefetcht2_impl() {
}


void op_prefetchnta_impl() {
}


void op_push_imm8_impl()
{
	uint32_t value = (int8_t) isa_inst.imm.b;
	isa_store_reg(x86_reg_esp, isa_regs->esp - 4);
	isa_mem_write(isa_mem, isa_regs->esp, 4, &value);

	x86_uinst_new(x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
}


void op_push_imm32_impl()
{
	uint32_t value = isa_inst.imm.d;
	isa_store_reg(x86_reg_esp, isa_regs->esp - 4);
	isa_mem_write(isa_mem, isa_regs->esp, 4, &value);

	x86_uinst_new(x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esp, 4, x86_dep_aux, 0, 0, 0, 0, 0, 0);
}


void op_push_rm32_impl()
{
	uint32_t value = isa_load_rm32();
	isa_store_reg(x86_reg_esp, isa_regs->esp - 4);
	isa_mem_write(isa_mem, isa_regs->esp, 4, &value);

	x86_uinst_new(x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esp, 4, x86_dep_aux, x86_dep_rm32, 0, 0, 0, 0, 0);
}


void op_push_ir32_impl()
{
	uint32_t value = isa_load_ir32();
	isa_store_reg(x86_reg_esp, isa_regs->esp - 4);
	isa_mem_write(isa_mem, isa_regs->esp, 4, &value);

	x86_uinst_new(x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esp, 4, x86_dep_aux, x86_dep_ir32, 0, 0, 0, 0, 0);
}


void op_pushf_impl()
{
	isa_store_reg(x86_reg_esp, isa_regs->esp - 4);
	isa_mem_write(isa_mem, isa_regs->esp, 4, &isa_regs->eflags);

	x86_uinst_new(x86_uinst_sub, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);
	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, x86_dep_zps, x86_dep_cf, x86_dep_of, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esp, 4, x86_dep_aux, x86_dep_aux2, 0, 0, 0, 0, 0);
}


void op_rdtsc_impl()
{
	uint32_t eax, edx;

	__ISA_ASM_START__
	asm volatile (
		"rdtsc\n\t"
		"mov %%eax, %0\n\t"
		"mov %%edx, %1\n\t"
		: "=g" (eax), "=g" (edx)
		:
		: "eax", "edx"
	);
	__ISA_ASM_END__

	isa_store_reg(x86_reg_edx, edx);
	isa_store_reg(x86_reg_eax, eax);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, x86_dep_eax, x86_dep_edx, 0, 0);
}


void op_ret_impl()
{
	if (isa_inst.segment) {
		isa_error("%s: not supported segment", __FUNCTION__);
		return;
	}

	isa_mem_read(isa_mem, isa_regs->esp, 4, &isa_target);
	isa_regs->esp += 4;
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4, 4, x86_dep_aux, 0, 0, x86_dep_aux, 0, 0, 0);  /* pop aux */
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);  /* add esp, 4 */
	x86_uinst_new(x86_uinst_ret, x86_dep_aux, 0, 0, 0, 0, 0, 0);  /* jmp aux */
}


void op_repz_ret_impl()
{
	op_ret_impl();
}


void op_ret_imm16_impl()
{
	uint16_t pop;

	if (isa_inst.segment) {
		isa_error("%s: not supported segment", __FUNCTION__);
		return;
	}

	isa_mem_read(isa_mem, isa_regs->esp, 4, &isa_target);
	pop = isa_inst.imm.w;
	isa_regs->esp += 4 + pop;
	isa_regs->eip = isa_target;

	x86_uinst_new(x86_uinst_effaddr, x86_dep_esp, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->esp - 4 - pop, 4, x86_dep_aux, 0, 0, x86_dep_aux, 0, 0, 0);  /* pop aux */
	x86_uinst_new(x86_uinst_add, x86_dep_esp, 0, 0, x86_dep_esp, 0, 0, 0);  /* add esp, 4 */
	x86_uinst_new(x86_uinst_ret, x86_dep_aux, 0, 0, 0, 0, 0, 0);  /* jmp aux */
}


void op_sahf_impl()
{
	isa_regs->eflags &= ~0xff;
	isa_regs->eflags |= isa_load_reg(x86_reg_ah);
	isa_regs->eflags &= ~0x28;
	isa_regs->eflags |= 0x2;

	x86_uinst_new(x86_uinst_move, x86_dep_eax, 0, 0, 0, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_sfence_impl()
{
}


void op_shld_rm32_r32_imm8_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	uint8_t imm8 = isa_inst.imm.b;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_shld_rm32_r32_cl_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	uint8_t cl = isa_load_reg(x86_reg_cl);
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, x86_dep_r32, x86_dep_ecx,
		x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_shrd_rm32_r32_imm8_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	uint8_t imm8 = isa_inst.imm.b;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_shrd_rm32_r32_cl_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	uint8_t cl = isa_load_reg(x86_reg_cl);
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_rm32(rm32);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_shift, x86_dep_rm32, x86_dep_r32, x86_dep_ecx,
		x86_dep_rm32, x86_dep_zps, x86_dep_cf, x86_dep_of);
}


void op_std_impl()
{
	isa_set_flag(x86_flag_df);

	x86_uinst_new(x86_uinst_move, 0, 0, 0, 0, x86_dep_df, 0, 0);
}


void op_xadd_rm8_r8_impl()
{
	uint8_t rm8 = isa_load_rm8();
	uint8_t r8 = isa_load_r8();
	uint8_t sum;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r8(rm8);
	isa_store_rm8(sum);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_rm8, x86_dep_r8, 0, x86_dep_aux, x86_dep_zps, x86_dep_cf, x86_dep_of);
	x86_uinst_new(x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_rm8, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_r8, 0, 0, 0);
}


void op_xadd_rm32_r32_impl()
{
	uint32_t rm32 = isa_load_rm32();
	uint32_t r32 = isa_load_r32();
	uint32_t sum;
	unsigned long flags = isa_regs->eflags;

	__ISA_ASM_START__
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
	__ISA_ASM_END__

	isa_store_r32(rm32);
	isa_store_rm32(sum);
	isa_regs->eflags = flags;

	x86_uinst_new(x86_uinst_add, x86_dep_rm32, x86_dep_r32, 0, x86_dep_aux, x86_dep_zps, x86_dep_cf, x86_dep_of);
	x86_uinst_new(x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_rm32, 0, 0, 0);
	x86_uinst_new(x86_uinst_move, x86_dep_aux, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_xchg_ir16_ax_impl()
{
	uint16_t ax, ir16;

	ax = isa_load_reg(x86_reg_ax);
	ir16 = isa_load_ir16();
	isa_store_reg(x86_reg_ax, ir16);
	isa_store_ir16(ax);

	x86_uinst_new(x86_uinst_move, x86_dep_ir16, x86_dep_eax, 0, x86_dep_ir16, x86_dep_eax, 0, 0);
}


void op_xchg_ir32_eax_impl()
{
	uint32_t eax, ir32;

	eax = isa_load_reg(x86_reg_eax);
	ir32 = isa_load_ir32();
	isa_store_reg(x86_reg_eax, ir32);
	isa_store_ir32(eax);

	x86_uinst_new(x86_uinst_move, x86_dep_ir32, x86_dep_eax, 0, x86_dep_ir32, x86_dep_eax, 0, 0);
}


void op_xchg_rm8_r8_impl()
{
	uint8_t rm8, r8;

	rm8 = isa_load_rm8();
	r8 = isa_load_r8();
	isa_store_rm8(r8);
	isa_store_r8(rm8);

	x86_uinst_new(x86_uinst_move, x86_dep_rm8, x86_dep_r8, 0, x86_dep_rm8, x86_dep_r8, 0, 0);
}


void op_xchg_rm16_r16_impl()
{
	uint16_t rm16, r16;

	rm16 = isa_load_rm16();
	r16 = isa_load_r16();
	isa_store_rm16(r16);
	isa_store_r16(rm16);

	x86_uinst_new(x86_uinst_move, x86_dep_rm16, x86_dep_r16, 0, x86_dep_rm16, x86_dep_r16, 0, 0);
}


void op_xchg_rm32_r32_impl()
{
	uint32_t rm32, r32;

	rm32 = isa_load_rm32();
	r32 = isa_load_r32();
	isa_store_rm32(r32);
	isa_store_r32(rm32);

	x86_uinst_new(x86_uinst_move, x86_dep_rm32, x86_dep_r32, 0, x86_dep_rm32, x86_dep_r32, 0, 0);
}

