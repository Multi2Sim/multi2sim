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
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "context.h"
#include "uinst.h"


/*
 * Global variables
 */

struct list_t *x86_uinst_list;
int x86_uinst_active;


/* Direct look-up table for regular dependences */
char *x86_uinst_dep_name[] =
{
	"-",  /* 0 */

	"eax",  /* 1 */
	"ecx",  /* 2 */
	"edx",  /* 3 */
	"ebx",  /* 4 */
	"esp",  /* 5 */
	"ebp",  /* 6 */
	"esi",  /* 7 */
	"edi",  /* 8 */

	"es",  /* 9 */
	"cs",  /* 10 */
	"ss",  /* 11 */
	"ds",  /* 12 */
	"fs",  /* 13 */
	"gs",  /* 14 */

	"zps", /* 15 */
	"of",  /* 16 */
	"cf",  /* 17 */
	"df",  /* 18 */

	"aux",  /* 19 */
	"aux2",  /* 20 */
	"ea",  /* 21 */
	"data",  /* 22 */

	"st0",  /* 23 */
	"st1",  /* 24 */
	"st2",  /* 25 */
	"st3",  /* 26 */
	"st4",  /* 27 */
	"st5",  /* 28 */
	"st6",  /* 29 */
	"st7",  /* 30 */

	"fpst",  /* 31 */
	"fpcw",  /* 32 */
	"fpaux",  /* 33 */

	"xmm0",  /* 34 */
	"xmm1",  /* 35 */
	"xmm2",  /* 36 */
	"xmm3",  /* 37 */
	"xmm4",  /* 38 */
	"xmm5",  /* 39 */
	"xmm6",  /* 40 */
	"xmm7",  /* 41 */
	"xmm_data"  /* 42 */
};


/* String map for the rest of the dependences */
struct str_map_t x86_uinst_dep_map =
{
	13, {
		{ "*RM8*", x86_dep_rm8 },
		{ "*RM16*", x86_dep_rm16 },
		{ "*RM32*", x86_dep_rm32 },

		{ "*IR8*", x86_dep_ir8 },
		{ "*IR16*", x86_dep_ir16 },
		{ "*IR32*", x86_dep_ir32 },

		{ "*R8*", x86_dep_r8 },
		{ "*R16*", x86_dep_r16 },
		{ "*R32*", x86_dep_r32 },

		{ "*EASEG*", x86_dep_easeg },
		{ "*EABAS*", x86_dep_eabas },
		{ "*EAIDX*", x86_dep_eaidx },

		{ "*STI*", x86_dep_sti },
	}
};


/* Information related with a micro-instruction opcode */
struct x86_uinst_info_t x86_uinst_info[x86_uinst_opcode_count] =
{

	{ "nop", 0 },

	{ "move", X86_UINST_INT },
	{ "add", X86_UINST_INT },
	{ "sub", X86_UINST_INT },
	{ "mult", X86_UINST_INT },
	{ "div", X86_UINST_INT },
	{ "effaddr", X86_UINST_INT },

	{ "and", X86_UINST_LOGIC },
	{ "or", X86_UINST_LOGIC },
	{ "xor", X86_UINST_LOGIC },
	{ "not", X86_UINST_LOGIC },
	{ "shift", X86_UINST_LOGIC },
	{ "sign", X86_UINST_LOGIC },

	{ "fmove", X86_UINST_FP },
	{ "fsign", X86_UINST_FP },
	{ "fround", X86_UINST_FP },

	{ "fadd", X86_UINST_FP },
	{ "fsub", X86_UINST_FP },
	{ "fcomp", X86_UINST_FP },
	{ "fmult", X86_UINST_FP },
	{ "fdiv", X86_UINST_FP },

	{ "fexp", X86_UINST_FP },
	{ "flog", X86_UINST_FP },
	{ "fsin", X86_UINST_FP },
	{ "fcos", X86_UINST_FP },
	{ "fsincos", X86_UINST_FP },
	{ "ftan", X86_UINST_FP },
	{ "fatan", X86_UINST_FP },
	{ "fsqrt", X86_UINST_FP },

	{ "fpush", X86_UINST_FP },
	{ "fpop", X86_UINST_FP },

