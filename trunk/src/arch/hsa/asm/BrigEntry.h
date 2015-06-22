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

#ifndef ARCH_HSA_DISASSEMBLER_BRIGENTRY_H
#define ARCH_HSA_DISASSEMBLER_BRIGENTRY_H

namespace HSA
{

class BrigSection;
class BrigFile;

// A BrigEntry is a piece of data record from hsa_code, hsa_data or hsa_operand
// section. 
class BrigEntry
{
protected:

	// Pointer to the first byte of the entry
	const char *base;

	// A Pointer to the section that this brig entry belongs to 
	const BrigSection *section;

	// A Pointer to the file that this brig entry belongs to
	const BrigFile *binary;

	// Return the pointer to the brig data. Accessing the buffer should 
	// be forbidden from the ouside of this function
	const char *getBuffer() const { return base; }

public:

	/// Constructor
	BrigEntry(const char *buffer) :
			base(buffer)
	{	
	};

	/// Get the offset of this entry in the section
	unsigned int getOffset() const;

	/// Set the BRIG section is belongs to
	void setSection(const BrigSection *section);

	/// Return the section that has this entry
	const BrigSection *getSection() const { return section; }

	/// Returns the file that has this entry
	const BrigFile *getBinary() const { return binary; };

	/// Set the file that this entry belongs to
	void setBinary(const BrigFile *binary) { this->binary = binary; }

};

}  // namespace HSA

#endif
