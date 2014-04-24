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

#include <cstdlib>
#include <iostream>

#include <arch/common/Runtime.h>
#include <arch/x86/asm/Asm.h>
#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/x86/emu/FileTable.h>
#include <arch/x86/emu/Signal.h>
#include <driver/opencl/OpenCLDriver.h>
#include <driver/opengl/OpenGLDriver.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>
#include <lib/esim/ESim.h>

#include "Wrapper.h"


void main_cpp(int argc, char **argv)
{
	// Read command line
	misc::CommandLine command_line(argc, argv);
	command_line.setErrorMessage("Please type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.\n");
	command_line.setHelp("Syntax:"
			"\n\n"
			"$ m2s [<options>] [<exe>] [<args>]"
			"\n\n"
			"Multi2Sim's command line can take a program "
			"executable <exe> as an argument, given as a binary "
			"file in any of the supported CPU architectures, and "
			"optionally followed by its arguments <args>. The "
			"following list of command-line options can be used "
			"for <options>:");

	// Register three sample command-line options
	long long m2s_max_time = 0;
	command_line.RegisterInt64("--max-time", m2s_max_time,
			"Maximum simulation time in seconds. The simulator "
			"will stop once this time is exceeded. A value of 0 "
			"(default) means no time limit.");
	
	std::string m2s_trace_file;
	command_line.RegisterString("--trace", m2s_trace_file,
			"Generate a trace file with debug information on the "
			"configuration of the modeled CPUs, GPUs, and memory "
			"system, as well as their dynamic simulation. The "
			"trace is a compressed plain-text file in format. The "
			"user should watch the size of the generated trace as "
			"simulation runs, since the trace file can quickly "
			"become extremely large.");
	
	std::string m2s_visual_file;
	command_line.RegisterString("--visual", m2s_visual_file,
			"Run the Multi2Sim Visualization Tool. This option "
			"consumes a file generated with the '--trace' option "
			"in a previous simulation. This option is only "
			"available on systems with support for GTK 3.0 or "
			"higher.");

	// Register module configurations
	command_line.AddConfig(x86::Asm::config);
	command_line.AddConfig(x86::Emu::config);

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	if (!command_line.Process(false))
		return;

	// Finish if C++ version of Multi2Sim is not activated
	if (!command_line.getUseCpp())
		return;

	// Register architectures
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
	arch_pool->Register("x86", "x86");
	arch_pool->Register("SouthernIslands", "si");

	// Register runtime and driver pairs
	comm::RuntimePool *runtime_pool = comm::RuntimePool::getInstance();
	runtime_pool->Register("OpenCL", "OpenCL", "m2s-opencl", "/dev/m2s-si-cl", Driver::OpenCLSIDriver::getInstance());

#ifdef HAVE_OPENGL
	runtime_pool->Register("OpenGL", "OpenGL", "m2s-opengl", "/dev/m2s-si-gl", Driver::OpenGLSIDriver::getInstance());
#endif

	// Test Regs
	if (command_line.getNumArguments())
	{
		x86::Emu *emu = x86::Emu::getInstance();
		x86::Context *context = emu->newContext();
		context->Load(command_line.getArguments(),
				std::vector<std::string>(), misc::getCwd(),
				"", "");
		esim::ESim *esim = esim::ESim::getInstance();
		while (!esim->hasFinished())
		{
			bool active = emu->Run();
			if (!active)
				esim->Finish(esim::ESimFinishCtx);
			esim->ProcessEvents();
		}

	}

	// End
	exit(0);
}

