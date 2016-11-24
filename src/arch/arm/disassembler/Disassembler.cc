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

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/String.h>

#include "Disassembler.h"
#include "Instruction.h"


namespace ARM
{

//
// Configuration options
//
std::string Disassembler::path;

void Disassembler::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Set category
	command_line->setCategory("ARM");

	// Option --mips-disasm <file>
	command_line->RegisterString("--arm-disasm", path,
			"Disassemble the ARM ELF file provided in <arg>, "
			"using the internal ARM disassembler. This option is "
			"incompatible with any other option.");

	// Incompatible options
	command_line->setIncompatible("--arm-disasm");
}


void Disassembler::ProcessOptions()
{
	// Run ARM disassembler
	if (!path.empty())
	{
		Disassembler *disassembler = Disassembler::getInstance();
		disassembler->DisassembleBinary(path);
		exit(0);
	}
}


Disassembler::Disassembler() : comm::Disassembler("ARM")
{
	// Form the Instruction table and read Information from table
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	SetupInstInfo(#_name, _fmt_str, Instruction::Category##_category, \
			_arg1, _arg2, Instruction::Opcode##_name);
#include "Instruction.def"
#undef DEFINST


	// Allocate Memory 
	dec_table_thumb32_asm			= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv1		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv2		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv3		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv4		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv5		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv6		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv7		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv8		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv9		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv10		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv11		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv12		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv13		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv14		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_lv15		= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_asm_ldst_mul		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul1		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul2		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul3		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul4		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul5		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst_mul6		= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_asm_ldst_dual		= new Instruction::Thumb32Info[32]();
	dec_table_thumb32_asm_ldst1_dual	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst2_dual	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_asm_ldst3_dual	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_dproc_shft_reg	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg3	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg4	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg5	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_shft_reg6	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_dproc_imm	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm3	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm4	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm5	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_imm6	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_dproc_reg	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg3	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg4	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg5	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg6	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_reg7	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_dproc_misc	= new Instruction::Thumb32Info[8]();
	dec_table_thumb32_dproc_misc1	= new Instruction::Thumb32Info[8]();

	dec_table_thumb32_dproc_bin_imm		= new Instruction::Thumb32Info[32]();
	dec_table_thumb32_dproc_bin_imm1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_bin_imm2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_dproc_bin_imm3	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_brnch_ctrl	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_brnch_ctrl1 	= new Instruction::Thumb32Info[8]();

	dec_table_thumb32_st_single	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single3	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single4	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single5	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_st_single6	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_ld_byte	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte2	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte3	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte4	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte5	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_byte6	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_ld_hfword	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_hfword1	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_hfword2	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_ld_word	= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_ld_word1	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_mult		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_mult1		= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_mult_long	= new Instruction::Thumb32Info[16]();

	dec_table_thumb32_mov		= new Instruction::Thumb32Info[16]();
	dec_table_thumb32_mov1		= new Instruction::Thumb32Info[16]();


	// Load store Multiple tables 
	dec_table_thumb32_asm[1].next_table 		=  dec_table_thumb32_asm_lv1;
	dec_table_thumb32_asm[1].next_table_high 	= 26;
	dec_table_thumb32_asm[1].next_table_low		= 26;

	dec_table_thumb32_asm_lv1[0].next_table		= dec_table_thumb32_asm_lv2;
	dec_table_thumb32_asm_lv1[0].next_table_high	= 25;
	dec_table_thumb32_asm_lv1[0].next_table_low	= 25;

	dec_table_thumb32_asm_lv2[0].next_table		= dec_table_thumb32_asm_lv3;
	dec_table_thumb32_asm_lv2[0].next_table_high	= 22;
	dec_table_thumb32_asm_lv2[0].next_table_low	= 22;

	dec_table_thumb32_asm_lv3[0].next_table		= dec_table_thumb32_asm_ldst_mul;
	dec_table_thumb32_asm_lv3[0].next_table_high	= 24;
	dec_table_thumb32_asm_lv3[0].next_table_low	= 23;

	dec_table_thumb32_asm_ldst_mul[1].next_table 		= dec_table_thumb32_asm_ldst_mul1;
	dec_table_thumb32_asm_ldst_mul[1].next_table_high 	= 20;
	dec_table_thumb32_asm_ldst_mul[1].next_table_low 	= 20;

	dec_table_thumb32_asm_ldst_mul1[1].next_table 		= dec_table_thumb32_asm_ldst_mul2;
	dec_table_thumb32_asm_ldst_mul1[1].next_table_high 	= 21;
	dec_table_thumb32_asm_ldst_mul1[1].next_table_low 	= 21;

	dec_table_thumb32_asm_ldst_mul2[1].next_table 		= dec_table_thumb32_asm_ldst_mul3;
	dec_table_thumb32_asm_ldst_mul2[1].next_table_high 	= 19;
	dec_table_thumb32_asm_ldst_mul2[1].next_table_low 	= 16;

	dec_table_thumb32_asm_ldst_mul[2].next_table 		= dec_table_thumb32_asm_ldst_mul4;
	dec_table_thumb32_asm_ldst_mul[2].next_table_high 	= 20;
	dec_table_thumb32_asm_ldst_mul[2].next_table_low 	= 20;

	dec_table_thumb32_asm_ldst_mul4[0].next_table 		= dec_table_thumb32_asm_ldst_mul5;
	dec_table_thumb32_asm_ldst_mul4[0].next_table_high 	= 21;
	dec_table_thumb32_asm_ldst_mul4[0].next_table_low 	= 21;

	dec_table_thumb32_asm_ldst_mul5[1].next_table 		= dec_table_thumb32_asm_ldst_mul6;
	dec_table_thumb32_asm_ldst_mul5[1].next_table_high 	= 19;
	dec_table_thumb32_asm_ldst_mul5[1].next_table_low 	= 16;

	// Load store Dual tables 
	dec_table_thumb32_asm_lv3[1].next_table		= dec_table_thumb32_asm_ldst_dual;
	dec_table_thumb32_asm_lv3[1].next_table_high	= 24;
	dec_table_thumb32_asm_lv3[1].next_table_low	= 20;

	dec_table_thumb32_asm_ldst_dual[(0x07)].next_table	 = dec_table_thumb32_asm_ldst1_dual;
	dec_table_thumb32_asm_ldst_dual[(0x07)].next_table_high	 = 19;
	dec_table_thumb32_asm_ldst_dual[(0x07)].next_table_low	 = 16;

	dec_table_thumb32_asm_ldst_dual[(0x08)].next_table	 = dec_table_thumb32_asm_ldst2_dual;
	dec_table_thumb32_asm_ldst_dual[(0x08)].next_table_high	 = 19;
	dec_table_thumb32_asm_ldst_dual[(0x08)].next_table_low	 = 16;

	dec_table_thumb32_asm_ldst_dual[(0x0d)].next_table	 = dec_table_thumb32_asm_ldst3_dual;
	dec_table_thumb32_asm_ldst_dual[(0x0d)].next_table_high	 = 7;
	dec_table_thumb32_asm_ldst_dual[(0x0d)].next_table_low	 = 4;

	// Data Processing Shifted Reg Tables 
	dec_table_thumb32_asm_lv2[1].next_table 	= dec_table_thumb32_dproc_shft_reg;
	dec_table_thumb32_asm_lv2[1].next_table_high	= 24;
	dec_table_thumb32_asm_lv2[1].next_table_low 	= 21;

	dec_table_thumb32_dproc_shft_reg[0].next_table  	= dec_table_thumb32_dproc_shft_reg1;
	dec_table_thumb32_dproc_shft_reg[0].next_table_high 	= 11;
	dec_table_thumb32_dproc_shft_reg[0].next_table_low 	= 8;

	dec_table_thumb32_dproc_shft_reg[2].next_table  	= dec_table_thumb32_dproc_shft_reg2;
	dec_table_thumb32_dproc_shft_reg[2].next_table_high 	= 19;
	dec_table_thumb32_dproc_shft_reg[2].next_table_low 	= 16;

	dec_table_thumb32_dproc_shft_reg[3].next_table  	= dec_table_thumb32_dproc_shft_reg3;
	dec_table_thumb32_dproc_shft_reg[3].next_table_high 	= 19;
	dec_table_thumb32_dproc_shft_reg[3].next_table_low 	= 16;

	dec_table_thumb32_dproc_shft_reg[4].next_table  	= dec_table_thumb32_dproc_shft_reg4;
	dec_table_thumb32_dproc_shft_reg[4].next_table_high 	= 11;
	dec_table_thumb32_dproc_shft_reg[4].next_table_low 	= 8;

	dec_table_thumb32_dproc_shft_reg[8].next_table  	= dec_table_thumb32_dproc_shft_reg5;
	dec_table_thumb32_dproc_shft_reg[8].next_table_high 	= 11;
	dec_table_thumb32_dproc_shft_reg[8].next_table_low 	= 8;

	dec_table_thumb32_dproc_shft_reg[(0xd)].next_table  		= dec_table_thumb32_dproc_shft_reg6;
	dec_table_thumb32_dproc_shft_reg[(0xd)].next_table_high 	= 11;
	dec_table_thumb32_dproc_shft_reg[(0xd)].next_table_low 		= 8;

	dec_table_thumb32_dproc_shft_reg2[(0xf)].next_table	  	= dec_table_thumb32_mov;
	dec_table_thumb32_dproc_shft_reg2[(0xf)].next_table_high 	= 5;
	dec_table_thumb32_dproc_shft_reg2[(0xf)].next_table_low 	= 4;


	// Data Processing Immediate Tables 
	dec_table_thumb32_asm[2].next_table 		= dec_table_thumb32_asm_lv4;
	dec_table_thumb32_asm[2].next_table_high 	= 15;
	dec_table_thumb32_asm[2].next_table_low 	= 15;

	dec_table_thumb32_asm_lv4[0].next_table 	= dec_table_thumb32_asm_lv5;
	dec_table_thumb32_asm_lv4[0].next_table_high 	= 25;
	dec_table_thumb32_asm_lv4[0].next_table_low 	= 25;

	dec_table_thumb32_asm_lv5[0].next_table 	= dec_table_thumb32_dproc_imm;
	dec_table_thumb32_asm_lv5[0].next_table_high 	= 24;
	dec_table_thumb32_asm_lv5[0].next_table_low 	= 21;

	dec_table_thumb32_dproc_imm[0].next_table  	= dec_table_thumb32_dproc_imm1;
	dec_table_thumb32_dproc_imm[0].next_table_high 	= 11;
	dec_table_thumb32_dproc_imm[0].next_table_low 	= 8;

	dec_table_thumb32_dproc_imm[2].next_table  	= dec_table_thumb32_dproc_imm2;
	dec_table_thumb32_dproc_imm[2].next_table_high 	= 19;
	dec_table_thumb32_dproc_imm[2].next_table_low 	= 16;

	dec_table_thumb32_dproc_imm[3].next_table  	= dec_table_thumb32_dproc_imm3;
	dec_table_thumb32_dproc_imm[3].next_table_high 	= 19;
	dec_table_thumb32_dproc_imm[3].next_table_low 	= 16;

	dec_table_thumb32_dproc_imm[4].next_table  	= dec_table_thumb32_dproc_imm4;
	dec_table_thumb32_dproc_imm[4].next_table_high 	= 11;
	dec_table_thumb32_dproc_imm[4].next_table_low 	= 8;

	dec_table_thumb32_dproc_imm[8].next_table  	= dec_table_thumb32_dproc_imm5;
	dec_table_thumb32_dproc_imm[8].next_table_high 	= 11;
	dec_table_thumb32_dproc_imm[8].next_table_low 	= 8;

	dec_table_thumb32_dproc_imm[(0xd)].next_table  		= dec_table_thumb32_dproc_imm6;
	dec_table_thumb32_dproc_imm[(0xd)].next_table_high 	= 11;
	dec_table_thumb32_dproc_imm[(0xd)].next_table_low 	= 8;


	// Data Processing Plain Binary Immediate Tables 
	dec_table_thumb32_asm_lv5[1].next_table 	= dec_table_thumb32_dproc_bin_imm;
	dec_table_thumb32_asm_lv5[1].next_table_high 	= 24;
	dec_table_thumb32_asm_lv5[1].next_table_low 	= 20;

	dec_table_thumb32_dproc_bin_imm[0].next_table 		= dec_table_thumb32_dproc_bin_imm1;
	dec_table_thumb32_dproc_bin_imm[0].next_table_high 	= 19;
	dec_table_thumb32_dproc_bin_imm[0].next_table_low 	= 16;

	dec_table_thumb32_dproc_bin_imm[(0x0a)].next_table 	= dec_table_thumb32_dproc_bin_imm2;
	dec_table_thumb32_dproc_bin_imm[(0x0a)].next_table_high = 19;
	dec_table_thumb32_dproc_bin_imm[(0x0a)].next_table_low 	= 16;

	dec_table_thumb32_dproc_bin_imm[(0x16)].next_table 	= dec_table_thumb32_dproc_bin_imm3;
	dec_table_thumb32_dproc_bin_imm[(0x16)].next_table_high = 19;
	dec_table_thumb32_dproc_bin_imm[(0x16)].next_table_low 	= 16;

	// Branch_control table 
	dec_table_thumb32_asm_lv4[1].next_table 	= dec_table_thumb32_brnch_ctrl;
	dec_table_thumb32_asm_lv4[1].next_table_high 	= 14;
	dec_table_thumb32_asm_lv4[1].next_table_low 	= 12;

	dec_table_thumb32_brnch_ctrl[0].next_table 	= dec_table_thumb32_brnch_ctrl1;
	dec_table_thumb32_brnch_ctrl[0].next_table_high = 25;
	dec_table_thumb32_brnch_ctrl[0].next_table_low 	= 25;

	dec_table_thumb32_brnch_ctrl[2].next_table 	= dec_table_thumb32_brnch_ctrl1;
	dec_table_thumb32_brnch_ctrl[2].next_table_high = 25;
	dec_table_thumb32_brnch_ctrl[2].next_table_low 	= 23;

	// Single Data table 
	dec_table_thumb32_asm[3].next_table 		= dec_table_thumb32_asm_lv6;
	dec_table_thumb32_asm[3].next_table_high 	= 26;
	dec_table_thumb32_asm[3].next_table_low 	= 26;

	dec_table_thumb32_asm_lv6[0].next_table 	= dec_table_thumb32_asm_lv7;
	dec_table_thumb32_asm_lv6[0].next_table_high 	= 25;
	dec_table_thumb32_asm_lv6[0].next_table_low 	= 24;

	dec_table_thumb32_asm_lv7[0].next_table 	= dec_table_thumb32_asm_lv8;
	dec_table_thumb32_asm_lv7[0].next_table_high 	= 22;
	dec_table_thumb32_asm_lv7[0].next_table_low 	= 20;

	for(unsigned int i = 0; i < 8; i++)
	{
		if(!(i % 2))
		{
			dec_table_thumb32_asm_lv8[i].next_table 	= dec_table_thumb32_st_single;
			dec_table_thumb32_asm_lv8[i].next_table_high 	= 23;
			dec_table_thumb32_asm_lv8[i].next_table_low 	= 21;
		}
	}

	dec_table_thumb32_st_single[0].next_table 	= dec_table_thumb32_st_single1;
	dec_table_thumb32_st_single[0].next_table_high 	= 11;
	dec_table_thumb32_st_single[0].next_table_low 	= 11;

	dec_table_thumb32_st_single1[1].next_table 	= dec_table_thumb32_st_single2;
	dec_table_thumb32_st_single1[1].next_table_high	= 9;
	dec_table_thumb32_st_single1[1].next_table_low 	= 9;

	dec_table_thumb32_st_single[1].next_table 	= dec_table_thumb32_st_single3;
	dec_table_thumb32_st_single[1].next_table_high 	= 11;
	dec_table_thumb32_st_single[1].next_table_low 	= 11;

	dec_table_thumb32_st_single3[1].next_table 	= dec_table_thumb32_st_single4;
	dec_table_thumb32_st_single3[1].next_table_high	= 9;
	dec_table_thumb32_st_single3[1].next_table_low 	= 9;

	dec_table_thumb32_st_single[2].next_table 	= dec_table_thumb32_st_single5;
	dec_table_thumb32_st_single[2].next_table_high 	= 11;
	dec_table_thumb32_st_single[2].next_table_low 	= 11;

	dec_table_thumb32_st_single5[1].next_table 	= dec_table_thumb32_st_single6;
	dec_table_thumb32_st_single5[1].next_table_high	= 9;
	dec_table_thumb32_st_single5[1].next_table_low 	= 9;

	// Load Byte Table 
	dec_table_thumb32_asm_lv7[1].next_table 	= dec_table_thumb32_asm_lv9;
	dec_table_thumb32_asm_lv7[1].next_table_high 	= 22;
	dec_table_thumb32_asm_lv7[1].next_table_low 	= 20;

	dec_table_thumb32_asm_lv9[1].next_table 	= dec_table_thumb32_ld_byte;
	dec_table_thumb32_asm_lv9[1].next_table_high 	= 24;
	dec_table_thumb32_asm_lv9[1].next_table_low 	= 23;

	dec_table_thumb32_asm_lv8[1].next_table 	= dec_table_thumb32_ld_byte;
	dec_table_thumb32_asm_lv8[1].next_table_high 	= 24;
	dec_table_thumb32_asm_lv8[1].next_table_low 	= 23;

	dec_table_thumb32_ld_byte[0].next_table 	= dec_table_thumb32_ld_byte1;
	dec_table_thumb32_ld_byte[0].next_table_high 	= 19;
	dec_table_thumb32_ld_byte[0].next_table_low 	= 16;

	for(unsigned int i = 0; i < 15; i++)
	{
		dec_table_thumb32_ld_byte1[i].next_table 	= dec_table_thumb32_ld_byte2;
		dec_table_thumb32_ld_byte1[i].next_table_high 	= 11;
		dec_table_thumb32_ld_byte1[i].next_table_low 	= 11;
	}

	dec_table_thumb32_ld_byte2[1].next_table 	= dec_table_thumb32_ld_byte3;
	dec_table_thumb32_ld_byte2[1].next_table_high 	= 10;
	dec_table_thumb32_ld_byte2[1].next_table_low 	= 8;

	dec_table_thumb32_ld_byte[2].next_table 	= dec_table_thumb32_ld_byte4;
	dec_table_thumb32_ld_byte[2].next_table_high 	= 19;
	dec_table_thumb32_ld_byte[2].next_table_low 	= 16;

	for(unsigned int i = 0; i < 15; i++)
	{
		dec_table_thumb32_ld_byte4[i].next_table 	= dec_table_thumb32_ld_byte5;
		dec_table_thumb32_ld_byte4[i].next_table_high 	= 11;
		dec_table_thumb32_ld_byte4[i].next_table_low 	= 11;
	}

	dec_table_thumb32_ld_byte5[1].next_table 	= dec_table_thumb32_ld_byte6;
	dec_table_thumb32_ld_byte5[1].next_table_high 	= 10;
	dec_table_thumb32_ld_byte5[1].next_table_low 	= 8;

	// Load Halfword Table 
	dec_table_thumb32_asm_lv7[1].next_table 	= dec_table_thumb32_asm_lv9;
	dec_table_thumb32_asm_lv7[1].next_table_high 	= 22;
	dec_table_thumb32_asm_lv7[1].next_table_low 	= 20;

	dec_table_thumb32_asm_lv9[3].next_table 	= dec_table_thumb32_ld_hfword;
	dec_table_thumb32_asm_lv9[3].next_table_high 	= 24;
	dec_table_thumb32_asm_lv9[3].next_table_low 	= 23;

	dec_table_thumb32_asm_lv8[3].next_table 	= dec_table_thumb32_ld_hfword;
	dec_table_thumb32_asm_lv8[3].next_table_high 	= 24;
	dec_table_thumb32_asm_lv8[3].next_table_low 	= 23;

	dec_table_thumb32_ld_hfword[0].next_table	= dec_table_thumb32_ld_hfword1;
	dec_table_thumb32_ld_hfword[0].next_table_high 	= 11;
	dec_table_thumb32_ld_hfword[0].next_table_low	= 11;

	dec_table_thumb32_ld_hfword[2].next_table	= dec_table_thumb32_ld_hfword2;
	dec_table_thumb32_ld_hfword[2].next_table_high 	= 11;
	dec_table_thumb32_ld_hfword[2].next_table_low	= 11;

	// Load Word Table 
	dec_table_thumb32_asm_lv7[1].next_table 	= dec_table_thumb32_asm_lv9;
	dec_table_thumb32_asm_lv7[1].next_table_high 	= 22;
	dec_table_thumb32_asm_lv7[1].next_table_low 	= 20;

	dec_table_thumb32_asm_lv9[5].next_table 	= dec_table_thumb32_ld_word;
	dec_table_thumb32_asm_lv9[5].next_table_high 	= 24;
	dec_table_thumb32_asm_lv9[5].next_table_low 	= 23;

	dec_table_thumb32_asm_lv8[5].next_table 	= dec_table_thumb32_ld_word;
	dec_table_thumb32_asm_lv8[5].next_table_high 	= 24;
	dec_table_thumb32_asm_lv8[5].next_table_low 	= 23;

	dec_table_thumb32_ld_word[0].next_table		= dec_table_thumb32_ld_word1;
	dec_table_thumb32_ld_word[0].next_table_high 	= 11;
	dec_table_thumb32_ld_word[0].next_table_low	= 11;

	// Data Processing Register Based Table 
	dec_table_thumb32_asm_lv7[2].next_table 	= dec_table_thumb32_dproc_reg;
	dec_table_thumb32_asm_lv7[2].next_table_high 	= 23;
	dec_table_thumb32_asm_lv7[2].next_table_low 	= 20;

	dec_table_thumb32_dproc_reg[0].next_table	= dec_table_thumb32_dproc_reg1;
	dec_table_thumb32_dproc_reg[0].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[0].next_table_low	= 7;

	dec_table_thumb32_dproc_reg[1].next_table	= dec_table_thumb32_dproc_reg2;
	dec_table_thumb32_dproc_reg[1].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[1].next_table_low	= 7;

	dec_table_thumb32_dproc_reg2[1].next_table	= dec_table_thumb32_dproc_reg3;
	dec_table_thumb32_dproc_reg2[1].next_table_high	= 19;
	dec_table_thumb32_dproc_reg2[1].next_table_low	= 16;

	dec_table_thumb32_dproc_reg[2].next_table	= dec_table_thumb32_dproc_reg4;
	dec_table_thumb32_dproc_reg[2].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[2].next_table_low	= 7;

	dec_table_thumb32_dproc_reg[3].next_table	= dec_table_thumb32_dproc_reg5;
	dec_table_thumb32_dproc_reg[3].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[3].next_table_low	= 7;

	dec_table_thumb32_dproc_reg[4].next_table	= dec_table_thumb32_dproc_reg6;
	dec_table_thumb32_dproc_reg[4].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[4].next_table_low	= 7;

	dec_table_thumb32_dproc_reg[5].next_table	= dec_table_thumb32_dproc_reg7;
	dec_table_thumb32_dproc_reg[5].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[5].next_table_low	= 7;

	dec_table_thumb32_dproc_reg[8].next_table	= dec_table_thumb32_dproc_misc;
	dec_table_thumb32_dproc_reg[8].next_table_high	= 7;
	dec_table_thumb32_dproc_reg[8].next_table_low	= 6;

	dec_table_thumb32_dproc_misc[2].next_table	= dec_table_thumb32_dproc_misc1;
	dec_table_thumb32_dproc_misc[2].next_table_high	= 21;
	dec_table_thumb32_dproc_misc[2].next_table_low	= 20;


	// Multiply Tables 
	dec_table_thumb32_asm_lv7[3].next_table 	= dec_table_thumb32_asm_lv10;
	dec_table_thumb32_asm_lv7[3].next_table_high 	= 23;
	dec_table_thumb32_asm_lv7[3].next_table_low 	= 23;

	dec_table_thumb32_asm_lv10[0].next_table 	= dec_table_thumb32_mult;
	dec_table_thumb32_asm_lv10[0].next_table_high 	= 5;
	dec_table_thumb32_asm_lv10[0].next_table_low 	= 4;

	dec_table_thumb32_mult[0].next_table 		= dec_table_thumb32_mult1;
	dec_table_thumb32_mult[0].next_table_high 	= 15;
	dec_table_thumb32_mult[0].next_table_low 	= 12;

	// Multiply Long Tables 
	dec_table_thumb32_asm_lv10[1].next_table 	= dec_table_thumb32_mult_long;
	dec_table_thumb32_asm_lv10[1].next_table_high 	= 22;
	dec_table_thumb32_asm_lv10[1].next_table_low 	= 20;

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	Thumb32SetupTable(#_name, _fmt_str, Instruction::Thumb32Category##_cat, _op1, _op2,\
		_op3, _op4, _op5, _op6, _op7, _op8, Instruction::Thumb32Opcode##_name);
#include "InstructionThumb32.def"
#undef DEFINST


	dec_table_thumb16_asm 			= new Instruction::Thumb16Info[8]();
	dec_table_thumb16_shft_ins		= new Instruction::Thumb16Info[16]();
	dec_table_thumb16_shft_ins_lv2		= new Instruction::Thumb16Info[16]();

	dec_table_thumb16_asm_lv1		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv2		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv3		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv4		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv5		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv6		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv7		= new Instruction::Thumb16Info[64]();
	dec_table_thumb16_asm_lv8		= new Instruction::Thumb16Info[4]();
	dec_table_thumb16_asm_lv9		= new Instruction::Thumb16Info[64]();


	dec_table_thumb16_data_proc		= new Instruction::Thumb16Info[32]();
	dec_table_thumb16_spcl_data_brex	= new Instruction::Thumb16Info[32]();
	dec_table_thumb16_spcl_data_brex_lv1	= new Instruction::Thumb16Info[4]();

	dec_table_thumb16_ld_st			= new Instruction::Thumb16Info[64]();
	dec_table_thumb16_ld_st_lv1		= new Instruction::Thumb16Info[64]();
	dec_table_thumb16_ld_st_lv2		= new Instruction::Thumb16Info[64]();


	dec_table_thumb16_misc			= new Instruction::Thumb16Info[128]();
	dec_table_thumb16_it			= new Instruction::Thumb16Info[256]();


	// Directing to Shift Instructions 
	dec_table_thumb16_asm[0].next_table 	= dec_table_thumb16_shft_ins;
	dec_table_thumb16_asm[0].next_table_high= 13;
	dec_table_thumb16_asm[0].next_table_low	= 11;

	dec_table_thumb16_shft_ins[3].next_table = dec_table_thumb16_shft_ins_lv2;
	dec_table_thumb16_shft_ins[3].next_table_high = 10;
	dec_table_thumb16_shft_ins[3].next_table_low = 9;

	// Directing to Data Processing Instructions 
	dec_table_thumb16_asm[1].next_table 	= dec_table_thumb16_asm_lv1;
	dec_table_thumb16_asm[1].next_table_high= 13;
	dec_table_thumb16_asm[1].next_table_low	= 13;

	dec_table_thumb16_asm_lv1[0].next_table 	= dec_table_thumb16_asm_lv2;
	dec_table_thumb16_asm_lv1[0].next_table_high	= 12;
	dec_table_thumb16_asm_lv1[0].next_table_low	= 12;

	dec_table_thumb16_asm_lv2[0].next_table 	= dec_table_thumb16_asm_lv3;
	dec_table_thumb16_asm_lv2[0].next_table_high	= 11;
	dec_table_thumb16_asm_lv2[0].next_table_low	= 10;

	dec_table_thumb16_asm_lv3[0].next_table 	= dec_table_thumb16_data_proc;
	dec_table_thumb16_asm_lv3[0].next_table_high	= 9;
	dec_table_thumb16_asm_lv3[0].next_table_low	= 6;

	// Directing to LD/ST Instructions 
	dec_table_thumb16_asm_lv1[1].next_table 	= dec_table_thumb16_ld_st;
	dec_table_thumb16_asm_lv1[1].next_table_high	= 15;
	dec_table_thumb16_asm_lv1[1].next_table_low	= 11;

	dec_table_thumb16_asm_lv2[1].next_table 	= dec_table_thumb16_ld_st;
	dec_table_thumb16_asm_lv2[1].next_table_high	= 15;
	dec_table_thumb16_asm_lv2[1].next_table_low	= 11;

	dec_table_thumb16_asm_lv4[0].next_table 	= dec_table_thumb16_ld_st;
	dec_table_thumb16_asm_lv4[0].next_table_high	= 15;
	dec_table_thumb16_asm_lv4[0].next_table_low	= 11;

	dec_table_thumb16_ld_st[10].next_table		= dec_table_thumb16_ld_st_lv1;
	dec_table_thumb16_ld_st[10].next_table_high 	= 10;
	dec_table_thumb16_ld_st[10].next_table_low 	= 9;

	dec_table_thumb16_ld_st[11].next_table 		= dec_table_thumb16_ld_st_lv2;
	dec_table_thumb16_ld_st[11].next_table_high 	= 10;
	dec_table_thumb16_ld_st[11].next_table_low 	= 9;

	// Directing to Special data Instructions and B&EX instructions
	dec_table_thumb16_asm_lv3[1].next_table 	= dec_table_thumb16_spcl_data_brex;
	dec_table_thumb16_asm_lv3[1].next_table_high	= 9;
	dec_table_thumb16_asm_lv3[1].next_table_low	= 7;

	dec_table_thumb16_spcl_data_brex[0].next_table 	= dec_table_thumb16_spcl_data_brex_lv1;
	dec_table_thumb16_spcl_data_brex[0].next_table_high	= 6;
	dec_table_thumb16_spcl_data_brex[0].next_table_low	= 6;

	// Directing to Misellaneous 16 bit thumb2 instructions 
	dec_table_thumb16_asm[2].next_table = dec_table_thumb16_asm_lv4;
	dec_table_thumb16_asm[2].next_table_high = 13;
	dec_table_thumb16_asm[2].next_table_low = 13;

	dec_table_thumb16_asm_lv4[1].next_table 	= dec_table_thumb16_asm_lv5;
	dec_table_thumb16_asm_lv4[1].next_table_high	= 12;
	dec_table_thumb16_asm_lv4[1].next_table_low	= 12;

	dec_table_thumb16_asm_lv5[1].next_table 	= dec_table_thumb16_misc;
	dec_table_thumb16_asm_lv5[1].next_table_high	= 11;
	dec_table_thumb16_asm_lv5[1].next_table_low	= 5;

	for(unsigned int i = 0; i < 8; i++)
	{
		dec_table_thumb16_misc[(0x78 + i)].next_table 		= dec_table_thumb16_it;
		dec_table_thumb16_misc[(0x78 + i)].next_table_high 	= 3;
		dec_table_thumb16_misc[(0x78 + i)].next_table_low 	= 0;
	}

	// Directing to PC and SP relative instructions 
	dec_table_thumb16_asm_lv5[0].next_table 	= dec_table_thumb16_asm_lv6;
	dec_table_thumb16_asm_lv5[0].next_table_high	= 11;
	dec_table_thumb16_asm_lv5[0].next_table_low	= 11;

	dec_table_thumb16_asm[3].next_table = dec_table_thumb16_asm_lv7;
	dec_table_thumb16_asm[3].next_table_high = 13;
	dec_table_thumb16_asm[3].next_table_low = 12;

	// Directing to Software interrupt instructions 
	dec_table_thumb16_asm_lv7[0].next_table 	= dec_table_thumb16_asm_lv8;
	dec_table_thumb16_asm_lv7[0].next_table_high	= 11;
	dec_table_thumb16_asm_lv7[0].next_table_low	= 11;

	// Directing to unconditional branch instructions 
	dec_table_thumb16_asm_lv7[1].next_table 	= dec_table_thumb16_asm_lv9; // entries [0 to 0xe] of lv9 are inst B //
	dec_table_thumb16_asm_lv7[1].next_table_high	= 11;
	dec_table_thumb16_asm_lv7[1].next_table_low	= 8;



#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	Thumb16SetupTable(#_name, _fmt_str, Instruction::Thumb16Category##_cat, _op1, _op2,\
		_op3, _op4, _op5, _op6, Instruction::Thumb16Opcode##_name);
#include "InstructionThumb.def"
#undef DEFINST

}


std::unique_ptr<Disassembler> Disassembler::instance;


Disassembler *Disassembler::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Disassembler());
	return instance.get();
}


