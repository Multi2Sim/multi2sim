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

#include <arch/common/CallStack.h>
#include <arch/common/Driver.h>
#include <arch/common/Runtime.h>
#include <arch/kepler/asm/Asm.h>
#include <arch/kepler/driver/Driver.h>
#include <arch/kepler/emu/Emu.h>
#include <arch/mips/asm/Asm.h>
#include <arch/mips/emu/Context.h>
#include <arch/mips/emu/Emu.h>
#include <arch/x86/asm/Asm.h>
#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/x86/emu/Signal.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/CPU.h>
#include <arch/hsa/asm/Asm.h>
#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/emu/Emu.h>
#include <arch/southern-islands/driver/Driver.h>
#include <arch/arm/asm/Asm.h>
#include <arch/arm/emu/Emu.h>
#include <dram/System.h>
#include <driver/opencl/OpenCLDriver.h>
#include <driver/opengl/OpenGLDriver.h>
#include <memory/MMU.h>
#include <memory/Manager.h>
#include <memory/System.h>
#include <network/System.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Environment.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Misc.h>
#include <lib/esim/Action.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>


//
// Configuration options
//

// Context configuration file
std::string m2s_context_config;

// Debug information in CUDA runtime
std::string m2s_cuda_debug;

// Event-driven simulator configuration file
std::string m2s_esim_config;

// Event-driven simulator debugger
std::string m2s_debug_esim;

// Inifile debugger
std::string m2s_debug_inifile;

// Call stack debugger
std::string m2s_debug_callstack;

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





//
// Functions
//


void RegisterRuntimes()
{
	// Get runtime pool
	comm::RuntimePool *runtime_pool = comm::RuntimePool::getInstance();

	// CUDA runtime
	runtime_pool->Register("CUDA", "libCUDA", "libm2s-cuda");

	// OpenCL runtime
	runtime_pool->Register("OpenCL", "libOpenCL", "libm2s-opencl");

	// HSA runtime
	runtime_pool->Register("HSA", "libHSA", "libm2s-hsa");

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

	// Kepler driver
	Kepler::Driver *kepler_driver = Kepler::Driver::getInstance();
	driver_pool->Register(kepler_driver);

	// Southern Islands driver
	SI::Driver *si_driver = SI::Driver::getInstance();
	driver_pool->Register(si_driver);
}


// Load a program from the command line
void LoadProgram(const std::vector<std::string> &args,
		const std::vector<std::string> &env = { },
		const std::string &cwd = "",
		const std::string &stdin_file_name = "",
		const std::string &stdout_file_name = "")
{
	// Skip if no program specified
	if (args.size() == 0)
		return;
	
	// Choose emulator based on ELF header
	std::string exe = misc::getFullPath(args[0], cwd);
	ELFReader::File elf_file(exe, false);
	comm::Emu *emu;
	switch (elf_file.getMachine())
	{
	case EM_386:

		emu = x86::Emu::getInstance();
		break;

	case EM_ARM:

		emu = ARM::Emu::getInstance();
		break;

	case EM_MIPS:

		emu = MIPS::Emu::getInstance();
		break;

	/*
	case 0: 
		
		// ELF file for HSA Brig format do not have machine code
		// FIXME: Rather than preload the whole brig file, modify the 
		// 	ELFReader to support brig file validation
		if(HSA::Asm::isValidBrigELF(exe))
		{
			emu = HSA::Emu::getInstance();
		}
		break;
	*/

	default:
		misc::fatal("%s: unsupported ELF architecture", exe.c_str());
	}

	// Load the program in selected emulator
	emu->LoadProgram(args, env, cwd, stdin_file_name, stdout_file_name);
}


