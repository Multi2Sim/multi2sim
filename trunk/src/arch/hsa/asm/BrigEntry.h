#ifndef HSA_ASM_BRIGENTRY_H
#define HSA_ASM_BRIGENTRY_H

#include "BrigFile.h"
#include "BrigDef.h"
#include "SymbolModifier.h"

namespace HSA{
/// Pure virtual class representing a brig entry
class BrigEntry
{
protected: 
	// Pointer to the first byte of the entry
	char *base;
	// Brig file that this entry belongs to 
	BrigFile *file;

	// Converts type to type string
	static const char *type2str(int type);

	// Convert profile to string to print out
	static const char *profile2str(int profile);
	
	// Convert profile to string
	static const char *machineModel2str(int machineModel);

	// Returns the align string
	static const char *align2str(unsigned char align);

	// Returns segments as string 
	// Segments are attributes such as global, readonly...
	static const char *seg2str(unsigned char seg);

	// Dump value list
	void dumpValueList(
			BrigDataOffset32_t data, 
			BrigType16_t type, 
			uint32_t elementCount, 
			std::ostream &os
		) const;
	// Dump Symbol Declaration
	static void dumpSymDecl(
			const BrigEntry *dir,
			std::ostream &os
		);
	
	// Dump Argument list
	// Returns a pointer to the next directive to be processed
	static char *dumpArgs(
			char *arg, 
			unsigned short argCount,
			std::ostream &os
		);
	
	// Dump the function or kernel body
	void dumpBody(
			int codeOffset,
			int nInst, 
			bool isDecl, 
			std::ostream &os
		) const;
public:
	/// Constructor
	BrigEntry(char *buf, BrigFile *file);

	/// Returns the size of the entry, the size is paddint into 4 bytes
	/// String entry should have different implementation, since the size
	/// field in string entry is 4 bytes long
	virtual unsigned int getSize() const;

	/// Dump the entry in HEX
	virtual void dumpHex(std::ostream &os) const;

	/// Virtual function for dump assembly
	virtual void Dump(std::ostream &os) const;

	/// Returns the pointer to the next entry in logic
	/// For example: for a directive entry, it should return next directive 
	/// entry of the same level.
	virtual char *next() const{return this->base + this->getSize();}

};
}

#endif
