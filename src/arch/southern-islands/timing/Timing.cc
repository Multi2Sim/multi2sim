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
	return false;
}


void Timing::WriteMemoryConfiguration(misc::IniFile *ini_file)
{
}


void Timing::ParseMemoryConfigurationEntry(misc::IniFile *ini_file,
		const std::string &section)
{
}


void Timing::CheckMemoryConfiguration(misc::IniFile *ini_file)
{
}


void Timing::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Southern Islands");

/*
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
*/
}


void Timing::ProcessOptions()
{
/*
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
*/
}


void Timing::ParseConfiguration(misc::IniFile *ini_file)
{
}

}