	{ "x-and", X86_UINST_XMM },  /* x86_uinst_xmm_and */
	{ "x-or", X86_UINST_XMM },  /* x86_uinst_xmm_or */
	{ "x-xor", X86_UINST_XMM },  /* x86_uinst_xmm_xor */
	{ "x-not", X86_UINST_XMM },  /* x86_uinst_xmm_not */
	{ "x-nand", X86_UINST_XMM },  /* x86_uinst_xmm_nand */
	{ "x-shift", X86_UINST_XMM },  /* x86_uinst_xmm_shift */
	{ "x-sign", X86_UINST_XMM },  /* x86_uinst_xmm_sign */

	{ "x-add", X86_UINST_XMM },
	{ "x-sub", X86_UINST_XMM },
	{ "x-comp", X86_UINST_XMM },
	{ "x-mult", X86_UINST_XMM },
	{ "x-div", X86_UINST_XMM },

	{ "x-fadd", X86_UINST_XMM },
	{ "x-fsub", X86_UINST_XMM },
	{ "x-fcomp", X86_UINST_XMM },
	{ "x-fmult", X86_UINST_XMM },
	{ "x-fdiv", X86_UINST_XMM },

	{ "x-fsqrt", X86_UINST_XMM },

	{ "x-move", X86_UINST_XMM },
	{ "x-shuf", X86_UINST_XMM },
	{ "x-conv", X86_UINST_XMM },

	{ "load", X86_UINST_MEM },
	{ "store", X86_UINST_MEM },
	{ "prefetch", X86_UINST_MEM },

	{ "call", X86_UINST_CTRL | X86_UINST_UNCOND },
	{ "ret", X86_UINST_CTRL | X86_UINST_UNCOND },
	{ "jump", X86_UINST_CTRL | X86_UINST_UNCOND },
	{ "branch", X86_UINST_CTRL | X86_UINST_COND },
	{ "ibranch", X86_UINST_CTRL | X86_UINST_COND },

	{ "syscall", 0 }
};




/*
 * Private functions
 */


/* This variable is set to 1 whenever a memory dependence is found and processed
 * in the current x86 macro-instruction. Subsequent memory dependences shouldn't
 * generate a new address computation, but just read the 'x86_dep_ea' dependence. */
static int x86_uinst_effaddr_emitted;


/* If dependence 'index' in 'uinst' is a memory operand, return its size in bytes.
 * Otherwise, return 0. 
 * Also returns a regular dependence corresponding the memory dependence data type. */
static int x86_uinst_mem_dep_size(struct x86_uinst_t *uinst, int index, 
		X86Context *ctx, enum x86_dep_t *mem_regular_dep)
{
	int dep;

	assert(index >= 0 && index < X86_UINST_MAX_DEPS);
	dep = uinst->dep[index];

	switch (dep)
	{

	case x86_dep_rm8:
	case x86_dep_rm16:
	case x86_dep_rm32:

		/* The 'modrm_mod' field indicates whether it's actually a memory
		 * dependence or a register. */
		if (ctx->inst.modrm_mod == 3)
			return 0;

		PTR_ASSIGN(mem_regular_dep, x86_dep_data);
		return 1 << (dep - x86_dep_rm8);

	case x86_dep_mem8:
	case x86_dep_mem16:
	case x86_dep_mem32:
	case x86_dep_mem64:
	case x86_dep_mem128:

		PTR_ASSIGN(mem_regular_dep, x86_dep_data);
		return 1 << (dep - x86_dep_mem8);

	case x86_dep_mem80:

		PTR_ASSIGN(mem_regular_dep, x86_dep_data);
		return 10;

	case x86_dep_xmmm32:
	case x86_dep_xmmm64:
	case x86_dep_xmmm128:
		
		/* The 'modrm_mod' field indicates whether it's actually a memory
		 * dependence or a register. */
		if(ctx->inst.modrm_mod == 3)
			return 0;

		PTR_ASSIGN(mem_regular_dep, x86_dep_xmm_data);
		return 1 << (dep - x86_dep_xmmm32 + 2);

	default:
		return 0;
	}
}


