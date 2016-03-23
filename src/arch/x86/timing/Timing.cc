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

#include <arch/common/Arch.h>
#include <memory/System.h>

#include "Alu.h"
#include "Timing.h"


namespace x86
{

std::unique_ptr<Timing> Timing::instance;

esim::Trace Timing::trace;

const int Timing::trace_version_major = 1;
const int Timing::trace_version_minor = 671;


//
// Configuration options
//

std::string Timing::config_file;

// Simulation kind
comm::Arch::SimKind Timing::sim_kind = comm::Arch::SimFunctional;

// Report file name
std::string Timing::report_file;

// Message to display with '--x86-help'
const std::string Timing::help_message =
		"The x86 Cpu configuration file is a plain text INI file, defining\n"
		"the parameters of the Cpu model used for a detailed (architectural) simulation.\n"
		"This configuration file is passed to Multi2Sim with option '--x86-config <file>,\n"
		"which must be accompanied by option '--x86-sim detailed'.\n"
		"\n"
		"The following is a list of the sections allowed in the Cpu configuration file,\n"
		"along with the list of variables for each section.\n"
		"\n"
		"Section '[ General ]':\n"
		"\n"
		"  Frequency = <freq> (Default = 1000 MHz)\n"
		"      Frequency in MHz for the x86 Cpu. Value between 1 and 10K.\n"
		"  Cores = <num_cores> (Default = 1)\n"
		"      Number of cores.\n"
		"  Threads = <num_threads> (Default = 1)\n"
		"      Number of hardware threads per core. The total number of computing nodes\n"
		"      in the Cpu model is equals to Cores * Threads.\n"
		"  FastForward = <num_inst> (Default = 0)\n"
		"      Number of x86 instructions to run with a fast functional simulation before\n"
		"      the architectural simulation starts.\n"
		"  ContextQuantum = <cycles> (Default = 100k)\n"
		"      If ContextSwitch is true, maximum number of cycles that a context can occupy\n"
		"      a Cpu hardware thread before it is replaced by other pending context.\n"
		"  RecoverKind = {Writeback|Commit} (Default = Writeback)\n"
		"      On branch misprediction, stage in the execution of the mispredicted branch\n"
		"      when processor recovery is triggered.\n"
		"  RecoverPenalty = <cycles> (Default = 0)\n"
		"      Number of cycles that the fetch stage gets stalled after a branch\n"
		"      misprediction.\n"
		"  PageSize = <size> (Default = 4kB)\n"
		"      Memory page size in bytes.\n"
		"  DataCachePerfect = {t|f} (Default = False)\n"
		"  ProcessPrefetchHints = {t|f} (Default = True)\n"
		"      If specified as false, the cpu will ignore any prefetch hints/instructions.\n"
		"  PrefetchHistorySize = <size> (Default = 10)\n"
		"      Number of past prefetches to keep track of, so as to avoid redundant prefetches\n"
		"      from being issued from the cpu to the cache module.\n"
		"  InstructionCachePerfect = {t|f} (Default = False)\n"
		"      Set these options to true to simulate a perfect data/instruction caches,\n"
		"      respectively, where every access results in a hit. If set to false, the\n"
		"      parameters of the caches are given in the memory configuration file\n"
		"  UseNCStore = {t|f} (Default = False)\n"
		"      Normally the Cpu uses cohnerency-enabled Store commands.  Setting this to True\n"
		"      causes the Cpu to issue NCStore commands to reduce protocol overhead.\n"
		"      When True, Cores and Threads must be set to 1.\n"
		"\n"
		"Section '[ Pipeline ]':\n"
		"\n"
		"  FetchKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
		"      Policy for fetching instruction from different threads. A shared fetch stage\n"
		"      fetches instructions from different threads in the same cycle; a time-slice\n"
		"      fetch switches between threads in a round-robin fashion.\n"
		"  DecodeWidth = <num_inst> (Default = 4)\n"
		"      Number of x86 instructions decoded per cycle.\n"
		"  DispatchKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
		"      Policy for dispatching instructions from different threads. If shared,\n"
		"      instructions from different threads are dispatched in the same cycle. Otherwise,\n"
		"      instruction dispatching is done in a round-robin fashion at a cycle granularity.\n"
		"  DispatchWidth = <num_inst> (Default = 4)\n"
		"      Number of microinstructions dispatched per cycle.\n"
		"  IssueKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
		"      Policy for issuing instructions from different threads. If shared, instructions\n"
		"      from different threads are issued in the same cycle; otherwise, instruction issue\n"
		"      is done round-robin at a cycle granularity.\n"
		"  IssueWidth = <num_inst> (Default = 4)\n"
		"      Number of microinstructions issued per cycle.\n"
		"  CommitKind = {Shared|TimeSlice} (Default = Shared)\n"
		"      Policy for committing instructions from different threads. If shared,\n"
		"      instructions from different threads are committed in the same cycle; otherwise,\n"
		"      they commit in a round-robin fashion.\n"
		"  CommitWidth = <num_inst> (Default = 4)\n"
		"      Number of microinstructions committed per cycle.\n"
		"  OccupancyStats = {t|f} (Default = False)\n"
		"      Calculate structures occupancy statistics. Since this computation requires\n"
		"      additional overhead, the option needs to be enabled explicitly. These statistics\n"
		"      will be attached to the Cpu report.\n"
		"\n"
		"Section '[ Queues ]':\n"
		"\n"
		"  FetchQueueSize = <bytes> (Default = 64)\n"
		"      Size of the fetch queue given in bytes.\n"
		"  UopQueueSize = <num_uops> (Default = 32)\n"
		"      Size of the uop queue size, given in number of uops.\n"
		"  RobKind = {Private|Shared} (Default = Private)\n"
		"      Reorder buffer sharing among hardware threads.\n"
		"  RobSize = <num_uops> (Default = 64)\n"
		"      Reorder buffer size in number of microinstructions (if private, per-thread size).\n"
		"  IqKind = {Private|Shared} (Default = Private)\n"
		"      Instruction queue sharing among threads.\n"
		"  IqSize = <num_uops> (Default = 40)\n"
		"      Instruction queue size in number of uops (if private, per-thread IQ size).\n"
		"  LsqKind = {Private|Shared} (Default = 20)\n"
		"      Load-store queue sharing among threads.\n"
		"  LsqSize = <num_uops> (Default = 20)\n"
		"      Load-store queue size in number of uops (if private, per-thread LSQ size).\n"
		"  RfKind = {Private|Shared} (Default = Private)\n"
		"      Register file sharing among threads.\n"
		"  RfIntSize = <entries> (Default = 80)\n"
		"      Number of integer physical register (if private, per-thread).\n"
		"  RfFpSize = <entries> (Default = 40)\n"
		"      Number of floating-point physical registers (if private, per-thread).\n"
		"  RfXmmSize = <entries> (Default = 40)\n"
		"      Number of XMM physical registers (if private, per-thread).\n"
		"\n"
		"Section '[ TraceCache ]':\n"
		"\n"
		"  Present = {t|f} (Default = False)\n"
		"      If true, a trace cache is included in the model. If false, the rest of the\n"
		"      options in this section are ignored.\n"
		"  Sets = <num_sets> (Default = 64)\n"
		"      Number of sets in the trace cache.\n"
		"  Assoc = <num_ways> (Default = 4)\n"
		"      Associativity of the trace cache. The product Sets * Assoc is the total\n"
		"      number of traces that can be stored in the trace cache.\n"
		"  TraceSize = <num_uops> (Default = 16)\n"
		"      Maximum size of a trace of uops.\n"
		"  BranchMax = <num_branches> (Default = 3)\n"
		"      Maximum number of branches contained in a trace.\n"
		"  QueueSize = <num_uops> (Default = 32)\n"
		"      Size of the trace queue size in uops.\n"
		"\n"
		"Section '[ FunctionalUnits ]':\n"
		"\n"
		"  The possible variables in this section follow the format\n"
		"      <func_unit>.<field> = <value>\n"
		"  where <func_unit> refers to a functional unit type, and <field> refers to a\n"
		"  property of it. Possible values for <func_unit> are:\n"
		"\n"
		"      IntAdd      Integer adder\n"
		"      IntMult     Integer multiplier\n"
		"      IntDiv      Integer divider\n"
		"\n"
		"      EffAddr     Operator for effective address computations\n"
		"      Logic       Operator for logic operations\n"
		"\n"
		"      FloatSimple    Simple floating-point operations\n"
		"      FloatAdd       Floating-point adder\n"
		"      FloatComp      Floating-point comparator\n"
		"      FloatMult      Floating-point multiplier\n"
		"      FloatDiv       Floating-point divider\n"
		"      FloatComplex   Operator for complex floating-point computations\n"
		"\n"
		"      XMMIntAdd      XMM integer adder\n"
		"      XMMIntMult     XMM integer multiplier\n"
		"      XMMIntDiv      XMM integer Divider\n"
		"\n"
		"      XMMLogic       XMM logic operations\n"
		"\n"
		"      XMMFloatAdd       XMM floating-point adder\n"
		"      XMMFloatComp      XMM floating-point comparator\n"
		"      XMMFloatMult      XMM floating-point multiplier\n"
		"      XMMFloatDiv       XMM floating-point divider\n"
		"      XMMFloatConv      XMM floating-point converter\n"
		"      XMMFloatComplex   Complex XMM floating-point operations\n"
		"\n"
		"  Possible values for <field> are:\n"
		"      Count       Number of functional units of a given kind.\n"
		"      OpLat       Latency of the operator.\n"
		"      IssueLat    Latency since an instruction was issued until the functional\n"
		"                  unit is available for the next use. For pipelined operators,\n"
		"                  IssueLat is smaller than OpLat.\n"
		"\n"
		"Section '[ BranchPredictor ]':\n"
		"\n"
		"  Kind = {Perfect|Taken|NotTaken|Bimodal|TwoLevel|Combined} (Default = TwoLevel)\n"
		"      Branch predictor type.\n"
		"  BTB.Sets = <num_sets> (Default = 256)\n"
		"      Number of sets in the BTB.\n"
		"  BTB.Assoc = <num_ways) (Default = 4)\n"
		"      BTB associativity.\n"
		"  Bimod.Size = <entries> (Default = 1024)\n"
		"      Number of entries of the bimodal branch predictor.\n"
		"  Choice.Size = <entries> (Default = 1024)\n"
		"      Number of entries for the choice predictor.\n"
		"  RAS.Size = <entries> (Default = 32)\n"
		"      Number of entries of the return address stack (RAS).\n"
		"  TwoLevel.L1Size = <entries> (Default = 1)\n"
		"      For the two-level adaptive predictor, level 1 size.\n"
		"  TwoLevel.L2Size = <entries> (Default = 1024)\n"
		"      For the two-level adaptive predictor, level 2 size.\n"
		"  TwoLevel.HistorySize = <size> (Default = 8)\n"
		"      For the two-level adaptive predictor, level 2 history size.\n"
		"\n";

const char *Timing::error_fast_forward =
	"The number of instructions specified in the x86 CPU configuration file "
	"for fast-forward (functional) execution has caused all contexts to end "
	"before the timing simulation could start. Please decrease the number "
	"of fast-forward instructions and retry.\n";

bool Timing::help = false;

int Timing::frequency = 1000;


Timing::Timing() : comm::Timing("x86")
{
	// Configure frequency domain with the frequency given by the user
	ConfigureFrequencyDomain(frequency);

	// Create CPU
	cpu = misc::new_unique<Cpu>(this);

	// Create the trace header related to CPU
	trace.Header(misc::fmt("x86.init version=\"%d.%d\" "
			"num_cores=%d num_threads=%d\n",
			trace_version_major, trace_version_minor,
			cpu->getNumCores(), cpu->getNumThreads()));
}


Timing *Timing::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance = misc::new_unique<Timing>();
	return instance.get();
}


