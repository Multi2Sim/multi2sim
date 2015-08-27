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
#include <arch/southern-islands/emulator/Emulator.h>

#include "ComputeUnit.h"
#include "Timing.h"


namespace SI
{

// Singleton instance
std::unique_ptr<Timing> Timing::instance;



//
// Configuration options
//

std::string Timing::config_file;

comm::Arch::SimKind Timing::sim_kind = comm::Arch::SimFunctional;

std::string Timing::report_file;

esim::Trace Timing::trace;

const std::string Timing::help_message =
	"The Southern Islands GPU configuration file is a plain text INI file\n"
	"defining the parameters of the Southern Islands model for a detailed\n"
	"(architectural) configuration. This file is passed to Multi2Sim with\n"
	"the '--si-config <file>' option, and should always be used together \n"
	"with option '--si-sim detailed'.\n" 
	"\n"
	"The following is a list of the sections allowed in the GPU "
	"configuration\n"
	"file, along with the list of variables for each section.\n"
	"\n"
	"Section '[ Device ]': parameters for the GPU.\n"
	"\n"
	"  Frequency = <value> (Default = 1000)\n"
	"      Frequency for the Southern Islands GPU in MHz.\n"
	"  NumComputeUnits = <num> (Default = 32)\n"
	"      Number of compute units in the GPU.\n"
	"\n"
	"Section '[ ComputeUnit ]': parameters for the Compute Units.\n"
	"\n"
	"  NumWavefrontPools = <num> (Default = 4)\n"
	"      Number of wavefront pools/SIMDs per compute unit.\n"
	"  MaxWorkGroupsPerWavefrontPool = <num> (Default = 10)\n"
	"      The maximum number of work groups that can be scheduled to a\n"
	"      wavefront pool at a time.\n"
	"  MaxWavefrontsPerWavefrontPool = <num> (Default = 10)\n"
	"      The maximum number of wavefronts that can be scheduled to a\n"
	"      wavefront pool at a time.\n"
	"  NumVectorRegisters = <num> (Default = 65536)\n"
	"      Number of vector registers per compute unit. These are\n"
	"      divided evenly between all wavefront pools/SIMDs.\n"
	"  NumScalarRegisters = <num> (Default = 2048)\n"
	"      Number of scalar registers per compute unit. These are\n"
	"      shared by all wavefront pools/SIMDs.\n"
	"\n"
	"Section '[ FrontEnd ]': parameters for fetch and issue.\n"
	"\n"
	"  FetchLatency = <cycles> (Default = 5)\n"
	"      Latency of instruction memory in number of cycles.\n"
	"  FetchWidth = <num> (Default = 4)\n"
	"      Maximum number of instructions fetched per cycle.\n"
	"  FetchBufferSize = <num> (Default = 10)\n"
	"      Size of the buffer holding fetched instructions.\n"
	"  IssueLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  IssueWidth = <num> (Default = 5)\n"
	"      Number of instructions that can be issued per cycle.\n"
	"  MaxInstIssuedPerType = <num> (Default = 1)\n"
	"      Maximum number of instructions that can be issued of each type\n"
	"      (SIMD, scalar, etc.) in a single cycle.\n"
	"\n"
	"Section '[ SIMDUnit ]': parameters for the SIMD Units.\n"
	"\n"
	"  NumSIMDLanes = <num> (Default = 16)\n"
	"      Number of lanes per SIMD.  This must divide the wavefront\n"
	"      size (64) evenly.\n" 
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  DecodeBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding decoded instructions.\n"
	"  ReadExecWriteLatency = <cycles> (Default = 8)\n"
	"      Number of cycles it takes to read operands from the register\n"
	"      files, execute the SIMD ALU operation, and write the results\n"
	"      out to the register file for a single subwavefront. It makes\n"
	"      sense to combine the three stages since they wavefront is\n"
	"      pipelined across all of them and can therefore be in different\n"
	"      stages at the same time.\n"
	"  ReadExecWriteBufferSize = <num> (Default = 2)\n"
	"      Size of the buffer holding instructions that have began the\n"
	"      read-exec-write stages.\n"
	"\n"
	"Section '[ ScalarUnit ]': parameters for the Scalar Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  ALULatency = <cycles> (Default = 4)\n"
	"      Latency of ALU execution in number of cycles.\n"
	"  ExecBufferSize = <num> (Default = 16)\n"
	"      Size of the buffer holding in-flight memory instructions and\n"
	"      executing ALU instructions.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ BranchUnit ]': parameters for the Branch Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  ExecLatency = <cycles> (Default = 1)\n"
	"      Latency of execution in number of cycles.\n"
	"  ExecBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding executing instructions.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ LDSUnit ]': parameters for the LDS Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  MaxInflightMem = <num> (Default = 32)\n"
	"      Maximum number of in-flight memory accesses.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ VectorMemUnit ]': parameters for the Vector Memory Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  MaxInflightMem = <num> (Default = 32)\n"
	"      Maximum number of in-flight memory accesses.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ LDS ]': defines the parameters of the Local Data Share\n"
	"on each compute unit.\n"
	"\n"
	"  Size = <bytes> (Default = 64 KB)\n"
	"      LDS capacity per compute unit. This value must be\n"
	"      equal to or larger than BlockSize * Banks.\n"
	"  AllocSize = <bytes> (Default = 16)\n"
	"      Minimum amount of LDS memory allocated at a time for\n"
	"      each work-group.\n" 
	"  BlockSize = <bytes> (Default = 64)\n"
	"      Access block size, used for access coalescing purposes\n"
	"      among work-items.\n"
	"  Latency = <num_cycles> (Default = 2)\n"
	"      Latency for an access in number of cycles.\n"
	"  Ports = <num> (Default = 4)\n"
	"      Number of ports.\n"
	"\n";

bool Timing::help = false;

int Timing::frequency = 1000;
	
	
Timing::Timing() : comm::Timing("SouthernIslands")
{
	// Configure frequency domain with the frequency given by the user
	ConfigureFrequencyDomain(frequency);

	// Create GPU
	gpu = misc::new_unique<Gpu>();
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


void Timing::WriteMemoryConfiguration(misc::IniFile *ini_file)
{
	// Cache geometry for vector L1
	std::string section = "CacheGeometry si-geo-vector-l1";
	ini_file->WriteInt(section, "Sets", 64);
	ini_file->WriteInt(section, "Assoc", 4);
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 1);
	ini_file->WriteString(section, "Policy", "LRU");

	// Cache geometry for scalar L1
	section = "CacheGeometry si-geo-scalar-l1";
	ini_file->WriteInt(section, "Sets", 64);
	ini_file->WriteInt(section, "Assoc", 4);
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 1);
	ini_file->WriteString(section, "Policy", "LRU");