static void x86_uinst_emit_effaddr(struct x86_uinst_t *uinst, int index, X86Context *ctx)
{
	struct x86_uinst_t *new_uinst;

	/* Check if it is a memory dependence */
	if (!x86_uinst_mem_dep_size(uinst, index, ctx, NULL))
		return;

	/* Record occurrence */
	x86_uinst_effaddr_emitted = 1;
	
	/* Emit 'effaddr' */
	new_uinst = x86_uinst_create();
	new_uinst->opcode = x86_uinst_effaddr;
	new_uinst->idep[0] = ctx->inst.segment ? ctx->inst.segment - x86_inst_reg_es + x86_dep_es : x86_dep_none;
	new_uinst->idep[1] = ctx->inst.ea_base ? ctx->inst.ea_base - x86_inst_reg_eax + x86_dep_eax : x86_dep_none;
	new_uinst->idep[2] = ctx->inst.ea_index ? ctx->inst.ea_index - x86_inst_reg_eax + x86_dep_eax : x86_dep_none;
	new_uinst->odep[0] = x86_dep_ea;
	list_add(x86_uinst_list, new_uinst);
}


static void x86_uinst_parse_dep(struct x86_uinst_t *uinst, int index, X86Context *ctx)
{
	int dep;

	/* Regular dependence */
	assert(index >= 0 && index < X86_UINST_MAX_DEPS);
	dep = uinst->dep[index];
	if (X86_DEP_IS_VALID(dep))
		return;
	
	/* Instruction dependent */
	switch (dep) {

	case x86_dep_none:

		break;

	case x86_dep_easeg:

		uinst->dep[index] = ctx->inst.segment ? ctx->inst.segment
			- x86_inst_reg_es + x86_dep_es : x86_dep_none;
		break;

	case x86_dep_eabas:

		uinst->dep[index] = ctx->inst.ea_base ? ctx->inst.ea_base
			- x86_inst_reg_eax + x86_dep_eax : x86_dep_none;
		break;

	case x86_dep_eaidx:

		uinst->dep[index] = ctx->inst.ea_index ? ctx->inst.ea_index
			- x86_inst_reg_eax + x86_dep_eax : x86_dep_none;
		break;

	/* If we reached this point, 'rmXXX' dependences are actually registers.
	 * Otherwise, they would have been handled by 'parse_idep'/'parse_odep' functions. */
	case x86_dep_rm8:

		assert(ctx->inst.modrm_mod == 3);
		uinst->dep[index] = ctx->inst.modrm_rm < 4 ? x86_dep_eax + ctx->inst.modrm_rm
			: x86_dep_eax + ctx->inst.modrm_rm - 4;
		break;

	case x86_dep_rm16:
	case x86_dep_rm32:

		assert(ctx->inst.modrm_mod == 3);
		uinst->dep[index] = x86_dep_eax + ctx->inst.modrm_rm;
		break;

	case x86_dep_r8:

		uinst->dep[index] = ctx->inst.reg < 4 ? x86_dep_eax + ctx->inst.reg
			: x86_dep_eax + ctx->inst.reg - 4;
		break;

	case x86_dep_r16:
	case x86_dep_r32:

		uinst->dep[index] = x86_dep_eax + ctx->inst.reg;
		break;

	case x86_dep_ir8:

		uinst->dep[index] = ctx->inst.opindex < 4 ? x86_dep_eax + ctx->inst.opindex
			: x86_dep_eax + ctx->inst.opindex - 4;
		break;

	case x86_dep_ir16:
	case x86_dep_ir32:

		uinst->dep[index] = x86_dep_eax + ctx->inst.opindex;
		break;

	case x86_dep_sreg:

		uinst->dep[index] = x86_dep_es + ctx->inst.reg;
		break;
	
	case x86_dep_sti:

		uinst->dep[index] = x86_dep_st0 + ctx->inst.opindex;
		break;
	
	case x86_dep_xmmm32:
	case x86_dep_xmmm64:
	case x86_dep_xmmm128:

		assert(ctx->inst.modrm_mod == 3);
		uinst->dep[index] = x86_dep_xmm0 + ctx->inst.modrm_rm;
		break;

	case x86_dep_xmm:

		uinst->dep[index] = x86_dep_xmm0 + ctx->inst.reg;
		break;

	default:

		panic("%s: unknown dep: %d\n", __FUNCTION__, dep);
	}
}


/* Add an input dependence.
 * Return FALSE if there was no free space. */