// Load programs from context configuration file
void LoadPrograms()
{
	// Load command-line program
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	LoadProgram(command_line->getArguments());

	// Load more programs if context configuration file was specified
	if (m2s_context_config.empty())
		return;

	// Load contexts
	misc::IniFile ini_file(m2s_context_config);
	for (int index = 0; ; index++)
	{
		// Stop if section does not exist
		std::string section = misc::fmt("Context %d", index);
		if (!ini_file.Exists(section))
			break;

		// Load
		std::string exe = ini_file.ReadString(section, "Exe");
		std::string cwd = ini_file.ReadString(section, "Cwd");
		std::string stdin_file_name = ini_file.ReadString(section, "Stdin");
		std::string stdout_file_name = ini_file.ReadString(section, "Stdout");
		
		// Arguments
		std::vector<std::string> args;
		std::string args_str = ini_file.ReadString(section, "Args");
		misc::StringTokenize(args_str, args);
		args.insert(args.begin(), exe);

		// Environment variables
		std::vector<std::string> env;
		std::string env_str = ini_file.ReadString(section, "Env");
		misc::Environment::getFromString(env_str, env);

		// Load program
		LoadProgram(args, env, cwd, stdin_file_name, stdout_file_name);
	}
}


void RegisterOptions()
{
	// Set error message
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->setErrorMessage("\nPlease type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.");

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

	// Context configuration
	command_line->RegisterString("--ctx-config <file>",
			m2s_context_config,
			"Use <file> as the context configuration file. This "
			"file describes the initial set of running applications, "
			"their arguments, and environment variables. Use option "
			"--ctx-config-help for a description of the context "
			"configuration file format.");
	
	// Debugger for call stack
	command_line->RegisterString("--debug-callstack <file>",
			m2s_debug_callstack,
			"Dump debug information related with the CPU context "
			"call stacks, including function invocations and "
			"returns.");
	
	// Event-driven simulator configuration
	command_line->RegisterString("--esim-config <file>",
			m2s_esim_config,
			"Use <file> as the event-driven simulation engine "
			"configuration file.  This file can describe a set of "
			"frequency domains, event types, times to schedule "
			"events, and commands to check that events were "
			"triggered at certain cycles.");
	
	// Debugger for event-driven simulator
	command_line->RegisterString("--debug-esim <file>",
			m2s_debug_esim,
			"Dump debug information related with the event-driven "
			"simulation engine.");
	
	// Debugger for Inifile parser
	command_line->RegisterString("--debug-inifile <file>",
			m2s_debug_inifile,
			"Dump debug information about all processed INI files "
			"into the specified path.");
	
	// Maximum simulation time
	command_line->RegisterInt64("--max-time <time> (default = 0)",
			m2s_max_time,
			"Maximum simulation time in seconds. The simulator "
			"will stop once this time is exceeded. A value of 0 "
			"(default) means no time limit.");
	
	// Trace file
	command_line->RegisterString("--trace <file>",
			m2s_trace_file,
			"Generate a trace file with debug information on the "
			"configuration of the modeled CPUs, GPUs, and memory "
			"system, as well as their dynamic simulation. The "
			"trace is a compressed plain-text file in format. The "
			"user should watch the size of the generated trace as "
			"simulation runs, since the trace file can quickly "
			"become extremely large.");
	
	// Visualization tool input file
	command_line->RegisterString("--visual <file>",
			m2s_visual_file,
			"Run the Multi2Sim Visualization Tool. This option "
			"consumes a file generated with the '--trace' option "
			"in a previous simulation. This option is only "
			"available on systems with support for GTK 3.0 or "
			"higher.");

	//
	// CUDA runtime options
	//

	// Category
	command_line->setCategory("CUDA", "CUDA Runtime Options");

	// Debug information
	command_line->RegisterString("--cuda-debug <file>",
			m2s_cuda_debug,
			"File used in the CUDA runtime to dump debug "
			"information related with CUDA ABI calls. This option "
			"is equivalent to environment variable M2S_CUDA_DEBUG.");

	
	//
	// OpenCL runtime options
	//

	// Category
	command_line->setCategory("OpenCL", "OpenCL Runtime Options");

	// Debug information
	command_line->RegisterString("--opencl-debug <file>",
			m2s_opencl_debug,
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
			"Possible values are: x86, southern-islands, "
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

	// CUDA runtime debug
	if (!m2s_cuda_debug.empty())
		environment->addVariable("M2S_CUDA_DEBUG", m2s_cuda_debug);

	// Call stack debugger
	if (!m2s_debug_callstack.empty())
		comm::CallStack::setDebugPath(m2s_debug_callstack);

	// Event-driven simulator debugger
	if (!m2s_debug_esim.empty())
		esim::Engine::setDebugPath(m2s_debug_esim);

	// Inifile debugger
	if (!m2s_debug_inifile.empty())
		misc::IniFile::setDebugPath(m2s_debug_inifile);

	// OpenCL runtime debug
	if (!m2s_opencl_debug.empty())
		environment->addVariable("M2S_OPENCL_DEBUG", m2s_opencl_debug);
	
	// OpenCL device list
	if (!m2s_opencl_devices.empty())
		environment->addVariable("M2S_OPENCL_DEVICES", m2s_opencl_devices);
	
	// OpenCL binary
	if (!m2s_opencl_binary.empty())
		environment->addVariable("M2S_OPENCL_BINARY", m2s_opencl_binary);

	// Go to event-driven simulation configuration if it's been passed.
	if (!m2s_esim_config.empty())
	{
		esim::Engine *esim = esim::Engine::getInstance();
		esim->TestLoop(m2s_esim_config);
		exit(0);
	}

	// Trace file
	if (!m2s_trace_file.empty())
	{
		esim::TraceSystem *trace_system = esim::TraceSystem::getInstance();
		trace_system->setPath(m2s_trace_file);
	}
}


void MainLoop()
{
	// Activate signal handler
	esim::Engine *esim = esim::Engine::getInstance();
	esim->EnableSignals();

	// Get singletons
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
			esim->Finish("ContextsFinished");

		// Count loop iterations, and check for limit in simulation time
		// only every 128k iterations. This avoids a constant overhead
		// of system calls.
		m2s_loop_iterations++;
		if (m2s_max_time > 0
				&& !(m2s_loop_iterations & ((1 << 17) - 1))
				&& esim->getRealTime() > m2s_max_time * 1000000)
			esim->Finish("MaxTime");
	}

	// Process all remaining events
	esim->ProcessAllEvents();

	// Restore default signal handlers
	esim->DisableSignals();
}


