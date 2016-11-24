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

#ifndef ARCH_HSA_DISASSEMBLER_BRIGFILE_H
#define ARCH_HSA_DISASSEMBLER_BRIGFILE_H

#include <cstdint>
#include <memory>
#include <vector>

#include <lib/cpp/Error.h>

#include "Brig.h"
#include "BrigSection.h"

namespace HSA
{
class BrigSection;
class BrigCodeEntry;
class BrigDataEntry;
class BrigOperandEntry;

/// This class represents the ELF file defined by HSA standard, or called 
/// BRIG format. It encapsulates the ELFReader class and provide unique 
/// interfaces to the other parts of the Multi2sim.
class BrigFile
{
	// The path to the file
	std::string path;

	// The buffer of the file content
	std::unique_ptr<char> buffer;

	// An list of BRIG sections, maps section index to section
	std::vector<std::unique_ptr<BrigSection>> sections;

	// Set the section vector
	void PrepareSections();

public:

	/// Constructor
	BrigFile() {};

	/// Loads a BRIG file from the file system, create sections
	void LoadFileByPath(const std::string &path);

	/// Loads a BRIG file from a memory block
	/// BrigFile(char *file, unsigned size);

	/// Load the file from a chunk of memory
	void LoadFileFromBuffer(const char *file);

	/// Destructor
	~BrigFile();

	/// Returns the path to the BRIG file
	const std::string &getPath() const { return path; }

	/// Return the buffer of the BRIG file
	const char *getBuffer() const { return buffer.get(); }

	/// Returns the section according to the type value passed in
	BrigSection *getBrigSection(BrigSectionIndex section_index) const;

	/// Checks if the loaded brig file is a brig file
	///
	/// \return
	///	Returns \c true if the loaded file is valid
	///
	static bool isBrigFile(const char *file);


	/// Returns the number of sections in the BRIG file
	unsigned int getNumSections() const;

	/// Retrieve an entry in the code section
	std::unique_ptr<BrigCodeEntry> getCodeEntryByOffset(
			unsigned int offset) const;

	/// Return the string that is stored in the hsa_data section by its 
	/// offset
	virtual const std::string getStringByOffset(unsigned int offset) const;

	/// Return the data entry at a certain offset
	virtual std::unique_ptr<BrigDataEntry> getDataEntryByOffset(
			unsigned int offset) const;

	/// Return an operand from the operand section by offset
	virtual std::unique_ptr<BrigOperandEntry> getOperandByOffset(
			unsigned int offset) const;
};

} // namespace HSA

#endif
