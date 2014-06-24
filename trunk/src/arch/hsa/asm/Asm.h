/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_HSA_ASM_ASM_H
#define ARCH_HSA_ASM_ASM_H
 
#include <arch/common/Asm.h>
#include <lib/cpp/CommandLine.h>

#include "BrigInstEntry.h"


namespace HSA
{

/// HSA dissassembler singleton
class Asm : public comm::Asm
{
	// File to disassemble
	static std::string path;

protected:
	
	// The decode table, 
	struct InstInfo inst_info[InstOpcodeCount];

	// Fill in the inst info with the data from Inst.def
	void InitTable(InstOpcode opcode, const char *name, const char *fmt_str);

	// Instance of the singleton
	static std::unique_ptr<Asm> instance;

	// Indent of current line
	int indent;

	// Private constructor for singleton
	Asm();

public:

	/// Checks if the elf file to be loaded
	static bool isValidBrigELF(const std::string &path)
	{
		BrigFile bf(path.c_str());
		return bf.isValid();
	}
	
	/// Returns the pointer to the only instance of has disassembler
	static Asm *getInstance();

	/// Return an array with instruction information
	const struct InstInfo *getInstInfo() const { return inst_info; }

	/// Disassemble the Brig file into HSAIL format
	void DisassembleBinary(const std::string &path) const;

	/// Get the current indent level
	int getIndent(){return indent;}

	/// Increase current indent level by 1
	void IndentMore(){++indent;}

	/// Decrease current indent level by 1
	void IndentLess(){--indent;}

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};


} // namespace HSA

#endif
