/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_X86_EMU_EMU_H
#define ARCH_X86_EMU_EMU_H

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

namespace x86
{

class Context;
class Asm;

/// x86 Emulator configuration
class EmuConfig : public misc::CommandLineConfig
{
};

/// x86 Emulator
class Emu
{
	// Unique instance of x86 emulator
	static std::unique_ptr<Emu> instance;

	// Disassembler
	Asm *as;

	// List of allocated contexts
	std::list<std::unique_ptr<Context>> contexts;

	// Private constructor. The only possible instance of the x86 emulator
	// can be obtained with a call to getInstance()
	Emu();

public:

	/// Get the only instance of the x86 emulator. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static Emu *getInstance();

	/// Create a new context associated with the emulator
	Context *NewContext(const std::vector<std::string> &args);
	
	/// Debug category for x86 ISA emulation
	static misc::Debug isa_debug;

	/// Debug category for system calls
	static misc::Debug syscall_debug;
};


}  // namespace x86

#endif

