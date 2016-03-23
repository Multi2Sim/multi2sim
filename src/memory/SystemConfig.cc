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
#include <network/EndNode.h>
#include <network/Node.h>
#include <network/Switch.h>

#include "Module.h"
#include "System.h"


namespace mem
{


const int System::trace_version_major = 1;
const int System::trace_version_minor = 678;

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
	"  DirectoryLatency = <cycles>\n"
	"      Access latency for directory. This variable is only allowed for a\n"
	"      main memory module.\n"
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
	"Please run 'm2s --mem-help' or consult the Multi2Sim Guide for "
	"a description of the memory system configuration file format.";

const char *System::err_config_net =
	"Network identifiers need to be declared either in the cache " 
	"configuration file, or in the network configuration file (option " 
	"'--net-config').";

const char *System::err_levels =
	"The path from a cache into main memory exceeds 10 levels of cache. "
	"This might be a symptom of a recursive reference in 'LowModules' "
	"lists.";

const char *System::err_block_size =
	"Block size in a cache must be greater or equal than its "
	"lower-level cache for correct behavior of directories and "
	"coherence protocols.";

const char *System::err_connect =
	"An external network is used that does not provide connectivity "
	"between a memory module and an associated low/high module. Please "
	"add the necessary links in the network configuration file.";

const char *System::err_mem_disjoint =
	"In current versions of Multi2Sim, it is not allowed having a "
	"memory module shared for different architectures. Please make sure "
	"that the sets of modules accessible by different architectures "
	"are disjoint.";

void System::ConfigReadGeneral(misc::IniFile *ini_file)
{
	// Section with general parameters
	std::string section = "General";

	// Frequency
	frequency = ini_file->ReadInt(section, "Frequency", frequency);
	if (!esim::Engine::isValidFrequency(frequency))
		throw Error(misc::fmt("%s: The value for 'Frequency' "
				"must be between 1MHz and 1000GHz.\n%s",
				ini_file->getPath().c_str(),
				err_config_note));
	debug << "Memory system frequency set to " << frequency << "MHz\n";
	
	// Update frequency of existing frequency domain. The reason why we
	// don't create the frequency domain here is because it needs to be
	// available when registering the events in the constructor of class
	// System, but back then we haven't parsed the memory configuration file
	// yet.
	assert(frequency_domain);
	frequency_domain->setFrequency(frequency);

	// Peer transfers
	// FIXME Disabling this option for now.  Potentially removing it
	// entirely.  See description in mod-stack.c
	//
	//mem_peer_transfers = config_read_bool(config, section, 
	//	"PeerTransfers", 1);
}


void System::ConfigReadNetworks(misc::IniFile *ini_file)
{
	// Create networks
	debug << "Creating internal networks:\n";
	for (auto it = ini_file->sections_begin(),
			e = ini_file->sections_end();
			it != e;
			++it)
	{
		// Check if this is the name of a network
		std::string name = *it;
		if (strncasecmp(name.c_str(), "Network ", 8))
			continue;
		name.erase(0, 8);

		// Check that network name is not duplicated

		// Create network
		addNetwork(name);
		debug << "\tNetwork '" << name << "' created\n";
	}
	debug << '\n';

	// Add network pointers to configuration file. This needs to be done 
	// separately, because configuration file writes alter enumeration of 
	// sections. Also check integrity of sections.
	for (auto it = networks.begin(), e = networks.end(); it != e; ++it)
	{
		// Get network section name
		net::Network *network = it->get();
		std::string section = "Network " + network->getName();
		assert(ini_file->Exists(section));

		// Add pointer
		ini_file->WritePointer(section, "ptr", network);

		// Check section integrity
		ini_file->Enforce(section, "DefaultInputBufferSize");
		ini_file->Enforce(section, "DefaultOutputBufferSize");
		ini_file->Enforce(section, "DefaultBandwidth");
		ini_file->Check(section);
	}
}


void System::ConfigInsertModuleInInternalNetwork(
		misc::IniFile *ini_file,
		Module *module,
		const std::string &network_name,
		const std::string &network_node_name,
		net::Network *&network,
		net::EndNode *&network_node)
{
	// No network specified
	network = nullptr;
	network_node = nullptr;
	if (network_name.empty())
		return;

	// Try to insert in private network
	std::string section = "Network " + network_name;
	network = (net::Network *) ini_file->ReadPointer(section, "ptr");
	if (!network)
	{
		ConfigInsertModuleInExternalNetwork(ini_file,
				module,
				network_name,
				network_node_name,
				network,
				network_node);
		return;
	}

	// For private networks, 'network_node_name' should be empty
	if (!network_node_name.empty())
		throw Error(misc::fmt("%s: %s: network node name should be "
				"empty.\n%s",
				ini_file->getPath().c_str(),
				module->getName().c_str(),
				err_config_note));

	// Network should not have this module already
	if (network->getNodeByUserData(module))
		throw Error(misc::fmt("%s: network '%s' already contains "
				"module '%s'.\n%s",
				ini_file->getPath().c_str(),
				network->getName().c_str(),
				module->getName().c_str(),
				err_config_note));

	// Read buffer sizes from network
	int default_input_buffer_size = ini_file->ReadInt(section,
			"DefaultInputBufferSize");
	int default_output_buffer_size = ini_file->ReadInt(section,
			"DefaultOutputBufferSize");
	if (!default_input_buffer_size)
		throw Error(misc::fmt("%s: network %s: variable "
				"'DefaultInputBufferSize' missing.\n%s",
				ini_file->getPath().c_str(),
				network->getName().c_str(),
				err_config_note));
	if (!default_output_buffer_size)
		throw Error(misc::fmt("%s: network %s: variable "
				"'DefaultOutputBufferSize' missing.\n%s",
				ini_file->getPath().c_str(),
				network->getName().c_str(),
				err_config_note));
	if (default_input_buffer_size < module->getBlockSize() + 8)
		throw Error(misc::fmt("%s: network %s: minimum input buffer "
				"size is %d for cache '%s'.\n%s",
				ini_file->getPath().c_str(),
				network->getName().c_str(),
				module->getBlockSize() + 8,
				module->getName().c_str(),
				err_config_note));
	if (default_output_buffer_size < module->getBlockSize() + 8)
		throw Error(misc::fmt("%s: network %s: minimum output buffer "
				"size is %d for cache '%s'.\n%s",
				ini_file->getPath().c_str(),
				network->getName().c_str(),
				module->getBlockSize() + 8,
				module->getName().c_str(),
				err_config_note));

	// Create node
	network_node = network->addEndNode(
			default_input_buffer_size,
			default_output_buffer_size,
			module->getName(),
			module);
}


void System::ConfigInsertModuleInExternalNetwork(
		misc::IniFile *ini_file,
		Module *module,
		const std::string &network_name,
		const std::string &network_node_name,
		net::Network *&network,
		net::EndNode *&network_node)
{
	// Get network
	net::System *network_system = net::System::getInstance();
	network = network_system->getNetworkByName(network_name);
	if (!network)
		throw Error(misc::fmt("%s: %s: invalid network name.\n%s%s",
				ini_file->getPath().c_str(),
				network_name.c_str(),
				err_config_note,
				err_config_net));

	// Node name must be specified
	if (network_node_name.empty())
		throw Error(misc::fmt("%s: %s: network node name required for "
				"external network.\n%s%s",
				ini_file->getPath().c_str(),
				module->getName().c_str(),
				err_config_note,
				err_config_net));

	// Get node
	network_node = dynamic_cast<net::EndNode *>(network->getNodeByName(
			network_node_name));
	if (!network_node)
		throw Error(misc::fmt("%s: network %s: node %s: invalid node "
				"name.\n%s%s",
				ini_file->getPath().c_str(),
				network_name.c_str(),
				network_node_name.c_str(),
				err_config_note,
				err_config_net));

	// No module must have been assigned previously to this node
	if (network_node->getUserData())
		throw Error(misc::fmt("%s: network %s: node '%s' already "
				"assigned.\n%s",
				ini_file->getPath().c_str(),
				network_name.c_str(),
				network_node_name.c_str(),
				err_config_note));

	// Network should not have this module already
	if (network->getNodeByUserData(module))
		throw Error(misc::fmt("%s: network %s: module '%s' is already "
				"present.\n%s",
				ini_file->getPath().c_str(),
				network_name.c_str(),
				module->getName().c_str(),
				err_config_note));

	// Assign module to network node and return
	network_node->setUserData(module);
}


Module *System::ConfigReadCache(misc::IniFile *ini_file,
		const std::string &section)
{
	// Cache parameters
	std::string geometry_section = "CacheGeometry " +
			ini_file->ReadString(section, "Geometry");
	ini_file->Enforce(section, "Geometry");
	ini_file->Enforce(geometry_section);
	ini_file->Enforce(geometry_section, "Latency");
	ini_file->Enforce(geometry_section, "Sets");
	ini_file->Enforce(geometry_section, "Assoc");
	ini_file->Enforce(geometry_section, "BlockSize");

	// Module name
	std::string module_name = section;
	assert(!strncasecmp(section.c_str(), "Module ", 7));
	module_name.erase(0, 7);
	misc::StringTrim(module_name);
	
	// Geometry values
	int num_sets = ini_file->ReadInt(geometry_section, "Sets", 16);
	int num_ways = ini_file->ReadInt(geometry_section, "Assoc", 2);
	int block_size = ini_file->ReadInt(geometry_section, "BlockSize", 256);
	int latency = ini_file->ReadInt(geometry_section, "Latency", 1);
	int directory_latency = ini_file->ReadInt(geometry_section, "DirectoryLatency", 0);
	std::string replacement_policy_str = ini_file->ReadString(geometry_section,
			"Policy", "LRU");
	std::string write_policy_str = ini_file->ReadString(geometry_section,
			"WritePolicy", "WriteBack");
	int mshr_size = ini_file->ReadInt(geometry_section, "MSHR", 16);
	int num_ports = ini_file->ReadInt(geometry_section, "Ports", 2);

	// Check replacement policy
	Cache::ReplacementPolicy replacement_policy =
			(Cache::ReplacementPolicy)
			Cache::ReplacementPolicyMap.MapString
			(replacement_policy_str);
	if (!replacement_policy)
		throw Error(misc::fmt("%s: Cache %s: %s: "
				"Invalid block replacement policy.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				replacement_policy_str.c_str(),
				err_config_note));

	// Check write policy
	Cache::WritePolicy write_policy =
			(Cache::WritePolicy)
			Cache::WritePolicyMap.MapString(write_policy_str);
	if (!write_policy)
		throw Error(misc::fmt("%s: Cache %s: %s: "
				"Invalid write policy.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				write_policy_str.c_str(),
				err_config_note));
	if (write_policy == Cache::WriteThrough)
		misc::Warning("%s: Cache %s: %s: Write policy "
				"not yet implemented, "
				"WriteBack policy being used.\n",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				write_policy_str.c_str());
	
	// Other checks
	if (num_sets < 1 || (num_sets & (num_sets - 1)))
		throw Error(misc::fmt("%s: cache %s: number of sets must be a "
				"power of two greater than 1.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (num_ways < 1 || (num_ways & (num_ways - 1)))
		throw Error(misc::fmt("%s: cache %s: associativity must be a "
				"power of two and > 1.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (block_size < 4 || (block_size & (block_size - 1)))
		throw Error(misc::fmt("%s: cache %s: block size must be power "
				"of two and at least 4.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (directory_latency < 0)
		throw Error(misc::fmt("%s: cache %s: invalid value for "
				"variable 'DirectoryLatency'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (latency < 0)
		throw Error(misc::fmt("%s: cache %s: invalid value for "
				"variable 'Latency'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (mshr_size < 1)
		throw Error(misc::fmt("%s: cache %s: invalid value for "
				"variable 'MSHR'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (num_ports < 1)
		throw Error(misc::fmt("%s: cache %s: invalid value for "
				"variable 'Ports'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));

	// Create module
	Module *module = addModule(module_name,
			Module::TypeCache,
			num_ports,
			block_size,
			latency);
	
	// Initialize module
	module->setDirectoryProperties(num_sets, num_ways, directory_latency);
	module->setMSHRSize(mshr_size);

	// High network
	std::string network_name = ini_file->ReadString(section, "HighNetwork");
	std::string network_node_name = ini_file->ReadString(section, "HighNetworkNode");
	net::Network *network;
	net::EndNode *network_node;
	ConfigInsertModuleInInternalNetwork(
			ini_file,
			module,
			network_name,
			network_node_name,
			network,
			network_node);
	module->setHighNetwork(network, network_node);
	
	// Low network
	network_name = ini_file->ReadString(section, "LowNetwork");
	network_node_name = ini_file->ReadString(section, "LowNetworkNode");
	ConfigInsertModuleInInternalNetwork(
			ini_file,
			module,
			network_name,
			network_node_name,
			network,
			network_node);
	module->setLowNetwork(network, network_node);

	// Create cache
	module->setCache(num_sets,
			num_ways,
			block_size,
			replacement_policy,
			write_policy);

	// Done
	return module;
}


Module *System::ConfigReadMainMemory(misc::IniFile *ini_file,
		const std::string &section)
{
	// Module name
	std::string module_name = section;
	assert(!strncasecmp(section.c_str(), "Module ", 7));
	module_name.erase(0, 7);
	misc::StringTrim(module_name);
	
	// Read parameters
	ini_file->Enforce(section, "Latency");
	ini_file->Enforce(section, "BlockSize");
	int block_size = ini_file->ReadInt(section, "BlockSize", 64);
	int latency = ini_file->ReadInt(section, "Latency", 1);
	int num_ports = ini_file->ReadInt(section, "Ports", 2);
	int directory_size = ini_file->ReadInt(section, "DirectorySize", 131072);
	int directory_num_ways = ini_file->ReadInt(section, "DirectoryAssoc", 16);
	int directory_latency = ini_file->ReadInt(section, "DirectoryLatency", 1);

	// Check parameters
	if (block_size < 1 || (block_size & (block_size - 1)))
		throw Error(misc::fmt("%s: %s: block size must be power of "
				"two.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (latency < 0)
		throw Error(misc::fmt("%s: %s: invalid value for variable "
				"'Latency'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (num_ports < 1)
		throw Error(misc::fmt("%s: %s: invalid value for variable "
				"'NumPorts'.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (directory_size < 1 || (directory_size & (directory_size - 1)))
		throw Error(misc::fmt("%s: %s: directory size must be a power "
				"of two.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (directory_num_ways < 1 || (directory_num_ways & (directory_num_ways - 1)))
		throw Error(misc::fmt("%s: %s: directory associativity must be "
				"a power of two.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));
	if (directory_num_ways > directory_size)
		throw Error(misc::fmt("%s: %s: invalid directory "
				"associativity.\n%s",
				ini_file->getPath().c_str(),
				module_name.c_str(),
				err_config_note));

	// Create module
	Module *module = addModule(module_name,
			Module::TypeMainMemory,
			num_ports,
			block_size,
			latency);

	// Initialize module
	int directory_num_sets = directory_size / directory_num_ways;
	module->setDirectoryProperties(directory_num_sets,
			directory_num_ways,
			directory_latency);

	// High network
	std::string network_name = ini_file->ReadString(section, "HighNetwork");
	std::string network_node_name = ini_file->ReadString(section, "HighNetworkNode");
	net::Network *network;
	net::EndNode *network_node;
	ConfigInsertModuleInInternalNetwork(
			ini_file,
			module,
			network_name,
			network_node_name,
			network,
			network_node);
	module->setHighNetwork(network, network_node);
	
	// Create cache
	module->setCache(directory_num_sets,
			directory_num_ways,
			block_size,
			Cache::ReplacementLRU,
			Cache::WriteBack);

	// Done
	return module;
}


void System::ConfigInvalidAddressRange(misc::IniFile *ini_file, Module *module)
{
	throw Error(misc::fmt("%s: %s: invalid format for 'AddressRange'.\n%s",
			ini_file->getPath().c_str(),
			module->getName().c_str(),
			err_config_note));
}


void System::ConfigReadModuleAddressRange(misc::IniFile *ini_file,
		Module *module,
		const std::string &section)
{
	// Read address range
	std::string range = ini_file->ReadString(section, "AddressRange");
	if (range.empty())
	{
		module->setRangeBounds(0, -1);
		return;
	}

	// Split in tokens
	std::vector<std::string> tokens;
	misc::StringTokenize(range, tokens);
	if (tokens.empty())
		ConfigInvalidAddressRange(ini_file, module);

	// First token - ADDR or BOUNDS
	if (!misc::StringCaseCompare(tokens[0], "BOUNDS"))
	{
		// Format is: BOUNDS <low> <high>
		if (tokens.size() != 3)
			ConfigInvalidAddressRange(ini_file, module);

		// Lower bound
		misc::StringError error;
		unsigned low = misc::StringToInt(tokens[1], error);
		if (error)
			throw Error(misc::fmt("%s: %s: invalid value '%s' in "
					"'AddressRange'",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					tokens[1].c_str()));
		if (low % module->getBlockSize())
			throw Error(misc::fmt("%s: %s: low address bound must "
					"be a multiple of block size.\n%s",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					err_config_note));

		// High bound
		unsigned high = misc::StringToInt(tokens[2], error);
		if (error)
			throw Error(misc::fmt("%s: %s: invalid value '%s' in "
					"'AddressRange'",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					tokens[2].c_str()));
		if ((high + 1) % module->getBlockSize())
			throw Error(misc::fmt("%s: %s: high address bound must "
					"be a multiple of block size minus 1.\n%s", 
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					err_config_note));

		// Set range
		module->setRangeBounds(low, high);
	}
	else if (!misc::StringCaseCompare(tokens[0], "ADDR"))
	{
		// Format is: ADDR DIV <div> MOD <mod> EQ <eq> */
		if (tokens.size() != 7)
			ConfigInvalidAddressRange(ini_file, module);

		// Token 'DIV'
		if (misc::StringCaseCompare(tokens[1], "DIV"))
			ConfigInvalidAddressRange(ini_file, module);

		// Field <div>
		misc::StringError error;
		int div = misc::StringToInt(tokens[2], error);
		if (error)
			throw Error(misc::fmt("%s: %s: invalid value '%s' in "
					"'AddressRange'",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					tokens[2].c_str()));
		if (div < 1)
			ConfigInvalidAddressRange(ini_file, module);
		if (div % module->getBlockSize())
			throw Error(misc::fmt("%s: %s: value for <div> must be "
					"a multiple of block size.\n%s",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					err_config_note));

		// Token 'MOD'
		if (misc::StringCaseCompare(tokens[3], "MOD"))
			ConfigInvalidAddressRange(ini_file, module);

		// Field <mod>
		int mod = misc::StringToInt(tokens[4], error);
		if (error)
			throw Error(misc::fmt("%s: %s: invalid value '%s' in "
					"'AddressRange'",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					tokens[4].c_str()));
		if (mod < 1)
			ConfigInvalidAddressRange(ini_file, module);

		// Token 'EQ'
		if (misc::StringCaseCompare(tokens[5], "EQ"))
			ConfigInvalidAddressRange(ini_file, module);

		// Field <eq>
		int eq = misc::StringToInt(tokens[6], error);
		if (error)
			throw Error(misc::fmt("%s: %s: invalid value '%s' in "
					"'AddressRange'",
					ini_file->getPath().c_str(),
					module->getName().c_str(),
					tokens[6].c_str()));
		if (eq >= mod)
			ConfigInvalidAddressRange(ini_file, module);

		// Set interleaved range
		module->setRangeInterleaved(mod, div, eq);
	}
	else
	{
		ConfigInvalidAddressRange(ini_file, module);
	}
}


void System::ConfigReadModules(misc::IniFile *ini_file)
{
	// Create modules
	debug << "Creating modules:\n";
	for (auto it = ini_file->sections_begin(),
			e = ini_file->sections_end();
			it != e;
			++it)
	{
		// Section for a module
		std::string section = *it;
		if (strncasecmp(section.c_str(), "Module ", 7))
			continue;

		// Module name
		std::string module_name = section;
		module_name.erase(0, 7);
		misc::StringTrim(module_name);

		// Create module based on type
		std::string module_type = ini_file->ReadString(section, "Type");
		Module *module;
		if (!misc::StringCaseCompare(module_type, "Cache"))
			module = ConfigReadCache(ini_file, section);
		else if (!misc::StringCaseCompare(module_type, "MainMemory"))
			module = ConfigReadMainMemory(ini_file, section);
		else
			throw Error(misc::fmt("%s: %s: invalid or missing "
					"value for 'Type'.\n%s",
					ini_file->getPath().c_str(),
					module_name.c_str(),
					err_config_note));

		// Read module address range
		ConfigReadModuleAddressRange(ini_file, module, section);

		// Debug
		debug << "\t" << module_name << '\n';
	}

	// Debug
	debug << '\n';

	// Add module pointers to configuration file. This needs to be done 
	// separately, because writes in the INI file will invalidate the
	// iterators in the sections. Also, check integrity of sections.
	for (auto &module : modules)
	{
		// Section name
		std::string section = "Module " + module->getName();

		// Verify that section is present
		if (!ini_file->Exists(section))
			throw misc::Panic(ini_file->getPath() + ": section '" +
					section + "' does not exist");

		// Add module pointer
		ini_file->WritePointer(section, "ptr", module.get());
	}
}


void System::ConfigCheckRouteToMainMemory(
		misc::IniFile *ini_file,
		Module *module,
		int block_size,
		int level)
{
	// Maximum levels
	if (level > 10)
		throw Error(misc::fmt("%s: %s: too many cache levels.\n%s%s",
				ini_file->getPath().c_str(),
				module->getName().c_str(),
				err_levels,
				err_config_note));

	// Check block size
	if (module->getBlockSize() < block_size)
		throw Error(misc::fmt("%s: %s: decreasing block size.\n%s%s",
				ini_file->getPath().c_str(),
				module->getName().c_str(),
				err_block_size,
				err_config_note));
	
	// Change current block size
	block_size = module->getBlockSize();

	// Dump current module
	debug << '\t'
			<< std::string(level * 2, ' ')
			<< module->getName()
			<< '\n';

	// Check that cache has a way to main memory
	if (!module->getNumLowModules() && module->getType() == Module::TypeCache)
		throw Error(misc::fmt("%s: %s: main memory not accessible from "
				"cache.\n%s",
				ini_file->getPath().c_str(),
				module->getName().c_str(),
				err_config_note));

	// Dump children
	for (int i = 0; i < module->getNumLowModules(); i++)
	{
		Module *low_module = module->getLowModule(i);
		ConfigCheckRouteToMainMemory(ini_file,
				low_module,
				block_size,
				level + 1);
	}
}


void System::ConfigReadLowModules(misc::IniFile *ini_file)
{
	// Traverse modules
	for (auto &module : modules)
	{
		// Ignore if not a cache
		if (module->getType() != Module::TypeCache)
			continue;

		// Section name
		std::string section = "Module " + module->getName();
		assert(ini_file->Exists(section));

		// Low module name list
		std::string low_module_names = ini_file->ReadString(section,
				"LowModules");
		if (low_module_names.empty())
			throw Error(misc::fmt("%s: %s: missing or invalid "
					"value for 'LowModules'.\n%s",
					ini_file->getPath().c_str(),
					section.c_str(),
					err_config_note));

		// For each element in the list
		std::vector<std::string> tokens;
		misc::StringTokenize(low_module_names, tokens, ", ");
		for (std::string &low_module_name : tokens)
		{
			// Check valid module name
			std::string section = "Module " + low_module_name;
			if (!ini_file->Exists(section))
				throw Error(misc::fmt("%s: %s: invalid module "
						"name in 'LowModules'.\n%s",
						ini_file->getPath().c_str(),
						module->getName().c_str(),
						err_config_note));

			// Get low cache and assign
			Module *low_module = (Module *) ini_file->ReadPointer(section, "ptr");
			assert(low_module);
			module->addLowModule(low_module);
			low_module->addHighModule(module.get());
		}
	}

	// Check paths to main memory
	debug << "Checking paths between caches and main memories:\n";
	for (auto &module : modules)
		ConfigCheckRouteToMainMemory(ini_file,
				module.get(),
				module->getBlockSize(),
				1);
	debug << '\n';
}


void System::ConfigReadEntries(misc::IniFile *ini_file)
{
	// Get architecture pool
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();

	// Read all [Entry <name>] sections
	debug << "Processing entries to the memory system:\n\n";
	for (auto it = ini_file->sections_begin(),
			e = ini_file->sections_end();
			it != e;
			++it)
	{
		// Discard if not an entry section
		std::string section = *it;
		if (strncasecmp(section.c_str(), "Entry ", 6))
			continue;

		// Name for the entry
		std::string entry_name = section;
		entry_name.erase(0, 6);
		misc::StringTrim(entry_name);
		if (entry_name.empty())
			throw Error(misc::fmt("%s: section [%s]: invalid entry "
					"name.\n%s",
					ini_file->getPath().c_str(),
					section.c_str(),
					err_config_note));

		// Check if variable 'Type' is used in the section. This
		// variable was used in previous versions, now it is replaced
		// with 'Arch'.
		if (ini_file->Exists(section, "Type"))
			throw Error(misc::fmt("%s: section [%s]: Variable "
					"'Type' is obsolete, use 'Arch' "
					"instead.\n%s",
					ini_file->getPath().c_str(),
					section.c_str(),
					err_config_note));

		// Read architecture in variable 'Arch'
		std::string arch_name = ini_file->ReadString(section, "Arch");
		misc::StringTrim(arch_name);
		if (arch_name.empty())
			throw Error(misc::fmt("%s: section [%s]: Variable "
					"'Arch' is missing.\n%s",
					ini_file->getPath().c_str(),
					section.c_str(),
					err_config_note));

		// Get architecture
		comm::Arch *arch = arch_pool->getByName(arch_name);
		if (!arch)
		{
			std::string names = arch_pool->getArchNames();
			throw Error(misc::fmt("%s: section [%s]: '%s' is an "
					"invalid value for 'Arch'.\n"
					"\tPossible values are %s.\n%s",
					ini_file->getPath().c_str(),
					section.c_str(),
					arch_name.c_str(),
					names.c_str(),
					err_config_note));
		}

		// An architecture with an entry in the memory configuration
		// file must undergo a detailed simulation.
		if (arch->getSimKind() == comm::Arch::SimFunctional)
			throw Error(misc::fmt("%s: section [%s]: %s "
					"architecture not under detailed simulation. "
					"A CPU/GPU architecture uses functional "
					"simulation by default. Please activate detailed "
					"simulation for the architecture.\n",
					ini_file->getPath().c_str(),
					section.c_str(),
					arch->getName().c_str()));

		// Call function to process entry. Each architecture implements
		// its own ways to process entries to the memory hierarchy.
		comm::Timing *timing = arch->getTiming();
		timing->ParseMemoryConfigurationEntry(ini_file, section);
	}

	// After processing all [Entry <name>] sections, check that all
	// architectures satisfy their entries to the memory hierarchy. Do it
	// for those architectures with active timing simulation.
	for (auto it = arch_pool->getTimingBegin(),
			e = arch_pool->getTimingEnd();
			it != e;
			++it)
	{
		comm::Arch *arch = *it;
		arch->getTiming()->CheckMemoryConfiguration(ini_file);
	}
}


void System::ConfigCreateSwitches(misc::IniFile *ini_file)
{
	// For each network, add a switch and create node connections
	debug << "Creating network switches and links for internal networks:\n";
	for (auto &network : networks)
	{
		// Get switch bandwidth
		std::string section = "Network " + network->getName();
		assert(ini_file->Exists(section));
		int default_bandwidth = ini_file->ReadInt(section, "DefaultBandwidth");
		if (default_bandwidth < 1)
			throw Error(misc::fmt("%s: %s: invalid or missing "
					"value for 'DefaultBandwidth'.\n%s",
					ini_file->getPath().c_str(),
					network->getName().c_str(),
					err_config_note));

		// Get input/output buffer sizes. Checks for these variables
		// have been done before.
		int default_input_buffer_size = ini_file->ReadInt(section, "DefaultInputBufferSize");
		int default_output_buffer_size = ini_file->ReadInt(section, "DefaultOutputBufferSize");
		assert(default_input_buffer_size > 0);
		assert(default_output_buffer_size > 0);

		// Create switch
		net::Node *network_switch = network->addSwitch(
				default_input_buffer_size,
				default_output_buffer_size,
				default_bandwidth,
				"Switch");
		debug << '\t' << network->getName() << ".Switch ->";

		// Create connections between switch and every end node */
		for (int i = 0; i < network->getNumNodes(); i++)
		{
			net::Node *network_node = network->getNode(i);
			if (network_node != network_switch)
			{
				std::string link_name =
						network_switch->getName() +
						"<->" +
						network_node->getName();
				network->addBidirectionalLink(
						link_name,
						network_node,
						network_switch,
						default_bandwidth,
						default_output_buffer_size,
						default_input_buffer_size,
						1);
				debug << ' ' << network_node->getName();
			}
		}

		// Calculate routes
		net::RoutingTable *routing_table = network->getRoutingTable();
		routing_table->Initialize();
		routing_table->FloydWarshall();

		// Debug
		debug << '\n';
	}
	debug << '\n';
}


void System::ConfigCheckRoutes(misc::IniFile *ini_file)
{
	// For each module, check accessibility to low/high modules
	debug << "Checking accessibility to low and high modules:\n";
	for (auto &module : modules)
	{
		// Debug
		debug << "\t" << module->getName() << '\n';

		// List of low modules
		debug << "\t\tLow modules:";
		for (int i = 0; i < module->getNumLowModules(); i++)
		{
			// Get low module
			Module *low_module = module->getLowModule(i);
			debug << " " << low_module->getName();

			// Check that nodes are in the same network
			if (module->getLowNetwork() != low_module->getHighNetwork())
				throw Error(misc::fmt("%s: %s: low node '%s' "
						"is not in the same network.\n%s",
						ini_file->getPath().c_str(),
						module->getName().c_str(),
						low_module->getName().c_str(),
						err_config_note));

			// Check that there is a route
			net::Network *network = module->getLowNetwork();
			net::RoutingTable *routing_table = network->getRoutingTable();
			net::RoutingTable::Entry *entry = routing_table->Lookup(
					module->getLowNetworkNode(),
					low_module->getHighNetworkNode());
			if (!entry->getBuffer())
				throw Error(misc::fmt("%s: %s: network does not "
						"connect '%s' with '%s'. %s",
						ini_file->getPath().c_str(),
						network->getName().c_str(),
						module->getName().c_str(),
						low_module->getName().c_str(),
						err_connect));
		}

		// List of high modules
		debug << "\n\t\tHigh modules:";
		for (int i = 0; i < module->getNumHighModules(); i++)
		{
			// Get high module
			Module *high_module = module->getHighModule(i);
			debug << " " << high_module->getName();

			// Check that noes are in the same network
			if (module->getHighNetwork() != high_module->getLowNetwork())
				throw Error(misc::fmt("%s: %s: high node '%s' "
						"is not in the same network.\n%s",
						ini_file->getPath().c_str(),
						module->getName().c_str(),
						high_module->getName().c_str(),
						err_config_note));

			// Check that there is a route
			net::Network *network = module->getHighNetwork();
			net::RoutingTable *routing_table = network->getRoutingTable();
			net::RoutingTable::Entry *entry = routing_table->Lookup(
					module->getHighNetworkNode(),
					high_module->getLowNetworkNode());
			if (!entry->getBuffer())
				throw Error(misc::fmt("%s: %s: network does not "
						"connect '%s' with '%s'. %s",
						ini_file->getPath().c_str(),
						network->getName().c_str(),
						module->getName().c_str(),
						high_module->getName().c_str(),
						err_connect));
		}

		// Debug
		debug << '\n';
	}

	// Debug
	debug << '\n';
}


void System::ConfigCalculateSubBlockSizes()
{
	debug << "Creating directories:\n";
	for (auto &module : modules)
	{
		// Calculate sub-block size
		int sub_block_size = module->getBlockSize();
		for (int i = 0; i < module->getNumHighModules(); i++)
		{
			Module *high_module = module->getHighModule(i);
			sub_block_size = std::min(sub_block_size,
					high_module->getBlockSize());
		}
		module->setSubBlockSize(sub_block_size);

		// Get number of nodes for directory
		net::Network *high_network = module->getHighNetwork();
		int num_nodes = high_network && high_network->getNumNodes() ?
				high_network->getNumNodes() : 1;

		// Create directory
		module->InitializeDirectory(
				module->getDirectoryNumSets(),
				module->getDirectoryNumWays(),
				module->getNumSubBlocks(),
				num_nodes);
		Directory *directory = module->getDirectory();
		debug << misc::fmt("\t%s - %dx%dx%d (%dx%dx%d effective) - "
				"%d entries, %d sub-blocks\n",
				module->getName().c_str(),
				directory->getNumSets(),
				directory->getNumWays(),
				num_nodes,
				directory->getNumSets(),
				directory->getNumWays(),
				module->getNumHighModules(),
				module->getDirectorySize(),
				module->getNumSubBlocks());
	}

	// Debug
	debug << '\n';
}


void System::ConfigSetModuleLevel(Module *module, int level)
{
	// If level is already set, do nothing
	if (module->getLevel() >= level)
		return;

	// Set level of module and lower modules
	module->setLevel(level);
	for (int i = 0; i < module->getNumLowModules(); i++)
	{
		Module *low_module = module->getLowModule(i);
		ConfigSetModuleLevel(low_module, level + 1);
	}
}


void System::ConfigCalculateModuleLevels()
{
	// Start recursive level assignment with L1 modules (entries to memory)
	// for all architectures.
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
	for (auto it = arch_pool->getTimingBegin(),
			e = arch_pool->getTimingEnd();
			it != e;
			++it)
	{
		// Get timing simulator
		comm::Arch *arch = *it;
		comm::Timing *timing = arch->getTiming();

		// Traverse entries to memory hierarchy
		for (int i = 0; i < timing->getNumEntryModules(); i++)
		{
			Module *module = timing->getEntryModule(i);
			ConfigSetModuleLevel(module, 1);
		}
	}

	// Debug
	debug << "Calculating module levels:\n";
	for (auto &module : modules)
	{
		debug << "\t" << module->getName() << " -> ";
		if (module->getLevel())
			debug << "level " << module->getLevel() << '\n';
		else
			debug << "not accessible\n";
	}
	debug << '\n';
}


void System::ConfigTrace()
{
	// Initialization
	trace.Header(misc::fmt("mem.init version=\"%d.%d\"\n",
			trace_version_major, trace_version_minor));

	// External Networks trace commands from network section
	net::System *net_system = net::System::getInstance();
	if (net_system->getNumNetworks())
	{
		net_system->TraceHeader();

		for (int i = 0; i < net_system->getNumNetworks(); i++)
		{
			net::Network *net = net_system->getNetwork(i);

			// Trace command from the memory section
			trace.Header(misc::fmt("mem.new_net name=\"%s\" "
					"num_nodes=%d\n",
					net->getName().c_str(), 
					net->getNumNodes()));
		}
	}
	else
	{
		net_system->trace.Off();
	}

	// Internal Networks
	for (auto& net : networks)
	{
		// Trace commands from Network section
		net->TraceHeader();

		// Trace commands from Memory section
		trace.Header(misc::fmt("mem.new_net name=\"%s\" num_nodes=%d\n",
				net->getName().c_str(), net->getNumNodes()));
	}

	// Modules
	for (auto &mod : modules)
	{
		// If module is unreachable, ignore it
		if (!mod->getLevel())
			continue;

		// High Network
		// Get high network name
		std::string high_net_name = "";
		if (net::Network *high_network = mod->getHighNetwork())
			high_net_name =  high_network->getName();

		// Get high network node index
		int high_net_node_index = 0;
		if (net::Node *high_net_node = mod->getHighNetworkNode())
			high_net_node_index = high_net_node->getIndex();

		// Low Network
		// Get low network name
		std::string low_net_name = "";
		if (net::Network *low_network = mod->getLowNetwork())
			low_net_name =  low_network->getName();

		// Get low network node index
		int low_net_node_index = 0;
		if (net::Node *low_net_node = mod->getLowNetworkNode())
			low_net_node_index = low_net_node->getIndex();

		// Get cache data: number of sets, associativity, block size
		unsigned num_sets = 0, assoc = 0, block_size = 0;
		if (Cache *cache = mod->getCache())
		{
			num_sets = cache->getNumSets();
			assoc = cache->getNumWays();
			block_size = cache->getBlockSize();
		}

		// Get number of sharers of directory
		unsigned num_sharers = 0;
		if (Directory *directory = mod->getDirectory()) {
		    num_sharers = directory->getNumNodes();
		}

		// Trace Header
		trace.Header(misc::fmt("mem.new_mod name=\"%s\" num_sets=%u assoc=%u "
				"block_size=%d sub_block_size=%d num_sharers=%d level=%d "
				"high_net=\"%s\" high_net_node=%d low_net=\"%s\" low_net_node=%d\n",
				mod->getName().c_str(), num_sets, assoc,
				block_size, mod->getSubBlockSize(),
				num_sharers, mod->getLevel(),
				high_net_name.c_str(), high_net_node_index,
				low_net_name.c_str(), low_net_node_index));
	}

}


void System::ConfigReadCommands(misc::IniFile *ini_file)
{
}


void System::ReadConfiguration()
{
	// Load memory system configuration file. If no file name has been given
	// by the user, create a default configuration for each architecture.
	misc::IniFile ini_file;
	if (config_file.empty())
	{
		// Create default configuration files for each architecture
		comm::ArchPool *arch_pool = comm::ArchPool::getInstance();
		for (auto it = arch_pool->getTimingBegin(),
				e = arch_pool->getTimingEnd();
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

	// Read from INI file
	ReadConfiguration(&ini_file);
}


void System::ReadConfiguration(misc::IniFile *ini_file)
{
	// Read general variables
	ConfigReadGeneral(ini_file);

	// Read networks
	ConfigReadNetworks(ini_file);

	// Read modules
	ConfigReadModules(ini_file);

	// Read low level caches
	ConfigReadLowModules(ini_file);

	// Read entries from requesting devices (CPUs/GPUs) to memory system
	// entries. This is presented in [Entry <name>] sections in the
	// configuration file.
	ConfigReadEntries(ini_file);

	// Create switches in internal networks
	ConfigCreateSwitches(ini_file);

	// Read commands from the configuration file. Commands are used to
	// artificially alter the initial state of the memory hierarchy for
	// debugging purposes.
	ConfigReadCommands(ini_file);

	// Check that all enforced sections and variables were specified.
	ini_file->Check();

	// Check routes to low and high modules
	ConfigCheckRoutes(ini_file);

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

