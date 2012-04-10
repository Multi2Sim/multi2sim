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
static void x86_isa_rep_init(void)
{
	if (x86_isa_ctx->last_eip == x86_isa_eip)
	{
		x86_isa_ctx->str_op_count++;
	}
	else
	{
		x86_isa_ctx->str_op_count = 0;
		x86_isa_ctx->str_op_esi = x86_isa_regs->esi;
		x86_isa_ctx->str_op_edi = x86_isa_regs->edi;
		x86_isa_ctx->str_op_dir = x86_isa_regs->eflags & (1 << 10) ? -1 : 1;
	}
}


#define OP_REP_IMPL(X, SIZE) \
	void x86_isa_rep_##X##_impl() \
	{ \
		x86_isa_rep_init(); \
		\
		if (x86_isa_regs->ecx) \
		{ \
			x86_isa_##X##_run(); \
			x86_isa_regs->ecx--; \
			x86_isa_regs->eip -= x86_isa_inst.size; \
		} \
		\
		x86_isa_##X##_uinst( \
			x86_isa_ctx->str_op_esi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir, \
			x86_isa_ctx->str_op_edi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, 0, 0, 0, 0, 0, 0); \
	}


#define OP_REPZ_IMPL(X, SIZE) \
	void x86_isa_repz_##X##_impl() \
	{ \
		x86_isa_rep_init(); \
		\
		if (x86_isa_regs->ecx) \
		{ \
			x86_isa_##X##_run(); \
			x86_isa_regs->ecx--; \
			if (x86_isa_get_flag(x86_flag_zf)) \
				x86_isa_regs->eip -= x86_isa_inst.size; \
		} \
		\
		x86_isa_##X##_uinst( \
			x86_isa_ctx->str_op_esi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir, \
			x86_isa_ctx->str_op_edi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
	}


#define OP_REPNZ_IMPL(X, SIZE) \
	void x86_isa_repnz_##X##_impl() \
	{ \
		x86_isa_rep_init(); \
		\
		if (x86_isa_regs->ecx) \
		{ \
			x86_isa_##X##_run(); \
			x86_isa_regs->ecx--; \
			if (!x86_isa_get_flag(x86_flag_zf)) \
				x86_isa_regs->eip -= x86_isa_inst.size; \
		} \
		\
		x86_isa_##X##_uinst( \
			x86_isa_ctx->str_op_esi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir, \
			x86_isa_ctx->str_op_edi + x86_isa_ctx->str_op_count * (SIZE) * x86_isa_ctx->str_op_dir); \
		x86_uinst_new(x86_uinst_sub, x86_dep_ecx, 0, 0, x86_dep_ecx, 0, 0, 0); \
		x86_uinst_new(x86_uinst_ibranch, x86_dep_ecx, x86_dep_zps, 0, 0, 0, 0, 0); \
	}




/*
 * CMPSB
 */

static void x86_isa_cmpsb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, edi, 1, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


static void x86_isa_cmpsb_run(void)
{
	uint8_t op1, op2;
	unsigned long flags;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->esi, 1, &op1);
	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->edi, 1, &op2);
	flags = x86_isa_regs->eflags;

	__X86_ISA_ASM_START__
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
	__X86_ISA_ASM_END__

	x86_isa_regs->eflags = flags;
	x86_isa_regs->esi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;
}


void x86_isa_cmpsb_impl()
{
	x86_isa_cmpsb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_cmpsb_run();
}




/*
 * CMPSD
 */

static void x86_isa_cmpsd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 4, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_load, edi, 4, x86_dep_edi, 0, 0, x86_dep_aux2, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_aux2, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


static void x86_isa_cmpsd_run(void)
{
	uint32_t op1, op2;
	unsigned long flags;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->edi, 4, &op1);
	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->esi, 4, &op2);
	flags = x86_isa_regs->eflags;

	__X86_ISA_ASM_START__ \
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
	__X86_ISA_ASM_END__ \

	x86_isa_regs->eflags = flags;
	x86_isa_regs->esi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;
}


void x86_isa_cmpsd_impl()
{
	x86_isa_cmpsd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_cmpsd_run();
}




/*
 * INSB
 */

static void x86_isa_insb_run(void)
{
	/* Not implemented */
}


