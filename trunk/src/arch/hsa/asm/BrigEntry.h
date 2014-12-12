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

#ifndef ARCH_HSA_ASM_BRIGENTRY_H
#define ARCH_HSA_ASM_BRIGENTRY_H

#include <iostream>
#include <memory>

namespace HSA
{

class BrigFile;
class BrigSection;

/// A BRIG Entry is an entry in the hsa_code or hsa_operand section. It is
/// can represent directive, instruction or operand
class BrigEntry
{
protected:

	// Pointer to the first byte of the entry
	const char *base;

	// A Pointer to the file that this brig entry belongs to 
	const BrigSection *section;

	// Return the pointer in the file to the entry. This function is set 
	// to be private because the BrigEntry should populates all the fields
	// and directly read from the the binary should be forbidden in the 
	// emulator
	const char *getBuffer() const { return base; }

public:
	
	/// Constructor
	BrigEntry(const char *buf, const BrigSection *section);

	/// Return the size in bytes of the entry
	unsigned getSize() const;

	/// Return the kind field of the entry
	unsigned getKind() const;

	/// Get the offset of this entry int the section
	unsigned int getOffset() const;

	/// Return the section that has this entry
	const BrigSection *getSection() const { return section; }

	/// Returns the file that has this entry
	BrigFile *getBinary() const { return section->getBinary(); }

	/// Dump the entry
	void Dump(std::ostream &os) const;

	/// Returns an unique pointer to the next entry
	std::unique_ptr<BrigEntry> next() const;

};

}  // namespace HSA

#endif

