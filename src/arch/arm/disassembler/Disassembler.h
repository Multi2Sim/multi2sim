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

#ifndef ARCH_ARM_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_ARM_DISASSEMBLER_DISASSEMBLER_H


#include <iostream>
#include <memory>
#include <vector>

#include <arch/common/Disassembler.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/ELFReader.h>

#include "Instruction.h"


namespace ARM
{

/// ARM disassembler
class Disassembler : public comm::Disassembler
{
public:
	
	/// Disassembly Mode
	enum DisassemblyMode
	{
		DisassemblyModeArm = 1,
		DisassemblyModeThumb
	};

private:

	// File to disassemble
	static std::string path;

	// Unique instance of the singleton
	static std::unique_ptr<Disassembler> instance;

	// Private constructor for singleton
	Disassembler();

public:

	/// Exception for ARM disassembler
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("ARM Disassembler");
		}
	};

	/// ARM instruction table 
	Instruction::Info inst_info[256*16];

	/// Pointers to the tables of instructions Thumb16
	Instruction::Thumb16Info *dec_table_thumb16_asm;
	Instruction::Thumb16Info *dec_table_thumb16_shft_ins;
	Instruction::Thumb16Info *dec_table_thumb16_shft_ins_lv2;

	Instruction::Thumb16Info *dec_table_thumb16_asm_lv1;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv2;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv3;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv4;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv5;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv6;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv7;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv8;
	Instruction::Thumb16Info *dec_table_thumb16_asm_lv9;

	Instruction::Thumb16Info *dec_table_thumb16_data_proc;
	Instruction::Thumb16Info *dec_table_thumb16_spcl_data_brex;
	Instruction::Thumb16Info *dec_table_thumb16_spcl_data_brex_lv1;

	Instruction::Thumb16Info *dec_table_thumb16_ld_st;
	Instruction::Thumb16Info *dec_table_thumb16_ld_st_lv1;
	Instruction::Thumb16Info *dec_table_thumb16_ld_st_lv2;

	Instruction::Thumb16Info *dec_table_thumb16_misc;
	Instruction::Thumb16Info *dec_table_thumb16_it;

	/// Pointers to the tables of instructions Thumb32
	Instruction::Thumb32Info *dec_table_thumb32_asm;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv1;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv2;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv3;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv4;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv5;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv6;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv7;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv8;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv9;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv10;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv11;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv12;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv13;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv14;
	Instruction::Thumb32Info *dec_table_thumb32_asm_lv15;

	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul1;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul2;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul3;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul4;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul5;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_mul6;

	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst_dual;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst1_dual;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst2_dual;
	Instruction::Thumb32Info *dec_table_thumb32_asm_ldst3_dual;

	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg1;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg2;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg3;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg4;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg5;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_shft_reg6;

	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm1;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm2;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm3;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm4;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm5;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_imm6;

	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg1;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg2;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg3;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg4;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg5;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg6;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_reg7;

	Instruction::Thumb32Info *dec_table_thumb32_dproc_misc;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_misc1;

	Instruction::Thumb32Info *dec_table_thumb32_st_single;
	Instruction::Thumb32Info *dec_table_thumb32_st_single1;
	Instruction::Thumb32Info *dec_table_thumb32_st_single2;
	Instruction::Thumb32Info *dec_table_thumb32_st_single3;
	Instruction::Thumb32Info *dec_table_thumb32_st_single4;
	Instruction::Thumb32Info *dec_table_thumb32_st_single5;
	Instruction::Thumb32Info *dec_table_thumb32_st_single6;

	Instruction::Thumb32Info *dec_table_thumb32_ld_byte;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte1;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte2;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte3;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte4;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte5;
	Instruction::Thumb32Info *dec_table_thumb32_ld_byte6;

	Instruction::Thumb32Info *dec_table_thumb32_ld_hfword;
	Instruction::Thumb32Info *dec_table_thumb32_ld_hfword1;
	Instruction::Thumb32Info *dec_table_thumb32_ld_hfword2;

	Instruction::Thumb32Info *dec_table_thumb32_ld_word;
	Instruction::Thumb32Info *dec_table_thumb32_ld_word1;

	Instruction::Thumb32Info *dec_table_thumb32_mult;
	Instruction::Thumb32Info *dec_table_thumb32_mult1;

	Instruction::Thumb32Info *dec_table_thumb32_dproc_bin_imm;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_bin_imm1;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_bin_imm2;
	Instruction::Thumb32Info *dec_table_thumb32_dproc_bin_imm3;

	Instruction::Thumb32Info *dec_table_thumb32_mult_long;

	Instruction::Thumb32Info *dec_table_thumb32_brnch_ctrl;
	Instruction::Thumb32Info *dec_table_thumb32_brnch_ctrl1;

	Instruction::Thumb32Info *dec_table_thumb32_mov;
	Instruction::Thumb32Info *dec_table_thumb32_mov1;

	/// Return an instance of the singleton
	static Disassembler *getInstance();

	/// Destructor
	~Disassembler();

	/// Thumb Disassembler Functions
	int TestThumb32(const char *inst_ptr);
	void Thumb16InstDebugDump();
	void Thumb32InstDebugDump();
	void SetupInstInfo(const char* name, const char* fmt_str,
			Instruction::Category category, int arg1, int arg2, 
			Instruction::Opcode inst_name);
	void Thumb32SetupTable(const char* name ,const char* fmt_str ,
		Instruction::Thumb32Category cat32 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, int op7, int op8, 
		Instruction::Thumb32Opcode inst_name);
	void Thumb16SetupTable(const char* name, const char* fmt_str ,
		Instruction::Thumb16Category cat16 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, Instruction::Thumb16Opcode inst_name);

	/// ELF file processing function
	void ElfSymbolFunc(const ELFReader::File &file,
			std::ostream &os,
			unsigned int inst_addr,
			DisassemblyMode disasm_mode);

	unsigned int ElfDumpWordSymbol(const ELFReader::File &file,
			std::ostream &os,
			unsigned int inst_addr,
			unsigned int *inst_ptr);

	unsigned int ElfDumpThumbWordSymbol(const ELFReader::File &file,
			std::ostream &os,
			unsigned int inst_addr,
			unsigned int *inst_ptr);

	void ElfSymbolListCreate(const ELFReader::File &file,
			std::vector<ELFReader::Symbol*> &symbol_list);

	/// Disasseble Mode
	DisassemblyMode DissassembleMode(
			const std::vector<ELFReader::Symbol*> &symbol_list,
			unsigned int addr);

	/// Disassembler
	void DisassembleBinary(const std::string &path);

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};

} //namespace ARM

#endif // ARCH_ARM_ASM_ASM_H

