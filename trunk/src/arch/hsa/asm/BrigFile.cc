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

#include "BrigFile.h"

namespace HSA
{

BrigFile::BrigFile(const std::string &path)
	: file(path)
	, brig_sections(6)
{	
	for(int i=0; i<file.getNumSections(); i++)
	{
		BrigSection * section = new BrigSection(file.getSection(i));

		//only add known section to section list
		if( section->getType() >=0 && section->getType() <=5 )
		{
			this->brig_sections[section->getType()]
					= std::unique_ptr<BrigSection> (section);
		} 
	}
}

BrigFile::~BrigFile()
{
}

BrigSection *BrigFile::getBrigSection(BrigSectionType type) const
{
	return this->brig_sections[type].get();
}

void BrigSection::dumpSectionHex() const
{
	printf(
			"\n********** Section %s **********\n", 
			this->getName().c_str()
		);
	const unsigned char *buf = (const unsigned char *)this->getBuffer();
	for(unsigned int i=0; i<this->getSize(); i++)
	{
		printf("%02x", buf[i]);
		if((i + 1) % 4 == 0)
		{
			printf(" ");
		}
		if((i + 1) % 16 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
}

}// end namespace
