/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_ARM_EMU_CONTEXT_H
#define ARCH_ARM_EMU_CONTEXT_H

#include <iostream>
#include <memory>
#include <vector>

#include <arch/common/CallStack.h>
#include <arch/arm/disassembler/Instruction.h>
#include <arch/arm/disassembler/Disassembler.h>
#include <lib/cpp/ELFReader.h>
#include <memory/Memory.h>
#include <memory/SpecMem.h>
#include <arch/common/FileTable.h>

#include "Regs.h"
#include "Signal.h"


namespace ARM
{

class Context;
class Emulator;

#define ARM_SET_TLS 0xF0005
#define ARM_EXIT_GROUP 248

enum ContextThumbIteq
{
	ContextThumbIteqDisabled = 0,
	ContextThumbIteqEnabled
};


enum ContextMode
{
	ContextModeArm = 1,
	ContextModeThumb
};


enum ContextInstType
{
	ContextInstTypeArm32 = 1,
	ContextInstTypeThumb16,
	ContextInstTypeThumb32
};


enum ContextState
{
	ContextStateRunning      = 0x00001,  // it is able to run instructions 
	ContextStateSpecMode     = 0x00002,  // executing in speculative mode 
	ContextStateSuspended    = 0x00004,  // suspended in a system call 
	ContextStateFinished     = 0x00008,  // no more inst to execute 
	ContextStateExclusive    = 0x00010,  // executing in excl mode 
	ContextStateLocked       = 0x00020,  // another context is running in excl mode 
	ContextStateHandler      = 0x00040,  // executing a signal handler 
	ContextStateSigsuspend   = 0x00080,  // suspended after syscall 'sigsuspend' 
	ContextStateNanosleep    = 0x00100,  // suspended after syscall 'nanosleep' 
	ContextStatePoll         = 0x00200,  // 'poll' system call 
	ContextStateRead         = 0x00400,  // 'read' system call 
	ContextStateWrite        = 0x00800,  // 'write' system call 
	ContextStateWaitpid      = 0x01000,  // 'waitpid' system call 
	ContextStateZombie       = 0x02000,  // zombie context 
	ContextStateFutex        = 0x04000,  // suspended in a futex 
	ContextStateAlloc        = 0x08000,  // allocated to a core/thread 
	ContextStateCallback     = 0x10000,  // suspended after syscall with callback 
	ContextStateNone         = 0x00000
};

/// Context list identifiers
enum ContextListType
{
	// No 'Invalid' identifier here
	ContextListRunning = 0,
	ContextListSuspended,
	ContextListZombie,
	ContextListFinished,
	ContextListAlloc,

	// Number of context lists
	ContextListCount
};

/// Category of operand 2
enum ContextOp2Catecory
{
	ContextOp2CatecoryImmd = 0,
	ContextOp2CatecoryReg
};

/// ARM Context
class Context
{
	// Emulator it belongs to
	Emulator *emulator;

	// Process ID
	int pid;

	// Virtual memory address space index
	int address_space_index;

	// Context state, expressed as a bitmap of flags, e.g.,
	// ContextSuspended | ContextFutex
	unsigned state = 0;

	// For segmented memory access in glibc
	unsigned glibc_segment_base = 0;
	unsigned glibc_segment_limit = 0;

	// Host thread that suspends and then schedules call to Emu::ProcessEvents()
	// The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	// is launched for this context (by caller).
	// It is clear when the context finished (by the host thread).
	// It should be accessed safely by locking the emulator mutex
	pthread_t host_thread_suspend;  // Thread
	bool host_thread_suspend_active = false;  // Thread-spawned flag

	// Host thread that lets time elapse and schedules call to
	// emu->ProcessEvents()
	pthread_t host_thread_timer;
	int host_thread_timer_active = false;
	long long host_thread_timer_wakeup;

	// Scheduler
	int sched_policy;
	int sched_priority;

	// Context memory. This object can be shared by multiple contexts, so it
	// is declared as a shared pointer. The last freed context pointing to
	// this memory object will be the one automatically freeing it.
	std::shared_ptr<mem::Memory> memory;

	// Speculative memory. Its initialization is deferred to be able to link
	// it with the actual memory, known only at context creation.
	std::unique_ptr<mem::SpecMem> spec_mem;

	// Register file. Each context has its own copy always.
	Regs regs;

	// Current emulated instruction
	Instruction inst;

	// File descriptor table, shared by contexts
	std::shared_ptr<comm::FileTable> file_table;

	// Call stack
	std::unique_ptr<comm::CallStack> call_stack;

