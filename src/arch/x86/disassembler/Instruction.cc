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

#include <cassert>
#include <cstring>

#include <arch/common/Disassembler.h>
#include <lib/cpp/Misc.h>

#include "Disassembler.h"
#include "Instruction.h"


namespace x86
{

const misc::StringMap Instruction::reg_map =
{
	{ "eax", RegEax },
	{ "ecx", RegEcx },
	{ "edx", RegEdx },
	{ "ebx", RegEbx },
	{ "esp", RegEsp },
	{ "ebp", RegEbp },
	{ "esi", RegEsi },
	{ "edi", RegEdi },
	{ "ax", RegAx },
	{ "cx", RegCx },
	{ "dx", RegDx },
	{ "bx", RegBx },
	{ "sp", RegSp },
	{ "bp", RegBp },
	{ "si", RegSi },
	{ "di", RegDi },
	{ "al", RegAl },
	{ "cl", RegCl },
	{ "dl", RegDl },
	{ "bl", RegBl },
	{ "ah", RegAh },
	{ "ch", RegCh },
	{ "dh", RegDh },
	{ "bh", RegBh },
	{ "es", RegEs },
	{ "cs", RegCs },
	{ "ss", RegSs },
	{ "ds", RegDs },
	{ "fs", RegFs },
	{ "gs", RegGs }
};

const Instruction::ModRMTableEntry Instruction::modrm_table[32] =
{
	{ RegEax, 0, 0 },
	{ RegEcx, 0, 0 },
	{ RegEdx, 0, 0 },
	{ RegEbx, 0, 0 },
	{ RegNone, 0, 1 },
	{ RegNone, 4, 0 },
	{ RegEsi, 0, 0 },
	{ RegEdi, 0, 0 },

	{ RegEax, 1, 0 },
	{ RegEcx, 1, 0 },
	{ RegEdx, 1, 0 },
	{ RegEbx, 1, 0 },
	{ RegNone, 1, 1 },
	{ RegEbp, 1, 0 },
	{ RegEsi, 1, 0 },
	{ RegEdi, 1, 0 },

	{ RegEax, 4, 0 },
	{ RegEcx, 4, 0 },
	{ RegEdx, 4, 0 },
	{ RegEbx, 4, 0 },
	{ RegNone, 4, 1 },
	{ RegEbp, 4, 0 },
	{ RegEsi, 4, 0 },
	{ RegEdi, 4, 0 },

	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 },
	{ RegNone, 0, 0 }
};


// Table to obtain the scale from its decoded value
static const unsigned ea_scale_table[4] = { 1, 2, 4, 8 };


Instruction::Instruction()
{
	disassembler = Disassembler::getInstance();
	Clear();
}


void Instruction::DumpMoffsAddr(std::ostream &os) const
{
	Reg reg = segment ? segment : RegDs;
	os << misc::fmt("%s:0x%x", reg_map.MapValue(reg), imm.d);
}


void Instruction::DumpAddr(std::ostream &os) const
{
	// Segment
	assert(modrm_mod != 3);
	std::string segment_str;
	if (segment)
	{
		assert(segment >= 0 && segment < RegCount);
		segment_str = reg_map.MapValue(segment);
		segment_str += ':';
	}

	// When there is only a displacement
	if (!ea_base && !ea_index)
	{
		if (segment_str.empty())
			segment_str = "ds:";
		os << segment_str;
		os << misc::fmt("0x%x", disp);
		return;
	}

	bool write_sign = false;
	os << segment_str << '[';
	if (ea_base)
	{
		os << reg_map.MapValue(ea_base);
		write_sign = true;
	}
	if (ea_index)
	{
		if (write_sign)
			os << '+';
		os << reg_map.MapValue(ea_index);
		if (ea_scale > 1)
			os << '*' << ea_scale;
		write_sign = true;
	}
	if (disp > 0)
	{
		if (write_sign)
			os << '+';
		os << misc::fmt("0x%x", disp);
	}
	if (disp < 0)
		os << misc::fmt("-0x%x", -disp);
	os << ']';
}


