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

#include <cpuarch.h>


struct string_map_t dep_map = {
	33, {
		{ "eax",        x86_dep_eax },
		{ "ecx",        x86_dep_ecx },
		{ "edx",        x86_dep_edx },
		{ "ebx",        x86_dep_ebx },
		{ "esp",        x86_dep_esp },
		{ "ebp",        x86_dep_ebp },
		{ "esi",        x86_dep_esi },
		{ "edi",        x86_dep_edi },
		
		{ "es",         x86_dep_es },
		{ "cs",         x86_dep_cs },
		{ "ss",         x86_dep_ss },
		{ "ds",         x86_dep_ds },
		{ "fs",         x86_dep_fs },
		{ "gs",         x86_dep_gs },

		{ "zps",        x86_dep_zps },
		{ "of",         x86_dep_of },
		{ "cf",         x86_dep_cf },
		{ "df",         x86_dep_df },

		{ "aux",        x86_dep_aux },
		{ "aux2",       x86_dep_aux2 },

		{ "addr",       x86_dep_ea },
		{ "data",       x86_dep_data },

		{ "st",		x86_dep_st0 },
		{ "st(1)",      x86_dep_st1 },
		{ "st(2)",      x86_dep_st2 },
		{ "st(3)",      x86_dep_st3 },
		{ "st(4)",      x86_dep_st4 },
		{ "st(5)",      x86_dep_st5 },
		{ "st(6)",      x86_dep_st6 },
		{ "st(7)",      x86_dep_st7 },

		{ "fpst",	x86_dep_fpst },
		{ "fpcw",	x86_dep_fpcw },
		{ "fpaux",	x86_dep_fpaux }
	}
};


/* uop_bank: table indexed by 'uop->uop' to obtain the
 * characteristics of an uop */
struct uop_bank_entry_t {
	char *name;
	int fu_class;
	int flags;
};

static struct uop_bank_entry_t uop_bank[uop_count + 1] = {
#define UOP(_uop, _fu_class, _flags) { #_uop, fu_##_fu_class, _flags},
#include "uop1.dat"
#undef UOP
	{"", 0, 0}
};


/* uop_table: table indexed by 'opcode' to obtain the list
 * of uops that correspond to a CISC instruction. */
struct uop_table_entry_t {
	int uop;
	int idep[IDEP_COUNT];
	int odep[ODEP_COUNT];
	struct uop_table_entry_t *next;
};

static struct uop_table_entry_t *uop_table[x86_opcode_count];


static struct repos_t *uop_repos;


static void uop_table_entry_add(enum x86_opcode_t opcode, struct uop_table_entry_t *entry)
{
	struct uop_table_entry_t *last;
	last = uop_table[opcode];
	if (!last) {
		uop_table[opcode] = entry;
		return;
	}
	while (last->next)
		last = last->next;
	last->next = entry;
}


void uop_init()
{
	enum x86_opcode_t opcode;
	struct uop_table_entry_t *entry;
	int i, j;

	uop_repos = repos_create(sizeof(struct uop_t), "uop_repos");

#define X86_INST(_opcode) opcode = op_##_opcode;
#define UOP(_uop, _idep0, _idep1, _idep2, _odep0, _odep1, _odep2, _odep3) \
	entry = calloc(1, sizeof(struct uop_table_entry_t)); \
	entry->uop = uop_##_uop; \
	entry->idep[0] = _idep0; \
	entry->idep[1] = _idep1; \
	entry->idep[2] = _idep2; \
	entry->odep[0] = _odep0; \
	entry->odep[1] = _odep1; \
	entry->odep[2] = _odep2; \
	entry->odep[3] = _odep3; \
	uop_table_entry_add(opcode, entry);
#include "uop2.dat"
#undef X86_INST
#undef UOP
	
	/* Integrity: check that input and output dependences are not
	 * duplicated. This may cause problems in other simulator modules. */
	for (opcode = 0; opcode < x86_opcode_count; opcode++) {
		for (entry = uop_table[opcode]; entry; entry = entry->next) {
			for (i = 0; i < IDEP_COUNT; i++)
				for (j = 0; j < i; j++)
					if (entry->idep[i] && entry->idep[i] == entry->idep[j])
						panic("uop2.dat: opcode=0x%x, repeated input dependence",
							opcode);
			for (i = 0; i < ODEP_COUNT; i++)
				for (j = 0; j < i; j++)
					if (entry->odep[i] && entry->odep[i] == entry->odep[j])
						panic("uop2.dat: opcode=0x%x, repeated output dependence",
							opcode);
		}
	}
}


void uop_done()
{
	enum x86_opcode_t opcode;
	struct uop_table_entry_t *entry;
	for (opcode = 0; opcode < x86_opcode_count; opcode++) {
		while (uop_table[opcode]) {
			entry = uop_table[opcode]->next;
			free(uop_table[opcode]);
			uop_table[opcode] = entry;
		}
	}

	repos_free(uop_repos);
}


