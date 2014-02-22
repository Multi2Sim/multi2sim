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

#include <lib/cpp/Debug.h>
#include <lib/cpp/ELFReader.h>
#include <mem-system/Memory.h>

#include "FileTable.h"
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

/// Context list identifiers
enum ContextListType
{
	// No 'Invalid' identifier here
	ContextListRunning = 0,
	ContextListSuspended,
	ContextListZombie,
	ContextListFinished,

	// Number of context lists
	ContextListCount
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
	std::shared_ptr<mem::Memory> memory;

	// Register file. Each context has its own copy always.
	Regs regs;

	// File descriptor table, private for each context.
	FileTable file_table;

	// Flag indicating whether this context is present in a certain context
	// list, and if true, iterator indicating its position inside of that
	// list.
	bool in_context_list[ContextListCount];
	std::list<Context *>::iterator context_list_iter[ContextListCount];

	// Add/remove context in a context list of the emulator
	void AddToContextList(ContextListType type);
	void RemoveFromContextList(ContextListType type);
	void UpdateContextList(ContextListType type, bool present);

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
		std::unique_ptr<ELFReader::File> binary;

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
		std::string stdin_file_name;
		std::string stdout_file_name;

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
	};

	// Loader information. This information can be shared among multiple
	// contexts. For this reason, it is declared as a shared pointer. The
	// last destructed context sharing this variable will automatically free
	// it.
	std::shared_ptr<Loader> loader;

	// Load environment variables in 'loader.env' into the stack
	void LoadEnv();

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

	// Load ELF binary, as already decoded in 'loader.binary'
	void LoadBinary();

	
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

	/// Create a context from a command line. To safely create a context,
	/// function Emu::NewContext() should be used instead.
	Context();

	/// Load a program from a command line into an existing context. The
	/// content is left in a state ready to start running the first x86 ISA
	/// instruction at the program entry.
	///
	/// \param args
	///	Command line to be used, where the first argument contains the
	///	path to the executable ELF file.
	/// \param env
	///	Array of environment variables. The environment variables
	///	actually loaded in the program is the vector of existing
	///	environment variables in the M2S process, together with any
	///	extra variable contained in this array.
	/// \param cwd
	///	Initial current working directory for the context. Relative
	///	paths used by the context will be relative to this directory.
	/// \param stdin_file_name
	///	File to redirect the standard input, or empty
	/// 	string for no redirection.
	/// \param stdout_file_name
	///	File to redirect the standard output and standard error output,
	///	or empty string for no redirection.
	void loadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env,
			const std::string &cwd,
			const std::string &stdin_file_name,
			const std::string &stdout_file_name);

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

