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
	// Put every in-function directive entry in the multimap.
	const char * directive_section_start = this->brig_sections[BrigSectionDirective].get()->getBuffer();
	const char * code_section_start = this->brig_sections[BrigSectionCode].get()->getBuffer();
	char * directive_ptr = (char *)directive_section_start + 4;
	char * kernel_function_end = 0;
	bool if_in_kernel_function = 0;
	struct BrigDirectiveBase * dir;
	while(directive_ptr < code_section_start)
	{
		dir = (struct BrigDirectiveBase *) directive_ptr;
		if(directive_ptr == kernel_function_end)
			if_in_kernel_function = 0;
		if(dir->kind == BRIG_DIRECTIVE_KERNEL || dir->kind == BRIG_DIRECTIVE_FUNCTION)
		{
			if_in_kernel_function = 1;
			struct BrigDirectiveExecutable * kernel_function = (struct BrigDirectiveExecutable *) dir;
			kernel_function_end = (char *)directive_section_start + kernel_function->nextTopLevelDirective;
		}else if(if_in_kernel_function)
		{
			// add only the directives in a function or a kernel to the map
			// code offset is converted to a pointer to the code entry.
			char * cod = (char *)code_section_start + dir->code;
			directive_code_map.insert( std::pair<char *, char *>(cod, directive_ptr) );
		}
		directive_ptr += (dir->size + 3) / 4 * 4;
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
