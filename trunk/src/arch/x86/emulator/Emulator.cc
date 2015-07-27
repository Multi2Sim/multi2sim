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

#include <arch/x86/disassembler/Disassembler.h>
#include <lib/esim/Engine.h>

#include "Context.h"
#include "Emulator.h"


namespace x86
{

std::string Emulator::call_debug_file;
std::string Emulator::context_debug_file;
std::string Emulator::cuda_debug_file;
std::string Emulator::glut_debug_file;
std::string Emulator::isa_debug_file;
std::string Emulator::loader_debug_file;
std::string Emulator::opencl_debug_file;
std::string Emulator::opengl_debug_file;
std::string Emulator::syscall_debug_file;

long long Emulator::max_instructions;

std::unique_ptr<Emulator> Emulator::instance;

misc::Debug Emulator::call_debug;
misc::Debug Emulator::context_debug;
misc::Debug Emulator::cuda_debug;
misc::Debug Emulator::glut_debug;
misc::Debug Emulator::isa_debug;
misc::Debug Emulator::loader_debug;
misc::Debug Emulator::opencl_debug;
misc::Debug Emulator::opengl_debug;
misc::Debug Emulator::syscall_debug;


void Emulator::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("x86");

	// Option --x86-debug-call <file>
	command_line->RegisterString("--x86-debug-call <file>", call_debug_file,
			"Dump debug information about function calls and "
			"returns. The control flow of an x86 program can be "
			"observed leveraging ELF symbols present in the program "
			"binary.");
	
	// Option --x86-debug-ctx <file>
	command_line->RegisterString("--x86-debug-ctx <file>", context_debug_file,
			"Dump debug information related with context creation, "
			"destruction, allocation, or state change.");

	// Option --x86-debug-cuda <file>
	command_line->RegisterString("--x86-debug-cuda <file>", cuda_debug_file,
			"Debug information for the CUDA driver.");

	// Option --x86-debug-glut <file>
	command_line->RegisterString("--x86-debug-glut <file>", glut_debug_file,
			"Debug information for the GLUT library, used by "
			"OpenGL programs.");

	// Option --x86-debug-isa <file>
	command_line->RegisterString("--x86-debug-isa <file>", isa_debug_file,
			"Debug information for dynamic execution of x86 "
			"instructions. Updates on the processor state can be "
			"analyzed using this information.");

	// Option --x86-debug-loader <file>
	command_line->RegisterString("--x86-debug-loader <file>", loader_debug_file,
			"Dump debug information extending the analysis of the "
			"ELF program binary. This information shows which ELF "
			"sections and symbols are loaded to the initial program "
			"memory image.");

	// Option --x86-debug-opencl <file>
	command_line->RegisterString("--x86-debug-opencl <file>", opencl_debug_file,
			"Debug information for the OpenCL driver.");
	
	// Option --x86-debug-opengl <file>
	command_line->RegisterString("--x86-debug-opengl <file>", opencl_debug_file,
			"Debug information for the OpenGL graphics driver.");
	
	// Option --x86-debug-syscall <file>
	command_line->RegisterString("--x86-debug-syscall <file>", syscall_debug_file,
			"Debug information for system calls performed by an x86 "
			"program, including system call code, aguments, and "
			"return value.");
		
	// Option --x86-max-inst <number>
	command_line->RegisterInt64("--x86-max-inst <number> (default = 0)",
			max_instructions,
			"Maximum number of x86 instructions. On x86 functional "
			"simulation, this limit is given in number of emulated "
			"instructions. On x86 detailed simulation, it is given as "
			"the number of committed (non-speculative) instructions. "
			"A value of 0 means no limit.");
}


void Emulator::ProcessOptions()
{
	// Debuggers
	call_debug.setPath(call_debug_file);
	context_debug.setPath(context_debug_file);
	cuda_debug.setPath(cuda_debug_file);
	glut_debug.setPath(glut_debug_file);
	isa_debug.setPath(isa_debug_file);
	loader_debug.setPath(loader_debug_file);
	opencl_debug.setPath(opencl_debug_file);
	opengl_debug.setPath(opengl_debug_file);
	syscall_debug.setPath(syscall_debug_file);
}


