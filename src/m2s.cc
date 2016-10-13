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
#include <sys/time.h>

#include <arch/common/CallStack.h>
#include <arch/common/Driver.h>
#include <arch/common/Runtime.h>
#include <arch/kepler/disassembler/Disassembler.h>
#include <arch/kepler/driver/Driver.h>
#include <arch/kepler/emulator/Emulator.h>
#include <arch/mips/disassembler/Disassembler.h>
#include <arch/mips/emulator/Context.h>
#include <arch/mips/emulator/Emulator.h>
#include <arch/x86/disassembler/Disassembler.h>
#include <arch/x86/emulator/Context.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Signal.h>
#include <arch/x86/timing/Timing.h>
#include <arch/hsa/disassembler/Disassembler.h>
#include <arch/hsa/driver/Driver.h>
#include <arch/hsa/emulator/Emulator.h>
#include <arch/southern-islands/driver/Driver.h>
#include <arch/southern-islands/emulator/Emulator.h>
#include <arch/southern-islands/timing/Timing.h>
#include <arch/arm/disassembler/Disassembler.h>
#include <arch/arm/emulator/Emulator.h>
#include <dram/System.h>
#include <memory/Mmu.h>
#include <memory/Manager.h>
#include <memory/System.h>
#include <network/System.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Environment.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/Terminal.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

extern "C"
{
#include <visual/common/visual.h>
}
//
// Configuration options
//

// Context configuration file
std::string m2s_context_config;

// Debug information in CUDA runtime
std::string m2s_cuda_debug;

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


void WelcomeMessage(std::ostream &os)
{
	// Compute simulation ID
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned min_id = misc::StringAlnumToInt("10000");
	unsigned max_id = misc::StringAlnumToInt("ZZZZZ");
	unsigned id = (tv.tv_sec * 1000000000 + tv.tv_usec)
			% (max_id - min_id + 1)
			+ min_id;
	std::string alnum_id = misc::StringIntToAlnum(id);
	
	// Blue color
	misc::Terminal::Blue(os);

	// Print welcome message in the standard error output
	os << '\n' << "; Multi2Sim " << VERSION << " - ";
	os << "A Simulation Framework for CPU-GPU Heterogeneous Computing\n";
	os << "; Please use command 'm2s --help' for a list of command-line options.\n";
	os << "; Simulation alpha-numeric ID: " << alnum_id << '\n';
	os << '\n';
	
	// Reset terminal color
	misc::Terminal::Reset(os);
}


