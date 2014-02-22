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

#include <arch/common/Emu.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

#include "Context.h"


namespace x86
{

class Context;
class Asm;


/// x86 Emulator configuration
class EmuConfig : public misc::CommandLineConfig
{
	std::string call_debug_file;
	std::string context_debug_file;
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


/// x86 emulator
class Emu : public Common::Emu
{
	// Unique instance of x86 emulator
	static std::unique_ptr<Emu> instance;

	// Disassembler
	Asm *as;

	// Primary list of contexts
	std::list<std::unique_ptr<Context>> contexts;

	// Secondary lists of contexts. Contexts in different states
	// are added/removed from this lists as their state gets updated.
	std::list<Context *> context_list[ContextListCount];

	// See setScheduleSignal()
	bool schedule_signal;

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

	/// Add a context to a context list. This function does not update
	/// the internal state of the context. Call Context::addToList()
	/// instead. The function returns an iterator to the inserted context.
	std::list<Context *>::iterator AddToContextList(ContextListType type,
			Context *context)
	{
		context_list[type].push_back(context);
		auto iter = context_list[type].end();
		return --iter;
	}

	/// Remove a context from a context list. This function does not
	/// update the internal state of the context. Call
	/// Context::removeFromList() instead.
	void RemoveFromContextList(ContextListType type,
			std::list<Context *>::iterator &iter)
	{
		context_list[type].erase(iter);
	}

	/// Return a constant reference to a context list, given its type.
	const std::list<Context *> &getContextList(ContextListType type) const {
			return context_list[type]; }
	
	/// Signals a call to the scheduler Timing::Schedule() in the
	/// beginning of next cycle. This flag is set any time a context changes
	/// its state in any bit other than ContextSpecMode. It can be set
	/// anywhere in the code by directly assigning a value to 1. E.g.:
	/// when a system call is executed to change the context's affinity.
	void setScheduleSignal() { schedule_signal = true; }

	/// Run one iteration of the emulation loop.
	/// \return This function \c true if the iteration had a useful
	/// emulation, and \c false if all contexts finished execution.
	bool Run();


	/// Debugger for function calls
	static misc::Debug call_debug;

	/// Debugger for x86 contexts
	static misc::Debug context_debug;

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

