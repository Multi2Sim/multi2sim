#ifndef HSA_ASM_BRIGSECTION_H
#define HSA_ASM_BRIGSECTION_H
#include "BrigFile.h"
#include "lib/cpp/ELFReader.h"
#include "lib/cpp/Misc.h"
#include "BrigDef.h"

namespace HSA{

class BrigFile;



/// This class represents a section in a BRIG file. 
class BrigSection
{
protected:
	// The file in which this section is contained
	BrigFile *file;

	// Type of the section
	BrigSectionType type;

	// The section of the elf file. 
	ELFReader::Section *elf_section;

public:
	/// Creates the BRIG section, set the type and read in buffer
	BrigSection(
		ELFReader::Section *elfSection
	);

	/// Deconstructor
	~BrigSection();

	/// Returns the name of section
	const std::string &getName() const { return elf_section->getName(); }

	/// Returns the size of the section
	Elf32_Word getSize() const { return elf_section->getSize(); }

	/// Returns a pointer to the section contest. 
	const char *getBuffer() const { return elf_section->getBuffer(); }
	
	/// Returns the type of the section
	BrigSectionType getType() const { return this->type; }

	/// Dump section on stdout in HEX
	void dumpSectionHex() const;

};
}

#endif