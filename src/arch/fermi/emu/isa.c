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

#include <repos.h>

#include <fermi-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


/* Some globals */

struct frm_grid_t *frm_isa_grid;  /* Current ND-Range */
struct frm_threadblock_t *frm_isa_threadblock;  /* Current work-group */
struct frm_warp_t *frm_isa_warp;  /* Current warp */
struct frm_thread_t *frm_isa_thread;  /* Current work-item */
struct frm_inst_t *frm_isa_inst;  /* Current instruction */
struct frm_alu_group_t *frm_isa_alu_group;  /* Current ALU group */

/* Repository of deferred tasks */
struct repos_t *frm_isa_write_task_repos;

/* Instruction execution table */
frm_isa_inst_func_t *frm_isa_inst_func;

/* Debug */
int frm_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void frm_isa_init()
{
	/* Allocate instruction execution table */
	frm_isa_inst_func = calloc(FRM_INST_COUNT, sizeof(frm_isa_inst_func_t));
	if (!frm_isa_inst_func)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	extern void frm_isa_##_name##_impl(); \
	frm_isa_inst_func[FRM_INST_##_name] = frm_isa_##_name##_impl;
#include <fermi-asm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
}


void frm_isa_done()
{
	/* Instruction execution table */
	free(frm_isa_inst_func);

	/* Repository of deferred tasks */
}




/*
 * Constant Memory
 */

void frm_isa_const_mem_write(int bank, int vector, int elem, void *pvalue)
{
	unsigned int addr;

	/* Mark CB0[0..8].{x,y,z,w} positions as initialized */
	if (!bank && vector < 9)
		frm_emu->const_mem_cb0_init[vector * 4 + elem] = 1;

	/* Write */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_write(frm_emu->const_mem, addr, 4, pvalue);
}


void frm_isa_const_mem_read(int bank, int vector, int elem, void *pvalue)
{
	unsigned int addr;

	/* Warn if a position within CB[0..8].{x,y,z,w} is used uninitialized */
	if (!bank && vector < 9 && !frm_emu->const_mem_cb0_init[vector * 4 + elem])
		warning("CB0[%d].%c is used uninitialized", vector, "xyzw"[elem]);
	
	/* Read */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_read(frm_emu->const_mem, addr, 4, pvalue);
}





/*
 * ALU Clauses
 */

/* Called before and ALU clause starts in 'frm_isa_warp' */
void frm_isa_alu_clause_start()
{
	/* Copy 'active' mask at the top of the stack to 'pred' mask */

	/* Flag 'push_before_done' will be set by the first PRED_SET* inst */

	/* Stats */
}


/* Called after an ALU clause completed in a warp */
void frm_isa_alu_clause_end()
{
	/* If CF inst was ALU_POP_AFTER, pop the stack */
}





/*
 * Instruction operands
 */

/* Dump a destination value depending on the format of the destination operand
 * in the current instruction, as specified by its flags. */
void frm_isa_dest_value_dump(void *value_ptr, FILE *f)
{
		fprintf(f, "%d", * (int *) value_ptr);
	
		fprintf(f, "0x%x", * (unsigned int *) value_ptr);
	
		fprintf(f, "%gf", * (float *) value_ptr);
}


/* Read source GPR */
unsigned int frm_isa_read_gpr(int gpr, int rel, int chan, int im)
{
	return (frm_isa_thread->gpr[gpr].elem);
}


/* Read source GPR in float format */
float frm_isa_read_gpr_float(int gpr, int rel, int chan, int im)
{
	union frm_reg_t reg;

	reg.as_uint = frm_isa_read_gpr(gpr, rel, chan, im);
	return reg.as_float;
}


void frm_isa_write_gpr(int gpr, int rel, int chan, unsigned int value)
{
	frm_isa_thread->gpr[(gpr)].elem = value;
}


void frm_isa_write_gpr_float(int gpr, int rel, int chan, float value)
{
	union frm_reg_t reg;

	reg.as_float = value;
	frm_isa_write_gpr(gpr, rel, chan, reg.as_uint);
}


/* Read source operand in ALU instruction.
 * This is a common function for both integer and float formats. */
static unsigned int gpu_isa_read_op_src_common(int src_idx, int *neg_ptr, int *abs_ptr)
{
	int sel;
	int rel;
	int chan;

	int32_t value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	frm_inst_get_op_src(frm_isa_inst, src_idx, &sel, &rel, &chan, neg_ptr, abs_ptr);

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
	{
		int index_mode;
		index_mode = 0;
		value = frm_isa_read_gpr(sel, rel, chan, index_mode);
		return value;
	}

	/* 128..159: Kcache 0 constant */
	if (IN_RANGE(sel, 128, 159))
	{
		return 0;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191))
	{
		return 0;
	}

	/* QA and QA.pop */
	if (sel == 219 || sel == 221)
	{
		return 0;
	}

	/* QB and QB.pop */
	if (sel == 220 || sel == 222)
	{
		return 0;
	}

	/* ALU_SRC_0 */
	if (sel == 248)
	{
		value = 0;
		return value;
	}

	/* ALU_SRC_1 */
	if (sel == 249)
	{
		union frm_reg_t reg;

		reg.as_float = 1.0f;
		value = reg.as_uint;

		return value;
	}

	/* ALU_SRC_1_INT */
	if (sel == 250)
	{
		value = 1;
		return value;
	}

	/* ALU_SRC_M_1_INT */
	if (sel == 251)
	{
		value = -1;
		return value;
	}

	/* ALU_SRC_0_5 */
	if (sel == 252)
	{
		union frm_reg_t reg;

		reg.as_float = 0.5f;
		value = reg.as_uint;

		return value;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253)
	{
		return 0;
	}

	/* ALU_SRC_PV */
	if (sel == 254)
	{
		value = frm_isa_thread->pv.elem;
		return value;
	}

	/* ALU_SRC_PS */
	if (sel == 255)
	{
		value = frm_isa_thread->pv.elem;
		return value;
	}

	/* Not implemented 'sel' field */
	fatal("%s: src_idx=%d, not implemented for sel=%d", __FUNCTION__, src_idx, sel);
	return 0;
}


unsigned int frm_isa_read_op_src_int(int src_idx)
{
	int neg, abs;
	int value; /* Signed */

	value = gpu_isa_read_op_src_common(src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && value < 0)
		value = -value;
	if (neg)
		value = -value;
	
	/* Return as unsigned */
	return value;
}


float frm_isa_read_op_src_float(int src_idx)
{
	union frm_reg_t reg;
	int neg;
	int abs;

	/* Read register */
	reg.as_uint = gpu_isa_read_op_src_common(src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && reg.as_float < 0.0)
		reg.as_float = -reg.as_float;
	if (neg)
		reg.as_float = -reg.as_float;
	
	/* Return */
	return reg.as_float;
}




/*
 * Deferred tasks for ALU group
 */

void frm_isa_enqueue_write_lds(unsigned int addr, unsigned int value, int value_size)
{
}


/* Write to destination operand in ALU instruction */
void frm_isa_enqueue_write_dest(unsigned int value)
{
}


void frm_isa_enqueue_write_dest_float(float value)
{
}


void frm_isa_enqueue_push_before(void)
{
}


void frm_isa_enqueue_pred_set(int cond)
{
}


void frm_isa_write_task_commit(void)
{
}