Disassembler::~Disassembler()
{
	// Thumb 16 tables 
	delete dec_table_thumb16_asm;

	delete dec_table_thumb16_shft_ins;
	delete dec_table_thumb16_shft_ins_lv2;

	delete dec_table_thumb16_asm_lv1;
	delete dec_table_thumb16_asm_lv2;
	delete dec_table_thumb16_asm_lv3;
	delete dec_table_thumb16_asm_lv4;
	delete dec_table_thumb16_asm_lv5;
	delete dec_table_thumb16_asm_lv6;
	delete dec_table_thumb16_asm_lv7;
	delete dec_table_thumb16_asm_lv8;
	delete dec_table_thumb16_asm_lv9;

	delete dec_table_thumb16_data_proc;
	delete dec_table_thumb16_spcl_data_brex;
	delete dec_table_thumb16_spcl_data_brex_lv1;
	delete dec_table_thumb16_ld_st;
	delete dec_table_thumb16_ld_st_lv1;
	delete dec_table_thumb16_ld_st_lv2;
	delete dec_table_thumb16_misc;
	delete dec_table_thumb16_it;

	// Thumb 32 tables 
	delete dec_table_thumb32_asm;
	delete dec_table_thumb32_asm_lv1;
	delete dec_table_thumb32_asm_lv2;
	delete dec_table_thumb32_asm_lv3;
	delete dec_table_thumb32_asm_lv4;
	delete dec_table_thumb32_asm_lv5;
	delete dec_table_thumb32_asm_lv6;
	delete dec_table_thumb32_asm_lv7;
	delete dec_table_thumb32_asm_lv8;
	delete dec_table_thumb32_asm_lv9;
	delete dec_table_thumb32_asm_lv10;
	delete dec_table_thumb32_asm_lv11;
	delete dec_table_thumb32_asm_lv12;
	delete dec_table_thumb32_asm_lv13;
	delete dec_table_thumb32_asm_lv14;
	delete dec_table_thumb32_asm_lv15;


	delete dec_table_thumb32_asm_ldst_mul;
	delete dec_table_thumb32_asm_ldst_mul1;
	delete dec_table_thumb32_asm_ldst_mul2;
	delete dec_table_thumb32_asm_ldst_mul3;
	delete dec_table_thumb32_asm_ldst_mul4;
	delete dec_table_thumb32_asm_ldst_mul5;
	delete dec_table_thumb32_asm_ldst_mul6;

	delete dec_table_thumb32_asm_ldst_dual;
	delete dec_table_thumb32_asm_ldst1_dual;
	delete dec_table_thumb32_asm_ldst2_dual;
	delete dec_table_thumb32_asm_ldst3_dual;

	delete dec_table_thumb32_dproc_shft_reg;
	delete dec_table_thumb32_dproc_shft_reg1;
	delete dec_table_thumb32_dproc_shft_reg2;
	delete dec_table_thumb32_dproc_shft_reg3;
	delete dec_table_thumb32_dproc_shft_reg4;
	delete dec_table_thumb32_dproc_shft_reg5;
	delete dec_table_thumb32_dproc_shft_reg6;

	delete dec_table_thumb32_dproc_imm;
	delete dec_table_thumb32_dproc_imm1;
	delete dec_table_thumb32_dproc_imm2;
	delete dec_table_thumb32_dproc_imm3;
	delete dec_table_thumb32_dproc_imm4;
	delete dec_table_thumb32_dproc_imm5;
	delete dec_table_thumb32_dproc_imm6;

	delete dec_table_thumb32_dproc_reg;
	delete dec_table_thumb32_dproc_reg1;
	delete dec_table_thumb32_dproc_reg2;
	delete dec_table_thumb32_dproc_reg3;
	delete dec_table_thumb32_dproc_reg4;
	delete dec_table_thumb32_dproc_reg5;
	delete dec_table_thumb32_dproc_reg6;
	delete dec_table_thumb32_dproc_reg7;

	delete dec_table_thumb32_dproc_misc;
	delete dec_table_thumb32_dproc_misc1;

	delete dec_table_thumb32_st_single;
	delete dec_table_thumb32_st_single1;
	delete dec_table_thumb32_st_single2;
	delete dec_table_thumb32_st_single3;
	delete dec_table_thumb32_st_single4;
	delete dec_table_thumb32_st_single5;
	delete dec_table_thumb32_st_single6;

	delete dec_table_thumb32_ld_byte;
	delete dec_table_thumb32_ld_byte1;
	delete dec_table_thumb32_ld_byte2;
	delete dec_table_thumb32_ld_byte3;
	delete dec_table_thumb32_ld_byte4;
	delete dec_table_thumb32_ld_byte5;
	delete dec_table_thumb32_ld_byte6;

	delete dec_table_thumb32_ld_hfword;
	delete dec_table_thumb32_ld_hfword1;
	delete dec_table_thumb32_ld_hfword2;

	delete dec_table_thumb32_ld_word;
	delete dec_table_thumb32_ld_word1;

	delete dec_table_thumb32_mult;
	delete dec_table_thumb32_mult1;

	delete dec_table_thumb32_dproc_bin_imm;
	delete dec_table_thumb32_dproc_bin_imm1;
	delete dec_table_thumb32_dproc_bin_imm2;
	delete dec_table_thumb32_dproc_bin_imm3;

	delete dec_table_thumb32_mult_long;
	delete dec_table_thumb32_mov;
	delete dec_table_thumb32_mov1;

	delete dec_table_thumb32_brnch_ctrl;
	delete dec_table_thumb32_brnch_ctrl1;
}


