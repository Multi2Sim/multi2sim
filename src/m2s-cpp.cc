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

#include <csignal>
#include <cstdlib>
#include <iostream>

#include <arch/common/Driver.h>
#include <arch/common/Runtime.h>
#include <arch/mips/asm/Asm.h>
#include <arch/mips/emu/Context.h>
#include <arch/mips/emu/Emu.h>
#include <arch/x86/asm/Asm.h>
#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/x86/emu/FileTable.h>
#include <arch/x86/emu/Signal.h>
#include <arch/hsa/asm/Asm.h>
#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/emu/Emu.h>
#include <arch/southern-islands/driver/Driver.h>
#include <driver/opencl/OpenCLDriver.h>
#include <driver/opengl/OpenGLDriver.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Environment.h>
#include <lib/cpp/Misc.h>
#include <lib/esim/ESim.h>

#include "Wrapper.h"


//
// Configuration options
//

// Maximum simulation time
long long m2s_max_time = 0;

// Binary file for OpenCL runtime
std::string m2s_opencl_binary;

// Debug information in OpenCL runtime
std::string m2s_opencl_debug;

// List of OpenCL devices for runtime
std::string m2s_opencl_devices;

// Trace file
std::string m2s_trace_file;

// Visualization tool input file
std::string m2s_visual_file;




//
// Global variables
//

// Number of iterations in the main simulation loop
long long m2s_loop_iterations = 0;

// Variable set to a value other than 0 by the signal handler when a signal is
// received from the user.
volatile int m2s_signal_received;




//
// Functions
//


void RegisterArchitectures()
{
	// Get architecture pool
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();

	// x86
	arch_pool->Register("x86",
			x86::Asm::getInstance(),
			x86::Emu::getInstance());
	
	// Southern Islands
	arch_pool->Register("SouthernIslands");

	// HSA
	arch_pool->Register("HSA");

	// MIPS
	arch_pool->Register("MIPS",
			MIPS::Asm::getInstance(),
			MIPS::Emu::getInstance());
}


void RegisterRuntimes()
{
	// Get runtime pool
	comm::RuntimePool *runtime_pool = comm::RuntimePool::getInstance();

	// OpenCL runtime
	runtime_pool->Register("OpenCL", "libOpenCL", "libm2s-opencl");

#ifdef HAVE_OPENGL
	// OpenGL runtime
	runtime_pool->Register("OpenGL", "libOpenGL", "libm2s-opengl");
#endif
}


void RegisterDrivers()
{
	// Get driver pool
	comm::DriverPool *driver_pool = comm::DriverPool::getInstance();

	// HSA driver
	HSA::Driver *hsa_driver = HSA::Driver::getInstance();
	driver_pool->Register(hsa_driver);

	// Southern Islands driver
	SI::Driver *si_driver = SI::Driver::getInstance();
	driver_pool->Register(si_driver);
}


// Load a program from the command line
void LoadCommandLineProgram()
{
	// No program specified
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	if (command_line->getNumArguments() == 0)
		return;
	
	// Get executable path
	std::string path = command_line->getArgument(0);

	// Read ELF header
	ELFReader::Header header(path);
	switch (header.getMachine())
	{
	case EM_386:
	{
		x86::Emu *emu = x86::Emu::getInstance();
		emu->loadProgram(command_line->getArguments());
		break;
	}

	case EM_ARM:
	{
		std::cout << "ARM\n";
		break;
	}

	case EM_MIPS:
	{
		MIPS::Emu *emu = MIPS::Emu::getInstance();
		emu->loadProgram(command_line->getArguments());
		break;
	}
	
	default:
		misc::fatal("%s: unsupported ELF architecture", path.c_str());
	}
}


