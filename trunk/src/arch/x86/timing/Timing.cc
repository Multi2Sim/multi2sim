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

#include "Timing.h"

namespace x86
{

//
// Configuration file name
//
std::string Timing::config_file;

//
// report file name
//
std::string Timing::report_file;

//
// MMU report file name
//
std::string Timing::mmu_report_file;

//
// Message to display with '--x86-help'
//
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


Timing::Timing()
{

}


Timing *Timing::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Timing());
	return instance.get();
}


void Timing::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("x86");

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
}


void Timing::ProcessOptions()
{
	// Configuration
	if (!config_file.empty())
		ParseConfiguration(config_file);

	// Print x86 configuration INI format
	if (help)
	{
		if (!help_message.empty())
		{
			misc::StringFormatter formatter(help_message);
			// FIXME
		}
	}
}


void Timing::ParseConfiguration(std::string &config_file)
{
	// Get INI file format from the original configuration file
	misc::IniFile ini_file(config_file);

	// Parse configuration by their sections
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section_name = ini_file.getSection(i);
		if (misc::StringPrefix(section_name, "General"))
		{
			CPU::ParseConfiguration(section_name, ini_file);
		}
		if (misc::StringPrefix(section_name, "Pipeline"))
		{
			CPU::ParseConfiguration(section_name, ini_file);
		}
		if (misc::StringPrefix(section_name, "Queues"))
		{
			CPU::ParseConfiguration(section_name, ini_file);
		}
		if (misc::StringPrefix(section_name, "BranchPredictor"))
		{
			BranchPredictor::ParseConfiguration(section_name, ini_file);
		}
		if (misc::StringPrefix(section_name, "TraceCache"))
		{
			TraceCache::ParseConfiguration(section_name, ini_file);
		}
	}
}

} // namespace x86
