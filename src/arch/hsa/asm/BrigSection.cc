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

#include <lib/cpp/String.h>

#include "BrigFile.h"
#include "BrigSection.h"

namespace HSA{

BrigSection::BrigSection(char *buffer):
		buffer(buffer)
{
}


BrigSection::~BrigSection()
{
}


unsigned long long BrigSection::getSize() const
{
	BrigSectionHeader *header = (BrigSectionHeader *)buffer;	
	return header->byteCount;
}


std::string BrigSection::getName() const
{
	BrigSectionHeader *header = (BrigSectionHeader *)buffer;	
	return std::string((char *)header->name, 0, 
			header->nameLength);
}


void BrigSection::DumpSectionHex(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n********** Section %s **********\n", 
			this->getName().c_str());
	
	for (unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", ((unsigned char *)buffer)[i]);
		if ((i + 1) % 4 == 0)
		{
			os << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			os << "\n";
		}
	}
	os << "\n";
}

}  // namespace HSA