void Disassembler::SetupInstInfo(const char* name, const char* fmt_str,
		Instruction::Category category, int arg1, int arg2, Instruction::Opcode inst_name)
{
	Instruction::Info *info;
	switch (category)
	{
	case Instruction::CategoryDprReg:

		info = &inst_info[arg1 * 16 + 0];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 1];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 2];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 3];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 4];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 5];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 6];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 7];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 8];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 10];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 12];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 14];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryDprImm:

		for (unsigned int i = 0; i < 16; i++ )
		{
			info = &inst_info[arg1 * 16 + i];
			info->inst = inst_name;
			info->category = category;
			info->name = name;
			info->fmt_str = fmt_str;
			info->size = 4;
			info->opcode = inst_name;
		}
		break;

	case Instruction::CategoryDprSat:

		info = &inst_info[arg1 * 16 + 5];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryPsr:

		info = &inst_info[arg1 * 16 + 0];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryMult:

		info = &inst_info[arg1 * 16 + 9];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryMultSign:

		info = &inst_info[arg1 * 16 + 8];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 10];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 12];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		info = &inst_info[arg1 * 16 + 14];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryMultLn:

		info = &inst_info[arg1 * 16 + 9];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryMultLnSign:

		info = &inst_info[arg1 * 16 + 8];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategorySdswp:

		info = &inst_info[arg1 * 16 + 9];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryBax:

		info = &inst_info[arg1 * 16 + arg2];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryHfwrdReg:

		info = &inst_info[arg1 * 16 + arg2];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryHfwrdImm:

		info = &inst_info[arg1 * 16 + arg2];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategorySdtr:

		if (arg2 == 0xff)
		{
			for (unsigned int i = 0; i < 16; i++){
				info = &inst_info[arg1 * 16 + i];
				info->inst = inst_name;
				info->category = category;
				info->name = name;
				info->fmt_str = fmt_str;
				info->size = 4;
				info->opcode = inst_name;
			}
		}
		else
		{
			info = &inst_info[arg1 * 16 + arg2];
			info->inst = inst_name;
			info->category = category;
			info->name = name;
			info->fmt_str = fmt_str;
			info->size = 4;
			info->opcode = inst_name;
			info = &inst_info[arg1 * 16 + (arg2 + 8)];
			info->inst = inst_name;
			info->category = category;
			info->name = name;
			info->fmt_str = fmt_str;
			info->size = 4;
			info->opcode = inst_name;
		}
		break;

	case Instruction::CategoryBrnch:

		for (unsigned int i = 0 ; i < 16; i++)
		{
			for (unsigned int j = 0 ; j < 16; j++)
			{
				info = &inst_info[(arg1 + i) * 16 + j];
				info->inst = inst_name;
				info->category = category;
				info->name = name;
				info->fmt_str = fmt_str;
				info->size = 4;
				info->opcode = inst_name;
			}
		}
		break;

	case Instruction::CategoryBdtr:

		for (unsigned int i = 0; i < 16; i++)
		{
			info = &inst_info[arg1 * 16 + i];
			info->inst = inst_name;
			info->category = category;
			info->name = name;
			info->fmt_str = fmt_str;
			info->size = 4;
			info->opcode = inst_name;
		}
		break;

	case Instruction::CategorySwiSvc:

		for (unsigned int i = 0 ; i < 16; i++)
		{
			for (unsigned int j = 0 ; j < 16; j++)
			{
				info = &inst_info[(arg1 + i) * 16 + j ];
				info->inst = inst_name;
				info->category = category;
				info->name = name;
				info->fmt_str = fmt_str;
				info->size = 4;
				info->opcode = inst_name;
			}
		}
		break;

	case Instruction::CategoryCprDtr:

		info = &inst_info[arg1 * 16 + arg2];
		info->inst = inst_name;
		info->category = category;
		info->name = name;
		info->fmt_str = fmt_str;
		info->size = 4;
		info->opcode = inst_name;
		break;

	case Instruction::CategoryVfp:

		for (unsigned int i = 0; i < 16; i++)
		{
			info = &inst_info[arg1 * 16 + i];
			info->inst = inst_name;
			info->category = category;
			info->name = name;
			info->fmt_str = fmt_str;
			info->size = 4;
			info->opcode = inst_name;
		}
		break;
	default:

		throw misc::Panic(misc::fmt("%d: not recognized", category));
		break;
	}
}


