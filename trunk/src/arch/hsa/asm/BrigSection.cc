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


}