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

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"

// Variable used in function Context::loadProgram()
extern char **environ;

	
namespace x86
{


misc::StringMap context_state_map = {
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



//
// Public functions
//

Context::Context()
{
	// Save emulator instance
	emu = Emu::getInstance();

	// Initialize
	state = 0;
	pid = 0;  // FIXME

	// Presence in context lists
	for (int i = 0; i < ContextListCount; i++)
		context_list_present[i] = false;
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


void Context::Execute()
{
	clearState(ContextRunning);
	setState(ContextFinished);
}


}  // namespace x86

