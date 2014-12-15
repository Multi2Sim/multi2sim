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

#include "BrigDataEntry.h"
#include "BrigFile.h"

namespace HSA
{

BrigFile::BrigFile(const std::string &path): 
		file(path),
		brig_sections()
{	
	for (int i = 0; i < file.getNumSections(); i++)
	{
		// Skip empty section
		if (file.getSection(i)->getSize() == 0) 
			continue;

		// Create section
		auto section = misc::new_unique<BrigSection>(
				file.getSection(i), this);
		brig_sections.push_back(std::move(section));
	}
}


BrigFile::~BrigFile()
{
}


BrigSection *BrigFile::getBrigSection(unsigned int index) const
{
	return this->brig_sections[index].get();
}


bool BrigFile::isValid() const
{
	// If the section names match BRIG standard, it is considered to be
	// a valid brig file
	std::vector<std::string> secNames =
	{
		"hsa_data", 
		"hsa_code",
		"hsa_operand",
	};

	// Traverse all sections and compare the names
	for(unsigned int i=1; i<secNames.size(); i++)
	{
		BrigSection *sec = this->brig_sections[i].get();
		if(!(sec->getName() == secNames[i])) 
		{
			// section name does not match
			return false;
		}
	}
	return true;
}


const std::string BrigFile::getStringByOffset(unsigned int offset) const
{
	BrigSection *data_section = getBrigSection(BrigSectionHsaData);
	std::unique_ptr<BrigDataEntry> data_entry = 
			data_section->getDataEntryByOffset(offset);
	return data_entry->getString();
}


/*
char *BrigFile::findMainFunction()
{
	// get pointers to code and dir section
	BrigSection *dirSection = this->getBrigSection(BrigSectionDirective);
	BrigSection *codeSection = this->getBrigSection(BrigSectionCode);
	char *codeBuf = (char *)codeSection->getBuffer();
	const char *dirBuf = dirSection->getBuffer();

	// skip the header, the size field, of the section
	char *bufPtr = (char *)dirBuf;
	bufPtr += 4;

	// Traverse all the directives to find function declarations
	while(bufPtr && bufPtr < dirBuf + dirSection->getSize())
	{
		BrigDirEntry dir(bufPtr, this);
		if(dir.getKind() == BRIG_DIRECTIVE_FUNCTION)
		{
			// function declarations
			struct BrigDirectiveFunction *dirStruct = 
					(struct BrigDirectiveFunction *)
					dir.getBuffer();
			std::string funcName = 
					BrigStrEntry::GetStringByOffset(this, 
						dirStruct->name);

			// Defines the program entry point as the function name
			// as \c &main
			if(funcName == "&main")
			{	
				char *firstInst = codeBuf + dirStruct->code;
				return firstInst;	
			}
		}
		bufPtr = dir.nextTop();
	}
	return nullptr;
}
*/

}  // namespace HSA
