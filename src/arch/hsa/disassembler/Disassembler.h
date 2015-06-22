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

#ifndef ARCH_HSA_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_HSA_DISASSEMBLER_DISASSEMBLER_H
 
#include <arch/common/Disassembler.h>
#include <lib/cpp/CommandLine.h>

namespace HSA
{

/// HSA disassembler singleton
class Disassembler : public comm::Disassembler
{
	// File to disassemble
	static std::string path;

protected:
	
	// Instance of the singleton
	static std::unique_ptr<Disassembler> instance;

	// Indent of current line
	int indent = 0;

	// Set if indentation is enabled
	bool doIndent = true;

	// Private constructor for singleton
	Disassembler() : comm::Disassembler("HSA") { }

public:

	/// Returns the pointer to the only instance of has disassembler
	static Disassembler *getInstance();

	/// Disassemble the Brig file into HSAIL format
	void DisassembleBinary(const std::string &path) const;

	/// Get the current indent level
	int getIndent() const
	{
		if (doIndent)
			return indent;
		else
			return 0;
	}

	/// Increase current indent level by 1
	void IndentMore() { ++indent; }

	/// Decrease current indent level by 1
	void IndentLess() { --indent; }

	/// Disable indentation
	void DisableIndentation() { doIndent = false; }

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};


}  // namespace HSA

#endif
