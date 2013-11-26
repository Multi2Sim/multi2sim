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

#ifndef ARCH_X86_ASM_ASM_H
#define ARCH_X86_ASM_ASM_H

#include <cassert>

#include <lib/cpp/CommandLine.h>

#include "Inst.h"


namespace x86
{


class AsmConfig : public misc::CommandLineConfig
{
	std::string path;
public:
	/// Register command-line options related with x86 disassembler
	void Register(misc::CommandLine &command_line);

	/// Process command-line options related with the x86 disassembler
	void Process();
};


class Asm
{
	// Constants used in asm.dat

	// For fields 'op1', 'op2', 'modrm', 'imm'
	static const int SKIP = 0x0100;

	// For field 'modrm'
	static const int REG = 0x0200;
	static const int MEM = 0x0400;

	// For fields 'op1', 'op2'
	static const int INDEX = 0x1000;

	// For 'imm' field
	static const int IB = 0x2000;
	static const int IW = 0x4000;
	static const int ID = 0x8000;

	// Unique instance of x86 disassembler
	static std::unique_ptr<Asm> instance;

	// Instruction information
	InstInfo inst_info[InstOpcodeCount];

	// Decoding tables
	InstDecodeInfo *dec_table[0x100];
	InstDecodeInfo *dec_table_0f[0x100];

	// Look-up table returning true if a byte is an x86 prefix
	bool is_prefix[0x100];

	// For decoding table initialization
	void InsertInstInfo(InstInfo *info);
	void InsertInstInfo(InstDecodeInfo **table,
			InstDecodeInfo *elem, int at);

	// Free decoding tables
	void FreeInstDecodeInfo(InstDecodeInfo *elem);

	// Constructor. Made public to avoid any external instantiations of the
	// x86 disassembler. Instead, the only instance can be obtain with a
	// call to getInstance().
	Asm();

public:

	/// Destructor
	~Asm();

	/// Return a unique instance of the x86 disassembler. This instance will
	/// be created only upon the first call to getInstance(), and will be
	/// alive util the end of the execution.
	static Asm *getInstance();

	/// Get instruction information for a given opcode
	const InstInfo *getInstInfo(InstOpcode opcode) const {
		assert(opcode >= 0 && opcode < InstOpcodeCount);
		return &inst_info[opcode];
	}

	/// Return the main decoding table, indexed for instruction decoding
	/// using the first instruction byte.
	InstDecodeInfo * const *getDecTable() const { return dec_table; }

	/// Return the secondary decoding table, indexed when the first byte
	/// of the instruction is 0x0f.
	InstDecodeInfo * const *getDecTable0f() const { return dec_table_0f; }

	/// Return \c true if \a byte is a valid x86 instruction prefix.
	bool isPrefix(unsigned char byte) const { return is_prefix[byte]; }

	/// Disassemble the x86 ELF executable contained in file \a path, and
	/// dump its content into the output stream given in \c os (or the
	/// standard output if no output stream is specified.
	void DisassembleBinary(const std::string &path,
			std::ostream &os = std::cout) const;

	/// Configuration for x86 disassembler
	static AsmConfig config;
};


}  // namespace x86

#endif

