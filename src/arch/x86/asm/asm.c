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

#include <assert.h>

#include <arch/common/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"


/*
 * Variables
 */

/* Global unique x86 disassembler */
struct x86_asm_t *x86_asm;


/*** Constants used in 'asm.dat' ***/

/* For fields 'op1', 'op2', 'modrm', 'imm' */
#define SKIP  0x0100

/* For field 'modrm' */
#define REG   0x0200
#define MEM   0x0400

/* For fields 'op1', 'op2' */
#define INDEX 0x1000

/* For 'imm' field */
#define IB    0x2000  /* for imm */
#define IW    0x4000  /* for imm */
#define ID    0x8000  /* for imm */


/* List of possible prefixes */
static unsigned char x86_asm_prefixes[] =
{
	0xf0,  /* lock */
	0xf2,  /* repnz */
	0xf3,  /* rep */
	0x66,  /* op */
	0x67,  /* addr */
	0x2e,  /* use cs */
	0x36,  /* use ss */
	0x3e,  /* use ds */
	0x26,  /* use es */
	0x64,  /* use fs */
	0x65   /* use gs */
};


static void x86_asm_inst_info_insert_at(struct x86_inst_info_elem_t **table,
	struct x86_inst_info_elem_t *elem, int at)
{
	struct x86_inst_info_elem_t *prev;

	/* First entry */
	if (!table[at]) {
		table[at] = elem;
		return;
	}

	/* Go to the end of the list */
	prev = table[at];
	while (prev->next)
		prev = prev->next;
	prev->next = elem;
}


static void x86_asm_inst_info_insert(struct x86_asm_t *as,
		struct x86_inst_info_t *info)
{
	struct x86_inst_info_elem_t *elem;
	struct x86_inst_info_elem_t **table;

	int index;
	int count;
	int i;

	/* Obtain the table where to insert, the initial index, and
	 * the number of times we must insert the instruction. */
	if ((info->op1 & 0xff) == 0x0f)
	{
		table = as->inst_info_table_0f;
		index = info->op2 & 0xff;
		count = info->op2 & INDEX ? 8 : 1;
	}
	else
	{
		table = as->inst_info_table;
		index = info->op1 & 0xff;
		count = info->op1 & INDEX ? 8 : 1;
	}

	/* Insert */
	for (i = 0; i < count; i++)
	{
		elem = xcalloc(1, sizeof(struct x86_inst_info_elem_t));
		elem->info = info;
		x86_asm_inst_info_insert_at(table, elem, index + i);
	}
}


static void x86_asm_inst_info_elem_free_list(struct x86_inst_info_elem_t *elem)
{
	struct x86_inst_info_elem_t *next;

	while (elem)
	{
		next = elem->next;
		free(elem);
		elem = next;
	}
}




/*
 * Public Functions
 */

struct x86_asm_t *x86_asm_create(void)
{
	struct x86_asm_t *as;
	struct asm_t *__as;

	struct x86_inst_info_t *info;

	int op;
	int i;

	/* Create parent */
	__as = asm_create();

	/* Initialize */
	as = xcalloc(1, sizeof(struct x86_asm_t));

