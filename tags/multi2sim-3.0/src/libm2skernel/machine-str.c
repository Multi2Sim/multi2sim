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
#define DUMP printf("\n"), x86_inst_dump(&isa_inst, stdout), printf("\n")


#define OP_REP_IMPL(X) \
	void op_rep_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx) { \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
		} \
	}


#define OP_REPZ_IMPL(X) \
	void op_repz_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx) { \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			if (!isa_get_flag(flag_zf)) \
				break; \
		} \
	}


#define OP_REPNZ_IMPL(X) \
	void op_repnz_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx) { \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			if (isa_get_flag(flag_zf)) \
				break; \
		} \
	}




/* Implementation of string instructions with the
 * REP, REPZ or REPNZ preffix. */

void op_insb_impl() { panic("insb not implemented"); }
void op_insd_impl() { panic("insd not implemented"); }
void op_lodsb_impl() { panic("lodsb not implemented"); }
void op_lodsd_impl() { panic("lodsd not implemented"); }
void op_outsb_impl() { panic("outsb not implemented"); }
void op_outsd_impl() { panic("outsd not implemented"); }

OP_REP_IMPL(insb)
OP_REP_IMPL(insd)

OP_REP_IMPL(movsb)
OP_REP_IMPL(movsw)
OP_REP_IMPL(movsd)

OP_REP_IMPL(outsb)
OP_REP_IMPL(outsd)

OP_REP_IMPL(lodsb)
OP_REP_IMPL(lodsd)

OP_REP_IMPL(stosb)
OP_REP_IMPL(stosd)

OP_REPZ_IMPL(cmpsb)
OP_REPZ_IMPL(cmpsd)

OP_REPZ_IMPL(scasb)
OP_REPZ_IMPL(scasd)

OP_REPNZ_IMPL(cmpsb)
OP_REPNZ_IMPL(cmpsd)

OP_REPNZ_IMPL(scasb)
OP_REPNZ_IMPL(scasd)




/* String Instructions */


void op_cmpsb_impl() {
	uint8_t op1, op2;
	unsigned long flags;

	mem_read(isa_mem, isa_regs->esi, 1, &op1);
	mem_read(isa_mem, isa_regs->edi, 1, &op2);
	flags = isa_regs->eflags;
	asm volatile (
		"push %1 ; popf\n\t"
		"mov %2, %%al\n\t"
		"cmp %3, %%al\n\t"
		"pushf ; pop %0"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "al"
	);
	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(flag_df) ? -1 : 1;
	isa_regs->edi += isa_get_flag(flag_df) ? -1 : 1;
}


void op_cmpsd_impl() {
	uint32_t op1, op2;
	unsigned long flags;

	mem_read(isa_mem, isa_regs->edi, 4, &op1);
	mem_read(isa_mem, isa_regs->esi, 4, &op2);
	flags = isa_regs->eflags;
	asm volatile (
		"push %1 ; popf\n\t"
		"mov %2, %%eax\n\t"
		"cmp %3, %%eax\n\t"
		"pushf ; pop %0"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "eax"
	);
	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(flag_df) ? -4 : 4;
	isa_regs->edi += isa_get_flag(flag_df) ? -4 : 4;
}


void op_movsb_impl() {
	uint8_t m8;
	mem_read(isa_mem, isa_regs->esi, 1, &m8);
	mem_write(isa_mem, isa_regs->edi, 1, &m8);
	isa_regs->edi += isa_get_flag(flag_df) ? -1 : 1;
	isa_regs->esi += isa_get_flag(flag_df) ? -1 : 1;
}


void op_movsw_impl() {
	uint16_t m16;
	mem_read(isa_mem, isa_regs->esi, 2, &m16);
	mem_write(isa_mem, isa_regs->edi, 2, &m16);
	isa_regs->edi += isa_get_flag(flag_df) ? -2 : 2;
	isa_regs->esi += isa_get_flag(flag_df) ? -2 : 2;
}


void op_movsd_impl() {
	uint32_t m32;
	mem_read(isa_mem, isa_regs->esi, 4, &m32);
	mem_write(isa_mem, isa_regs->edi, 4, &m32);
	isa_regs->edi += isa_get_flag(flag_df) ? -4 : 4;
	isa_regs->esi += isa_get_flag(flag_df) ? -4 : 4;
}


void op_scasb_impl() {
	uint8_t al = isa_load_reg(reg_al);
	uint8_t m8;
	unsigned long flags = isa_regs->eflags;
	mem_read(isa_mem, isa_regs->edi, 1, &m8);
	asm volatile (
		"push %3 ; popf\n\t"
		"mov %1, %%al\n\t"
		"cmp %2, %%al\n\t"
		"pushf ; pop %0\n\t"
		: "=g" (flags)
		: "m" (al), "m" (m8), "g" (flags)
		: "al"
	);
	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(flag_df) ? -1 : 1;
}


void op_scasd_impl() {
	uint32_t eax = isa_load_reg(reg_eax);
	uint32_t m32;
	unsigned long flags = isa_regs->eflags;
	mem_read(isa_mem, isa_regs->edi, 4, &m32);
	asm volatile (
		"push %3 ; popf\n\t"
		"mov %1, %%eax\n\t"
		"cmp %2, %%eax\n\t"
		"pushf ; pop %0\n\t"
		: "=g" (flags)
		: "m" (eax), "m" (m32), "g" (flags)
		: "eax"
	);
	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(flag_df) ? -4 : 4;
}


void op_stosb_impl() {
	uint8_t m8 = isa_load_reg(reg_al);
	uint32_t addr = isa_load_reg(reg_edi);
	mem_write(isa_mem, addr, 1, &m8);
	isa_regs->edi += isa_get_flag(flag_df) ? -1 : 1;
}


void op_stosd_impl() {
	uint32_t m32 = isa_load_reg(reg_eax);
	uint32_t addr = isa_load_reg(reg_edi);
	mem_write(isa_mem, addr, 4, &m32);
	isa_regs->edi += isa_get_flag(flag_df) ? -4 : 4;
}



