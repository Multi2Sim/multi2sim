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

#ifndef ARCH_HSA_DISASSEMBLER_BRIGDATA_H
#define ARCH_HSA_DISASSEMBLER_BRIGDATA_H

#include <iostream>
#include <memory>

#include "Brig.h"
#include "BrigEntry.h"

namespace HSA
{

class BrigEntry;

/// A BrigEntry is an entry in the hsa_data section. The major difference is
/// that is uses 4 bytes for the size of the entry
class BrigDataEntry : public BrigEntry
{
	// The struct that of the data entry
	struct BrigData *data;

public:

	/// Constructor
	BrigDataEntry(const char *buf) : 
			BrigEntry(buf)
	{
		data = (BrigData *)buf;
	}

	/// Return the data as string
	const std::string getString() const;

	/// Return the byte count of the data entry
	unsigned int getByteCount() const;

	/// Return the pointer to the byte field 
	const unsigned char *getBytes() const;

};
	
}  // namespace HSA

#endif


