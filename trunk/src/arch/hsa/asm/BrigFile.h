#ifndef HSA_ASM_BRIGFILE_H
#define HSA_ASM_BRIGFILE_H
#include <cstdint>
#include <map>
#include "lib/cpp/ELFReader.h"
#include <elf.h>
#include "BrigSection.h"
#include "BrigDef.h"

namespace HSA{

class BrigSection;


/// This class represents the ELF file defined by HSA standard, or called 
/// BRIG format. It encapsulates the ELFReader class and provide unique 
/// interfaces to the other parts of the Multi2sim.
class BrigFile
{
protected:

	// The elf file, read by ELFReader
	ELFReader::File file;

	// A vector that holds 5 sections defined in BRIG standard;
	// brigSections[0] - .string
	// brigSections[1] - .directive
	// ...
	// The index corresponds with the BrigSectionType enumerator 
	std::vector<std::unique_ptr<BrigSection>> brig_sections;

	// A map that stores pointers to directive entries and their code offsets.
	// key: code offset of a directive entry, converted to a pointer to a code entry
	// mapped value: a pointer to a directive entry
	std::multimap<BrigInstBase *, BrigDirectiveBase *> directive_code_map;

public:
	/// Loads a BRIG File from the file system, create sections
	BrigFile(const std::string &path);

	/// Destructor
	~BrigFile();

	/// Returns the path to the BRIG file
	const std::string &getPath() const { return file.getPath(); }

	/// Returns the section according to the type value passed in
	BrigSection *getBrigSection (
		enum BrigSectionType type
	) const;
	
	/// Disassembles the binary file, output to stdout
	void disassemble();
};

}

#endif
