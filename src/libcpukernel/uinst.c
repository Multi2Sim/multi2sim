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


/*
 * Global variables
 */

struct list_t *x86_uinst_list;

struct string_map_t x86_uinst_dep_map = {
	50, {
		{ "none", x86_dep_none },

		{ "eax", x86_dep_eax },
		{ "ecx", x86_dep_ecx },
		{ "edx", x86_dep_edx },
		{ "ebx", x86_dep_ebx },
		{ "esp", x86_dep_esp },
		{ "ebp", x86_dep_ebp },
		{ "esi", x86_dep_esi },
		{ "edi", x86_dep_edi },

		{ "es", x86_dep_es },
		{ "cs", x86_dep_cs },
		{ "ss", x86_dep_ss },
		{ "ds", x86_dep_ds },
		{ "fs", x86_dep_fs },
		{ "gs", x86_dep_gs },

		{ "zps", x86_dep_zps },
		{ "of", x86_dep_of },
		{ "cf", x86_dep_cf },
		{ "df", x86_dep_df },

		{ "aux", x86_dep_aux },
		{ "aux2", x86_dep_aux2 },
		{ "ea", x86_dep_ea },
		{ "data", x86_dep_data },

		{ "st0", x86_dep_st0 },
		{ "st1", x86_dep_st1 },
		{ "st2", x86_dep_st2 },
		{ "st3", x86_dep_st3 },
		{ "st4", x86_dep_st4 },
		{ "st5", x86_dep_st5 },
		{ "st6", x86_dep_st6 },
		{ "st7", x86_dep_st7 },
		{ "fpst", x86_dep_fpst },
		{ "fpcw", x86_dep_fpcw },
		{ "fpaux", x86_dep_fpaux },

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
		{ "*FPOP*", x86_dep_fpop },
		{ "*FPOP2*", x86_dep_fpop2 },
		{ "*FPUSH*", x86_dep_fpush }
	}
};

struct x86_uinst_info_t
{
	char *name;
} x86_uinst_info[x86_uinst_opcode_count] = {

	{ "nop" },

	{ "move" },
	{ "add" },
	{ "sub" },
	{ "mult" },
	{ "div" },
	{ "effaddr" },

	{ "and" },
	{ "or" },
	{ "xor" },
	{ "not" },
	{ "shift" },
	{ "sign" },

	{ "fp_move" },
	{ "fp_simple" },
	{ "fp_add" },
	{ "fp_comp" },
	{ "fp_mult" },
	{ "fp_div" },
	{ "fp_complex" },

	{ "load" },
	{ "store" },

	{ "call" },
	{ "ret" },
	{ "jump" },
	{ "branch" },

	{ "syscall" }
};




/*
 * Private functions
 */


/* This variable is set to 1 whenever an 'rm8', 'rm16', or 'rm32' dependence
 * has been processed in the current macroinstruction. Implication:
 *   Subsequent 'rmXXX' dependences shouldn't generate a new address computation,
 *   but just read the 'x86_dep_ea' dependence.
 * The variable is reset on a call to 'x86_uinst_clear' */
static int x86_uinst_dep_rm_occurred;


static void x86_uinst_emit_effaddr(struct x86_uinst_t *uinst, int index)
{
	struct x86_uinst_t *new_uinst;
	int dep;

	/* Not an 'rmXXX' dependence */
	dep = uinst->dep[index];
	if (isa_inst.modrm_mod == 3 || dep < x86_dep_rm8 || dep > x86_dep_rm32)
		return;

	/* Record occurrence */
	x86_uinst_dep_rm_occurred = 1;
	
	/* Emit 'effaddr' */
	new_uinst = x86_uinst_create();
	new_uinst->opcode = x86_uinst_effaddr;
	new_uinst->idep[0] = isa_inst.segment ? isa_inst.segment - x86_reg_es + x86_dep_es : x86_dep_none;
	new_uinst->idep[1] = isa_inst.ea_base ? isa_inst.ea_base - x86_reg_eax + x86_dep_eax : x86_dep_none;
	new_uinst->idep[2] = isa_inst.ea_index ? isa_inst.ea_index - x86_reg_eax + x86_dep_eax : x86_dep_none;
	new_uinst->odep[0] = x86_dep_ea;
	list_add(x86_uinst_list, new_uinst);
}


