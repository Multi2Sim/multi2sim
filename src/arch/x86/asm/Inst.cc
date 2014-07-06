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

#include <arch/common/Asm.h>
#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"


namespace x86
{

// Register names
misc::StringMap inst_reg_map =
{
	{ "eax", InstRegEax },
	{ "ecx", InstRegEcx },
	{ "edx", InstRegEdx },
	{ "ebx", InstRegEbx },
	{ "esp", InstRegEsp },
	{ "ebp", InstRegEbp },
	{ "esi", InstRegEsi },
	{ "edi", InstRegEdi },
	{ "ax", InstRegAx },
	{ "cx", InstRegCx },
	{ "dx", InstRegDx },
	{ "bx", InstRegBx },
	{ "sp", InstRegSp },
	{ "bp", InstRegBp },
	{ "si", InstRegSi },
	{ "di", InstRegDi },
	{ "al", InstRegAl },
	{ "cl", InstRegCl },
	{ "dl", InstRegDl },
	{ "bl", InstRegBl },
	{ "ah", InstRegAh },
	{ "ch", InstRegCh },
	{ "dh", InstRegDh },
	{ "bh", InstRegBh },
	{ "es", InstRegEs },
	{ "cs", InstRegCs },
	{ "ss", InstRegSs },
	{ "ds", InstRegDs },
	{ "fs", InstRegFs },
	{ "gs", InstRegGs }
};


// Table indexed by pairs ModRM.mod and ModRM.rm, containing
// information about what will come next and effective address
// computation.
struct InstModRMTableEntry
{
	InstReg ea_base;
	int disp_size;
	int sib_size;
};


static const InstModRMTableEntry inst_modrm_table[32] =
{
	{ InstRegEax, 0, 0 },
	{ InstRegEcx, 0, 0 },
	{ InstRegEdx, 0, 0 },
	{ InstRegEbx, 0, 0 },
	{ InstRegNone, 0, 1 },
	{ InstRegNone, 4, 0 },
	{ InstRegEsi, 0, 0 },
	{ InstRegEdi, 0, 0 },

	{ InstRegEax, 1, 0 },
	{ InstRegEcx, 1, 0 },
	{ InstRegEdx, 1, 0 },
	{ InstRegEbx, 1, 0 },
	{ InstRegNone, 1, 1 },
	{ InstRegEbp, 1, 0 },
	{ InstRegEsi, 1, 0 },
	{ InstRegEdi, 1, 0 },

	{ InstRegEax, 4, 0 },
	{ InstRegEcx, 4, 0 },
	{ InstRegEdx, 4, 0 },
	{ InstRegEbx, 4, 0 },
	{ InstRegNone, 4, 1 },
	{ InstRegEbp, 4, 0 },
	{ InstRegEsi, 4, 0 },
	{ InstRegEdi, 4, 0 },

	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 },
	{ InstRegNone, 0, 0 }
};


// Table to obtain the scale from its decoded value
static const unsigned inst_ea_scale_table[4] = { 1, 2, 4, 8 };


Inst::Inst()
{
	as = Asm::getInstance();
	Clear();
}


void Inst::DumpMoffsAddr(std::ostream &os) const
{
	InstReg reg = segment ? segment : InstRegDs;
	os << misc::fmt("%s:0x%x", inst_reg_map.MapValue(reg), imm.d);
}