bool Timing::Run()
{
	// Stop if there are no more contexts left
	Emulator *emulator = Emulator::getInstance();
	assert(emulator->getNumFinishedContexts() <= emulator->getNumContexts());
	if (emulator->getNumContexts() == 0)
		return false;

	// Fast-forward simulation
	if (Cpu::getNumFastForwardInstructions()
			&& emulator->getNumInstructions()
			< Cpu::getNumFastForwardInstructions())
		FastForward();

	// Stop if maximum number of CPU instructions exceeded
	esim::Engine *esim_engine = esim::Engine::getInstance();
	if (Emulator::getMaxInstructions()
			&& cpu->getNumCommittedInstructions()
			>= Emulator::getMaxInstructions()
			- Cpu::getNumFastForwardInstructions())
		esim_engine->Finish("X86MaxInstructions");

	// Stop if maximum number of cycles exceeded
	if (Cpu::max_cycles && getCycle() >= Cpu::max_cycles)
		esim_engine->Finish("X86MaxCycles");

	// Stop if any previous reason met
	if (esim_engine->hasFinished())
		return true;

	// Empty uop trace list. This dumps the last trace line for instructions
	// that were freed in the previous simulation cycle.
	cpu->EmptyTraceList();

	// Run processor stages
	cpu->Run();

	// Process host threads generating events
	emulator->ProcessEvents();

	// Still simulating
	return true;
}


