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

#include <arch/hsa/asm/AsmService.h>

#include "Emu.h"
#include "Variable.h"
#include "SegmentManager.h"

namespace HSA
{

Variable::~Variable()
{
	if (!isFormal)
	{
		if (segment)
		{
			segment->Free(address);
		}
		else
		{
			mem::Manager *manager = Emu::getInstance()->getMemoryManager();
			manager->Free(address);
		}
	}
}


Variable::Variable(const std::string& name, BrigTypeX type,
		unsigned long long dim, unsigned address,
		SegmentManager *segment, bool isFormal = false) :
		name(name),
		type(type),
		size(AsmService::TypeToSize(type)),
		dim(dim),
		address(address),
		segment(segment),
		isFormal(isFormal)
{
}


template<typename T>
void Variable::DumpValue(std::ostream &os) const
{
	// Get the buffer of the value
	char *buffer = getBuffer();

	// Dump the value
	os << *(T *)buffer;


	// Dump hex format
	unsigned char *char_buffer = (unsigned char *)buffer;

	os << " ( 0x";
	for (unsigned int i = 0; i < sizeof(T); i++)
	{
		os << misc::fmt("%02x", char_buffer[sizeof(T) - i - 1]);
	}
	os << " ) ";

}


void Variable::Dump(std::ostream &os, unsigned int indent,
		bool is_simple_format) const
{
	// Dump input or output
	if (this->isInput())
	{
		os << "Input ";
	}

	// Print indent
	for (unsigned int i = 0; i < indent; i++)
	{
		os << " ";
	}

	if (!is_simple_format)
	{
		// Dump information of the argument.
		os << misc::fmt("%s %s(0x%x)",
				AsmService::TypeToString(type).c_str(),
				name.c_str(), address);
	}
	else
	{
		// Dump information of the argument.
		os << misc::fmt("%s", name.c_str());
	}


	// If this is not a formal variable
	if (!isFormal)
	{
		os << " = ";
		switch (type)
		{
		case BRIG_TYPE_U8:

			DumpValue<unsigned char>(os);
			break;

		case BRIG_TYPE_U16:

			DumpValue<unsigned short>(os);
			break;

		case BRIG_TYPE_U32:

			DumpValue<unsigned int>(os);
			break;

		case BRIG_TYPE_U64:

			DumpValue<unsigned long>(os);
			break;

		case BRIG_TYPE_S8:

			DumpValue<char>(os);
			break;

		case BRIG_TYPE_S16:

			DumpValue<short>(os);
			break;

		case BRIG_TYPE_S32:

			DumpValue<int>(os);
			break;

		case BRIG_TYPE_S64:

			DumpValue<long>(os);
			break;

		case BRIG_TYPE_F32:

			DumpValue<float>(os);
			break;

		case BRIG_TYPE_F64:

			DumpValue<double>(os);
			break;

		default:
			throw misc::Panic("Unimplemented variable type.");
			break;
		}
	}


	// New line
	// os << "\n";

}


unsigned Variable::getFlatAddress() const
{
	if (segment)
	{
		return segment->getFlatAddress(address);
	}
	return address;
}


char *Variable::getBuffer() const
{
	mem::Memory *memory = Emu::getInstance()->getMemory();
	char *buffer = memory->getBuffer(getFlatAddress(), size,
			mem::Memory::AccessWrite);
	return buffer;
}

}  // namespace HSA

