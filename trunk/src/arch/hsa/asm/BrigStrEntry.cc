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

#include "BrigStrEntry.h"

namespace HSA{

BrigStrEntry::BrigStrEntry(char *buf, BrigFile *file)
	: BrigEntry(buf, file)
{
}

unsigned int BrigStrEntry::getByteCount() const
{
	struct BrigString *str = (struct BrigString *)base;
	return str->byteCount;
}

unsigned int BrigStrEntry::getSize() const
{
	unsigned int byteCount = this->getByteCount();
	return (byteCount + 7) * 4 / 4;
}

void BrigStrEntry::Dump(std::ostream &os = std::cout) const
{
	char *temp = (char *)base;
	temp += 4;
	for(unsigned int i=0; i<this->getByteCount(); i++)
	{
		os << *temp;
		temp++;
	}
}

void BrigStrEntry::DumpString(
		BrigFile *file, 
		uint32_t offset,
		std::ostream& os = std::cout)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	char *temp = (char *)stringSection->getBuffer();
	temp += offset;
	BrigStrEntry str(temp, file);
	str.Dump(os);
}

std::string BrigStrEntry::getString() const
{
	std::stringstream ss;
	unsigned char *temp = (unsigned char *)base;
	temp += 4;
	for(unsigned int i=0; i<this->getByteCount(); i++)
	{
		ss << *temp;
		temp++;
	}
	return ss.str();
}

std::string BrigStrEntry::GetStringByOffset(
		BrigFile *file,
		uint32_t offset
	)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	char *temp = (char *)stringSection->getBuffer();
	temp += offset;
	BrigStrEntry str(temp, file);
	return str.getString();
}
}