void Timing::FastForward()
{
	// Fast-forward simulation
	Emulator *emulator = Emulator::getInstance();
	esim::Engine *esim_engine = esim::Engine::getInstance();
	while (emulator->getNumInstructions()
			< Cpu::getNumFastForwardInstructions()
			&& !esim_engine->hasFinished())
		emulator->Run();

	// Output warning if simulation finished during fast-forward execution
	if (esim_engine->hasFinished())
		misc::Warning("x86 fast-forwarding finished simulation.\n%s",
				Timing::error_fast_forward);
}


void Timing::WriteMemoryConfiguration(misc::IniFile *ini_file)
{
	// Cache geometry for L1
	std::string section = "CacheGeometry x86-geo-l1";
	ini_file->WriteInt(section, "Sets", 16);
	ini_file->WriteInt(section, "Assoc", 2);
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 1);
	ini_file->WriteString(section, "Policy", "LRU");

	// Cache geometry for L2
	section = "CacheGeometry x86-geo-l2";
	ini_file->WriteInt(section, "Sets", 64);
	ini_file->WriteInt(section, "Assoc", 4);
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 10);
	ini_file->WriteString(section, "Policy", "LRU");

	// L1 caches and entries
	for (int i = 0; i < Cpu::getNumCores(); i++)
	{
		// L1 cache
		std::string module_name = misc::fmt("x86-l1-%d", i);
		section = "Module " + module_name;
		ini_file->WriteString(section, "Type", "Cache");
		ini_file->WriteString(section, "Geometry", "x86-geo-l1");
		ini_file->WriteString(section, "LowNetwork", "x86-net-l1-l2");
		ini_file->WriteString(section, "LowModules", "x86-l2");

		// Entry
		for (int j = 0; j < Cpu::getNumThreads(); j++)
		{
			section = misc::fmt("Entry x86-core-%d-thread-%d", i, j);
			ini_file->WriteString(section, "Arch", "x86");
			ini_file->WriteInt(section, "Core", i);
			ini_file->WriteInt(section, "Thread", j);
			ini_file->WriteString(section, "Module", module_name);
		}
	}

	// L2 cache
	section = "Module x86-l2";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "x86-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "x86-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "x86-net-l2-mm");
	ini_file->WriteString(section, "LowModules", "x86-mm");

	// Main memory
	section = "Module x86-mm";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "x86-net-l2-mm");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);

	// Network connecting L1 caches and L2
	section = "Network x86-net-l1-l2";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 144);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 144);
	ini_file->WriteInt(section, "DefaultBandwidth", 72);

	// Network connecting L2 cache and global memory
	section = "Network x86-net-l2-mm";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);
}


