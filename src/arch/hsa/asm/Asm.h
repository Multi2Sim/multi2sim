#ifndef ARCH_HSA_ASM_ASM_H
#define ARCH_HSA_ASM_ASM_H

#include "BrigInstEntry.h"

namespace HSA
{

class Asm
{
protected:
	
	// The decode table, 
	struct InstInfo inst_info[InstOpcodeCount];

	//Fill in the inst info with the data from asm.dat 
	void InitTable(InstOpcode opcode, const char *name, const char *fmt_str);

	// Global instance of the HSA disassembler 
	static std::unique_ptr<Asm> as;

	// Private constructor
	Asm();

	// Disallow copy constructor
	Asm(const Asm &as);

	// Disallow assign operator
	void operator=(const Asm &as);
public:
	
	/// Returns the pointer to the only instance of has disassembler
	static Asm *getInstance();

	/// Returns the inst_info array
	const struct InstInfo *getInstInfo() const
	{
		return inst_info;
	}

	/// Disassemble the Brig file into HSAIL format
	void DisassembleBinary(const std::string &path) const;

}; // class Asm

} // namespace HSA

#endif
