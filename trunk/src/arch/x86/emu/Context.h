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
#include "UInst.h"


namespace x86
{

class Context;
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

// Saved host flags during emulation
extern long context_host_flags;

// Assembly code used before and after instruction emulation when the host flags
// are affected by the guest code
#define __X86_CONTEXT_SAVE_FLAGS__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (context_host_flags));
#define __X86_CONTEXT_RESTORE_FLAGS__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	: "=m" (context_host_flags));




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

	// Instruction pointers
	unsigned last_eip;  // Address of last emulated instruction
	unsigned current_eip;  // Address of currently emulated instruction
	unsigned target_eip;  // Target address for branch, even if not taken

	// Virtual address of the memory access performed by the last emulated
	// instruction.
	unsigned effective_address;

	// Last emulated instruction
	Inst inst;
	
	// For segmented memory access in glibc
	unsigned glibc_segment_base;
	unsigned glibc_segment_limit;


	// Update the context state, updating also the presence on the context
	// in the various context lists in the emulator.
	void UpdateState(unsigned state);

	// Dump debug information about a call instruction
	void DebugCallInst();
	

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

	// String map from program header types
	static misc::StringMap program_header_type_map;

	// Loader information. This information can be shared among multiple
	// contexts. For this reason, it is declared as a shared pointer. The
	// last destructed context sharing this variable will automatically free
	// it.
	std::shared_ptr<Loader> loader;

	// Load environment variables in 'loader.env' into the stack
	void LoadEnv();

	// Load ELF sections from binary
	void LoadELFSections(ELFReader::File *binary);

	// Load dynamic linker
	void LoadInterp();

	// Load program headers
	void LoadProgramHeaders();

	// Load entry of the auxiliary vector
	void LoadAVEntry(unsigned &sp, unsigned type, unsigned value);

	// Load auxiliary vector and return its size in bytes
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


	///////////////////////////////////////////////////////////////////////
	//
	// Functions related with x86 micro-instructions. These functions are
	// implemented in file ContextUInst.cc
	//
	///////////////////////////////////////////////////////////////////////

	// True if we are in timing simulation and need to active
	// micro-instructions.
	bool uinst_active;

	// True if the effective address computation for the current macro-
	// instruction has already been emitted. This flag is used to avoid
	// multiple address computations for macro-instruction that implicitly
	// load, operate, and store.
	bool uinst_effaddr_emitted;

	// List of micro-instructions produced during the emulation of the last
	// x86 macro-instruction.
	std::vector<std::unique_ptr<UInst>> uinst_list;

	// Clear the list of micro-instructions
	void ClearUInstList() {
		uinst_list.clear();
		uinst_effaddr_emitted = false;
	}

	// If dependence at position \a index is a memory operand, return its
	// associated standard dependence in \a std_dep and its size as the
	// return value of the function. The function returns 0 if the
	// dependence is not a memory dependence.
	int getMemoryDepSize(UInst *uinst, int index, UInstDep &std_dep);

	// Emit the effective address computation micro-instructions. Argument
	// \a index is the dependency index, a value between 0 and
	// UInstMaxDeps - 1
	void EmitUInstEffectiveAddress(UInst *uinst, int index);

	// Parse regular dependence, given as a global dependence index. The
	// value in \a index must be between 0 and UInstMaxDeps -1
	void ParseUInstDep(UInst *uinst, int index);

	// Parse input dependences. Argument \a index is a value between 0 and
	// UInstMaxIDeps - 1
	void ParseUInstIDep(UInst *uinst, int index);
	
	// Parse output dependence. Argument \a index is a value between 0 and
	// UInstMaxODeps - 1
	void ParseUInstODep(UInst *uinst, int index);

	// Process a newly created micro-instruction. The object must have been
	// allocated with \c new. This function will insert it into \c
	// uinst_list, and assign it to a smart pointer for automatic release.
	void ProcessNewUInst(UInst *uinst);

	// Add a new memory micro-instruction to the list only if we're running
	// in timing simulation mode. This function can be invoked directly by
	// the instruction emulation functions. This function is written inline
	// to avoid passing the high number of arguments.
	void newMemoryUInst(UInstOpcode opcode, unsigned address, int size,
			UInstDep idep0, UInstDep idep1, UInstDep idep2,
			UInstDep odep0, UInstDep odep1, UInstDep odep2,
			UInstDep odep3)
	{
		// Discard if we're in function simulation mode
		if (!uinst_active)
			return;

		// Create micro-instruction
		UInst *uinst = new UInst(opcode);
		uinst->setMemoryAccess(address, size);
		uinst->setIDep(0, idep0);
		uinst->setIDep(1, idep1);
		uinst->setIDep(2, idep2);
		uinst->setODep(0, odep0);
		uinst->setODep(1, odep1);
		uinst->setODep(2, odep2);
		uinst->setODep(3, odep3);

		// Process it
		ProcessNewUInst(uinst);
	}

