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

#include <iomanip>

#include <lib/cpp/ELFReader.h>

#include "Asm.h"
#include "Inst.h"

using namespace MIPS;
using namespace std;

/* Opcodes and secondary opcodes */
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


/*
 * Class 'Asm'
 */

Asm::Asm()
{
	/* Allocate storage for the instruction tables */
	dec_table                   = new InstInfo[64]();
	dec_table_special           = new InstInfo[64]();
	dec_table_special_movci     = new InstInfo[2]();
	dec_table_special_srl       = new InstInfo[2]();
	dec_table_special_srlv      = new InstInfo[2]();

	dec_table_regimm            = new InstInfo[32]();

	dec_table_cop0              = new InstInfo[32]();
	dec_table_cop0_c0           = new InstInfo[64]();
	dec_table_cop0_notc0        = new InstInfo[32]();
	dec_table_cop0_notc0_mfmc0  = new InstInfo[2]();

	dec_table_cop1              = new InstInfo[32]();
	dec_table_cop1_bc1          = new InstInfo[4]();
	dec_table_cop1_s            = new InstInfo[64]();
	dec_table_cop1_s_movcf      = new InstInfo[2]();
	dec_table_cop1_d            = new InstInfo[64]();
	dec_table_cop1_d_movcf      = new InstInfo[2]();
	dec_table_cop1_w            = new InstInfo[64]();
	dec_table_cop1_l            = new InstInfo[64]();
	dec_table_cop1_ps           = new InstInfo[64]();

	dec_table_cop2              = new InstInfo[32]();
	dec_table_cop2_bc2          = new InstInfo[4]();

	dec_table_special2          = new InstInfo[64]();

	dec_table_special3          = new InstInfo[64]();
	dec_table_special3_bshfl    = new InstInfo[32]();

	/* Initiate values for the 'next_table', 'next_table_low' and 'next_table_high'
	 * fields of the tables */
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

	/* Build the tables from asm.dat */
	InstInfo *current_table;
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
	current_table[table_arg[i]].opcode = INST_##_name;		\
	current_table[table_arg[i]].name = #_name;			\
	current_table[table_arg[i]].fmt_str = _fmt_str;			\
	current_table[table_arg[i]].size = 4;
#include "asm.dat"
#undef DEFINST
}


Asm::~Asm()
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

////////////
#include <lib/cpp/IniFile.h>
///////////

void Asm::DisassembleBinary(string path)
{
	{ /////////////
		IniFile f;

		f.WriteString("section1", "var1", "value1");
		f.WriteString("section1", "var1_2", "value1_2");

		f.WriteString("Section2", "Var2_1", "value2_1");
		f.WriteString("Section2", "Var2_1", "overwrite_value");

		f.WriteDouble("Section1", "var_double", 1.3);

		f.Dump(cout);
		return;
	}

	ELFReader::File file(path);
	Inst inst(this);

	unsigned int curr_sym;
	unsigned int pos;

	/* Read Sections */
	for (int i = 0; i < file.GetNumSections(); i++)
	{
		/* Skip if section does not contain code */
		ELFReader::Section *section = file.GetSection(i);
		if (!(section->GetFlags() & SHF_EXECINSTR))
			continue;

		/* Title */
		cout << "\n\nDisassembly of section " << section->GetName() << ":";

		/* Symbol */
		curr_sym = 0;
		ELFReader::Symbol *symbol = file.GetSymbol(0);

		/* Decode and dump instructions */
		for (pos = 0; pos < section->GetSize(); pos += 4)
		{
			/* Symbol */
			while (symbol && symbol->GetValue() < section->GetAddr() + pos)
			{
				curr_sym++;
				symbol = file.GetSymbol(curr_sym);
			}
			if (symbol && symbol->GetValue() ==
					section->GetAddr() + pos)
				cout << "\n\n" << setw(8) << setfill('0') << hex
						<< section->GetAddr() + pos
						<< " <" << symbol->GetName() << ">:";

			/* Decode and dump */
			inst.Decode(section->GetAddr() + pos,
					section->GetBuffer() + pos);
			inst.DumpHex(cout);
			inst.Dump(cout);

			/* Symbol name */
			if (inst.target)
			{
				ELFReader::Symbol *print_symbol;
				print_symbol = file.GetSymbolByAddress(inst.target);
				if (print_symbol)
				{
					if (print_symbol->GetValue() == inst.target)
						cout << " <" << print_symbol->GetName() << ">";
					else
						cout << " <" << print_symbol->GetName() << "+0x"
								<< hex <<
								inst.target -
								print_symbol->GetValue()
								<< ">";
				}
			}
		}
	}

	/* End */
	cout << '\n';
}




/*
 * C Wrapper
 */

struct MIPSAsmWrap *MIPSAsmWrapCreate(void)
{
	return (struct MIPSAsmWrap *) new Asm;
}


void MIPSAsmWrapFree(struct MIPSAsmWrap *self)
{
	delete (Asm *) self;
}


void MIPSAsmWrapDisassembleBinary(struct MIPSAsmWrap *self, char *path)
{
	Asm *as;
	as = (Asm *) self;
	as->DisassembleBinary(path);
}