// Load a program from the command line
void LoadProgram(const std::vector<std::string> &arguments,
		const std::vector<std::string> &environment = {},
		const std::string &current_directory = "",
		const std::string &stdin_file_name = "",
		const std::string &stdout_file_name = "")
{
	// Skip if no program specified
	if (arguments.size() == 0)
		return;
	
	// Choose emulator based on ELF header
	std::string exe = misc::getFullPath(arguments[0], current_directory);
	ELFReader::File elf_file(exe, false);
	comm::Emulator *emulator;
	switch (elf_file.getMachine())
	{
	case EM_386:

		emulator = x86::Emulator::getInstance();
		break;

	case EM_ARM:

		emulator = ARM::Emulator::getInstance();
		break;

	case EM_MIPS:

		emulator = MIPS::Emulator::getInstance();
		break;

	default:

		throw misc::Error(misc::fmt("%s: unsupported ELF architecture",
				exe.c_str()));
	}

	// Load the program in selected emulator
	emulator->LoadProgram(arguments,
			environment,
			current_directory,
			stdin_file_name,
			stdout_file_name);
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
		std::string current_directory = ini_file.ReadString(section, "Cwd");
		std::string stdin_file_name = ini_file.ReadString(section, "Stdin");
		std::string stdout_file_name = ini_file.ReadString(section, "Stdout");
		
		// Arguments
		std::vector<std::string> arguments;
		std::string args_str = ini_file.ReadString(section, "Args");
		misc::StringTokenize(args_str, arguments);
		arguments.insert(arguments.begin(), exe);

		// Environment variables
		std::vector<std::string> environment;
		std::string env_str = ini_file.ReadString(section, "Env");
		misc::Environment::getFromString(env_str, environment);

		// Load program
		LoadProgram(arguments,
				environment,
				current_directory,
				stdin_file_name,
				stdout_file_name);
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

	// Debugger for call stack
	command_line->RegisterString("--call-debug <file>",
			m2s_debug_callstack,
			"Dump debug information related with the CPU context "
			"call stacks, including function invocations and "
			"returns.");

	// Context configuration
	command_line->RegisterString("--ctx-config <file>",
			m2s_context_config,
			"Use <file> as the context configuration file. This "
			"file describes the initial set of running applications, "
			"their arguments, and environment variables. Use option "
			"--ctx-config-help for a description of the context "
			"configuration file format.");
	
	// Debugger for event-driven simulator
	command_line->RegisterString("--esim-debug <file>",
			m2s_debug_esim,
			"Dump debug information related with the event-driven "
			"simulation engine.");
	
	// Debugger for Inifile parser
	command_line->RegisterString("--inifile-debug <file>",
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

	// Trace file
	if (!m2s_trace_file.empty())
	{
		esim::TraceSystem *trace_system = esim::TraceSystem::getInstance();
		trace_system->setPath(m2s_trace_file);
	}

	// Visualization
	if (!m2s_visual_file.empty())
		visual_run(m2s_visual_file.c_str());
		
}


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
		int num_active_emulators;
		int num_active_timing_simulators;
		arch_pool->Run(num_active_emulators,
				num_active_timing_simulators);

		// Event-driven simulation. Only process events and advance to
		// next global simulation cycle if any architecture performed a
		// useful timing simulation.
		if (num_active_timing_simulators)
			esim->ProcessEvents();

		// If neither functional nor timing simulation was performed for
		// any architecture, it means that all guest contexts finished
		// execution - simulation can end.
		if (!num_active_emulators && !num_active_timing_simulators)
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


void DumpStatisticsSummary(std::ostream &os = std::cerr)
{
	// No summary dumped if no simulation was run
	if (m2s_loop_iterations < 2)
		return;
	
	// Print in blue
	misc::Terminal::Blue(os);

	// Header
	os << '\n' << ";\n"
			<< "; Simulation Statistics Summary\n"
			<< ";\n"
			<< "\n";

	// Calculate real time in seconds
	esim::Engine *esim_engine = esim::Engine::getInstance();
	double time_in_seconds = (double) esim_engine->getRealTime() / 1.0e6;

	// General statistics
	os << "[ General ]\n";
	os << misc::fmt("RealTime = %.2f [s]\n", time_in_seconds);
	os << "SimEnd = " << esim_engine->getFinishReason() << '\n';

	// General detailed simulation statistics
	if (esim_engine->getTime())
	{
		long long cycles = esim_engine->getCycle();
		os << misc::fmt("SimTime = %.2f [ns]\n", esim_engine->getTime() / 1000.0);
		os << misc::fmt("Frequency = %d [MHz]\n", esim_engine->getFrequency());
		os << misc::fmt("Cycles = %lld\n", cycles);
	}

	// End
	os << '\n';

	// Summary for all architectures
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
	arch_pool->DumpSummary();

	// Reset terminal color
	misc::Terminal::Reset(os);
}

void DumpReports()
{
	// Reports for all architectures
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
	arch_pool->DumpReports();

	// Dumping memory report
	if (mem::System::hasInstance())
	{
		mem::System *mem_system = mem::System::getInstance();

		// Dump the memory report
		mem_system->DumpReport();
	}

	// Dumping network report
	if (net::System::hasInstance())
	{
		// Dump the network report
		net::System *net_system = net::System::getInstance();
		net_system->DumpReport();

		// Dump the network data for static visualization
		net_system->StaticGraph();

		// Dump the network routing table
		net_system->DumpRoutes();
	}
}


int MainProgram(int argc, char **argv)
{
	// Print welcome message in standard error output
	WelcomeMessage(std::cerr);

	// Read command line
	RegisterOptions();
	HSA::Disassembler::RegisterOptions();
	HSA::Driver::RegisterOptions();
	HSA::Emulator::RegisterOptions();
	Kepler::Disassembler::RegisterOptions();
	Kepler::Driver::RegisterOptions();
	Kepler::Emulator::RegisterOptions();
	mem::Mmu::RegisterOptions();
	mem::Manager::RegisterOptions();
	MIPS::Disassembler::RegisterOptions();
	MIPS::Emulator::RegisterOptions();
	SI::Driver::RegisterOptions();
	SI::Disassembler::RegisterOptions();
	SI::Emulator::RegisterOptions();
	SI::Timing::RegisterOptions();
	x86::Disassembler::RegisterOptions();
	x86::Emulator::RegisterOptions();
	x86::Timing::RegisterOptions();
	mem::System::RegisterOptions();
	dram::System::RegisterOptions();
	net::System::RegisterOptions();
	ARM::Disassembler::RegisterOptions();
	ARM::Emulator::RegisterOptions();

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->Process(argc, argv, false);
	
	// Process command line
	ProcessOptions();
	HSA::Disassembler::ProcessOptions();
	HSA::Driver::ProcessOptions();
	HSA::Emulator::ProcessOptions();
	Kepler::Disassembler::ProcessOptions();
	Kepler::Driver::ProcessOptions();
	Kepler::Emulator::ProcessOptions();
	mem::Mmu::ProcessOptions();
	mem::Manager::ProcessOptions();
	MIPS::Disassembler::ProcessOptions();
	MIPS::Emulator::ProcessOptions();
	SI::Driver::ProcessOptions();
	SI::Disassembler::ProcessOptions();
	SI::Emulator::ProcessOptions();
	SI::Timing::ProcessOptions();
	x86::Disassembler::ProcessOptions();
	x86::Emulator::ProcessOptions();
	x86::Timing::ProcessOptions();
	mem::System::ProcessOptions();
	dram::System::ProcessOptions();
	net::System::ProcessOptions();
	ARM::Disassembler::ProcessOptions();
	ARM::Emulator::ProcessOptions();

	// Initialize memory system, only if there is at least one timing
	// simulation active. Check this in the architecture pool after all
	// '--xxx-sim' command-line options have been processed.
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();

	// Throw an error if we have two instances of simulation options. For
	// example network stand alone simulation is invoked with timing
	// simulations
	if (arch_pool->getNumTiming())
	{
		if (net::System::isStandAlone() || dram::System::isStandAlone())
			throw misc::Error("Cannot have both "
						"stand-alone and detailed "
						"simulation active at the same "
						"time");
	}
	else
	{
		if (net::System::isStandAlone() && dram::System::isStandAlone())
			throw misc::Error("Cannot have both "
						"stand-alone and detailed "
						"simulation active at the same "
						"time");
	}
			
	if (arch_pool->getNumTiming())
	{
		// We need to load the network configuration file prior to
		// parsing memory config file. The memory config file searches
		// for the external network if it cannot find the network
		// in the memory configuration file sections.
		net::System *net_system = net::System::getInstance();
		net_system->ReadConfiguration();

		// Parse the memory configuration file
		mem::System *memory_system = mem::System::getInstance();
		memory_system->ReadConfiguration();
	}

	// Initialize network system, only if the option --net-sim is used
	if (net::System::isStandAlone())
	{
		net::System *net_system = net::System::getInstance();
		net_system->ReadConfiguration();
		net_system->StandAlone();
	}

	// Initialize dram system, only if the option --dram-sim is used
	if (dram::System::isStandAlone())
	{
		dram::System *dram_system = dram::System::getInstance();
		dram_system->ReadConfiguration();
		dram_system->Run();
	}

	// Register drivers and runtimes
	RegisterDrivers();
	RegisterRuntimes();

	// Load programs
	LoadPrograms();
		
	// Main simulation loop
	MainLoop();

	// Statistics summary
	DumpStatisticsSummary();

	// Reports
	DumpReports();

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
}