static int x86_uinst_add_idep(struct x86_uinst_t *uinst, enum x86_dep_t dep)
{
	int i;

	for (i = 0; i < X86_UINST_MAX_IDEPS; i++)
		if (!uinst->idep[i])
			break;
	if (i == X86_UINST_MAX_IDEPS)
		return 0;
	uinst->idep[i] = dep;
	return 1;
}


static void x86_uinst_parse_odep(struct x86_uinst_t *uinst, int index, X86Context *ctx)
{
	struct x86_uinst_t *new_uinst;
	enum x86_dep_t mem_regular_dep;
	int mem_dep_size;
	int dep;

	/* Get dependence */
	assert(index >= X86_UINST_MAX_IDEPS && index < X86_UINST_MAX_DEPS);
	dep = uinst->dep[index];
	if (!dep)
		return;

	/* Memory dependence */
	mem_dep_size = x86_uinst_mem_dep_size(uinst, index, ctx, &mem_regular_dep);
	if (mem_dep_size)
	{
		/* If uinst is 'move', just convert it into a 'store' */
		if (uinst->opcode == x86_uinst_move)
		{
			/* Try to add 'ea' as input dependence */
			if (x86_uinst_add_idep(uinst, x86_dep_ea))
			{
				uinst->opcode = x86_uinst_store;
				uinst->dep[index] = x86_dep_none;
				uinst->address = ctx->effective_address;
				uinst->size = mem_dep_size;
				return;
			}
		}

		/* Store */
		new_uinst = x86_uinst_create();
		new_uinst->opcode = x86_uinst_store;
		new_uinst->idep[0] = x86_dep_ea;
		new_uinst->idep[1] = mem_regular_dep;
		new_uinst->address = ctx->effective_address;
		new_uinst->size = mem_dep_size;
		list_add(x86_uinst_list, new_uinst);

		/* Output dependence of instruction is x86_dep_data */
		uinst->dep[index] = mem_regular_dep;
		return;
	}

	/* Regular dependence */
	x86_uinst_parse_dep(uinst, index, ctx);
}


static void x86_uinst_parse_idep(struct x86_uinst_t *uinst, int index, X86Context *ctx)
{
	struct x86_uinst_t *new_uinst;
	enum x86_dep_t mem_regular_dep;
	int mem_dep_size;
	int dep;

	/* Get dependence */
	assert(index >= 0 && index < X86_UINST_MAX_IDEPS);
	dep = uinst->dep[index];
	if (!dep)
		return;
	
	/* Memory dependence */
	mem_dep_size = x86_uinst_mem_dep_size(uinst, index, ctx, &mem_regular_dep);
	if (mem_dep_size)
	{
		/* If uinst is 'move', just convert it into a 'load' */
		/* Replace 'rmXXX' by 'ea' dependence */
		if (uinst->opcode == x86_uinst_move)
		{
			uinst->opcode = x86_uinst_load;
			uinst->dep[index] = x86_dep_ea;
			uinst->address = ctx->effective_address;
			uinst->size = mem_dep_size;
			return;
		}

		/* Load */
		new_uinst = x86_uinst_create();
		new_uinst->opcode = x86_uinst_load;
		new_uinst->idep[0] = x86_dep_ea;
		new_uinst->odep[0] = mem_regular_dep;
		new_uinst->address = ctx->effective_address;
		new_uinst->size = mem_dep_size;
		list_add(x86_uinst_list, new_uinst);

		/* Input dependence of instruction is converted into 'x86_dep_data' */
		uinst->dep[index] = mem_regular_dep;
		return;
	}

	/* Regular dependence */
	x86_uinst_parse_dep(uinst, index, ctx);
}


static char *x86_uinst_dep_name_get(int dep)
{
	if (X86_DEP_IS_VALID(dep))
		return x86_uinst_dep_name[dep];
	else
		return str_map_value(&x86_uinst_dep_map, dep);
}




/*
 * Public Functions
 */


void x86_uinst_init(void)
{
	x86_uinst_list = list_create();
	x86_uinst_active = arch_x86->sim_kind == arch_sim_kind_detailed;
}


void x86_uinst_done(void)
{
	x86_uinst_clear();
	list_free(x86_uinst_list);
}


struct x86_uinst_t *x86_uinst_create(void)
{
	struct x86_uinst_t *uinst;

