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
#include <lib/cpp/CommandLine.h>
#include <memory/System.h>
#include <arch/kepler/emulator/Emulator.h>
#include <arch/kepler/emulator/Grid.h>

#include "SM.h"
#include "Timing.h"


namespace Kepler
{

// Singleton instance
std::unique_ptr<Timing> Timing::instance;



//
// Configuration options
//

std::string Timing::config_file;

//comm::Arch::SimKind Timing::sim_kind = comm::Arch::SimDetailed;

std::string Timing::report_file;

esim::Trace Timing::trace;

const std::string Timing::help_message =
		"The Kepler GPU configuration file is a plain text INI file defining\n"
		"the parameters of the Kepler model for a detailed (architectural)\n"
		"configuration. This file is passed to Multi2Sim with the\n"
		"'--kpl-config <file>' option, and should always be used together \n"
		"with option '--kpl-sim detailed'.\n"
		"\n"
		"The following is a list of the sections allowed in the GPU "
		"configuration\n"
		"file, along with the list of variables for each section.\n"
		"\n"
		"Section '[ Device ]': parameters for the GPU.\n"
		"\n"
		"  Frequency = <value> (Default = 745)\n"
		"      Frequency for the Kepler GPU in MHz.\n"
		"  NumSMs = <num> (Default = 15)\n"
		"      Number of streaming multiprocessor in the GPU.\n"
		"\n"
		"Section '[ SM ]': parameters for the Streaming multiprocessors.\n"
		"\n"
		"  NumWarpPools = <num> (Default = 4)\n"
		"      Number of warp pools per SM.\n"
		"  MaxBlocksPerWarpPool = <num> (Default = 4)\n"
		"      The maximum number of blocks that can be scheduled to a\n"
		"      warp pool at a time.\n"
		"  MaxWarpsPerWarpPool = <num> (Default = 16)\n"
		"      The maximum number of warps that can be scheduled to a\n"
		"      warp pool at a time.\n"
		"  NumRegisters = <num> (Default = 65536)\n"
		"      Number of registers per SM. These are\n"
		"      divided evenly between all warp pools.\n"
		"\n"
		"Section '[ FrontEnd ]': parameters for fetch and dispatch.\n"
		"\n"
		"  FetchLatency = <cycles> (Default = 5)\n"
		"      Latency of instruction memory in number of cycles.\n"
		"  FetchWidth = <num> (Default = 8)\n"
		"      Maximum number of instructions fetched per cycle.\n"
		"  FetchBufferSize = <num> (Default = 10)\n"
		"      Size of the buffer holding fetched instructions.\n"
		"  DispatchLatency = <cycles> (Default = 1)\n"
		"      Latency of the decode stage in number of cycles.\n"
		"  DispatchWidth = <num> (Default = 5)\n"
		"      Number of instructions that can be issued per cycle.\n"
		"  MaxInstIssuedPerType = <num> (Default = 1)\n"
		"      Maximum number of instructions that can be issued of each type\n"
		"      (SIMD, scalar, etc.) in a single cycle.\n"
		"\n"
		"Section '[ SPU ]': parameters for the Single Precision Units.\n"
		"\n"
		"  NumSPULanes = <num> (Default = 32)\n"
		"      Number of lanes per SPU.  This must divide the warp\n"
		"      size (32) evenly.\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		"  ExecutionLatency = <cycles> (Default = 1)\n"
		"      Latency of execution in number of cycles.\n"
		"  ExecutionBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding executing instructions.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n"
		"Section '[ DPU ]': parameters for the Double Precision Units.\n"
		"\n"
		"  NumSPULanes = <num> (Default = 32)\n"
		"      Number of lanes per DPU.  This must divide the warp\n"
		"      size (32) evenly.\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		"  ExecutionLatency = <cycles> (Default = 1)\n"
		"      Latency of execution in number of cycles.\n"
		"  ExecutionBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding executing instructions.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n"
		"Section '[ SFU ]': parameters for the Special Function Units.\n"
		"\n"
		"  NumSPULanes = <num> (Default = 32)\n"
		"      Number of lanes per SFU.  This must divide the warp\n"
		"      size (32) evenly.\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		"  ExecutionLatency = <cycles> (Default = 1)\n"
		"      Latency of execution in number of cycles.\n"
		"  ExecutionBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding executing instructions.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n"
		"Section '[ IMU ]': parameters for the Integer Math Units.\n"
		"\n"
		"  NumSPULanes = <num> (Default = 32)\n"
		"      Number of lanes per IMU.  This must divide the warp\n"
		"      size (32) evenly.\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		"  ExecutionLatency = <cycles> (Default = 1)\n"
		"      Latency of execution in number of cycles.\n"
		"  ExecutionBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding executing instructions.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n"
		"Section '[ BRU ]': parameters for the Branch Units.\n"
		"\n"
		"  NumSPULanes = <num> (Default = 32)\n"
		"      Number of lanes per BRU.  This must divide the warp\n"
		"      size (32) evenly.\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		"  ExecutionLatency = <cycles> (Default = 1)\n"
		"      Latency of execution in number of cycles.\n"
		"  ExecutionBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding executing instructions.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n"
		"Section '[ LSU ]': parameters for the Single Precision Units.\n"
		"\n"
		"  Width = <num> (Default = 1)\n"
		"      Maximum number of instructions processed per cycle.\n"
		"  DispatchBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding dispatched instructions.\n"
		"  ReadLatency = <cycles> (Default = 1)\n"
		"      Latency of register file access in number of cycles for reads.\n"
		"  ReadBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register read instructions.\n"
		" MaxInFlightMemAcccesses = <num> (Defalut = 32)\n"
		"		Size of in flight memory accesses.\n"
		"  WriteLatency = <cycles> (Default = 1)\n"
		"      Latency of register file writes in number of cycles.\n"
		"  WriteBufferSize = <num> (Default = 1)\n"
		"      Size of the buffer holding register write instructions.\n"
		"\n";

bool Timing::help = false;

int Timing::frequency = 732;


Timing::Timing() : comm::Timing("Kepler")
{
	// Configure frequency domain with the frequency given by the user
	ConfigureFrequencyDomain(frequency);

	// Create GPU
	//gpu = misc::new_unique<GPU>();
}


Timing *Timing::getInstance()
{
	// Instance already exists
	if (instance)
		return instance.get();

	// Create instance
	instance = misc::new_unique<Timing>();
	Timing::getInstance()->CreateGPU();
	return instance.get();
}


Grid *Timing::addGrid(Function *function)
{
	// Create the grid and add it to the grid list
	grids.emplace_back(new Grid(function));
	return grids.back().get();
}


void Timing::WriteMemoryConfiguration(misc::IniFile *ini_file)
{
	// Cache geometry for L1 (16KB for L1 and 48KB for shared memory by default)
	std::string section = "CacheGeometry kpl-geo-l1";
	ini_file->WriteInt(section, "Sets", 32);
	ini_file->WriteInt(section, "Assoc", 4);
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 6);
	ini_file->WriteString(section, "Policy", "LRU");

