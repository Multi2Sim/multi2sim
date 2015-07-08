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

#ifndef ARCH_X86_EMULATOR_CONTEXT_H
#define ARCH_X86_EMULATOR_CONTEXT_H

#include <deque>
#include <memory>

#include <arch/common/CallStack.h>
#include <arch/common/Context.h>
#include <arch/common/FileTable.h>
#include <lib/cpp/Debug.h>
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/String.h>
#include <memory/Memory.h>
#include <memory/MMU.h>
#include <memory/SpecMem.h>

#include "Regs.h"
#include "Signal.h"
#include "Uinst.h"


extern int x86_cpu_num_threads;
extern int x86_cpu_num_cores;

namespace x86
{

// Forward declarations
class Context;
class Emulator;


// Assembly code used before and after instruction emulation when the host flags
// are affected by the guest code
#define __X86_ISA_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (Context::host_flags));

#define __X86_ISA_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	: "=m" (Context::host_flags));

#define __X86_CONTEXT_SAVE_FLAGS__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (Context::host_flags));

#define __X86_CONTEXT_RESTORE_FLAGS__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	: "=m" (Context::host_flags));


// Assembly code used before and after emulation of floating-point operations
#define __X86_CONTEXT_FP_BEGIN__ { \
	unsigned short fpu_ctrl = regs.getFpuCtrl(); \
	asm volatile ( \
		"pushf\n\t" \
		"pop %0\n\t" \
		"fnstenv %1\n\t" \
		"fnclex\n\t" \
		"fldcw %2\n\t" \
		: "=m" (Context::host_flags), "=m" (*Context::host_fpenv) \
		: "m" (fpu_ctrl) \
	); \
}

#define __X86_CONTEXT_FP_END__ { \
	unsigned short fpu_ctrl; \
	asm volatile ( \
		"push %0\n\t" \
		"popf\n\t" \
		"fnstcw %1\n\t" \
		"fldenv %2\n\t" \
		: "=m" (Context::host_flags), "=m" (fpu_ctrl) \
		: "m" (*Context::host_fpenv) \
	); \
	regs.setFpuCtrl(fpu_ctrl); \
}




/// x86 Context
class Context : public comm::Context
{
public:

	/// Context list identifiers
	enum ListType
	{
		// No 'Invalid' identifier here
		ListRunning,
		ListSuspended,
		ListZombie,
		ListFinished,

		// Number of context lists
		ListCount
	};

	/// Context states
	enum State
	{
		StateInvalid      = 0x00000,
		StateRunning      = 0x00001,  // it is able to run instructions
		StateSpecMode     = 0x00002,  // executing in speculative mode
		StateSuspended    = 0x00004,  // suspended in a system call
		StateFinished     = 0x00008,  // no more inst to execute
		StateExclusive    = 0x00010,  // executing in excl mode
		StateLocked       = 0x00020,  // another context is running in excl mode
		StateHandler      = 0x00040,  // executing a signal handler
		StateSigsuspend   = 0x00080,  // suspended after syscall 'sigsuspend'
		StateNanosleep    = 0x00100,  // suspended after syscall 'nanosleep'
		StatePoll         = 0x00200,  // 'poll' system call
		StateRead         = 0x00400,  // 'read' system call
		StateWrite        = 0x00800,  // 'write' system call
		StateWaitpid      = 0x01000,  // 'waitpid' system call
		StateZombie       = 0x02000,  // zombie context
		StateFutex        = 0x04000,  // suspended in a futex
		StateAlloc        = 0x08000,  // allocated to a core/thread
		StateCallback     = 0x10000,  // suspended after syscall with callback
		StateMapped       = 0x20000   // mapped to a core/thread
	};

	/// String map for State
	static const misc::StringMap StateMap;

private:

	// Saved host flags during instruction emulation
	static long host_flags;

	// Saved host floating-point environment during instruction emulation
	static unsigned char host_fpenv[28];

	// Emulator that it belongs to
	Emulator *emulator;

	// Context state, expressed as a bitmap of flags, e.g.,
	// ContextSuspended | ContextFutex
	unsigned state = 0;

	// Context memory. This object can be shared by multiple contexts, so it
	// is declared as a shared pointer. The last freed context pointing to
	// this memory object will be the one automatically freeing it.
	std::shared_ptr<mem::Memory> memory;