void Inst::DumpAddr(std::ostream &os) const
{
	// Segment
	assert(modrm_mod != 3);
	std::string segment_str;
	if (segment)
	{
		assert(segment >= 0 && segment < InstRegCount);
		segment_str = inst_reg_map.MapValue(segment);
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
		os << inst_reg_map.MapValue(ea_base);
		write_sign = true;
	}
	if (ea_index)
	{
		if (write_sign)
			os << '+';
		os << inst_reg_map.MapValue(ea_index);
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


void Inst::Dump(std::ostream &os) const
{
	// Instruction must have been decoded
	assert(decoded);

	// Get instruction information
	const InstInfo *info = as->getInstInfo(opcode);
	bool name_printed = false;
	const char *fmt = info->fmt;
	const char *fmt_first_arg = index(fmt, '_');
	int name_length = fmt_first_arg ? fmt_first_arg - fmt : strlen(fmt);

	// Dump instruction
	while (*fmt)
	{
		// Check tokens
		int length = 0;
		if (comm::Asm::isToken(fmt, "r8", length))
		{
			os << inst_reg_map.MapValue(modrm_reg
					+ InstRegAl);
		}
		else if (comm::Asm::isToken(fmt, "r16", length))
		{
			os << inst_reg_map.MapValue(modrm_reg
					+ InstRegAx);
		}
		else if (comm::Asm::isToken(fmt, "r32", length))
		{
			os << inst_reg_map.MapValue(modrm_reg
					+ InstRegEax);
		}
		else if (comm::Asm::isToken(fmt, "rm8", length))
		{
			if (modrm_mod == 0x03)
				os << inst_reg_map.MapValue(modrm_rm
						+ InstRegAl);
			else
			{
				os << "BYTE PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "rm16", length))
		{
			if (modrm_mod == 0x03)
				os << inst_reg_map.MapValue(modrm_rm
						+ InstRegAx);
			else
			{
				os << "WORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "rm32", length))
		{
			if (modrm_mod == 0x03)
				os << inst_reg_map.MapValue(modrm_rm
						+ InstRegEax);
			else
			{
				os << "DWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "r32m8", length))
		{
			if (modrm_mod == 3)
				os << inst_reg_map.MapValue(modrm_rm
						+ InstRegEax);
			else
			{
				os << "BYTE PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "r32m16", length))
		{
			if (modrm_mod == 3)
				os << inst_reg_map.MapValue(modrm_rm
						+ InstRegEax);
			else
			{
				os << "WORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "m", length))
		{
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "imm8", length))
		{
			os << misc::fmt("0x%x", imm.b);
		}
		else if (comm::Asm::isToken(fmt, "imm16", length))
		{
			os << misc::fmt("0x%x", imm.w);
		}
		else if (comm::Asm::isToken(fmt, "imm32", length))
		{
			os << misc::fmt("0x%x", imm.d);
		}
		else if (comm::Asm::isToken(fmt, "rel8", length))
		{
			os << misc::fmt("%x", (char) imm.b + eip + size);
		}
		else if (comm::Asm::isToken(fmt, "rel16", length))
		{
			os << misc::fmt("%x", (short) imm.w + eip + size);
		}
		else if (comm::Asm::isToken(fmt, "rel32", length))
		{
			os << misc::fmt("%x", imm.d + eip + size);
		}
		else if (comm::Asm::isToken(fmt, "moffs8", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "moffs16", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "moffs32", length))
		{
			DumpMoffsAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m8", length))
		{
			os << "BYTE PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m16", length))
		{
			os << "WORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m32", length))
		{
			os << "DWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m64", length))
		{
			os << "QWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m80", length))
		{
			os << "TBYTE PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "m128", length))
		{
			os << "XMMWORD PTR ";
			DumpAddr(os);
		}
		else if (comm::Asm::isToken(fmt, "st0", length))
		{
			os << "st";
		}
		else if (comm::Asm::isToken(fmt, "sti", length))
		{
			os << misc::fmt("st(%d)", opindex);
		}
		else if (comm::Asm::isToken(fmt, "ir8", length))
		{
			os << inst_reg_map.MapValue(opindex + InstRegAl);
		}
		else if (comm::Asm::isToken(fmt, "ir16", length))
		{
			os << inst_reg_map.MapValue(opindex + InstRegAx);
		}
		else if (comm::Asm::isToken(fmt, "ir32", length))
		{
			os << inst_reg_map.MapValue(opindex + InstRegEax);
		}
		else if (comm::Asm::isToken(fmt, "sreg", length))
		{
			os << inst_reg_map.MapValue(reg + InstRegEs);
		}
		else if (comm::Asm::isToken(fmt, "xmmm32", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "DWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "xmmm64", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "QWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "xmmm128", length))
		{
			if (modrm_mod == 3)
				os << "xmm" << (unsigned) modrm_rm;
			else
			{
				os << "XMMWORD PTR ";
				DumpAddr(os);
			}
		}
		else if (comm::Asm::isToken(fmt, "xmm", length))
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


void Inst::Clear()
{
	decoded = false;

	eip = 0;
	size = 0;
	opcode = InstOpcodeInvalid;
	format = nullptr;

	prefix_size = 0;
	opcode_size = 0;
	modrm_size = 0;
	sib_size = 0;
	disp_size = 0;
	imm_size = 0;

	opindex = 0;
	segment = InstRegNone;
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

	ea_base = InstRegNone;
	ea_index = InstRegNone;
	ea_scale = 0;

	reg = 0;
}


void Inst::Decode(const char *buffer, unsigned eip)
{
	// Initialize instruction
	Clear();
	decoded = true;
	this->eip = eip;
	op_size = 4;
	addr_size = 4;

	// Prefixes
	while (as->isPrefix(*buffer))
	{
		switch ((unsigned char) *buffer)
		{

		case 0xf0:
			// lock prefix is ignored
			break;

		case 0xf2:
			prefixes |= InstPrefixRepnz;
			break;

		case 0xf3:
			prefixes |= InstPrefixRep;
			break;

		case 0x66:
			prefixes |= InstPrefixOp;
			op_size = 2;
			break;

		case 0x67:
			prefixes |= InstPrefixAddr;
			addr_size = 2;
			break;

		case 0x2e:
			segment = InstRegCs;
			break;

		case 0x36:
			segment = InstRegSs;
			break;

		case 0x3e:
			segment = InstRegDs;
			break;

		case 0x26:
			segment = InstRegEs;
			break;

		case 0x64:
			segment = InstRegFs;
			break;

		case 0x65:
			segment = InstRegGs;
			break;

		default:
			
			throw Asm::Error(misc::fmt("Invalid prefix (0x%x)",
					(unsigned char) *buffer));

		}

		// One more prefix
		buffer++;
		prefix_size++;
	}

	// Obtain lookup table and index
	unsigned char buf8 = *buffer;
	unsigned buf32 = * (unsigned *) buffer;
	const InstDecodeInfo * const *table;
	int index;
	if (buf8 == 0x0f)
	{
		table = as->getDecTable0f();
		index = * (unsigned char *) (buffer + 1);
	}
	else
	{
		table = as->getDecTable();
		index = buf8;
	}

	// Find instruction
	const InstDecodeInfo *elem = nullptr;
	const InstInfo *info = nullptr;
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
		const InstModRMTableEntry *modrm_table_entry =
				&inst_modrm_table[(modrm_mod << 3) | modrm_rm];
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
			ea_scale = inst_ea_scale_table[sib_scale];
			ea_index = sib_index == 0x04 ? InstRegNone :
					(InstReg) (InstRegEax + sib_index);
			ea_base = (InstReg) (sib_base + InstRegEax);
			if (sib_base == 0x05 && modrm_mod == 0x00)
			{
				ea_base = InstRegNone;
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