	// Address of last instruction executed
	unsigned last_ip;

	// Address of next instruction to be emulated
	unsigned target_ip;

  	// Address of currently emulated instruction
	unsigned current_ip;

	// For checking if the instruction is in IF-THEN Block
	unsigned int iteq_inst_num;
	unsigned int iteq_block_flag;

	// The type of the current instruction ARM/Thumb16/Thumb32
	ContextInstType inst_type;

	// Set instruction type
	void setInstType(ContextInstType type) { inst_type = type; }

	// Get instruction type
	ContextInstType getInstType() { return inst_type; }

	// symbol list used for getting the ARM operating mode
	std::vector<ELFReader::Symbol *> thumb_symbol_list;

	// Fault Management
	unsigned int fault_addr;
	int fault_value;

	// Parent context
	Context *parent = nullptr;

	// Context group initiator. There is only one group parent (if not null)
	// with many group children, no tree organization.
	Context *group_parent = nullptr;

	// Signal to send parent when finished
	int exit_signal = 0;

	// For zombie contexts
	int exit_code = 0;

	unsigned int clear_child_tid = 0;

	// robust futex list
	unsigned int robust_list_head = 0;

	// Virtual address of the memory access performed by the last emulated
	// instruction.
	unsigned effective_address;

	// Update the context state, updating also the presence on the context
	// in the various context lists in the emulator.
	void UpdateState(unsigned state);

	// Virtual files
	std::string OpenProcSelfMaps();

	// Host thread function
	void HostThreadSuspend();
	static void *HostThreadSuspend(void *data) {
		((Context *) data)->HostThreadSuspend();
		return nullptr;
	}

	// Cancel host thread
	void HostThreadSuspendCancel();
	void HostThreadSuspendCancelUnsafe();

	// Callbacks for suspended contexts
	typedef bool (Context::*CanWakeupFn)();
	typedef void (Context::*WakeupFn)();

	// Stored callbacks for functions used to the wakeup mechanism of
	// suspended contexts. Variable 'wakeup_state' contains the state
	// or states that will be set when suspended and cleared when
	// waken up
	CanWakeupFn can_wakeup_fn;
	WakeupFn wakeup_fn;
	ContextState wakeup_state;

	// Suspend a context, using callbacks 'can_wakeup_fn' and 'wakeup_fn'
	// to check whether the context can wakeup and to wake it up,
	// respectively. Argument 'wakeup_state' specified a temporary
	// state added to the context when suspended, and removed when
	// waken up.
	void Suspend(CanWakeupFn can_wakeup_fn, WakeupFn wakeup_fn,
			ContextState wakeup_state);

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

	// Load ELF sections from binary
	void LoadELFSections(ELFReader::File *binary);

	// Load program headers
	void LoadProgramHeaders();

	// Load ELF binary, as already decoded in 'loader.binary'
	void LoadBinary();

	// Load content of stack
	void LoadStack();

	// Load entry of the auxiliary vector
	void LoadAVEntry(unsigned &sp, unsigned type, unsigned value);

	// Load auxiliary vector and return its size in bytes
	unsigned LoadAV(unsigned where);

	// Load dynamic linker
	void LoadInterp();

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

	///////////////////////////////////////////////////////////////////////
	//
	// Functions and fields related with the emulation of execute instructions,
	// implemented in Machine.cc.
	//
	///////////////////////////////////////////////////////////////////////

	// Prototype of a member function of class Context devoted to the
	// execution of ISA instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef void (Context::*ExecuteInstFn)();

	// Instruction emulation functions. Each entry of Inst.def will be
	// expanded into a function prototype. For example, entry
	// 	DEFINST(AND_reg, "and%cond %rd, %rn, %op2", DprReg, 0x00, 0x0)
	// is expanded to
	//	void ExecuteInst_AND_reg();
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) void ExecuteInst_##_name();
#include <arch/arm/disassembler/Instruction.def>
#undef DEFINST

	// Table of functions
	static ExecuteInstFn execute_inst_fn[Instruction::OpcodeCount];

	// Prototype of a member function of class Context devoted to the
	// execution of ISA thumb16 instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef void (Context::*ExecuteInstThumb16Fn)();

	// Instruction emulation functions. Each entry of Inst.def will be
	// expanded into a function prototype. For example, entry
	// 	DEFINST(LSL_imm, "lsls %rd, %rm, %immd5", MovshiftReg,
	//		0x0, 0x0, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff)
	// is expanded to
	//	void ExecuteInstThumb16_LSL_imm();
#define DEFINST(_name, _fmt_str, _cat, _op1, _op2, _op3, _op4, _op5, _op6) \
	void ExecuteInstThumb16_##_name();
#include <arch/arm/disassembler/InstructionThumb.def>
#undef DEFINST

