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
	std::multimap<char *, char *> directive_code_map;

	// Stores the current code-directive-map iterator for faster map look-up
	std::multimap<char *, char *>::iterator map_it;

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
	
	/// Return a pointer to a code entry where the next directive entry will be dumped.
	const char * getCode() const { return map_it->first;}

	/// Return a pointer to a directive entry which needs to be dumped at the current code entry.
	const char * getDirective() const { return map_it->second; }

	/// Move to next directive entry (could be the same code entry)
	void moveToNextDirective() { map_it++; }

	/// Search for the code in the map and update the map iterator
	/// This function should be used when entering a new function or kernel in case the code section is not in order
	/// Returns FALSE if code is not found
	bool searchCode(char * code);

	/// Disassembles the binary file, output to stdout
	void disassemble();

	void printCode();
};

}

#endif