	// Cache geometry for L2
	section = "CacheGeometry kpl-geo-l2";
	ini_file->WriteInt(section, "Sets", 32);
	ini_file->WriteInt(section, "Assoc", 16);
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 20);
	ini_file->WriteString(section, "Policy", "LRU");

	// Create L1 caches
	for (int i = 0; i < GPU::num_sms; i++)
	{
		section = misc::fmt("Module kpl-l1-%d", i);
		ini_file->WriteString(section, "Type", "Cache");
		ini_file->WriteString(section, "Geometry", "kpl-geo-l1");
		ini_file->WriteString(section, "LowNetwork", "kpl-net-l1-l2");
		ini_file->WriteString(section, "LowModules",
				"kpl-l2-0 kpl-l2-1 kpl-l2-2 kpl-l2-3 kpl-l2-4 kpl-l2-5");
	}

	// Create entries from SM to L1
	for (int i = 0; i < GPU::num_sms; i++)
	{
		// Entry
		section = misc::fmt("Entry kpl-sm-%d", i);
		ini_file->WriteString(section, "Arch", "Kepler");
		ini_file->WriteInt(section, "SM", i);

		std::string value = misc::fmt("kpl-l1-%d", i);
		ini_file->WriteString(section, "Module", value);
	}

	// l2 caches
	section = "Module kpl-l2-0";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-0-gm-0");
	ini_file->WriteString(section, "LowModules", "kpl-gm-0");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 0");

	section = "Module kpl-l2-1";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-1-gm-1");
	ini_file->WriteString(section, "LowModules", "kpl-gm-1");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 1");

	section = "Module kpl-l2-2";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-2-gm-2");
	ini_file->WriteString(section, "LowModules", "kpl-gm-2");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 2");

	section = "Module kpl-l2-3";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-3-gm-3");
	ini_file->WriteString(section, "LowModules", "kpl-gm-3");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 3");

	section = "Module kpl-l2-4";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-4-gm-4");
	ini_file->WriteString(section, "LowModules", "kpl-gm-4");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 4");

	section = "Module kpl-l2-5";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "kpl-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "kpl-net-l2-5-gm-5");
	ini_file->WriteString(section, "LowModules", "kpl-gm-5");
	ini_file->WriteString(section, "AddressRange",
			"ADDR DIV 128 MOD 6 EQ 5");

	// Global memory
	section = "Module kpl-gm-0";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-0-gm-0");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 0");

	section = "Module kpl-gm-1";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-1-gm-1");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 1");

	section = "Module kpl-gm-2";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-2-gm-2");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 2");

	section = "Module kpl-gm-3";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-3-gm-3");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 3");

	section = "Module kpl-gm-4";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-4-gm-4");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 4");

	section = "Module kpl-gm-5";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "kpl-net-l2-5-gm-5");
	ini_file->WriteInt(section, "BlockSize", 128);
	ini_file->WriteInt(section, "Latency", 300);
	ini_file->WriteString(section, "AddressRange",
		"ADDR DIV 128 MOD 6 EQ 5");

	// Newwork connecting L1s and L2s
	section = "Network kpl-net-l1-l2";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	// Networks connecting memory controllers and global memory banks
	section = "Network kpl-net-l2-0-gm-0";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network kpl-net-l2-1-gm-1";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network kpl-net-l2-2-gm-2";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network kpl-net-l2-3-gm-3";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network kpl-net-l2-4-gm-4";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network kpl-net-l2-5-gm-5";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);
}


