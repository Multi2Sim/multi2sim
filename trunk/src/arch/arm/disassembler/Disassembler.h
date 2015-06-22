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

#include "Inst.h"


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
	InstInfo inst_info[256*16];

	/// Pointers to the tables of instructions Thumb16
	InstThumb16Info *dec_table_thumb16_asm;
	InstThumb16Info *dec_table_thumb16_shft_ins;
	InstThumb16Info *dec_table_thumb16_shft_ins_lv2;

	InstThumb16Info *dec_table_thumb16_asm_lv1;
	InstThumb16Info *dec_table_thumb16_asm_lv2;
	InstThumb16Info *dec_table_thumb16_asm_lv3;
	InstThumb16Info *dec_table_thumb16_asm_lv4;
	InstThumb16Info *dec_table_thumb16_asm_lv5;
	InstThumb16Info *dec_table_thumb16_asm_lv6;
	InstThumb16Info *dec_table_thumb16_asm_lv7;
	InstThumb16Info *dec_table_thumb16_asm_lv8;
	InstThumb16Info *dec_table_thumb16_asm_lv9;

	InstThumb16Info *dec_table_thumb16_data_proc;
	InstThumb16Info *dec_table_thumb16_spcl_data_brex;
	InstThumb16Info *dec_table_thumb16_spcl_data_brex_lv1;

	InstThumb16Info *dec_table_thumb16_ld_st;
	InstThumb16Info *dec_table_thumb16_ld_st_lv1;
	InstThumb16Info *dec_table_thumb16_ld_st_lv2;

	InstThumb16Info *dec_table_thumb16_misc;
	InstThumb16Info *dec_table_thumb16_it;

	/// Pointers to the tables of instructions Thumb32
	InstThumb32Info *dec_table_thumb32_asm;
	InstThumb32Info *dec_table_thumb32_asm_lv1;
	InstThumb32Info *dec_table_thumb32_asm_lv2;
	InstThumb32Info *dec_table_thumb32_asm_lv3;
	InstThumb32Info *dec_table_thumb32_asm_lv4;
	InstThumb32Info *dec_table_thumb32_asm_lv5;
	InstThumb32Info *dec_table_thumb32_asm_lv6;
	InstThumb32Info *dec_table_thumb32_asm_lv7;
	InstThumb32Info *dec_table_thumb32_asm_lv8;
	InstThumb32Info *dec_table_thumb32_asm_lv9;
	InstThumb32Info *dec_table_thumb32_asm_lv10;
	InstThumb32Info *dec_table_thumb32_asm_lv11;
	InstThumb32Info *dec_table_thumb32_asm_lv12;
	InstThumb32Info *dec_table_thumb32_asm_lv13;
	InstThumb32Info *dec_table_thumb32_asm_lv14;
	InstThumb32Info *dec_table_thumb32_asm_lv15;

	InstThumb32Info *dec_table_thumb32_asm_ldst_mul;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul1;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul2;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul3;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul4;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul5;
	InstThumb32Info *dec_table_thumb32_asm_ldst_mul6;

	InstThumb32Info *dec_table_thumb32_asm_ldst_dual;
	InstThumb32Info *dec_table_thumb32_asm_ldst1_dual;
	InstThumb32Info *dec_table_thumb32_asm_ldst2_dual;
	InstThumb32Info *dec_table_thumb32_asm_ldst3_dual;

	InstThumb32Info *dec_table_thumb32_dproc_shft_reg;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg1;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg2;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg3;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg4;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg5;
	InstThumb32Info *dec_table_thumb32_dproc_shft_reg6;

	InstThumb32Info *dec_table_thumb32_dproc_imm;
	InstThumb32Info *dec_table_thumb32_dproc_imm1;
	InstThumb32Info *dec_table_thumb32_dproc_imm2;
	InstThumb32Info *dec_table_thumb32_dproc_imm3;
	InstThumb32Info *dec_table_thumb32_dproc_imm4;
	InstThumb32Info *dec_table_thumb32_dproc_imm5;
	InstThumb32Info *dec_table_thumb32_dproc_imm6;

	InstThumb32Info *dec_table_thumb32_dproc_reg;
	InstThumb32Info *dec_table_thumb32_dproc_reg1;
	InstThumb32Info *dec_table_thumb32_dproc_reg2;
	InstThumb32Info *dec_table_thumb32_dproc_reg3;
	InstThumb32Info *dec_table_thumb32_dproc_reg4;
	InstThumb32Info *dec_table_thumb32_dproc_reg5;
	InstThumb32Info *dec_table_thumb32_dproc_reg6;
	InstThumb32Info *dec_table_thumb32_dproc_reg7;

	InstThumb32Info *dec_table_thumb32_dproc_misc;
	InstThumb32Info *dec_table_thumb32_dproc_misc1;

	InstThumb32Info *dec_table_thumb32_st_single;
	InstThumb32Info *dec_table_thumb32_st_single1;
	InstThumb32Info *dec_table_thumb32_st_single2;
	InstThumb32Info *dec_table_thumb32_st_single3;
	InstThumb32Info *dec_table_thumb32_st_single4;
	InstThumb32Info *dec_table_thumb32_st_single5;
	InstThumb32Info *dec_table_thumb32_st_single6;

	InstThumb32Info *dec_table_thumb32_ld_byte;
	InstThumb32Info *dec_table_thumb32_ld_byte1;
	InstThumb32Info *dec_table_thumb32_ld_byte2;
	InstThumb32Info *dec_table_thumb32_ld_byte3;
	InstThumb32Info *dec_table_thumb32_ld_byte4;
	InstThumb32Info *dec_table_thumb32_ld_byte5;
	InstThumb32Info *dec_table_thumb32_ld_byte6;

	InstThumb32Info *dec_table_thumb32_ld_hfword;
	InstThumb32Info *dec_table_thumb32_ld_hfword1;
	InstThumb32Info *dec_table_thumb32_ld_hfword2;

	InstThumb32Info *dec_table_thumb32_ld_word;
	InstThumb32Info *dec_table_thumb32_ld_word1;

	InstThumb32Info *dec_table_thumb32_mult;
	InstThumb32Info *dec_table_thumb32_mult1;

	InstThumb32Info *dec_table_thumb32_dproc_bin_imm;
	InstThumb32Info *dec_table_thumb32_dproc_bin_imm1;
	InstThumb32Info *dec_table_thumb32_dproc_bin_imm2;
	InstThumb32Info *dec_table_thumb32_dproc_bin_imm3;

	InstThumb32Info *dec_table_thumb32_mult_long;

	InstThumb32Info *dec_table_thumb32_brnch_ctrl;
	InstThumb32Info *dec_table_thumb32_brnch_ctrl1;

	InstThumb32Info *dec_table_thumb32_mov;
	InstThumb32Info *dec_table_thumb32_mov1;

	/// Return an instance of the singleton
	static Disassembler *getInstance();

	/// Destructor
	~Disassembler();

	/// Thumb Disassembler Functions
	int TestThumb32(const char *inst_ptr);
	void Thumb16InstDebugDump();
	void Thumb32InstDebugDump();
	void SetupInstInfo(const char* name, const char* fmt_str,
			InstCategory category, int arg1, int arg2, InstOpcode inst_name);
	void Thumb32SetupTable(const char* name ,const char* fmt_str ,
		InstThumb32Category cat32 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, int op7, int op8, InstThumb32Opcode inst_name);
	void Thumb16SetupTable(const char* name, const char* fmt_str ,
		InstThumb16Category cat16 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, InstThumb16Opcode inst_name);

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