	/* Initialize instruction information list */
	as->inst_info_list = xcalloc(x86_inst_opcode_count, sizeof(struct x86_inst_info_t));
#define DEFINST(__name, __op1, __op2, __op3, __modrm, __imm, __prefixes) \
	info = &as->inst_info_list[x86_inst_##__name]; \
	info->opcode = x86_inst_##__name; \
	info->op1 = __op1; \
	info->op2 = __op2; \
	info->op3 = __op3; \
	info->modrm = __modrm; \
	info->imm = __imm; \
	info->prefixes = __prefixes; \
	info->fmt = #__name;
#include "asm.dat"
#undef DEFINST

	/* Initialize table of prefixes */
	for (i = 0; i < sizeof(x86_asm_prefixes); i++)
		as->is_prefix[x86_asm_prefixes[i]] = 1;

	/* Initialize x86_opcode_info_table. This table contains lists of
	 * information about machine instructions. To find an instruction
	 * in the table, it can be indexed by the first byte of its opcode. */
	for (op = 1; op < x86_inst_opcode_count; op++)
	{
		/* Insert into table */
		info = &as->inst_info_list[op];
		x86_asm_inst_info_insert(as, info);

		/* Compute 'match_mask' and 'mach_result' fields. Start with
		 * the 'modrm' field in the instruction format definition. */
		if (!(info->modrm & SKIP))
		{
			info->modrm_size = 1;

			/* If part of the offset is in the 'reg' field of the ModR/M byte,
			 * it must be matched. */
			if (!(info->modrm & REG))
			{
				info->match_mask = 0x38;
				info->match_result = (info->modrm & 0x7) << 3;
			}

			/* If instruction expects a memory operand, the 'mod' field of 
			 * the ModR/M byte cannot be 11. */
			if (info->modrm & MEM)
			{
				info->nomatch_mask = 0xc0;
				info->nomatch_result = 0xc0;
			}
		}

		/* Third opcode byte */
		if (!(info->op3 & SKIP))
		{
			info->opcode_size++;
			info->match_mask <<= 8;
			info->match_result <<= 8;
			info->nomatch_mask <<= 8;
			info->nomatch_result <<= 8;
			info->match_mask |= 0xff;
			info->match_result |= info->op3 & 0xff;
			assert(!(info->op3 & INDEX));
		}

		/* Second opcode byte */
		if (!(info->op2 & SKIP))
		{
			info->opcode_size++;
			info->match_mask <<= 8;
			info->match_result <<= 8;
			info->nomatch_mask <<= 8;
			info->nomatch_result <<= 8;
			info->match_mask |= 0xff;
			info->match_result |= info->op2 & 0xff;

			/* The opcode has an index */
			if (info->op2 & INDEX) {
				info->match_mask &= 0xfffffff8;
				info->opindex_shift = 8;
			}
		}

		/* First opcode byte (always there) */
		info->opcode_size++;
		info->match_mask <<= 8;
		info->match_result <<= 8;
		info->nomatch_mask <<= 8;
		info->nomatch_result <<= 8;
		info->match_mask |= 0xff;
		info->match_result |= info->op1 & 0xff;
		if (info->op1 & INDEX)
		{
			info->match_mask &= 0xfffffff8;
			info->opindex_shift = 0;
		}

		/* Immediate size */
		if (info->imm & IB)
			info->imm_size = 1;
		if (info->imm & IW)
			info->imm_size = 2;
		if (info->imm & ID)
			info->imm_size = 4;
	}
	/* Class information */
	CLASS_INIT(as, X86_ASM_TYPE, __as);
	__as->free = x86_asm_free;

	/* Return */
	return as;
}


void x86_asm_free(struct asm_t *__as)
{
	struct x86_asm_t *as;
	int i;

	/* Get class instance */
	as = X86_ASM(__as);

	/* Free parent */
	asm_free(__as);

	/* Free instruction info tables */
	for (i = 0; i < 0x100; i++)
	{
		x86_asm_inst_info_elem_free_list(as->inst_info_table[i]);
		x86_asm_inst_info_elem_free_list(as->inst_info_table_0f[i]);
	}
	
	/* Free */
	free(as->inst_info_list);
	free(as);
}




/*
 * Static Public Functions
 * (not related with 'x86_asm_t' class)
 */

void x86_asm_init(void)
{
	/* Host restrictions */
	M2S_HOST_GUEST_MATCH(sizeof(union x86_inst_xmm_reg_t), 16);

	/* Create disassembler */
	assert(!x86_asm);
	x86_asm = x86_asm_create();

}


void x86_asm_done(void)
{
	x86_asm_free(ASM(x86_asm));
}


void x86_asm_disassemble_binary(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	struct elf_buffer_t *buffer;
	struct elf_symbol_t *symbol;

	struct x86_inst_t inst;
	int curr_sym;
	int i;

	/* Open ELF file */
	x86_asm_init();
	elf_file = elf_file_create_from_path(path);

	/* Read sections */
	for (i = 0; i < list_count(elf_file->section_list); i++)
	{
		/* Get section and skip if it does not contain code */
		section = list_get(elf_file->section_list, i);
		if (!(section->header->sh_flags & SHF_EXECINSTR))
			continue;
		buffer = &section->buffer;

		/* Title */
		printf("**\n** Disassembly for section '%s'\n**\n\n", section->name);

		/* Disassemble */
		curr_sym = 0;
		symbol = list_get(elf_file->symbol_table, curr_sym);
		while (buffer->pos < buffer->size)
		{
			unsigned int eip;
			char str[MAX_STRING_SIZE];

			/* Read instruction */
			eip = section->header->sh_addr + buffer->pos;
			x86_inst_decode(&inst, eip, elf_buffer_tell(buffer));
			if (inst.size)
			{
				elf_buffer_read(buffer, NULL, inst.size);
				x86_inst_dump_buf(&inst, str, MAX_STRING_SIZE);
			}
			else
			{
				elf_buffer_read(buffer, NULL, 1);
				strcpy(str, "???");
			}

			/* Symbol */
			while (symbol && symbol->value < eip)
			{
				curr_sym++;
				symbol = list_get(elf_file->symbol_table, curr_sym);
			}
			if (symbol && symbol->value == eip)
				printf("\n%08x <%s>:\n", eip, symbol->name);

			/* Print */
			printf("%8x:  %s\n", eip, str);
		}

		/* Pad */
		printf("\n\n");
	}

	/* Free ELF */
	elf_file_free(elf_file);
	x86_asm_done();

	/* End */
	mhandle_done();
	exit(0);
}
