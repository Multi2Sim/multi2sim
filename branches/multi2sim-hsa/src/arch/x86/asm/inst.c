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


/* Register names */
struct str_map_t x86_inst_reg_map =
{
	30, {
		{ "eax", X86InstRegEax },
		{ "ecx", X86InstRegEcx },
		{ "edx", X86InstRegEdx },
		{ "ebx", X86InstRegEbx },
		{ "esp", X86InstRegEsp },
		{ "ebp", X86InstRegEbp },
		{ "esi", X86InstRegEsi },
		{ "edi", X86InstRegEdi },
		{ "ax", X86InstRegAx },
		{ "cx", X86InstRegCx },
		{ "dx", X86InstRegDx },
		{ "bx", X86InstRegBx },
		{ "sp", X86InstRegSp },
		{ "bp", X86InstRegBp },
		{ "si", X86InstRegSi },
		{ "di", X86InstRegDi },
		{ "al", X86InstRegAl },
		{ "cl", X86InstRegCl },
		{ "dl", X86InstRegDl },
		{ "bl", X86InstRegBl },
		{ "ah", X86InstRegAh },
		{ "ch", X86InstRegCh },
		{ "dh", X86InstRegDh },
		{ "bh", X86InstRegBh },
		{ "es", X86InstRegEs },
		{ "cs", X86InstRegCs },
		{ "ss", X86InstRegSs },
		{ "ds", X86InstRegDs },
		{ "fs", X86InstRegFs },
		{ "gs", X86InstRegGs }
	}
};


/* Table indexed by pairs ModRM.mod and ModRM.rm, containing
 * information about what will come next and effective address
 * computation. */
struct x86_inst_modrm_table_entry_t
{
	X86InstReg ea_base;
	int disp_size;
	int sib_size;
};


static struct x86_inst_modrm_table_entry_t x86_inst_modrm_table[32] =
{
	{X86InstRegEax, 0, 0},
	{X86InstRegEcx, 0, 0},
	{X86InstRegEdx, 0, 0},
	{X86InstRegEbx, 0, 0},
	{X86InstRegNone, 0, 1},
	{X86InstRegNone, 4, 0},
	{X86InstRegEsi, 0, 0},
	{X86InstRegEdi, 0, 0},

	{X86InstRegEax, 1, 0},
	{X86InstRegEcx, 1, 0},
	{X86InstRegEdx, 1, 0},
	{X86InstRegEbx, 1, 0},
	{X86InstRegNone, 1, 1},
	{X86InstRegEbp, 1, 0},
	{X86InstRegEsi, 1, 0},
	{X86InstRegEdi, 1, 0},

	{X86InstRegEax, 4, 0},
	{X86InstRegEcx, 4, 0},
	{X86InstRegEdx, 4, 0},
	{X86InstRegEbx, 4, 0},
	{X86InstRegNone, 4, 1},
	{X86InstRegEbp, 4, 0},
	{X86InstRegEsi, 4, 0},
	{X86InstRegEdi, 4, 0},

	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0},
	{X86InstRegNone, 0, 0}
};


/* Table to obtain the scale from its decoded value */
unsigned int x86_inst_ea_scale_table[4] = { 1, 2, 4, 8 };





/*
 * Class 'X86Inst'
 */

void X86InstCreate(X86Inst *self, X86Asm *as)
{
	self->as = as;
	X86InstClear(self);
}


void X86InstDestroy(X86Inst *self)
{
	X86InstClear(self);
	self->as = NULL;
}


static void X86InstMoffsAddrDumpBuf(X86Inst *self, char **pbuf, int *psize)
{
	X86InstReg reg;

	reg = self->segment ? self->segment : X86InstRegDs;
	str_printf(pbuf, psize, "%s:0x%x", str_map_value(&x86_inst_reg_map, reg), self->imm.d);
}


