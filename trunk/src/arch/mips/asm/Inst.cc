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

#include <ext/stdio_filebuf.h>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <arch/common/Asm.h>
#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"

using namespace Misc;
using namespace std;
using namespace MIPS;



/* Register names */
#define MIPS_INST_REG_COUNT 32
static const char *inst_reg_name[MIPS_INST_REG_COUNT] =
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



Inst::Inst(Asm *as)
{
	this->as = as;
	bytes.word = 0;
	addr = 0;
	target = 0;
	info = NULL;
}


void Inst::Decode(unsigned int addr, void *buf)
{
	InstInfo *current_table;

	int current_table_low;
	int current_table_high;
	int loop_iteration;

	unsigned int table_arg;

	/* Store the instruction */
	bytes.word = * (unsigned int *) buf;
	this->addr = addr;
	target = 0;

	/* We start with the first table mips_asm_table, with the
	 * opcode field as argument */
	current_table = as->dec_table;
	current_table_low = 26;
	current_table_high = 31;
	loop_iteration = 0;
	table_arg = Misc::GetBit(bytes.word, current_table_high,
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
			table_arg = GetBit(bytes.word, current_table_high,
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
	info = &current_table[table_arg];
}


void Inst::DumpBufSa(char **buf_ptr, int *size_ptr)
{
	unsigned int sa;

	sa = bytes.standard.sa;
	if (sa != 0)
		str_printf(buf_ptr, size_ptr, "0x%x", sa);
	else
		str_printf(buf_ptr, size_ptr, "%d", sa);
}


void Inst::DumpBufRd(char **buf_ptr, int *size_ptr)
{
	unsigned int rd;
	int token_len;

	rd = bytes.standard.rd;

	if (Common::Asm::IsToken(info->name, "RDHWR", &token_len))
	{
		str_printf(buf_ptr, size_ptr, "$%d", rd);
	}
	else if (Common::Asm::IsToken(info->name, "JALR", &token_len))
	{
		if (rd == (int) 31)
		{
		}
		else
		{
			str_printf(buf_ptr, size_ptr, ",");
			str_printf(buf_ptr, size_ptr, "%s",
				inst_reg_name[rd]);
		}
	}
	else
		str_printf(buf_ptr, size_ptr, "%s",
			inst_reg_name[rd]);
}


void Inst::DumpBufRt(char **buf_ptr, int *size_ptr)
{
	unsigned int rt;

	rt = bytes.standard.rt;
	str_printf(buf_ptr, size_ptr, "%s", inst_reg_name[rt]);
}


void Inst::DumpBufRs(char **buf_ptr, int *size_ptr)
{
	unsigned int rs;

	rs = bytes.standard.rs;
	str_printf(buf_ptr, size_ptr, "%s", inst_reg_name[rs]);
}


void Inst::DumpBufTarget(char **buf_ptr, int *size_ptr)
{
	target = bytes.target.target;
	target = target << 2;
	target = target;
	str_printf(buf_ptr, size_ptr, "%x", target);
}


void Inst::DumpBufOffset(char **buf_ptr, int *size_ptr)
{
	unsigned int offset;

	offset = bytes.offset_imm.offset;
	if (offset & 0x8000)
		offset = -((offset ^ 0xffff) + 1);
	str_printf(buf_ptr, size_ptr, "%d", (int) offset);
}


void Inst::DumpBufOffsetbr(char **buf_ptr, int *size_ptr)
{
	unsigned int offsetbr;

	offsetbr = bytes.cc.offsetbr;
	if (offsetbr & 0x8000)
		offsetbr = -((offsetbr ^ 0xffff) + 1);
	offsetbr = (offsetbr << 2) + addr + 4;
	target = offsetbr;
	str_printf(buf_ptr, size_ptr, "%x", offsetbr);
}


void Inst::DumpBufImm(char **buf_ptr, int *size_ptr)
{
	unsigned int imm;

	imm = bytes.offset_imm.offset;
	if (imm & 0x8000)
		imm = -((imm ^ 0xffff) + 1);
	str_printf(buf_ptr, size_ptr, "%d", (int) imm);
}


void Inst::DumpBufImmhex(char **buf_ptr, int *size_ptr)
{
	unsigned int immhex;

	immhex = bytes.offset_imm.offset;
	str_printf(buf_ptr, size_ptr, "0x%x", immhex);
}


void Inst::DumpBufBase(char **buf_ptr, int *size_ptr)
{
	unsigned int base;

	base = bytes.offset_imm.base;
	str_printf(buf_ptr, size_ptr, "%s", inst_reg_name[base]);
}


void Inst::DumpBufSel(char **buf_ptr, int *size_ptr)
{
	unsigned int sel;

	sel = bytes.sel.sel;
	str_printf(buf_ptr, size_ptr, "%d", sel);
}


void Inst::DumpBufCc(char **buf_ptr, int *size_ptr)
{
	unsigned int cc;

	cc = bytes.cc.cc;
	str_printf(buf_ptr, size_ptr, "$fcc%d", cc);
}


void Inst::DumpBufPos(char **buf_ptr, int *size_ptr)
{
	unsigned int pos;

	pos = bytes.standard.sa;
	str_printf(buf_ptr, size_ptr, "0x%x", pos);
}


void Inst::DumpBufFs(char **buf_ptr, int *size_ptr)
{
	unsigned int fs;
	int token_len;

	fs = bytes.standard.rd;
	if ((Common::Asm::IsToken(info->name, "CFC1", &token_len)) ||
		(Common::Asm::IsToken(info->name, "CTC1", &token_len)))
		str_printf(buf_ptr, size_ptr, "$%d", fs);
	else
		str_printf(buf_ptr, size_ptr, "$f%d", fs);
}


void Inst::DumpBufSize(char **buf_ptr, int *size_ptr)
{
	unsigned int size;
	unsigned int pos;

	int token_len;

	pos = bytes.standard.sa;
	size = bytes.standard.rd;

	if (Common::Asm::IsToken(info->name, "INS", &token_len))
		size = size + 1 - pos;
	else if (Common::Asm::IsToken(info->name, "EXT", &token_len))
		size++;
	str_printf(buf_ptr, size_ptr, "0x%x", size);
}


void Inst::DumpBufFt(char **buf_ptr, int *size_ptr)
{
	unsigned int ft;

	ft = bytes.standard.rt;
	str_printf(buf_ptr, size_ptr, "$f%d", ft);
}


void Inst::DumpBufFd(char **buf_ptr, int *size_ptr)
{
	unsigned int fd;

	fd = bytes.standard.sa;
	str_printf(buf_ptr, size_ptr, "$f%d", fd);
}


void Inst::DumpBufCode(char **buf_ptr, int *size_ptr)
{
	unsigned int code;

	code = bytes.code.code;
	str_printf(buf_ptr, size_ptr, "0x%x", code);
}


void Inst::DumpBuf(char *buf, int size)
{
	char *orig_buf = buf;
	const char *fmt_str;

	int i = 0;
	int token_len;

	unsigned int rd;
	unsigned int rt;
	unsigned int rs;
	unsigned int sa;

	/* Get format string */
	fmt_str = info->fmt_str;
	if (!fmt_str || !*fmt_str)
		fatal("Instruction not implememted.\n");

	/* Traverse format string */
	while (*fmt_str)
	{
		rd = bytes.standard.rd;
		rt = bytes.standard.rt;
		rs = bytes.standard.rs;
		sa = bytes.standard.sa;

		/*
		 *  DEAL WITH PSEUDO INSTRUCTIONS
		 */

		/* SLL ZERO, ZERO, ZERO => NOP */
		if (Common::Asm::IsToken(fmt_str, "sll", &token_len))
		{
			if ((rd | rt | sa) == 0)
			{
				str_printf(&buf, &size, "nop");
				break;
			}
		}
		/* ADDU RD, RS, ZERO => MOVE RD, RS */
		else if (Common::Asm::IsToken(fmt_str, "addu", &token_len))
		{
			if (rt == 0)
			{
				str_printf(&buf, &size, "move\t");
				DumpBufRd(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufRs(&buf, &size);
				break;
			}
		}
		/* BGEZAL ZERO, OFFSET => BAL OFFSET */
		else if (Common::Asm::IsToken(fmt_str, "bgezal",
				&token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "bal\t");
				DumpBufOffsetbr(&buf, &size);
				break;
			}
		}
		/* BEQ ZERO, ZERO, OFFSET => B OFFSET */
		/* BEQ RS, ZERO, OFFSET => BEQZ RS, OFFSET */
		else if (Common::Asm::IsToken(fmt_str, "beq", &token_len))
		{
			if ((rs | rt) == 0)
			{
				str_printf(&buf, &size, "b\t");
				DumpBufOffsetbr(&buf, &size);
				break;
			}
			else if (rt == 0)
			{
				str_printf(&buf, &size, "beqz\t");
				DumpBufRs(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufOffsetbr(&buf, &size);
				break;
			}
		}
		/* ADDIU RT, ZERO, IMM => LI RT, IMM */
		else if (Common::Asm::IsToken(fmt_str, "addiu",
				&token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "li\t");
				DumpBufRt(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufImm(&buf, &size);
				break;
			}
		}
		/* ORI RT, ZERO, IMM => LI RT, IMM */
		else if (Common::Asm::IsToken(fmt_str, "ori", &token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "li\t");
				DumpBufRt(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufImmhex(&buf, &size);
				break;
			}
		}
		/* BNE RS, ZERO, OFFSET => BNEZ RS, OFFSET */
		else if (Common::Asm::IsToken(fmt_str, "bne", &token_len))
		{
			if (rt == 0)
			{
				str_printf(&buf, &size, "bnez\t");
				DumpBufRs(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufOffsetbr(&buf, &size);
				break;
			}
		}
		/* SUBU RD, ZERO, RT => NEGU RD, RT */
		else if (Common::Asm::IsToken(fmt_str, "subu", &token_len))
		{
			if (rs == 0)
			{
				str_printf(&buf, &size, "negu\t");
				DumpBufRd(&buf, &size);
				str_printf(&buf, &size, ",");
				DumpBufRt(&buf, &size);
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
		if (Common::Asm::IsToken(fmt_str, "sa", &token_len))
			DumpBufSa(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "rd", &token_len))
			DumpBufRd(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "rt", &token_len))
			DumpBufRt(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "rs", &token_len))
			DumpBufRs(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "target",
				&token_len))
			DumpBufTarget(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "offset",
				&token_len))
			DumpBufOffset(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "offsetbr",
				&token_len))
			DumpBufOffsetbr(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "Imm", &token_len))
			DumpBufImm(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "Immhex",
				&token_len))
			DumpBufImmhex(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "base", &token_len))
			DumpBufBase(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "sel", &token_len))
			DumpBufSel(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "cc", &token_len))
			DumpBufCc(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "pos", &token_len))
			DumpBufPos(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "size", &token_len))
			DumpBufSize(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "fs", &token_len))
			DumpBufFs(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "ft", &token_len))
			DumpBufFt(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "fd", &token_len))
			DumpBufFd(&buf, &size);
		else if (Common::Asm::IsToken(fmt_str, "code", &token_len))
			DumpBufCode(&buf, &size);
		else
			fatal("%s: token not recognized\n", fmt_str);

		fmt_str += token_len;

	}
}


