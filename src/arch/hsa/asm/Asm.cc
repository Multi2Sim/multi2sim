/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "Asm.h"
#include "BrigFile.h"
#include "BrigSection.h"
#include "BrigDirEntry.h"

namespace HSA
{

// File to disassembler, set by user
std::string Asm::path;

// Singleton instance
std::unique_ptr<Asm> Asm::instance;


void Asm::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option --hsa-disasm <file>
	command_line->RegisterString("--hsa-disasm <file>", path,
			"Disassemble the HSA BRIG ELF file provided in <arg>. "	
			"This option is incompatible with any other option.");

	// Option incompatibility
	command_line->setIncompatible("--hsa-disasm");
}


void Asm::ProcessOptions()
{
	// Run hsa disassembler
	if (!path.empty())
	{
		Asm *as = Asm::getInstance();
		as->DisassembleBinary(path);
		exit(0);
	}
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


Asm::Asm()
{
	
#define DEFINST(_name, _opstr)	\
		InitTable(Inst_##_name, #_name, _opstr);
#include "Inst.def"
#undef DEFINST
	
	indent = 0;
}


void Asm::InitTable(InstOpcode opcode, const char *name, const char *fmt_str)
{
	inst_info[opcode].opcode = opcode;
	inst_info[opcode].name = name;
	inst_info[opcode].fmt_str = fmt_str;	
}


void Asm::DisassembleBinary(const std::string &path) const
{
	BrigFile brig_file(path.c_str());
	BrigSection *brig_section = 
			brig_file.getBrigSection(BrigSectionDirective);
	const char *buffer = brig_section->getBuffer();

	char *buffer_pointer = (char *)buffer;

	// Increment by 4 to skip the section size field
	buffer_pointer += 4;

	// Traverse all top level directives and dump them
	while (buffer_pointer &&
			buffer_pointer < buffer + brig_section->getSize())
	{
		BrigDirEntry dir(buffer_pointer, &brig_file);
		dir.Dump();
		buffer_pointer = dir.nextTop();
	}
}

}  // namespace HSA
