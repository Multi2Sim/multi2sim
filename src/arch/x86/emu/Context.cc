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

#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <arch/common/Arch.h>
#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"

// Variable used in function Context::loadProgram()
extern char **environ;

	
namespace x86
{


misc::StringMap context_state_map =
{
	{ "running",      ContextRunning },
	{ "specmode",     ContextSpecMode },
	{ "suspended",    ContextSuspended },
	{ "finished",     ContextFinished },
	{ "exclusive",    ContextExclusive },
	{ "locked",       ContextLocked },
	{ "handler",      ContextHandler },
	{ "sigsuspend",   ContextSigsuspend },
	{ "nanosleep",    ContextNanosleep },
	{ "poll",         ContextPoll },
	{ "read",         ContextRead },
	{ "write",        ContextWrite },
	{ "waitpid",      ContextWaitpid },
	{ "zombie",       ContextZombie },
	{ "futex",        ContextFutex },
	{ "alloc",        ContextAlloc },
	{ "callback",     ContextCallback },
	{ "mapped",       ContextMapped }
};

long context_host_flags;
unsigned char context_host_fpenv[28];



//
// Private functions
//

void Context::UpdateState(unsigned state)
{
	// If the difference between the old and new state lies in other
	// states other than 'ContextSpecMode', a reschedule is marked. */
	unsigned diff = this->state ^ state;
	if (diff & ~ContextSpecMode)
		emu->setScheduleSignal();
	
	// Update state
	this->state = state;
	if (this->state & ContextFinished)
		this->state = ContextFinished
				| (state & ContextAlloc)
				| (state & ContextMapped);
	if (this->state & ContextZombie)
		this->state = ContextZombie
				| (state & ContextAlloc)
				| (state & ContextMapped);
	if (!(this->state & ContextSuspended) &&
			!(this->state & ContextFinished) &&
			!(this->state & ContextZombie) &&
			!(this->state & ContextLocked))
		this->state |= ContextRunning;
	else
		this->state &= ~ContextRunning;
	
	// Update presence of context in emulator lists depending on its state
	emu->UpdateContextInList(ContextListRunning, this, this->state & ContextRunning);
	emu->UpdateContextInList(ContextListZombie, this, this->state & ContextZombie);
	emu->UpdateContextInList(ContextListFinished, this, this->state & ContextFinished);
	emu->UpdateContextInList(ContextListSuspended, this, this->state & ContextSuspended);

	// Dump new state (ignore ContextSpecMode state, it's too frequent)
	if (Emu::context_debug && (diff & ~ContextSpecMode))
	{
		Emu::context_debug << misc::fmt(
				"inst %lld: context %d changed state to %s\n",
				emu->getInstructions(), pid,
				context_state_map.MapFlags(this->state).c_str());
	}

	// Resume or pause timer depending on whether there are any contexts
	// currently running.
	if (emu->getContextList(ContextListRunning).size())
		emu->StartTimer();
	else
		emu->StopTimer();
}


int Context::FutexWake(unsigned futex, unsigned count, unsigned bitset)
{
	Context *wakeup_context;
	int wakeup_count = 0;

	// Look for threads suspended in this futex
	while (count)
	{
		wakeup_context = nullptr;
		for (Context *context : emu->getContextList(ContextListSuspended))
		{
			if (!context->getState(ContextFutex) || context->wakeup_futex != futex)
				continue;
			if (!(context->wakeup_futex_bitset & bitset))
				continue;
			if (!wakeup_context || context->wakeup_futex_sleep <
					wakeup_context->wakeup_futex_sleep)
				wakeup_context = context;
		}

		if (wakeup_context)
		{
			// Wake up context
			wakeup_context->clearState(ContextFutex);
			wakeup_context->clearState(ContextSuspended);
			emu->syscall_debug << misc::fmt("  futex 0x%x: thread %d woken up\n",
					futex, wakeup_context->pid);
			wakeup_count++;
			count--;

			// Set system call return value
			wakeup_context->regs.setEax(0);
		}
		else
		{
			break;
		}
	}
	return wakeup_count;
}


void Context::ExitRobustList()
{
	// Read the offset from the list head. This is how the structure is
	// represented in the kernel:
	// struct robust_list {
	//      struct robust_list __user *next;
	// }
	//struct robust_list_head {
	//	struct robust_list list;
	//	long futex_offset;
	//	struct robust_list __user *list_op_pending;
	// }
	// See linux/Documentation/robust-futex-ABI.txt for details
	// about robust futex wake up at thread exit.
	//

	unsigned lock_entry = robust_list_head;
	if (!lock_entry)
		return;

	emu->syscall_debug << misc::fmt("ctx %d: processing robust futex list\n",
			pid);
	for (;;)
	{
		unsigned int next, offset, lock_word;
		memory->Read(lock_entry, 4, (char *) &next);
		memory->Read(lock_entry + 4, 4, (char *) &offset);
		memory->Read(lock_entry + offset, 4, (char *) &lock_word);

		emu->syscall_debug << misc::fmt("  lock_entry=0x%x: "
				"offset=%d, lock_word=0x%x\n",
				lock_entry, offset, lock_word);

		// Stop processing list if 'next' points to robust list
		if (!next || next == robust_list_head)
			break;
		lock_entry = next;
	}
}


std::string Context::OpenProcSelfMaps()
{
	// Create temporary file
	int fd;
	FILE *f = NULL;
	char path[256];
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		misc::fatal("ctx_gen_proc_self_maps: cannot create temporary file");

	// Get the first page
	unsigned end = 0;
	for (;;)
	{
		// Get start of next range
		mem::MemoryPage *page = memory->getNextPage(end);
		if (!page)
			break;
		unsigned start = page->tag;
		end = page->tag;
		int perm = page->perm & (mem::MemoryAccessRead |
				mem::MemoryAccessWrite | mem::MemoryAccessExec);

		// Get end of range
		for (;;)
		{
			page = memory->getPage(end + mem::MemoryPageSize);
			if (!page)
				break;
			int page_perm = page->perm & (mem::MemoryAccessRead |
					mem::MemoryAccessWrite | mem::MemoryAccessExec);
			if (page_perm != perm)
				break;
			end += mem::MemoryPageSize;
			perm = page_perm;
		}

		// Dump range
		fprintf(f, "%08x-%08x %c%c%c%c 00000000 00:00\n", start,
				end + mem::MemoryPageSize,
				perm & mem::MemoryAccessRead ? 'r' : '-',
				perm & mem::MemoryAccessWrite ? 'w' : '-',
				perm & mem::MemoryAccessExec ? 'x' : '-',
				'p');
	}

	// Close file
	fclose(f);
	return path;
}

std::string Context::OpenProcCPUInfo()
{
	int node;
	int fd;
	FILE *f = NULL;
	char path[256];
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		misc::fatal("ctx_gen_proc_self_maps: cannot create temporary file");

	for (int i = 0; i < x86_cpu_num_cores; i++)
	{
		for (int j = 0; j < x86_cpu_num_threads; j++)
		{
			node = i * x86_cpu_num_threads + j;
			fprintf(f, "processor : %d\n", node);
			fprintf(f, "vendor_id : Multi2Sim\n");
			fprintf(f, "cpu family : 6\n");
			fprintf(f, "model : 23\n");
			fprintf(f, "model name : Multi2Sim\n");
			fprintf(f, "stepping : 6\n");
			fprintf(f, "microcode : 0x607\n");
			fprintf(f, "cpu MHz : 800.000\n");
			fprintf(f, "cache size : 3072 KB\n");
			fprintf(f, "physical id : 0\n");
			fprintf(f, "siblings : %d\n", x86_cpu_num_cores * x86_cpu_num_threads);
			fprintf(f, "core id : %d\n", i);
			fprintf(f, "cpu cores : %d\n", x86_cpu_num_cores);
			fprintf(f, "apicid : %d\n", node);
			fprintf(f, "initial apicid : %d\n", node);
			fprintf(f, "fpu : yes\n");
			fprintf(f, "fpu_exception : yes\n");
			fprintf(f, "cpuid level : 10\n");
			fprintf(f, "wp : yes\n");
			fprintf(f, "flags : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr "
					"pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse "
					"sse2 ss ht tm pbe syscall nx lm constant_tsc arch_perfmon "
					"pebs bts rep_good nopl aperfmperf pni dtes64 monitor ds_cpl "
					"vmx est tm2 ssse3 cx16 xtpr pdcm sse4_1 lahf_lm ida dtherm "
					"tpr_shadow vnmi flexpriority\n");
			fprintf(f, "bogomips : 4189.40\n");
			fprintf(f, "clflush size : 32\n");
			fprintf(f, "cache_alignment : 32\n");
			fprintf(f, "address sizes : 32 bits physical, 32 bits virtual\n");
			fprintf(f, "power management :\n");
			fprintf(f, "\n");
		}
	}

	// Close file 
	fclose(f);
	return path;
}

//
// Public functions
//

Context::Context()
{
	// Save emulator instance
	emu = Emu::getInstance();

	// Initialize
	state = 0;
	glibc_segment_base = 0;
	glibc_segment_limit = 0;
	pid = emu->getPid();
	parent = nullptr;
	group_parent = nullptr;
	exit_signal = 0;
	exit_code = 0;
	clear_child_tid = 0;
	robust_list_head = 0;
	host_thread_suspend_active = false;
	host_thread_timer_active = false;
	sched_policy = SCHED_RR;
	sched_priority = 1;  // Lowest priority

	wakeup_fn = nullptr;
	can_wakeup_fn = nullptr;
	
	// String operations
	str_op_esi = 0;
	str_op_edi = 0;
	str_op_dir = 0;
	str_op_count = 0;

	// Presence in context lists
	for (int i = 0; i < ContextListCount; i++)
		context_list_present[i] = false;

	// Micro-instructions
	uinst_active = Emu::config.getSimKind() == comm::ArchSimDetailed;
	uinst_effaddr_emitted = false;

	// Debug
	emu->context_debug << "Context " << pid << " created\n";
}


Context::~Context()
{
	// Debug
	emu->context_debug << "Context " << pid << " destroyed\n";
}


void Context::Load(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// String in 'args' must contain at least one non-empty element
	if (!args.size() || args[0].empty())
		misc::panic("%s: function invoked with no program name, or with an "
				"empty program.", __FUNCTION__);

	// Program must not have been loaded before
	if (loader.get() || memory.get())
		misc::panic("%s: program '%s' has already been loaded in a "
				"previous call to this function.",
				__FUNCTION__, args[0].c_str());
	
	// Create new memory image
	assert(!memory.get());
	memory.reset(new mem::Memory());
	address_space_index = emu->getAddressSpaceIndex();

	// Create signal handler table
	signal_handler_table.reset(new SignalHandlerTable());

	// Create speculative memory, and link it with the real memory
	spec_mem.reset(new mem::SpecMem(memory.get()));

	// Create file descriptor table
	file_table.reset(new FileTable());
	
	// Create new loader info
	assert(!loader.get());
	loader.reset(new Loader());
	loader->args = args;
	loader->cwd = cwd;
	loader->stdin_file_name = stdin_file_name;
	loader->stdout_file_name = stdout_file_name;

	// Add environment variables
	for (int i = 0; environ[i]; i++)
		loader->env.emplace_back(environ[i]);
	for (auto &var : env)
		loader->env.emplace_back(var);

	// Load the binary
	LoadBinary();
}


void Context::Clone(Context *parent)
{
	// Register file contexts are copied from parent
	regs = parent->regs;

	// The memory image of the cloned context if the same. The memory
	// structure must be only freed by the parent when all its children have
	// been killed. The set of signal handlers is the same, too.
	address_space_index = parent->address_space_index;
	memory = parent->memory;
	
	// Create speculative memory, linked with the real memory
	spec_mem.reset(new mem::SpecMem(memory.get()));

	// Reference to parent's loader
	loader = parent->loader;

	// Signal handlers and file descriptor table
	signal_handler_table = parent->signal_handler_table;
	file_table = parent->file_table;

	// Libc segment
	glibc_segment_base = parent->glibc_segment_base;
	glibc_segment_limit = parent->glibc_segment_limit;

	// Update other fields
	this->parent = parent;
}
	

void Context::Fork(Context *parent)
{
	// Register file contexts are copied from parent
	regs = parent->regs;

	// Memory
	address_space_index = emu->getAddressSpaceIndex();
	memory.reset(new mem::Memory());
	memory->Clone(*parent->memory);
	
	// Create speculative memory, linked with the real memory
	spec_mem.reset(new mem::SpecMem(memory.get()));

	// Reference to parent's loader
	loader = parent->loader;

	// Signal handlers and file descriptor table
	signal_handler_table.reset(new SignalHandlerTable());
	file_table.reset(new FileTable());

	// Libc segment
	glibc_segment_base = parent->glibc_segment_base;
	glibc_segment_limit = parent->glibc_segment_limit;

	// Update other fields
	this->parent = parent;
}
	

void Context::DebugCallInst()
{
#if 0
	struct elf_symbol_t *from;
	struct elf_symbol_t *to;

	struct x86_loader_t *loader = self->loader;
	struct x86_regs_t *regs = self->regs;

	char *action;
	int i;

	/* Do nothing on speculative mode */
	if (self->state & X86ContextSpecMode)
		return;

	/* Call or return. Otherwise, exit */
	if (!strncmp(self->inst.format, "call", 4))
		action = "call";
	else if (!strncmp(self->inst.format, "ret", 3))
		action = "ret";
	else
		return;

	/* Debug it */
	for (i = 0; i < self->function_level; i++)
		X86ContextDebugCall("| ");
	from = elf_symbol_get_by_address(loader->elf_file, self->curr_eip, NULL);
	to = elf_symbol_get_by_address(loader->elf_file, regs->eip, NULL);
	if (from)
		X86ContextDebugCall("%s", from->name);
	else
		X86ContextDebugCall("0x%x", self->curr_eip);
	X86ContextDebugCall(" - %s to ", action);
	if (to)
		X86ContextDebugCall("%s", to->name);
	else
		X86ContextDebugCall("0x%x", regs->eip);
	X86ContextDebugCall("\n");

	/* Change current level */
	if (strncmp(self->inst.format, "call", 4))
		self->function_level--;
	else
		self->function_level++;
#endif
}


void Context::HostThreadSuspend()
{
	// Get current time
	esim::ESim *esim = esim::ESim::getInstance();
	long long now = esim->getRealTime();

	// Detach this thread - we don't want the parent to have to join it to
	// release its resources. The thread termination can be observed by
	// atomically checking the 'host_thread_suspend_active' field of the
	// context.
	pthread_detach(pthread_self());

	// Suspended in system call 'nanosleep'
	if (getState(ContextNanosleep))
	{
		// Calculate remaining sleep time in microseconds
		long long timeout = syscall_nanosleep_wakeup_time > now ?
				syscall_nanosleep_wakeup_time - now : 0;
		usleep(timeout);

	}

	// Suspended in system call 'read'
	if (getState(ContextRead))
	{
		// Get file descriptor
		FileDesc *desc = file_table->getFileDesc(syscall_read_fd);
		if (!desc)
			misc::panic("%s: invalid file descriptor (%d)",
					__FUNCTION__, syscall_read_fd);

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = POLLIN;
		int err = poll(&host_fds, 1, -1);
		if (err < 0)
			misc::panic("%s: unexpected error in host 'poll'",
					__FUNCTION__);
	}

	// Suspended in system call 'write'
	if (getState(ContextWrite))
	{
		// Get file descriptor
		FileDesc *desc = file_table->getFileDesc(syscall_write_fd);
		if (!desc)
			misc::panic("%s: invalid file descriptor (%d)",
					__FUNCTION__, syscall_write_fd);

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = POLLOUT;
		int err = poll(&host_fds, 1, -1);
		if (err < 0)
			misc::panic("%s: unexpected error in host 'poll'",
					__FUNCTION__);
	}

	// Suspended in system call 'poll'
	if (getState(ContextPoll))
	{
		// Get file descriptor
		FileDesc *desc = file_table->getFileDesc(syscall_poll_fd);
		if (!desc)
			misc::panic("%s: invalid file descriptor (%d)",
					__FUNCTION__, syscall_poll_fd);

		// Calculate timeout for host call in milliseconds from now
		int timeout;
		if (!syscall_poll_time)
			timeout = -1;
		else if (syscall_poll_time < now)
			timeout = 0;
		else
			timeout = (syscall_poll_time - now) / 1000;

		// Perform blocking host 'poll'
		struct pollfd host_fds;
		host_fds.fd = desc->getHostIndex();
		host_fds.events = ((syscall_poll_events & 4) ? POLLOUT : 0) |
				((syscall_poll_events & 1) ? POLLIN : 0);
		int err = poll(&host_fds, 1, timeout);
		if (err < 0)
			misc::panic("%s: unexpected error in host 'poll'",
					__FUNCTION__);
	}

	// Event occurred - thread finishes
	emu->LockMutex();
	emu->ProcessEventsScheduleUnsafe();
	host_thread_suspend_active = false;
	emu->UnlockMutex();
}


void Context::HostThreadSuspendCancelUnsafe()
{
	if (host_thread_suspend_active)
	{
		if (pthread_cancel(host_thread_suspend))
			misc::fatal("%s: context %d: error canceling host thread",
					__FUNCTION__, pid);
		host_thread_suspend_active = false;
	}
	emu->ProcessEventsScheduleUnsafe();
}
	

void Context::HostThreadSuspendCancel()
{
	emu->LockMutex();
	HostThreadSuspendCancelUnsafe();
	emu->UnlockMutex();
}


void Context::Suspend(CanWakeupFn can_wakeup_fn, WakeupFn wakeup_fn,
		ContextState wakeup_state)
{
	// Checks
	assert(!getState(ContextSuspended));
	assert(!this->can_wakeup_fn);
	assert(!this->wakeup_fn);

	// Store callbacks and data
	this->can_wakeup_fn = can_wakeup_fn;
	this->wakeup_fn = wakeup_fn;
	this->wakeup_state = wakeup_state;

	// Suspend context
	setState(ContextSuspended);
	setState(ContextCallback);
	setState(wakeup_state);
	emu->ProcessEventsSchedule();
}


bool Context::CanWakeup()
{
	// Checks
	assert(getState(ContextCallback));
	assert(getState(ContextSuspended));
	assert(this->can_wakeup_fn);

	// Invoke callback
	return (this->*can_wakeup_fn)();
}


void Context::Wakeup()
{
	// Checks
	assert(getState(ContextCallback));
	assert(getState(ContextSuspended));
	assert(this->wakeup_fn);

	// Wakeup context
	(this->*wakeup_fn)();
	clearState(ContextCallback);
	clearState(ContextSuspended);
	clearState(wakeup_state);

	// Reset callbacks and free data
	can_wakeup_fn = nullptr;
	wakeup_fn = nullptr;
}


void Context::HostThreadTimerCancelUnsafe()
{
	if (!host_thread_timer_active)
		return;
	if (pthread_cancel(host_thread_timer))
		misc::fatal("%s: context %d: error canceling host thread",
				__FUNCTION__, pid);
	host_thread_timer_active = false;
	emu->ProcessEventsScheduleUnsafe();
}


void Context::HostThreadTimerCancel()
{
	emu->LockMutex();
	HostThreadTimerCancelUnsafe();
	emu->UnlockMutex();
}


void Context::Execute()
{
	// Memory permissions should not be checked if the context is executing in
	// speculative mode. This will prevent guest segmentation faults to occur.
	bool spec_mode = getState(ContextSpecMode);
	if (spec_mode)
		memory->setSafe(false);
	else
		memory->setSafeDefault();

	// Read instruction from memory. Memory should be accessed here in unsafe mode
	// (i.e., allowing segmentation faults) if executing speculatively.
	char buffer[20];
	char *buffer_ptr = memory->getBuffer(regs.getEip(), 20,
			mem::MemoryAccessExec);
	if (!buffer_ptr)
	{
		// Disable safe mode. If a part of the 20 read bytes does not
		// belong to the actual instruction, and they lie on a page with
		// no permissions, this would generate an undesired protection
		// fault.
		memory->setSafe(false);
		buffer_ptr = buffer;
		memory->Access(regs.getEip(), 20, buffer_ptr,
				mem::MemoryAccessExec);
	}

	// Return to default safe mode
	memory->setSafeDefault();

	// Disassemble
	inst.Decode(buffer_ptr, regs.getEip());
	if (inst.getOpcode() == InstOpcodeInvalid && !spec_mode)
		misc::fatal("0x%x: not supported x86 instruction "
				"(%02x %02x %02x %02x...)",
				regs.getEip(), buffer_ptr[0], buffer_ptr[1],
				buffer_ptr[2], buffer_ptr[3]);

	// Clear existing list of microinstructions, though the architectural
	// simulator might have cleared it already. A new list will be generated
	// for the next executed x86 instruction.
	ClearUInstList();

	// Set last, current, and target instruction addresses
	last_eip = current_eip;
	current_eip = regs.getEip();
	target_eip = 0;

	// Reset effective address
	effective_address = 0;

	// Debug
	if (emu->isa_debug)
		emu->isa_debug << misc::fmt("%d %8lld %x: ", pid,
				emu->getInstructions(), current_eip)
				<< inst
				<< misc::fmt("  (%d bytes)",
						inst.getSize());

	// Advance instruction pointer
	regs.incEip(inst.getSize());
	
	// Call instruction emulation function
	if (inst.getOpcode())
	{
		ExecuteInstFn fn = execute_inst_fn[inst.getOpcode()];
		(this->*fn)();
	}
	
	// Debug
	emu->isa_debug << '\n';
	if (emu->call_debug)
		DebugCallInst();

	// Stats
	emu->incInstructions();
}


void Context::FinishGroup(int exit_code)
{
	// Make call on group parent only
	if (group_parent)
	{
		assert(!group_parent->group_parent);
		group_parent->FinishGroup(exit_code);
		return;
	}

	// Context already finished
	if (getState(ContextFinished) || getState(ContextZombie))
		return;

	// Finish all contexts in the group
	for (auto &context : emu->getContexts())
	{
		if (context->group_parent != this && context.get() != this)
			continue;

		if (context->getState(ContextZombie))
			context->setState(ContextFinished);
		if (context->getState(ContextHandler))
			context->ReturnFromSignalHandler();
		context->HostThreadSuspendCancel();
		context->HostThreadTimerCancel();

		// Child context of 'context' goes to state 'finished'.
		// Context 'context' goes to state 'zombie' or 'finished' if it has a parent
		if (context.get() == this)
			context->setState(context->parent ? ContextZombie : ContextFinished);
		else
			context->setState(ContextFinished);
		context->exit_code = exit_code;
	}

	// Process events
	emu->ProcessEventsSchedule();
}


void Context::Finish(int exit_code)
{
	// Context already finished
	if (getState(ContextFinished) || getState(ContextZombie))
		return;

	// If context is waiting for host events, cancel spawned host threads
	HostThreadSuspendCancel();
	HostThreadTimerCancel();

	// From now on, all children have lost their parent. If a child is
	// already zombie, finish it, since its parent won't be able to waitpid it
	// anymore.
	for (auto &context : emu->getContexts())
	{
		if (context->parent == this)
		{
			context->parent = nullptr;
			if (context->getState(ContextZombie))
				context->setState(ContextFinished);
		}
	}

	// Send finish signal to parent
	if (exit_signal && parent)
	{
		emu->syscall_debug << misc::fmt("  sending signal %d to pid %d\n",
				exit_signal, parent->pid);
		parent->signal_mask_table.getPending().Add(exit_signal);
		emu->ProcessEventsSchedule();
	}

	// If clear_child_tid was set, a futex() call must be performed on
	// that pointer. Also wake up futexes in the robust list. */
	if (clear_child_tid)
	{
		unsigned int zero = 0;
		memory->Write(clear_child_tid, 4, (char *) &zero);
		FutexWake(clear_child_tid, 1, -1);
	}
	ExitRobustList();

	// If we are in a signal handler, stop it.
	if (getState(ContextHandler))
		ReturnFromSignalHandler();

	// Finish context26
	setState(parent ? ContextZombie : ContextFinished);
	this->exit_code = exit_code;
	emu->ProcessEventsSchedule();
}


Context *Context::getZombie(int pid)
{
	for (Context *context : emu->getContextList(ContextListZombie))
	{
		if (context->parent != this)
			continue;
		if (context->pid == pid || pid == -1)
			return context;
	}
	return nullptr;
}

}  // namespace x86