void Emulator::AddContextToList(Context::ListType type, Context *context)
{
	// Nothing if already present
	if (context->context_list_present[type])
		return;

	// Add context
	context->context_list_present[type] = true;
	context_list[type].push_back(context);
	auto iterator = context_list[type].end();
	context->context_list_iterator[type] = --iterator;
}

	
void Emulator::RemoveContextFromList(Context::ListType type, Context *context)
{
	// Nothing if not present
	if (!context->context_list_present[type])
		return;

	// Remove context
	context->context_list_present[type] = false;
	auto iterator = context->context_list_iterator[type];
	context_list[type].erase(iterator);
}
	

void Emulator::UpdateContextInList(Context::ListType type, Context *context,
			bool present)
{
	if (present && !context->context_list_present[type])
	{
		AddContextToList(type, context);
	}
	else if (!present && context->context_list_present[type])
	{
		RemoveContextFromList(type, context);
	}
}


Emulator *Emulator::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance = misc::new_unique<Emulator>();
	return instance.get();
}


Context *Emulator::newContext()
{
	// Create context and add to context list
	auto it = contexts.insert(contexts.end(),
			misc::new_unique<Context>());
	
	// Get created context, and save position in context list
	Context *context = it->get();
	context->contexts_iterator = it;

	// Set the context in running state. This call will add it automatically
	// to the emulator list of running contexts.
	context->setState(Context::StateRunning);

	// Return
	return context;
}


Context *Emulator::getContext(int pid)
{
	// Find context
	for (auto &context : contexts)
		if (context->getId() == pid)
			return context.get();
	
	// Not found
	return nullptr;
}


void Emulator::LoadProgram(const std::vector<std::string> &args,
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


void Emulator::FreeContext(Context *context)
{
	// Remove context from all context lists
	for (int i = 0; i < Context::ListCount; i++)
		RemoveContextFromList((Context::ListType) i, context);
	
	// Remove from main context list. This will invoke the context
	// destructor and free it.
	contexts.erase(context->contexts_iterator);
}


void Emulator::ProcessEventsSchedule()
{
	LockMutex();
	process_events_force = true;
	UnlockMutex();
}

void Emulator::ProcessEvents()
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
	auto &suspended_context_list = getContextList(Context::ListSuspended);
	auto iterator_next = suspended_context_list.end();
	for (auto iterator = suspended_context_list.begin();
			iterator != suspended_context_list.end();
			iterator = iterator_next)
	{
		// Save iterator to next element here, since the context can be
		// removed from the suspended list if waken up.
		iterator_next = iterator;
		++iterator_next;
		Context *context = *iterator;
		assert(context->getState(Context::StateSuspended));
		assert(context->context_list_iterator[Context::ListSuspended] == iterator);
		assert(context->context_list_present[Context::ListSuspended]);

		// Context suspended in a system call using a custom wake up
		// check call-back function. NOTE: this is a new mechanism. It'd
		// be nice if all other system calls started using it. It is
		// nicer, since it allows for a check of wake up conditions
		// together with the system call itself, without having
		// distributed code for the implementation of a system call
		// (e.g. 'read').
		if (context->getState(Context::StateCallback) &&
				context->CanWakeup())
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
	for (Context *context : context_list[Context::ListRunning])
		context->CheckSignalHandler();
	
	// Unlock
	UnlockMutex();
}


bool Emulator::Run()
{
	// Stop if there is no more contexts
	if (!contexts.size())
		return false;

	// Stop if maximum number of CPU instructions exceeded
	if (max_instructions && instructions >= max_instructions)
		esim->Finish("x86MaxInst");

	// Stop if any previous reason met
	if (esim->hasFinished())
		return true;

	// Run an instruction from every running context. During execution, a
	// context can remove itself from the running list, so traversing the
	// running list is not an option.
	for (auto &context : contexts)
	{
		// Skip if not running
		if (!context->getState(Context::StateRunning))
			continue;

		// Run one iteration
		context->Execute();
	}

	// Free finished contexts
	while (context_list[Context::ListFinished].size())
		FreeContext(context_list[Context::ListFinished].front());

	// Process list of suspended contexts
	ProcessEvents();

	// Still running
	return true;
}

} // namespace x86

