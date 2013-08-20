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

#include "asm.h"
#include "inst.h"



/* Register names */
#define MIPS_INST_REG_COUNT 32
static char *mips_reg_name[MIPS_INST_REG_COUNT] =
{
	"zero",
	"at",
	"v0",
	"v1",
	"a0",
	"a1",
	"a2",
	"a3",
	"t0",
	"t1",
	"t2",
	"t3",
	"t4",
	"t5",
	"t6",
	"t7",
	"s0",
	"s1",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"t8",
	"t9",
	"k0",
	"k1",
	"gp",
	"sp",
	"s8",
	"ra"
};





/*
 * Class 'MIPSInst'
 */

void MIPSInstCreate(MIPSInst *self, MIPSAsm *as)
{
	/* Initialize */
	self->as = as;
}


void MIPSInstDestroy(MIPSInst *self)
{
}


void MIPSInstDecode(MIPSInst *self, unsigned int addr, void *buf)
{
	MIPSAsm *as = self->as;

	struct mips_inst_info_t *current_table;

	int current_table_low;
	int current_table_high;
	int loop_iteration;

	unsigned int table_arg;

	/* Store the instruction */
	self->bytes.word = * (unsigned int *) buf;
	self->addr = addr;
	self->target = 0;

	/* We start with the first table mips_asm_table, with the
	 * opcode field as argument */
	current_table = as->dec_table;
	current_table_low = 26;
	current_table_high = 31;
	loop_iteration = 0;
	table_arg = BITS32(self->bytes.word, current_table_high,
			current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while (1)
	{
		if (current_table[table_arg].next_table
				&& loop_iteration < 4)
		{
			current_table_high = current_table[table_arg].next_table_high;
			current_table_low = current_table[table_arg].next_table_low;
			current_table = current_table[table_arg].next_table;
			table_arg = BITS32(self->bytes.word, current_table_high,
					current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 4)
		{
			fatal("%s: invalid instruction", __FUNCTION__);
		}
		else
		{
			break;
		}
	}

	/* Instruction found */
	self->info = &current_table[table_arg];
}


static void MIPSInstDumpBufSa(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int sa;

	sa = self->bytes.standard.sa;
	if (sa != 0)
		str_printf(buf_ptr, size_ptr, "0x%x", sa);
	else
		str_printf(buf_ptr, size_ptr, "%d", sa);
}


static void MIPSInstDumpBufRd(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int rd;
	int token_len;

	rd = self->bytes.standard.rd;

	if (asm_is_token(self->info->name, "RDHWR", &token_len))
		str_printf(buf_ptr, size_ptr, "$%d", rd);
	else if (asm_is_token(self->info->name, "JALR", &token_len))
	{
		if (rd == (int) 31)
		{
		}
		else
		{
			str_printf(buf_ptr, size_ptr, ",");
			str_printf(buf_ptr, size_ptr, "%s",
				mips_reg_name[rd]);
		}
	}
	else
		str_printf(buf_ptr, size_ptr, "%s",
			mips_reg_name[rd]);
}


static void MIPSInstDumpBufRt(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int rt;

	rt = self->bytes.standard.rt;
	str_printf(buf_ptr, size_ptr, "%s", mips_reg_name[rt]);
}


static void MIPSInstDumpBufRs(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int rs;

	rs = self->bytes.standard.rs;
	str_printf(buf_ptr, size_ptr, "%s", mips_reg_name[rs]);
}


static void MIPSInstDumpBufTarget(MIPSInst *self,
		char **buf_ptr, int *size_ptr)
{
	unsigned int target;

	target = self->bytes.target.target;
	target = target << 2;
	self->target = target;
	str_printf(buf_ptr, size_ptr, "%x", target);
}


static void MIPSInstDumpBufOffset(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int offset;

	offset = self->bytes.offset_imm.offset;
	if (offset & 0x8000)
		offset = -((offset ^ 0xffff) + 1);
	str_printf(buf_ptr, size_ptr, "%d", (int) offset);
}


static void MIPSInstDumpBufOffsetbr(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int offsetbr;

	offsetbr = self->bytes.cc.offsetbr;
	if (offsetbr & 0x8000)
		offsetbr = -((offsetbr ^ 0xffff) + 1);
	offsetbr = (offsetbr << 2) + self->addr + 4;
	self->target = offsetbr;
	str_printf(buf_ptr, size_ptr, "%x", offsetbr);
}


static void MIPSInstDumpBufImm(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int imm;

	imm = self->bytes.offset_imm.offset;
	if (imm & 0x8000)
		imm = -((imm ^ 0xffff) + 1);
	str_printf(buf_ptr, size_ptr, "%d", (int) imm);
}


static void MIPSInstDumpBufImmhex(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int immhex;

	immhex = self->bytes.offset_imm.offset;
	str_printf(buf_ptr, size_ptr, "0x%x", immhex);
}


static void MIPSInstDumpBufBase(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int base;

	base = self->bytes.offset_imm.base;
	str_printf(buf_ptr, size_ptr, "%s", mips_reg_name[base]);
}


static void MIPSInstDumpBufSel(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int sel;

	sel = self->bytes.sel.sel;
	str_printf(buf_ptr, size_ptr, "%d", sel);
}


static void MIPSInstDumpBufCc(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int cc;

	cc = self->bytes.cc.cc;
	str_printf(buf_ptr, size_ptr, "$fcc%d", cc);
}


static void MIPSInstDumpBufPos(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int pos;

	pos = self->bytes.standard.sa;
	str_printf(buf_ptr, size_ptr, "0x%x", pos);
}


static void MIPSInstDumpBufFs(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int fs;
	int token_len;

	fs = self->bytes.standard.rd;
	if ((asm_is_token(self->info->name, "CFC1", &token_len)) ||
		(asm_is_token(self->info->name, "CTC1", &token_len)))
		str_printf(buf_ptr, size_ptr, "$%d", fs);
	else
		str_printf(buf_ptr, size_ptr, "$f%d", fs);
}


static void MIPSInstDumpBufSize(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int size;
	unsigned int pos;

	int token_len;

	pos = self->bytes.standard.sa;
	size = self->bytes.standard.rd;

	if (asm_is_token(self->info->name, "INS", &token_len))
		size = size + 1 - pos;
	else if (asm_is_token(self->info->name, "EXT", &token_len))
		size++;
	str_printf(buf_ptr, size_ptr, "0x%x", size);
}


static void MIPSInstDumpBufFt(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int ft;

	ft = self->bytes.standard.rt;
	str_printf(buf_ptr, size_ptr, "$f%d", ft);
}


static void MIPSInstDumpBufFd(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int fd;

	fd = self->bytes.standard.sa;
	str_printf(buf_ptr, size_ptr, "$f%d", fd);
}


static void MIPSInstDumpBufCode(MIPSInst *self, char **buf_ptr, int *size_ptr)
{
	unsigned int code;

	code = self->bytes.code.code;
	str_printf(buf_ptr, size_ptr, "0x%x", code);
}


void MIPSInstDumpBuf(MIPSInst *self, char *buf, int size)
{
	char *orig_buf = buf;
	char *fmt_str;

	int i = 0;
	int token_len;

	unsigned int rd;
	unsigned int rt;
	unsigned int rs;
	unsigned int sa;

	/* Get format string */
	fmt_str = self->info->fmt_str;
	if (!fmt_str || !*fmt_str)
		fatal("Instruction not implememted.\n");

	/* Traverse format string */
	while (*fmt_str)
	{
		rd = self->bytes.standard.rd;
		rt = self->bytes.standard.rt;
		rs = self->bytes.standard.rs;
		sa = self->bytes.standard.sa;

		/*
		 *  DEAL WITH PSEUDO INSTRUCTIONS
		 */

		/* SLL ZERO, ZERO, ZERO => NOP */
		if (asm_is_token(fmt_str, "sll", &token_len))
		{
			if ((rd | rt | sa) == 0)
			{
				str_printf(&buf, &size, "nop");
				break;
			}
		}
		/* ADDU RD, RS, ZERO => MOVE RD, RS */
		else if (asm_is_token(fmt_str, "addu", &token_len))
		{
			if (rt == 0)
			{
				str_printf(&buf, &size, "move\t");
				MIPSInstDumpBufRd(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufRs(self, &buf, &size);
				break;
			}
		}
		/* BGEZAL ZERO, OFFSET => BAL OFFSET */
		else if (asm_is_token(fmt_str, "bgezal",
				&token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "bal\t");
				MIPSInstDumpBufOffsetbr(self, &buf, &size);
				break;
			}
		}
		/* BEQ ZERO, ZERO, OFFSET => B OFFSET */
		/* BEQ RS, ZERO, OFFSET => BEQZ RS, OFFSET */
		else if (asm_is_token(fmt_str, "beq", &token_len))
		{
			if ((rs | rt) == 0)
			{
				str_printf(&buf, &size, "b\t");
				MIPSInstDumpBufOffsetbr(self, &buf, &size);
				break;
			}
			else if (rt == 0)
			{
				str_printf(&buf, &size, "beqz\t");
				MIPSInstDumpBufRs(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufOffsetbr(self, &buf, &size);
				break;
			}
		}
		/* ADDIU RT, ZERO, IMM => LI RT, IMM */
		else if (asm_is_token(fmt_str, "addiu",
				&token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "li\t");
				MIPSInstDumpBufRt(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufImm(self, &buf, &size);
				break;
			}
		}
		/* ORI RT, ZERO, IMM => LI RT, IMM */
		else if (asm_is_token(fmt_str, "ori", &token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "li\t");
				MIPSInstDumpBufRt(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufImmhex(self, &buf, &size);
				break;
			}
		}
		/* BNE RS, ZERO, OFFSET => BNEZ RS, OFFSET */
		else if (asm_is_token(fmt_str, "bne", &token_len))
		{
			if (rt == 0)
			{
				str_printf(&buf, &size, "bnez\t");
				MIPSInstDumpBufRs(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufOffsetbr(self, &buf, &size);
				break;
			}
		}
		/* SUBU RD, ZERO, RT => NEGU RD, RT */
		else if (asm_is_token(fmt_str, "subu", &token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "negu\t");
				MIPSInstDumpBufRd(self, &buf, &size);
				str_printf(&buf, &size, ",");
				MIPSInstDumpBufRt(self, &buf, &size);
				break;
			}
		}
		/* DONE WITH PSEUDO INSTRUCTIONS */

		if (*fmt_str != '%')
		{
			if (*fmt_str != ' ' || buf != orig_buf)
			{
				if (*fmt_str == '_')
					str_printf(&buf,
							&size, ".");
				else
					str_printf(&buf,
							&size, "%c",
							*fmt_str);
			}
			++fmt_str;
			continue;
		}
		if (i == 0)
			str_printf(&buf, &size, "\t");
		i = 1;
		++fmt_str;
		if (asm_is_token(fmt_str, "sa", &token_len))
			MIPSInstDumpBufSa(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rd", &token_len))
			MIPSInstDumpBufRd(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rt", &token_len))
			MIPSInstDumpBufRt(self, &buf, &size);
		else if (asm_is_token(fmt_str, "rs", &token_len))
			MIPSInstDumpBufRs(self, &buf, &size);
		else if (asm_is_token(fmt_str, "target",
				&token_len))
			MIPSInstDumpBufTarget(self, &buf, &size);
		else if (asm_is_token(fmt_str, "offset",
				&token_len))
			MIPSInstDumpBufOffset(self, &buf, &size);
		else if (asm_is_token(fmt_str, "offsetbr",
				&token_len))
			MIPSInstDumpBufOffsetbr(self, &buf, &size);
		else if (asm_is_token(fmt_str, "Imm", &token_len))
			MIPSInstDumpBufImm(self, &buf, &size);
		else if (asm_is_token(fmt_str, "Immhex",
				&token_len))
			MIPSInstDumpBufImmhex(self, &buf, &size);
		else if (asm_is_token(fmt_str, "base", &token_len))
			MIPSInstDumpBufBase(self, &buf, &size);
		else if (asm_is_token(fmt_str, "sel", &token_len))
			MIPSInstDumpBufSel(self, &buf, &size);
		else if (asm_is_token(fmt_str, "cc", &token_len))
			MIPSInstDumpBufCc(self, &buf, &size);
		else if (asm_is_token(fmt_str, "pos", &token_len))
			MIPSInstDumpBufPos(self, &buf, &size);
		else if (asm_is_token(fmt_str, "size", &token_len))
			MIPSInstDumpBufSize(self, &buf, &size);
		else if (asm_is_token(fmt_str, "fs", &token_len))
			MIPSInstDumpBufFs(self, &buf, &size);
		else if (asm_is_token(fmt_str, "ft", &token_len))
			MIPSInstDumpBufFt(self, &buf, &size);
		else if (asm_is_token(fmt_str, "fd", &token_len))
			MIPSInstDumpBufFd(self, &buf, &size);
		else if (asm_is_token(fmt_str, "code", &token_len))
			MIPSInstDumpBufCode(self, &buf, &size);
		else
			fatal("%s: token not recognized\n", fmt_str);

		fmt_str += token_len;

	}
}


void MIPSInstDump(MIPSInst *self, FILE *f)
{
	char buf[200];

	MIPSInstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}


void MIPSInstDumpHex(MIPSInst *self, FILE *f)
{
	fprintf(f, "\n%8x:\t%08x \t", self->addr, self->bytes.word);
}