void Disassembler::Thumb32SetupTable(const char* name , const char* fmt_str ,
		Instruction::Thumb32Category cat32 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, int op7, int op8, Instruction::Thumb32Opcode inst_name)
{
	Instruction::Thumb32Info *current_table;

	// We initially start with the first table arm_asm_table, with the opcode field as argument 
	current_table = dec_table_thumb32_asm;
	int op[8] = {op1, op2, op3, op4, op5, op6, op7, op8};
	int i = 0;
	while (1)
	{
		if (current_table[op[i]].next_table && (op[i] >= 0))
		{
			current_table = current_table[op[i]].next_table;
			i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat32 = cat32;
			current_table[op[i]].size = 4;
			current_table[op[i]].inst_32 = inst_name;
			current_table[op[i]].opcode = inst_name;

			break;
		}
	}
}


void Disassembler::Thumb16SetupTable(const char* name , const char* fmt_str ,
	Instruction::Thumb16Category cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, Instruction::Thumb16Opcode inst_name)
{
	// We initially start with the first table arm_asm_table, with the
	// opcode field as argument.
	struct Instruction::Thumb16Info *current_table = dec_table_thumb16_asm;

	// FIXME - comment
	int op[6] = {op1, op2, op3, op4, op5, op6};

	// FIXME - comment
	int i = 0;
	while (true)
	{
		if (current_table[op[i]].next_table && (op[i] >= 0))
		{
			current_table = current_table[op[i]].next_table;
			i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat16 = cat16;
			current_table[op[i]].size = 2;
			current_table[op[i]].inst_16 = inst_name;
			current_table[op[i]].opcode = inst_name;

			break;
		}
	}
}


