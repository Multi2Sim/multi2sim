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

#ifndef ARCH_KEPLER_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_KEPLER_DISASSEMBLER_DISASSEMBLER_H


#include <memory>

#include <arch/common/Disassembler.h>
#include <lib/cpp/CommandLine.h>

#include "Instruction.h"


namespace Kepler
{

class Disassembler : public comm::Disassembler
{
	// Instruction information
	Instruction::Info inst_info[Instruction::OpcodeCount];


	// Decoding tables

	Instruction::DecodeInfo dec_table[4];

	Instruction::DecodeInfo dec_table_a[8];
	Instruction::DecodeInfo dec_table_b[4];
	Instruction::DecodeInfo dec_table_c[4];

	Instruction::DecodeInfo dec_table_a_a[64];

	Instruction::DecodeInfo dec_table_b_a[2];
	Instruction::DecodeInfo dec_table_b_b[4];
	Instruction::DecodeInfo dec_table_b_c[4];
	Instruction::DecodeInfo dec_table_b_d[4];

	// DecodeInfo dec_table_b_c_a[2] is deleted due to FSET
	Instruction::DecodeInfo dec_table_b_c_b[2];
	Instruction::DecodeInfo dec_table_b_c_c[2];
	Instruction::DecodeInfo dec_table_b_c_d[4];

	Instruction::DecodeInfo dec_table_b_c_d_a[4];
	Instruction::DecodeInfo dec_table_b_c_d_b[4];
	Instruction::DecodeInfo dec_table_b_c_d_c[4];

	Instruction::DecodeInfo dec_table_b_c_d_c_a[2];
	Instruction::DecodeInfo dec_table_b_c_d_c_b[2];
	Instruction::DecodeInfo dec_table_b_c_d_c_c[2];

	Instruction::DecodeInfo dec_table_b_d_a[32];

	Instruction::DecodeInfo dec_table_c_a[4];
	Instruction::DecodeInfo dec_table_c_b[16];

	Instruction::DecodeInfo dec_table_c_a_a[2];
	Instruction::DecodeInfo dec_table_c_a_b[2];

	Instruction::DecodeInfo dec_table_c_b_a[2];
	Instruction::DecodeInfo dec_table_c_b_b[2];
	Instruction::DecodeInfo dec_table_c_b_c[4];
	Instruction::DecodeInfo dec_table_c_b_d[32];
	Instruction::DecodeInfo dec_table_c_b_e[4];
	// Instruction::DecodeInfo dec_table_c_b_f[1];

	Instruction::DecodeInfo dec_table_c_b_c_a[4];
	Instruction::DecodeInfo dec_table_c_b_c_b[4];
	Instruction::DecodeInfo dec_table_c_b_c_c[4];

	Instruction::DecodeInfo dec_table_c_b_c_c_a[2];
	Instruction::DecodeInfo dec_table_c_b_c_c_b[2];
	Instruction::DecodeInfo dec_table_c_b_c_c_c[2];

	Instruction::DecodeInfo dec_table_c_b_e_a[8];
	Instruction::DecodeInfo dec_table_c_b_e_b[2];
	Instruction::DecodeInfo dec_table_c_b_e_c[2];

	Instruction::DecodeInfo dec_table_c_b_e_a_a[8];
	Instruction::DecodeInfo dec_table_c_b_e_a_b[2];

	Instruction::DecodeInfo dec_table_c_b_e_a_a_a[2];
	Instruction::DecodeInfo dec_table_c_b_e_a_a_b[8];
	Instruction::DecodeInfo dec_table_c_b_e_a_a_c[2];
	Instruction::DecodeInfo dec_table_c_b_e_a_a_d[4];

	Instruction::DecodeInfo dec_table_c_b_e_a_a_c_a[2];

	Instruction::DecodeInfo dec_table_c_b_e_a_b_a[4];
	Instruction::DecodeInfo dec_table_c_b_e_a_b_b[16];

	Instruction::DecodeInfo dec_table_c_b_e_a_b_a_a[4];
	Instruction::DecodeInfo dec_table_c_b_e_a_b_a_b[2];
	Instruction::DecodeInfo dec_table_c_b_e_a_b_a_c[4];

	Instruction::DecodeInfo dec_table_c_b_e_b_a[2];
	Instruction::DecodeInfo dec_table_c_b_e_b_b[4];

	Instruction::DecodeInfo dec_table_c_b_e_b_a_a[16];

	// Global instance of the Kepler disassembler
	static std::unique_ptr<Disassembler> instance;

	// The path of cubin file disassembler gets from command line
	static std::string path;

	template<typename... Args> void InitTable(Instruction::Opcode opcode,
			const char *name, const char *fmt_str, Args&&... args)
	{
		int argv[sizeof...(args)];
		InitTableWithArray(opcode, name, fmt_str, 0,
				argv, args...);
	}

	template<typename... Args> void InitTableWithArray(Instruction::Opcode opcode,
			const char *name, const char *fmt_str, int argc,
			int argv[], int arg, Args&&... args)
	{
		argv[argc] = arg;
		InitTableWithArray(opcode, name, fmt_str, argc + 1, argv, args...);
	}
	
	void InitTableWithArray(Instruction::Opcode opcode, const char *name,
			const char *fmt_str, int argc, int argv[]);

	// Private constructor for singleton
	Disassembler();

public:

	/// Return instance of the singleton
	static Disassembler *getInstance();

	/// Return a pointer to the decoding table, which will be indexed by
	/// instruction bits for instruction decoding purposes.
	const Instruction::DecodeInfo *getDecTable() const { return dec_table; }

	/// Disassemble the \c .cubin file in \a path, using the same format as
	/// NVIDIA's \c cuobjdump tool.
	void DisassembleBinary(const std::string &path) const;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};

}  // namespace Kepler

#endif  // ARCH_KEPLER_ASM_ASM_H

