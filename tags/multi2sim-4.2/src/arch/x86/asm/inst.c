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

#include <ctype.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"


/*
 * Variables
 */

/* Register names */
struct str_map_t x86_inst_reg_map =
{
	30, {
		{ "eax", x86_inst_reg_eax },
		{ "ecx", x86_inst_reg_ecx },
		{ "edx", x86_inst_reg_edx },
		{ "ebx", x86_inst_reg_ebx },
		{ "esp", x86_inst_reg_esp },
		{ "ebp", x86_inst_reg_ebp },
		{ "esi", x86_inst_reg_esi },
		{ "edi", x86_inst_reg_edi },
		{ "ax", x86_inst_reg_ax },
		{ "cx", x86_inst_reg_cx },
		{ "dx", x86_inst_reg_dx },
		{ "bx", x86_inst_reg_bx },
		{ "sp", x86_inst_reg_sp },
		{ "bp", x86_inst_reg_bp },
		{ "si", x86_inst_reg_si },
		{ "di", x86_inst_reg_di },
		{ "al", x86_inst_reg_al },
		{ "cl", x86_inst_reg_cl },
		{ "dl", x86_inst_reg_dl },
		{ "bl", x86_inst_reg_bl },
		{ "ah", x86_inst_reg_ah },
		{ "ch", x86_inst_reg_ch },
		{ "dh", x86_inst_reg_dh },
		{ "bh", x86_inst_reg_bh },
		{ "es", x86_inst_reg_es },
		{ "cs", x86_inst_reg_cs },
		{ "ss", x86_inst_reg_ss },
		{ "ds", x86_inst_reg_ds },
		{ "fs", x86_inst_reg_fs },
		{ "gs", x86_inst_reg_gs }
	}
};


/* Table indexed by pairs ModRM.mod and ModRM.rm, containing
 * information about what will come next and effective address
 * computation. */
struct x86_inst_modrm_table_entry_t
{
	enum x86_inst_reg_t ea_base;
	int disp_size;
	int sib_size;
};


static struct x86_inst_modrm_table_entry_t x86_inst_modrm_table[32] =
{
	{x86_inst_reg_eax, 0, 0},
	{x86_inst_reg_ecx, 0, 0},
	{x86_inst_reg_edx, 0, 0},
	{x86_inst_reg_ebx, 0, 0},
	{x86_inst_reg_none, 0, 1},
	{x86_inst_reg_none, 4, 0},
	{x86_inst_reg_esi, 0, 0},
	{x86_inst_reg_edi, 0, 0},

	{x86_inst_reg_eax, 1, 0},
	{x86_inst_reg_ecx, 1, 0},
	{x86_inst_reg_edx, 1, 0},
	{x86_inst_reg_ebx, 1, 0},
	{x86_inst_reg_none, 1, 1},
	{x86_inst_reg_ebp, 1, 0},
	{x86_inst_reg_esi, 1, 0},
	{x86_inst_reg_edi, 1, 0},

	{x86_inst_reg_eax, 4, 0},
	{x86_inst_reg_ecx, 4, 0},
	{x86_inst_reg_edx, 4, 0},
	{x86_inst_reg_ebx, 4, 0},
	{x86_inst_reg_none, 4, 1},
	{x86_inst_reg_ebp, 4, 0},
	{x86_inst_reg_esi, 4, 0},
	{x86_inst_reg_edi, 4, 0},

	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0},
	{x86_inst_reg_none, 0, 0}
};


/* Table to obtain the scale from its decoded value */
unsigned int x86_inst_ea_scale_table[4] = { 1, 2, 4, 8 };





/*
 * Private Functions
 */

static void x86_inst_moffs_addr_dump_buf(struct x86_inst_t *inst, char **pbuf, int *psize)
{
	enum x86_inst_reg_t reg;

	reg = inst->segment ? inst->segment : x86_inst_reg_ds;
	str_printf(pbuf, psize, "%s:0x%x", str_map_value(&x86_inst_reg_map, reg), inst->imm.d);
}


static void x86_inst_addr_dump_buf(struct x86_inst_t *inst, char **pbuf, int *psize)
{
	int putsign = 0;
	char seg[20];
	assert(inst->modrm_mod != 0x03);

	/* Segment */
	seg[0] = 0;
	if (inst->segment) {
		assert(inst->segment >= 0 && inst->segment < x86_inst_reg_count);
		strcpy(seg, str_map_value(&x86_inst_reg_map, inst->segment));
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
		str_printf(pbuf, psize, "%s", str_map_value(&x86_inst_reg_map, inst->ea_base));
		putsign = 1;
	}
	if (inst->ea_index) {
		str_printf(pbuf, psize, "%s%s", putsign ? "+" : "",
			str_map_value(&x86_inst_reg_map, inst->ea_index));
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

struct x86_inst_t *x86_inst_create(void)
{
	struct x86_inst_t *inst;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct x86_inst_t));

	/* Return */
	return inst;
}


void x86_inst_free(struct x86_inst_t *inst)
{
	free(inst);
}