// Load programs from context configuration file
void LoadPrograms()
{
#if 0
	struct config_t *config;

	char section[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];
	char *exe_file_name;
	char *cwd_path;

	Elf32_Ehdr ehdr;

	int id;

	/* Continue processing the context configuration file, if specified. */
	if (!*ctx_config_file_name)
		return;

	/* Open file */
	config = config_create(ctx_config_file_name);
	if (*ctx_config_file_name)
		config_load(config);

	/* Iterate through consecutive contexts */
	for (id = 0; ; id++)
	{
		/* Read section */
		snprintf(section, sizeof section, "Context %d", id);
		if (!config_section_exists(config, section))
			break;

		/* Read executable full path */
		exe_file_name = config_read_string(config, section, "Exe", "");
		cwd_path = config_read_string(config, section, "Cwd", "");
		file_full_path(exe_file_name, cwd_path, exe_full_path, sizeof exe_full_path);

		/* Load context depending on architecture */
		elf_file_read_header(exe_full_path, &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			X86EmuLoadContextsFromConfig(x86_emu, config, section);
			break;

		case EM_ARM:
			arm_ctx_load_from_ctx_config(config, section);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	/* Close file */
	config_check(config);
	config_free(config);
#endif
}


void SignalHandler(int signum)
{
	// If a signal SIGINT has been caught already and not processed, it is
	// time to not defer it anymore. Execution ends here.
	if (m2s_signal_received == signum && signum == SIGINT)
	{
		std::cerr << "SIGINT received\n";
		exit(1);
	}

	// Just record that we are receiving a signal. It is not a good idea to
	// process it now, since we might be interfering some critical
	// execution. The signal will be processed at the end of the simulation
	// loop iteration.
	m2s_signal_received = signum;
}


void ProcessSignal()
{
	// Process signal
	esim::ESim *esim = esim::ESim::getInstance();
	switch (m2s_signal_received)
	{

	case SIGINT:
	{
		// Second time signal was received, abort
		if (esim->hasFinished())
			abort();

		// Try to normally finish simulation
		esim->Finish(esim::ESimFinishSignal);
		misc::warning("signal SIGINT received");
		break;
	}

	case SIGUSR1:
	{
		// FIXME - support for simulation dump
		break;
	}

	default:

		std::cerr << "Signal " << m2s_signal_received << " received\n";
		exit(1);
	}

	// Signal already processed
	m2s_signal_received = 0;
}


void RegisterOptions()
{
	// Set error message
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->setErrorMessage("Please type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.\n");

	// Set help message
	command_line->setHelp("Syntax:"
			"\n\n"
			"$ m2s [<options>] [<exe>] [<args>]"
			"\n\n"
			"Multi2Sim's command line can take a program "
			"executable <exe> as an argument, given as a binary "
			"file in any of the supported CPU architectures, and "
			"optionally followed by its arguments <args>. The "
			"following list of command-line options can be used "
			"for <options>:");
	

	//
	// General Multi2Sim Options
	//
	
	// Set category for following options
	command_line->setCategory("default", "General Multi2Sim Options");
	
	// Maximum simulation time
	command_line->RegisterInt64("--max-time <time> (default = 0)",
			m2s_max_time,
			"Maximum simulation time in seconds. The simulator "
			"will stop once this time is exceeded. A value of 0 "
			"(default) means no time limit.");
	
	// Trace file
	command_line->RegisterString("--trace <file>", m2s_trace_file,
			"Generate a trace file with debug information on the "
			"configuration of the modeled CPUs, GPUs, and memory "
			"system, as well as their dynamic simulation. The "
			"trace is a compressed plain-text file in format. The "
			"user should watch the size of the generated trace as "
			"simulation runs, since the trace file can quickly "
			"become extremely large.");
	
	// Visualization tool input file
	command_line->RegisterString("--visual <file>", m2s_visual_file,
			"Run the Multi2Sim Visualization Tool. This option "
			"consumes a file generated with the '--trace' option "
			"in a previous simulation. This option is only "
			"available on systems with support for GTK 3.0 or "
			"higher.");

	
	//
	// OpenCL runtime options
	//

	// Category
	command_line->setCategory("OpenCL", "OpenCL Runtime Options");

	// Debug information
	command_line->RegisterString("--opencl-debug <file>", m2s_opencl_debug,
			"Debug file used in the OpenCL runtime to dump debug "
			"information related with OpenCL API calls. This "
			"option is equivalent to environment variable "
			"M2S_OPENCL_DEBUG.");
	
	// Device list
	command_line->RegisterString("--opencl-devices <list>",
			m2s_opencl_devices,
			"List of devices exposed by the OpenCL runtime to the "
			"guest application in calls to clGetDeviceIDs, in the "
			"specified order. The list of devices should be "
			"separated by commas. This option is equivalent to "
			"environment variable M2S_OPENCL_DEVICES. "
			"Possible values are: x86, hsa, southern-islands, "
			"union.");
	
	// Binary
	command_line->RegisterString("--opencl-binary <file>",
			m2s_opencl_binary,
			"OpenCL kernel binary used for an OpenCL guest "
			"application executing a call to "
			"clCreateProgramWithSource. This option is equivalent "
			"to environment variable M2S_OPENCL_BINARY.");
	
}


void ProcessOptions()
{
	// Get environment
	misc::Environment *environment = misc::Environment::getInstance();

	// OpenCL runtime debug
	if (!m2s_opencl_debug.empty())
		environment->addVariable("M2S_OPENCL_DEBUG", m2s_opencl_debug);
	
	// OpenCL device list
	if (!m2s_opencl_devices.empty())
		environment->addVariable("M2S_OPENCL_DEVICES", m2s_opencl_devices);
	
	// OpenCL binary
	if (!m2s_opencl_binary.empty())
		environment->addVariable("M2S_OPENCL_BINARY", m2s_opencl_binary);
}


void MainLoop()
{
	// Install signal handlers
	signal(SIGINT, &SignalHandler);
	signal(SIGABRT, &SignalHandler);
	signal(SIGUSR1, &SignalHandler);
	signal(SIGUSR2, &SignalHandler);

	// Get singletons
	esim::ESim *esim = esim::ESim::getInstance();
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();

	// Simulation loop
	while (!esim->hasFinished())
	{
		// Run iteration for all architectures. This function returns
		// the number of architectures actively running emulation, as
		// well as the number of architectures running an active timing
		// simulation.
		int num_emu_active, num_timing_active;
		arch_pool->Run(num_emu_active, num_timing_active);

		// Event-driven simulation. Only process events and advance to
		// next global simulation cycle if any architecture performed a
		// useful timing simulation.
		if (num_timing_active)
			esim->ProcessEvents();

		// If neither functional nor timing simulation was performed for
		// any architecture, it means that all guest contexts finished
		// execution - simulation can end.
		if (!num_emu_active && !num_timing_active)
			esim->Finish(esim::ESimFinishCtx);

		// Count loop iterations, and check for limit in simulation time
		// only every 128k iterations. This avoids a constant overhead
		// of system calls.
		m2s_loop_iterations++;
		if (m2s_max_time > 0
				&& !(m2s_loop_iterations & ((1 << 17) - 1))
				&& esim->getRealTime() > m2s_max_time * 1000000)
			esim->Finish(esim::ESimFinishMaxTime);

		// Signal received
		if (m2s_signal_received)
			ProcessSignal();
	}

	/* Restore default signal handlers */
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, SIG_DFL);
}


void main_cpp(int argc, char **argv)
{
	// Read command line
	RegisterOptions();
	HSA::Asm::RegisterOptions();
	HSA::Driver::RegisterOptions();
	HSA::Emu::RegisterOptions();
	MIPS::Asm::RegisterOptions();
	MIPS::Emu::RegisterOptions();
	SI::Driver::RegisterOptions();
	x86::Asm::RegisterOptions();
	x86::Emu::RegisterOptions();

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	if (!command_line->Process(argc, argv, false))
		return;

	// Finish if C++ version of Multi2Sim is not activated
	if (!command_line->getUseCpp())
		return;

	// Process command line
	ProcessOptions();
	HSA::Asm::ProcessOptions();
	HSA::Driver::ProcessOptions();
	HSA::Emu::ProcessOptions();
	MIPS::Asm::ProcessOptions();
	MIPS::Emu::ProcessOptions();
	SI::Driver::ProcessOptions();
	x86::Asm::ProcessOptions();
	x86::Emu::ProcessOptions();

	// Register architectures, runtimes, and drivers
	RegisterArchitectures();
	RegisterRuntimes();
	RegisterDrivers();

	// Load programs
	LoadCommandLineProgram();
	LoadPrograms();

	// Main simulation loop
	MainLoop();

	// End
	exit(0);
}