void Timing::ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
		const std::string &section)
{
	// Read SM
	int sm_id = ini_file->ReadInt(section, "SM", -1);
	if (sm_id < 0)
		throw Error(misc::fmt("%s: section [%s]: invalid or missing "
				"value for 'SM'",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Check SM boundaries
	if (sm_id >= GPU::num_sms)
	{
		misc::Warning("%s: section [%s] ignored, referring to Kepler SM %d. "
				"This section refers to an SM that does not currently exist."
				" Please review your Kepler configuration file if this is not "
				"the desired behavior.",
				ini_file->getPath().c_str(),
				section.c_str(),
				sm_id);
		return;
	}

	// Check that entry has not been assigned before
	SM *sm = gpu->getSM(sm_id);
	if (sm->cache)
		throw misc::Error(misc::fmt("%s: section [%s]: entry from SM %d already "
				"assigned. A different [Entry <name>] section in the "
				"memory configuration file has already "
				"assigned an entry for this particular compute "
				"unit. Please review your configuration file "
				"to avoid duplicates.",
				ini_file->getPath().c_str(),
				section.c_str(),
				sm_id));

	// Read modules
	std::string cache_name;
	cache_name = ini_file->ReadString(section, "Module");

	// Assign cache
	mem::System *mem_system = mem::System::getInstance();
	sm->cache = mem_system->getModule(cache_name);
	if (!sm->cache)
		throw misc::Error(misc::fmt("%s: [%s]: '%s' is not a valid "
						"module name. The given module name must match "
						"a module declared in a section [Module <name>] "
						"in the memory configuration file.\n",
						ini_file->getPath().c_str(),
						section.c_str(),
						cache_name.c_str()));
	// Add modules to list of memory entries
	entry_modules.push_back(sm->cache);

	// Debug
	mem::System::debug << misc::fmt("\tKepler SM %d\n", sm_id)
		<< "\t\tEntry for mem -> "
		<< sm->cache->getName() << '\n'
		<< '\n';
}


void Timing::CheckMemoryConfiguration(misc::IniFile *ini_file)
{
	// Check that all SM have an entry to the memory hierarchy
	for (auto it = gpu->getSMsBegin(),
			e = gpu->getSMsEnd();
			it != e;
			++it)
	{
		SM *sm = it->get();
		if (!sm->cache)
			throw Error(misc::fmt("%s: Kepler SM %d has no entry to memory. "
					"Please add a new [Entry <name>] section in your "
					"memory configuration file to associate "
					"this compute unit with a memory module.\n",
					ini_file->getPath().c_str(),
					sm->getId()));
	}
}

void Timing::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Kepler");

	// Option --kpl-sim<kind>
	/*command_line->RegisterEnum("--kpl-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of Kepler simulation."); */

	// Option --kpl-config <file>
	command_line->RegisterString("--kpl-config <file>", config_file,
			"Configuration file for the Kepler GPU timing "
			"model, including parameters such as number of SM, "
			"cuda cores, or warp size. Type 'm2s --kpl-help' "
			"for details on the file format.");

	// Option --kpl-report <file>
	command_line->RegisterString("--kpl-report <file>", report_file,
			"File to dump a report of the Kepler GPU pipeline, including "
			"statistics such as active execution units, SM occupancy, "
			"cuda cores utilization, etc. Use together with a detailed "
			"GPU simulation (option '--kpl-sim detailed').");

	// Option --kpl-max-cycles <int>
	command_line->RegisterInt64("--kpl-max-cycles <cycles>", GPU::max_cycles,
			"Maximum number of cycles for the timing simulator "
			"to run.  If this maximum is reached, the simulation "
			"will finish with the KplMaxCycles string.");

	// Option --kpl-help
	command_line->RegisterBool("--kpl-help", help,
			"Display a help message describing the format of the "
			"Kepler GPU configuration file.");
}


void Timing::ProcessOptions()
{
	// Configuration file passed with option '--kpl-config'
	misc::IniFile ini_file;
	if (!config_file.empty())
		ini_file.Load(config_file);

	// Instantiate timing simulator if ' --kpl-sim detailed' is present
	if (Emulator::getSimKind() == comm::Arch::SimDetailed)
	{
		// First :: parse configuration. This initializes all configuration
		// static variables in class Timing, Gpu, SM, ...
		ParseConfiguration(&ini_file);

		// Second: generate instance of timing simulator. The constructor of
		// each class will use the previously initialized static variables to
		// initialize the instances.
		getInstance();
	}

	// Print configuration INI file format
	if (help)
	{
		misc::StringFormatter formatter(help_message);
		std::cerr << formatter;
		exit(0);
	}
}


void Timing::ParseConfiguration(misc::IniFile *ini_file)
{
	// Section [Device]
	std::string section = "Device";

	// Frenquency domain
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz. \n",
				ini_file->getPath().c_str()));
	GPU::num_sms = ini_file->ReadInt(section, "NumSMs",
				GPU::num_sms);
	GPU::shared_memory_size = ini_file->ReadInt(section, "SharedMemorySize",
			GPU::shared_memory_size);

	// Section [SM]
	section = "SM";
	SM::num_warp_pools = ini_file->ReadInt(section, "NumWarpPools",
			SM::num_warp_pools);
	SM::max_blocks_per_warp_pool = ini_file->ReadInt(section,
			"MaxBlocksPerWarpPool", SM::max_blocks_per_warp_pool);
	SM::max_warps_per_warp_pool = ini_file->ReadInt(section,
			"MaxWarpsPerWarpPool", SM::max_warps_per_warp_pool);

	// Section [FrontEnd]
	section = "FrontEnd";
	SM::fetch_latency = ini_file->ReadInt(section, "FetchLatency",
			SM::fetch_latency);
	SM::fetch_width = ini_file->ReadInt(section, "FetchWidth",
			SM::fetch_width);
	SM::fetch_buffer_size = ini_file->ReadInt(section, "FetchBufferSize",
			SM::fetch_buffer_size);
	SM::dispatch_latency = ini_file->ReadInt(section, "DispatchLatency",
			SM::dispatch_latency);
	SM::dispatch_width = ini_file->ReadInt(section, "DispatchWidth",
			SM::dispatch_width);
	SM::max_instructions_dispatched_per_type = ini_file->ReadInt(section,
			"MaxInstructionsDispatchedPerType",
			SM::max_instructions_dispatched_per_type);

	// Section [SPU]
	section = "SPU";
	SPU::num_spu_lanes = ini_file->ReadInt(section, "NumSPULanes",
			SPU::num_spu_lanes);
	SPU::width = ini_file->ReadInt(section, "Width", SPU::width);
	SPU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			SPU::dispatch_buffer_size);
	SPU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			SPU::read_latency);
	SPU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			SPU::read_buffer_size);
	SPU::execute_latency = ini_file->ReadInt(section, "ExecutionLatency",
			SPU::read_latency);
	SPU::execute_buffer_size = ini_file->ReadInt(section,
			"ExecutionBufferSize", SPU::execute_buffer_size);
	SPU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			SPU::write_latency);
	SPU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			SPU::write_buffer_size);

	// Section [DPU]
	section = "DPU";
	DPU::num_dpu_lanes = ini_file->ReadInt(section, "NumDPULanes",
			DPU::num_dpu_lanes);
	DPU::width = ini_file->ReadInt(section, "Width", DPU::width);
	DPU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			DPU::dispatch_buffer_size);
	DPU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			DPU::read_latency);
	DPU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			DPU::read_buffer_size);
	DPU::execute_latency = ini_file->ReadInt(section, "ExecutionLatency",
			DPU::read_latency);
	DPU::execute_buffer_size = ini_file->ReadInt(section,
			"ExecutionBufferSize", DPU::execute_buffer_size);
	DPU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			DPU::write_latency);
	DPU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			DPU::write_buffer_size);

	// Section [SFU]
	SFU::num_sfu_lanes = ini_file->ReadInt(section, "NumSFULanes",
			SFU::num_sfu_lanes);
	SFU::width = ini_file->ReadInt(section, "Width", SFU::width);
	SFU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			SFU::dispatch_buffer_size);
	SFU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			SFU::read_latency);
	SFU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			SFU::read_buffer_size);
	SFU::execute_latency = ini_file->ReadInt(section, "ExecutionLatency",
			SFU::read_latency);
	SFU::execute_buffer_size = ini_file->ReadInt(section,
			"ExecutionBufferSize", SFU::execute_buffer_size);
	SFU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			SFU::write_latency);
	SPU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			SPU::write_buffer_size);

	// Section [IMU]
	IMU::num_imu_lanes = ini_file->ReadInt(section, "NumIMULanes",
			IMU::num_imu_lanes);
	IMU::width = ini_file->ReadInt(section, "Width", IMU::width);
	IMU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			IMU::dispatch_buffer_size);
	IMU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			IMU::read_latency);
	IMU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			IMU::read_buffer_size);
	IMU::execute_latency = ini_file->ReadInt(section, "ExecutionLatency",
			IMU::read_latency);
	IMU::execute_buffer_size = ini_file->ReadInt(section,
			"ExecutionBufferSize", IMU::execute_buffer_size);
	IMU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			IMU::write_latency);
	IMU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			IMU::write_buffer_size);

	// Section [BRU]
	BRU::num_bru_lanes = ini_file->ReadInt(section, "NumBRULanes",
			BRU::num_bru_lanes);
	BRU::width = ini_file->ReadInt(section, "Width", BRU::width);
	BRU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			BRU::dispatch_buffer_size);
	BRU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			BRU::read_latency);
	BRU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			BRU::read_buffer_size);
	BRU::execute_latency = ini_file->ReadInt(section, "ExecutionLatency",
			BRU::execute_latency);
	BRU::execute_buffer_size = ini_file->ReadInt(section,
			"ExecutionBufferSize", BRU::execute_buffer_size);
	BRU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			BRU::write_latency);
	BRU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			BRU::write_buffer_size);

	// Section [LSU]
	LSU::width = ini_file->ReadInt(section, "Width", LSU::width);
	LSU::dispatch_buffer_size = ini_file->ReadInt(section, "DispatchBufferSize",
			LSU::dispatch_buffer_size);
	LSU::read_latency = ini_file->ReadInt(section, "ReadLatency",
			LSU::read_latency);
	LSU::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
			LSU::read_buffer_size);
	LSU::max_inflight_mem_accesses = ini_file->ReadInt(section,
			"MaxInflightMemAccesses", LSU::max_inflight_mem_accesses);
	LSU::write_latency = ini_file->ReadInt(section, "WriteLatency",
			LSU::write_latency);
	LSU::write_buffer_size = ini_file->ReadInt(section, "WriteBufferSize",
			LSU::write_buffer_size);
}


