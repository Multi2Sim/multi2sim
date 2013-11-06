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
	unsigned int cat;
	unsigned int func;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _op) \
	info = &inst_info[FRM_INST_##_name]; \
	info->opcode = INST_##_name; \
	cat = _op >> 8; \
	if (cat <= 3) \
		func = (_op & 0xf8) >> 3; \
	else \
		func = (_op & 0xfc) >> 2; \
	info->category = (InstCategory)(_op >> 8); \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->op = _op; \
	info->size = 8; \
	dec_table[cat][func] = info;
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
	for (int i = 0; i < file.getNumSections(); i++)
	{
		/* Determine if section is .text.kernel_name */
		ELFReader::Section *section = file.getSection(i);
		if (!strncmp(section->getName().c_str(), ".text.", 6))
		{
			cout << "\t\tFunction : " << section->getName().c_str() + 6 << "\n";
			for (unsigned int pos = 0; pos < section->getSize(); pos += 8)
			{
				/* Decode instruction */
				inst.Decode(pos, section->getBuffer() + pos);

				/* Dump instruction */
				cout << "\t";
				inst.DumpPC(cout);
				cout << "\t";
				inst.Dump(cout);
				cout << ";";
				cout << "\t";
				inst.DumpHex(cout);
				cout << "\n";
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


FrmInstInfo *FrmAsmWrapGetInstInfo(struct FrmAsmWrap *self, FrmInstOpcode opcode)
{
	Asm *as = (Asm *) self;
	return (FrmInstInfo *) as->GetInstInfo((InstOpcode) opcode);
}


FrmInstInfo *FrmAsmWrapGetDecTable(struct FrmAsmWrap *self, int cat, int func)
{
	Asm *as = (Asm *) self;
	return (FrmInstInfo *) as->GetDecTable(cat, func);
}
