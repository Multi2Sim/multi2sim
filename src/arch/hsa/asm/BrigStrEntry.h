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

#ifndef HSA_ASM_BRIGSTRENTRY_H
#define HSA_ASM_BRIGSTRENTRY_H

#include "BrigEntry.h"

namespace HSA
{

/// Represents a Brig String Entry
class BrigStrEntry : public BrigEntry
{

public:

	/// Constructor
	BrigStrEntry(char *buf, BrigFile *file);

	/// Returns number of byte 
	unsigned int getByteCount() const;

	/// Returns the bytes that this string entry takes
	unsigned int getSize() const;

	/// Dumps the string
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigStrEntry &string)
	{
		string.Dump(os);
		return os;
	}

	/// Dumps the string simply with file and string offset
	static void DumpString(BrigFile *file, uint32_t offset,
			std::ostream &os);
	
	// Converts the Brig String Entry to a std::string
	std::string getString() const;

	// Returns the std::string from a offset to the brig string section
	static std::string GetStringByOffset(BrigFile *file,
			uint32_t offset);

};

}  // namespace HSA

#endif