static int uop_dep_parse(struct list_t *uop_list, int dep)
{
	/* Regular dependecy */
	if (X86_DEP_IS_VALID(dep))
		return dep;
	
	/* Instruction dependent */
	switch (dep) {

	case x86_dep_none:
	case x86_dep_fpop:
	case x86_dep_fpop2:
	case x86_dep_fpush:
		return dep;

	case x86_dep_rm8:
		return isa_inst.modrm_rm < 4 ? x86_dep_eax + isa_inst.modrm_rm : x86_dep_eax + isa_inst.modrm_rm - 4;

	case x86_dep_rm16:
	case x86_dep_rm32:
		return x86_dep_eax + isa_inst.modrm_rm;

	case x86_dep_r8:
		return isa_inst.reg < 4 ? x86_dep_eax + isa_inst.reg : x86_dep_eax + isa_inst.reg - 4;

	case x86_dep_r16:
	case x86_dep_r32:
		return x86_dep_eax + isa_inst.reg;

	case x86_dep_ir8:
		return isa_inst.opindex < 4 ? x86_dep_eax + isa_inst.opindex : x86_dep_eax + isa_inst.opindex - 4;

	case x86_dep_ir16:
	case x86_dep_ir32:
		return x86_dep_eax + isa_inst.opindex;

	case x86_dep_sreg:
		return x86_dep_es + isa_inst.reg;
	
	case x86_dep_sti:
		return x86_dep_st0 + isa_inst.opindex;

	}

	panic("uop_dep_parse: unknown dep: 0x%x\n", dep);
	return 0;
}


static int uop_idep_parse(struct list_t *uop_list, int dep)
{
	struct uop_t *uop;

	/* Load data from memory first if:
	 *   Dependence is rm8/rm16/rm32 and it is a memory reference.
	 *   Dependence is m8/m16/m32/m64 */
	if (((dep == x86_dep_rm8 || dep == x86_dep_rm16 || dep == x86_dep_rm32) && isa_inst.modrm_mod != 3) ||
		dep == x86_dep_mem32)
	{
		
		/* Compute effective address */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_effaddr;
		uop->idep[0] = isa_inst.segment ? isa_inst.segment - x86_reg_es + x86_dep_es : x86_dep_none;
		uop->idep[1] = isa_inst.ea_base ? isa_inst.ea_base - x86_reg_eax + x86_dep_eax : x86_dep_none;
		uop->idep[2] = isa_inst.ea_index ? isa_inst.ea_index - x86_reg_eax + x86_dep_eax : x86_dep_none;
		uop->odep[0] = x86_dep_ea;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Load */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_load;
		uop->idep[0] = x86_dep_ea;
		uop->odep[0] = x86_dep_data;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Input dependence of instruction is converted into x86_dep_data */
		return x86_dep_data;
	}

	/* Effective address parts */
	if (dep == x86_dep_easeg)
		return isa_inst.segment ? isa_inst.segment - x86_reg_es + x86_dep_es : x86_dep_none;
	if (dep == x86_dep_eabas)
		return isa_inst.ea_base ? isa_inst.ea_base - x86_reg_eax + x86_dep_eax : x86_dep_none;
	if (dep == x86_dep_eaidx)
		return isa_inst.ea_index ? isa_inst.ea_index - x86_reg_eax + x86_dep_eax : x86_dep_none;

	/* Regular dependence */
	return uop_dep_parse(uop_list, dep);
}


static int uop_odep_parse(struct list_t *uop_list, int dep)
{
	struct uop_t *uop;

	/* Insert store uops */
	if (((dep == x86_dep_rm8 || dep == x86_dep_rm16 || dep == x86_dep_rm32) && isa_inst.modrm_mod != 3) ||
		dep == x86_dep_mem32)
	{
		/* Compute effective address.
		 * FIXME: The address computation should be removed if there was a
		 * previous load with the same effective address (e.g. x86_dep_rm32
		 * as source and destination dependence. */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_effaddr;
		uop->idep[0] = isa_inst.segment ? isa_inst.segment - x86_reg_es + x86_dep_es : x86_dep_none;
		uop->idep[1] = isa_inst.ea_base ? isa_inst.ea_base - x86_reg_eax + x86_dep_eax : x86_dep_none;
		uop->idep[2] = isa_inst.ea_index ? isa_inst.ea_index - x86_reg_eax + x86_dep_eax : x86_dep_none;
		uop->odep[0] = x86_dep_ea;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Store */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_store;
		uop->idep[0] = x86_dep_ea;
		uop->idep[1] = x86_dep_data;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Output dependence of instruction is x86_dep_data */
		return x86_dep_data;
	}
	
	/* Regular dependence */
	return uop_dep_parse(uop_list, dep);
}


void uop_free_if_not_queued(struct uop_t *uop)
{
	if (uop->in_fetchq || uop->in_uopq || uop->in_iq ||
		uop->in_lq || uop->in_sq ||
		uop->in_rob || uop->in_eventq)
		return;
	repos_free_object(uop_repos, uop);
}


