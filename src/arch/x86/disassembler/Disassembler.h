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

#ifndef ARCH_X86_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_X86_DISASSEMBLER_DISASSEMBLER_H

#include <cassert>

#include <arch/common/Disassembler.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Error.h>

#include "Instruction.h"


namespace x86
{


class Disassembler : public comm::Disassembler
{
	// Disassemble a file
	static std::string path;

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
	static std::unique_ptr<Disassembler> instance;

	// Instruction information
	Instruction::Info inst_info[Instruction::OpcodeCount];

	// Decoding tables
	Instruction::DecodeInfo *dec_table[0x100];
	Instruction::DecodeInfo *dec_table_0f[0x100];

	// Look-up table returning true if a byte is an x86 prefix
	bool is_prefix[0x100];

	// For decoding table initialization
	void InsertInstInfo(Instruction::Info *info);
	void InsertInstInfo(Instruction::DecodeInfo **table,
			Instruction::DecodeInfo *elem, int at);

	// Free decoding tables
	void FreeInstDecodeInfo(Instruction::DecodeInfo *elem);

	// Private constructor for singleton
	Disassembler();

public:

	/// Exception for the x86 disassembler
	class Error : public misc::Error
	{
	public:
		
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("x86 disassembler");
		}
	};

	/// Register options in the command line
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Destructor
	~Disassembler();

	/// Get instance of singleton
	static Disassembler *getInstance();

	/// Get instruction information for a given opcode
	const Instruction::Info *getInstInfo(Instruction::Opcode opcode) const
	{
		assert(opcode >= 0 && opcode < Instruction::OpcodeCount);
		return &inst_info[opcode];
	}

	/// Return the main decoding table, indexed for instruction decoding
	/// using the first instruction byte.
	Instruction::DecodeInfo * const *getDecTable() const { return dec_table; }

	/// Return the secondary decoding table, indexed when the first byte
	/// of the instruction is 0x0f.
	Instruction::DecodeInfo * const *getDecTable0f() const { return dec_table_0f; }

	/// Return \c true if \a byte is a valid x86 instruction prefix.
	bool isPrefix(unsigned char byte) const { return is_prefix[byte]; }

	/// Disassemble the x86 ELF executable contained in file \a path, and
	/// dump its content into the output stream given in \c os (or the
	/// standard output if no output stream is specified.
	void DisassembleBinary(const std::string &path,
			std::ostream &os = std::cout) const;
};


}  // namespace x86

#endif