void Disassembler::ElfSymbolFunc(const ELFReader::File &file, std::ostream &os,
		unsigned int inst_addr,
		DisassemblyMode disasm_mode)
{
	ELFReader::Symbol *symbol;
	//unsigned int prev_symbol;

	if (disasm_mode == DisassemblyModeArm)
	{
		for (int i = 0; i < file.getNumSymbols(); i++)
		{
			symbol = file.getSymbol(i);
			if(symbol->getValue() == inst_addr)
			{
				if(!(symbol->getName().compare(0, 1, "$")))
				{
					continue;
				}
				else
				{
					os << misc::fmt("\n%08x <%s>\n",
							symbol->getValue(),
							symbol->getName().c_str());
					//prev_symbol = symbol->getValue();
					break;
				}
			}
		}
	}
	else if (disasm_mode == DisassemblyModeThumb)
	{
		for (int i = 0; i < file.getNumSymbols(); i++)
		{
			symbol = file.getSymbol(i);
			if(symbol->getValue() == (inst_addr + 1))
			{
				if(!(symbol->getName().compare(0, 1, "$")))
				{
					continue;
				}
				else
				{

					os << misc::fmt("\n%08x <%s>\n",
							(symbol->getValue() -1),
							symbol->getName().c_str());
					//prev_symbol = symbol->getValue();
					break;
				}
			}
		}
	}
	return;
}