void Timing::ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
		const std::string &section)
{
	// Allow these sections in case we quit before reading them.
	ini_file->Allow(section, "DataModule");
	ini_file->Allow(section, "InstModule");
	ini_file->Allow(section, "Module");

	// Check right presence of sections
	bool unified_present = ini_file->Exists(section, "Module");
	bool data_inst_present = ini_file->Exists(section, "DataModule") &&
			ini_file->Exists(section, "InstModule");
	if (!(unified_present ^ data_inst_present))
		throw Error(misc::fmt("%s: section [%s]: invalid combination of "
				"modules. An x86 entry to the memory hierarchy "
				"needs to specify either a unified entry for "
				"data and instructions (variable 'Module'), or "
				"two separate entries for data and instructions "
				"(variables 'DataModule' and 'InstModule'), "
				"but not both.\n",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Read core
	int core_index = ini_file->ReadInt(section, "Core", -1);
	if (core_index < 0)
		throw Error(misc::fmt("%s: section [%s]: invalid or missing "
				"value for 'Core'",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Read thread
	int thread_index = ini_file->ReadInt(section, "Thread", -1);
	if (thread_index < 0)
		throw Error(misc::fmt("%s: section [%s]: invalid or missing "
				"value for 'Thread'",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Check bounds
	if (core_index >= Cpu::getNumCores() ||
			thread_index >= Cpu::getNumThreads())
	{
		misc::Warning("%s: section [%s] ignored, referring to x86 Core "
				"%d, Thread %d.\n"
				"\tThis section refers to a core or thread "
				"that does not currently exists. Please review "
				"your x86 configuration file if this behavior "
				"is not desired.",
				ini_file->getPath().c_str(),
				section.c_str(),
				core_index,
				thread_index);
		return;
	}

	// Check that entry has not been assigned before
	Thread *thread = cpu->getThread(core_index, thread_index);
	if (thread->data_module || thread->instruction_module)
	{
		assert(thread->data_module && thread->instruction_module);
		throw Error(misc::fmt("%s: section [%s]: entry from Core %d, "
				"Thread %d already assigned.\n"
				"\tA different [Entry <name>] section in the "
				"memory configuration file has already "
				"assigned an entry for this particular core and "
				"thread. Please review your configuration file "
				"to avoid duplicates.",
				ini_file->getPath().c_str(),
				section.c_str(),
				core_index,
				thread_index));
	}

	// Read modules
	std::string data_module_name;
	std::string instruction_module_name;
	if (data_inst_present)
	{
		data_module_name = ini_file->ReadString(section, "DataModule");
		instruction_module_name = ini_file->ReadString(section, "InstModule");
		assert(!data_module_name.empty());
		assert(!instruction_module_name.empty());
	}
	else
	{
		data_module_name = instruction_module_name = ini_file->ReadString(
				section, "Module");
		assert(!data_module_name.empty());
	}

	// Assign data module
	mem::System *memory_system = mem::System::getInstance();
	mem::Module *data_module = memory_system->getModule(data_module_name);
	thread->data_module = data_module;
	if (!data_module)
		throw Error(misc::fmt("%s: section [%s]: '%s' is not a valid "
				"module name.\n"
				"\tThe given module name must match a module "
				"declared in a section [Module <name>] in the "
				"memory configuration file.\n",
				ini_file->getPath().c_str(),
				section.c_str(),
				data_module_name.c_str()));

	// Assign instruction module
	mem::Module *instruction_module = memory_system->getModule(instruction_module_name);
	thread->instruction_module = instruction_module;
	if (!instruction_module)
		throw Error(misc::fmt("%s: section [%s]: '%s' is not a valid "
				"module name.\n"
				"\tThe given module name must match a module "
				"declared in a section [Module <name>] in the "
				"memory configuration file.\n",
				ini_file->getPath().c_str(),
				section.c_str(),
				instruction_module_name.c_str()));
	
	// Add modules to entry list
	entry_modules.push_back(data_module);
	if (data_module != instruction_module)
		entry_modules.push_back(instruction_module);

	// Debug
	mem::System::debug <<
			"\tx86 Core " << core_index << ", "
			"Thread " << thread_index << '\n' <<
			"\t\tEntry for instructions -> " <<
			instruction_module->getName() << '\n' <<
			"\t\tEntry for data -> " << 
			data_module->getName() << '\n' <<
			'\n';
}


void Timing::CheckMemoryConfiguration(misc::IniFile *ini_file)
{
	// Check that all cores/threads have an entry to the memory hierarchy.
	for (int i = 0; i < cpu->getNumCores(); i++)
	{
		Core *core = cpu->getCore(i);
		for (int j = 0; j < core->getNumThreads(); j++)
		{
			Thread *thread = core->getThread(j);
			if (!thread->data_module || !thread->instruction_module)
				throw Error(misc::fmt("%s: x86 Core %d, Thread %d "
						"lacks a data/instruction entry to memory.\n"
						"\tPlease add a new [Entry <name>] section "
						"in your memory configuration file to "
						"associate this hardware thread with a "
						"memory module.",
						ini_file->getPath().c_str(),
						i, j));
		}
	}
}


void Timing::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("x86");
	
	// Option --x86-sim <kind>
	command_line->RegisterEnum("--x86-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of x86 simulation.");

	// Option --x86-config <file>
	command_line->RegisterString("--x86-config <file>", config_file,
			"Configuration file for the x86 Cpu timing model, including parameters "
			"describing stage bandwidth, structures size, and other parameters of "
			"processor cores and threads. Type 'm2s --x86-help' for details on the file "
			"format.");

	// Option --x86-report <file>
	command_line->RegisterString("--x86-report <file>", report_file,
			"File to dump a report of the x86 Cpu pipeline, including statistics such "
			"as the number of instructions handled in every pipeline stage, read/write "
			"accesses performed on pipeline queues, etc. This option is only valid for "
			"detailed x86 simulation (option '--x86-sim detailed').");

	// Option --x86-help
	command_line->RegisterBool("--x86-help", help,
			"Display a help message describing the format of the x86 Cpu context "
			"configuration file.");

	// Option --x86-debug-trace-cache <file>
	command_line->RegisterString("--x86-debug-trace-cache <file>",
			TraceCache::debug_file,
			"Debug information for trace cache.");
	
	// Option --x86-debug-register-file <file>
	command_line->RegisterString("--x86-debug-register-file <file>",
			RegisterFile::debug_file,
			"Debug information for the register file.");

	// Option --x86-max-cycles <int>
	command_line->RegisterInt64("--x86-max-cycles <cycles>", Cpu::max_cycles,
			"Maximum number of cycles for the timing simulator "
			"to run.  If this maximum is reached, the simulation "
			"will finish with the X86MaxCycles string.");

}


void Timing::ProcessOptions()
{
	// Configuration
	misc::IniFile ini_file;
	if (!config_file.empty())
		ini_file.Load(config_file);

	// Instantiate timing simulator if '--x86-sim detailed' is present
	if (sim_kind == comm::Arch::SimDetailed)
	{
		// First: parse configuration
		ParseConfiguration(&ini_file);

		// Second: generate instance
		getInstance();
	}

	// Check valid file in '--x86-report'
	if (!report_file.empty())
	{
		std::ofstream os(report_file);
		if (!os.good())
			throw Error(misc::fmt("%s: Cannot open report file",
					report_file.c_str()));
	}

	// Print x86 configuration INI format
	if (help)
	{
		std::cerr << help_message;
		exit(0);
	}

	// Debuggers
	TraceCache::debug.setPath(TraceCache::debug_file);
	RegisterFile::debug.setPath(RegisterFile::debug_file);
}


void Timing::ParseConfiguration(misc::IniFile *ini_file)
{
	// Parse configuration by their sections
	std::string section = "General";
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz.\n",
				ini_file->getPath().c_str()));

	// Parse CPU configuration by their sections
	Cpu::ParseConfiguration(ini_file);

	// Parse register file configuration by their sections
	RegisterFile::ParseConfiguration(ini_file);

	// Parse branch predictor configuration by their sections
	BranchPredictor::ParseConfiguration(ini_file);

	// Parse trace cache configuration by their sections
	TraceCache::ParseConfiguration(ini_file);

	// Parse ALU configuration by their sections
	Alu::ParseConfiguration(ini_file);
}


void Timing::DumpSummary(std::ostream &os) const
{
	// Simulated time in nanoseconds
	esim::FrequencyDomain *frequency_domain = getFrequencyDomain();
	double cycle_time = (double) frequency_domain->getCycleTime() / 1e3;
	os << misc::fmt("SimTime = %.2f [ns]\n", getCycle() * cycle_time);
	
	// Frequency
	os << misc::fmt("Frequency = %d [MHz]\n", frequency_domain->getFrequency());

	// Cycles
	os << misc::fmt("Cycles = %lld\n", getCycle());

	// Cycles per second
	Emulator *emulator = Emulator::getInstance();
	double time_in_seconds = (double) emulator->getTimerValue() / 1e6;
	double cycles_per_second = time_in_seconds > 0.0 ?
			(double) getCycle() / time_in_seconds : 0.0;
	os << misc::fmt("CyclesPerSecond = %.0f\n", cycles_per_second);
	
	// Fast-forward instructions
	os << misc::fmt("FastForwardInstructions = %lld\n", Cpu::getNumFastForwardInstructions());

	// Number of committed instructions
	os << misc::fmt("CommittedInstructions = %lld\n", cpu->getNumCommittedInstructions());

	// Number of committed instructions per cycle
	double instructions_per_cycle = cpu->getCycle() ?
			(double) cpu->getNumCommittedInstructions()
			/ cpu->getCycle() : 0.0;
	os << misc::fmt("CommittedInstructionsPerCycle = %.4g\n", instructions_per_cycle);
	
	// Number of committed micro-instruction
	os << misc::fmt("CommittedMicroInstructions = %lld\n", cpu->getNumCommittedUinsts());

	// Number of committed micro-instructions per cycle
	double uinsts_per_cycle = cpu->getCycle() ?
			(double) cpu->getNumCommittedUinsts()
			/ cpu->getCycle() : 0.0;
	os << misc::fmt("CommittedMicroInstructionsPerCycle = %.4g\n", uinsts_per_cycle);

	// Branch prediction accuracy
	double branch_accuracy = cpu->getNumBranches() ?
			(double) (cpu->getNumBranches()
			- cpu->getNumMispredictedBranches())
			/ cpu->getNumBranches()
			: 0.0;
	os << misc::fmt("BranchPredictionAccuracy = %.4g\n", branch_accuracy);
}


void Timing::DumpUopReport(std::ostream &os, const long long *uop_stats,
		const std::string &prefix, int peak_ipc) const
{
	// Counters
	long long uinst_int_count = 0;
	long long uinst_logic_count = 0;
	long long uinst_fp_count = 0;
	long long uinst_mem_count = 0;
	long long uinst_ctrl_count = 0;
	long long uinst_total = 0;

	// Traverse uops
	for (int i = 0; i < Uinst::OpcodeCount; i++)
	{
		// Get micro-instruction name and flags
		std::string name = Uinst::getInfo((Uinst::Opcode) i)->name;
		unsigned flags = Uinst::getInfo((Uinst::Opcode) i)->flags;

		// Count instructions
		os << misc::fmt("%s.Uop.%s = %lld\n", prefix.c_str(),
				name.c_str(), uop_stats[i]);

		// Count instruction types
		if (flags & Uinst::FlagInt)
			uinst_int_count += uop_stats[i];
		if (flags & Uinst::FlagLogic)
			uinst_logic_count += uop_stats[i];
		if (flags & Uinst::FlagFp)
			uinst_fp_count += uop_stats[i];
		if (flags & Uinst::FlagMem)
			uinst_mem_count += uop_stats[i];
		if (flags & Uinst::FlagCtrl)
			uinst_ctrl_count += uop_stats[i];
		uinst_total += uop_stats[i];
	}

	// Dump stats
	os << misc::fmt("%s.Integer = %lld\n", prefix.c_str(), uinst_int_count);
	os << misc::fmt("%s.Logic = %lld\n", prefix.c_str(), uinst_logic_count);
	os << misc::fmt("%s.FloatingPoint = %lld\n", prefix.c_str(), uinst_fp_count);
	os << misc::fmt("%s.Memory = %lld\n", prefix.c_str(), uinst_mem_count);
	os << misc::fmt("%s.Ctrl = %lld\n", prefix.c_str(), uinst_ctrl_count);
	os << misc::fmt("%s.WndSwitch = %lld\n", prefix.c_str(),
			uop_stats[Uinst::OpcodeCall] + uop_stats[Uinst::OpcodeRet]);
	os << misc::fmt("%s.Total = %lld\n", prefix.c_str(), uinst_total);
	os << misc::fmt("%s.IPC = %.4g\n", prefix.c_str(), getCycle() ?
			(double) uinst_total / getCycle() : 0.0);
	os << misc::fmt("%s.DutyCycle = %.4g\n", prefix.c_str(),
			getCycle() && peak_ipc ?
			(double) uinst_total / getCycle() / peak_ipc : 0.0);
	os << '\n';
}


void Timing::DumpReport() const
{
	// Ignore if no report file was specified
	if (report_file.empty())
		return;

	// Open file for writing
	std::ofstream os(report_file);
	if (!os.good())
		throw Error(misc::fmt("%s: Cannot open x86 report",
				report_file.c_str()));

	// Dump CPU configuration
	os << misc::fmt(";\n; CPU Configuration\n;\n\n");
	DumpConfiguration(os);

	// CPU statistics
	os << misc::fmt(";\n; Simulation Statistics\n;\n\n");

	// Time in microseconds that the emulator for this architecture has been
	// running so far.
	Emulator *emulator = Emulator::getInstance();
	long long now = emulator->getTimerValue();

	// Global statistics
	os << "; Global statistics\n";
	os << "[ Global ]\n";
	os << misc::fmt("Cycles = %lld\n", getCycle());
	os << misc::fmt("Time = %.2f\n", (double) now / 1e6);
	os << misc::fmt("CyclesPerSecond = %.0f\n", now ?
			(double) getCycle() / now * 1e6 : 0.0);
	os << '\n';
	
	// Dispatch stage
	os << "; Dispatch stage\n";
	DumpUopReport(os, cpu->getNumDispatchedUinstArray(),
			"Dispatch", Cpu::getDispatchWidth());

	// Issue stage
	os << "; Issue stage\n";
	DumpUopReport(os, cpu->getNumIssuedUinstArray(),
			"Issue", Cpu::getIssueWidth());

	// Commit stage
	os << "; Commit stage\n";
	DumpUopReport(os, cpu->getNumCommittedUinstArray(),
			"Commit", Cpu::getCommitWidth());

	// Committed branches
	os << "; Committed branches\n";
	os << ";    Branches - Number of committed control uops\n";
	os << ";    Squashed - Number of mispredicted uops squashed from the ROB\n";
	os << ";    Mispred - Number of mispredicted branches in the correct path\n";
	os << ";    PredAcc - Prediction accuracy\n";
	os << misc::fmt("Commit.Branches = %lld\n", cpu->getNumBranches());
	os << misc::fmt("Commit.Squashed = %lld\n", cpu->getNumSquashedUinsts());
	os << misc::fmt("Commit.Mispred = %lld\n", cpu->getNumMispredictedBranches());
	os << misc::fmt("Commit.PredAcc = %.4g\n", cpu->getNumBranches() ?
			(double) (cpu->getNumBranches()
			- cpu->getNumMispredictedBranches())
			/ cpu->getNumBranches() : 0.0);
	os << '\n';

	// Report for each core
	for (int i = 0; i < Cpu::getNumCores(); i++)
	{
		// Cores
		Core *core = cpu->getCore(i);
		os << misc::fmt("\n; Statistics for core %d\n", core->getId());
		os << misc::fmt("[ %s ]\n\n", core->getName().c_str());

		// Functional units
		Alu *alu = core->getAlu();
		alu->DumpReport(os);

		// Dispatch slots
		if (Cpu::getDispatchKind() == Cpu::DispatchKindTimeslice)
		{
			// Header
			os << "; Dispatch slots usage (sum = cycles * dispatch width)\n";
			os << ";    used - dispatch slot was used by a non-spec uop\n";
			os << ";    spec - used by a mispeculated uop\n";
			os << ";    ctx - no context allocated to thread\n";
			os << ";    uopq,rob,iq,lsq,rename - no space in structure\n";

			// Stats
			os << misc::fmt("Dispatch.Stall.used = %lld\n", core->getDispatchStall(Thread::DispatchStallUsed));
			os << misc::fmt("Dispatch.Stall.spec = %lld\n", core->getDispatchStall(Thread::DispatchStallSpeculative));
			os << misc::fmt("Dispatch.Stall.uop_queue = %lld\n", core->getDispatchStall(Thread::DispatchStallUopQueue));
			os << misc::fmt("Dispatch.Stall.rob = %lld\n", core->getDispatchStall(Thread::DispatchStallReorderBuffer));
			os << misc::fmt("Dispatch.Stall.iq = %lld\n", core->getDispatchStall(Thread::DispatchStallInstructionQueue));
			os << misc::fmt("Dispatch.Stall.lsq = %lld\n", core->getDispatchStall(Thread::DispatchStallLoadStoreQueue));
			os << misc::fmt("Dispatch.Stall.rename = %lld\n", core->getDispatchStall(Thread::DispatchStallRename));
			os << misc::fmt("Dispatch.Stall.ctx = %lld\n", core->getDispatchStall(Thread::DispatchStallContext));
			os << '\n';
		}

		// Dispatch stage
		os << "; Dispatch stage\n";
		DumpUopReport(os, core->getNumDispatchedUinstArray(),
				"Dispatch", Cpu::getDispatchWidth());

		// Issue stage
		os << "; Issue stage\n";
		DumpUopReport(os, core->getNumIssuedUinstArray(),
				"Issue", Cpu::getIssueWidth());

		// Commit stage
		os << "; Commit stage\n";
		DumpUopReport(os, core->getNumCommittedUinstArray(),
				"Commit", Cpu::getCommitWidth());

		// Committed branches
		os << "; Committed branches\n";
		os << misc::fmt("Commit.Branches = %lld\n", core->getNumBranches());
		os << misc::fmt("Commit.Squashed = %lld\n", core->getNumSquashedUinsts());
		os << misc::fmt("Commit.Mispred = %lld\n", core->getNumMispredictedBranches());
		os << misc::fmt("Commit.PredAcc = %.4g\n", core->getNumBranches() ?
				(double) (core->getNumBranches()
				- core->getNumMispredictedBranches())
				/ core->getNumBranches() : 0.0);
		os << '\n';

		// Occupancy statistics
		os << "; Structure statistics (reorder buffer, instruction queue,\n";
		os << "; load-store queue, and integer/floating-point/XMM register file)\n";
		os << ";    Size - Available size\n";
		os << ";    Reads, Writes - Accesses to the structure\n";

		// Shared reorder buffer statistics
		if (Cpu::getReorderBufferKind() == Cpu::ReorderBufferKindShared)
		{
			os << misc::fmt("ROB.Size = %d\n", Cpu::getReorderBufferSize() * Cpu::getNumThreads());
			os << misc::fmt("ROB.Reads = %lld\n", core->getNumReorderBufferReads());
			os << misc::fmt("ROB.Writes = %lld\n", core->getNumReorderBufferWrites());
		}

		// Shared instruction queue statistics
		if (Cpu::getInstructionQueueKind() == Cpu::InstructionQueueKindShared)
		{
			os << misc::fmt("IQ.Size = %d\n", Cpu::getInstructionQueueSize() * Cpu::getNumThreads());
			os << misc::fmt("IQ.Reads = %lld\n", core->getNumInstructionQueueReads());
			os << misc::fmt("IQ.Writes = %lld\n", core->getNumInstructionQueueWrites());
		}

		// Shared load-store queue statistics
		if (Cpu::getLoadStoreQueueKind() == Cpu::LoadStoreQueueKindShared)
		{
			os << misc::fmt("LSQ.Size = %d\n", Cpu::getLoadStoreQueueSize() * Cpu::getNumThreads());
			os << misc::fmt("LSQ.Reads = %lld\n", core->getNumLoadStoreQueueReads());
			os << misc::fmt("LSQ.Writes = %lld\n", core->getNumLoadStoreQueueWrites());
		}

		// Shared register file statistics
		if (RegisterFile::getKind() == RegisterFile::KindShared)
		{
			// Integer register file
			os << misc::fmt("RF_Int.Size = %d\n", RegisterFile::getIntegerSize());
			os << misc::fmt("RF_Int.Reads = %lld\n", core->getNumIntegerRegisterReads());
			os << misc::fmt("RF_Int.Writes = %lld\n", core->getNumIntegerRegisterWrites());

			// Floating-point register file
			os << misc::fmt("RF_Fp.Size = %d\n", RegisterFile::getFloatingPointSize());
			os << misc::fmt("RF_Fp.Reads = %lld\n", core->getNumFloatingPointRegisterReads());
			os << misc::fmt("RF_Fp.Writes = %lld\n", core->getNumFloatingPointRegisterWrites());

			// XMM register file
			os << misc::fmt("RF_Xmm.Size = %d\n", RegisterFile::getXmmSize());
			os << misc::fmt("RF_Xmm.Reads = %lld\n", core->getNumXmmRegisterReads());
			os << misc::fmt("RF_Xmm.Writes = %lld\n", core->getNumXmmRegisterWrites());
		}

		// Done
		os << '\n';

		// Per-thread report
		for (int j = 0; j < Cpu::getNumThreads(); j++)
		{
			// Title
			Thread *thread = core->getThread(j);
			os << misc::fmt("\n; Statistics for core %d - thread %d\n", i, j);
			os << misc::fmt("[ %s ]\n\n", thread->getName().c_str());

			// Dispatch stage
			os << "; Dispatch stage\n";
			DumpUopReport(os, thread->getNumDispatchedUinstArray(),
					"Dispatch", Cpu::getDispatchWidth());

			// Issue stage
			os << "; Issue stage\n";
			DumpUopReport(os, thread->getNumIssuedUinstArray(),
					"Issue", Cpu::getIssueWidth());

			// Commit stage
			os << "; Commit stage\n";
			DumpUopReport(os, thread->getNumCommittedUinstArray(),
					"Commit", Cpu::getCommitWidth());

			// Committed branches
			os << "; Committed branches\n";
			os << misc::fmt("Commit.Branches = %lld\n", thread->getNumBranches());
			os << misc::fmt("Commit.Squashed = %lld\n", thread->getNumSquashedUinsts());
			os << misc::fmt("Commit.Mispred = %lld\n", thread->getNumMispredictedBranches());
			os << misc::fmt("Commit.PredAcc = %.4g\n", thread->getNumBranches() ?
					(double) (thread->getNumBranches()
					- thread->getNumMispredictedBranches())
					/ thread->getNumBranches() : 0.0);
			os << '\n';

			// Occupancy statistics
			os << "; Structure statistics (reorder buffer, instruction queue,\n";
			os << "; load-store queue, integer/floating-point/XMM register file,\n";
			os << "; and renaming table)\n";

			// Shared reorder buffer statistics
			if (Cpu::getReorderBufferKind() == Cpu::ReorderBufferKindPrivate)
			{
				os << misc::fmt("ROB.Size = %d\n", Cpu::getReorderBufferSize());
				os << misc::fmt("ROB.Reads = %lld\n", thread->getNumReorderBufferReads());
				os << misc::fmt("ROB.Writes = %lld\n", thread->getNumReorderBufferWrites());
			}

			// Shared instruction queue statistics
			if (Cpu::getInstructionQueueKind() == Cpu::InstructionQueueKindPrivate)
			{
				os << misc::fmt("IQ.Size = %d\n", Cpu::getInstructionQueueSize());
				os << misc::fmt("IQ.Reads = %lld\n", thread->getNumInstructionQueueReads());
				os << misc::fmt("IQ.Writes = %lld\n", thread->getNumInstructionQueueWrites());
			}

			// Shared load-store queue statistics
			if (Cpu::getLoadStoreQueueKind() == Cpu::LoadStoreQueueKindPrivate)
			{
				os << misc::fmt("LSQ.Size = %d\n", Cpu::getLoadStoreQueueSize());
				os << misc::fmt("LSQ.Reads = %lld\n", thread->getNumLoadStoreQueueReads());
				os << misc::fmt("LSQ.Writes = %lld\n", thread->getNumLoadStoreQueueWrites());
			}

			// Shared register file statistics
			if (RegisterFile::getKind() == RegisterFile::KindPrivate)
			{
				// Integer register file
				os << misc::fmt("RF_Int.Size = %d\n", RegisterFile::getIntegerSize());
				os << misc::fmt("RF_Int.Reads = %lld\n", thread->getNumIntegerRegisterReads());
				os << misc::fmt("RF_Int.Writes = %lld\n", thread->getNumIntegerRegisterWrites());

				// Floating-point register file
				os << misc::fmt("RF_Fp.Size = %d\n", RegisterFile::getFloatingPointSize());
				os << misc::fmt("RF_Fp.Reads = %lld\n", thread->getNumFloatingPointRegisterReads());
				os << misc::fmt("RF_Fp.Writes = %lld\n", thread->getNumFloatingPointRegisterWrites());

				// XMM register file
				os << misc::fmt("RF_Xmm.Size = %d\n", RegisterFile::getXmmSize());
				os << misc::fmt("RF_Xmm.Reads = %lld\n", thread->getNumXmmRegisterReads());
				os << misc::fmt("RF_Xmm.Writes = %lld\n", thread->getNumXmmRegisterWrites());
			}

			// Register aliasing table statistics
			RegisterFile *register_file = thread->getRegisterFile();
			os << misc::fmt("RAT.IntReads = %lld\n", register_file->getNumIntegerRatReads());
			os << misc::fmt("RAT.IntWrites = %lld\n", register_file->getNumIntegerRatWrites());
			os << misc::fmt("RAT.FpReads = %lld\n", register_file->getNumFloatingPointRatReads());
			os << misc::fmt("RAT.FpWrites = %lld\n", register_file->getNumFloatingPointRatWrites());
			os << misc::fmt("RAT.XmmReads = %lld\n", register_file->getNumXmmRatReads());
			os << misc::fmt("RAT.XmmWrites = %lld\n", register_file->getNumXmmRatWrites());

			// Branch target buffer statistics
			os << misc::fmt("BTB.Reads = %lld\n", thread->getNumBtbReads());
			os << misc::fmt("BTB.Writes = %lld\n", thread->getNumBtbWrites());

			// Done
			os << '\n';

			// Trace cache statistics
			TraceCache *trace_cache = thread->getTraceCache();
			if (TraceCache::isPresent() && trace_cache)
				trace_cache->DumpReport(os);
		}
	}
}


void Timing::DumpConfiguration(std::ofstream &os) const
{
	// General configuration
	os << "[ Config.General ]\n";
	os << misc::fmt("Frequency = %d\n", frequency);
	os << misc::fmt("Cores = %d\n", cpu->getNumCores());
	os << misc::fmt("Threads = %d\n", cpu->getNumThreads());
	os << misc::fmt("FastForward = %lld\n", cpu->getNumFastForwardInstructions());
	os << misc::fmt("ContextQuantum = %d\n", cpu->getContextQuantum());
	os << misc::fmt("ThreadQuantum = %d\n", cpu->getThreadQuantum());
	os << misc::fmt("ThreadSwitchPenalty = %d\n", cpu->getThreadSwitchPenalty());
	os << misc::fmt("RecoverKind = %s\n", cpu->recover_kind_map[cpu->getRecoverKind()]);
	os << misc::fmt("RecoverPenalty = %d\n", cpu->getRecoverPenalty());
	os << std::endl;

	// Pipeline
	os << misc::fmt("[ Config.Pipeline ]\n");
	os << misc::fmt("FetchKind = %s\n", cpu->fetch_kind_map[cpu->getFetchKind()]);
	os << misc::fmt("DecodeWidth = %d\n", cpu->getDecodeWidth());
	os << misc::fmt("DispatchKind = %s\n", cpu->dispatch_kind_map[cpu->getDispatchKind()]);
	os << misc::fmt("DispatchWidth = %d\n", cpu->getDispatchWidth());
	os << misc::fmt("IssueKind = %s\n", cpu->issue_kind_map[cpu->getIssueKind()]);
	os << misc::fmt("IssueWidth = %d\n", cpu->getIssueWidth());
	os << misc::fmt("CommitKind = %s\n", cpu->commit_kind_map[cpu->getCommitKind()]);
	os << misc::fmt("CommitWidth = %d\n", cpu->getCommitWidth());
	os << misc::fmt("OccupancyStats = %s\n", cpu->getOccupancyStats() ? "True" : "False");
	os << std::endl;

	// Queues
	os << misc::fmt("[ Config.Queues ]\n");
	os << misc::fmt("FetchQueueSize = %d\n", cpu->getFetchQueueSize());
	os << misc::fmt("UopQueueSize = %d\n", cpu->getUopQueueSize());
	os << misc::fmt("RobKind = %s\n", cpu->reorder_buffer_kind_map[cpu->getReorderBufferKind()]);
	os << misc::fmt("RobSize = %d\n", cpu->getReorderBufferSize());
	os << misc::fmt("IqKind = %s\n", cpu->instruction_queue_kind_map[cpu->getInstructionQueueKind()]);
	os << misc::fmt("IqSize = %d\n", cpu->getInstructionQueueSize());
	os << misc::fmt("LsqKind = %s\n", cpu->load_store_queue_kind_map[cpu->getLoadStoreQueueKind()]);
	os << misc::fmt("LsqSize = %d\n", cpu->getLoadStoreQueueSize());
	os << misc::fmt("RfKind = %s\n", RegisterFile::KindMap[RegisterFile::getKind()]);
	os << misc::fmt("RfIntSize = %d\n", RegisterFile::getIntegerSize());
	os << misc::fmt("RfFpSize = %d\n", RegisterFile::getFloatingPointSize());
	os << misc::fmt("RfXmmSize = %d\n", RegisterFile::getXmmSize());
	os << std::endl;

	// Trace Cache
	os << misc::fmt("[ Config.TraceCache ]\n");
	os << misc::fmt("Present = %s\n", TraceCache::isPresent() ? "True" : "False");
	os << misc::fmt("Sets = %d\n", TraceCache::getNumSets());
	os << misc::fmt("Assoc = %d\n", TraceCache::getNumWays());
	os << misc::fmt("TraceSize = %d\n", TraceCache::getTraceSize());
	os << misc::fmt("BranchMax = %d\n", TraceCache::getMaxBranches());
	os << misc::fmt("QueueSize = %d\n", TraceCache::getQueueSize());
	os << misc::fmt("\n");

	// ALU
	Alu::DumpConfiguration(os);

	// Branch Predictor
	os << misc::fmt("[ Config.BranchPredictor ]\n");
	os << misc::fmt("Kind = %s\n", BranchPredictor::KindMap[BranchPredictor::getKind()]);
	os << misc::fmt("BTB.Sets = %d\n", BranchPredictor::getBtbNumSets());
	os << misc::fmt("BTB.Assoc = %d\n", BranchPredictor::getBtbNumWays());
	os << misc::fmt("Bimod.Size = %d\n", BranchPredictor::getBimodSize());
	os << misc::fmt("Choice.Size = %d\n", BranchPredictor::getChoiceSize());
	os << misc::fmt("RAS.Size = %d\n", BranchPredictor::getRasSize());
	os << misc::fmt("TwoLevel.L1Size = %d\n", BranchPredictor::getTwoLevelL1Size());
	os << misc::fmt("TwoLevel.L2Size = %d\n", BranchPredictor::getTwoLevelL2Size());
	os << misc::fmt("TwoLevel.L2Height = %d\n", BranchPredictor::getTwoLevelL2Height());
	os << misc::fmt("TwoLevel.HistorySize = %d\n", BranchPredictor::getTwoLevelHistorySize());
	os << misc::fmt("\n");

	// End of configuration
	os << '\n';
}

} // namespace x86
