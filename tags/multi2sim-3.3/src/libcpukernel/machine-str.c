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


/* Reset or update iteration counters for string operations. */
static void op_rep_init(void)
{
	if (isa_ctx->last_eip == isa_eip)
	{
		isa_ctx->str_op_count++;
	}
	else
	{
		isa_ctx->str_op_count = 0;
		isa_ctx->str_op_esi = isa_regs->esi;
		isa_ctx->str_op_edi = isa_regs->edi;
		isa_ctx->str_op_dir = isa_regs->eflags & (1 << 10) ? -1 : 1;
	}
}


#define OP_REP_IMPL(X, SIZE) \
	void op_rep_##X##_impl() \
	{ \
		op_rep_init(); \
		\
		if (isa_regs->ecx) \
		{ \
			op_##X##_run(); \
			isa_regs->ecx--; \
			isa_regs->eip -= isa_inst.size; \
		} \
		\
		op_##X##_uinst( \
			isa_ctx->str_op_esi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir, \
			isa_ctx->str_op_edi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, 0, 0, 0, 0, 0, 0); \
	}


#define OP_REPZ_IMPL(X, SIZE) \
	void op_repz_##X##_impl() \
	{ \
		op_rep_init(); \
		\
		if (isa_regs->ecx) \
		{ \
			op_##X##_run(); \
			isa_regs->ecx--; \
			if (isa_get_flag(x86_flag_zf)) \
				isa_regs->eip -= isa_inst.size; \
		} \
		\
		op_##X##_uinst( \
			isa_ctx->str_op_esi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir, \
			isa_ctx->str_op_edi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
	}


#define OP_REPNZ_IMPL(X, SIZE) \
	void op_repnz_##X##_impl() \
	{ \
		op_rep_init(); \
		\
		if (isa_regs->ecx) \
		{ \
			op_##X##_run(); \
			isa_regs->ecx--; \
			if (!isa_get_flag(x86_flag_zf)) \
				isa_regs->eip -= isa_inst.size; \
		} \
		\
		op_##X##_uinst( \
			isa_ctx->str_op_esi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir, \
			isa_ctx->str_op_edi + isa_ctx->str_op_count * (SIZE) * isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
	}




/*
 * CMPSB
 */

static void op_cmpsb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, edi, 1, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


static void op_cmpsb_run(void)
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


void op_cmpsb_impl()
{
	op_cmpsb_uinst(isa_regs->esi, isa_regs->edi);
	op_cmpsb_run();
}




/*
 * CMPSD
 */

static void op_cmpsd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 4, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, edi, 4, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


static void op_cmpsd_run(void)
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


void op_cmpsd_impl()
{
	op_cmpsd_uinst(isa_regs->esi, isa_regs->edi);
	op_cmpsd_run();
}




/*
 * INSB
 */

static void op_insb_run(void)
{
	/* Not implemented */
}


