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


namespace MIPS
{


//
// Configuration options
//

std::string Disassembler::path;



// Opcodes and secondary opcodes
#define OPCODE_SPECIAL              0x00
#define OPCODE_SPECIAL2             0x1c
#define OPCODE_SPECIAL3             0x1f
#define OPCODE_REGIMM               0x01
#define OPCODE_COP0                 0x10
#define OPCODE_COP1                 0x11
#define OPCODE_COP2                 0x12
#define OPCODE_COP1X                0x13

#define OPCODE_SPECIAL_MOVCI        0x01
#define OPCODE_SPECIAL_SRL          0x02
#define OPCODE_SPECIAL_SRLV         0x06

#define OPCODE_COP0_C0              0x01
#define OPCODE_COP0_NOTC0           0x00
#define OPCODE_COP0_NOTC0_MFMC0     0x0b

#define OPCODE_COP1_BC1             0x08
#define OPCODE_COP1_S               0x10
#define OPCODE_COP1_S_MOVCF         0x10
#define OPCODE_COP1_D               0x11
#define OPCODE_COP1_D_MOVCF         0x11
#define OPCODE_COP1_W               0x14
#define OPCODE_COP1_L               0x15
#define OPCODE_COP1_PS              0x16

#define OPCODE_COP2_BC2             0x08

#define OPCODE_SPECIAL3_BSHFL       0x20


void Disassembler::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Set category
	command_line->setCategory("MIPS");

	// Option --mips-disasm <file>
	command_line->RegisterString("--mips-disasm", path,
			"Disassemble the MIPS ELF file provided in <arg>, "
			"using the internal MIPS disassembler. This option is "
			"incompatible with any other option.");

	// Incompatible options
	command_line->setIncompatible("--mips-disasm");}


void Disassembler::ProcessOptions()
{
	// Run x86 disassembler
	if (!path.empty())
	{
		Disassembler *disassembler = Disassembler::getInstance();
		disassembler->DisassembleBinary(path);
		exit(0);
	}
}


