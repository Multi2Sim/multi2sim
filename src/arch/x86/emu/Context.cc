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



//
// Private functions
//

Context::ExecuteInstFn Context::execute_inst_fn[InstOpcodeCount] =
{
		nullptr  // For InstOpcodeNone
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) \
		 , &Context::ExecuteInst_##name
#include <arch/x86/asm/asm.dat>
#undef DEFINST
};


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


void Context::loadProgram(const std::vector<std::string> &args,
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


}  // namespace x86

