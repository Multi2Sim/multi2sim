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
	std::string call_debug_file;
	std::string ctx_debug_file;
	std::string cuda_debug_file;
	std::string glut_debug_file;
	std::string isa_debug_file;
	std::string loader_debug_file;
	std::string opencl_debug_file;
	std::string opengl_debug_file;
	std::string syscall_debug_file;

public:

	/// Register command-line options related with the x86 emulator
	void Register(misc::CommandLine &command_line);

	/// Process command-line options related with the x86 emualtor
	void Process();
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

	/// The x86 emulator is a singleton class. The only possible instance of
	/// it will be allocated the first time this function is invoked.
	static Emu *getInstance();

	/// Create a new context associated with the emulator
	Context *newContext(const std::vector<std::string> &args,
			const std::vector<std::string> &env,
			const std::string &cwd,
			const std::string &stdin_file_name,
			const std::string &stdout_file_name);


	/// Debugger for function calls
	static misc::Debug call_debug;

	/// Debugger for x86 contexts
	static misc::Debug ctx_debug;

	/// Debugger for CUDA driver
	static misc::Debug cuda_debug;

	/// Debugger for GLUT driver
	static misc::Debug glut_debug;
	
	/// Debugger for x86 ISA emulation
	static misc::Debug isa_debug;

	/// Debugger for program loader
	static misc::Debug loader_debug;

	/// Debugger for OpenCL driver
	static misc::Debug opencl_debug;

	/// Debugger for OpenGL driver
	static misc::Debug opengl_debug;
	
	/// Debugger for system calls
	static misc::Debug syscall_debug;

	

	/// Configuration for x86 emulator
	static EmuConfig config;
};


}  // namespace x86

#endif