Disassembler::Disassembler() : comm::Disassembler("MIPS")
{
	// Allocate storage for the instruction tables
	dec_table                   = new Instruction::Info[64]();
	dec_table_special           = new Instruction::Info[64]();
	dec_table_special_movci     = new Instruction::Info[2]();
	dec_table_special_srl       = new Instruction::Info[2]();
	dec_table_special_srlv      = new Instruction::Info[2]();

	dec_table_regimm            = new Instruction::Info[32]();

	dec_table_cop0              = new Instruction::Info[32]();
	dec_table_cop0_c0           = new Instruction::Info[64]();
	dec_table_cop0_notc0        = new Instruction::Info[32]();
	dec_table_cop0_notc0_mfmc0  = new Instruction::Info[2]();

	dec_table_cop1              = new Instruction::Info[32]();
	dec_table_cop1_bc1          = new Instruction::Info[4]();
	dec_table_cop1_s            = new Instruction::Info[64]();
	dec_table_cop1_s_movcf      = new Instruction::Info[2]();
	dec_table_cop1_d            = new Instruction::Info[64]();
	dec_table_cop1_d_movcf      = new Instruction::Info[2]();
	dec_table_cop1_w            = new Instruction::Info[64]();
	dec_table_cop1_l            = new Instruction::Info[64]();
	dec_table_cop1_ps           = new Instruction::Info[64]();

	dec_table_cop2              = new Instruction::Info[32]();
	dec_table_cop2_bc2          = new Instruction::Info[4]();

	dec_table_special2          = new Instruction::Info[64]();

	dec_table_special3          = new Instruction::Info[64]();
	dec_table_special3_bshfl    = new Instruction::Info[32]();

	// Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	// fields of the tables
	dec_table[OPCODE_SPECIAL].next_table =
		dec_table_special;
	dec_table[OPCODE_SPECIAL].next_table_low                  = 0;
	dec_table[OPCODE_SPECIAL].next_table_high                 = 5;

	dec_table_special[OPCODE_SPECIAL_SRL].next_table =
		dec_table_special_srl;
	dec_table_special[OPCODE_SPECIAL_SRL].next_table_low        = 21;
	dec_table_special[OPCODE_SPECIAL_SRL].next_table_high       = 21;

	dec_table_special[OPCODE_SPECIAL_SRLV].next_table =
		dec_table_special_srlv;
	dec_table_special[OPCODE_SPECIAL_SRLV].next_table_low       = 6;
	dec_table_special[OPCODE_SPECIAL_SRLV].next_table_high      = 6;

	dec_table_special[OPCODE_SPECIAL_MOVCI].next_table =
		dec_table_special_movci;
	dec_table_special[OPCODE_SPECIAL_MOVCI].next_table_low      = 16;
	dec_table_special[OPCODE_SPECIAL_MOVCI].next_table_high     = 16;

	dec_table[OPCODE_REGIMM].next_table =
		dec_table_regimm;
	dec_table[OPCODE_REGIMM].next_table_low                       = 16;
	dec_table[OPCODE_REGIMM].next_table_high                      = 20;

	dec_table[OPCODE_COP0].next_table =
		dec_table_cop0;
	dec_table[OPCODE_COP0].next_table_low                         = 25;
	dec_table[OPCODE_COP0].next_table_high                        = 25;

	dec_table_cop0[OPCODE_COP0_C0].next_table =
		dec_table_cop0_c0;
	dec_table_cop0[OPCODE_COP0_C0].next_table_low                 = 0;
	dec_table_cop0[OPCODE_COP0_C0].next_table_high                = 5;

	dec_table_cop0[OPCODE_COP0_NOTC0].next_table =
		dec_table_cop0_notc0;
	dec_table_cop0[OPCODE_COP0_NOTC0].next_table_low              = 21;
	dec_table_cop0[OPCODE_COP0_NOTC0].next_table_high             = 25;

	dec_table_cop0_notc0[OPCODE_COP0_NOTC0_MFMC0].next_table =
		dec_table_cop0_notc0_mfmc0;
	dec_table_cop0_notc0[OPCODE_COP0_NOTC0_MFMC0].next_table_low   = 5;
	dec_table_cop0_notc0[OPCODE_COP0_NOTC0_MFMC0].next_table_high  = 5;

	dec_table[OPCODE_COP1].next_table =
		dec_table_cop1;
	dec_table[OPCODE_COP1].next_table_low                          = 21;
	dec_table[OPCODE_COP1].next_table_high                         = 25;

	dec_table_cop1[OPCODE_COP1_BC1].next_table =
		dec_table_cop1_bc1;
	dec_table_cop1[OPCODE_COP1_BC1].next_table_low                 = 16;
	dec_table_cop1[OPCODE_COP1_BC1].next_table_high                = 17;

	dec_table_cop1[OPCODE_COP1_S].next_table =
		dec_table_cop1_s;
	dec_table_cop1[OPCODE_COP1_S].next_table_low                   = 0;
	dec_table_cop1[OPCODE_COP1_S].next_table_high                  = 5;

	dec_table_cop1_s[OPCODE_COP1_S_MOVCF].next_table =
		dec_table_cop1_s_movcf;
	dec_table_cop1_s[OPCODE_COP1_S_MOVCF].next_table_low           = 16;
	dec_table_cop1_s[OPCODE_COP1_S_MOVCF].next_table_high          = 16;

	dec_table_cop1[OPCODE_COP1_D].next_table =
		dec_table_cop1_d;
	dec_table_cop1[OPCODE_COP1_D].next_table_low                   = 0;
	dec_table_cop1[OPCODE_COP1_D].next_table_high                  = 5;

	dec_table_cop1_d[OPCODE_COP1_D_MOVCF].next_table =
		dec_table_cop1_d_movcf;
	dec_table_cop1_d[OPCODE_COP1_D_MOVCF].next_table_low           = 16;
	dec_table_cop1_d[OPCODE_COP1_D_MOVCF].next_table_high          = 16;

	dec_table_cop1[OPCODE_COP1_W].next_table =
		dec_table_cop1_w;
	dec_table_cop1[OPCODE_COP1_W].next_table_low                   = 0;
	dec_table_cop1[OPCODE_COP1_W].next_table_high                  = 5;

	dec_table_cop1[OPCODE_COP1_L].next_table =
		dec_table_cop1_l;
	dec_table_cop1[OPCODE_COP1_L].next_table_low                   = 0;
	dec_table_cop1[OPCODE_COP1_L].next_table_high                  = 5;

	dec_table_cop1[OPCODE_COP1_PS].next_table =
		dec_table_cop1_ps;
	dec_table_cop1[OPCODE_COP1_PS].next_table_low                  = 0;
	dec_table_cop1[OPCODE_COP1_PS].next_table_high                 = 5;

	dec_table[OPCODE_COP2].next_table = dec_table_cop2;
	dec_table[OPCODE_COP2].next_table_low                          = 21;
	dec_table[OPCODE_COP2].next_table_high                         = 25;

	dec_table[OPCODE_SPECIAL2].next_table =
		dec_table_special2;
	dec_table[OPCODE_SPECIAL2].next_table_low                      = 0;
	dec_table[OPCODE_SPECIAL2].next_table_high                     = 5;

	dec_table[OPCODE_SPECIAL3].next_table =
		dec_table_special3;
	dec_table[OPCODE_SPECIAL3].next_table_low                      = 0;
	dec_table[OPCODE_SPECIAL3].next_table_high                     = 5;

	dec_table_special3[OPCODE_SPECIAL3_BSHFL].next_table =
		dec_table_special3_bshfl;
	dec_table_special3[OPCODE_SPECIAL3_BSHFL].next_table_low        = 6;
	dec_table_special3[OPCODE_SPECIAL3_BSHFL].next_table_high       = 10;

	// Build the tables from Inst.def
	Instruction::Info *current_table;
	unsigned int table_arg[4];
	int i = 0;

#define DEFINST(_name,_fmt_str,_op0,_op1,_op2,_op3)			\
	current_table = dec_table;					\
	table_arg[0] = _op0;						\
	table_arg[1] = _op1;						\
	table_arg[2] = _op2;						\
	table_arg[3] = _op3;						\
	for(i = 0; i < 4; i++)						\
	{								\
		if (current_table[table_arg[i]].next_table)		\
			current_table = current_table[table_arg[i]].next_table; \
		else							\
			break;						\
	}								\
	current_table[table_arg[i]].opcode = Instruction::Opcode##_name;\
	current_table[table_arg[i]].name = #_name;			\
	current_table[table_arg[i]].fmt_str = _fmt_str;			\
	current_table[table_arg[i]].size = 4;
#include "Instruction.def"
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
	delete dec_table;
	delete dec_table_special;
	delete dec_table_special_movci;
	delete dec_table_special_srl;
	delete dec_table_special_srlv;

	delete dec_table_regimm;

	delete dec_table_cop0;
	delete dec_table_cop0_c0;
	delete dec_table_cop0_notc0;
	delete dec_table_cop0_notc0_mfmc0;

	delete dec_table_cop1;
	delete dec_table_cop1_bc1;
	delete dec_table_cop1_s;
	delete dec_table_cop1_s_movcf;
	delete dec_table_cop1_d;
	delete dec_table_cop1_d_movcf;
	delete dec_table_cop1_w;
	delete dec_table_cop1_l;
	delete dec_table_cop1_ps;

	delete dec_table_cop2;
	delete dec_table_cop2_bc2;

	delete dec_table_special2;

	delete dec_table_special3;
	delete dec_table_special3_bshfl;
}