	// Speculative memory. Its initialization is deferred to be able to link
	// it with the actual memory, known only at context creation.
	std::unique_ptr<mem::SpecMem> spec_mem;

	// Register file. Each context has its own copy always.
	Regs regs;

	// Backup register file that is used for recovery
	Regs backup_regs;

	// File descriptor table, shared by contexts
	std::shared_ptr<comm::FileTable> file_table;

	// Call stack
	std::unique_ptr<comm::CallStack> call_stack;

	// Instruction pointers
	unsigned last_eip;  // Address of last emulated instruction
	unsigned current_eip;  // Address of currently emulated instruction
	unsigned target_eip;  // Target address for branch, even if not taken

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

	// Robust futex list
	unsigned int robust_list_head = 0;

	// Virtual address of the memory access performed by the last emulated
	// instruction.
	unsigned last_effective_address;
	
	// For emulation of string operations
	unsigned int str_op_esi = 0;  // Initial value for register 'esi'
	unsigned int str_op_edi = 0;  // Initial value for register 'edi'
	int str_op_dir = 0;  // Direction: 1 = forward, -1 = backward
	int str_op_count = 0;  // Number of iterations in string operation //

	// Last emulated instruction
	Instruction inst;
	
	// For segmented memory access in glibc
	unsigned glibc_segment_base = 0;
	unsigned glibc_segment_limit = 0;

	// Host thread that suspends and then schedules call to ProcessEvents()
	// The 'host_thread_suspend_active' flag is set when a 'host_thread_suspend' thread
	// is launched for this context (by caller).
	// It is clear when the context finished (by the host thread).
	// It should be accessed safely by locking the emulator mutex
	pthread_t host_thread_suspend;  // Thread
	bool host_thread_suspend_active = false;  // Thread-spawned flag

	// Host thread that lets time elapse and schedules call to
	// ProcessEvents()
	pthread_t host_thread_timer;
	int host_thread_timer_active = false;
	long long host_thread_timer_wakeup;
	



	//
	// Scheduler
	//

	// Cycle when the context was allocated to a node (core/thread),
	long long alloc_cycle;

	// Cycle when the context was evicted to a node (core/thread),
	long long evict_cycle;

	// The context is mapped and allocated, but its eviction is in progress.
	// It will be effectively evicted once the last instruction reaches the
	// commit stage.
	int evict_signal;

	// If context is in state 'mapped', these two variables represent the
	// node (core/thread) associated with the context.
	int core_index;
	int thread_index;

	int sched_policy = SCHED_RR;
	int sched_priority = 1;

	// Variables used to wake up suspended contexts.
	unsigned wakeup_futex;  // Address of futex where context is suspended
	unsigned wakeup_futex_bitset;  // Bit mask for selective futex wakeup
	long long wakeup_futex_sleep;  // Assignment from futex_sleep_count


	// Update the context state, updating also the presence on the context
	// in the various context lists in the emulator.
	void UpdateState(unsigned state);

	// Futex-related calls
	int FutexWake(unsigned futex, unsigned count, unsigned bitset);
	void ExitRobustList();

	// Virtual files
	std::string OpenProcSelfMaps();
	std::string OpenProcCPUInfo();

	// Virtual devices
	std::string OpenDevM2SSICL();

	// Dump debug information about a call instruction
	void DebugCallInst();

	// Host thread function
	void HostThreadSuspend();
	static void *HostThreadSuspend(void *data)
	{
		((Context *) data)->HostThreadSuspend();
		return nullptr;
	}
	
	// Cancel host thread
	void HostThreadSuspendCancel();
	void HostThreadSuspendCancelUnsafe();

	// Cancel timer thread
	void HostThreadTimerCancel();
	void HostThreadTimerCancelUnsafe();

	// Callbacks for suspended contexts
	typedef bool (Context::*CanWakeupFn)();
	typedef void (Context::*WakeupFn)();

	// Stored callbacks for functions used to the wakeup mechanism of
	// suspended contexts. Variable 'wakeup_state' contains the state
	// or states that will be set when suspended and cleared when
	// waken up
	CanWakeupFn can_wakeup_fn = nullptr;
	WakeupFn wakeup_fn = nullptr;
	State wakeup_state;