	// Table of functions
	static ExecuteInstThumb16Fn execute_inst_thumb16_fn[Instruction::Thumb16OpcodeCount];

	// Prototype of a member function of class Context devoted to the
	// execution of ISA thumb 32 instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef void (Context::*ExecuteInstThumb32Fn)();

	// Instruction emulation functions. Each entry of Inst.def will be
	// expanded into a function prototype. For example, entry
	// 	DEFINST(STREX, "strex %rt2, %rt, [%rn  %immd8]", LdStDouble,
	//		0x1,0x0,0x0,0x1,0x04,0xffffffff,0xffffffff,0xffffffff)
	// is expanded to
	//	void ExecuteInstThumb32_STREX();

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	void ExecuteInstThumb32_##_name();
#include <arch/arm/disassembler/InstructionThumb32.def>
#undef DEFINST

	// Table of functions
	static ExecuteInstFn execute_inst_thumb32_fn[Instruction::Thumb32OpcodeCount];

	// Perform rotation right operation
	unsigned int IsaRotr(unsigned int value, int shift);

	// Get the operand 2 value
	int IsaGetOp2(unsigned int op2, ContextOp2Catecory cat);

	// FIXME
	int IsaOp2Carry(unsigned int op2, ContextOp2Catecory cat);

	// Get the address in ARM mode 2
	unsigned int IsaGetAddrAmode2();

	// Get the address in ARM mode 3 with Imm
	unsigned int IsaGetAddrAmode3Imm();

	// Store register
	void IsaRegStore(unsigned int reg_no, int value);

	// Store register in safe mode
	void IsaRegStoreSafe(unsigned int reg_no, unsigned int value);

	// Load register
	void IsaRegLoad(unsigned int reg_no, int &value);

	// Branch operation
	void IsaBranch();

	// Check the condition
	bool IsaCheckCond();

	// Store operation
	void IsaAmode4sStr();

	// Load operation
	void IsaAmode4sLd();

	// Print CPSR
	void IsaCpsrPrint();

	// Return CPSR value
	unsigned int IsaRetCpsrVal();

	// Set CPSR value
	void IsaSetCpsrVal(unsigned int op2);

	// Substract
	void IsaSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3);

	// Substract rev
	void IsaSubtractRev(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3);

	// Add
	void IsaAdd(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3);

	// Multiply
	void IsaMultiply();

	// Multiply with setting condition flag
	void IsaMultiplySettingCF();

	// Multiply long
	void IsaSignedMultiplyLong();

	// FIXME
	unsigned int IsaBitCount(unsigned int ip_num);

	// system call
	void IsaSyscall();

	// FIXME
	bool IsaInvalidAddrStr(unsigned int addr, int value);
	bool IsaInvalidAddrLdr(unsigned int addr, unsigned int *value);

	// Thumb add
	void IsaThumbAdd(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set);

	// Thumb substract
	void IsaThumbSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set);

	// Thumb substrac rev
	void IsaThumbRevSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set);

	// FIXME
	unsigned int IsaThumb32Immd12(Instruction::Thumb32Category cat);

	// Thumb32 branch link
	void IsaThumb32BranchLink();

	// Thumb32 Branch
	void IsaThumb32Branch();

	// Thumb check condition
	unsigned int IsaThumbCheckCond(unsigned int cond);

	// FIXME
	void IsaThumbIteq();

	// Thumb extend immdiate
	unsigned int IsaThumbImmdExtend(unsigned int immd);

	///////////////////////////////////////////////////////////////////////
	//
	// Functions and fields related with the emulation of system calls,
	// implemented in ContextSyscall.cc.
	//
	///////////////////////////////////////////////////////////////////////

	// Emulate a system call
	void ExecuteSyscall();

	// Enumeration with all system call codes. Each entry of
	// ContextSyscall.def will be expanded into a code. For example, entry
	//	DEFSYSCALL(exit, 1)
	// will produce code
	//	SyscallCode_exit
	// There is a last element 'SyscallCodeCount' that will be one unit
	// higher than the highest system call code found in
	// ContextSyscall.def
	enum
	{
#define DEFSYSCALL(name, code) SyscallCode_##name = code,
#include "ContextSyscall.def"
#undef DEFSYSCALL
		SyscallCodeCount
	};

	// System call emulation functions. Each entry of ContextSyscall.def
	// will be expanded into a function prototype. For example, entry
	//	DEFSYSCALL(exit, 1)
	// is expanded to
	//	void ExecuteSyscall_exit();