unsigned int Disassembler::ElfDumpWordSymbol(const ELFReader::File &file, std::ostream &os,
		unsigned int inst_addr, unsigned int *inst_ptr)
{
	ELFReader::Symbol *symbol;
	unsigned int word_flag;
	symbol = file.getSymbolByAddress(inst_addr);

	if (!(symbol->getName().compare(0, 2, "$d")))
	{
		os << misc::fmt(".word   0x%08x\n", *inst_ptr);
		word_flag = 1;
	}
	else
	{
		word_flag = 0;
	}

	return word_flag;
}


unsigned int Disassembler::ElfDumpThumbWordSymbol(const ELFReader::File &file, std::ostream &os,
		unsigned int inst_addr, unsigned int *inst_ptr)
{
	ELFReader::Symbol *symbol;
	unsigned int word_flag;
	symbol = file.getSymbolByAddress(inst_addr);;

	if (!(symbol->getName().compare(0, 2, "$d")))
	{
		os << misc::fmt(".word   0x%08x\n", *inst_ptr);
		word_flag = 1;
	}
	else
	{
		word_flag = 0;
	}

	return word_flag;
}


void Disassembler::ElfSymbolListCreate(const ELFReader::File &file,
			std::vector<ELFReader::Symbol*> &symbol_list)
{
	for (int i = 0; i < file.getNumSymbols(); i++)
	{
		if (!(file.getSymbol(i)->getName().compare(0, 2, "$a"))
			|| !(file.getSymbol(i)->getName().compare(0, 2, "$t")))
		{
			symbol_list.push_back(file.getSymbol(i));
		}
	}
}