	// Add a new micro-instruction to the list only if we're running
	// in timing simulation mode, omitting the \a address and \a size
	// arguments. This function can be invoked directly by the instruction
	// emulation functions.
	void newUInst(UInstOpcode opcode, UInstDep idep0, UInstDep idep1,
			UInstDep idep2, UInstDep odep0, UInstDep odep1,
			UInstDep odep2, UInstDep odep3)
	{
		newMemoryUInst(opcode, 0, 0, idep0, idep1, idep2, odep0, odep1,
				odep2, odep3);
	}


	///////////////////////////////////////////////////////////////////////
	//
	// Functions and fields related with x86 instruction emulation,
	// implemented in ContextIsaXXX.cc files
	//
	///////////////////////////////////////////////////////////////////////

	// Prototype of a member function of class Context devoted to the
	// execution of ISA instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef void (Context::*ExecuteInstFn)();

	// Instruction emulation functions. Each entry of asm.dat will be
	// expanded into a function prototype. For example, entry
	//
	// 	DEFINST(adc_al_imm8, 0x14, SKIP, SKIP, SKIP, IB, 0)
	//
	// is expanded to
	//
	//	void ExecuteInst_adc_al_imm8();
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) void ExecuteInst_##name();
#include <arch/x86/asm/asm.dat>
#undef DEFINST

	// Table of functions
	static ExecuteInstFn execute_inst_fn[InstOpcodeCount];

	// Fatal error message during instruction emulation, shown only while
	// in non-speculative mode.
	void IsaError(const char *fmt, ...);
	
	// These are some functions created automatically by the macros in
	// ContextIsaStd.cc, but corresponding to non-existing instructions.
	// Since they're not declared in asm.dat, they must be explicitly
	// declared here.
	void ExecuteInst_test_rm16_imm8();
	void ExecuteInst_test_rm32_imm8();
	void ExecuteInst_test_r8_rm8();
	void ExecuteInst_test_r16_rm16();
	void ExecuteInst_test_r32_rm32();

	// Load from register/memory
	unsigned char LoadRm8();
	unsigned short LoadRm16();
	unsigned int LoadRm32();
	unsigned short LoadR32M16();
	unsigned long long LoadM64();

	// Store into register/memory
	void StoreRm8(unsigned char value);
	void StoreRm16(unsigned short value);
	void StoreRm32(unsigned int value);
	void StoreM64(unsigned long long value);

	// Load value from register
	unsigned char LoadR8() { return regs.Read((InstReg) (inst.getModRmReg() + InstRegAl)); }
	unsigned short LoadR16() { return regs.Read((InstReg) (inst.getModRmReg() + InstRegAx)); }
	unsigned int LoadR32() { return regs.Read((InstReg) (inst.getModRmReg() + InstRegEax)); }
	unsigned short LoadSReg() { return regs.Read((InstReg) (inst.getModRmReg() + InstRegEs)); }

	// Store value into register
	void StoreR8(unsigned char value) { regs.Write((InstReg) (inst.getModRmReg() + InstRegAl), value); }
	void StoreR16(unsigned short value) { regs.Write((InstReg) (inst.getModRmReg() + InstRegAx), value); }
	void StoreR32(unsigned int value) { regs.Write((InstReg) (inst.getModRmReg() + InstRegEax), value); }
	void StoreSReg(unsigned short value) { regs.Write((InstReg) (inst.getModRmReg() + InstRegEs), value); }

	// Load value from index register
	unsigned char LoadIR8() { return regs.Read((InstReg) (inst.getOpIndex() + InstRegAl)); }
	unsigned short LoadIR16() { return regs.Read((InstReg) (inst.getOpIndex() + InstRegAx)); }
	unsigned int LoadIR32() { return regs.Read((InstReg) (inst.getOpIndex() + InstRegEax)); }

	// Store value into index register
	void StoreIR8(unsigned char value) { regs.Write((InstReg) (inst.getOpIndex() + InstRegAl), value); }
	void StoreIR16(unsigned short value) { regs.Write((InstReg) (inst.getOpIndex() + InstRegAx), value); }
	void StoreIR32(unsigned int value) { regs.Write((InstReg) (inst.getOpIndex() + InstRegEax), value); }

	// Return the final address obtained from binding \a address inside
	// the corresponding segment. The segment boundaries are checked.
	unsigned getLinearAddress(unsigned offset);

	// Return the effective address obtained from the 'SIB' and 'disp'
	// fields, and store it in 'effective_address' field for the future
	unsigned getEffectiveAddress();

public:
	
	/// Position of the context in the main context list. This field is
	/// managed by the emulator. When a context is removed from the main
	/// context list, it is automatically freed.
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;

	/// Flag indicating whether this context is present in a certain context
	/// list of the emulator. This field is exclusively managed by the
	/// emulator.
	bool context_list_present[ContextListCount];

	/// Position of the context in a certain context list. This field is
	/// exclusively managed by the emulator.
	std::list<Context *>::iterator context_list_iter[ContextListCount];

	/// Create a context from a command line. To safely create a context,
	/// function Emu::NewContext() should be used instead.
	Context();

	/// Destructor
	~Context();

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
	void clearState(ContextState state) { UpdateState(this->state
			& ~state); }

	/// Run one instruction for the context at the position pointed to by
	/// register \c eip.
	void Execute();
};

}  // namespace x86

#endif