static void X86InstAddrDumpBuf(X86Inst *self, char **pbuf, int *psize)
{
	int putsign = 0;
	char seg[20];
	assert(self->modrm_mod != 0x03);

	/* Segment */
	seg[0] = 0;
	if (self->segment) {
		assert(self->segment >= 0 && self->segment < X86InstRegCount);
		strcpy(seg, str_map_value(&x86_inst_reg_map, self->segment));
		strcat(seg, ":");
	}

	/* When there is only a displacement */
	if (!self->ea_base && !self->ea_index) {
		if (!seg[0])
			strcpy(seg, "ds:");
		str_printf(pbuf, psize, "%s0x%x", seg, self->disp);
		return;
	}

	str_printf(pbuf, psize, "%s[", seg);
	if (self->ea_base) {
		str_printf(pbuf, psize, "%s", str_map_value(&x86_inst_reg_map, self->ea_base));
		putsign = 1;
	}
	if (self->ea_index) {
		str_printf(pbuf, psize, "%s%s", putsign ? "+" : "",
			str_map_value(&x86_inst_reg_map, self->ea_index));
		if (self->ea_scale > 1)
			str_printf(pbuf, psize, "*%d", self->ea_scale);
		putsign = 1;
	}
	if (self->disp > 0)
		str_printf(pbuf, psize, "%s0x%x", putsign ? "+" : "", self->disp);
	if (self->disp < 0)
		str_printf(pbuf, psize, "-0x%x", -self->disp);
	str_printf(pbuf, psize, "]");
}


void X86InstDump(X86Inst *self, FILE *f)
{
	char buf[MAX_LONG_STRING_SIZE];

	X86InstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}