static void x86_uinst_parse_dep(struct x86_uinst_t *uinst, int index)
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
	case x86_dep_fpop:
	case x86_dep_fpop2:
	case x86_dep_fpush:

		break;

	case x86_dep_rm8:

		uinst->dep[index] = isa_inst.modrm_rm < 4 ? x86_dep_eax + isa_inst.modrm_rm
			: x86_dep_eax + isa_inst.modrm_rm - 4;
		break;

	case x86_dep_rm16:
	case x86_dep_rm32:

		uinst->dep[index] = x86_dep_eax + isa_inst.modrm_rm;
		break;

	case x86_dep_r8:

		uinst->dep[index] = isa_inst.reg < 4 ? x86_dep_eax + isa_inst.reg
			: x86_dep_eax + isa_inst.reg - 4;
		break;

	case x86_dep_r16:
	case x86_dep_r32:

		uinst->dep[index] = x86_dep_eax + isa_inst.reg;
		break;

	case x86_dep_ir8:

		uinst->dep[index] = isa_inst.opindex < 4 ? x86_dep_eax + isa_inst.opindex
			: x86_dep_eax + isa_inst.opindex - 4;
		break;

	case x86_dep_ir16:
	case x86_dep_ir32:

		uinst->dep[index] = x86_dep_eax + isa_inst.opindex;
		break;

	case x86_dep_sreg:

		uinst->dep[index] = x86_dep_es + isa_inst.reg;
		break;
	
	case x86_dep_sti:

		uinst->dep[index] = x86_dep_st0 + isa_inst.opindex;
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


static void x86_uinst_parse_odep(struct x86_uinst_t *uinst, int index)
{
	struct x86_uinst_t *new_uinst;
	int size;
	int dep;

	/* Get dependence */
	assert(index >= X86_UINST_MAX_IDEPS && index < X86_UINST_MAX_DEPS);
	dep = uinst->dep[index];
	if (!dep)
		return;

	if (isa_inst.modrm_mod != 3 && dep >= x86_dep_rm8 && dep <= x86_dep_rm32)
	{
		/* Calculate operand size (1 for 'rm8', 2 for 'rm16', 4 for 'rm32') */
		size = 1 << (dep - x86_dep_rm8);

		/* If uinst is 'move', just convert it into a 'store' */
		if (uinst->opcode == x86_uinst_move) {

			/* Try to add 'ea' as input dependence */
			if (x86_uinst_add_idep(uinst, x86_dep_ea)) {
				uinst->opcode = x86_uinst_store;
				uinst->dep[index] = x86_dep_none;
				uinst->address = isa_effective_address();
				uinst->size = size;
				return;
			}
		}

		/* Store */
		new_uinst = x86_uinst_create();
		new_uinst->opcode = x86_uinst_store;
		new_uinst->idep[0] = x86_dep_ea;
		new_uinst->idep[1] = x86_dep_data;
		new_uinst->address = isa_effective_address();
		new_uinst->size = size;
		list_add(x86_uinst_list, new_uinst);

		/* Output dependence of instruction is x86_dep_data */
		uinst->dep[index] = x86_dep_data;
	}
	else
	{
		/* Regular dependence */
		x86_uinst_parse_dep(uinst, index);
	}
}


static void x86_uinst_parse_idep(struct x86_uinst_t *uinst, int index)
{
	struct x86_uinst_t *new_uinst;
	int size;
	int dep;

	/* Get dependence */
	assert(index >= 0 && index < X86_UINST_MAX_IDEPS);
	dep = uinst->dep[index];
	if (!dep)
		return;
	
	if (isa_inst.modrm_mod != 3 && dep >= x86_dep_rm8 && dep <= x86_dep_rm32)
	{
		/* Calculate operand size (1 for 'rm8', 2 for 'rm16', 4 for 'rm32') */
		size = 1 << (dep - x86_dep_rm8);

		/* If uinst is 'move', just convert it into a 'load' */
		/* Replace 'rmXXX' by 'ea' dependence */
		if (uinst->opcode == x86_uinst_move) {
			uinst->opcode = x86_uinst_load;
			uinst->dep[index] = x86_dep_ea;
			uinst->address = isa_effective_address();
			uinst->size = size;
			return;
		}

		/* Load */
		new_uinst = x86_uinst_create();
		new_uinst->opcode = x86_uinst_load;
		new_uinst->idep[0] = x86_dep_ea;
		new_uinst->odep[0] = x86_dep_data;
		new_uinst->address = isa_effective_address();
		new_uinst->size = size;
		list_add(x86_uinst_list, new_uinst);

		/* Input dependence of instruction is converted into 'x86_dep_data' */
		uinst->dep[index] = x86_dep_data;

	}
	else if (dep == x86_dep_easeg)
	{
		uinst->dep[index] = isa_inst.segment ? isa_inst.segment
			- x86_reg_es + x86_dep_es : x86_dep_none;
	}
	else if (dep == x86_dep_eabas)
	{
		uinst->dep[index] = isa_inst.ea_base ? isa_inst.ea_base
			- x86_reg_eax + x86_dep_eax : x86_dep_none;
	}
	else if (dep == x86_dep_eaidx)
	{
		uinst->dep[index] = isa_inst.ea_index ? isa_inst.ea_index
			- x86_reg_eax + x86_dep_eax : x86_dep_none;
	}
	else
	{
		/* Regular dependence */
		x86_uinst_parse_dep(uinst, index);
	}
}




