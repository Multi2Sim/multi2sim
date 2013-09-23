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

#include <cstring>

#include <lib/cpp/ELFReader.h>

#include "Asm.h"


using namespace Fermi;
using namespace std;


Asm::Asm()
{
	InstInfo *info;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _category, _op) \
	info = &inst_info[INST_##_name]; \
	info->opcode = INST_##_name; \
	info->category = InstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->op = _op; \
	info->size = 8; \
	dec_table[_op] = info;
#include "asm.dat"
#undef DEFINST

}


void Asm::DisassembleBinary(string path)
{
	/* Initialization */
	ELFReader::File file(path);
	Inst inst(this);

	/* Dump disassembly */
	cout << "\n\tcode for sm_20\n";
	for (auto it = file.section_list.begin();
			it != file.section_list.end(); ++it)
	{
		/* Determine if section is .text.kernel_name */
		ELFReader::Section *section = *it;
		if (!strncmp(section->name.c_str(), ".text.", 6))
		{
			/* Decode and dump instructions */
			cout << "\t\tFunction : " << section->name.c_str() + 6 << "\n";
			for (unsigned int pos = 0; pos < section->size; pos += 8)
			{
				inst.Decode(pos, section->buffer + pos);
				inst.DumpHex(cout);
				inst.Dump(cout);
				cout << ";\n";
			}
			cout << "\t\t" << std::string(38, '.') << "\n\n\n";
		}
	}
}


void Asm::DisassembleBuffer(char *ptr, unsigned int size)
{
	Inst inst(this);
	for (unsigned int pos = 0; pos < size; pos += 8)
	{
		inst.Decode(pos, ptr + pos);
		inst.DumpHex(cout);
		inst.Dump(cout);
		cout << ";\n";
	}
}




/*
 * C Wrapper
 */

struct FrmAsmWrap *FrmAsmWrapCreate(void)
{
	return (struct FrmAsmWrap *) new Asm();
}


void FrmAsmWrapFree(struct FrmAsmWrap *self)
{
	delete (Asm *) self;
}


void FrmAsmWrapDisassembleBinary(struct FrmAsmWrap *self, char *path)
{
	Asm *as = (Asm *) self;
	as->DisassembleBinary(path);
}


void FrmAsmWrapDisassembleBuffer(struct FrmAsmWrap *self, char *buffer, unsigned int size)
{
	Asm *as = (Asm *) self;
	as->DisassembleBuffer(buffer, size);
}

