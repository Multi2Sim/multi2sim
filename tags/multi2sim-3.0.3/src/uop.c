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

#include <m2s.h>


struct string_map_t dep_map = {
	33, {
		{ "eax",        DEAX },
		{ "ecx",        DECX },
		{ "edx",        DEDX },
		{ "ebx",        DEBX },
		{ "esp",        DESP },
		{ "ebp",        DEBP },
		{ "esi",        DESI },
		{ "edi",        DEDI },
		
		{ "es",         DES },
		{ "cs",         DCS },
		{ "ss",         DSS },
		{ "ds",         DDS },
		{ "fs",         DFS },
		{ "gs",         DGS },

		{ "zps",        DZPS },
		{ "of",         DOF },
		{ "cf",         DCF },
		{ "df",         DDF },

		{ "aux",        DAUX },
		{ "aux2",       DAUX2 },

		{ "addr",       DEA },
		{ "data",       DDATA },

		{ "st",		DST0 },
		{ "st(1)",      DST1 },
		{ "st(2)",      DST2 },
		{ "st(3)",      DST3 },
		{ "st(4)",      DST4 },
		{ "st(5)",      DST5 },
		{ "st(6)",      DST6 },
		{ "st(7)",      DST7 },

		{ "fpst",	DFPST },
		{ "fpcw",	DFPCW },
		{ "fpaux",	DFPAUX }
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


static void uop_table_entry_add(x86_opcode_t opcode,
	struct uop_table_entry_t *entry)
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
	x86_opcode_t opcode;
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
	x86_opcode_t opcode;
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
	if (DEP_IS_VALID(dep))
		return dep;
	
	/* Instruction dependent */
	switch (dep) {

	case DNONE:
	case DFPOP:
	case DFPOP2:
	case DFPUSH:
		return dep;

	case DRM8:
		return isa_inst.modrm_rm < 4 ? DEAX + isa_inst.modrm_rm : DEAX + isa_inst.modrm_rm - 4;

	case DRM16:
	case DRM32:
		return DEAX + isa_inst.modrm_rm;

	case DR8:
		return isa_inst.reg < 4 ? DEAX + isa_inst.reg : DEAX + isa_inst.reg - 4;

	case DR16:
	case DR32:
		return DEAX + isa_inst.reg;

	case DIR8:
		return isa_inst.opindex < 4 ? DEAX + isa_inst.opindex : DEAX + isa_inst.opindex - 4;

	case DIR16:
	case DIR32:
		return DEAX + isa_inst.opindex;

	case DSREG:
		return DES + isa_inst.reg;
	
	case DSTI:
		return DST0 + isa_inst.opindex;

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
	if (((dep == DRM8 || dep == DRM16 || dep == DRM32) && isa_inst.modrm_mod != 3) ||
		dep == DMEM)
	{
		
		/* Compute effective address */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_effaddr;
		uop->idep[0] = isa_inst.segment ? isa_inst.segment - reg_es + DES : DNONE;
		uop->idep[1] = isa_inst.ea_base ? isa_inst.ea_base - reg_eax + DEAX : DNONE;
		uop->idep[2] = isa_inst.ea_index ? isa_inst.ea_index - reg_eax + DEAX : DNONE;
		uop->odep[0] = DEA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Load */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_load;
		uop->idep[0] = DEA;
		uop->odep[0] = DDATA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Input dependence of instruction is converted into DDATA */
		return DDATA;
	}

	/* Effective address parts */
	if (dep == DEASEG)
		return isa_inst.segment ? isa_inst.segment - reg_es + DES : DNONE;
	if (dep == DEABAS)
		return isa_inst.ea_base ? isa_inst.ea_base - reg_eax + DEAX : DNONE;
	if (dep == DEAIDX)
		return isa_inst.ea_index ? isa_inst.ea_index - reg_eax + DEAX : DNONE;

	/* Regular dependence */
	return uop_dep_parse(uop_list, dep);
}


static int uop_odep_parse(struct list_t *uop_list, int dep)
{
	struct uop_t *uop;

	/* Insert store uops */
	if (((dep == DRM8 || dep == DRM16 || dep == DRM32) && isa_inst.modrm_mod != 3) ||
		dep == DMEM)
	{
		/* Compute effective address.
		 * FIXME: The address computation should be removed if there was a
		 * previous load with the same effective address (e.g. DRM32
		 * as source and destination dependence. */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_effaddr;
		uop->idep[0] = isa_inst.segment ? isa_inst.segment - reg_es + DES : DNONE;
		uop->idep[1] = isa_inst.ea_base ? isa_inst.ea_base - reg_eax + DEAX : DNONE;
		uop->idep[2] = isa_inst.ea_index ? isa_inst.ea_index - reg_eax + DEAX : DNONE;
		uop->odep[0] = DEA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Store */
		uop = repos_create_object(uop_repos);
		uop->uop = uop_store;
		uop->idep[0] = DEA;
		uop->idep[1] = DDATA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Output dependence of instruction is DDATA */
		return DDATA;
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
		if (!DEP_IS_VALID(uop->idep[i]))
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
		if (loreg == DFPOP)
			fp_top_of_stack--;
		else if (loreg == DFPOP2)
			fp_top_of_stack -= 2;
		else if (loreg == DFPUSH)
			fp_top_of_stack++;
		if (!DEP_IS_VALID(loreg))
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

