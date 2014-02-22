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

#include <arch/x86/asm/Asm.h>

#include "Context.h"
#include "Emu.h"


namespace x86
{

//
// Class 'EmuConfig'
//

void EmuConfig::Register(misc::CommandLine &command_line)
{
	// Option --x86-debug-call <file>
	command_line.RegisterString("--x86-debug-call", call_debug_file,
			"Dump debug information about function calls and "
			"returns. The control flow of an x86 program can be "
			"observed leveraging ELF symbols present in the program "
			"binary.");
	
	// Option --x86-debug-ctx <file>
	command_line.RegisterString("--x86-debug-ctx", context_debug_file,
			"Dump debug information related with context creation, "
			"destruction, allocation, or state change.");

	// Option --x86-debug-cuda <file>
	command_line.RegisterString("--x86-debug-cuda", cuda_debug_file,
			"Debug information for the CUDA driver.");

	// Option --x86-debug-glut <file>
	command_line.RegisterString("--x86-debug-glut", glut_debug_file,
			"Debug information for the GLUT library, used by "
			"OpenGL programs.");

	// Option --x86-debug-isa <file>
	command_line.RegisterString("--x86-debug-isa", isa_debug_file,
			"Debug information for dynamic execution of x86 "
			"instructions. Updates on the processor state can be "
			"analyzed using this information.");

	// Option --x86-debug-loader <file>
	command_line.RegisterString("--x86-debug-loader", loader_debug_file,
			"Dump debug information extending the analysis of the "
			"ELF program binary. This information shows which ELF "
			"sections and symbols are loaded to the initial program "
			"memory image.");

	// Option --x86-debug-opencl <file>
	command_line.RegisterString("--x86-debug-opencl", opencl_debug_file,
			"Debug information for the OpenCL driver.");
	
	// Option --x86-debug-opengl <file>
	command_line.RegisterString("--x86-debug-opengl", opencl_debug_file,
			"Debug information for the OpenGL graphics driver.");
	
	// Option --x86-debug-syscall <file>
	command_line.RegisterString("--x86-debug-syscall", syscall_debug_file,
			"Debug information for system calls performed by an x86 "
			"program, including system call code, aguments, and "
			"return value.");
}


void EmuConfig::Process()
{
	// Debuggers
	Emu::call_debug.setPath(call_debug_file);
	Emu::context_debug.setPath(context_debug_file);
	Emu::cuda_debug.setPath(cuda_debug_file);
	Emu::glut_debug.setPath(glut_debug_file);
	Emu::isa_debug.setPath(isa_debug_file);
	Emu::loader_debug.setPath(loader_debug_file);
	Emu::opencl_debug.setPath(opencl_debug_file);
	Emu::opengl_debug.setPath(opengl_debug_file);
	Emu::syscall_debug.setPath(syscall_debug_file);
}



//
// Class 'Emu'
//

// Emulator singleton
std::unique_ptr<Emu> Emu::instance;

// Debuggers
misc::Debug Emu::call_debug;
misc::Debug Emu::context_debug;
misc::Debug Emu::cuda_debug;
misc::Debug Emu::glut_debug;
misc::Debug Emu::isa_debug;
misc::Debug Emu::loader_debug;
misc::Debug Emu::opencl_debug;
misc::Debug Emu::opengl_debug;
misc::Debug Emu::syscall_debug;

// Configuration
EmuConfig Emu::config;


Emu::Emu() : Common::Emu("x86")
{
	// Obtain instance to disassembler
	as = Asm::getInstance();
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


Context *Emu::newContext(const std::vector<std::string> &args,
			const std::vector<std::string> &env,
			const std::string &cwd,
			const std::string &stdin_file_name,
			const std::string &stdout_file_name)
{
	// Create context and add to context list
	Context *context = new Context();
	contexts.emplace_back(context);

	// Load program
	context->loadProgram(args, env, cwd, stdin_file_name,
			stdout_file_name);

	// Return
	return context;
}


bool Emu::Run()
{
#if 0
	X86Emu *emu = asX86Emu(self);
	X86Context *ctx;

	/* Stop if there is no context running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (x86_emu_max_inst && asEmu(self)->instructions >= x86_emu_max_inst)
		esim_finish = esim_finish_x86_max_inst;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = emu->running_list_head; ctx; ctx = ctx->running_list_next)
		X86ContextExecute(ctx);

	/* Free finished contexts */
	while (emu->finished_list_head)
		delete(emu->finished_list_head);

	/* Process list of suspended contexts */
	X86EmuProcessEvents(emu);

	/* Still running */
	return TRUE;
#endif
	return true;
}

} // namespace x86