	uinst = xcalloc(1, sizeof(struct x86_uinst_t));
	uinst->idep = uinst->dep;
	uinst->odep = &uinst->dep[X86_UINST_MAX_IDEPS];
	return uinst;
}


void x86_uinst_free(struct x86_uinst_t *uinst)
{
	free(uinst);
}


void __x86_uinst_new_mem(X86Context *ctx,
	enum x86_uinst_opcode_t opcode, unsigned int address, int size,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3)
{
	struct x86_uinst_t *uinst;
	int i;

	/* Create micro-instruction */
	assert(arch_x86->sim_kind == arch_sim_kind_detailed);
	uinst = x86_uinst_create();
	uinst->opcode = opcode;
	uinst->idep[0] = idep0;
	uinst->idep[1] = idep1;
	uinst->idep[2] = idep2;
	uinst->odep[0] = odep0;
	uinst->odep[1] = odep1;
	uinst->odep[2] = odep2;
	uinst->odep[3] = odep3;
	uinst->address = address;
	uinst->size = size;

	/* Emit effective address computation if needed. */
	for (i = 0; !x86_uinst_effaddr_emitted && i < X86_UINST_MAX_DEPS; i++)
		x86_uinst_emit_effaddr(uinst, i, ctx);
	
	/* Parse input dependences */
	for (i = 0; i < X86_UINST_MAX_IDEPS; i++)
		x86_uinst_parse_idep(uinst, i, ctx);
	
	/* Add micro-instruction */
	list_add(x86_uinst_list, uinst);
	
	/* Parse output dependences */
	for (i = 0; i < X86_UINST_MAX_ODEPS; i++)
		x86_uinst_parse_odep(uinst, i + X86_UINST_MAX_IDEPS, ctx);
}


void __x86_uinst_new(X86Context *ctx,
	enum x86_uinst_opcode_t opcode, enum x86_dep_t idep0, enum x86_dep_t idep1,
	enum x86_dep_t idep2, enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3)
{
	__x86_uinst_new_mem(ctx, opcode, 0, 0, idep0, idep1, idep2,
		odep0, odep1, odep2, odep3);
}


void x86_uinst_clear(void)
{
	/* Clear list */
	while (list_count(x86_uinst_list))
		x86_uinst_free(list_remove_at(x86_uinst_list, 0));
	
	/* Forget occurrence of effective address computation in previous inst */
	x86_uinst_effaddr_emitted = 0;
}


void x86_uinst_dump_buf(struct x86_uinst_t *uinst, char *buf, int size)
{
	char *comma;
	enum x86_dep_t dep;
	int dep_count;
	int i;

	/* Instruction name */
	str_printf(&buf, &size, "%s ", x86_uinst_info[uinst->opcode].name);

	/* Output operands */
	comma = "";
	dep_count = 0;
	for (i = 0; i < X86_UINST_MAX_ODEPS; i++)
	{
		dep = uinst->odep[i];
		if (!dep)
			continue;
		dep_count++;
		str_printf(&buf, &size, "%s%s", comma, x86_uinst_dep_name_get(dep));
		comma = ",";
	}
	if (!dep_count)
		str_printf(&buf, &size, "-");

	/* Separator */
	str_printf(&buf, &size, "/");

	/* Input operands */
	comma = "";
	dep_count = 0;
	for (i = 0; i < X86_UINST_MAX_IDEPS; i++)
	{
		dep = uinst->idep[i];
		if (!dep)
			continue;
		dep_count++;
		str_printf(&buf, &size, "%s%s", comma, x86_uinst_dep_name_get(dep));
		comma = ",";
	}
	if (!dep_count)
		str_printf(&buf, &size, "-");

	/* Memory address */
	if (uinst->size)
		str_printf(&buf, &size, " [0x%x,%d]", uinst->address, uinst->size);
}


void x86_uinst_dump(struct x86_uinst_t *uinst, FILE *f)
{
	char buf[MAX_STRING_SIZE];

	x86_uinst_dump_buf(uinst, buf, sizeof(buf));
	fprintf(f, "%s", buf);
}


void x86_uinst_list_dump(FILE *f)
{
	struct x86_uinst_t *uinst;
	int i;

	for (i = 0; i < list_count(x86_uinst_list); i++)
	{
		uinst = list_get(x86_uinst_list, i);
		fprintf(f, "  ");
		x86_uinst_dump(uinst, f);
	}
}