void Timing::DumpConfiguration(std::ofstream &os)
{
	// Device
	os << misc::fmt("[ Config.Device]\n");
	os << misc::fmt("Frequency = %d\n", frequency);
	os << misc::fmt("NumSM = %d\n", GPU::num_sms);
	os << misc::fmt("\n");

	// SM
	os << misc::fmt("[ Config.SM ]\n");
	os << misc::fmt("NumWarpPools = %d\n", SM::num_warp_pools);
	os << misc::fmt("NumRegisters = %d\n", GPU::num_registers);
	os << misc::fmt("MaxBlocksPerWarpPool = %d\n",
			SM::max_blocks_per_warp_pool);
	os << misc::fmt("MaxWarpsPerWarpPool = %d\n", SM::max_warps_per_warp_pool);
	os << misc::fmt("\n");

	// Front End
	os << misc::fmt("[ Config.FrontEnd ]\n");
	os << misc::fmt("FetchLatency = %d\n", SM::fetch_latency);
	os << misc::fmt("FetchBufferSize = %d\n", SM::fetch_buffer_size);
	os << misc::fmt("DispatchLatency = %d\n", SM::dispatch_latency);
	os << misc::fmt("DispatchWidth = %d\n", SM::dispatch_width);
	os << misc::fmt("MaxInstDispatchedPerType = %d\n",
			SM::max_instructions_dispatched_per_type);
	os << misc::fmt("\n");

	// SPU
	os << misc::fmt("[ Config.SPU ]\n");
	os << misc::fmt("NumSPULanes = %d\n", SPU::num_spu_lanes);
	os << misc::fmt("Width = %d\n", SPU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", SPU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", SPU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", SPU::read_buffer_size);
	os << misc::fmt("ExecutionLatency = %d\n", SPU::execute_latency);
	os << misc::fmt("ExecutionBufferSize = %d\n", SPU::execute_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", SPU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", SPU::write_buffer_size);
	os << misc::fmt("\n");

	// DPU
	os << misc::fmt("[ Config.DPU ]\n");
	os << misc::fmt("NumSPULanes = %d\n", DPU::num_dpu_lanes);
	os << misc::fmt("Width = %d\n", DPU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", DPU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", DPU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", DPU::read_buffer_size);
	os << misc::fmt("ExecutionLatency = %d\n", DPU::execute_latency);
	os << misc::fmt("ExecutionBufferSize = %d\n", DPU::execute_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", DPU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", DPU::write_buffer_size);
	os << misc::fmt("\n");

	// SFU
	os << misc::fmt("[ Config.SFU ]\n");
	os << misc::fmt("NumSPULanes = %d\n", SFU::num_sfu_lanes);
	os << misc::fmt("Width = %d\n", SFU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", SFU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", SFU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", SFU::read_buffer_size);
	os << misc::fmt("ExecutionLatency = %d\n", SFU::execute_latency);
	os << misc::fmt("ExecutionBufferSize = %d\n", SFU::execute_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", SFU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", SFU::write_buffer_size);
	os << misc::fmt("\n");

	// IMU
	os << misc::fmt("[ Config.IMU ]\n");
	os << misc::fmt("NumSPULanes = %d\n", IMU::num_imu_lanes);
	os << misc::fmt("Width = %d\n", IMU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", IMU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", IMU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", IMU::read_buffer_size);
	os << misc::fmt("ExecutionLatency = %d\n", IMU::execute_latency);
	os << misc::fmt("ExecutionBufferSize = %d\n", IMU::execute_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", IMU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", IMU::write_buffer_size);
	os << misc::fmt("\n");

	// BRU
	os << misc::fmt("[ Config.BRU ]\n");
	os << misc::fmt("NumSPULanes = %d\n", BRU::num_bru_lanes);
	os << misc::fmt("Width = %d\n", BRU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", BRU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", BRU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", BRU::read_buffer_size);
	os << misc::fmt("ExecutionLatency = %d\n", BRU::execute_latency);
	os << misc::fmt("ExecutionBufferSize = %d\n", BRU::execute_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", BRU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", BRU::write_buffer_size);
	os << misc::fmt("\n");

	// LSU
	os << misc::fmt("[ Config.LSU ]\n");
	os << misc::fmt("Width = %d\n", SPU::width);
	os << misc::fmt("DispatchBufferSize = %d\n", SPU::dispatch_buffer_size);
	os << misc::fmt("ReadLatency = %d\n", SPU::read_latency);
	os << misc::fmt("ReadBufferSize = %d\n", SPU::read_buffer_size);
	os << misc::fmt("WriteLatency = %d\n", SPU::write_latency);
	os << misc::fmt("WriteBufferSize = %d\n", SPU::write_buffer_size);
	os << misc::fmt("\n");

	// End of configuration
	os << misc::fmt("\n");
}


void Timing::DumpReport()
{
	// Check if the reprot file have been set
	if (report_file.empty())
		return;

	// Open file for writing
	std::ofstream report;
	report.open(report_file);

	// Dump GPU configuration
	report << misc::fmt(";\n; GPU Configuration\n; \n\n");
	DumpConfiguration(report);

	// Report for device
	report << misc::fmt(";\n; Simulation Statistics\n;\n\n");
	Emulator *emulator = Emulator::getInstance();
	double instructions_per_cycle = getCycle() ?
		(double)(emulator->getNumInstructions()/this->getCycle()) : 0.0;
	report << misc::fmt("[ Device ]\n\n");
	report << misc::fmt("GridCount = %d\n", emulator->getNumGrids());
	report << misc::fmt("Instructions = %lld\n",
			emulator->getNumInstructions()); // TODO Blocks, classify instructions
	report << misc::fmt("Cycles = %lld\n", getCycle());
	report << misc::fmt("instructionsPerCycle = %.4g\n",
			instructions_per_cycle);
	report << misc::fmt("\n\n");

	// Report for SM
	for (auto it = gpu->getSMsBegin(), e = gpu->getSMsEnd(); it != e; ++it)
	{
		// Calculate relevant values for each SM
		SM *sm = it->get();
		instructions_per_cycle = getCycle() ?
			(double)(sm->num_total_instructions/getCycle()) : 0.0;

		// Report statistics for each SM
		report << misc::fmt("[ SM %d ]\n\n", sm->getId());
		report << misc::fmt("BlockCount = %lld\n", sm->num_mapped_blocks);
		report << misc::fmt("SPU Instructions = %lld\n",
				sm->num_spu_instructions);
		report << misc::fmt("DPU Instructions = %lld\n",
				sm->num_dpu_instructions);
		report << misc::fmt("SFU Instructions = %lld\n",
				sm->num_sfu_instructions);
		report << misc::fmt("IMU Instructions = %lld\n",
				sm->num_imu_instructions);
		report << misc::fmt("BRU Instructions = %lld\n",
				sm->num_branch_instructions);
		report << misc::fmt("LDS Instructions = %lld\n",
				sm->num_memory_instructions);
		report << misc::fmt("Cycles = %lld\n", getCycle());
		report << misc::fmt("InstructionsPerCycle = %.4g\n",
				instructions_per_cycle);
		report << misc::fmt("RegistersReads = %lld\n", sm->num_regiters_reads);
		report << misc::fmt("RegistersWrites = %lld\n", sm->num_regiters_reads);
		report << misc::fmt("\n\n");
	}

	// Close the report file
	report.close();
}


bool Timing::Run()
{
	// For Efficiency when no Kepler emulation is selected, exit here if the
	// list of existing Grid is empty;
	if (!this->getNumGrids())
		return false;
	else
	//if (emulator->getNumPendingGrids())
	{
		// Get grid
		//Grid *grid = *emulator->getPendingGridsBegin();

		Grid *grid = grids.begin()->get();

		while (gpu->getNumAvailableSMs() > 0)
		{
			// Check if there is available SM
			if(grid->getNumMappedBlocks() < grid->getThreadBlockCount())
			{
				// Setup Block pointer
				//std::unique_ptr<ThreadBlock> thread_block;

				ThreadBlock *thread_block;

				// Block ID and Block 3D ID
				int thread_block_id;
				unsigned thread_block_3d_id[3];

				// Map one block to SMs
				thread_block_id = grid->getNumMappedBlocks();

				// Remove blocks from list and get its ID
				// Threadblock.X
				thread_block_3d_id[0] = thread_block_id /
						(grid->getThreadBlockCount3(1) *
								grid->getThreadBlockCount3(2));

				// Threadblock.Y
				thread_block_3d_id[1] = (thread_block_id %
						(grid->getThreadBlockCount3(1) *
								grid->getThreadBlockCount3(2))) /
									grid->getThreadBlockCount3(2);
				// ThreadBlock.Z
				thread_block_3d_id[2] = ((thread_block_id %
						(grid->getThreadBlockCount3(1) *
								grid->getThreadBlockCount3(2))) %
									grid->getThreadBlockCount3(2))  ;

				grid->WaitingToRunning(thread_block_id, thread_block_3d_id);

			//	thread_block.reset(grid->getRunningThreadBlocksBegin()
				//	->release());

				auto it = grid->getRunningThreadBlocksBegin();
				std::advance(it,thread_block_id);
				thread_block = it->get();

				// Get an available SM
				SM *sm;
				sm = gpu->getAvailableSM();
				assert(sm);

				// Remove it from the available SM list. It will be
				// re-added later if it still has room for more blocks;
				gpu->RemoveFromAvailableSMs(sm);

				// Maps the block to a SM
				sm->MapBlock(thread_block);

				//Increase number of mapped thread block in the grid by one
				grid->IncreseNumMappedBlocks();

			//	if (grid->getNumMappedBlocks() == grid->getThreadBlockCount())
				//{
					//emulator->PopPendingGrid();
					//break;
				//}
			}
			else if (grid->getNumMappedBlocks() == grid->getThreadBlockCount())
				break;
		}
	}

	// Stop if maximum number of GPU cycles exceeded
	esim::Engine *esim_engine = esim::Engine::getInstance();
	if (GPU::max_cycles && getCycle() >=
			GPU::max_cycles)
		esim_engine->Finish("KplMaxCycles");

	// TODO Stop if maximum number of GPU instructions exceeded

	// Stop if there was a simulation stall
	if (gpu->last_complete_cycle && gpu->last_complete_cycle > 1000000)
	{
		std::cout<<"\n\n************TOO LONG******************\n\n";
		esim_engine->Finish("KplStall");
	}

	// Stop if any reason met
	if (esim_engine->hasFinished())
	{
		std::cout<<"esim finished"<<std::endl;
		return true;

	}

	// Run on loop iteration on each busy computer unit
	gpu->Run();

	// Still running
	return true;
}

} // namespace Kepler