	// Cache geometry for L2
	section = "CacheGeometry si-geo-l2";
	ini_file->WriteInt(section, "Sets", 128);
	ini_file->WriteInt(section, "Assoc", 16);
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 10);
	ini_file->WriteString(section, "Policy", "LRU");

	// Create scalar L1 caches
	for (int i = 0; i < (Gpu::num_compute_units + 3) / 4; i++)
	{
		section = misc::fmt("Module si-scalar-l1-%d", i);
		ini_file->WriteString(section, "Type", "Cache");
		ini_file->WriteString(section, "Geometry", 
			"si-geo-scalar-l1");
		ini_file->WriteString(section, "LowNetwork", 
			"si-net-l1-l2");
		ini_file->WriteString(section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	// Create vector L1 caches
	for (int i = 0; i < Gpu::num_compute_units; i++)
	{
		section = misc::fmt("Module si-vector-l1-%d", i);
		ini_file->WriteString(section, "Type", "Cache");
		ini_file->WriteString(section, "Geometry", 
			"si-geo-vector-l1");
		ini_file->WriteString(section, "LowNetwork", 
			"si-net-l1-l2");
		ini_file->WriteString(section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	// Create entries from compute units to L1s
	for (int i = 0; i < Gpu::num_compute_units; i++)
	{
		// Entry
		section = misc::fmt("Entry si-cu-%d", i);
		ini_file->WriteString(section, "Arch", "SouthernIslands");
		ini_file->WriteInt(section, "ComputeUnit", i);

		std::string value = misc::fmt("si-vector-l1-%d", i);
		ini_file->WriteString(section, "DataModule", value);
		
		value = misc::fmt("si-scalar-l1-%d", i / 4);
		ini_file->WriteString(section, "ConstantDataModule", value);
	}

	// L2 caches
	section = "Module si-l2-0";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-0-gm-0");
	ini_file->WriteString(section, "LowModules", "si-gm-0");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 0");

	section = "Module si-l2-1";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-1-gm-1");
	ini_file->WriteString(section, "LowModules", "si-gm-1");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 1");
	
	section = "Module si-l2-2";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-2-gm-2");
	ini_file->WriteString(section, "LowModules", "si-gm-2");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 2");

	section = "Module si-l2-3";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-3-gm-3");
	ini_file->WriteString(section, "LowModules", "si-gm-3");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 3");

	section = "Module si-l2-4";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-4-gm-4");
	ini_file->WriteString(section, "LowModules", "si-gm-4");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 4");

	section = "Module si-l2-5";
	ini_file->WriteString(section, "Type", "Cache");
	ini_file->WriteString(section, "Geometry", "si-geo-l2");
	ini_file->WriteString(section, "HighNetwork", "si-net-l1-l2");
	ini_file->WriteString(section, "LowNetwork", "si-net-l2-5-gm-5");
	ini_file->WriteString(section, "LowModules", "si-gm-5");
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 5");

	// Global memory
	section = "Module si-gm-0";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-0-gm-0");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 0");

	section = "Module si-gm-1";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-1-gm-1");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 1");

	section = "Module si-gm-2";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-2-gm-2");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 2");

	section = "Module si-gm-3";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-3-gm-3");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 3");

	section = "Module si-gm-4";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-4-gm-4");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 4");

	section = "Module si-gm-5";
	ini_file->WriteString(section, "Type", "MainMemory");
	ini_file->WriteString(section, "HighNetwork", "si-net-l2-5-gm-5");
	ini_file->WriteInt(section, "BlockSize", 64);
	ini_file->WriteInt(section, "Latency", 100);
	ini_file->WriteString(section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 5");

	// Network connecting L1s and L2s
	section = "Network si-net-l1-l2";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	// Networks connecting memory controllers and global memory banks
	section = "Network si-net-l2-0-gm-0";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network si-net-l2-1-gm-1";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network si-net-l2-2-gm-2";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network si-net-l2-3-gm-3";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network si-net-l2-4-gm-4";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);

	section = "Network si-net-l2-5-gm-5";
	ini_file->WriteInt(section, "DefaultInputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultOutputBufferSize", 528);
	ini_file->WriteInt(section, "DefaultBandwidth", 264);
}