void Disassembler::DisassembleBinary(const std::string &path)
{
	ELFReader::File file(path);
	Instruction inst;

	unsigned int curr_sym;
	unsigned int pos;

	// Read Sections
	for (int i = 0; i < file.getNumSections(); i++)
	{
		// Skip if section does not contain code
		ELFReader::Section *section = file.getSection(i);
		if (!(section->getFlags() & SHF_EXECINSTR))
			continue;

		/* Title */
		std::cout << "\n\nDisassembly of section " << section->getName() << ":";

		/* Symbol */
		curr_sym = 0;
		ELFReader::Symbol *symbol = file.getSymbol(0);

		/* Decode and dump instructions */
		for (pos = 0; pos < section->getSize(); pos += 4)
		{
			/* Symbol */
			while (symbol && symbol->getValue() < section->getAddr() + pos)
			{
				curr_sym++;
				symbol = file.getSymbol(curr_sym);
			}
			if (symbol && symbol->getValue() ==
					section->getAddr() + pos)
				std::cout << misc::fmt("\n\n%08x <%s>:",
						section->getAddr() + pos,
						symbol->getName().c_str());

			/* Decode and dump */
			inst.Decode(section->getAddr() + pos,
					section->getBuffer() + pos);
			inst.DumpHex(std::cout);
			inst.Dump(std::cout);

			/* Symbol name */
			if (inst.getTarget())
			{
				ELFReader::Symbol *print_symbol;
				print_symbol = file.getSymbolByAddress(inst.getTarget());
				if (print_symbol)
				{
					if (print_symbol->getValue() == inst.getTarget())
						std::cout << " <" << print_symbol->getName() << ">";
					else
						std::cout << misc::fmt(" <%s+0x%x>",
								print_symbol->getName().c_str(),
								inst.getTarget() -
								print_symbol->getValue());
				}
			}
		}
	}

	/* End */
	std::cout << '\n';
}




}  // namespace MIPS

