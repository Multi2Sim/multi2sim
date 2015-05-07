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
#include <arch/common/Timing.h>
#include <lib/esim/Engine.h>

#include "System.h"


namespace mem
{


const std::string System::help_message =
	"Option '--mem-config <file>' is used to configure the memory system. The\n"
	"configuration file is a plain-text file in the IniFile format. The memory\n"
	"system is formed of a set of cache modules, main memory modules, and\n"
	"interconnects.\n"
	"\n"
	"Interconnects can be defined in two different configuration files. The first\n"
	"way is using option '--net-config <file>' (use option '--help-net-config'\n"
	"for more information). Any network defined in the network configuration file\n"
	"can be referenced from the memory configuration file. These networks will be\n"
	"referred hereafter as external networks.\n"
	"\n"
	"The second option to define a network straight in the memory system\n"
	"configuration. This alternative is provided for convenience and brevity. By\n"
	"using sections [Network <name>], networks with a default topology are\n"
	"created which include a single switch, and one bidirectional link from the\n"
	"switch to every end node present in the network.\n"
	"\n"
	"The following sections and variables can be used in the memory system\n"
	"configuration file:\n"
	"\n"
	"Section [General] defines global parameters affecting the entire memory\n"
	"system.\n"
	"\n"
	"  Frequency = <value>  (Default = 1000)\n"
	"      Frequency of the memory system in MHz.\n"
	"  PageSize = <size>  (Default = 4096)\n"
	"      Memory page size. Virtual addresses are translated into new physical\n"
	"      addresses in ascending order at the granularity of the page size.\n"
	"\n"
	"Section [Module <name>] defines a generic memory module. This section is\n"
	"used to declare both caches and main memory modules accessible from CPU\n"
	"cores or GPU compute units.\n"
	"\n"
	"  Type = {Cache|MainMemory}  (Required)\n"
	"      Type of the memory module. From the simulation point of view, the\n"
	"      difference between a cache and a main memory module is that the former\n"
	"      contains only a subset of the data located at the memory locations it\n"
	"      serves.\n"
	"  Geometry = <geo>\n"
	"      Cache geometry, defined in a separate section of type\n"
	"      [Geometry <geo>]. This variable is required for cache modules.\n"
	"  LowNetwork = <net>\n"
	"      Network connecting the module with other lower-level modules, i.e.,\n"
	"      modules closer to main memory. This variable is mandatory for caches,\n"
	"      and should not appear for main memory modules. Value <net> can refer\n"
	"      to an internal network defined in a [Network <net>] section, or to an\n"
	"      external network defined in the network configuration file.\n"
	"  LowNetworkNode = <node>\n"
	"      If 'LowNetwork' points to an external network, node in the network\n"
	"      that the module is mapped to. For internal networks, this variable\n"
	"      should be omitted.\n"
	"  HighNetwork = <net>\n"
	"      Network connecting the module with other higher-level modules, i.e.,\n"
	"      modules closer to CPU cores or GPU compute units. For highest level\n"
	"      modules accessible by CPU/GPU, this variable should be omitted.\n"
	"  HighNetworkNode = <node>\n"
	"      If 'HighNetwork' points to an external network, node that the module\n"
	"      is mapped to.\n"
	"  LowModules = <mod1> [<mod2> ...]\n"
	"      List of lower-level modules. For a cache module, this variable is\n"
	"      required. If there is only one lower-level module, it serves the\n"
	"      entire address space for the current module. If there are several\n"
	"      lower-level modules, each served a disjoint subset of the address\n"
	"      space. This variable should be omitted for main memory modules.\n"
	"  BlockSize = <size>\n"
	"      Block size in bytes. This variable is required for a main memory\n"
	"      module. It should be omitted for a cache module (in this case, the\n"
	"      block size is specified in the corresponding cache geometry section).\n"
	"  Latency = <cycles>\n"
	"      Memory access latency. This variable is required for a main memory\n"
	"      module, and should be omitted for a cache module (the access latency\n"
	"      is specified in the corresponding cache geometry section).\n"
	"  Ports = <num>\n"
	"      Number of read/write ports. This variable is only allowed for a main\n"
	"      memory module. The number of ports for a cache is specified in a\n"
	"      separate cache geometry section.\n"
	"  DirectorySize <size>\n"
	"      Size of the directory in number of blocks. The size of a directory\n"
	"      limits the number of different blocks that can reside in upper-level\n"
	"      caches. If a cache requests a new block from main memory, and its\n"
	"      directory is full, a previous block must be evicted from the\n"
	"      directory, and all its occurrences in the memory hierarchy need to be\n"
	"      first invalidated. This variable is only allowed for a main memory\n"
	"      module.\n"
	"  DirectoryAssoc = <assoc>\n"
	"      Directory associativity in number of ways. This variable is only\n"
	"      allowed for a main memory module.\n"
	"  AddressRange = { BOUNDS <low> <high> | ADDR DIV <div> MOD <mod> EQ <eq> }\n"
	"      Physical address range served by the module. If not specified, the\n"
	"      entire address space is served by the module. There are two possible\n"
	"      formats for the value of 'Range':\n"
	"      With the first format, the user can specify the lowest and highest\n"
	"      byte included in the address range. The value in <low> must be a\n"
	"      multiple of the module block size, and the value in <high> must be a\n"
	"      multiple of the block size minus 1.\n"
	"      With the second format, the address space can be split between\n"
	"      different modules in an interleaved manner. If dividing an address\n"
	"      by <div> and modulo <mod> makes it equal to <eq>, it is served by\n"
	"      this module. The value of <div> must be a multiple of the block size.\n"
	"      When a module serves only a subset of the address space, the user must\n"
	"      make sure that the rest of the modules at the same level serve the\n"
	"      remaining address space.\n"
	"\n"
	"Section [CacheGeometry <geo>] defines a geometry for a cache. Caches using\n"
	"this geometry are instantiated [Module <name>] sections.\n"
	"\n"
	"  Sets = <num_sets> (Required)\n"
	"      Number of sets in the cache.\n"
	"  Assoc = <num_ways> (Required)\n"
	"      Cache associativity. The total number of blocks contained in the cache\n"
	"      is given by the product Sets * Assoc.\n"
	"  BlockSize = <size> (Required)\n"
	"      Size of a cache block in bytes. The total size of the cache is given\n"
	"      by the product Sets * Assoc * BlockSize.\n"
	"  Latency = <cycles> (Required)\n"
	"      Hit latency for a cache in number of cycles.\n"
	"  Policy = {LRU|FIFO|Random} (Default = LRU)\n"
	"      Block replacement policy.\n"
	"  WritePolicy = {WriteBack|WriteThrough} (Default = WriteBack)\n"
	"      Cache write policy.\n"
	"  MSHR = <size> (Default = 16)\n"
	"      Miss status holding register (MSHR) size in number of entries. This\n"
	"      value determines the maximum number of accesses that can be in flight\n"
	"      for the cache, including the time since the access request is\n"
	"      received, until a potential miss is resolved.\n"
	"  Ports = <num> (Default = 2)\n"
	"      Number of ports. The number of ports in a cache limits the number of\n"
	"      concurrent hits. If an access is a miss, it remains in the MSHR while\n"
	"      it is resolved, but releases the cache port.\n"
	"  DirectoryLatency = <cycles> (Default = 1)\n"
	"      Latency for a directory access in number of cycles.\n"
	"  EnablePrefetcher = {t|f} (Default = False)\n"
	"      Whether the hardware should automatically perform prefetching.\n"
	"      The prefetcher related options below will be ignored if this is\n"
	"      not true.\n"
	"  PrefetcherType = {GHB_PC_CS|GHB_PC_DC} (Default GHB_PC_CS)\n"
	"      Specify the type of global history buffer based prefetcher to use.\n"
	"      GHB_PC_CS - Program Counter indexed, Constant Stride.\n"
	"      GHB_PC_DC - Program Counter indexed, Delta Correlation.\n"
	"  PrefetcherGHBSize = <size> (Default = 256)\n"
	"      The hardware prefetcher does global history buffer based prefetching.\n"
	"      This option specifies the size of the global history buffer.\n"
	"  PrefetcherITSize = <size> (Default = 64)\n"
	"      The hardware prefetcher does global history buffer based prefetching.\n"
	"      This option specifies the size of the index table used.\n"
	"  PrefetcherLookupDepth = <num> (Default = 2)\n"
	"      This option specifies the history (pattern) depth upto which the\n"
	"      prefetcher looks at the history to decide when to prefetch.\n"
	"\n"
	"Section [Network <net>] defines an internal default interconnect, formed of\n"
	"a single switch connecting all modules pointing to the network. For every\n"
	"module in the network, a bidirectional link is created automatically between\n"
	"the module and the switch, together with the suitable input/output buffers\n"
	"in the switch and the module.\n"
	"\n"
	"  DefaultInputBufferSize = <size>\n"
	"      Size of input buffers for end nodes (memory modules) and switch.\n"
	"  DefaultOutputBufferSize = <size>\n"
	"      Size of output buffers for end nodes and switch. \n"
	"  DefaultBandwidth = <bandwidth>\n"
	"      Bandwidth for links and switch crossbar in number of bytes per cycle.\n"
	"\n"
	"Section [Entry <name>] creates an entry into the memory system. An entry is\n"
	"a connection between a CPU core/thread or a GPU compute unit with a module\n"
	"in the memory system.\n"
	"\n"
	"  Arch = { x86 | Evergreen | SouthernIslands | ... }\n"
	"      CPU or GPU architecture affected by this entry.\n"
	"  Core = <core>\n"
	"      CPU core identifier. This is a value between 0 and the number of cores\n"
	"      minus 1, as defined in the CPU configuration file. This variable\n"
	"      should be omitted for GPU entries.\n"
	"  Thread = <thread>\n"
	"      CPU thread identifier. Value between 0 and the number of threads per\n"
	"      core minus 1. Omitted for GPU entries.\n"
	"  ComputeUnit = <id>\n"
	"      GPU compute unit identifier. Value between 0 and the number of compute\n"
	"      units minus 1, as defined in the GPU configuration file. This variable\n"
	"      should be omitted for CPU entries.\n"
	"  DataModule = <mod>\n"
	"  ConstantDataModule = <mod>\n"
	"  InstModule = <mod>\n"
	"      In architectures supporting separate data/instruction caches, modules\n"
	"      used to access memory for each particular purpose.\n"
	"  Module = <mod>\n"
	"      Module used to access the memory hierarchy. For architectures\n"
	"      supporting separate data/instruction caches, this variable can be used\n"
	"      instead of 'DataModule', 'InstModule', and 'ConstantDataModule' to\n"
	"      indicate that data and instruction caches are unified.\n"
	"\n";

const char *System::err_config_note =
	"\tPlease run 'm2s --mem-help' or consult the Multi2Sim Guide for "
	"a description of the memory system configuration file format.\n";

const char *System::err_config_net =
	"\tNetwork identifiers need to be declared either in the cache " 
	"configuration file, or in the network configuration file (option " 
	"'--net-config').\n";

const char *System::err_levels =
	"\tThe path from a cache into main memory exceeds 10 levels of cache. "
	"This might be a symptom of a recursive reference in 'LowModules' "
	"lists. If you really intend to have a high number of cache levels, "
	"increase variable MEM_SYSTEM_MAX_LEVELS in '" __FILE__ "'\n";

const char *System::err_block_size =
	"\tBlock size in a cache must be greater or equal than its\n" 
	"\tlower-level cache for correct behavior of directories and\n" 
	"\tcoherence protocols.\n";

const char *System::err_connect =
	"\tAn external network is used that does not provide connectivity\n"
	"\tbetween a memory module and an associated low/high module. Please\n"
	"\tadd the necessary links in the network configuration file.\n";

const char *System::err_mem_disjoint =
	"\tIn current versions of Multi2Sim, it is not allowed having a\n" 
	"\tmemory module shared for different architectures. Please make sure\n"
	"\tthat the sets of modules accessible by different architectures\n"
	"\tare disjoint.\n";

void System::ConfigReadGeneral(misc::IniFile *ini_file)
{
	// Section with general parameters
	std::string section = "General";

	// Frequency
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw misc::Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz.\n%s",
				ini_file->getPath().c_str(),
				err_config_note));
	
	// Create frequency domain
	esim::Engine *engine = esim::Engine::getInstance();
	frequency_domain = engine->RegisterFrequencyDomain("Memory", frequency);

	// Peer transfers
	// FIXME Disabling this option for now.  Potentially removing it
	// entirely.  See description in mod-stack.c
	//
	//mem_peer_transfers = config_read_bool(config, section, 
	//	"PeerTransfers", 1);
}


