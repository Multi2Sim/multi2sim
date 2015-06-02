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

#include "Timing.h"


namespace x86
{

// Singleton instance
std::unique_ptr<Timing> Timing::instance;



//
// Configuration options
//

std::string Timing::config_file;

// Simulation kind
comm::Arch::SimKind Timing::sim_kind = comm::Arch::SimFunctional;

// Report file name
std::string Timing::report_file;

// MMU report file name
std::string Timing::mmu_report_file;

// Message to display with '--x86-help'
const std::string Timing::help_message =
		"The x86 CPU configuration file is a plain text INI file, defining\n"
		"the parameters of the CPU model used for a detailed (architectural) simulation.\n"
		"This configuration file is passed to Multi2Sim with option '--x86-config <file>,\n"
		"which must be accompanied by option '--x86-sim detailed'.\n"
		"\n"
		"The following is a list of the sections allowed in the CPU configuration file,\n"
		"along with the list of variables for each section.\n"
		"\n"
		"Section '[ General ]':\n"
		"\n"
		"  Frequency = <freq> (Default = 1000 MHz)\n"
		"      Frequency in MHz for the x86 CPU. Value between 1 and 10K.\n"
		"  Cores = <num_cores> (Default = 1)\n"
		"      Number of cores.\n"
		"  Threads = <num_threads> (Default = 1)\n"
		"      Number of hardware threads per core. The total number of computing nodes\n"
		"      in the CPU model is equals to Cores * Threads.\n"
		"  FastForward = <num_inst> (Default = 0)\n"
		"      Number of x86 instructions to run with a fast functional simulation before\n"
		"      the architectural simulation starts.\n"
		"  ContextQuantum = <cycles> (Default = 100k)\n"
		"      If ContextSwitch is true, maximum number of cycles that a context can occupy\n"
		"      a CPU hardware thread before it is replaced by other pending context.\n"
		"  ThreadQuantum = <cycles> (Default = 1k)\n"
		"      For multithreaded processors (Threads > 1) configured as coarse-grain multi-\n"
		"      threading (FetchKind = SwitchOnEvent), number of cycles in which instructions\n"
		"      are fetched from the same thread before switching.\n"
		"  ThreadSwitchPenalty = <cycles> (Default = 0)\n"
		"      For coarse-grain multithreaded processors (FetchKind = SwitchOnEvent), number\n"
		"      of cycles that the fetch stage stalls after a thread switch.\n"
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
		"      Normally the CPU uses cohnerency-enabled Store commands.  Setting this to True\n"
		"      causes the CPU to issue NCStore commands to reduce protocol overhead.\n"
		"      When True, Cores and Threads must be set to 1.\n"
		"\n"
		"Section '[ Pipeline ]':\n"
		"\n"
		"  FetchKind = {Shared|TimeSlice|SwitchOnEvent} (Default = TimeSlice)\n"
		"      Policy for fetching instruction from different threads. A shared fetch stage\n"
		"      fetches instructions from different threads in the same cycle; a time-slice\n"
		"      fetch switches between threads in a round-robin fashion; option SwitchOnEvent\n"
		"      switches thread fetch on long-latency operations or thread quantum expiration.\n"
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
		"      will be attached to the CPU report.\n"
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

bool Timing::help = false;

// Default frequency
int Timing::frequency = 1000;


Timing *Timing::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Timing());
	return instance.get();
}