void x86_inst_dump(struct x86_inst_t *inst, FILE *f)
{
	char buf[MAX_LONG_STRING_SIZE];

	x86_inst_dump_buf(inst, buf, sizeof buf);
	fprintf(f, "%s", buf);
}


void x86_inst_dump_buf(struct x86_inst_t *inst, char *buf, int size)
{
	struct x86_inst_info_t *info;

	int length;
	int name_printed;
	int name_length;
	int i;

	char *fmt;
	char *fmt_first_arg;

	/* Get instruction information from the globally initialized x86
	 * assembler. */
	assert(x86_asm);
	assert(IN_RANGE(inst->opcode, 0, x86_inst_opcode_count - 1));
	info = &x86_asm->inst_info_list[inst->opcode];

	/* Null-terminate output string in case 'fmt' is empty */
	if (size)
		*buf = '\0';

	/* Get instruction name length */
	name_printed = 0;
	fmt = info->fmt;
	fmt_first_arg = index(fmt, '_');
	name_length = fmt_first_arg ? fmt_first_arg - fmt : strlen(fmt);

	/* Dump instruction */
	while (*fmt)
	{
		/* Assume no token found */
		length = 0;

		/* Check tokens */
		if (asm_is_token(fmt, "r8", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->modrm_reg + x86_inst_reg_al));
		}
		else if (asm_is_token(fmt, "r16", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->modrm_reg + x86_inst_reg_ax));
		}
		else if (asm_is_token(fmt, "r32", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map, 
					inst->modrm_reg + x86_inst_reg_eax));
		}
		else if (asm_is_token(fmt, "rm8", &length))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map, 
					inst->modrm_rm + x86_inst_reg_al));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "rm16", &length))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map,
					inst->modrm_rm + x86_inst_reg_ax));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "rm32", &length))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map,
					inst->modrm_rm + x86_inst_reg_eax));
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "r32m8", &length))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->modrm_rm + x86_inst_reg_eax));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "r32m16", &length))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
						inst->modrm_rm + x86_inst_reg_eax));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "m", &length))
		{
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "imm8", &length))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.b);
		}
		else if (asm_is_token(fmt, "imm16", &length))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.w);
		}
		else if (asm_is_token(fmt, "imm32", &length))
		{
			str_printf(&buf, &size, "0x%x", inst->imm.d);
		}
		else if (asm_is_token(fmt, "rel8", &length))
		{
			str_printf(&buf, &size, "%x", (int8_t) inst->imm.b + inst->eip + inst->size);
		}
		else if (asm_is_token(fmt, "rel16", &length))
		{
			str_printf(&buf, &size, "%x", (int16_t) inst->imm.w + inst->eip + inst->size);
		}
		else if (asm_is_token(fmt, "rel32", &length))
		{
			str_printf(&buf, &size, "%x", inst->imm.d + inst->eip + inst->size);
		}
		else if (asm_is_token(fmt, "moffs8", &length))
		{
			x86_inst_moffs_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "moffs16", &length))
		{
			x86_inst_moffs_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "moffs32", &length))
		{
			x86_inst_moffs_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m8", &length))
		{
			str_printf(&buf, &size, "BYTE PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m16", &length))
		{
			str_printf(&buf, &size, "WORD PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m32", &length))
		{
			str_printf(&buf, &size, "DWORD PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m64", &length))
		{
			str_printf(&buf, &size, "QWORD PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m80", &length))
		{
			str_printf(&buf, &size, "TBYTE PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "m128", &length))
		{
			str_printf(&buf, &size, "XMMWORD PTR ");
			x86_inst_addr_dump_buf(inst, &buf, &size);
		}
		else if (asm_is_token(fmt, "st0", &length))
		{
			str_printf(&buf, &size, "st");
		}
		else if (asm_is_token(fmt, "sti", &length))
		{
			str_printf(&buf, &size, "st(%d)", inst->opindex);
		}
		else if (asm_is_token(fmt, "ir8", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->opindex + x86_inst_reg_al));
		}
		else if (asm_is_token(fmt, "ir16", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->opindex + x86_inst_reg_ax));
		}
		else if (asm_is_token(fmt, "ir32", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->opindex + x86_inst_reg_eax));
		}
		else if (asm_is_token(fmt, "sreg", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					inst->reg + x86_inst_reg_es));
		}
		else if (asm_is_token(fmt, "xmmm32", &length))
		{
			if (inst->modrm_mod == 3)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmmm64", &length))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "QWORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmmm128", &length))
		{
			if (inst->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", inst->modrm_rm);
			else
			{
				str_printf(&buf, &size, "XMMWORD PTR ");
				x86_inst_addr_dump_buf(inst, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmm", &length))
		{
			str_printf(&buf, &size, "xmm%d", inst->modrm_reg);
		}

		/* Token was found, advance format string and continue */
		if (length)
		{
			fmt += length;
			continue;
		}

		/* Print literal alphanumerics */
		while (*fmt && isalnum(*fmt))
			str_printf(&buf, &size, "%c", *fmt++);

		/* Print literal non-alphanumerics */
		while (*fmt && !isalnum(*fmt))
		{
			if (*fmt == '_')
			{
				if (name_printed)
				{
					str_printf(&buf, &size, ",");
				}
				else
				{
					name_printed = 1;
					for (i = 0; i < 7 - name_length; i++)
						str_printf(&buf, &size, " ");
				}
			}
			else
			{
				str_printf(&buf, &size, "%c", *fmt);
			}
			fmt++;
		}
	}
}


char *x86_inst_get_name(enum x86_inst_opcode_t opcode)
{
	assert(x86_asm);
	if (!IN_RANGE(opcode, 1, x86_inst_opcode_count - 1))
		return "<invalid>";
	return x86_asm->inst_info_list[opcode].fmt;
}


void x86_inst_decode(struct x86_inst_t *inst, unsigned int eip, void *buf)
{
	struct x86_inst_info_elem_t **table;
	struct x86_inst_info_elem_t *elem;
	struct x86_inst_info_t *info;
	struct x86_inst_modrm_table_entry_t *modrm_table_entry;

	int index;

	unsigned char buf8;
	unsigned int buf32;

	/* Assembler must be initialized */
	assert(x86_asm);


	/* Initialize instruction */

	inst->eip = eip;
	inst->size = 0;
	inst->opcode = 0;
	inst->format = NULL;

	inst->prefix_size = 0;
	inst->opcode_size = 0;
	inst->modrm_size = 0;
	inst->sib_size = 0;
	inst->disp_size = 0;
	inst->imm_size = 0;

	inst->opindex = 0;
	inst->segment = 0;
	inst->prefixes = 0;

	inst->op_size = 4;
	inst->addr_size = 4;

	inst->modrm = 0;
	inst->modrm_mod = 0;
	inst->modrm_reg = 0;
	inst->modrm_rm = 0;

	inst->sib = 0;
	inst->sib_scale = 0;
	inst->sib_index = 0;
	inst->sib_base = 0;

	inst->disp = 0;
	inst->imm.d = 0;

	inst->ea_base = 0;
	inst->ea_index = 0;
	inst->ea_scale = 0;

	inst->reg = 0;


	/* Prefixes */
	while (x86_asm->is_prefix[* (unsigned char *) buf])
	{
		switch (* (unsigned char *) buf)
		{

		case 0xf0:
			/* lock prefix is ignored */
			break;

		case 0xf2:
			inst->prefixes |= x86_inst_prefix_repnz;
			break;

		case 0xf3:
			inst->prefixes |= x86_inst_prefix_rep;
			break;

		case 0x66:
			inst->prefixes |= x86_inst_prefix_op;
			inst->op_size = 2;
			break;

		case 0x67:
			inst->prefixes |= x86_inst_prefix_addr;
			inst->addr_size = 2;
			break;

		case 0x2e:
			inst->segment = x86_inst_reg_cs;
			break;

		case 0x36:
			inst->segment = x86_inst_reg_ss;
			break;

		case 0x3e:
			inst->segment = x86_inst_reg_ds;
			break;

		case 0x26:
			inst->segment = x86_inst_reg_es;
			break;

		case 0x64:
			inst->segment = x86_inst_reg_fs;
			break;

		case 0x65:
			inst->segment = x86_inst_reg_gs;
			break;

		default:
			panic("%s: invalid prefix", __FUNCTION__);

		}

		/* One more prefix */
		buf++;
		inst->prefix_size++;
	}

	/* Obtain lookup table and index */
	buf8 = * (unsigned char *) buf;
	buf32 = * (unsigned int *) buf;
	inst->opcode = x86_inst_opcode_invalid;
	if (buf8 == 0x0f)
	{
		table = x86_asm->inst_info_table_0f;
		index = * (unsigned char *) (buf + 1);
	}
	else
	{
		table = x86_asm->inst_info_table;
		index = buf8;
	}

	/* Find instruction */
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
		modrm_table_entry = &x86_inst_modrm_table[(inst->modrm_mod << 3)
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
			inst->ea_scale = x86_inst_ea_scale_table[inst->sib_scale];
			inst->ea_index = inst->sib_index == 0x04 ? x86_inst_reg_none :
				inst->sib_index + x86_inst_reg_eax;
			inst->ea_base = inst->sib_base + x86_inst_reg_eax;
			if (inst->sib_base == 0x05 && inst->modrm_mod == 0x00)
			{
				inst->ea_base = x86_inst_reg_none;
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
		inst->imm.b = * (unsigned char *) buf;
		break;

	case 2:
		inst->imm.w = * (unsigned short *) buf;
		break;

	case 4:
		inst->imm.d = * (unsigned int *) buf;
		break;
	}
	buf += inst->imm_size;  /* Skip imm */

	/* Calculate total size */
	inst->size = inst->prefix_size + inst->opcode_size + inst->modrm_size +
		inst->sib_size + inst->disp_size + inst->imm_size;
}
