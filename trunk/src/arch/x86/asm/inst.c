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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include <assert.h>
#include <string.h>

#include "asm.h"
#include "inst.h"


/*
 * Private Functions
 */

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

void x86_inst_dump_buf(struct x86_inst_t *inst, char *buf, int size)
{
	enum x86_opcode_t op;
	struct x86_inst_info_t *info;
	char *fmt;
	int word = 0;

	/* Get instruction information from the globally initialized x86
	 * assembler. */
	assert(x86_asm);
	op = inst->opcode;
	assert(IN_RANGE(op, 0, x86_opcode_count - 1));
	info = &x86_asm->inst_info_list[op];
	fmt = info->fmt;

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
	assert(x86_asm);
	if (!IN_RANGE(opcode, 1, x86_opcode_count - 1))
		return "<invalid>";
	return x86_asm->inst_info_list[opcode].fmt;
}