int uop_exists(struct uop_t *uop)
{
	return repos_allocated_object(uop_repos, uop);
}


void uop_dump_buf(struct uop_t *uop, char *buf, int size)
{
	int i, loreg, count, fp_top_of_stack;
	char *comma;

	dump_buf(&buf, &size, "%s ", uop_bank[uop->uop].name);
	comma = "";
	for (i = count = 0; i < IDEP_COUNT; i++) {
		if (!X86_DEP_IS_VALID(uop->idep[i]))
			continue;
		dump_buf(&buf, &size, "%s%s", comma,
			map_value(&dep_map, uop->idep[i]));
		comma = ",";
		count++;
	}
	if (!count)
		dump_buf(&buf, &size, "-");
	dump_buf(&buf, &size, "/");
	comma = "";
	fp_top_of_stack = 0;
	for (i = count = 0; i < ODEP_COUNT; i++) {
		loreg = uop->odep[i];
		if (loreg == x86_dep_fpop)
			fp_top_of_stack--;
		else if (loreg == x86_dep_fpop2)
			fp_top_of_stack -= 2;
		else if (loreg == x86_dep_fpush)
			fp_top_of_stack++;
		if (!X86_DEP_IS_VALID(loreg))
			continue;
		dump_buf(&buf, &size, "%s%s", comma, map_value(&dep_map, loreg));
		comma = ",";
		count++;
	}
	if (!count)
		dump_buf(&buf, &size, "-");
	if (fp_top_of_stack)
		dump_buf(&buf, &size, "(st=>%+d)", fp_top_of_stack);
}


void uop_dump(struct uop_t *uop, FILE *f)
{
	char buf[100];
	uop_dump_buf(uop, buf, sizeof(buf));
	fprintf(f, "%s", buf);
}


void uop_list_dump(struct list_t *uop_list, FILE *f)
{
	struct uop_t *uop;
	int i;
	
	for (i = 0; i < list_count(uop_list); i++) {
		uop = list_get(uop_list, i);
		fprintf(f, "%3d. ", i);
		uop_dump(uop, f);
		fprintf(f, "\n");
	}
}


void uop_lnlist_dump(struct lnlist_t *uop_list, FILE *f)
{
	struct uop_t *uop;
	
	lnlist_head(uop_list);
	while (!lnlist_eol(uop_list)) {
		uop = lnlist_get(uop_list);
		fprintf(f, "%3d. ", lnlist_current(uop_list));
		uop_dump(uop, f);
		fprintf(f, "\n");
		lnlist_next(uop_list);
	}
}


/* Update 'uop->ready' field of all instructions in a list as per the result
 * obtained by 'rf_ready'. The 'uop->ready' field is redundant and should always
 * match the return value of 'rf_ready' while an uop is in the ROB.
 * A debug message is dumped when the uop transitions to ready. */
void uop_lnlist_check_if_ready(struct lnlist_t *uop_list)
{
	struct uop_t *uop;
	lnlist_head(uop_list);
	for (lnlist_head(uop_list); !lnlist_eol(uop_list); lnlist_next(uop_list)) {
		uop = lnlist_get(uop_list);
		if (uop->ready || !rf_ready(uop))
			continue;
		uop->ready = 1;
		esim_debug("uop action=\"update\", core=%d, seq=%lld, ready=1\n",
			uop->core, (long long) uop->di_seq);
	}
}


/* Decode the macroinstruction currently stored in 'isa_inst', and insert
 * uops into 'list'. If any decoded microinstruction is a control instruction,
 * return it, otherwise return the first decoded uop. */
struct uop_t *uop_decode(struct list_t *list)
{
	struct uop_table_entry_t *entry;
	struct uop_t *uop, *ret = NULL;
	int count, i;

	count = list_count(list);
	for (entry = uop_table[isa_inst.opcode]; entry; entry = entry->next) {
		
		/* Create uop. No more uops allowed if a control uop was found. */
		if (ret)
			panic("uop_decode: no uop allowed after control uop");
		uop = repos_create_object(uop_repos);

		/* Input dependencies, maybe add 'load' uops to the list */
		for (i = 0; i < IDEP_COUNT; i++)
			uop->idep[i] = uop_idep_parse(list, entry->idep[i]);

		/* Insert uop */
		list_add(list, uop);

		/* Output dependencies, maybe add 'store' uops to the list */
		for (i = 0; i < ODEP_COUNT; i++)
			uop->odep[i] = uop_odep_parse(list, entry->odep[i]);

		/* Rest */
		uop->uop = entry->uop;
		uop->fu_class = uop_bank[entry->uop].fu_class;
		uop->flags = uop_bank[entry->uop].flags;
		if (uop->flags & FCTRL)
			ret = uop;
	}

	/* Return last conditional uop, or first new uop of the list. */
	return ret ? ret : list_get(list, count);
}