	// Suspend a context, using callbacks 'can_wakeup_fn' and 'wakeup_fn'
	// to check whether the context can wakeup and to wake it up,
	// respectively. Argument 'wakeup_state' specified a temporary
	// state added to the context when suspended, and removed when
	// waken up.
	void Suspend(CanWakeupFn can_wakeup_fn, WakeupFn wakeup_fn,
			State wakeup_state);
	



	//
	// Program loading (ContextLoader.cc)
	//

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



	
	//
	// Signal handling (ContextSignal.cc)
	//

	// Table of signal handlers, possibly shared by multiple contexts
	std::shared_ptr<SignalHandlerTable> signal_handler_table;

	// Table of signal masks, each context has its own
	SignalMaskTable signal_mask_table;

	// Run a signal handler for signal \a sig. The value of \a sig must be
	// between 1 and 64.
	void RunSignalHandler(int sig);

	// Return from a signal handler
	void ReturnFromSignalHandler();

	
	
	
	//
	// Micro-instructions (ContextUinst.cc)
	//

	// True if we are in timing simulation and need to activate
	// micro-instructions.
	bool uinst_active = false;

	// True if the effective address computation for the current macro-
	// instruction has already been emitted. This flag is used to avoid
	// multiple address computations for macro-instruction that implicitly
	// load, operate, and store.
	bool uinst_effaddr_emitted = false;

	// List of micro-instructions produced during the emulation of the last
	// x86 macro-instruction.
	std::deque<std::shared_ptr<Uinst>> uinsts;

	// Clear the list of micro-instructions
	void ClearUinsts()
	{
		uinsts.clear();
		uinst_effaddr_emitted = false;
	}

	// If dependence at position \a index is a memory operand, return its
	// associated standard dependence in \a std_dep and its size as the
	// return value of the function. The function returns 0 if the
	// dependence is not a memory dependence.
	int getMemoryDepSize(Uinst *uinst, int index, Uinst::Dep &std_dep);

	// Emit the effective address computation micro-instructions. Argument
	// \a index is the dependency index, a value between 0 and
	// UinstMaxDeps - 1
	void EmitUinstEffectiveAddress(Uinst *uinst, int index);

	// Parse regular dependence, given as a global dependence index. The
	// value in \a index must be between 0 and UinstMaxDeps -1
	void ParseUinstDep(Uinst *uinst, int index);

	// Parse input dependences. Argument \a index is a value between 0 and
	// UinstMaxIDeps - 1
	void ParseUinstIDep(Uinst *uinst, int index);
	
	// Parse output dependence. Argument \a index is a value between 0 and
	// UinstMaxODeps - 1
	void ParseUinstODep(Uinst *uinst, int index);




	//
	// Instruction emulation (ContextIsaXXX.cc)
	//

	// Prototype of a member function of class Context devoted to the
	// execution of ISA instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef void (Context::*ExecuteInstFn)();

