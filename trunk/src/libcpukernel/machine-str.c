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


/* Observation:
 * Speculative values in 'isa_regs->ecx' can cause instructions with repetition
 * prefixes to run an overly high number of iterations. This is why these
 * instructions should be stopped on speculative execution. */


#define OP_REP_IMPL(X) \
	void op_rep_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx && !isa_spec_mode) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
		} \
	}


#define OP_REPZ_IMPL(X) \
	void op_repz_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx && !isa_spec_mode) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			if (!isa_get_flag(x86_flag_zf)) \
				break; \
		} \
	}


#define OP_REPNZ_IMPL(X) \
	void op_repnz_##X##_impl() { \
		isa_inst.rep = 0; \
		while (isa_regs->ecx && !isa_spec_mode) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			if (isa_get_flag(x86_flag_zf)) \
				break; \
		} \
	}




/* Implementation of string instructions with the
 * REP, REPZ or REPNZ preffix. */

void op_insb_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_insd_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_lodsb_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_lodsd_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_outsb_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_outsd_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}



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


void op_cmpsb_impl()
{
	uint8_t op1, op2;
	unsigned long flags;

	isa_mem_read(isa_mem, isa_regs->esi, 1, &op1);
	isa_mem_read(isa_mem, isa_regs->edi, 1, &op2);
	flags = isa_regs->eflags;

	__ISA_ASM_START__
	asm volatile (
		"push %1\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"cmp %3, %%al\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "al"
	);
	__ISA_ASM_END__

	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -1 : 1;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


void op_cmpsd_impl()
{
	uint32_t op1, op2;
	unsigned long flags;

	isa_mem_read(isa_mem, isa_regs->edi, 4, &op1);
	isa_mem_read(isa_mem, isa_regs->esi, 4, &op2);
	flags = isa_regs->eflags;

	__ISA_ASM_START__ \
	asm volatile (
		"push %1\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"cmp %3, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "eax"
	);
	__ISA_ASM_END__ \

	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -4 : 4;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
}


void op_movsb_impl()
{
	uint8_t m8;
	isa_mem_read(isa_mem, isa_regs->esi, 1, &m8);
	isa_mem_write(isa_mem, isa_regs->edi, 1, &m8);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


void op_movsw_impl()
{
	uint16_t m16;
	isa_mem_read(isa_mem, isa_regs->esi, 2, &m16);
	isa_mem_write(isa_mem, isa_regs->edi, 2, &m16);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -2 : 2;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -2 : 2;
}


void op_movsd_impl()
{
	uint32_t m32;
	isa_mem_read(isa_mem, isa_regs->esi, 4, &m32);
	isa_mem_write(isa_mem, isa_regs->edi, 4, &m32);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -4 : 4;
}


void op_scasb_impl()
{
	uint8_t al = isa_load_reg(x86_reg_al);
	uint8_t m8;
	unsigned long flags = isa_regs->eflags;

	isa_mem_read(isa_mem, isa_regs->edi, 1, &m8);

	__ISA_ASM_START__ \
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %1, %%al\n\t"
		"cmp %2, %%al\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (al), "m" (m8), "g" (flags)
		: "al"
	);
	__ISA_ASM_END__ \

	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


void op_scasd_impl()
{
	uint32_t eax = isa_load_reg(x86_reg_eax);
	uint32_t m32;
	unsigned long flags = isa_regs->eflags;

	isa_mem_read(isa_mem, isa_regs->edi, 4, &m32);

	__ISA_ASM_START__ \
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %1, %%eax\n\t"
		"cmp %2, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (eax), "m" (m32), "g" (flags)
		: "eax"
	);
	__ISA_ASM_END__ \

	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
}


void op_stosb_impl()
{
	uint8_t m8 = isa_load_reg(x86_reg_al);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	isa_mem_write(isa_mem, addr, 1, &m8);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


void op_stosd_impl()
{
	uint32_t m32 = isa_load_reg(x86_reg_eax);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	isa_mem_write(isa_mem, addr, 4, &m32);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
}

