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

#ifndef HSA_ASM_BRIGIMMED_H
#define HSA_ASM_BRIGIMMED_H

#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"
#include <map>
#include "BrigEntry.h"


namespace HSA
{

/// Abstraction of brig immediate item
class BrigImmed
{
	// pointer to the immediate item
	unsigned char* ptr;

	// type of the item
	int type;

public:
	/// Constructor
	BrigImmed(unsigned char* ptr, int type)
	{
		this->ptr = ptr; 
		this->type = type;
	}

	/// Definition of functions to dump immedite items
	typedef 
		unsigned char* (BrigImmed::*DumpImmedFn)(
			unsigned char *ptr, std::ostream &os
		) const;

	/// Map from type to size ot the item
	static std::map<int, int> type_to_size_map;

	/// Returns the size of the element according to the type field
	int getSize();

	/// Map of the functions for dumping immed
	static std::map<int, DumpImmedFn> dump_immed_fn;

	/// Dump operand
	void Dump(std::ostream &os = std::cout) const
	{
		// os << "Dump immed type: " << BrigEntry::type2str(this->type);
		DumpImmedFn fn = 
			BrigImmed::dump_immed_fn[this->type];
		(this->*fn)(this->ptr, os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(
			std::ostream &os, 
			const BrigImmed &immed
		) 
	{
		immed.Dump(os);
		return os;
	}

	/// Dumps the immed operand according to the inst type
	unsigned char* dumpImmedNONE(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU32(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU64(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS32(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS64(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF32(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF64(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB1(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB32(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB64(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedB128(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedSAMP(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedROIMG(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedRWIMG(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedFBAR(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU8X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU8X8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU8X16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU16X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU16X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU16X8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU32X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU32X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedU64X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS8X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS8X8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS8X16(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS16X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS16X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS16X8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS32X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS32X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedS64X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF16X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF16X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF16X8(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF32X2(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF32X4(unsigned char *ptr, std::ostream &os) const;
	unsigned char* dumpImmedF64X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedUnsupported(std::ostream &os) const;
};

}

#endif