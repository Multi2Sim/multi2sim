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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"


/* Flags used in asm.dat */
#define SKIP  0x0100  /* for op1, op2, modrm, imm */
#define REG   0x0200  /* for modrm */
#define MEM   0x0400  /* for modrm */
#define INDEX 0x1000  /* for op1, op2 */
#define IB    0x2000  /* for imm */
#define IW    0x4000  /* for imm */
#define ID    0x8000  /* for imm */


/* This struct contains information derived from asm.dat, which
 * is initialized in disasm_init, to form linked lists in the table
 * x86_opcode_info_table and a single list in x86_opcode_info_list. */
struct x86_opcode_info_t
{
	/* Obtained from asm.dat */
	enum x86_opcode_t opcode;
	uint32_t op1, op2, op3, modrm, imm;
	int prefixes;  /* Mask of prefixes of type 'enum x86_prefix_enum' */
	char *fmt;

	/* Derived fields */
	uint32_t match_mask, match_result;
	uint32_t nomatch_mask, nomatch_result;
	int opindex_shift;  /* pos to shift inst to obtain index of op1/op2 if any */
	int impl_reg;  /* implied register in op1 (0-7) */
	int opcode_size;  /* size of opcode (1 or 2), not counting the modrm part. */
	int modrm_size;  /* size of modrm field (0 or 1) */
	int imm_size;  /* Immediate size (0, 1, 2, or 4) */
};


/* Containers for opcode infos. We need this because an info can belong to
 * different lists when there are registers embedded in the opcodes. */
struct x86_opcode_info_elem_t
{
	struct x86_opcode_info_t *info;
	struct x86_opcode_info_elem_t *next;
};


/* Table for fast access of instruction data, indexed by the instruction opcode.
 * The second table contains instructions whose first opcode byte is 0x0f, and
 * is indexed by the second byte of its opcode. */
static struct x86_opcode_info_elem_t *x86_opcode_info_table[0x100];
static struct x86_opcode_info_elem_t *x86_opcode_info_table_0f[0x100];