int MainProgram(int argc, char **argv)
{
	// Read command line
	RegisterOptions();
	HSA::Asm::RegisterOptions();
	HSA::Driver::RegisterOptions();
	HSA::Emu::RegisterOptions();
	Kepler::Asm::RegisterOptions();
	Kepler::Driver::RegisterOptions();
	Kepler::Emu::RegisterOptions();
	mem::MMU::RegisterOptions();
	mem::Manager::RegisterOptions();
	MIPS::Asm::RegisterOptions();
	MIPS::Emu::RegisterOptions();
	SI::Driver::RegisterOptions();
	x86::Asm::RegisterOptions();
	x86::Emu::RegisterOptions();
	x86::Timing::RegisterOptions();
	mem::System::RegisterOptions();
	dram::System::RegisterOptions();
	net::System::RegisterOptions();
	ARM::Asm::RegisterOptions();
	ARM::Emu::RegisterOptions();

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->Process(argc, argv, false);
	
	// Process command line
	ProcessOptions();
	HSA::Asm::ProcessOptions();
	HSA::Driver::ProcessOptions();
	HSA::Emu::ProcessOptions();
	Kepler::Asm::ProcessOptions();
	Kepler::Driver::ProcessOptions();
	Kepler::Emu::ProcessOptions();
	mem::MMU::ProcessOptions();
	mem::Manager::ProcessOptions();
	MIPS::Asm::ProcessOptions();
	MIPS::Emu::ProcessOptions();
	SI::Driver::ProcessOptions();
	x86::Asm::ProcessOptions();
	x86::Emu::ProcessOptions();
	x86::Timing::ProcessOptions();
	mem::System::ProcessOptions();
	dram::System::ProcessOptions();
	net::System::ProcessOptions();
	ARM::Asm::ProcessOptions();
	ARM::Emu::ProcessOptions();

	// Register architectures, runtimes, and drivers
	// FIXME - This initialization should be lazy, meaning that a particular
	// emulator, timing simulator, runtime, or driver should only be
	// initialized once we find that it is actually needed, leveraging
	// calls to getInstance() of singletons.
	RegisterRuntimes();
	RegisterDrivers();

	// Initialize memory system, only if there is at least one timing
	// simulation active. Check this in the architecture pool after all
	// '--xxx-sim' command-line options have been processed.
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
	if (arch_pool->getNumTiming())
	{
		mem::System *memory_system = mem::System::getInstance();
		memory_system->ReadConfiguration();
	}

	// Load programs
	LoadPrograms();
		
	// Main simulation loop
	MainLoop();

	// Success
	return 0;
}