void Inst::Dump(std::ostream &os)
{
	char buf[200];

	DumpBuf(buf, sizeof buf);
	os << buf;
}


void Inst::DumpHex(std::ostream &os)
{
	os << "\n" << setfill(' ') << setw(8) << hex << addr <<
			":\t" << setfill('0') << setw(8) << hex <<
			bytes.word << " \t" << setfill(' ');
}




/*
 * C Wrapper
 */

struct MIPSInstWrap *MIPSInstWrapCreate(struct MIPSAsmWrap *as)
{
	return (MIPSInstWrap *) new Inst((Asm *) as);
}


void MIPSInstWrapFree(struct MIPSInstWrap *self)
{
	delete (Inst *) self;
}


void MIPSInstWrapCopy(struct MIPSInstWrap *left, struct MIPSInstWrap *right)
{
	Inst *ileft = (Inst *) left;
	Inst *iright = (Inst *) right;
	*ileft = *iright;
}


void MIPSInstWrapDecode(struct MIPSInstWrap *self, unsigned int addr, void *buf)
{
	Inst *inst = (Inst *) self;
	inst->Decode(addr, buf);
}


void MIPSInstWrapDump(struct MIPSInstWrap *self, FILE *f)
{
	Inst *inst = (Inst *) self;
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	ostream os(&filebuf);
	inst->Dump(os);
}


MIPSInstOpcode MIPSInstWrapGetOpcode(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (MIPSInstOpcode) inst->GetOpcode();
}


const char *MIPSInstWrapGetName(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->GetName().c_str();
}


unsigned int MIPSInstWrapGetAddress(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->GetAddress();
}


MIPSInstBytes *MIPSInstWrapGetBytes(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (MIPSInstBytes *) inst->GetBytes();
}
