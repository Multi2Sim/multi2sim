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

#include <fstream>
#include <iomanip>
#include <iostream>

#include <arch/common/Disassembler.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Disassembler.h"
#include "Inst.h"


namespace MIPS
{


// Register names
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



Inst::Inst()
{
	this->disassembler = Disassembler::getInstance();
	bytes.word = 0;
	addr = 0;
	target = 0;
	info = NULL;
}


void Inst::Decode(unsigned addr, const char *buf)
{
	InstInfo *current_table;

	int current_table_low;
	int current_table_high;
	int loop_iteration;

	unsigned table_arg;

	// Store the instruction
	bytes.word = * (unsigned *) buf;
	this->addr = addr;
	target = 0;

	/* We start with the first table mips_asm_table, with the
	 * opcode field as argument */
	current_table = disassembler->dec_table;
	current_table_low = 26;
	current_table_high = 31;
	loop_iteration = 0;
	table_arg = misc::getBits32(bytes.word, current_table_high,
			current_table_low);

	// Find next tables if the instruction belongs to another table
	while (1)
	{
		if (current_table[table_arg].next_table
				&& loop_iteration < 4)
		{
			current_table_high = current_table[table_arg].next_table_high;
			current_table_low = current_table[table_arg].next_table_low;
			current_table = current_table[table_arg].next_table;
			table_arg = misc::getBits32(bytes.word, current_table_high,
					current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 4)
		{
			throw Disassembler::Error(misc::fmt("Invalid instruction at "
					"0x%x", addr));
		}
		else
		{
			break;
		}
	}

	// Instruction found
	info = &current_table[table_arg];
}


void Inst::DumpSa(std::ostream &os) const
{
	unsigned sa;

	sa = bytes.standard.sa;
	if (sa != 0)
		os << misc::fmt("0x%x", sa);
	else
		os << '0';
}


void Inst::DumpRd(std::ostream &os) const
{
	unsigned rd;
	int token_len;

	rd = bytes.standard.rd;

	if (comm::Disassembler::isToken(info->name, "RDHWR", token_len))
	{
		os << '$' << rd;
	}
	else if (comm::Disassembler::isToken(info->name, "JALR", token_len))
	{
		if (rd != 31)
			os << ',' << inst_reg_name[rd];
	}
	else
		os << inst_reg_name[rd];
}


void Inst::DumpRt(std::ostream &os) const
{
	unsigned rt = bytes.standard.rt;
	os << inst_reg_name[rt];
}


void Inst::DumpRs(std::ostream &os) const
{
	unsigned rs = bytes.standard.rs;
	os << inst_reg_name[rs];
}


void Inst::DumpTarget(std::ostream &os)
{
	target = bytes.target.target << 2;
	os << misc::fmt("%x", target);
}


void Inst::DumpOffset(std::ostream &os) const
{
	int offset = misc::SignExtend32(bytes.offset_imm.offset, 16);
	os << offset;
}


void Inst::DumpOffsetbr(std::ostream &os)
{
	int offsetbr = misc::SignExtend32(bytes.cc.offsetbr, 16);
	offsetbr = (offsetbr << 2) + addr + 4;
	target = offsetbr;
	os << misc::fmt("%x", offsetbr);
}


void Inst::DumpImm(std::ostream &os) const
{
	int imm = misc::SignExtend32(bytes.offset_imm.offset, 16);
	os << imm;
}


void Inst::DumpImmhex(std::ostream &os) const
{
	unsigned immhex = bytes.offset_imm.offset;
	os << misc::fmt("0x%x", immhex);
}


void Inst::DumpBase(std::ostream &os) const
{
	unsigned base = bytes.offset_imm.base;
	os << inst_reg_name[base];
}


void Inst::DumpSel(std::ostream &os) const
{
	unsigned sel = bytes.sel.sel;
	os << sel;
}


void Inst::DumpCc(std::ostream &os) const
{
	unsigned cc = bytes.cc.cc;
	os << "$fcc" << cc;
}


void Inst::DumpPos(std::ostream &os) const
{
	unsigned pos = bytes.standard.sa;
	os << misc::fmt("0x%x", pos);
}


void Inst::DumpFs(std::ostream &os) const
{
	unsigned fs = bytes.standard.rd;
	if ((comm::Disassembler::isToken(info->name, "CFC1")) ||
			(comm::Disassembler::isToken(info->name, "CTC1")))
		os << '$' << fs;
	else
		os << "$f" << fs;
}


void Inst::DumpSize(std::ostream &os) const
{
	unsigned pos = bytes.standard.sa;
	unsigned size = bytes.standard.rd;

	if (comm::Disassembler::isToken(info->name, "INS"))
		size = size + 1 - pos;
	else if (comm::Disassembler::isToken(info->name, "EXT"))
		size++;
	os << misc::fmt("0x%x", size);
}


void Inst::DumpFt(std::ostream &os) const
{
	unsigned ft = bytes.standard.rt;
	os << "$f" << ft;
}


void Inst::DumpFd(std::ostream &os) const
{
	unsigned fd = bytes.standard.sa;
	os << "$f" << fd;
}


void Inst::DumpCode(std::ostream &os) const
{
	unsigned code = bytes.code.code;
	os << misc::fmt("0x%x", code);
}


void Inst::DumpHint(std::ostream &os) const
{
	unsigned hint = bytes.standard.rs;
	os << misc::fmt("0x%x", hint);
}


void Inst::Dump(std::ostream &os)
{
	int token_len;

	// Get format string
	const char *fmt_str = info->fmt_str;
	if (!fmt_str || !*fmt_str)
		throw misc::Panic("Instruction not implemented");

	// Traverse format string
	bool first_token = true;
	while (*fmt_str)
	{
		unsigned rd = bytes.standard.rd;
		unsigned rt = bytes.standard.rt;
		unsigned rs = bytes.standard.rs;
		unsigned sa = bytes.standard.sa;

		//
		//  PSEUDO INSTRUCTIONS
		//

		// SLL ZERO, ZERO, ZERO => NOP
		if (comm::Disassembler::isToken(fmt_str, "sll", token_len))
		{
			if ((rd | rt | sa) == 0)
			{
				os << "nop";
				break;
			}
		}
		// ADDU RD, RS, ZERO => MOVE RD, RS
		else if (comm::Disassembler::isToken(fmt_str, "addu", token_len))
		{
			if (rt == 0)
			{
				os << "move\t";
				DumpRd(os);
				os << ',';
				DumpRs(os);
				break;
			}
		}
		// BGEZAL ZERO, OFFSET => BAL OFFSET
		else if (comm::Disassembler::isToken(fmt_str, "bgezal", token_len))
		{
			if (rs == 0)
			{
				os << "bal\t";
				DumpOffsetbr(os);
				break;
			}
		}
		// BEQ ZERO, ZERO, OFFSET => B OFFSET
		// BEQ RS, ZERO, OFFSET => BEQZ RS, OFFSET
		else if (comm::Disassembler::isToken(fmt_str, "beq", token_len))
		{
			if ((rs | rt) == 0)
			{
				os << "b\t";
				DumpOffsetbr(os);
				break;
			}
			else if (rt == 0)
			{
				os << "beqz\t";
				DumpRs(os);
				os << ',';
				DumpOffsetbr(os);
				break;
			}
		}
		// ADDIU RT, ZERO, IMM => LI RT, IMM
		else if (comm::Disassembler::isToken(fmt_str, "addiu",
				token_len))
		{
			if (rs == 0)
			{
				os << "li\t";
				DumpRt(os);
				os << ',';
				DumpImm(os);
				break;
			}
		}
		// ORI RT, ZERO, IMM => LI RT, IMM
		else if (comm::Disassembler::isToken(fmt_str, "ori", token_len))
		{
			if (rs == 0)
			{
				os << "li\t";
				DumpRt(os);
				os << ',';
				DumpImmhex(os);
				break;
			}
		}
		// BNE RS, ZERO, OFFSET => BNEZ RS, OFFSET
		else if (comm::Disassembler::isToken(fmt_str, "bne", token_len))
		{
			if (rt == 0)
			{
				os << "bnez\t";
				DumpRs(os);
				os << ',';
				DumpOffsetbr(os);
				break;
			}
		}
		// SUBU RD, ZERO, RT => NEGU RD, RT
		else if (comm::Disassembler::isToken(fmt_str, "subu", token_len))
		{
			if (rs == 0)
			{
				os << "negu\t";
				DumpRd(os);
				os << ',';
				DumpRt(os);
				break;
			}
		}
		// DONE WITH PSEUDO INSTRUCTIONS

		if (*fmt_str != '%')
		{
			if (*fmt_str == '_')
				os << '.';
			else if (*fmt_str != ' ')
				os << *fmt_str;
			++fmt_str;
			continue;
		}

		if (first_token)
			os << '\t';
		first_token = false;
		++fmt_str;
		if (comm::Disassembler::isToken(fmt_str, "sa", token_len))
			DumpSa(os);
		else if (comm::Disassembler::isToken(fmt_str, "rd", token_len))
			DumpRd(os);
		else if (comm::Disassembler::isToken(fmt_str, "rt", token_len))
			DumpRt(os);
		else if (comm::Disassembler::isToken(fmt_str, "rs", token_len))
			DumpRs(os);
		else if (comm::Disassembler::isToken(fmt_str, "target",
				token_len))
			DumpTarget(os);
		else if (comm::Disassembler::isToken(fmt_str, "offset",
				token_len))
			DumpOffset(os);
		else if (comm::Disassembler::isToken(fmt_str, "offsetbr",
				token_len))
			DumpOffsetbr(os);
		else if (comm::Disassembler::isToken(fmt_str, "Imm", token_len))
			DumpImm(os);
		else if (comm::Disassembler::isToken(fmt_str, "Immhex",
				token_len))
			DumpImmhex(os);
		else if (comm::Disassembler::isToken(fmt_str, "base", token_len))
			DumpBase(os);
		else if (comm::Disassembler::isToken(fmt_str, "sel", token_len))
			DumpSel(os);
		else if (comm::Disassembler::isToken(fmt_str, "cc", token_len))
			DumpCc(os);
		else if (comm::Disassembler::isToken(fmt_str, "pos", token_len))
			DumpPos(os);
		else if (comm::Disassembler::isToken(fmt_str, "size", token_len))
			DumpSize(os);
		else if (comm::Disassembler::isToken(fmt_str, "fs", token_len))
			DumpFs(os);
		else if (comm::Disassembler::isToken(fmt_str, "ft", token_len))
			DumpFt(os);
		else if (comm::Disassembler::isToken(fmt_str, "fd", token_len))
			DumpFd(os);
		else if (comm::Disassembler::isToken(fmt_str, "code", token_len))
			DumpCode(os);
		else if (comm::Disassembler::isToken(fmt_str, "hint", token_len))
			DumpHint(os);
		else
			throw misc::Panic(misc::fmt("%s: Unrecognized "
					"token", fmt_str));

		fmt_str += token_len;

	}
}


void Inst::DumpHex(std::ostream &os)
{
	os << misc::fmt("\n%8x:\t%08x \t", addr, bytes.word);
}


} // namespace MIPS


