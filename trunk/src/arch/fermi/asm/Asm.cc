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
#include <iomanip>
#include <vector>

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
	vector<Inst *> inst_vector;
	unsigned int max_inst_len;
	Inst *inst;

	/* Dump disassembly */
	cout << "\n\tcode for sm_20\n";
	for (int i = 0; i < file.getNumSections(); i++)
	{
		/* Determine if section is .text.kernel_name */
		ELFReader::Section *section = file.getSection(i);
		if (!strncmp(section->getName().c_str(), ".text.", 6))
		{
			max_inst_len = 0;
			for (unsigned int pos = 0; pos < section->getSize(); pos += 8)
			{
				inst = new Inst(this);

				/* Decode instruction */
				inst->Decode(pos, section->getBuffer() + pos);
				inst->DumpToBuf();

				inst_vector.push_back(inst);
			}

			for (unsigned int j = 0; j < inst_vector.size(); ++j)
			{
				unsigned int inst_len = inst_vector[j]->GetString().length();
				if (inst_len > max_inst_len)
					max_inst_len = inst_len;
			}

			/* Dump */
			cout << "\t\tFunction : " << section->getName().c_str() + 6 << "\n";
			cout << "\t.headerflags    @\"EF_CUDA_SM20 EF_CUDA_PTX_SM(EF_CUDA_SM20)\"" << "\n";
			for (unsigned int j = 0; j < inst_vector.size(); ++j)
			{
				Inst *inst = inst_vector[j];
				cout << "        ";
				inst->DumpPC(cout);
				inst->Dump(cout, max_inst_len);
				inst->DumpHex(cout);
				cout << "\n";

				delete inst;
			}
			cout << "\t\t" << setfill('.') << setw(32) << "." << "\n\n\n";
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
		inst.Dump(cout, 100);
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
