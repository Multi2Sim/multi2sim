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

#ifndef ARCH_MIPS_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_MIPS_DISASSEMBLER_DISASSEMBLER_H

#include <memory>

#include <arch/common/Disassembler.h>
#include <lib/cpp/Error.h>

#include "Instruction.h"


namespace MIPS
{


class Disassembler : public comm::Disassembler
{
	// File to disassemble
	static std::string path;

	// Unique instance of the singleton
	static std::unique_ptr<Disassembler> instance;

	// Private constructor for singleton
	Disassembler();

public:

	/// Exception for MIPS disassembler
	class Error : public misc::Error
	{
	public:
		
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("MIPS Disassembler");
		}
	};

	// Decoding tables
	Instruction::Info *dec_table;
	Instruction::Info *dec_table_special;
	Instruction::Info *dec_table_special_movci;
	Instruction::Info *dec_table_special_srl;
	Instruction::Info *dec_table_special_srlv;

	Instruction::Info *dec_table_regimm;

	Instruction::Info *dec_table_cop0;
	Instruction::Info *dec_table_cop0_c0;
	Instruction::Info *dec_table_cop0_notc0;
	Instruction::Info *dec_table_cop0_notc0_mfmc0;

	Instruction::Info *dec_table_cop1;
	Instruction::Info *dec_table_cop1_bc1;
	Instruction::Info *dec_table_cop1_s;
	Instruction::Info *dec_table_cop1_s_movcf;
	Instruction::Info *dec_table_cop1_d;
	Instruction::Info *dec_table_cop1_d_movcf;
	Instruction::Info *dec_table_cop1_w;
	Instruction::Info *dec_table_cop1_l;
	Instruction::Info *dec_table_cop1_ps;

	Instruction::Info *dec_table_cop2;
	Instruction::Info *dec_table_cop2_bc2;

	Instruction::Info *dec_table_special2;

	Instruction::Info *dec_table_special3;
	Instruction::Info *dec_table_special3_bshfl;
	
	
	/// Return an instance of the singleton
	static Disassembler *getInstance();

	/// Destructor
	~Disassembler();
	
	/// Disassemble binary file given a path
	void DisassembleBinary(const std::string &path);

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};



}  // namespace MIPS


#endif  // MIPS_ASM_ASM_H

