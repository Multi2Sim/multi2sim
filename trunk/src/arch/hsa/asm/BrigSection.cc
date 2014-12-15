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

#include <lib/cpp/String.h>

#include "BrigFile.h"
#include "BrigEntry.h"
#include "BrigDataEntry.h"
#include "BrigSection.h"

namespace HSA{

BrigSection::BrigSection(ELFReader::Section *elfSection, BrigFile *binary):
		binary(binary),
		elf_section(elfSection)
{
}


BrigSection::~BrigSection()
{
}


void BrigSection::DumpSectionHex(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n********** Section %s **********\n", 
			this->getName().c_str());
	
	const unsigned char *buf = (const unsigned char *)this->getBuffer();
	for (unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", buf[i]);
		if ((i + 1) % 4 == 0)
		{
			os << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			os << "\n";
		}
	}
	os << "\n";
}


std::unique_ptr<BrigEntry> BrigSection::getFirstEntry() const
{
	struct BrigSectionHeader *header = 
			(struct BrigSectionHeader *)getBuffer();
	return getEntryByOffset(header->headerByteCount);
}


std::unique_ptr<BrigEntry> BrigSection::getEntryByOffset(
		unsigned int offset) const
{
	struct BrigSectionHeader *header = 
			(struct BrigSectionHeader *)getBuffer();
	if (offset < header->headerByteCount)
		return std::unique_ptr<BrigEntry>(nullptr);

	if (offset >= header->byteCount)
		return std::unique_ptr<BrigEntry>(nullptr);

	char *entry_base = (char *)getBuffer() + offset;
	return std::unique_ptr<BrigEntry>(new BrigEntry(entry_base, this));
}


std::unique_ptr<BrigDataEntry> BrigSection::getDataEntryByOffset(
		unsigned int offset) const
{
	// Check if the request is made on hsa_data section
	if (elf_section->getName() != "hsa_data")
		throw misc::Panic("Data entry is only allowed in hsa_data "
				"section");

	// Get the section header for the section length
	struct BrigSectionHeader *header = 
			(struct BrigSectionHeader *)getBuffer();

	// Check if the offset is two small
	if (offset < header->headerByteCount)
		return std::unique_ptr<BrigDataEntry>(nullptr);

	// Check if the offset is beyond the boundary of current section
	if (offset >= header->byteCount)
		return std::unique_ptr<BrigDataEntry>(nullptr);

	// Return the entry
	char *entry_base = (char *)getBuffer() + offset;
	return std::unique_ptr<BrigDataEntry>(
			new BrigDataEntry(entry_base, this));
}

}  // namespace HSA