bool Timing::Run()
{
	return false;
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
	for (int i = 0; i < CPU::getNumCores(); i++)
	{
		// L1 cache
		std::string module_name = misc::fmt("x86-l1-%d", i);
		section = "Module " + module_name;
		ini_file->WriteString(section, "Type", "Cache");
		ini_file->WriteString(section, "Geometry", "x86-geo-l1");
		ini_file->WriteString(section, "LowNetwork", "x86-net-l1-l2");
		ini_file->WriteString(section, "LowModules", "x86-l2");

		// Entry
		for (int j = 0; j < CPU::getNumThreads(); j++)
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
	if (core_index >= CPU::getNumCores() ||
			thread_index >= CPU::getNumThreads())
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
	if (thread->getDataModule() || thread->getInstModule())
	{
		assert(thread->getDataModule() && thread->getInstModule());
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
	std::string inst_module_name;
	if (data_inst_present)
	{
		data_module_name = ini_file->ReadString(section, "DataModule");
		inst_module_name = ini_file->ReadString(section, "InstModule");
		assert(!data_module_name.empty());
		assert(!inst_module_name.empty());
	}
	else
	{
		data_module_name = inst_module_name = ini_file->ReadString(
				section, "Module");
		assert(!data_module_name.empty());
	}

	// Assign data module
	mem::System *memory_system = mem::System::getInstance();
	mem::Module *data_module = memory_system->getModule(data_module_name);
	thread->setDataModule(data_module);
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
	mem::Module *inst_module = memory_system->getModule(inst_module_name);
	thread->setInstModule(inst_module);
	if (!inst_module)
		throw Error(misc::fmt("%s: section [%s]: '%s' is not a valid "
				"module name.\n"
				"\tThe given module name must match a module "
				"declared in a section [Module <name>] in the "
				"memory configuration file.\n",
				ini_file->getPath().c_str(),
				section.c_str(),
				inst_module_name.c_str()));
	
	// Add modules to entry list
	entry_modules.push_back(data_module);
	if (data_module != inst_module)
		entry_modules.push_back(inst_module);

	// Debug
	mem::System::debug <<
			"\tx86 Core " << core_index << ", "
			"Thread " << thread_index << '\n' <<
			"\t\tEntry for instructions -> " <<
			inst_module->getName() << '\n' <<
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
			if (!thread->getDataModule() || !thread->getInstModule())
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
			"Configuration file for the x86 CPU timing model, including parameters"
			"describing stage bandwidth, structures size, and other parameters of"
			"processor cores and threads. Type 'm2s --x86-help' for details on the file"
			"format.");

	// Option --x86-mmu-report <file>
	command_line->RegisterString("--x86-mmu-report <file>", mmu_report_file,
			"File to dump a report of the x86 MMU. Use together with a detailed"
			"CPU simulation (option '--x86-sim detailed').");

	// Option --x86-report <file>
	command_line->RegisterString("--x86-report <file>", report_file,
			"File to dump a report of the x86 CPU pipeline, including statistics such"
			"as the number of instructions handled in every pipeline stage, read/write"
			"accesses performed on pipeline queues, etc. This option is only valid for"
			"detailed x86 simulation (option '--x86-sim detailed').");

	// Option --x86-help
	command_line->RegisterBool("--x86-help", help,
			"Display a help message describing the format of the x86 CPU context"
			"configuration file.");

	// Option --x86-debug-trace-cache <file>
	command_line->RegisterString("--x86-debug-trace-cache <file>", TraceCache::debug_file,
			"Debug information for trace cache.");
}


void Timing::ProcessOptions()
{
	// Configuration
	misc::IniFile ini_file;
	if (!config_file.empty())
		ini_file.Load(config_file);
	ParseConfiguration(&ini_file);

	// Instantiate timing simulator if '--x86-sim detailed' is present
	if (sim_kind == comm::Arch::SimDetailed)
		getInstance();

	// Print x86 configuration INI format
	if (help)
	{
		if (!help_message.empty())
		{
			misc::StringFormatter formatter(help_message);
			std::cerr << formatter;
			exit(1);
		}
	}

	// Debuggers
	TraceCache::debug.setPath(TraceCache::debug_file);
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
	CPU::ParseConfiguration(ini_file);

	// Parse Register File configuration by their sections
	RegisterFile::ParseConfiguration(ini_file);

	// Parse Branch Predictor configuration by their sections
	BranchPredictor::ParseConfiguration(ini_file);

	// Parse Trace Cacher configuration by their sections
	TraceCache::ParseConfiguration(ini_file);


}

} // namespace x86
