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

#include "disasm.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <debug.h>


/* Flags used in machine.dat */
#define SKIP  0x0100  /* for op1, op2, op3, imm */
#define REG   0x0200  /* for op3 */
#define MEM   0x0400  /* for op3 */
#define OP3   0x0800  /* for op3 means that it is the 3rd opcode byte. */
#define INDEX 0x1000  /* for op1, op2 */
#define IB    0x2000  /* for imm */
#define IW    0x4000  /* for imm */
#define ID    0x8000  /* for imm */


/* This struct contains information derived from machine.dat, which
 * is initialized in disasm_init, to form linked lists in the table
 * x86_opcode_info_table and a single list in x86_opcode_info_list. */
typedef struct x86_opcode_info_struct {

	/* Obtained from machine.dat */
	x86_opcode_t opcode;
	uint32_t op1, op2, op3, imm;
	x86_prefix_t prefixes;
	char *fmt;

	/* Derived fields */
	uint32_t match_mask, match_result;
	uint32_t nomatch_mask, nomatch_result;
	int opindex_shift;  /* pos to shift inst to obtain index of op1/op2 if any */
	int impl_reg;  /* implied register in op1 (0-7) */
	int opcode_size;  /* size of opcode (1 or 2), not counting the modrm part. */
	int modrm_size;  /* size of modrm field (0 or 1) */
	int imm_size;  /* Immediate size (0, 1, 2, or 4) */
} x86_opcode_info_t;


/* Containers for opcode infos. We need this because an info can belong to
 * different lists when there are registers embedded in the opcodes. */
typedef struct x86_opcode_info_elem_struct {
	x86_opcode_info_t *info;
	struct x86_opcode_info_elem_struct *next;
} x86_opcode_info_elem_t;


/* Table for fast access of instruction data, indexed by the instruction opcode.
 * The second table contains instructions whose first opcode byte is 0x0f, and
 * is indexed by the second byte of its opcode. */
static x86_opcode_info_elem_t *x86_opcode_info_table[0x100];
static x86_opcode_info_elem_t *x86_opcode_info_table_0f[0x100];


