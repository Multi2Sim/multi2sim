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
#include "BrigCodeEntry.h"
#include "BrigSection.h"

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
	indent = 0;
}


bool Asm::isValidBrigELF(const std::string &path)
{
	BrigFile bf(path.c_str());
	return bf.isValid();
}


void Asm::DisassembleBinary(const std::string &path) const
{
	BrigFile brig_file(path.c_str());
	BrigSection *brig_section = 
			brig_file.getBrigSection(BrigSectionHsaCode);

	auto entry = brig_section->getFirstEntry<BrigCodeEntry>();
	while(entry.get())
	{
		entry->Dump(std::cout);
		entry = entry->NextTopLevelEntry();
	}
}

}  // namespace HSA