int main(int argc, char **argv)
{
	// Main exception handler
	try
	{
		// Run main program
		return MainProgram(argc, argv);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		return 1;
	}

/*

	using namespace mem; //////////

	const std::string mem_config_0 =
			"[CacheGeometry geo-l1]\n"
			"Sets = 16\n"
			"Assoc = 2\n"
			"BlockSize = 64\n"
			"Latency = 2\n"
			"Policy = LRU\n"
			"Ports = 2\n"
			"\n"
			"[CacheGeometry geo-l2]\n"
			"Sets = 4\n"
			"Assoc = 4\n"
			"BlockSize = 128\n"
			"Latency = 20\n"
			"Policy = LRU\n"
			"Ports = 4\n"
			"\n"
			"; 4 l1 cpu caches\n"
			"\n"
			"[Module mod-l1-0]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net0\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-l2-0\n"
			"\n"
			"[Module mod-l1-1]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net0\n"
			"LowNetworkNode = n1\n"
			"LowModules = mod-l2-0\n"
			"\n"
			"[Module mod-l1-2]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net1\n"
			"LowNetworkNode = n3\n"
			"LowModules = mod-l2-1\n"
			"\n"
			"[Module mod-l1-3]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net1\n"
			"LowNetworkNode = n4\n"
			"LowModules = mod-l2-1\n"
			"\n"
			"; 1 l2 cpu cache\n"
			"\n"
			"[Module mod-l2-0]\n"
			"Type = Cache\n"
			"Geometry = geo-l2\n"
			"HighNetwork = net0\n"
			"HighNetworkNode = n2\n"
			"LowNetwork = net2\n"
			"LowNetworkNode = n6\n"
			"LowModules = mod-mm\n"
			"\n"
			"[Module mod-l2-1]\n"
			"Type = Cache\n"
			"Geometry = geo-l2\n"
			"HighNetwork = net1\n"
			"HighNetworkNode = n5\n"
			"LowNetwork = net2\n"
			"LowNetworkNode = n7\n"
			"LowModules = mod-mm\n"
			"\n"
			"; 1 mm\n"
			"\n"
			"[Module mod-mm]\n"
			"Type = MainMemory\n"
			"BlockSize = 128\n"
			"Latency = 200\n"
			"HighNetwork = net2\n"
			"HighNetworkNode = n8\n"
			"\n"
			"\n"
			"[Entry core-0]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"DataModule = mod-l1-0\n"
			"InstModule = mod-l1-0\n"
			"\n"
			"[Entry core-1]\n"
			"Arch = x86\n"
			"Core = 1\n"
			"Thread = 0\n"
			"DataModule = mod-l1-1\n"
			"InstModule = mod-l1-1\n"
			"\n"
			"[Entry core-2]\n"
			"Arch = x86\n"
			"Core = 2\n"
			"Thread = 0\n"
			"DataModule = mod-l1-2\n"
			"InstModule = mod-l1-2\n"
			"\n"
			"[Entry core-3]\n"
			"Arch = x86\n"
			"Core = 3\n"
			"Thread = 0\n"
			"DataModule = mod-l1-3\n"
			"InstModule = mod-l1-3\n";

	const std::string mem_config_1 =
			"[ CacheGeometry geo-l1 ]\n"
			"Sets = 4\n"
			"Assoc = 1\n"
			"BlockSize = 64\n"
			"Latency = 1\n"
			"Policy = LRU\n"
			"Ports = 2\n"
			"\n"
			"[ CacheGeometry geo-l2 ]\n"
			"Sets = 8\n"
			"Assoc = 1\n"
			"BlockSize = 64\n"
			"Latency = 2\n"
			"Policy = LRU\n"
			"Ports = 4\n"
			"\n"
			"[ CacheGeometry geo-l3 ]\n"
			"Sets = 16\n"
			"Assoc = 1\n"
			"BlockSize = 64\n"
			"Latency = 4\n"
			"Policy = LRU\n"
			"Ports = 8\n"
			"\n"
			"\n"
			"; L1 Caches\n"
			"\n"
			"[ Module mod-l1-0 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net-0\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-l2-0\n"
			"\n"
			"[ Module mod-l1-1 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net-0\n"
			"LowNetworkNode = n1\n"
			"LowModules = mod-l2-0\n"
			"\n"
			"[ Module mod-l1-2 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net-1\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-l2-1\n"
			"\n"
			"[ Module mod-l1-3 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net-1\n"
			"LowNetworkNode = n1\n"
			"LowModules = mod-l2-1\n"
			"\n"
			"\n"
			"; L2 Caches\n"
			"\n"
			"[ Module mod-l2-0 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l2\n"
			"HighNetwork = net-0\n"
			"HighNetworkNode = n2\n"
			"LowNetwork = net-2\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-l3\n"
			"\n"
			"[ Module mod-l2-1 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l2\n"
			"HighNetwork = net-1\n"
			"HighNetworkNode = n2\n"
			"LowNetwork = net-2\n"
			"LowNetworkNode = n1\n"
			"LowModules = mod-l3\n"
			"\n"
			"\n"
			"; L3 Cache\n"
			"\n"
			"[ Module mod-l3 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l3\n"
			"HighNetwork = net-2\n"
			"HighNetworkNode = n2\n"
			"LowNetwork = net-3\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-mm\n"
			"\n"
			"\n"
			"; Main Memory\n"
			"\n"
			"[ Module mod-mm ]\n"
			"Type = MainMemory\n"
			"BlockSize = 64\n"
			"DirectorySize = 32\n"
			"DirectoryAssoc = 1\n"
			"Latency = 8\n"
			"HighNetwork = net-3\n"
			"HighNetworkNode = n1\n"
			"\n"
			"\n"
			"; Cores\n"
			"\n"
			"[ Entry core-0 ]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"Module = mod-l1-0\n"
			"\n"
			"[ Entry core-1 ]\n"
			"Arch = x86\n"
			"Core = 1\n"
			"Thread = 0\n"
			"Module = mod-l1-1\n"
			"\n"
			"[ Entry core-2 ]\n"
			"Arch = x86\n"
			"Core = 2\n"
			"Thread = 0\n"
			"Module = mod-l1-2\n"
			"\n"
			"[ Entry core-3 ]\n"
			"Arch = x86\n"
			"Core = 3\n"
			"Thread = 0\n"
			"Module = mod-l1-3\n";

	const std::string x86_config =
			"[ General ]\n"
			"Cores = 4\n"
			"Threads = 1\n";

	const std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 1024\n"
			"DefaultOutputBufferSize = 1024\n"
			"DefaultBandwidth = 256\n"
			"\n"
			"[ Network.net1 ]\n"
			"DefaultInputBufferSize = 1024\n"
			"DefaultOutputBufferSize = 1024\n"
			"DefaultBandwidth = 256\n"
			"\n"
			"[ Network.net2 ]\n"
			"DefaultInputBufferSize = 1024\n"
			"DefaultOutputBufferSize = 1024\n"
			"DefaultBandwidth = 256\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n2 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net1.Node.n3 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net1.Node.n4 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net1.Node.n5 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net2.Node.n6 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net2.Node.n7 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net2.Node.n8 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch \n"
			"\n"
			"[ Network.net1.Node.s1 ]\n"
			"Type = Switch \n"
			"\n"
			"[ Network.net2.Node.s2 ]\n"
			"Type = Switch \n"
			"\n"
			"[ Network.net0.Link.l0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.l1 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.l2 ]\n"
			"Type = Bidirectional\n"
			"Source = n2 \n"
			"Dest = s0\n"
			"\n"
			"[ Network.net1.Link.l3 ]\n"
			"Type = Bidirectional\n"
			"Source = n3 \n"
			"Dest = s1\n"
			"\n"
			"[ Network.net1.Link.l4 ]\n"
			"Type = Bidirectional\n"
			"Source = n4 \n"
			"Dest = s1\n"
			"\n"
			"[ Network.net1.Link.l5 ]\n"
			"Type = Bidirectional\n"
			"Source = n5 \n"
			"Dest = s1\n"
			"\n"
			"[ Network.net2.Link.l6 ]\n"
			"Type = Bidirectional\n"
			"Source = n6 \n"
			"Dest = s2\n"
			"\n"
			"[ Network.net2.Link.l7 ]\n"
			"Type = Bidirectional\n"
			"Source = n7 \n"
			"Dest = s2\n"
			"\n"
			"[ Network.net2.Link.l8 ]\n"
			"Type = Bidirectional\n"
			"Source = n8 \n"
			"Dest = s2\n";

	// Load configuration files
	misc::IniFile ini_file_mem;
	misc::IniFile ini_file_x86;
	misc::IniFile ini_file_net;
	ini_file_mem.LoadFromString(mem_config_0);
	ini_file_x86.LoadFromString(x86_config);
	ini_file_net.LoadFromString(net_config);

	// Set up x86 timing simulator
	x86::Timing::ParseConfiguration(&ini_file_x86);
	x86::Timing::getInstance();

	// Set up network system
	net::System *network_system = net::System::getInstance();
	network_system->ParseConfiguration(&ini_file_net);

	// Set up memory system
	System *memory_system = System::getInstance();
	memory_system->ReadConfiguration(&ini_file_mem);

	// Get modules
	Module *module_l1_0 = memory_system->getModule("mod-l1-0");
	Module *module_l1_1 = memory_system->getModule("mod-l1-1");
	Module *module_l2_0 = memory_system->getModule("mod-l2-0");
	Module *module_mm = memory_system->getModule("mod-mm");

	// Set block states
	module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockModified, 0x0);
	module_l1_1->getCache()->getBlock(1, 0)->setStateTag(Cache::BlockModified, 0x40);
	module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
	module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
	module_l2_0->setOwner(0, 0, 0, module_l1_0);
	module_l2_0->setOwner(0, 0, 1, module_l1_1);
	module_l2_0->setSharer(0, 0, 0, module_l1_0);
	module_l2_0->setSharer(0, 0, 1, module_l1_1);
	module_mm->setOwner(0, 0, 0, module_l2_0);
	module_mm->setSharer(0, 0, 0, module_l2_0);

	// Accesses
	int witness = -2;
	module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
	module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

	// Simulation loop
	esim::Engine::setDebugPath("stdout");/////////
	esim::Engine *esim_engine = esim::Engine::getInstance();
	while (witness < 0)
		esim_engine->ProcessEvents();
	*/
}