Disassembler::DisassemblyMode Disassembler::DissassembleMode(
		const std::vector<ELFReader::Symbol*> &symbol_list,
		unsigned int addr)
{
	ELFReader::Symbol *symbol = NULL;
	DisassemblyMode disasm_mode;
	unsigned int tag_index = 0;

	// Binary search
	int lo = 0;
	int mid;
	int hi = symbol_list.size() - 1;;
	while(lo <= hi)
	{
		mid = (lo + hi) / 2;
		symbol = symbol_list[mid];

		if(addr < symbol->getValue())
			hi = mid - 1;
		else if(addr > symbol->getValue())
			lo = mid + 1;
		else
			lo = mid + 1;
	}

	// Search finished
	tag_index = mid;
	symbol = symbol_list[tag_index];
	if (!symbol)
		return DisassemblyModeArm;

	// Possible symbols
	if(!(symbol->getName().compare(0, 2, "$a")))
		disasm_mode = DisassemblyModeArm;
	else if(!(symbol->getName().compare(0, 2, "$t")))
		disasm_mode = DisassemblyModeThumb;

	// Return
	return disasm_mode;
}


int Disassembler::TestThumb32(const char *inst_ptr)
{
	unsigned int byte_index;
	unsigned int arg1;
	Instruction inst;
	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst.getBytes()->bytes[byte_index] = *(inst_ptr
			+ byte_index);

	arg1 = ((inst.getBytes()->bytes[1] & 0xf8) >> 3);

	if((arg1 == 0x1d) || (arg1 == 0x1e) || (arg1 == 0x1f))
	{
		return (1);
	}
	else
	{
		return(0);
	}
}


