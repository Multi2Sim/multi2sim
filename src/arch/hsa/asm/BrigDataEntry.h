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

#ifndef ARCH_HSA_ASM_BRIGDATA_H
#define ARCH_HSA_ASM_BRIGDATA_H

#include <iostream>
#include <memory>

#include "BrigSection.h"
#include "BrigDef.h"

namespace HSA
{

class BrigFile;
class BrigSection;

// FIXME Combine BrigData and BrigEntry into one shared base class BrigBasicEntry
/// A BrigEntry is an entry in the hsa_data section. The major difference is
/// that is uses 4 bytes for the size of the entry
class BrigDataEntry
{
	// Pointer to the first byte of the data entry
	const char *base;

	// The struct that of the data entry
	struct BrigData *data;

	// A pointer to the section that owns this brig data
	const BrigSection *section;

	// Return the pointer to the brig data. Accessing the buffer should 
	// be forbidden from the ouside of this function
	const char *getBuffer() const { return base; }

public:

	/// Constructor
	BrigDataEntry(const char *buf, const BrigSection* section);

	/// Return the size in bytes of the entry
	unsigned int getSize() const;

	/// Return the section that has this entry
	const BrigSection *getSection() const { return section; }

	/// Returns the file that has this entry
	BrigFile *getBinary() const { return section->getBinary(); }

	/// Returns the data as string
	const std::string getString() const;

};
	
}  // namespace HSA

#endif