static void x86_isa_insb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_insb_impl()
{
	x86_isa_insb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_insb_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * INSD
 */

static void x86_isa_insd_run(void)
{
	/* Not implemented */
}


static void x86_isa_insd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_insd_impl()
{
	x86_isa_insd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_insd_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * LODSB
 */

static void x86_isa_lodsb_run(void)
{
	/* Not implemented */
}


static void x86_isa_lodsb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_lodsb_impl()
{
	x86_isa_lodsb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_lodsb_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * LODSD
 */

static void x86_isa_lodsd_run(void)
{
	/* Not implemented */
}


static void x86_isa_lodsd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_lodsd_impl()
{
	x86_isa_lodsd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_lodsd_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * MOVSB
 */

static void x86_isa_movsb_run(void)
{
	uint8_t m8;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->esi, 1, &m8);
	x86_isa_mem_write(x86_isa_mem, x86_isa_regs->edi, 1, &m8);

	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;
	x86_isa_regs->esi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;
}


static void x86_isa_movsb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void x86_isa_movsb_impl()
{
	x86_isa_movsb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_movsb_run();
}




/*
 * MOVSW
 */

static void x86_isa_movsw_run(void)
{
	uint16_t m16;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->esi, 2, &m16);
	x86_isa_mem_write(x86_isa_mem, x86_isa_regs->edi, 2, &m16);

	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -2 : 2;
	x86_isa_regs->esi += x86_isa_get_flag(x86_flag_df) ? -2 : 2;

}


static void x86_isa_movsw_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 1, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void x86_isa_movsw_impl()
{
	x86_isa_movsw_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_movsw_run();
}



/*
 * MOVSD
 */

static void x86_isa_movsd_run(void)
{
	uint32_t m32;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->esi, 4, &m32);
	x86_isa_mem_write(x86_isa_mem, x86_isa_regs->edi, 4, &m32);

	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;
	x86_isa_regs->esi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;

}


static void x86_isa_movsd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, esi, 4, x86_dep_esi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new_mem(x86_uinst_store, edi, 4, x86_dep_edi, x86_dep_aux, 0, 0, 0, 0, 0);

	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_esi, x86_dep_df, 0, x86_dep_esi, 0, 0, 0);
}


void x86_isa_movsd_impl()
{
	x86_isa_movsd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_movsd_run();
}




/*
 * OUTSB
 */

static void x86_isa_outsb_run(void)
{
	/* Not implemented */
}


static void x86_isa_outsb_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_outsb_impl()
{
	x86_isa_outsb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_outsb_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * OUTSD
 */

static void x86_isa_outsd_run(void)
{
	/* Not implemented */
}


static void x86_isa_outsd_uinst(uint32_t esi, uint32_t edi)
{
	/* Not implemented */
}


void x86_isa_outsd_impl()
{
	x86_isa_outsd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_outsd_run();
	x86_isa_error("%s: not implemented", __FUNCTION__);
}




/*
 * SCAS
 */

static void x86_isa_scasb_run(void)
{
	uint8_t al = x86_isa_load_reg(x86_reg_al);
	uint8_t m8;
	unsigned long flags = x86_isa_regs->eflags;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->edi, 1, &m8);

	__X86_ISA_ASM_START__ \
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
	__X86_ISA_ASM_END__ \

	x86_isa_regs->eflags = flags;
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;

}


static void x86_isa_scasb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, edi, 1, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void x86_isa_scasb_impl()
{
	x86_isa_scasb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_scasb_run();
}



/*
 * SCASD
 */

static void x86_isa_scasd_run(void)
{
	uint32_t eax = x86_isa_load_reg(x86_reg_eax);
	uint32_t m32;
	unsigned long flags = x86_isa_regs->eflags;

	x86_isa_mem_read(x86_isa_mem, x86_isa_regs->edi, 4, &m32);

	__X86_ISA_ASM_START__ \
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
	__X86_ISA_ASM_END__ \

	x86_isa_regs->eflags = flags;
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;

}


static void x86_isa_scasd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_load, edi, 4, x86_dep_edi, 0, 0, x86_dep_aux, 0, 0, 0);
	x86_uinst_new(x86_uinst_sub, x86_dep_aux, x86_dep_eax, 0, x86_dep_zps, x86_dep_of, x86_dep_cf, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void x86_isa_scasd_impl()
{
	x86_isa_scasd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_scasd_run();
}



/*
 * STOSB
 */

static void x86_isa_stosb_run(void)
{
	uint8_t m8 = x86_isa_load_reg(x86_reg_al);
	uint32_t addr = x86_isa_load_reg(x86_reg_edi);
	
	x86_isa_mem_write(x86_isa_mem, addr, 1, &m8);
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -1 : 1;
}


static void x86_isa_stosb_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_store, edi, 1, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void x86_isa_stosb_impl()
{
	x86_isa_stosb_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_stosb_run();
}




/*
 * STOSD
 */

static void x86_isa_stosd_run(void)
{
	uint32_t m32 = x86_isa_load_reg(x86_reg_eax);
	uint32_t addr = x86_isa_load_reg(x86_reg_edi);
	
	x86_isa_mem_write(x86_isa_mem, addr, 4, &m32);
	x86_isa_regs->edi += x86_isa_get_flag(x86_flag_df) ? -4 : 4;
}


static void x86_isa_stosd_uinst(uint32_t esi, uint32_t edi)
{
	x86_uinst_new_mem(x86_uinst_store, edi, 4, x86_dep_edi, x86_dep_eax, 0, 0, 0, 0, 0);
	x86_uinst_new(x86_uinst_add, x86_dep_edi, x86_dep_df, 0, x86_dep_edi, 0, 0, 0);
}


void x86_isa_stosd_impl()
{
	x86_isa_stosd_uinst(x86_isa_regs->esi, x86_isa_regs->edi);
	x86_isa_stosd_run();
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