static void op_insb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_insb_impl()
{
	op_insb_uinst(isa_regs->esi, isa_regs->edi);
	op_insb_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * INSD
 */

static void op_insd_run(void)
{
	/* Not implemented */
}


static void op_insd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_insd_impl()
{
	op_insd_uinst(isa_regs->esi, isa_regs->edi);
	op_insd_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * LODSB
 */

static void op_lodsb_run(void)
{
	/* Not implemented */
}


static void op_lodsb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_lodsb_impl()
{
	op_lodsb_uinst(isa_regs->esi, isa_regs->edi);
	op_lodsb_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * LODSD
 */

static void op_lodsd_run(void)
{
	/* Not implemented */
}


static void op_lodsd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_lodsd_impl()
{
	op_lodsd_uinst(isa_regs->esi, isa_regs->edi);
	op_lodsd_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * MOVSB
 */

static void op_movsb_run(void)
{
	uint8_t m8;

	isa_mem_read(isa_mem, isa_regs->esi, 1, &m8);
	isa_mem_write(isa_mem, isa_regs->edi, 1, &m8);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


static void op_movsb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void op_movsb_impl()
{
	op_movsb_uinst(isa_regs->esi, isa_regs->edi);
	op_movsb_run();
}




/*
 * MOVSW
 */

static void op_movsw_run(void)
{
	uint16_t m16;

	isa_mem_read(isa_mem, isa_regs->esi, 2, &m16);
	isa_mem_write(isa_mem, isa_regs->edi, 2, &m16);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -2 : 2;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -2 : 2;

}


static void op_movsw_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void op_movsw_impl()
{
	op_movsw_uinst(isa_regs->esi, isa_regs->edi);
	op_movsw_run();
}



/*
 * MOVSD
 */

static void op_movsd_run(void)
{
	uint32_t m32;

	isa_mem_read(isa_mem, isa_regs->esi, 4, &m32);
	isa_mem_write(isa_mem, isa_regs->edi, 4, &m32);

	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
	isa_regs->esi += isa_get_flag(x86_flag_df) ? -4 : 4;

}


static void op_movsd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 4, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 4, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void op_movsd_impl()
{
	op_movsd_uinst(isa_regs->esi, isa_regs->edi);
	op_movsd_run();
}




/*
 * OUTSB
 */

static void op_outsb_run(void)
{
	/* Not implemented */
}


static void op_outsb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_outsb_impl()
{
	op_outsb_uinst(isa_regs->esi, isa_regs->edi);
	op_outsb_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * OUTSD
 */

static void op_outsd_run(void)
{
	/* Not implemented */
}


static void op_outsd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void op_outsd_impl()
{
	op_outsd_uinst(isa_regs->esi, isa_regs->edi);
	op_outsd_run();
	isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * SCAS
 */

static void op_scasb_run(void)
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


static void op_scasb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, edi, 1, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_scasb_impl()
{
	op_scasb_uinst(isa_regs->esi, isa_regs->edi);
	op_scasb_run();
}



/*
 * SCASD
 */

static void op_scasd_run(void)
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


static void op_scasd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, edi, 4, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_scasd_impl()
{
	op_scasd_uinst(isa_regs->esi, isa_regs->edi);
	op_scasd_run();
}



/*
 * STOSB
 */

static void op_stosb_run(void)
{
	uint8_t m8 = isa_load_reg(x86_reg_al);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	
	isa_mem_write(isa_mem, addr, 1, &m8);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -1 : 1;
}


static void op_stosb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_stosb_impl()
{
	op_stosb_uinst(isa_regs->esi, isa_regs->edi);
	op_stosb_run();
}




/*
 * STOSD
 */

static void op_stosd_run(void)
{
	uint32_t m32 = isa_load_reg(x86_reg_eax);
	uint32_t addr = isa_load_reg(x86_reg_edi);
	
	isa_mem_write(isa_mem, addr, 4, &m32);
	isa_regs->edi += isa_get_flag(x86_flag_df) ? -4 : 4;
}


static void op_stosd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_store, edi, 4, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void op_stosd_impl()
{
	op_stosd_uinst(isa_regs->esi, isa_regs->edi);
	op_stosd_run();
}




/*
 * Repetition prefixes
 */

OP_REP_IMPL(insb, 1)
OP_REP_IMPL(insd, 4)

OP_REP_IMPL(movsb, 1)
OP_REP_IMPL(movsw, 2)
OP_REP_IMPL(movsd, 4)

OP_REP_IMPL(outsb, 1)
OP_REP_IMPL(outsd, 4)

OP_REP_IMPL(lodsb, 1)
OP_REP_IMPL(lodsd, 4)

OP_REP_IMPL(stosb, 1)
OP_REP_IMPL(stosd, 4)

OP_REPZ_IMPL(cmpsb, 1)
OP_REPZ_IMPL(cmpsd, 4)

OP_REPZ_IMPL(scasb, 1)
OP_REPZ_IMPL(scasd, 4)

OP_REPNZ_IMPL(cmpsb, 1)
OP_REPNZ_IMPL(cmpsd, 4)

OP_REPNZ_IMPL(scasb, 1)
OP_REPNZ_IMPL(scasd, 4)