#define DEFSYSCALL(name, code) int ExecuteSyscall_##name();
#include "ContextSyscall.def"
#undef DEFSYSCALL

	// System call names
	static const char *syscall_name[SyscallCodeCount + 1];

	// Prototype of a member function of class Context devoted to the
	// execution of System Calls. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef int (Context::*ExecuteSyscallFn)();

	// Table of system call execution functions
	static const ExecuteSyscallFn execute_syscall_fn[SyscallCodeCount + 1];

	// System call 'nanosleep'
	long long syscall_nanosleep_wakeup_time;
	void SyscallNanosleepWakeup();
	bool SyscallNanosleepCanWakeup();

	// System call 'read'
	int syscall_read_fd;
	void SyscallReadWakeup();
	bool SyscallReadCanWakeup();

	// System call 'write'
	int syscall_write_fd;
	void SyscallWriteWakeup();
	bool SyscallWriteCanWakeup();

	// System call 'poll'
	int syscall_poll_time;
	int syscall_poll_fd;
	int syscall_poll_events;
	void SyscallPollWakeup();
	bool SyscallPollCanWakeup();

	// Auxiliary system call functions
	int SyscallMmapAux(unsigned int addr, unsigned int len, int prot,
			int flags, int guest_fd, int offset);
	comm::FileDescriptor *SyscallOpenVirtualFile(const std::string &path,
			int flags, int mode);
	comm::FileDescriptor *SyscallOpenVirtualDevice(const std::string &path,
			int flags, int mode);

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

	/// Constructor
	Context();

	/// Destructor
	~Context();

	/// Load a program on the context. The meaning of each argument is
	/// identical to the prototype of comm::Emulator::Load().
	void Load(const std::vector<std::string> &args,
			const std::vector<std::string> &env,
			const std::string &cwd,
			const std::string &stdin_file_name,
			const std::string &stdout_file_name);

	/// Given a file name, return its full path based on the current working
	/// directory for the context.
	std::string getFullPath(const std::string &path)
	{
		return misc::getFullPath(path, loader->cwd);
	}

	/// Check whether a context suspended with a call to Suspend() is ready
	/// to wake up, by invoking the 'can_wakeup' callback.
	bool CanWakeup();

	/// Wake up a context in suspended state that went to sleep with a call
	/// to Suspend(). The 'wakeup_fn' callback function is invoked, and the
	/// wakeup data is internally freed by reseting the smart pointer.
	void Wakeup();

	/// Check whether there is any pending unblocked signal in the context,
	/// and invoke the corresponding signal handler.
	void CheckSignalHandler();


	// Check any pending signal, and run the corresponding signal handler by
	// considering that the signal interrupted a system call
	// (\c syscall_intr). This has the following implication on the return
	// address from the signal handler:
	//   -If flag \c SA_RESTART is set for the handler, the return address
	//    is the system call itself, which must be repeated.
	//   -If flag \c SA_RESTART is not set, the return address is the
	//    instruction next to the system call, and register '//FIXME: 2(?)' is set to
	//    \c -EINTR.
	void CheckSignalHandlerIntr();

	/// Run one instruction for the context at the position pointed to by
	/// register program counter.
	void Execute();
	void ExecuteInst();

	/// Finish a context group. This call does a subset of action of the
	/// Finish() call, but for all parent and child contexts sharing a
	/// memory map.
	void FinishGroup(int status);

	/// Finish a context. If the context has no parent, its state will be
	/// set to ContextFinished. If it has, its state is set to
	/// ContextZombie, waiting for a call to 'waitpid'. The children of the
	/// finished context will set their 'parent' attribute to null. The
	/// zombie children will be finished.
	void Finish(int status);

	/// Return \c true if flag \a state is part of the context state
	bool getState(ContextState state) const { return this->state & state; }

	/// Set flag \a state in the context state
	void setState(ContextState state) { UpdateState(this->state | state); }

	/// Clear flag \a state in the context state
	void clearState(ContextState state) { UpdateState(this->state
			& ~state); }

	/// Return the context mode
	ContextMode OperateMode(unsigned int addr);

	/// Check Kuser helper
	unsigned int CheckKuserHelper();

	/// Check whether the operating mode is Thumb 32
	bool IsThumb32(const char *inst_ptr);
};

}  // namespace ARM

#endif // ARCH_ARM_EMU_CONTEXT_H