	// Instruction emulation functions. Each entry of Inst.def will be
	// expanded into a function prototype. For example, entry
	// 	DEFINST(adc_al_imm8, 0x14, SKIP, SKIP, SKIP, IB, 0)
	// is expanded to
	//	void ExecuteInst_adc_al_imm8();
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) void ExecuteInst_##name();
#include <arch/x86/disassembler/Instruction.def>
#undef DEFINST

	// Table of functions
	static ExecuteInstFn execute_inst_fn[Instruction::OpcodeCount];

	// Safe memory accesses, based on the current speculative mode
	void MemoryRead(unsigned int address, int size, void *buffer);
	void MemoryWrite(unsigned int address, int size, void *buffer);
	
	// These are some functions created automatically by the macros in
	// ContextIsaStd.cc, but corresponding to non-existing instructions.
	// Since they're not declared in Inst.def, they must be explicitly
	// declared here.
	void ExecuteInst_test_rm16_imm8();
	void ExecuteInst_test_rm32_imm8();
	void ExecuteInst_test_r8_rm8();
	void ExecuteInst_test_r16_rm16();
	void ExecuteInst_test_r32_rm32();

	// Functions used in ContextIsaStr.cc to generate micro-instructions
	// for string operations
	void newUinst_cmpsb(unsigned int esi, unsigned int edi);
	void newUinst_cmpsd(unsigned int esi, unsigned int edi);
	void newUinst_insb(unsigned int esi, unsigned int edi);
	void newUinst_insd(unsigned int esi, unsigned int edi);
	void newUinst_lodsb(unsigned int esi, unsigned int edi);
	void newUinst_lodsd(unsigned int esi, unsigned int edi);
	void newUinst_movsb(unsigned int esi, unsigned int edi);
	void newUinst_movsw(unsigned int esi, unsigned int edi);
	void newUinst_movsd(unsigned int esi, unsigned int edi);
	void newUinst_outsb(unsigned int esi, unsigned int edi);
	void newUinst_outsd(unsigned int esi, unsigned int edi);
	void newUinst_scasb(unsigned int esi, unsigned int edi);
	void newUinst_scasd(unsigned int esi, unsigned int edi);
	void newUinst_stosb(unsigned int esi, unsigned int edi);
	void newUinst_stosd(unsigned int esi, unsigned int edi);

	// Functions used in ContextIsaStr.cc for execution of string
	// instructions
	void ExecuteStringInst_cmpsb();
	void ExecuteStringInst_cmpsd();
	void ExecuteStringInst_insb();
	void ExecuteStringInst_insd();
	void ExecuteStringInst_lodsb();
	void ExecuteStringInst_lodsd();
	void ExecuteStringInst_movsb();
	void ExecuteStringInst_movsw();
	void ExecuteStringInst_movsd();
	void ExecuteStringInst_outsb();
	void ExecuteStringInst_outsd();
	void ExecuteStringInst_scasb();
	void ExecuteStringInst_scasd();
	void ExecuteStringInst_stosb();
	void ExecuteStringInst_stosd();

	// Additional instructions
	void ExecuteInst_fist_m64();

	// Reset or update iteration counters for string instructions with
	// 'repXXX' prefixes.
	void StartRepInst();

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
	unsigned char LoadR8() { return regs.Read(inst.getModRmReg() + Instruction::RegAl); }
	unsigned short LoadR16() { return regs.Read(inst.getModRmReg() + Instruction::RegAx); }
	unsigned int LoadR32() { return regs.Read(inst.getModRmReg() + Instruction::RegEax); }
	unsigned short LoadSReg() { return regs.Read(inst.getModRmReg() + Instruction::RegEs); }

	// Store value into register
	void StoreR8(unsigned char value) { regs.Write(inst.getModRmReg() + Instruction::RegAl, value); }
	void StoreR16(unsigned short value) { regs.Write(inst.getModRmReg() + Instruction::RegAx, value); }
	void StoreR32(unsigned int value) { regs.Write(inst.getModRmReg() + Instruction::RegEax, value); }
	void StoreSReg(unsigned short value) { regs.Write(inst.getModRmReg() + Instruction::RegEs, value); }

	// Load value from index register
	unsigned char LoadIR8() { return regs.Read(inst.getOpIndex() + Instruction::RegAl); }
	unsigned short LoadIR16() { return regs.Read(inst.getOpIndex() + Instruction::RegAx); }
	unsigned int LoadIR32() { return regs.Read(inst.getOpIndex() + Instruction::RegEax); }

	// Store value into index register
	void StoreIR8(unsigned char value) { regs.Write(inst.getOpIndex() + Instruction::RegAl, value); }
	void StoreIR16(unsigned short value) { regs.Write(inst.getOpIndex() + Instruction::RegAx, value); }
	void StoreIR32(unsigned int value) { regs.Write(inst.getOpIndex() + Instruction::RegEax, value); }

	void LoadFpu(int index, unsigned char *value);
	void StoreFpu(int index, unsigned char *value);
	void PopFpu(unsigned char *value);
	void PushFpu(unsigned char *value);

	float LoadFloat();
	double LoadDouble();
	void LoadExtended(unsigned char *value);
	void StoreFloat(float value);
	void StoreDouble(double value);
	void StoreExtended(unsigned char *value);

	// Store the code bits (14, 10, 9, and 8) of the FPU state word into the
	// 'code' register
	void StoreFpuCode(unsigned short status);

	// Read the state register, by building it from the 'top' and 'code'
	// fields
	unsigned short LoadFpuStatus();


	// Return the final address obtained from binding \a address inside
	// the corresponding segment. The segment boundaries are checked.
	unsigned getLinearAddress(unsigned offset);

	// Return the effective address obtained from the 'SIB' and 'disp'
	// fields, and store it in 'effective_address' field for the future
	unsigned getEffectiveAddress();

	// Return a memory address contained in the immediate value
	unsigned getMoffsAddress();


	// Import the value into XMM register
	// \param value
	// 		Reference to the value to be copied
	void StoreXMM(const XMMValue &value);

	// Export the value in XMM register to memory
	// \param value
	// 		Referemce to the XMMValue object to receice the value
	void LoadXMM(XMMValue &value);

	// Store lower 32/64/128 bits from value into XMM register or memory
	void StoreXMMM32(const XMMValue &value);
	void StoreXMMM64(const XMMValue &value);
	void StoreXMMM128(const XMMValue &value);

	// Load the lower 32/64/128 bits into lower bits of value
	void LoadXMMM32(XMMValue &value);
	void LoadXMMM64(XMMValue &value);
	void LoadXMMM128(XMMValue &value);


	///////////////////////////////////////////////////////////////////////
	//
	// Functions and fields related with the emulation of system calls,
	// implemented in ContextSyscall.cc.
	//
	///////////////////////////////////////////////////////////////////////

	// Emulate a system call
	void ExecuteSyscall();

	// Enumeration with all system call codes. Each entry of syscall.dat
	// will be expanded into a code. For example, entry
	//	DEFSYSCALL(exit, 1)
	// will produce code
	//	SyscallCode_exit
	// There is a last element 'SyscallCodeCount' that will be one unit
	// higher than the highest system call code found in syscall.dat.
	enum
	{
#define DEFSYSCALL(name, code) SyscallCode_##name = code,
#include "syscall.dat"
#undef DEFSYSCALL
		SyscallCodeCount
	};

	// System call emulation functions. Each entry of syscall.dat will be
	// expanded into a function prototype. For example, entry
	//	DEFSYSCALL(exit, 1)
	// is expanded to
	//	void ExecuteSyscall_exit();
