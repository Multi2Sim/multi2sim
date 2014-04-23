#ifndef HSA_ASM_BRIGSTRENTRY_H
#define HSA_ASM_BRIGSTRENTRY_H

#include "BrigEntry.h"

namespace HSA{

/// Represents a Brig String Entry
class BrigStrEntry : public BrigEntry
{
public:
	/// Constructor
	BrigStrEntry(char *buf, BrigFile *file);

	/// Returns number of byte 
	unsigned int getByteCount() const;

	/// Returns the bytes that this string entry takes
	unsigned int getSize() const;

	/// Dumps the string
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(
			std::ostream &os, 
			const BrigStrEntry &string
		) 
	{
		string.Dump(os);
		return os;
	}

	/// Dumps the string simply with file and string offset
	static void DumpString(
			BrigFile *file, 
			uint32_t offset, 
			std::ostream &os
		);
	
	// Converts the Brig String Entry to a std::string
	std::string getString() const;

	// Returns the std::string from a offset to the brig string section
	static std::string GetStringByOffset(
			BrigFile *file,
			uint32_t offset
		);
};

}

#endif
