#ifndef ARCH_HSA_ASM_ASM_H
#define ARCH_HSA_ASM_ASM_H

#include <lib/cpp/CommandLine.h>

#include "BrigInstEntry.h"


namespace HSA
{


class Asm
{
	// File to disassemble
	static std::string path;

protected:
	
	// The decode table, 
	struct InstInfo inst_info[InstOpcodeCount];

	// Fill in the inst info with the data from asm.dat
	void InitTable(InstOpcode opcode, const char *name, const char *fmt_str);

	// Instance of the singleton
	static std::unique_ptr<Asm> instance;

	// Private constructor for singleton
	Asm();

public:
	
	/// Returns the pointer to the only instance of has disassembler
	static Asm *getInstance();

	/// Return an array with instruction information
	const struct InstInfo *getInstInfo() const { return inst_info; }

	/// Disassemble the Brig file into HSAIL format
	void DisassembleBinary(const std::string &path) const;

	/// Indentation of current line
	int indent;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};


} // namespace HSA

#endif
