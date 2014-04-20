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