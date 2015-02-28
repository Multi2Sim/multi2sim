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
#include <vector>

#include <lib/cpp/ELFReader.h>

#include "Asm.h"


namespace Fermi
{


std::unique_ptr<Asm> Asm::instance;

Asm::Asm()
{
	unsigned cat, op_in_cat;
	InstInfo *info;

	// Read information about all instructions
#define DEFINST(_op, _fmt_str, _opcode) \
		cat = _opcode >> 8; \
		op_in_cat = (cat <= 3) ? (_opcode & 0xf8) >> 3 : (_opcode & 0xfc) >> 2; \
		info = &inst_info_table[cat][op_in_cat]; \
		info->id = InstId##_op; \
		info->op = InstOp##_op; \
		info->category = (InstCategory) cat; \
		info->name = #_op; \
		info->fmt_str = _fmt_str;
#include "asm.dat"
#undef DEFINST
}

Asm *Asm::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Asm());
	return instance.get();
}


void Asm::DisassembleBinary(const std::string &path) const
{
	ELFReader::File file(path);
	std::vector<ELFReader::Section *> text_sections;
	std::vector<std::vector<std::unique_ptr<Inst>>> instructions;
	std::vector<unsigned> max_inst_len;
	unsigned kernel_index;

	// Disassemble binary
	kernel_index = 0;
	for (int i = 0; i < file.getNumSections(); i++)
	{
		// Determine if section is a text section
		ELFReader::Section *section = file.getSection(i);
		if (section->getName().compare(0, 6, ".text.") == 0)
		{
			std::vector<std::unique_ptr<Inst>> instructions_in_kernel;

			// remember text sections
			text_sections.push_back(section);

			// Disassemble instructions
			max_inst_len.push_back(0);
			for (unsigned pos = 0; pos < section->getSize(); pos += 8)
			{
				std::unique_ptr<Inst> inst(new Inst());

				inst->Decode(pos, section->getBuffer() + pos);
				inst->DumpToBuf();

				if (inst->getStr().length() > max_inst_len[kernel_index])
					max_inst_len[kernel_index] = inst->getStr().length();

				instructions_in_kernel.push_back(std::move(inst));
			}

			instructions.push_back(std::move(instructions_in_kernel));

			++kernel_index;
		}
	}

	// Dump assembly
	kernel_index = 0;
	std::cout << "\n\tcode for sm_20\n";
	for (std::vector<ELFReader::Section *>::iterator
			it = text_sections.begin(); it != text_sections.end();
			++it, ++kernel_index)
	{
		const std::string kernel_name = (*it)->getName().substr(6);
		const std::string dots(17 + kernel_name.length(), '.');
		std::cout << "\t\tFunction : " << kernel_name << "\n";
		std::cout << "\t.headerflags    "
				<< "@\"EF_CUDA_SM20 EF_CUDA_PTX_SM(EF_CUDA_SM20)\"\n";
		for (std::vector<std::unique_ptr<Inst>>::iterator
				it = instructions[kernel_index].begin();
				it != instructions[kernel_index].end(); ++it)
		{
			std::cout << "        ";
			(*it)->DumpPC(std::cout);
			(*it)->Dump(std::cout, max_inst_len[kernel_index]);
			(*it)->DumpHex(std::cout);
			std::cout << "\n";
		}
		std::cout << "\t\t" << dots << "\n\n\n";
	}
}


}  // namespace Fermi