void Timing::ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
		const std::string &section)
{
	// Allow these sections in case we quit before reading them.
	ini_file->Allow(section, "DataModule");
	ini_file->Allow(section, "ConstantDataModule");
	ini_file->Allow(section, "Module");

	// Unified or separate data and constant memory
	bool unified_present = ini_file->Exists(section, "Module");
	bool separate_present = ini_file->Exists(section, "DataModule") &&
			ini_file->Exists(section, "ConstantDataModule");
	if (!unified_present && !separate_present)
		throw Error(misc::fmt("%s: section [%s]: "
				"variable 'Module' missing.",
				ini_file->getPath().c_str(),
				section.c_str()));
	if (!(unified_present ^ separate_present))
		throw Error(misc::fmt("%s: section [%s]: invalid combination "
				"of modules. A Southern Islands entry to the "
				"memory hierarchy needs to specify either a "
				"unified entry for vector and scalar caches "
				"(variable 'Module'), or two separate entries "
				"for data and scalar (constant) data "
				"(variables 'DataModule' and "
				"'ConstantDataModule'), but not both.\n",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Read compute unit
	int compute_unit_id = ini_file->ReadInt(section, "ComputeUnit", -1);
	if (compute_unit_id < 0)
		throw Error(misc::fmt("%s: section [%s]: invalid or missing "
				"value for 'ComputeUnit'",
				ini_file->getPath().c_str(),
				section.c_str()));

	// Check compute unit boundaries
	if (compute_unit_id >= Gpu::num_compute_units)
	{
		misc::Warning("%s: section [%s] ignored, referring "
				"to Southern Islands compute unit %d. This "
				"section refers to a compute unit that does "
				"not currently exist. Please review your "
				"Southern Islands configuration file if this "
				"is not the desired behavior.",
				ini_file->getPath().c_str(),
				section.c_str(),
				compute_unit_id);
		return;
	}

	// Check that entry has not been assigned before
	ComputeUnit *compute_unit = gpu->getComputeUnit(compute_unit_id);
	if (compute_unit->vector_cache)
		throw misc::Error(misc::fmt("%s: section [%s]: entry from "
				"compute unit %d already assigned. "
				"A different [Entry <name>] section in the "
				"memory configuration file has already "
				"assigned an entry for this particular compute "
				"unit. Please review your configuration file "
				"to avoid duplicates.",
				ini_file->getPath().c_str(),
				section.c_str(),
				compute_unit_id));

	// Read modules
	std::string vector_cache_name;
	std::string scalar_cache_name;
	if (separate_present)
	{
		vector_cache_name = ini_file->ReadString(section, "DataModule");
		scalar_cache_name = ini_file->ReadString(section, "ConstantDataModule");
	}
	else
	{
		vector_cache_name = scalar_cache_name =
				ini_file->ReadString(section, "Module");
	}
	if (vector_cache_name.empty() || scalar_cache_name.empty())
		throw misc::Error(misc::fmt("%s: [%s]: invalid name for vector "
				"or scalar cache",
				ini_file->getPath().c_str(),
				section.c_str()));
	
	// Assign vector cache
	mem::System *mem_system = mem::System::getInstance();
	compute_unit->vector_cache = mem_system->getModule(vector_cache_name);
	if (!compute_unit->vector_cache)
		throw misc::Error(misc::fmt("%s: [%s]: '%s' is not a valid "
				"module name. The given module name must match "
				"a module declared in a section [Module <name>] "
				"in the memory configuration file.\n",
				ini_file->getPath().c_str(),
				section.c_str(),
				vector_cache_name.c_str()));
	
	// Assign scalar cache
	compute_unit->scalar_cache = mem_system->getModule(scalar_cache_name);
	if (!compute_unit->scalar_cache)
		throw misc::Error(misc::fmt("%s: [%s]: '%s' is not a valid "
				"module name: The given module name must match "
				"a module declared in a section [Module <name>] "
				"in the memory configuration file.\n",
				ini_file->getPath().c_str(),
				section.c_str(),
				scalar_cache_name.c_str()));
	
	// Add modules to list of memory entries
	entry_modules.push_back(compute_unit->vector_cache);
	entry_modules.push_back(compute_unit->scalar_cache);
	
	// Debug
	mem::System::debug << misc::fmt("\tSouthern Islands compute unit %d\n",
			compute_unit_id)
			<< "\t\tEntry for vector mem -> "
			<< compute_unit->vector_cache->getName() << '\n'
			<< "\t\tEntry for scalar mem -> "
			<< compute_unit->scalar_cache->getName() << '\n'
			<< '\n';
}


void Timing::CheckMemoryConfiguration(misc::IniFile *ini_file)
{
	// Check that all compute units have an entry to the memory hierarchy.
	for (auto it = gpu->getComputeUnitsBegin(),
			e = gpu->getComputeUnitsEnd();
			it != e;
			++it)
	{
		ComputeUnit *compute_unit = it->get();
		if (!compute_unit->vector_cache)
			throw Error(misc::fmt("%s: Southern Islands compute "
					"unit %d has no entry to memory. Please "
					"add a new [Entry <name>] section in your "
					"memory configuration file to associate "
					"this compute unit with a memory module.\n",
					ini_file->getPath().c_str(),
					compute_unit->getIndex()));
	}
}


void Timing::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Southern Islands");

	// Option --si-sim <kind>
	command_line->RegisterEnum("--si-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::Arch::SimKindMap,
			"Level of accuracy of Southern Islands simulation.");

	// Option --si-config <file>
	command_line->RegisterString("--si-config <file>", config_file,
			"Configuration file for the Southern Islands GPU timing "
			"model, including parameters such as number of compute "
			"units, stream cores, or wavefront size. Type 'm2s "
			"--si-help' for details on the file format.");

	// Option --si-report <file>
	command_line->RegisterString("--si-report <file>", report_file,
			"File to dump a report of the GPU pipeline, including "
			"statistics such as active execution units, compute "
			"unit occupancy, stream cores utilization, etc. Use "
			"together with a detailed GPU simulation (option "
			"'--si-sim detailed').");

	// Option --si-max-cycles <int>
	command_line->RegisterInt64("--si-max-cycles <cycles>", Gpu::max_cycles,
			"Maximum number of cycles for the timing simulator "
			"to run.  If this maximum is reached, the simulation "
			"will finish with the SIMaxCycles string.");

	// Option --si-help
	command_line->RegisterBool("--si-help", help,
			"Display a help message describing the format of the "
			"Southern Islands GPU configuration file.");
}


void Timing::ProcessOptions()
{
	// Configuration file passed with option '--si-config'
	misc::IniFile ini_file;
	if (!config_file.empty())
		ini_file.Load(config_file);
		
	// Instantiate timing simulator if '--si-sim detailed' is present
	if (sim_kind == comm::Arch::SimDetailed)
	{
		// First: parse configuration. This initializes all configuration
		// static variables in class Timing, Gpu, ComputeUnit, ...
		ParseConfiguration(&ini_file);

		// Second: generate instance of timing simulator. The constructor
		// of each class will use the previously initialized static
		// variables to initialize the instances. 
		Timing *timing = getInstance();

		// Initialize the memory structures according to the default
		// memory configuration.
		timing->WriteMemoryConfiguration(&ini_file);
		
		// Parse the memory configuration. This will initialize the 
		// scalar and vector caches of the compute units.
		std::string section;
		for (int i = 0; i < Gpu::num_compute_units; i++)
		{
			section = misc::fmt("Entry si-cu-%d", i);
			timing->ParseMemoryConfigurationEntry(&ini_file, 
					section);
		}
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

	// Frequency domain
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz.\n",
				ini_file->getPath().c_str()));
	Gpu::num_compute_units = ini_file->ReadInt(section, "NumComputeUnits",
						   Gpu::num_compute_units);

	// Section [ComputeUnit]
	section = "ComputeUnit";
	ComputeUnit::num_wavefront_pools = ini_file->ReadInt(section, "NumWavefrontPools",
					ComputeUnit::num_wavefront_pools);
	ComputeUnit::max_work_groups_per_wavefront_pool = ini_file->ReadInt(section,
					"MaxWorkGroupsPerWavefrontPool",
					ComputeUnit::max_work_groups_per_wavefront_pool);
	ComputeUnit::max_wavefronts_per_wavefront_pool = ini_file->ReadInt(section,
					"MaxWavefrontsPerWavefrontPool",
					ComputeUnit::max_wavefronts_per_wavefront_pool);
	//TODO ComputeUnit::num_vector_registers
	//TODO ComputeUnit::num_scalar_register

	// Section [FrontEnd]
	section = "FrontEnd";
	ComputeUnit::fetch_latency = ini_file->ReadInt(section, "FetchLatency",
					ComputeUnit::fetch_latency);
	ComputeUnit::fetch_width = ini_file->ReadInt(section, "FetchWidth",
					ComputeUnit::fetch_width);
	ComputeUnit::fetch_buffer_size = ini_file->ReadInt(section, "FetchBufferSize",
					ComputeUnit::fetch_buffer_size);
	ComputeUnit::issue_latency = ini_file->ReadInt(section, "IssueLatency",
					ComputeUnit::issue_latency);
	ComputeUnit::issue_width = ini_file->ReadInt(section, "IssueWidth",
					ComputeUnit::issue_width);
	ComputeUnit::max_instructions_issued_per_type = ini_file->ReadInt(section,
					"MaxInstructionsIssuedPerType",
					ComputeUnit::max_instructions_issued_per_type);

	// Section [SimdUnit]
	section = "SimdUnit";
	SimdUnit::num_simd_lanes = ini_file->ReadInt(section, "NumSIMDLanes",
					SimdUnit::num_simd_lanes);
	SimdUnit::width = ini_file->ReadInt(section, "Width", SimdUnit::width);
	SimdUnit::issue_buffer_size = ini_file->ReadInt(section, "IssueBufferSize",
					SimdUnit::issue_buffer_size);
	SimdUnit::decode_latency = ini_file->ReadInt(section, "DecodeLatency",
					SimdUnit::decode_latency);
	// TODO SimdUnit::decode_width
	SimdUnit::decode_buffer_size = ini_file->ReadInt(section, "DecodeBufferSize",
					SimdUnit::decode_buffer_size);
	SimdUnit::read_exec_write_latency = ini_file->ReadInt(section,
					"ReadExecWriteLatency",
					SimdUnit::read_exec_write_latency);
	SimdUnit::read_exec_write_buffer_size = ini_file->ReadInt(section,
					"ReadExecWriteBufferSize",
					SimdUnit::read_exec_write_buffer_size);

	// Section [ScalarUnit]
	section = "ScalarUnit";
	ScalarUnit::width = ini_file->ReadInt(section, "Width", ScalarUnit::width);
	ScalarUnit::issue_buffer_size = ini_file->ReadInt(section, "IssueBufferSize",
					ScalarUnit::issue_buffer_size);
	ScalarUnit::decode_latency = ini_file->ReadInt(section, "DecodeLatency",
					ScalarUnit::decode_latency);
	// TODO ScalarUnit::decode_width
	ScalarUnit::read_latency = ini_file->ReadInt(section, "ReadLatency",
					ScalarUnit::read_latency);
	ScalarUnit::read_buffer_size = ini_file->ReadInt(section,
					"ReadBufferSize",
					ScalarUnit::read_buffer_size);
	// TODO ScalarUnit::alu_latency
	ScalarUnit::exec_buffer_size = ini_file->ReadInt(section,
					"ExecBufferSize",
					ScalarUnit::exec_buffer_size);
	ScalarUnit::write_latency = ini_file->ReadInt(section, "WriteLatency",
					ScalarUnit::write_latency);
	ScalarUnit::write_buffer_size = ini_file->ReadInt(section,
					"write_buffer_size",
					ScalarUnit::write_buffer_size);

	// Section [BranchUnit]
	section = "BranchUnit";
	BranchUnit::width = ini_file->ReadInt(section, "Width", BranchUnit::width);
	BranchUnit::issue_buffer_size = ini_file->ReadInt(section,
					"IssueBufferSize",
					BranchUnit::issue_buffer_size);
	BranchUnit::decode_latency = ini_file->ReadInt(section, "DecodeLatency",
					BranchUnit::decode_latency);
	// TODO BranchUnit::decode_width
	BranchUnit::read_latency = ini_file->ReadInt(section, "ReadLatency",
					BranchUnit::read_latency);
	BranchUnit::read_buffer_size = ini_file->ReadInt(section, "ReadBufferSize",
					BranchUnit::read_buffer_size);
	BranchUnit::exec_latency = ini_file->ReadInt(section, "ExecLatency",
					BranchUnit::exec_latency);
	BranchUnit::exec_buffer_size = ini_file->ReadInt(section, "ExecBufferSize",
					BranchUnit::exec_buffer_size);
	BranchUnit::write_latency = ini_file->ReadInt(section, "WriteLatency",
					BranchUnit::write_latency);
	BranchUnit::write_buffer_size = ini_file->ReadInt(section,
					"WriteBufferSize",
					BranchUnit::write_buffer_size);

	// Section [LDSUnit]
	section = "LDSUnit";
	LdsUnit::width = ini_file->ReadInt(section, "Width", LdsUnit::width);
	LdsUnit::issue_buffer_size = ini_file->ReadInt(section, "IssueBufferSize",
					LdsUnit::issue_buffer_size);
	LdsUnit::decode_latency = ini_file->ReadInt(section, "DecodeLatency",
					LdsUnit::decode_latency);
	// TODO LdsUnit::decode_width
	LdsUnit::read_latency = ini_file->ReadInt(section, "ReadLatency",
					LdsUnit::read_latency);
	LdsUnit::read_buffer_size = ini_file->ReadInt(section,
					"ReadBufferSize",
					LdsUnit::read_buffer_size);
	LdsUnit::max_in_flight_mem_accesses = ini_file->ReadInt(section,
					"MaxInflightMem",
					LdsUnit::max_in_flight_mem_accesses);
	LdsUnit::write_latency = ini_file->ReadInt(section, "WriteLatency",
					LdsUnit::write_latency);
	LdsUnit::write_buffer_size = ini_file->ReadInt(section,
					"WriteBufferSize",
					LdsUnit::write_buffer_size);

	// Section [VectorMemUnit]
	VectorMemoryUnit::width = ini_file->ReadInt(section, "Width",
					VectorMemoryUnit::width);
	VectorMemoryUnit::issue_buffer_size = ini_file->ReadInt(section,
					"IssueBufferSize",
					VectorMemoryUnit::issue_buffer_size);
	VectorMemoryUnit::decode_latency = ini_file->ReadInt(section,
					"DecodeLatency",
					VectorMemoryUnit::decode_latency);
	// TODO VectorMemoryUnit::decode_width
	VectorMemoryUnit::read_latency = ini_file->ReadInt(section,
					"ReadLatency",
					VectorMemoryUnit::read_latency);
	VectorMemoryUnit::read_buffer_size = ini_file->ReadInt(section,
					"ReadBufferSize",
					VectorMemoryUnit::read_buffer_size);
	VectorMemoryUnit::max_inflight_mem_accesses = ini_file->ReadInt(section,
					"MaxInflightMem",
					VectorMemoryUnit::max_inflight_mem_accesses);
	VectorMemoryUnit::write_latency = ini_file->ReadInt(section,
					"WriteLatency",
					VectorMemoryUnit::write_latency);
	VectorMemoryUnit::write_buffer_size = ini_file->ReadInt(section,
					"WriteBufferSize",
					VectorMemoryUnit::write_buffer_size);

	// TODO Section [LDS]
}


