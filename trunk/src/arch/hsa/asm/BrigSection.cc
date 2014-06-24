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

#include "BrigSection.h"
#include "BrigFile.h"

namespace HSA{

BrigSection::BrigSection(ELFReader::Section *elfSection)
{
	this->elf_section = elfSection;
	std::string sectionName = elfSection->getName();	
	
	//Determine section type according to its name
	if(sectionName.compare(".strtab") == 0)
	{
		this->type = BrigSectionString; 
	}
	else if(sectionName.compare(".directives") == 0)
	{
		this->type = BrigSectionDirective;
	}
	else if(sectionName.compare(".operands") == 0)
	{
		this->type = BrigSectionOperand;
	}
	else if(sectionName.compare(".code") == 0)
	{
		this->type = BrigSectionCode;
	}
	else if(sectionName.compare(".debug") == 0)
	{
		this->type = BrigSectionDebug;
	}
	else if(sectionName.compare(".shstrtab") == 0)
	{
		this->type = BrigSectionShstrtab;
	}
	else if(sectionName.compare("") == 0)
	{
		this->type = BrigSectionUnknown;
	}
	else
	{
		misc::panic("Unexpected section: %s", sectionName.c_str());
		this->type = BrigSectionUnknown;
	}
}

BrigSection::~BrigSection()
{
}

void BrigSection::dumpSectionHex(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n********** Section %s **********\n", 
			this->getName().c_str());
	
	const unsigned char *buf = (const unsigned char *)this->getBuffer();
	for(unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", buf[i]);
		if((i + 1) % 4 == 0)
		{
			os << " ";
		}
		if((i + 1) % 16 == 0)
		{
			os << "\n";
		}
	}
	os << "\n";
}

}