void Disassembler::DisassembleBinary(const std::string &path)
{
	ELFReader::File file(path);
	ELFReader::Section *section;
	std::vector<ELFReader::Symbol*> symbol_list;
	Instruction inst;
	static DisassemblyMode disasm_mode;

	// Find the section
	int i;
	unsigned int inst_index = 0;
	unsigned int inst_pos;
	for (i = 0; i < file.getNumSections(); ++i)
	{
		section = file.getSection(i);
		if (!(section->getName().compare(0, 5, ".text")))
			break;
	}
	if (i == file.getNumSections())
		throw misc::Panic(".text section not found!\n");

	// Create the symbol list
	ElfSymbolListCreate(file, symbol_list);

	// Decode and dump instructions
	for (inst_pos = 0; inst_pos < section->getSize();)
	{
		disasm_mode = DissassembleMode(symbol_list,
			(section->getAddr() + inst_index));

		if (disasm_mode == DisassemblyModeArm)
		{
			ElfSymbolFunc(
				file,
				std::cout,
				(section->getAddr() + inst_index),
				disasm_mode);

			inst.DumpHex(std::cout, (unsigned int *)(section->getBuffer() + inst_pos),
					(section->getAddr() + inst_index));

			if (!ElfDumpWordSymbol(file, std::cout, (section->getAddr() + inst_index),
				(unsigned int *)(section->getBuffer() + inst_pos)))
			{
				inst.Decode(section->getAddr() + inst_index, section->getBuffer() + inst_pos);
				inst.Dump(std::cout);
			}

			// Increment instruction buffer index by 4 for ARM mode
			inst_index += 4;
			inst_pos += 4;
		}

		else if(disasm_mode == DisassemblyModeThumb)
		{

			if(TestThumb32(section->getBuffer() + inst_pos))
			{

				ElfSymbolFunc(
					file,
					std::cout,
					(section->getAddr() + inst_index),
					disasm_mode);

				inst.Thumb32DumpHex(std::cout, (unsigned int *)(section->getBuffer() + inst_pos),
					(section->getAddr() + inst_index));

				if (!ElfDumpThumbWordSymbol(file, std::cout, (section->getAddr() + inst_index),
					(unsigned int *)(section->getBuffer() + inst_pos)))
				{

					inst.Thumb32Decode(section->getBuffer() + inst_pos,
							section->getAddr() + inst_index);
					inst.Thumb32Dump(std::cout);

					// Increment instruction buffer index by 4 for Thumb32 mode */
					inst_index += 4;
					inst_pos += 4;
				}
				else
				{
					inst_index += 4;
					inst_pos += 4;
				}
			}

			else
			{

				ElfSymbolFunc(
					file,
					std::cout,
					(section->getAddr() + inst_index),
					disasm_mode);

				inst.Thumb16DumpHex(std::cout, (unsigned int *)(section->getBuffer() + inst_pos),
					(section->getAddr() + inst_index));
				if (!ElfDumpThumbWordSymbol(file, std::cout, (section->getAddr() + inst_index),
					(unsigned int *)(section->getBuffer() + inst_pos)))
				{
					inst.Thumb16Decode(section->getBuffer() + inst_pos, section->getAddr() + inst_index);
					inst.Thumb16Dump(std::cout);

					// Increment instruction buffer index by 2 for Thumb16 mode
					inst_index += 2;
					inst_pos += 2;
				}
				else
				{
					inst_index += 2;
					inst_pos += 2;
				}
			}
		}
	}
}

} //namespace ARM

