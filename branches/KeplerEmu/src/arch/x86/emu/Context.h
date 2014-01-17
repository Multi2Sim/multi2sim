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

#ifndef ARCH_X86_EMU_CONTEXT_H
#define ARCH_X86_EMU_CONTEXT_H

#include <memory>

#include <lib/cpp/ELFReader.h>
#include <mem-system/Memory.h>

#include "Regs.h"
#include "Signal.h"


namespace x86
{

class Emu;

/// Context states
enum ContextState
{
	ContextInvalid      = 0x00000,
	ContextRunning      = 0x00001,  // it is able to run instructions
	ContextSpecMode     = 0x00002,  // executing in speculative mode
	ContextSuspended    = 0x00004,  // suspended in a system call
	ContextFinished     = 0x00008,  // no more inst to execute
	ContextExclusive    = 0x00010,  // executing in excl mode
	ContextLocked       = 0x00020,  // another context is running in excl mode
	ContextHandler      = 0x00040,  // executing a signal handler
	ContextSigsuspend   = 0x00080,  // suspended after syscall 'sigsuspend'
	ContextNanosleep    = 0x00100,  // suspended after syscall 'nanosleep'
	ContextPoll         = 0x00200,  // 'poll' system call
	ContextRead         = 0x00400,  // 'read' system call
	ContextWrite        = 0x00800,  // 'write' system call
	ContextWaitpid      = 0x01000,  // 'waitpid' system call
	ContextZombie       = 0x02000,  // zombie context
	ContextFutex        = 0x04000,  // suspended in a futex
	ContextAlloc        = 0x08000,  // allocated to a core/thread
	ContextCallback     = 0x10000,  // suspended after syscall with callback
	ContextMapped       = 0x20000   // mapped to a core/thread
};


/// x86 Context
class Context
{
	// Emulator that it belongs to
	Emu *emu;

	// Process ID
	int pid;

	// Context state, expressed as a bitmap of flags, e.g.,
	// ContextSuspended | ContextFutex
	unsigned state;

	// Context memory. This object can be shared by multiple contexts, so it
	// is declared as a shared pointer. The las freed context pointing to
	// this memory object will be the one automatically freeing it.
	std::shared_ptr<Memory::Memory> memory;

	// Register file. Each context has its own copy always.
	Regs regs;


	// Update the context state
	void UpdateState(unsigned state);
	

	///////////////////////////////////////////////////////////////////////
	//
	// Functions implemented in ContextLoader.cc. These are the functions
	// related with the program loading process.
	//
	///////////////////////////////////////////////////////////////////////

	/// Structure containing information initialized by the program loader,
	/// associated with a context. When a context is created from a program
	/// executable, a Loader object is associated to it. All child contexts
	/// spawned by it will share the same Loader object.
	struct Loader
	{
		// Program executable
		ELFReader::File file;

		// Command-line arguments
		std::vector<std::string> args;

		// Environment variables
		std::vector<std::string> env;

		// Executable interpreter
		std::string interp;

		// Executable file name
		std::string exe;

		// Current working directory
		std::string cwd;

		// File name for standard input and output
		std::string stdin_file;
		std::string stdout_file;

		// Stack
		unsigned stack_base;
		unsigned stack_top;
		unsigned stack_size;
		unsigned environ_base;

		// Lowest address initialized
		unsigned bottom;

		// Program entries
		unsigned prog_entry;
		unsigned interp_prog_entry;

		// Program headers
		unsigned phdt_base;
		unsigned phdr_count;

		// Random bytes
		unsigned at_random_addr;
		unsigned at_random_addr_holder;

		// Constructor
		Loader(const std::string &path) : file(path) { }
	};

	// Loader information. This information can be shared among multiple
	// contexts. For this reason, it is declared as a shared pointer. The
	// last destructed context sharing this variable will automatically free
	// it.
	std::shared_ptr<Loader> loader;

	// Add arguments to stack
	void AddArgsVector();

	// Add one particular argument to the stack
	void AddArgsString(char *args);

	// Add environment variables to the stack
	void AddEnv(const std::string &env);

	// Load ELF sections from binary
	void LoadELFSections();

	// Load dynamic linker
	void LoadInterp();

	// Load program headers
	void LoadProgramHeaders();

	// Load auxiliary vector
	unsigned LoadAV(unsigned where);

	// Load content of stack
	void LoadStack();

	// Load ELF binary
	void LoadExe();

	
	///////////////////////////////////////////////////////////////////////
	//
	// Fields and functions related with signal handling. The functions are
	// implemented in ContextSignal.cc.
	//
	///////////////////////////////////////////////////////////////////////

	// Table of signal handlers, possibly shared by multiple contexts
	std::shared_ptr<SignalHandlerTable> signal_handler_table;

	// Table of signal masks, each context has its own
	SignalMaskTable signal_mask_table;

	// Run a signal handler for signal \a sig. The value of \a sig must be
	// between 1 and 64.
	void RunSignalHandler(int sig);

	// Return from a signal handler
	void ReturnFromSignalHandler();

	void CheckSignalHandler();

	// Check any pending signal, and run the corresponding signal handler by
	// considering that the signal interrupted a system call
	// (\c syscall_intr). This has the following implication on the return
	// address from the signal handler:
	//   -If flag \c SA_RESTART is set for the handler, the return address
	//    is the system call itself, which must be repeated.
	//   -If flag \c SA_RESTART is not set, the return address is the
	//    instruction next to the system call, and register 'eax' is set to
	//    \c -EINTR.
	void CheckSignalHandlerIntr();

public:

	// Create a context from a command line, given as a vector of arguments.
	// Contexts should be created directly only internally in class Emu. To
	// create a context, function Emu::NewContext() should be used instead.
	Context(Emu *emu, const std::vector<std::string> &args);

	/// Given a file name, return its full path based on the current working
	/// directory for the context.
	std::string getFullPath(const std::string &path);

	/// Return \c true if flag \a state is part of the context state
	bool getState(ContextState state) const { return this->state & state; }

	/// Set flag \a state in the context state
	void setState(ContextState state) { UpdateState(this->state | state); }

	/// Clear flag \a state in the context state
	void clearState(ContextState state) {
		UpdateState(this->state & ~state);
	}
};

}  // namespace x86

#endif

