/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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

#include "Emu.h"
#include "Context.h"

namespace MIPS
{


//
// Configuration variables
//

// Simulation kind
comm::ArchSimKind Emu::sim_kind = comm::ArchSimFunctional;

// Maximum number of instructions
long long Emu::max_instructions = 0;




//
// Static variables
//

// Emulator singleton
std::unique_ptr<Emu> Emu::instance;


// Debuggers
misc::Debug Emu::loader_debug;
misc::Debug Emu::context_debug;



//
// Functions
//

void Emu::RegisterOptions()
{
}


void Emu::ProcessOptions()
{
}


Emu::Emu() : comm::Emu("MIPS")
{
	// Initialize
	pid = 100;
	process_events_force = false;
	schedule_signal = false;
	futex_sleep_count = 0;
	address_space_index = 0;
}

Emu::~Emu()
{

}

Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Emu());
	return instance.get();
}


Context *Emu::newContext()
{
	// Create context and add to context list
	Context *context = new Context();
	contexts.emplace_back(context);

	// Save position in context list
	auto iter = contexts.end();
	context->contexts_iter = --iter;

	// Set the context in running state. This call will add it automatically
	// to the emulator list of running contexts.
	context->setState(ContextRunning);

	// Return
	return context;
}


void Emu::LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env,
		const std::string &cwd,
		const std::string &stdin_file_name,
		const std::string &stdout_file_name)
{
	// Create new context
	Context *context = newContext();
	context->Load(args,
			env,
			cwd,
			stdin_file_name,
			stdout_file_name);
}


void Emu::freeContext(Context *context)
{
	// Remove context from all context lists
/*	for (int i = 0; i < ContextListCount; i++)
		RemoveContextFromList((ContextListType) i, context);

	// Remove from main context list. This will invoke the context
	// destructor and free it.
	contexts.erase(context->contexts_iter); */
}


bool Emu::Run()
{
	// Stop if there is no more contexts
	if (!contexts.size())
		return false;

	// Stop if maximum number of CPU instructions exceeded
	if (max_instructions && instructions >= max_instructions)
		esim->Finish("MIPSMaxInst");

	// Stop if any previous reason met
	if (esim->hasFinished())
		return true;

	// Run an instruction from every running context. During execution, a
	// context can remove itself from the running list, so we need to save
	// the iterator to the next element before executing.
	auto end = context_list[ContextListRunning].end();
	for (auto it = context_list[ContextListRunning].begin(); it != end; )
	{
		// Save position of next context
		auto next = it;
		++next;

		// Run one iteration
		Context *context = *it;
		context->Execute();

		// Move to saved next context
		it = next;
	}

	// Free finished contexts
	while (context_list[ContextListFinished].size())
		freeContext(context_list[ContextListFinished].front());

	// Process list of suspended contexts
	//ProcessEvents();


	// Still running
	return true;
}

} // namespace MIPS
