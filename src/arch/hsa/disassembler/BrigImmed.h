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

#ifndef ARCH_HSA_DISASSEMBLER_BRIGIMMED_H
#define ARCH_HSA_DISASSEMBLER_BRIGIMMED_H

#include <map>
#include <cstring>

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>

#include "Brig.h"
#include "AsmService.h" 

namespace HSA
{

/// Abstraction of brig immediate item
class BrigImmed
{
	// pointer to the immediate item
	const unsigned char* ptr;

	// type of the item
	BrigType type;

public:

	/// Constructor
	BrigImmed(const unsigned char* ptr, BrigType type) : 
			ptr(ptr),
			type(type)
	{}

	/// Definition of functions to dump immediate items
	typedef 
		const unsigned char* (BrigImmed::*DumpImmedFn)(
			const unsigned char *ptr, std::ostream &os
		) const;

	/// Map from type to size of the item
	static std::map<int, int> type_to_size_map;

	/// Returns the size of the element according to the type field
	int getSize();

	/// Returns the value of the immediate type
	void getImmedValue(void *buffer)
	{
		int size = getSize();
		memcpy(buffer, ptr, size);
	};

	/// Map of the functions for dumping immed
	static std::map<int, DumpImmedFn> dump_immed_fn;

	/// Dump operand
	void Dump(std::ostream &os = std::cout) const
	{
		// os << "Dump immed type: " << BrigEntry::type2str(this->type);
		DumpImmedFn fn = 
			BrigImmed::dump_immed_fn[type];

		// Check if the function is 
		if (!fn)
		{
			throw misc::Panic(misc::fmt(
					"Immed type %s not supported\n", 
					AsmService::TypeToString(type).c_str()));
		}

		// Call function to dump
		(this->*fn)(this->ptr, os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigImmed &immed)
	{
		immed.Dump(os);
		return os;
	}

	/// Dumps the immed operand according to the inst type
	const unsigned char* dumpImmedNONE(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU32(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU64(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedSIG32(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedSIG64(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS32(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS64(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF32(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF64(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB1(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB32(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB64(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedB128(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedSAMP(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedROIMG(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedRWIMG(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU8X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU8X8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU8X16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU16X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU16X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU16X8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU32X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU32X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedU64X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS8X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS8X8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS8X16(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS16X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS16X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS16X8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS32X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS32X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedS64X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF16X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF16X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF16X8(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF32X2(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF32X4(const unsigned char *ptr, std::ostream &os) const;
	const unsigned char* dumpImmedF64X2(const unsigned char *ptr, std::ostream &os) const;
	void dumpImmedUnsupported(std::ostream &os) const;
};

}  // namespace HSA

#endif