bool Timing::Run()
{

	// Get SI Driver
	Emulator *emulator = Emulator::getInstance();

	// For efficiency when no Southern Islands emulation is selected, 
	// exit here if the list of existing ND-Ranges is empty. 
	if (!emulator->getNumNDRanges())
		return false;

	// Add any available work groups to the waiting list
	for (auto it = emulator->getNDRangesBegin();
			it != emulator->getNDRangesEnd();
			++it)
	{
		// Get pointer to NDRange
		NDRange *ndrange = it->get();

		// Break if there are no waiting work groups
		if (ndrange->isWaitingWorkGroupsEmpty())
			break;

		// Setup WorkGroup pointer
		WorkGroup *work_group = nullptr;
		
		// Map work groups to compute units
		for (unsigned i = 0; i < ndrange->getNumWaitingWorkgroups(); 
				i++)
		{
			// Remove work group from list and get its ID
			long work_group_id = ndrange->GetWaitingWorkGroup();
			work_group = ndrange->ScheduleWorkGroup(work_group_id);

			// Get an available compute unit
			ComputeUnit *compute_unit = 
					gpu->getAvailableComputeUnit();
			assert(compute_unit);
			compute_unit->setAvailable(false);

			// Map the work group to a compute unit
			compute_unit->MapWorkGroup(work_group);
		}
	}


	// Stop if maximum number of GPU cycles exceeded
	esim::Engine *esim_engine = esim::Engine::getInstance();
	if (Gpu::max_cycles && getCycle() >=
			Gpu::max_cycles)
		esim_engine->Finish("SIMaxCycles");

	// Stop if maximum number of GPU instructions exceeded
	if (Emulator::getMaxInstructions() && emulator->getNumInstructions() >=
			Emulator::getMaxInstructions())
		esim_engine->Finish("SIMaxInstructions");

	// Stop if there was a simulation stall
	if (gpu->last_complete_cycle && gpu->last_complete_cycle > 1000000)
	{
		//warning("Southern Islands GPU simulation stalled.\n%s",
		//	si_err_stall);
		esim_engine->Finish("SIStall");
	}

	// Stop if any reason met
	if (esim_engine->hasFinished())
		return true;

	// Run one loop iteration on each busy compute unit
	gpu->Run();

	// Still running
	return true;
}

}