/*
 * Public Functions
 */


void x86_uinst_init(void)
{
	x86_uinst_list = list_create();
}


void x86_uinst_done(void)
{
	x86_uinst_clear();
	list_free(x86_uinst_list);
}


struct x86_uinst_t *x86_uinst_create(void)
{
	struct x86_uinst_t *uinst;

	uinst = calloc(1, sizeof(struct x86_uinst_t));
	if (!uinst)
		fatal("%s: out of memory", __FUNCTION__);
	uinst->idep = uinst->dep;
	uinst->odep = &uinst->dep[X86_UINST_MAX_IDEPS];
	return uinst;
}


void x86_uinst_free(struct x86_uinst_t *uinst)
{
	free(uinst);
}


void __x86_uinst_new_mem(enum x86_uinst_opcode_t opcode, uint32_t address, int size,
	enum x86_dep_t idep0, enum x86_dep_t idep1, enum x86_dep_t idep2,
	enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3)
{
	struct x86_uinst_t *uinst;
	int i;

	/* Do nothing for functional simulation */
	if (cpu_sim_kind == cpu_sim_kind_functional)
		return;
	
	/* Create uinst */
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

	/* Emit effective address computation if any dependence is 'rmXXX' and none
	 * 'effaddr' uinst has been inserted in the list so far. */
	for (i = 0; !x86_uinst_dep_rm_occurred && i < X86_UINST_MAX_DEPS; i++)
		x86_uinst_emit_effaddr(uinst, i);
	
	/* Parse input dependences */
	for (i = 0; i < X86_UINST_MAX_IDEPS; i++)
		x86_uinst_parse_idep(uinst, i);
	
	/* Add micro-instruction */
	list_add(x86_uinst_list, uinst);
	
	/* Parse output dependences */
	for (i = 0; i < X86_UINST_MAX_ODEPS; i++)
		x86_uinst_parse_odep(uinst, i + X86_UINST_MAX_IDEPS);
}


void __x86_uinst_new(enum x86_uinst_opcode_t opcode, enum x86_dep_t idep0, enum x86_dep_t idep1,
	enum x86_dep_t idep2, enum x86_dep_t odep0, enum x86_dep_t odep1, enum x86_dep_t odep2,
	enum x86_dep_t odep3)
{
	__x86_uinst_new_mem(opcode, 0, 0, idep0, idep1, idep2,
		odep0, odep1, odep2, odep3);
}


void x86_uinst_clear(void)
{
	/* Clear list */
	while (list_count(x86_uinst_list))
		x86_uinst_free(list_remove_at(x86_uinst_list, 0));
	
	/* Forget occurrence of 'rmXXX' in previous inst */
	x86_uinst_dep_rm_occurred = 0;
}


void x86_uinst_dump(struct x86_uinst_t *uinst, FILE *f)
{
	char *comma;
	enum x86_dep_t dep;
	int i;

	/* Instruction name */
	fprintf(f, "%s ", x86_uinst_info[uinst->opcode].name);

	/* Output operands */
	comma = "";
	fprintf(f, "<");
	for (i = 0; i < X86_UINST_MAX_ODEPS; i++) {
		dep = uinst->odep[i];
		if (!dep)
			continue;
		fprintf(f, "%s%s", comma, map_value(&x86_uinst_dep_map, dep));
		comma = ",";
	}

	/* Separator */
	fprintf(f, ">/<");

	/* Input operands */
	comma = "";
	for (i = 0; i < X86_UINST_MAX_IDEPS; i++) {
		dep = uinst->idep[i];
		if (!dep)
			continue;
		fprintf(f, "%s%s", comma, map_value(&x86_uinst_dep_map, dep));
		comma = ",";
	}
	fprintf(f, ">");

	/* Memory address */
	if (uinst->size)
		fprintf(f, " [0x%x,%d]", uinst->address, uinst->size);

	/* End */
	fprintf(f, "\n");
}


void x86_uinst_list_dump(FILE *f)
{
	struct x86_uinst_t *uinst;
	int i;

	for (i = 0; i < list_count(x86_uinst_list); i++) {
		uinst = list_get(x86_uinst_list, i);
		fprintf(f, "  ");
		x86_uinst_dump(uinst, f);
	}
}

