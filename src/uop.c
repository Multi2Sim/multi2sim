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
	22, {
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

		{ "ZF-PF-SF",   DZPS },
		{ "OF",         DOF },
		{ "CF",         DCF },

		{ "aux",        DAUX },
		{ "aux2",       DAUX2 },

		{ "addr",       DEA },
		{ "data",       DDATA },

		{ "fp",		DFP }
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

	uop_repos = repos_create(sizeof(struct uop_t), "uop_repos");

#define X86_INST(_opcode) opcode = op_##_opcode;
#define UOP(_uop, _idep0, _idep1, _idep2, _odep0, _odep1, _odep2, _odep3, _odep4) \
	entry = calloc(1, sizeof(struct uop_table_entry_t)); \
	entry->uop = uop_##_uop; \
	entry->idep[0] = _idep0; \
	entry->idep[1] = _idep1; \
	entry->idep[2] = _idep2; \
	entry->odep[0] = _odep0; \
	entry->odep[1] = _odep1; \
	entry->odep[2] = _odep2; \
	entry->odep[3] = _odep3; \
	entry->odep[4] = _odep4; \
	uop_table_entry_add(opcode, entry);
#include "uop2.dat"
#undef X86_INST
#undef UOP
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


static int uop_dep_parse(x86_inst_t *inst, struct list_t *uop_list, int dep)
{
	/* Regular dependecy */
	if (!dep || (dep >= DFIRST && dep <= DLAST))
		return dep;
	
	/* Instruction dependent */
	switch (dep) {
	case DRM8:
		return inst->modrm_rm < 4 ? DEAX + inst->modrm_rm : DEAX + inst->modrm_rm - 4;
	case DRM16: case DRM32:
		return DEAX + inst->modrm_rm;
	case DR8:
		return inst->reg < 4 ? DEAX + inst->reg : DEAX + inst->reg - 4;
	case DR16: case DR32:
		return DEAX + inst->reg;
	case DIR8:
		return inst->opindex < 4 ? DEAX + inst->opindex : DEAX + inst->opindex - 4;
	case DIR16: case DIR32:
		return DEAX + inst->opindex;
	case DSREG:
		return DES + inst->reg;
	}

	panic("uop_dep_parse: unknown dep: 0x%x\n", dep);
	return 0;
}


static int uop_idep_parse(x86_inst_t *inst, struct list_t *uop_list, int dep)
{
	struct uop_t *uop;

	/* Load data from memory first if:
	 *   Dependence is rm8/rm16/rm32 and it is a memory reference.
	 *   Dependence is m8/m16/m32/m64 */
	if (((dep == DRM8 || dep == DRM16 || dep == DRM32) && inst->modrm_mod != 3) ||
		dep == DMEM)
	{
		
		/* Compute effective address */
		uop = repos_create_object(uop_repos);
		uop->inst = inst;
		uop->uop = uop_effaddr;
		uop->idep[0] = inst->segment ? inst->segment - reg_es + DES : DNONE;
		uop->idep[1] = inst->ea_base ? inst->ea_base - reg_eax + DEAX : DNONE;
		uop->idep[2] = inst->ea_index ? inst->ea_index - reg_eax + DEAX : DNONE;
		uop->odep[0] = DEA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Load */
		uop = repos_create_object(uop_repos);
		uop->inst = inst;
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
		return inst->segment ? inst->segment - reg_es + DES : DNONE;
	if (dep == DEABAS)
		return inst->ea_base ? inst->ea_base - reg_eax + DEAX : DNONE;
	if (dep == DEAIDX)
		return inst->ea_index ? inst->ea_index - reg_eax + DEAX : DNONE;

	/* Regular dependence */
	return uop_dep_parse(inst, uop_list, dep);
}


static int uop_odep_parse(x86_inst_t *inst, struct list_t *uop_list, int dep)
{
	struct uop_t *uop;

	/* Insert store uops */
	if (((dep == DRM8 || dep == DRM16 || dep == DRM32) && inst->modrm_mod != 3) ||
		dep == DMEM)
	{
		/* Compute effective address.
		 * FIXME: The address computation should be removed if there was a
		 * previous load with the same effective address (e.g. DRM32
		 * as source and destination dependence. */
		uop = repos_create_object(uop_repos);
		uop->inst = inst;
		uop->uop = uop_effaddr;
		uop->idep[0] = inst->segment ? inst->segment - reg_es + DES : DNONE;
		uop->idep[1] = inst->ea_base ? inst->ea_base - reg_eax + DEAX : DNONE;
		uop->idep[2] = inst->ea_index ? inst->ea_index - reg_eax + DEAX : DNONE;
		uop->odep[0] = DEA;
		uop->fu_class = uop_bank[uop->uop].fu_class;
		uop->flags = uop_bank[uop->uop].flags;
		list_add(uop_list, uop);

		/* Store */
		uop = repos_create_object(uop_repos);
		uop->inst = inst;
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
	return uop_dep_parse(inst, uop_list, dep);
}


void uop_free_if_not_queued(struct uop_t *uop)
{
	if (uop->in_fetchq || uop->in_iq ||
		uop->in_lq || uop->in_sq ||
		uop->in_rob || uop->in_eventq)
		return;
	repos_free_object(uop_repos, uop);
}


int uop_exists(struct uop_t *uop)
{
	return repos_allocated_object(uop_repos, uop);
}


void uop_dump(struct uop_t *uop, FILE *f)
{
	int i;
	char *comma;
	
	fprintf(f, "%s ", uop_bank[uop->uop].name);
	comma = "";
	for (i = 0; i < IDEP_COUNT; i++) {
		if (!uop->idep[i])
			continue;
		fprintf(f, "%s%s", comma, map_value(&dep_map, uop->idep[i]));
		comma = ",";
	}
	fprintf(f, "/");
	comma = "";
	for (i = 0; i < ODEP_COUNT; i++) {
		if (!uop->odep[i])
			continue;
		fprintf(f, "%s%s", comma, map_value(&dep_map, uop->odep[i]));
		comma = ",";
	}
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


void uop_decode(x86_inst_t *inst, struct list_t *uop_list)
{
	struct uop_table_entry_t *entry;
	struct uop_t *uop;
	int i, count = 0;

	for (entry = uop_table[inst->opcode]; entry; entry = entry->next) {
		
		/* Create uop */
		uop = repos_create_object(uop_repos);
		uop->inst = inst;

		/* Input dependencies, maybe add 'load' uops to the list */
		for (i = 0; i < IDEP_COUNT; i++)
			uop->idep[i] = uop_idep_parse(inst, uop_list, entry->idep[i]);

		/* Insert uop */
		list_add(uop_list, uop);

		/* Output dependencies, maybe add 'store' uops to the list */
		for (i = 0; i < ODEP_COUNT; i++)
			uop->odep[i] = uop_odep_parse(inst, uop_list, entry->odep[i]);

		/* Rest */
		uop->uop = entry->uop;
		uop->fu_class = uop_bank[entry->uop].fu_class;
		uop->flags = uop_bank[entry->uop].flags;
		count++;
	}
}


void uop_pdg_recover(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	if (uop->lmpred_miss) {
		uop->lmpred_miss = 0;
		THREAD.lmpred_misses--;
		assert(THREAD.lmpred_misses >= 0);
	}
}