/* List of instructions. */
static x86_opcode_info_t x86_opcode_info_list[x86_opcode_count] = {
	{op_none, 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define DEFINST(name,op1,op2,op3,imm,pfx) \
	,{op_##name,op1,op2,op3,imm,pfx,#name,0,0,0,0,0,0,0,0,0}
#include <machine.dat>
#undef DEFINST
};


/* Register names */
char *x86_register_name[x86_register_count] = {
	"",
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
	"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
	"es", "cs", "ss", "ds", "fs", "gs"
};


/* Table indexed by pairs ModRM.mod and ModRM.rm, containing
 * information about what will come next and effective address
 * computation. */
typedef struct {
	x86_register_t ea_base;
	int disp_size;
	int sib_size;
} modrm_table_entry_t;
modrm_table_entry_t modrm_table[32] = {
	{reg_eax, 0, 0}, {reg_ecx, 0, 0},
	{reg_edx, 0, 0}, {reg_ebx, 0, 0},
	{reg_none, 0, 1}, {reg_none, 4, 0},
	{reg_esi, 0, 0}, {reg_edi, 0, 0},

	{reg_eax, 1, 0}, {reg_ecx, 1, 0},
	{reg_edx, 1, 0}, {reg_ebx, 1, 0},
	{reg_none, 1, 1}, {reg_ebp, 1, 0},
	{reg_esi, 1, 0}, {reg_edi, 1, 0},

	{reg_eax, 4, 0}, {reg_ecx, 4, 0},
	{reg_edx, 4, 0}, {reg_ebx, 4, 0},
	{reg_none, 4, 1}, {reg_ebp, 4, 0},
	{reg_esi, 4, 0}, {reg_edi, 4, 0},

	{reg_none, 0, 0}, {reg_none, 0, 0},
	{reg_none, 0, 0}, {reg_none, 0, 0},
	{reg_none, 0, 0}, {reg_none, 0, 0},
	{reg_none, 0, 0}, {reg_none, 0, 0}
};


/* Table to obtain the scale
 * from its decoded value */
uint32_t ea_scale_table[4] = { 1, 2, 4, 8};


static void x86_opcode_info_insert_at(x86_opcode_info_elem_t **table,
	x86_opcode_info_elem_t *elem, int at)
{
	x86_opcode_info_elem_t *prev;

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


static void x86_opcode_info_insert(x86_opcode_info_t *info)
{
	x86_opcode_info_elem_t *elem;
	x86_opcode_info_elem_t **table;
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
		elem = calloc(1, sizeof(x86_opcode_info_elem_t));
		elem->info = info;
		x86_opcode_info_insert_at(table, elem, index + i);
	}
}


void disasm_init()
{
	x86_opcode_t op;
	x86_opcode_info_t *info;

	/* Initialize x86_opcode_info_table. This table contains lists of
	 * information about machine instructions. To find an instruction
	 * in the table, it can be indexed by the first byte of its opcode. */
	for (op = 1; op < x86_opcode_count; op++) {
		
		/* Insert into table */
		info = &x86_opcode_info_list[op];
		x86_opcode_info_insert(info);

		/* Compute match_mask and mach_result fields. Start with
		 * the 'op3' field in the instruction format definition. */
		if (info->op3 & OP3) {

			/* If op3 is representing the 3rd opcode byte, it must be
			 * matched entirely. Otherwise, it is the ModR/M field. */
			info->opcode_size++;
			info->match_mask = 0xff;
			info->match_result = info->op3 & 0xff;

		} else if (!(info->op3 & SKIP)) {

			info->modrm_size = 1;

			/* If part of the offset is in the 'reg' field of the ModR/M byte,
			 * it must be matched. */
			if (!(info->op3 & REG)) {
				info->match_mask = 0x38;
				info->match_result = (info->op3 & 0x7) << 3;
			}

			/* If instruction expects a memory operand, the 'mod' field of 
			 * the ModR/M byte cannot be 11. */
			if (info->op3 & MEM) {
				info->nomatch_mask = 0xc0;
				info->nomatch_result = 0xc0;
			}
		}

		/* Second opcode byte */
		if (!(info->op2 & SKIP)) {
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
		if (info->op1 & INDEX) {
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

	/* Print table */
	/*{
		int i, count;
		for (i = 0; i < 0x100; i++) {
			count = 0;
			info = x86_opcode_info_table[i];
			while (info) {
				count++;
				info = info->next;
			}
			printf("x86_opcode_info_table[0x%x].count = %d\n",
				i, count);
		}
		abort();
	}*/
}


void x86_opcode_info_elem_free_list(x86_opcode_info_elem_t *elem)
{
	x86_opcode_info_elem_t *next;
	while (elem) {
		next = elem->next;
		free(elem);
		elem = next;
	}
}


void disasm_done()
{
	int i;
	for (i = 0; i < 0x100; i++) {
		x86_opcode_info_elem_free_list(x86_opcode_info_table[i]);
		x86_opcode_info_elem_free_list(x86_opcode_info_table_0f[i]);
	}
}


/* Pointer to 'inst' is declared volatile to avoid optimizations when calling 'memset' */
void x86_disasm(void *buf, uint32_t eip, volatile x86_inst_t *inst)
{
	x86_opcode_info_elem_t **table, *elem;
	x86_opcode_info_t *info;
	int index;
	uint32_t buf32;
	modrm_table_entry_t *modrm_table_entry;
	int was_any_prefix;

	/* Initialize instruction */
	memset((void *) inst, 0, sizeof(x86_inst_t));
	inst->eip = eip;
	inst->op_size = 4;
	inst->addr_size = 4;

	/* Prefixes */
	do {
		was_any_prefix = 1;
		switch (* (unsigned char *) buf) {
		case 0xf0: /* lock prefix is ignored */ break;
		case 0xf2: inst->prefixes |= prefix_repnz; break;
		case 0xf3: inst->prefixes |= prefix_rep; break;
		case 0x66: inst->prefixes |= prefix_op; inst->op_size = 2; break;
		case 0x67: inst->prefixes |= prefix_addr; inst->addr_size = 2; break;
		case 0x2e: inst->segment = reg_cs; break;
		case 0x36: inst->segment = reg_ss; break;
		case 0x3e: inst->segment = reg_ds; break;
		case 0x26: inst->segment = reg_es; break;
		case 0x64: inst->segment = reg_fs; break;
		case 0x65: inst->segment = reg_gs; break;
		default: was_any_prefix = 0;
		}

		if (was_any_prefix) {
			buf++;
			inst->prefix_size++;
		}
	} while (was_any_prefix);

	/* Find instruction */
	buf32 = * (uint32_t *) buf;
	inst->opcode = op_none;
	table = * (unsigned char *) buf == 0x0f ? x86_opcode_info_table_0f : x86_opcode_info_table;
	index = * (unsigned char *) buf == 0x0f ? * (unsigned char *) (buf + 1): * (unsigned char *) buf;
	for (elem = table[index]; elem; elem = elem->next) {
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
	if (inst->modrm_size) {
		
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
		if (inst->sib_size) {
			inst->sib = * (unsigned char *) buf;
			inst->sib_scale = (inst->sib & 0xc0) >> 6;
			inst->sib_index = (inst->sib & 0x38) >> 3;
			inst->sib_base = inst->sib & 0x07;
			inst->ea_scale = ea_scale_table[inst->sib_scale];
			inst->ea_index = inst->sib_index == 0x04 ? reg_none :
				inst->sib_index + reg_eax;
			inst->ea_base = inst->sib_base + reg_eax;
			if (inst->sib_base == 0x05 && inst->modrm_mod == 0x00) {
				inst->ea_base = reg_none;
				inst->disp_size = 4;
			}
			buf += inst->sib_size;  /* Skip SIB */
		}

		/* Decode Displacement */
		switch (inst->disp_size) {
		case 1: inst->disp = * (int8_t *) buf; break;
		case 2: inst->disp = * (int16_t *) buf; break;
		case 4: inst->disp = * (int32_t *) buf; break;
		}
		buf += inst->disp_size;  /* Skip disp */
	}

	/* Decode Immediate */
	inst->imm_size = info->imm_size;
	switch (inst->imm_size) {
	case 0: break;
	case 1: inst->imm.b = * (uint8_t *) buf; break;
	case 2: inst->imm.w = * (uint16_t *) buf; break;
	case 4: inst->imm.d = * (uint32_t *) buf; break;
	}
	buf += inst->imm_size;  /* Skip imm */

	/* Calculate total size */
	inst->size = inst->prefix_size + inst->opcode_size + inst->modrm_size +
		inst->sib_size + inst->disp_size + inst->imm_size;
}


int is_fmt_char(char c)
{
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9');
}


int is_next_word(char *src, char *word)
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


void x86_moffs_address_dump_buf(x86_inst_t *inst, char **pbuf, int *psize)
{
	dump_buf(pbuf, psize, "%s:0x%x",
		x86_register_name[inst->segment ? inst->segment : reg_ds],
		inst->imm.d);
}


void x86_memory_address_dump_buf(x86_inst_t *inst, char **pbuf, int *psize)
{
	int putsign = 0;
	char seg[10];
	assert(inst->modrm_mod != 0x03);

	/* Segment */
	seg[0] = 0;
	if (inst->segment) {
		strcpy(seg, x86_register_name[inst->segment]);
		strcat(seg, ":");
	}

	/* When there is only a displacement */
	if (!inst->ea_base && !inst->ea_index) {
		if (!seg[0])
			strcpy(seg, "ds:");
		dump_buf(pbuf, psize, "%s0x%x", seg, inst->disp);
		return;
	}

	dump_buf(pbuf, psize, "%s[", seg);
	if (inst->ea_base) {
		dump_buf(pbuf, psize, "%s", x86_register_name[inst->ea_base]);
		putsign = 1;
	}
	if (inst->ea_index) {
		dump_buf(pbuf, psize, "%s%s", putsign ? "+" : "",
			x86_register_name[inst->ea_index]);
		if (inst->ea_scale > 1)
			dump_buf(pbuf, psize, "*%d", inst->ea_scale);
		putsign = 1;
	}
	if (inst->disp > 0)
		dump_buf(pbuf, psize, "%s0x%x", putsign ? "+" : "", inst->disp);
	if (inst->disp < 0)
		dump_buf(pbuf, psize, "-0x%x", -inst->disp);
	dump_buf(pbuf, psize, "]");
}


char *x86_inst_name(x86_opcode_t opcode)
{
	if (opcode < 1 || opcode >= x86_opcode_count)
		return NULL;
	return x86_opcode_info_list[opcode].fmt;
}


void x86_inst_dump_buf(x86_inst_t *inst, char *buf, int size)
{
	x86_opcode_t op = inst->opcode;
	x86_opcode_info_t *info = &x86_opcode_info_list[op];
	char *fmt = info->fmt;
	int word = 0;

	while (*fmt) {
		if (is_next_word(fmt, "r8")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->modrm_reg + reg_al]);
			fmt += 2;
		} else if (is_next_word(fmt, "r16")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->modrm_reg + reg_ax]);
			fmt += 3;
		} else if (is_next_word(fmt, "r32")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->modrm_reg + reg_eax]);
			fmt += 3;
		} else if (is_next_word(fmt, "rm8")) {
			if (inst->modrm_mod == 0x03)
				dump_buf(&buf, &size, "%s",
					x86_register_name[inst->modrm_rm + reg_al]);
			else {
				dump_buf(&buf, &size, "BYTE PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 3;
		} else if (is_next_word(fmt, "rm16")) {
			if (inst->modrm_mod == 0x03)
				dump_buf(&buf, &size, "%s",
					x86_register_name[inst->modrm_rm + reg_ax]);
			else {
				dump_buf(&buf, &size, "WORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 4;
		} else if (is_next_word(fmt, "rm32")) {
			if (inst->modrm_mod == 0x03)
				dump_buf(&buf, &size, "%s",
					x86_register_name[inst->modrm_rm + reg_eax]);
			else {
				dump_buf(&buf, &size, "DWORD PTR ");
				x86_memory_address_dump_buf(inst, &buf, &size);
			}
			fmt += 4;
		} else if (is_next_word(fmt, "m")) {
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt++;
		} else if (is_next_word(fmt, "imm8")) {
			dump_buf(&buf, &size, "0x%x", inst->imm.b);
			fmt += 4;
		} else if (is_next_word(fmt, "imm16")) {
			dump_buf(&buf, &size, "0x%x", inst->imm.w);
			fmt += 5;
		} else if (is_next_word(fmt, "imm32")) {
			dump_buf(&buf, &size, "0x%x", inst->imm.d);
			fmt += 5;
		} else if (is_next_word(fmt, "rel8")) {
			dump_buf(&buf, &size, "%x", (int8_t) inst->imm.b + inst->eip + inst->size);
			fmt += 4;
		} else if (is_next_word(fmt, "rel16")) {
			dump_buf(&buf, &size, "%x", (int16_t) inst->imm.w + inst->eip + inst->size);
		} else if (is_next_word(fmt, "rel32")) {
			dump_buf(&buf, &size, "%x", inst->imm.d + inst->eip + inst->size);
			fmt += 5;
		} else if (is_next_word(fmt, "moffs8")) {
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 6;
		} else if (is_next_word(fmt, "moffs16")) {
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 7;
		} else if (is_next_word(fmt, "moffs32")) {
			x86_moffs_address_dump_buf(inst, &buf, &size);
			fmt += 7;
		} else if (is_next_word(fmt, "m8")) {
			dump_buf(&buf, &size, "BYTE PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 2;
		} else if (is_next_word(fmt, "m16")) {
			dump_buf(&buf, &size, "WORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		} else if (is_next_word(fmt, "m32")) {
			dump_buf(&buf, &size, "DWORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		} else if (is_next_word(fmt, "m64")) {
			dump_buf(&buf, &size, "QWORD PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		} else if (is_next_word(fmt, "m80")) {
			dump_buf(&buf, &size, "TBYTE PTR ");
			x86_memory_address_dump_buf(inst, &buf, &size);
			fmt += 3;
		} else if (is_next_word(fmt, "st0")) {
			dump_buf(&buf, &size, "st");
			fmt += 3;
		} else if (is_next_word(fmt, "sti")) {
			dump_buf(&buf, &size, "st(%d)", inst->opindex);
			fmt += 3;
		} else if (is_next_word(fmt, "ir8")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->opindex + reg_al]);
			fmt += 3;
		} else if (is_next_word(fmt, "ir16")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->opindex + reg_ax]);
			fmt += 4;
		} else if (is_next_word(fmt, "ir32")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->opindex + reg_eax]);
			fmt += 4;
		} else if (is_next_word(fmt, "sreg")) {
			dump_buf(&buf, &size, "%s", x86_register_name[inst->reg + reg_es]);
			fmt += 4;
		} else {
			while (*fmt && is_fmt_char(*fmt))
				dump_buf(&buf, &size, "%c", *fmt++);
			while (*fmt && !is_fmt_char(*fmt)) {
				if (*fmt == '_') {
					dump_buf(&buf, &size, "%s", word ? ", " : " ");
					word++;
				} else {
					dump_buf(&buf, &size, "%c", *fmt);
				}
				fmt++;
			}
		}
	}
}


void x86_inst_dump(x86_inst_t *inst, FILE *f)
{
	char buf[100];
	x86_inst_dump_buf(inst, buf, sizeof(buf));
	fprintf(f, "%s", buf);
}