void Instruction::Dump(std::ostream &os) const
{
	// Instruction must have been decoded
	assert(decoded);

	// Get instruction information
	const Info *info = disassembler->getInstInfo(opcode);
	bool name_printed = false;
	const char *fmt = info->fmt;
	const char *fmt_first_arg = index(fmt, '_');
	int name_length = fmt_first_arg ? fmt_first_arg - fmt : strlen(fmt);

	// Dump instruction
	while (*fmt)
	{
		// Check tokens
		int length = 0;
		if (comm::Disassembler::isToken(fmt, "r8", length))
		{
			os << reg_map.MapValue(modrm_reg
					+ RegAl);
		}
		else if (comm::Disassembler::isToken(fmt, "r16", length))
		{
			os << reg_map.MapValue(modrm_reg
					+ RegAx);
		}
		else if (comm::Disassembler::isToken(fmt, "r32", length))
		{
			os << reg_map.MapValue(modrm_reg
					+ RegEax);
		}
		else if (comm::Disassembler::isToken(fmt, "rm8", length))
		{
			if (modrm_mod == 0x03)
				os << reg_map.MapValue(modrm_rm
						+ RegAl);
			else
			{
				os << "BYTE PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "rm16", length))
		{
			if (modrm_mod == 0x03)
				os << reg_map.MapValue(modrm_rm
						+ RegAx);
			else
			{
				os << "WORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "rm32", length))
		{
			if (modrm_mod == 0x03)
				os << reg_map.MapValue(modrm_rm
						+ RegEax);
			else
			{
				os << "DWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "r32m8", length))
		{
			if (modrm_mod == 3)
				os << reg_map.MapValue(modrm_rm
						+ RegEax);
			else
			{
				os << "BYTE PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "r32m16", length))
		{
			if (modrm_mod == 3)
				os << reg_map.MapValue(modrm_rm
						+ RegEax);
			else
			{
				os << "WORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "m", length))
		{
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "imm8", length))
		{
			os << misc::fmt("0x%x", imm.b);
		}
		else if (comm::Disassembler::isToken(fmt, "imm16", length))
		{
			os << misc::fmt("0x%x", imm.w);
		}
		else if (comm::Disassembler::isToken(fmt, "imm32", length))
		{
			os << misc::fmt("0x%x", imm.d);
		}
		else if (comm::Disassembler::isToken(fmt, "rel8", length))
		{
			os << misc::fmt("%x", (char) imm.b + eip + size);
		}
		else if (comm::Disassembler::isToken(fmt, "rel16", length))
		{
			os << misc::fmt("%x", (short) imm.w + eip + size);
		}
		else if (comm::Disassembler::isToken(fmt, "rel32", length))
		{
			os << misc::fmt("%x", imm.d + eip + size);
		}
		else if (comm::Disassembler::isToken(fmt, "moffs8", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "moffs16", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "moffs32", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m8", length))
		{
			os << "BYTE PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m16", length))
		{
			os << "WORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m32", length))
		{
			os << "DWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m64", length))
		{
			os << "QWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m80", length))
		{
			os << "TBYTE PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "m128", length))
		{
			os << "XMMWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Disassembler::isToken(fmt, "st0", length))
		{
			os << "st";
		}
		else if (comm::Disassembler::isToken(fmt, "sti", length))
		{
			os << misc::fmt("st(%d)", opindex);
		}
		else if (comm::Disassembler::isToken(fmt, "ir8", length))
		{
			os << reg_map.MapValue(opindex + RegAl);
		}
		else if (comm::Disassembler::isToken(fmt, "ir16", length))
		{
			os << reg_map.MapValue(opindex + RegAx);
		}
		else if (comm::Disassembler::isToken(fmt, "ir32", length))
		{
			os << reg_map.MapValue(opindex + RegEax);
		}
		else if (comm::Disassembler::isToken(fmt, "sreg", length))
		{
			os << reg_map.MapValue(reg + RegEs);
		}
		else if (comm::Disassembler::isToken(fmt, "xmmm32", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "DWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "xmmm64", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "QWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "xmmm128", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "XMMWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Disassembler::isToken(fmt, "xmm", length))
		{
			os << "xmm" << (unsigned) modrm_reg;
		}

		// Token was found, advance format string and continue
		if (length)
		{
			fmt += length;
			continue;
		}

		// Print literal alphanumerics
		while (*fmt && isalnum(*fmt))
			os << *fmt++;

		// Print literal non-alphanumerics
		while (*fmt && !isalnum(*fmt))
		{
			if (*fmt == '_')
			{
				if (name_printed)
				{
					os << ',';
				}
				else
				{
					name_printed = true;
					for (int i = 0; i < 6 - name_length; i++)
						os << ' ';
					os << ' ';
				}
			}
			else
			{
				os << *fmt;
			}
			fmt++;
		}
	}
}


void Instruction::Clear()
{
	decoded = false;

	eip = 0;
	size = 0;
	opcode = OpcodeInvalid;
	format = nullptr;

	prefix_size = 0;
	opcode_size = 0;
	modrm_size = 0;
	sib_size = 0;
	disp_size = 0;
	imm_size = 0;

	opindex = 0;
	segment = RegNone;
	prefixes = 0;

	op_size = 0;
	addr_size = 0;

	modrm = 0;
	modrm_mod = 0;
	modrm_reg = 0;
	modrm_rm = 0;

	sib = 0;
	sib_scale = 0;
	sib_index = 0;
	sib_base = 0;

	disp = 0;
	imm.d = 0;

	ea_base = RegNone;
	ea_index = RegNone;
	ea_scale = 0;

	reg = 0;
}


void Instruction::Decode(const char *buffer, unsigned eip)
{
	// Initialize instruction
	Clear();
	decoded = true;
	this->eip = eip;
	op_size = 4;
	addr_size = 4;

	// Prefixes
	while (disassembler->isPrefix(*buffer))
	{
		switch ((unsigned char) *buffer)
		{

		case 0xf0:
			// lock prefix is ignored
			break;

		case 0xf2:
			prefixes |= PrefixRepnz;
			break;

		case 0xf3:
			prefixes |= PrefixRep;
			break;

		case 0x66:
			prefixes |= PrefixOp;
			op_size = 2;
			break;

		case 0x67:
			prefixes |= PrefixAddr;
			addr_size = 2;
			break;

		case 0x2e:
			segment = RegCs;
			break;

		case 0x36:
			segment = RegSs;
			break;

		case 0x3e:
			segment = RegDs;
			break;

		case 0x26:
			segment = RegEs;
			break;

		case 0x64:
			segment = RegFs;
			break;

		case 0x65:
			segment = RegGs;
			break;

		default:
			
			throw Disassembler::Error(misc::fmt("Invalid prefix (0x%x)",
					(unsigned char) *buffer));

		}

		// One more prefix
		buffer++;
		prefix_size++;
	}

	// Obtain lookup table and index
	unsigned char buf8 = *buffer;
	unsigned buf32 = * (unsigned *) buffer;
	const DecodeInfo * const *table;
	int index;
	if (buf8 == 0x0f)
	{
		table = disassembler->getDecTable0f();
		index = * (unsigned char *) (buffer + 1);
	}
	else
	{
		table = disassembler->getDecTable();
		index = buf8;
	}

	// Find instruction
	const DecodeInfo *elem = nullptr;
	const Info *info = nullptr;
	for (elem = table[index]; elem; elem = elem->next)
	{
		info = elem->info;
		if (info->nomatch_mask && (buf32 & info->nomatch_mask) ==
				info->nomatch_result)
			continue;
		if ((buf32 & info->match_mask) == info->match_result
				&& info->prefixes == prefixes)
			break;
	}

	// Instruction not implemented
	if (!elem)
		return;

	// Instruction found
	format = info->fmt;
	opcode = info->opcode;
	opcode_size = info->opcode_size;
	modrm_size = info->modrm_size;
	opindex = (buf32 >> info->opindex_shift) & 0x7;
	buffer += opcode_size;  // Skip opcode

	// Decode the ModR/M field
	if (modrm_size)
	{
		// Split modrm into fields
		modrm = * (unsigned char *) buffer;
		modrm_mod = (modrm & 0xc0) >> 6;
		modrm_reg = (modrm & 0x38) >> 3;
		modrm_rm = modrm & 0x07;
		reg = modrm_reg;

		// Access ModRM table
		const ModRMTableEntry *modrm_table_entry =
				&modrm_table[(modrm_mod << 3) | modrm_rm];
		sib_size = modrm_table_entry->sib_size;
		disp_size = modrm_table_entry->disp_size;
		ea_base = modrm_table_entry->ea_base;
		buffer += modrm_size;  // Skip modrm

		// Decode SIB
		if (sib_size)
		{
			sib = * (unsigned char *) buffer;
			sib_scale = (sib & 0xc0) >> 6;
			sib_index = (sib & 0x38) >> 3;
			sib_base = sib & 0x07;
			ea_scale = ea_scale_table[sib_scale];
			ea_index = sib_index == 0x04 ? RegNone :
					(Reg) (RegEax + sib_index);
			ea_base = (Reg) (sib_base + RegEax);
			if (sib_base == 0x05 && modrm_mod == 0x00)
			{
				ea_base = RegNone;
				disp_size = 4;
			}
			buffer += sib_size;  // Skip SIB
		}

		// Decode Displacement
		switch (disp_size)
		{
		case 1:
			disp = *buffer;
			break;

		case 2:
			disp = * (short *) buffer;
			break;

		case 4:
			disp = * (int *) buffer;
			break;
		}
		buffer += disp_size;  // Skip disp
	}

	// Decode Immediate
	imm_size = info->imm_size;
	switch (imm_size)
	{
	case 1:
		imm.b = * (unsigned char *) buffer;
		break;

	case 2:
		imm.w = * (unsigned short *) buffer;
		break;

	case 4:
		imm.d = * (unsigned int *) buffer;
		break;
	}
	buffer += imm_size;  // Skip imm

	// Calculate total size
	size = prefix_size + opcode_size + modrm_size +
		sib_size + disp_size + imm_size;
}


} // namespace x86