/* List of possible prefixes */
static unsigned char x86_prefixes[] = {
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
static unsigned char x86_byte_is_prefix[256];


/* List of instructions. */
static struct x86_opcode_info_t x86_opcode_info_list[x86_opcode_count] =
{
	{ x86_op_none, 0, 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, 0, 0, 0, 0 }

#define DEFINST(name,op1,op2,op3,modrm,imm,pfx) \
, { op_##name, op1, op2, op3, modrm, imm, pfx, #name, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#include "asm.dat"
#undef DEFINST

};


/* Register names */
char *x86_reg_name[x86_reg_count] =
{
	"",

	"eax",
	"ecx",
	"edx",
	"ebx",
	"esp",
	"ebp",
	"esi",
	"edi",

	"ax",
	"cx",
	"dx",
	"bx",
	"sp",
	"bp",
	"si",
	"di",

	"al",
	"cl",
	"dl",
	"bl",
	"ah",
	"ch",
	"dh",
	"bh",

	"es",
	"cs",
	"ss",
	"ds",
	"fs",
	"gs"
};


/* Table indexed by pairs ModRM.mod and ModRM.rm, containing
 * information about what will come next and effective address
 * computation. */
struct x86_modrm_table_entry_t {
	enum x86_reg_t ea_base;
	int disp_size;
	int sib_size;
};


static struct x86_modrm_table_entry_t modrm_table[32] =
{
	{x86_reg_eax, 0, 0},
	{x86_reg_ecx, 0, 0},
	{x86_reg_edx, 0, 0},
	{x86_reg_ebx, 0, 0},
	{x86_reg_none, 0, 1},
	{x86_reg_none, 4, 0},
	{x86_reg_esi, 0, 0},
	{x86_reg_edi, 0, 0},

	{x86_reg_eax, 1, 0},
	{x86_reg_ecx, 1, 0},
	{x86_reg_edx, 1, 0},
	{x86_reg_ebx, 1, 0},
	{x86_reg_none, 1, 1},
	{x86_reg_ebp, 1, 0},
	{x86_reg_esi, 1, 0},
	{x86_reg_edi, 1, 0},

	{x86_reg_eax, 4, 0},
	{x86_reg_ecx, 4, 0},
	{x86_reg_edx, 4, 0},
	{x86_reg_ebx, 4, 0},
	{x86_reg_none, 4, 1},
	{x86_reg_ebp, 4, 0},
	{x86_reg_esi, 4, 0},
	{x86_reg_edi, 4, 0},

	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0},
	{x86_reg_none, 0, 0}
};


/* Table to obtain the scale
 * from its decoded value */
uint32_t ea_scale_table[4] = { 1, 2, 4, 8};


static void x86_opcode_info_insert_at(struct x86_opcode_info_elem_t **table,
	struct x86_opcode_info_elem_t *elem, int at)
{
	struct x86_opcode_info_elem_t *prev;

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


static void x86_opcode_info_insert(struct x86_opcode_info_t *info)
{
	struct x86_opcode_info_elem_t *elem;
	struct x86_opcode_info_elem_t **table;
	int index, i, count;

	/* Obtain the table where to insert, the initial index, and
	 * the number of times we must insert the instruction. */
	if ((info->op1 & 0xff) == 0x0f) {
		table = x86_opcode_info_table_0f;
		index = info->op2 & 0xff;
		count = info->op2 & INDEX ? 8 : 1;
	} else {
		table = x86_opcode_info_table;
		index = info->op1 & 0xff;
		count = info->op1 & INDEX ? 8 : 1;
	}

	/* Insert */
	for (i = 0; i < count; i++) {
		elem = xcalloc(1, sizeof(struct x86_opcode_info_elem_t));
		elem->info = info;
		x86_opcode_info_insert_at(table, elem, index + i);
	}
}


static void x86_opcode_info_elem_free_list(struct x86_opcode_info_elem_t *elem)
{
	struct x86_opcode_info_elem_t *next;
	while (elem) {
		next = elem->next;
		free(elem);
		elem = next;
	}
}


static char *x86_reg_name_get(enum x86_reg_t reg)
{
	if (reg >= 0 && reg <= x86_reg_count)
		return x86_reg_name[reg];
	else
		return "(inv-reg)";
}


static int is_fmt_char(char c)
{
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9');
}


static int is_next_word(char *src, char *word)
{
	int len = strlen(word);
	if (strlen(src) < len)
		return 0;
	if (strncmp(src, word, len))
		return 0;
	if (is_fmt_char(src[len]))
		return 0;
	return 1;
}


static void x86_moffs_address_dump_buf(struct x86_inst_t *inst, char **pbuf, int *psize)
{
	enum x86_reg_t reg;

	reg = inst->segment ? inst->segment : x86_reg_ds;
	str_printf(pbuf, psize, "%s:0x%x", x86_reg_name_get(reg), inst->imm.d);
}


static void x86_memory_address_dump_buf(struct x86_inst_t *inst, char **pbuf, int *psize)
{
	int putsign = 0;
	char seg[20];
	assert(inst->modrm_mod != 0x03);

	/* Segment */
	seg[0] = 0;
	if (inst->segment) {
		assert(inst->segment >= 0 && inst->segment < x86_reg_count);
		strcpy(seg, x86_reg_name_get(inst->segment));
		strcat(seg, ":");
	}

	/* When there is only a displacement */
	if (!inst->ea_base && !inst->ea_index) {
		if (!seg[0])
			strcpy(seg, "ds:");
		str_printf(pbuf, psize, "%s0x%x", seg, inst->disp);
		return;
	}

	str_printf(pbuf, psize, "%s[", seg);
	if (inst->ea_base) {
		str_printf(pbuf, psize, "%s", x86_reg_name_get(inst->ea_base));
		putsign = 1;
	}
	if (inst->ea_index) {
		str_printf(pbuf, psize, "%s%s", putsign ? "+" : "",
			x86_reg_name_get(inst->ea_index));
		if (inst->ea_scale > 1)
			str_printf(pbuf, psize, "*%d", inst->ea_scale);
		putsign = 1;
	}
	if (inst->disp > 0)
		str_printf(pbuf, psize, "%s0x%x", putsign ? "+" : "", inst->disp);
	if (inst->disp < 0)
		str_printf(pbuf, psize, "-0x%x", -inst->disp);
	str_printf(pbuf, psize, "]");
}






/*
 * Public Functions
 */

void x86_disasm_init()
{
	enum x86_opcode_t op;
	struct x86_opcode_info_t *info;
	int i;

	/* Host restrictions */
	M2S_HOST_GUEST_MATCH(sizeof(union x86_xmm_reg_t), 16);

	/* Initialize table of prefixes */
	for (i = 0; i < sizeof(x86_prefixes); i++)
		x86_byte_is_prefix[x86_prefixes[i]] = 1;

	/* Initialize x86_opcode_info_table. This table contains lists of
	 * information about machine instructions. To find an instruction
	 * in the table, it can be indexed by the first byte of its opcode. */
	for (op = 1; op < x86_opcode_count; op++)
	{
		/* Insert into table */
		info = &x86_opcode_info_list[op];
		x86_opcode_info_insert(info);

		/* Compute match_mask and mach_result fields. Start with
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
}


void x86_disasm_done()
{
	int i;
	for (i = 0; i < 0x100; i++)
	{
		x86_opcode_info_elem_free_list(x86_opcode_info_table[i]);
		x86_opcode_info_elem_free_list(x86_opcode_info_table_0f[i]);
	}
}


/* Pointer to 'inst' is declared volatile to avoid optimizations when calling 'memset' */
void x86_disasm(void *buf, uint32_t eip, volatile struct x86_inst_t *inst)
{
	struct x86_opcode_info_elem_t **table, *elem;
	struct x86_opcode_info_t *info;
	int index;
	uint32_t buf32;
	struct x86_modrm_table_entry_t *modrm_table_entry;

	/* Initialize instruction */
	memset((void *) inst, 0, sizeof(struct x86_inst_t));
	inst->eip = eip;
	inst->op_size = 4;
	inst->addr_size = 4;

	/* Prefixes */
	while (x86_byte_is_prefix[* (unsigned char *) buf])
	{
		switch (* (unsigned char *) buf)
		{

		case 0xf0:
			/* lock prefix is ignored */
			break;

		case 0xf2:
			inst->prefixes |= x86_prefix_repnz;
			break;

		case 0xf3:
			inst->prefixes |= x86_prefix_rep;
			break;

		case 0x66:
			inst->prefixes |= x86_prefix_op;
			inst->op_size = 2;
			break;

		case 0x67:
			inst->prefixes |= x86_prefix_addr;
			inst->addr_size = 2;
			break;

		case 0x2e:
			inst->segment = x86_reg_cs;
			break;

		case 0x36:
			inst->segment = x86_reg_ss;
			break;

		case 0x3e:
			inst->segment = x86_reg_ds;
			break;

		case 0x26:
			inst->segment = x86_reg_es;
			break;

		case 0x64:
			inst->segment = x86_reg_fs;
			break;

		case 0x65:
			inst->segment = x86_reg_gs;
			break;

		default:
			panic("%s: invalid prefix", __FUNCTION__);

		}

		/* One more prefix */
		buf++;
		inst->prefix_size++;
	}

	/* Find instruction */
	buf32 = * (uint32_t *) buf;
	inst->opcode = x86_op_none;
	table = * (unsigned char *) buf == 0x0f ? x86_opcode_info_table_0f : x86_opcode_info_table;
	index = * (unsigned char *) buf == 0x0f ? * (unsigned char *) (buf + 1): * (unsigned char *) buf;
	for (elem = table[index]; elem; elem = elem->next)
	{
		info = elem->info;
		if (info->nomatch_mask && (buf32 & info->nomatch_mask) ==
			info->nomatch_result)
			continue;
		if ((buf32 & info->match_mask) == info->match_result
			&& info->prefixes == inst->prefixes)
			break;
	}

	/* Instruction not implemented */
	if (!elem)
		return;
	
	/* Instruction found */
	inst->format = info->fmt;
	inst->opcode = info->opcode;
	inst->opcode_size = info->opcode_size;
	inst->modrm_size = info->modrm_size;
	inst->opindex = (buf32 >> info->opindex_shift) & 0x7;
	buf += inst->opcode_size;  /* Skip opcode */

	/* Decode the ModR/M field */
	if (inst->modrm_size)
	{
		/* Split modrm into fields */
		inst->modrm = * (unsigned char *) buf;
		inst->modrm_mod = (inst->modrm & 0xc0) >> 6;
		inst->modrm_reg = (inst->modrm & 0x38) >> 3;
		inst->modrm_rm = inst->modrm & 0x07;
		inst->reg = inst->modrm_reg;

		/* Access ModRM table */
		modrm_table_entry = &modrm_table[(inst->modrm_mod << 3)
			| inst->modrm_rm];
		inst->sib_size = modrm_table_entry->sib_size;
		inst->disp_size = modrm_table_entry->disp_size;
		inst->ea_base = modrm_table_entry->ea_base;
		buf += inst->modrm_size;  /* Skip modrm */

		/* Decode SIB */
		if (inst->sib_size)
		{
			inst->sib = * (unsigned char *) buf;
			inst->sib_scale = (inst->sib & 0xc0) >> 6;
			inst->sib_index = (inst->sib & 0x38) >> 3;
			inst->sib_base = inst->sib & 0x07;
			inst->ea_scale = ea_scale_table[inst->sib_scale];
			inst->ea_index = inst->sib_index == 0x04 ? x86_reg_none :
				inst->sib_index + x86_reg_eax;
			inst->ea_base = inst->sib_base + x86_reg_eax;
			if (inst->sib_base == 0x05 && inst->modrm_mod == 0x00)
			{
				inst->ea_base = x86_reg_none;
				inst->disp_size = 4;
			}
			buf += inst->sib_size;  /* Skip SIB */
		}

		/* Decode Displacement */
		switch (inst->disp_size)
		{
		case 1:
			inst->disp = * (int8_t *) buf;
			break;

		case 2:
			inst->disp = * (int16_t *) buf;
			break;

		case 4:
			inst->disp = * (int32_t *) buf;
			break;
		}
		buf += inst->disp_size;  /* Skip disp */
	}

	/* Decode Immediate */
	inst->imm_size = info->imm_size;
	switch (inst->imm_size)
	{
	case 1:
		inst->imm.b = * (uint8_t *) buf;
		break;

	case 2:
		inst->imm.w = * (uint16_t *) buf;
		break;

	case 4:
		inst->imm.d = * (uint32_t *) buf;
		break;
	}
	buf += inst->imm_size;  /* Skip imm */

	/* Calculate total size */
	inst->size = inst->prefix_size + inst->opcode_size + inst->modrm_size +
		inst->sib_size + inst->disp_size + inst->imm_size;
}


/* Stand-alone disassembler */
void x86_disasm_file(char *file_name)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	struct elf_buffer_t *buffer;
	struct elf_symbol_t *symbol;

	struct x86_inst_t inst;
	int curr_sym;
	int i;

	/* Open ELF file */
	x86_disasm_init();
	elf_file = elf_file_create_from_path(file_name);

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
			uint32_t eip;
			char str[MAX_STRING_SIZE];

			/* Read instruction */
			eip = section->header->sh_addr + buffer->pos;
			x86_disasm(elf_buffer_tell(buffer), eip, &inst);
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
	x86_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}


void x86_inst_dump_buf(struct x86_inst_t *inst, char *buf, int size)
{
	enum x86_opcode_t op = inst->opcode;
	struct x86_opcode_info_t *info = &x86_opcode_info_list[op];
	char *fmt = info->fmt;
	int word = 0;

	/* Null-terminate output string in case 'fmt' is empty */
	if (size)
		*buf = '\0';
	
	/* Dump instruction */
	while (*fmt)
	{
		if (is_next_word(fmt, "r8"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->modrm_reg + x86_reg_al));
			fmt += 2;
		}
		else if (is_next_word(fmt, "r16"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->modrm_reg + x86_reg_ax));
			fmt += 3;
		}
		else if (is_next_word(fmt, "r32"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->modrm_reg + x86_reg_eax));
			fmt += 3;
		}
		else if (is_next_word(fmt, "rm8"))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					x86_reg_name_get(inst->modrm_rm + x86_reg_al));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 3;
		}
		else if (is_next_word(fmt, "rm16"))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					x86_reg_name_get(inst->modrm_rm + x86_reg_ax));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 4;
		}
		else if (is_next_word(fmt, "rm32"))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					x86_reg_name_get(inst->modrm_rm + x86_reg_eax));
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 4;
		}
		else if (is_next_word(fmt, "r32m8"))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "%s", x86_reg_name_get(inst->modrm_rm + x86_reg_eax));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 5;
		}
		else if (is_next_word(fmt, "r32m16"))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "%s", x86_reg_name_get(inst->modrm_rm + x86_reg_eax));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 6;
		}
		else if (is_next_word(fmt, "m"))
		{
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt++;
		}
		else if (is_next_word(fmt, "imm8"))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.b);
			fmt += 4;
		}
		else if (is_next_word(fmt, "imm16"))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.w);
			fmt += 5;
		}
		else if (is_next_word(fmt, "imm32"))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.d);
			fmt += 5;
		}
		else if (is_next_word(fmt, "rel8"))
		{
			str_printf(&buf, &size, "%x", (int8_t) inst->imm.b + inst->eip + inst->size);
			fmt += 4;
		}
		else if (is_next_word(fmt, "rel16"))
		{
			str_printf(&buf, &size, "%x", (int16_t) inst->imm.w + inst->eip + inst->size);
		}
		else if (is_next_word(fmt, "rel32"))
		{
			str_printf(&buf, &size, "%x", inst->imm.d + inst->eip + inst->size);
			fmt += 5;
		}
		else if (is_next_word(fmt, "moffs8"))
		{
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 6;
		}
		else if (is_next_word(fmt, "moffs16"))
		{
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 7;
		}
		else if (is_next_word(fmt, "moffs32"))
		{
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 7;
		}
		else if (is_next_word(fmt, "m8"))
		{
			str_printf(&buf, &size, "BYTE PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 2;
		}
		else if (is_next_word(fmt, "m16"))
		{
			str_printf(&buf, &size, "WORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		}
		else if (is_next_word(fmt, "m32"))
		{
			str_printf(&buf, &size, "DWORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		}
		else if (is_next_word(fmt, "m64"))
		{
			str_printf(&buf, &size, "QWORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		}
		else if (is_next_word(fmt, "m80"))
		{
			str_printf(&buf, &size, "TBYTE PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		}
		else if (is_next_word(fmt, "m128"))
		{
			str_printf(&buf, &size, "XMMWORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 4;
		}
		else if (is_next_word(fmt, "st0"))
		{
			str_printf(&buf, &size, "st");
			fmt += 3;
		}
		else if (is_next_word(fmt, "sti"))
		{
			str_printf(&buf, &size, "st(%d)", inst->opindex);
			fmt += 3;
		}
		else if (is_next_word(fmt, "ir8"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->opindex + x86_reg_al));
			fmt += 3;
		}
		else if (is_next_word(fmt, "ir16"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->opindex + x86_reg_ax));
			fmt += 4;
		}
		else if (is_next_word(fmt, "ir32"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->opindex + x86_reg_eax));
			fmt += 4;
		}
		else if (is_next_word(fmt, "sreg"))
		{
			str_printf(&buf, &size, "%s", x86_reg_name_get(inst->reg + x86_reg_es));
			fmt += 4;
		}
		else if (is_next_word(fmt, "xmmm32"))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 6;
		}
		else if (is_next_word(fmt, "xmmm64"))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "QWORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 6;
		}
		else if (is_next_word(fmt, "xmmm128"))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "XMMWORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 7;
		}
		else if (is_next_word(fmt, "xmm"))
		{
			str_printf(&buf, &size, "xmm%d", inst->modrm_reg);
			fmt += 3;
		}
		else
		{
			while (*fmt && is_fmt_char(*fmt))
				str_printf(&buf, &size, "%c", *fmt++);
			while (*fmt && !is_fmt_char(*fmt))
			{
				if (*fmt == '_')
				{
					str_printf(&buf, &size, "%s", word ? ", " : " ");
					word++;
				}
				else
				{
					str_printf(&buf, &size, "%c", *fmt);
				}
				fmt++;
			}
		}
	}
}


void x86_inst_dump(struct x86_inst_t *inst, FILE *f)
{
	char buf[100];
	x86_inst_dump_buf(inst, buf, sizeof(buf));
	fprintf(f, "%s", buf);
}


char *x86_inst_name(enum x86_opcode_t opcode)
{
	if (opcode < 1 || opcode >= x86_opcode_count)
		return NULL;
	return x86_opcode_info_list[opcode].fmt;
}