void X86InstDumpBuf(X86Inst *self, char *buf, int size)
{
	X86Asm *as;

	struct x86_inst_info_t *info;

	int length;
	int name_printed;
	int name_length;
	int i;

	char *fmt;
	char *fmt_first_arg;

	/* Get instruction information from the globally initialized x86
	 * assembler. */
	as = self->as;
	assert(as);
	assert(IN_RANGE(self->opcode, 0, X86InstOpcodeCount - 1));
	info = &as->inst_info_list[self->opcode];

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
					self->modrm_reg + X86InstRegAl));
		}
		else if (asm_is_token(fmt, "r16", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->modrm_reg + X86InstRegAx));
		}
		else if (asm_is_token(fmt, "r32", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map, 
					self->modrm_reg + X86InstRegEax));
		}
		else if (asm_is_token(fmt, "rm8", &length))
		{
			if (self->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map, 
					self->modrm_rm + X86InstRegAl));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "rm16", &length))
		{
			if (self->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map,
					self->modrm_rm + X86InstRegAx));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "rm32", &length))
		{
			if (self->modrm_mod == 0x03)
				str_printf(&buf, &size, "%s",
					str_map_value(&x86_inst_reg_map,
					self->modrm_rm + X86InstRegEax));
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "r32m8", &length))
		{
			if (self->modrm_mod == 3)
				str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->modrm_rm + X86InstRegEax));
			else
			{
				str_printf(&buf, &size, "BYTE PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "r32m16", &length))
		{
			if (self->modrm_mod == 3)
				str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
						self->modrm_rm + X86InstRegEax));
			else
			{
				str_printf(&buf, &size, "WORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "m", &length))
		{
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "imm8", &length))
		{
			str_printf(&buf, &size, "0x%x", self->imm.b);
		}
		else if (asm_is_token(fmt, "imm16", &length))
		{
			str_printf(&buf, &size, "0x%x", self->imm.w);
		}
		else if (asm_is_token(fmt, "imm32", &length))
		{
			str_printf(&buf, &size, "0x%x", self->imm.d);
		}
		else if (asm_is_token(fmt, "rel8", &length))
		{
			str_printf(&buf, &size, "%x", (int8_t) self->imm.b + self->eip + self->size);
		}
		else if (asm_is_token(fmt, "rel16", &length))
		{
			str_printf(&buf, &size, "%x", (int16_t) self->imm.w + self->eip + self->size);
		}
		else if (asm_is_token(fmt, "rel32", &length))
		{
			str_printf(&buf, &size, "%x", self->imm.d + self->eip + self->size);
		}
		else if (asm_is_token(fmt, "moffs8", &length))
		{
			X86InstMoffsAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "moffs16", &length))
		{
			X86InstMoffsAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "moffs32", &length))
		{
			X86InstMoffsAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m8", &length))
		{
			str_printf(&buf, &size, "BYTE PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m16", &length))
		{
			str_printf(&buf, &size, "WORD PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m32", &length))
		{
			str_printf(&buf, &size, "DWORD PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m64", &length))
		{
			str_printf(&buf, &size, "QWORD PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m80", &length))
		{
			str_printf(&buf, &size, "TBYTE PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "m128", &length))
		{
			str_printf(&buf, &size, "XMMWORD PTR ");
			X86InstAddrDumpBuf(self, &buf, &size);
		}
		else if (asm_is_token(fmt, "st0", &length))
		{
			str_printf(&buf, &size, "st");
		}
		else if (asm_is_token(fmt, "sti", &length))
		{
			str_printf(&buf, &size, "st(%d)", self->opindex);
		}
		else if (asm_is_token(fmt, "ir8", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->opindex + X86InstRegAl));
		}
		else if (asm_is_token(fmt, "ir16", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->opindex + X86InstRegAx));
		}
		else if (asm_is_token(fmt, "ir32", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->opindex + X86InstRegEax));
		}
		else if (asm_is_token(fmt, "sreg", &length))
		{
			str_printf(&buf, &size, "%s", str_map_value(&x86_inst_reg_map,
					self->reg + X86InstRegEs));
		}
		else if (asm_is_token(fmt, "xmmm32", &length))
		{
			if (self->modrm_mod == 3)
				str_printf(&buf, &size, "xmm%d", self->modrm_rm);
			else
			{
				str_printf(&buf, &size, "DWORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmmm64", &length))
		{
			if (self->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", self->modrm_rm);
			else
			{
				str_printf(&buf, &size, "QWORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmmm128", &length))
		{
			if (self->modrm_mod == 0x03)
				str_printf(&buf, &size, "xmm%d", self->modrm_rm);
			else
			{
				str_printf(&buf, &size, "XMMWORD PTR ");
				X86InstAddrDumpBuf(self, &buf, &size);
			}
		}
		else if (asm_is_token(fmt, "xmm", &length))
		{
			str_printf(&buf, &size, "xmm%d", self->modrm_reg);
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


void X86InstClear(X86Inst *self)
{
	self->eip = 0;
	self->size = 0;
	self->opcode = 0;
	self->format = NULL;

	self->prefix_size = 0;
	self->opcode_size = 0;
	self->modrm_size = 0;
	self->sib_size = 0;
	self->disp_size = 0;
	self->imm_size = 0;

	self->opindex = 0;
	self->segment = 0;
	self->prefixes = 0;

	self->op_size = 0;
	self->addr_size = 0;

	self->modrm = 0;
	self->modrm_mod = 0;
	self->modrm_reg = 0;
	self->modrm_rm = 0;

	self->sib = 0;
	self->sib_scale = 0;
	self->sib_index = 0;
	self->sib_base = 0;

	self->disp = 0;
	self->imm.d = 0;

	self->ea_base = 0;
	self->ea_index = 0;
	self->ea_scale = 0;

	self->reg = 0;
}


void X86InstDecode(X86Inst *self, unsigned int eip, void *buf)
{
	X86Asm *as;

	struct x86_inst_info_elem_t **table;
	struct x86_inst_info_elem_t *elem;
	struct x86_inst_info_t *info;
	struct x86_inst_modrm_table_entry_t *modrm_table_entry;

	int index;

	unsigned char buf8;
	unsigned int buf32;

	/* Get assembler */
	as = self->as;
	assert(as);

	/* Initialize instruction */
	X86InstClear(self);
	self->eip = eip;
	self->op_size = 4;
	self->addr_size = 4;

	/* Prefixes */
	while (as->is_prefix[* (unsigned char *) buf])
	{
		switch (* (unsigned char *) buf)
		{

		case 0xf0:
			/* lock prefix is ignored */
			break;

		case 0xf2:
			self->prefixes |= X86InstPrefixRepnz;
			break;

		case 0xf3:
			self->prefixes |= X86InstPrefixRep;
			break;

		case 0x66:
			self->prefixes |= X86InstPrefixOp;
			self->op_size = 2;
			break;

		case 0x67:
			self->prefixes |= X86InstPrefixAddr;
			self->addr_size = 2;
			break;

		case 0x2e:
			self->segment = X86InstRegCs;
			break;

		case 0x36:
			self->segment = X86InstRegSs;
			break;

		case 0x3e:
			self->segment = X86InstRegDs;
			break;

		case 0x26:
			self->segment = X86InstRegEs;
			break;

		case 0x64:
			self->segment = X86InstRegFs;
			break;

		case 0x65:
			self->segment = X86InstRegGs;
			break;

		default:
			panic("%s: invalid prefix", __FUNCTION__);

		}

		/* One more prefix */
		buf++;
		self->prefix_size++;
	}

	/* Obtain lookup table and index */
	buf8 = * (unsigned char *) buf;
	buf32 = * (unsigned int *) buf;
	self->opcode = X86InstOpcodeInvalid;
	if (buf8 == 0x0f)
	{
		table = as->inst_info_table_0f;
		index = * (unsigned char *) (buf + 1);
	}
	else
	{
		table = as->inst_info_table;
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
			&& info->prefixes == self->prefixes)
			break;
	}

	/* Instruction not implemented */
	if (!elem)
		return;

	/* Instruction found */
	self->format = info->fmt;
	self->opcode = info->opcode;
	self->opcode_size = info->opcode_size;
	self->modrm_size = info->modrm_size;
	self->opindex = (buf32 >> info->opindex_shift) & 0x7;
	buf += self->opcode_size;  /* Skip opcode */

	/* Decode the ModR/M field */
	if (self->modrm_size)
	{
		/* Split modrm into fields */
		self->modrm = * (unsigned char *) buf;
		self->modrm_mod = (self->modrm & 0xc0) >> 6;
		self->modrm_reg = (self->modrm & 0x38) >> 3;
		self->modrm_rm = self->modrm & 0x07;
		self->reg = self->modrm_reg;

		/* Access ModRM table */
		modrm_table_entry = &x86_inst_modrm_table[(self->modrm_mod << 3)
			| self->modrm_rm];
		self->sib_size = modrm_table_entry->sib_size;
		self->disp_size = modrm_table_entry->disp_size;
		self->ea_base = modrm_table_entry->ea_base;
		buf += self->modrm_size;  /* Skip modrm */

		/* Decode SIB */
		if (self->sib_size)
		{
			self->sib = * (unsigned char *) buf;
			self->sib_scale = (self->sib & 0xc0) >> 6;
			self->sib_index = (self->sib & 0x38) >> 3;
			self->sib_base = self->sib & 0x07;
			self->ea_scale = x86_inst_ea_scale_table[self->sib_scale];
			self->ea_index = self->sib_index == 0x04 ? X86InstRegNone :
				self->sib_index + X86InstRegEax;
			self->ea_base = self->sib_base + X86InstRegEax;
			if (self->sib_base == 0x05 && self->modrm_mod == 0x00)
			{
				self->ea_base = X86InstRegNone;
				self->disp_size = 4;
			}
			buf += self->sib_size;  /* Skip SIB */
		}

		/* Decode Displacement */
		switch (self->disp_size)
		{
		case 1:
			self->disp = * (int8_t *) buf;
			break;

		case 2:
			self->disp = * (int16_t *) buf;
			break;

		case 4:
			self->disp = * (int32_t *) buf;
			break;
		}
		buf += self->disp_size;  /* Skip disp */
	}

	/* Decode Immediate */
	self->imm_size = info->imm_size;
	switch (self->imm_size)
	{
	case 1:
		self->imm.b = * (unsigned char *) buf;
		break;

	case 2:
		self->imm.w = * (unsigned short *) buf;
		break;

	case 4:
		self->imm.d = * (unsigned int *) buf;
		break;
	}
	buf += self->imm_size;  /* Skip imm */

	/* Calculate total size */
	self->size = self->prefix_size + self->opcode_size + self->modrm_size +
		self->sib_size + self->disp_size + self->imm_size;
}
