

#ifndef ARM_ASM_ASM_H
#define ARM_ASM_ASM_H


#include <iostream>
#include <memory>
#include <vector>

#include <arch/common/Asm.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/ELFReader.h>


#include "Inst.h"

namespace ARM
{

// Disassembly Mode 
enum AsmDisassemblyMode
{
	AsmDisassemblyModeArm = 1,
	AsmDisassemblyModeThumb
};


// Shift operators 
enum AsmShiftOperator
{
	AsmShiftOperatorLsl = 0,
	AsmShiftOperatorLsr,
	AsmShiftOperatorAsr,
	AsmShiftOperatorRor
};

// Condition Fields 
enum AsmConditionCodes
{
	AsmConditionCodesEQ = 0, // Equal 
	AsmConditionCodesNE,	// Not Equal 
	AsmConditionCodesCS,	// Unsigned higher 
	AsmConditionCodesCC,	// Unsigned Lower 
	AsmConditionCodesMI,	// Negative 
	AsmConditionCodesPL,	// Positive or Zero 
	AsmConditionCodesVS,	// Overflow 
	AsmConditionCodesVC,	// No Overflow 
	AsmConditionCodesHI,	// Unsigned Higher 
	AsmConditionCodesLS,	// Unsigned Lower 
	AsmConditionCodesGE,	// Greater or Equal 
	AsmConditionCodesLT,	// Less Than 
	AsmConditionCodesGT,	// Greater than 
	AsmConditionCodesLE,	// Less than or equal 
	AsmConditionCodesAL	// Always 
};

enum AsmUserRegisters
{
	AsmUserRegistersR0 = 0,
	AsmUserRegistersR1,
	AsmUserRegistersR2,
	AsmUserRegistersR3,
	AsmUserRegistersR4,
	AsmUserRegistersR5,
	AsmUserRegistersR6,
	AsmUserRegistersR7,
	AsmUserRegistersR8,
	AsmUserRegistersR9,
	AsmUserRegistersR10,
	AsmUserRegistersR11,
	AsmUserRegistersR12,
	AsmUserRegistersR13,	// Stack Pointer sp 
	AsmUserRegistersR14,	// Link register lr 
	AsmUserRegistersR15	// Program Counter pc 
};

enum AsmPsrRegisters
{
	AsmPsrRegistersCPSR = 0,
	AsmPsrRegistersSPSR
};


class Asm : public comm::Asm
{
	// File to disassemble
	static std::string path;

	// Unique instance of the singleton
	static std::unique_ptr<Asm> instance;

	// Private constructor for singleton
	Asm();

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

	// ARM instruction table 
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
	static Asm *getInstance();

	/// Destructor
	~Asm();

	/// Thumb Disassembler Functions
	int TestThumb32(const char *inst_ptr);
	void Thumb16InstDebugDump();
	void Thumb32InstDebugDump();
	void Thumb32SetupTable(const char* name ,const char* fmt_str ,
		InstThumb32Category cat32 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, int op7, int op8, InstThumb32Opcode inst_name);
	void Thumb16SetupTable(const char* name, const char* fmt_str ,
		InstThumb16Category cat16 , int op1 , int op2 , int op3 ,
		int op4 , int op5 , int op6, InstThumb16Opcode inst_name);

	/// ELF file processing function
	void ElfSymbolFunc(const ELFReader::File &file,
		unsigned int inst_addr, AsmDisassemblyMode disasm_mode);
	unsigned int ElfDumpWordSymbol(const ELFReader::File &file,
			unsigned int inst_addr, unsigned int *inst_ptr);
	unsigned int ElfDumpThumbWordSymbol(const ELFReader::File &file,
			unsigned int inst_addr, unsigned int *inst_ptr);
	void ElfSymbolListCreate(const ELFReader::File &file,
			std::vector<ELFReader::Symbol*> &symbol_list);

	/// Disasseble Mode
	AsmDisassemblyMode DissassembleMode(const std::vector<ELFReader::Symbol*> &symbol_list,
			unsigned int addr);

	/// Disassembler
	void DisassembleBinary(const std::string &path);

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

};


} //namespace ARM


#endif // ARM_ASM_ASM_H
