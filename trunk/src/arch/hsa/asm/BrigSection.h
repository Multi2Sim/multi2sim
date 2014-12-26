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

#ifndef HSA_ASM_BRIGSECTION_H
#define HSA_ASM_BRIGSECTION_H
 
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>

#include "BrigDef.h"
#include "BrigEntry.h"

namespace HSA
{

class BrigFile;

/// This class represents a section in a BRIG file. 
class BrigSection
{

protected:

	// Brig file that this section belongs to
	BrigFile *binary;

	// The section of the elf file. 
	ELFReader::Section *elf_section;

public:

	/// Creates the BRIG section, set the type and read in buffer
	BrigSection(ELFReader::Section *elfSection, BrigFile *binary);

	/// Deconstructor
	~BrigSection();

	/// Return the binary file that contains this section
	BrigFile *getBinary() const { return binary; }

	/// Returns the name of section
	const std::string &getName() const { return elf_section->getName(); }

	/// Returns the size of the section
	Elf32_Word getSize() const { return elf_section->getSize(); }

	/// Returns a pointer to the section contest. 
	const char *getBuffer() const { return elf_section->getBuffer(); }

	/// Dump section on stdout in HEX
	void DumpSectionHex(std::ostream &os) const;

	/// Return an unique_ptr to the first entry in the section
	template <typename T>
	std::unique_ptr<T> getFirstEntry() const
	{
		struct BrigSectionHeader *header =
				(struct BrigSectionHeader *)getBuffer();
		return getEntryByOffset<T>(header->headerByteCount);
	}

	/// Return an unique_ptr to the entry at a specific offset. If the
	/// the offset is invalid, return nullptr
	template<typename T>	
	std::unique_ptr<T> getEntryByOffset(unsigned int offset) const
	{
		struct BrigSectionHeader *header =
				(struct BrigSectionHeader *)getBuffer();
		if (offset < header->headerByteCount)
			return std::unique_ptr<T>(nullptr);

		if (offset >= header->byteCount)
			return std::unique_ptr<T>(nullptr);

		char *entry_base = (char *)getBuffer() + offset;
		return std::unique_ptr<T>(new T(entry_base, this));
	}

};

}  // namespace HSA

#endif

