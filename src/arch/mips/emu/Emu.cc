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


void Emu::ProcessEvents()
{
	// Check if events need actually be checked.
	LockMutex();
	if (!process_events_force)
	{
		UnlockMutex();
		return;
	}

	// By default, no subsequent call to ProcessEvents() is assumed
	process_events_force = false;


	//
	// LOOP 1
	// Look at the list of suspended contexts and try to find
	// one that needs to be waken up.
	//
	auto &suspended_context_list = getContextList(ContextListSuspended);
	auto iter_next = suspended_context_list.end();
	for (auto iter = suspended_context_list.begin();
				iter != suspended_context_list.end();
				iter = iter_next)
	{
		// Save iterator to next element here, since the context can be
		// removed from the suspended list if waken up.
		iter_next = iter;
		++iter_next;
		Context *context = *iter;
		assert(context->getState(ContextSuspended));
		assert(context->context_list_iter[ContextListSuspended] == iter);
		assert(context->context_list_present[ContextListSuspended]);

		// Context suspended in a system call using a custom wake up
		// check call-back function. NOTE: this is a new mechanism. It'd
		// be nice if all other system calls started using it. It is
		// nicer, since it allows for a check of wake up conditions
		// together with the system call itself, without having
		// distributed code for the implementation of a system call
		// (e.g. 'read').
		if (context->getState(ContextCallback) && context->CanWakeup())
		{
			context->Wakeup();
			continue;
		}
	}
#if 0
	/*
	 * LOOP 2
	 * Check list of all contexts for expired timers.
	 */
	for (context = self->context_list_head; context; context = context->context_list_next)
	{
		int sig[3] = { 14, 26, 27 };  // SIGALRM, SIGVTALRM, SIGPROF
		int i;

		// If there is already a 'ke_host_thread_timer' running, do nothing.
		if (context->host_thread_timer_active)
			continue;

		/* Check for any expired 'itimer': itimer_value < now
		 * In this case, send corresponding signal to process.
		 * Then calculate next 'itimer' occurrence: itimer_value = now + itimer_interval */
		for (i = 0; i < 3; i++ )
		{
			// Timer inactive or not expired yet
			if (!context->itimer_value[i] || context->itimer_value[i] > now)
				continue;

			/* Timer expired - send a signal.
			 * The target process might be suspended, so the host thread is canceled, and a new
			 * call to 'X86EmuProcessEvents' is scheduled. Since 'ke_process_events_mutex' is
			 * already locked, the thread-unsafe version of 'x86_ctx_host_thread_suspend_cancel' is used. */
			X86ContextHostThreadSuspendCancelUnsafe(context);
			self->process_events_force = 1;
			x86_sigset_add(&context->signal_mask_table->pending, sig[i]);

			// Calculate next occurrence
			context->itimer_value[i] = 0;
			if (context->itimer_interval[i])
				context->itimer_value[i] = now + context->itimer_interval[i];
		}

		// Calculate the time when next wakeup occurs.
		context->host_thread_timer_wakeup = 0;
		for (i = 0; i < 3; i++)
		{
			if (!context->itimer_value[i])
				continue;
			assert(context->itimer_value[i] >= now);
			if (!context->host_thread_timer_wakeup || context->itimer_value[i] < context->host_thread_timer_wakeup)
				context->host_thread_timer_wakeup = context->itimer_value[i];
		}

		// If a new timer was set, launch ke_host_thread_timer' again
		if (context->host_thread_timer_wakeup)
		{
			context->host_thread_timer_active = 1;
			if (pthread_create(&context->host_thread_timer, NULL, X86ContextHostThreadTimer, context))
				fatal("%s: could not create child thread", __FUNCTION__);
		}
	}


#endif
	//
	// LOOP 3
	// Process pending signals in running contexts to launch signal handlers
	//
	for (Context *context : context_list[ContextListRunning])
		context->CheckSignalHandler();

	// Unlock
	UnlockMutex();
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
	ProcessEvents();

	// Still running
	return true;
}

} // namespace MIPS