#define DEFSYSCALL(name, code) int ExecuteSyscall_##name();
#include "syscall.dat"
#undef DEFSYSCALL

	// System call names
	static const char *syscall_name[SyscallCodeCount + 1];

	// Prototype of a member function of class Context devoted to the
	// execution of ISA instructions. The emulator has a table indexed by an
	// instruction identifier that points to all instruction emulation
	// functions.
	typedef int (Context::*ExecuteSyscallFn)();

	// Table of system call execution functions
	static const ExecuteSyscallFn execute_syscall_fn[SyscallCodeCount + 1];

	// Auxiliary system call functions
	int SyscallMmapAux(unsigned int addr, unsigned int len, int prot,
			int flags, int guest_fd, int offset);
	comm::FileDescriptor *SyscallOpenVirtualFile(const std::string &path,
			int flags, int mode);
	comm::FileDescriptor *SyscallOpenVirtualDevice(const std::string &path,
			int flags, int mode);

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

	// System call 'sigsuspend'
	void SyscallSigsuspendWakeup();
	bool SyscallSigsuspendCanWakeup();

	// System call 'waitpid'
	int syscall_waitpid_pid;
	void SyscallWaitpidWakeup();
	bool SyscallWaitpidCanWakeup();

public:
	
	/// Position of the context in the main context list. This field is
	/// managed by the emulator. When a context is removed from the main
	/// context list, it is automatically freed.
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;

	/// Flag indicating whether this context is present in a certain context
	/// list of the emulator. This field is exclusively managed by the
	/// emulator.
	bool context_list_present[ListCount] = { };

	/// Position of the context in a certain context list. This field is
	/// exclusively managed by the emulator.
	std::list<Context *>::iterator context_list_iter[ListCount];

	/// Create a context from a command line. To safely create a context,
	/// function NewContext() should be used instead. After the
	/// creation of a context, its basic data structures are initialized
	/// with Load(), Clone(), or Fork().
	Context();

	/// Destructor
	~Context();

	/// Load a program on the context. The meaning of each argument is
	/// identical to the prototype of comm::Emulator::Load().
	void Load(const std::vector<std::string> &args,
			const std::vector<std::string> &env = { },
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "");

	/// Initialize the context by cloning the main data structures from a
	/// parent context.
	void Clone(Context *parent);

	/// Initialize the context by forking a parent context.
	void Fork(Context *parent);

	/// Given a file name, return its full path based on the current working
	/// directory for the context.
	std::string getFullPath(const std::string &path)
	{
		return misc::getFullPath(path, loader->cwd);
	}

	/// Look for zombie child. If 'pid' is -1, the first finished child in
	/// the zombie contexts list is return. Otherwise, 'pid' is the pid of
	/// the child process. If no child has finished, return nullptr.
	Context *getZombie(int pid);

	/// Return \c true if flag \a state is part of the context state
	bool getState(State state) const { return this->state & state; }

	/// Set flag \a state in the context state
	void setState(State state) { UpdateState(this->state | state); }

	/// Clear flag \a state in the context state
	void clearState(State state) { UpdateState(this->state & ~state); }

	// Finish a context. If the context has no parent, its state will be
	// set to ContextFinished. If it has, its state is set to
	// ContextZombie, waiting for a call to 'waitpid'. The children of the
	// finished context will set their 'parent' attribute to null. The
	// zombie children will be finished.
	void Finish(int status);

	// Finish a context group. This call does a subset of action of the
	// Finish() call, but for all parent and child contexts sharing a
	// memory map.
	void FinishGroup(int status);
	
	/// Suspend the context. This is a virtual function mandated by the
	/// parent class comm::Context.
	void Suspend() override;

	/// Check whether a context suspended with a call to Suspend() is ready
	/// to wake up, by invoking the 'can_wakeup' callback.
	bool CanWakeup();

	/// Wake up the context. This is a virtual function mandated by the
	/// parent class comm::Context. The 'wakeup_fn' callback function is
	/// invoked, and the wakeup data is internally freed by reseting the
	/// smart pointer.
	void Wakeup() override;

	/// Return whether the context is suspended. This is a virtual function
	/// mandated by the parent class comm::Context.
	bool isSuspended() override;

	// Check whether there is any pending unblocked signal in the context,
	// and invoke the corresponding signal handler.
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

	/// Run one instruction for the context at the position pointed to by
	/// register \c eip.
	void Execute();

	/// Return a reference of the register file
	Regs &getRegs() { return regs; }

	/// Return the last emulated instruction.
	Instruction *getInstruction() { return &inst; }

	/// Return a constant reference of the memory
	mem::Memory &getMem()
	{
		assert(memory.get());
		return *memory;
	}

	/// Return a pointer of the memory. This is used when an hsa environment
	/// is created and the hsa emulator uses the host programs memory object
	std::shared_ptr<mem::Memory> __getMemSharedPtr()
	{
		assert(memory.get());
		return memory;
	}

	/// Force a new 'eip' value for the context. The forced value should be
	/// the same as the current 'eip' under normal circumstances. If it is
	/// not, speculative execution starts, which will end on the next call
	/// to 'recover' function.
	void setEip(unsigned int eip);

	/// Recover the context from speculative mode
	void Recover();

	/// Get Target EIP
	int getTargetEip() { return target_eip; }



	//
	// Micro-instructions
	//

	/// Add a new memory micro-instruction to the list only if we're running
	/// in timing simulation mode.
	void newMemoryUinst(Uinst::Opcode opcode,
			unsigned address,
			int size,
			int idep0,
			int idep1,
			int idep2,
			int odep0,
			int odep1,
			int odep2,
			int odep3);

	/// Add a new micro-instruction to the list only if we're running
	/// in timing simulation mode, omitting the \a address and \a size
	/// arguments.
	void newUinst(Uinst::Opcode opcode,
			int idep0,
			int idep1,
			int idep2,
			int odep0,
			int odep1,
			int odep2,
			int odep3)
	{
		newMemoryUinst(opcode,
				0,
				0,
				idep0,
				idep1,
				idep2,
				odep0,
				odep1,
				odep2,
				odep3);
	}

	/// Return the number of micro-instructions produced by the emulation of
	/// the last x86 instruction with an invocation to Context::Execute().
	int getNumUinsts() const { return uinsts.size(); }

	/// Extract the micro-instruction at the head of the micro-instruction
	/// list, and return an ownership reference to it. If the result is not
	/// immediately captured by a shared pointer, the micro-instruction will
	/// be freed.
	std::shared_ptr<Uinst> ExtractUinst()
	{
		assert(uinsts.size() > 0);
		std::shared_ptr<Uinst> uinst = uinsts.front();
		uinsts.pop_front();
		return uinst;
	}



	
	//
	// Public fields
	//

	// Memory management unit, which can be shared by multiple contexts.
	// The MMU is used for timing simulation purposes.
	std::shared_ptr<mem::MMU> mmu;

	// Address space within the mmu;
	mem::MMU::Space *mmu_space = nullptr;
};

}  // namespace x86

#endif

