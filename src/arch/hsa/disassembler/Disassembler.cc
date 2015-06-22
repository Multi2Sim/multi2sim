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

#include "Brig.h"
#include "BrigFile.h"
#include "BrigSection.h"
#include "BrigCodeEntry.h"
#include "Disassembler.h"


namespace HSA
{

// File to disassembler, set by user
std::string Disassembler::path;

// Singleton instance
std::unique_ptr<Disassembler> Disassembler::instance;


void Disassembler::RegisterOptions()
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


void Disassembler::ProcessOptions()
{
	// Run hsa disassembler
	if (!path.empty())
	{
		Disassembler *disassembler = Disassembler::getInstance();
		disassembler->DisassembleBinary(path);
		exit(0);
	}
}


Disassembler *Disassembler::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Disassembler());
	return instance.get();
}


void Disassembler::DisassembleBinary(const std::string &path) const
{
	BrigFile brig_file;
	brig_file.LoadFileByPath(path);
	BrigSection *brig_section = 
			brig_file.getBrigSection(BRIG_SECTION_INDEX_CODE);

	// Iterate all entries in code section
	auto entry = brig_section->getFirstEntry<BrigCodeEntry>();
	while(entry.get())
	{
		entry->Dump(std::cout);
		entry = entry->NextTopLevelEntry();
	}
}

}  // namespace HSA