void System::ConfigReadNetworks(misc::IniFile *ini_file)
{
}


void System::ConfigReadCache(misc::IniFile *ini_file,
		const std::string &section)
{
}


void System::ConfigReadMainMemory(misc::IniFile *ini_file,
		const std::string &section)
{
}


void System::ConfigReadModuleAddressRange(misc::IniFile *ini_file,
		Module *module,
		const std::string &section)
{
}


void System::ConfigReadModules(misc::IniFile *ini_file)
{
}


void System::ConfigCheckRouteToMainMemory(Module *module,
		int block_size,
		int level)
{
}


void System::ConfigReadLowModules(misc::IniFile *ini_file)
{
}


void System::ConfigReadEntries(misc::IniFile *ini_file)
{
}


void System::ConfigCreateSwitches(misc::IniFile *ini_file)
{
}


void System::ConfigCheckRoutes()
{
}


void System::ConfigCalculateSubBlockSizes()
{
}


void System::ConfigSetModuleLevel(Module *module, int level)
{
}


void System::ConfigCalculateModuleLevels()
{
}


void System::ConfigTrace()
{
}


void System::ConfigReadCommands(misc::IniFile *ini_file)
{
}


void System::ConfigRead()
{
	// Load memory system configuration file. If no file name has been given
	// by the user, create a default configuration for each architecture.
	misc::IniFile ini_file;
	if (config_file.empty())
	{
		// Create default configuration files for each architecture
		comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
		for (auto it = arch_pool->timing_begin(),
				e = arch_pool->timing_end();
				it != e;
				++it)
		{
			comm::Timing *timing = (*it)->getTiming();
			assert(timing);
			timing->WriteMemoryConfiguration(&ini_file);
		}
	}
	else
	{
		// Load from file
		ini_file.Load(config_file);
	}

	// Read general variables
	ConfigReadGeneral(&ini_file);

	// Read networks
	ConfigReadNetworks(&ini_file);

	// Read modules
	ConfigReadModules(&ini_file);

	// Read low level caches
	ConfigReadLowModules(&ini_file);

	// Read entries from requesting devices (CPUs/GPUs) to memory system
	// entries. This is presented in [Entry <name>] sections in the
	// configuration file.
	ConfigReadEntries(&ini_file);

	// Create switches in internal networks
	ConfigCreateSwitches(&ini_file);

	// Read commands from the configuration file. Commands are used to
	// artificially alter the initial state of the memory hierarchy for
	// debugging purposes.
	ConfigReadCommands(&ini_file);

	// Check that all enforced sections and variables were specified.
	ini_file.Check();

	// Check routes to low and high modules
	ConfigCheckRoutes();

	// Check for disjoint memory hierarchies for different architectures.
	// FIXME We don't know if device is fused until runtime, so we can't
	// check this in advance.
	// arch_for_each(mem_config_check_disjoint, NULL);

	// Compute sub-block sizes, based on high modules. This function also
	// initializes the directories in modules other than L1.
	ConfigCalculateSubBlockSizes();

	// Compute cache levels relative to the CPU/GPU entry points
	ConfigCalculateModuleLevels();

	// Dump configuration to trace file
	ConfigTrace();
}


}

