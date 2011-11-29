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


/* For macros implementing string operations with repetition prefixes, loops are repeated
 * only once if there is speculative execution. There are two reasons for this:
 *   1) A speculative value in register 'isa_regs->ecx' is undefined and could be extremely
 *      large, causing the functional simulator to stall if the insruction is executed
 *      normally.
 *   2) Internal branch micro-instructions (x86_uinst_ibranch) generated here are interpreted
 *      by the architectural simulator as branches that jump to the same instruction. The
 *      BTB always returns 'uop->eip' as a branch target address. This will cause the
 *      string instruction to be fetched continuously until the first non-taken branch is
 *      resolved. For every time the string instruction is fetched in speculative mode,
 *      i.e., any time but the first one, only one group of micro-instructions will be
 *      inserted to the pipeline, corresponding to one iteration of the loop.
 */


#define OP_REP_IMPL(X) \
	void op_rep_##X##_impl() \
	{ \
		isa_inst.rep = 0; \
		if (isa_spec_mode) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, 0, 0, 0, 0, 0, 0); \
			return; \
		} \
		\
		while (isa_regs->ecx) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, 0, 0, 0, 0, 0, 0); \
		} \
	}


#define OP_REPZ_IMPL(X) \
	void op_repz_##X##_impl() \
	{ \
		isa_inst.rep = 0; \
		if (isa_spec_mode) { \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
			return; \
		} \
		\
		while (isa_regs->ecx) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
			if (!isa_get_flag(x86_flag_zf)) \
				break; \
		} \
	}


#define OP_REPNZ_IMPL(X) \
	void op_repnz_##X##_impl() \
	{ \
		isa_inst.rep = 0; \
		if (isa_spec_mode) { \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
			return; \
		} \
		\
		while (isa_regs->ecx) \
		{ \
			op_##X##_impl(); \
			isa_regs->ecx--; \
			isa_inst.rep++; \
			\
			x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
			x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
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




/*
 * String Instructions
 */


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

	x86_uinst_new_mem(x86_uinst_load, isa_regs->esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 1, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -1 : 1;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
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

	x86_uinst_new_mem(x86_uinst_load, isa_regs->esi, 4, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 4, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	isa_regs->eflags = flags;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -4 : 4;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_movsb_impl()
{
	uint8_t m8;

	isa_mem_read(isa_mem, isa_regs->esi, 1, &m8);
	isa_mem_write(isa_mem, isa_regs->edi, 1, &m8);

	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 1, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esi, 1, x86_dep_esi, x86_dep_aux, 0, 0, 0, 0, 0);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -1 : 1;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void op_movsw_impl()
{
	uint16_t m16;

	isa_mem_read(isa_mem, isa_regs->esi, 2, &m16);
	isa_mem_write(isa_mem, isa_regs->edi, 2, &m16);

	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 2, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esi, 2, x86_dep_esi, x86_dep_aux, 0, 0, 0, 0, 0);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -2 : 2;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -2 : 2;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void op_movsd_impl()
{
	uint32_t m32;

	isa_mem_read(isa_mem, isa_regs->esi, 4, &m32);
	isa_mem_write(isa_mem, isa_regs->edi, 4, &m32);

	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 4, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, isa_regs->esi, 4, x86_dep_esi, x86_dep_aux, 0, 0, 0, 0, 0);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -4 : 4;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
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

	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 1, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
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

	x86_uinst_new_mem(x86_uinst_load, isa_regs->edi, 4, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	isa_regs->eflags = flags;
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_stosb_impl()
{
	uint8_t m8 = isa_load_reg(x86_reg_al);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	isa_mem_write(isa_mem, addr, 1, &m8);

	x86_uinst_new_mem(x86_uinst_store, addr, 1, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_stosd_impl()
{
	uint32_t m32 = isa_load_reg(x86_reg_eax);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	isa_mem_write(isa_mem, addr, 4, &m32);

	x86_uinst_new_mem(x86_uinst_store, addr, 4, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}

