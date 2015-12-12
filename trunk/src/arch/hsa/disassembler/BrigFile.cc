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
#include <cstring>
#include <fstream>
#include <istream>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>

#include "Brig.h"
#include "BrigOperandEntry.h"
#include "BrigCodeEntry.h"
#include "BrigDataEntry.h"
#include "BrigFile.h"

namespace HSA
{

void BrigFile::LoadFileByPath(const std::string &path)
{	
	// Open file
	std::ifstream f(path);
	if (!f)
		throw misc::Error(misc::fmt("Cannot open file %s.", 
					path.c_str()));

	// Get file size
	f.seekg(0, std::ios_base::end);
	int size = f.tellg();
	f.seekg(0, std::ios_base::beg);

	// Load the entire file into buffer and close
	buffer.reset(new char[size]);
	f.read(buffer.get(), size);
	f.close();

	// Verify the file is valid
	if (!isBrigFile(buffer.get()))
	{
		throw misc::Error(misc::fmt("%s is not a valid BRIG file",
					path.c_str()));
	}

	// Prepare sections
	PrepareSections();
}


void BrigFile::LoadFileFromBuffer(const char *file)
{
	// Check if the file is a BRIG file
	if (!isBrigFile(file))
	{
		throw misc::Error("Not a valid BRIG file.");
	}

	// Get the size of the module
	BrigModuleHeader *header = (BrigModuleHeader *)file;
	unsigned long long byteCount = header->byteCount;
	buffer.reset(new char[byteCount]);
	memcpy(buffer.get(), file, byteCount);

	// Prepare sections
	PrepareSections();

}


BrigFile::~BrigFile()
{

}


unsigned int BrigFile::getNumSections() const 
{
	BrigModuleHeader *header = (BrigModuleHeader *)buffer.get();
	return header->sectionCount;
}


void BrigFile::PrepareSections()
{
	for (unsigned int i = 0; i < getNumSections(); i++)
	{
		// Get section index
		BrigModuleHeader *header = (BrigModuleHeader *)buffer.get();
		unsigned long long *section_indices = 
			(unsigned long long *)
			(buffer.get() + header->sectionIndex);

		// From the section index, get section offset
		unsigned long long section_offset = section_indices[i];

		// Create section
		auto section = misc::new_unique<BrigSection>(
				buffer.get() + section_offset);
		section->setBinary(this);
		sections.push_back(std::move(section));
	}
}


BrigSection *BrigFile::getBrigSection(BrigSectionIndex section_index) const
{
	return sections[section_index].get();
}


bool BrigFile::isBrigFile(const char *file)
{
	BrigModuleHeader *header = (BrigModuleHeader *)file;
	if (strncmp(header->identification, "HSA BRIG", 8) == 0)
	{
		return true;
	}
	return false;
}


std::unique_ptr<BrigCodeEntry> BrigFile::getCodeEntryByOffset(
		unsigned int offset) const
{
	BrigSection *code_section = getBrigSection(BRIG_SECTION_INDEX_CODE);
	std::unique_ptr<BrigCodeEntry> entry =
			code_section->getEntryByOffset<BrigCodeEntry>(offset);
	return entry;
}


const std::string BrigFile::getStringByOffset(unsigned int offset) const
{
	BrigSection *data_section = getBrigSection(BRIG_SECTION_INDEX_DATA);
	std::unique_ptr<BrigDataEntry> data_entry =
			data_section->getEntryByOffset<BrigDataEntry>(offset);
	return data_entry->getString();
}


std::unique_ptr<BrigDataEntry> BrigFile::getDataEntryByOffset(
		unsigned int offset) const
{
	BrigSection *data_section = getBrigSection(BRIG_SECTION_INDEX_DATA);
	std::unique_ptr<BrigDataEntry> data_entry =
			data_section->getEntryByOffset<BrigDataEntry>(offset);
	return data_entry;
}


std::unique_ptr<BrigOperandEntry> BrigFile::getOperandByOffset(
		unsigned int offset) const
{
	BrigSection *operand_section = getBrigSection(BRIG_SECTION_INDEX_OPERAND);
	std::unique_ptr<BrigOperandEntry> entry = operand_section->
			getEntryByOffset<BrigOperandEntry>(offset);
	return entry;
}

}  // namespace HSA
