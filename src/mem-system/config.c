/*
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

#include <arch/common/arch.h>
#include <arch/common/arch-list.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/timing/cpu.h>
#include <arch/evergreen/emu/emu.h>
#include <arch/evergreen/timing/compute-unit.h>
#include <arch/evergreen/timing/gpu.h>
#include <arch/southern-islands/timing/timing.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <network/net-system.h>
#include <network/network.h>
#include <network/node.h>
#include <network/routing-table.h>

#include "cache.h"
#include "command.h"
#include "directory.h"
#include "mem-system.h"
#include "mmu.h"
#include "prefetcher.h"

/*
 * Global Variables
 */

char *mem_config_file_name = "";

char *mem_config_help =
	"Option '--mem-config <file>' is used to configure the memory system. The\n"
	"configuration file is a plain-text file in the IniFile format. The memory system\n"
	"is formed of a set of cache modules, main memory modules, and interconnects.\n"
	"\n"
	"Interconnects can be defined in two different configuration files. The first way\n"
	"is using option '--net-config <file>' (use option '--help-net-config' for more\n"
	"information). Any network defined in the network configuration file can be\n"
	"referenced from the memory configuration file. These networks will be referred\n"
	"hereafter as external networks.\n"
	"\n"
	"The second option to define a network straight in the memory system\n"
	"configuration. This alternative is provided for convenience and brevity. By\n"
	"using sections [Network <name>], networks with a default topology are created,\n"
	"which include a single switch, and one bidirectional link from the switch to\n"
	"every end node present in the network.\n"
	"\n"
	"The following sections and variables can be used in the memory system\n"
	"configuration file:\n"
	"\n"
	"Section [General] defines global parameters affecting the entire memory system.\n"
	"\n"
	"  PageSize = <size>  (Default = 4096)\n"
	"      Memory page size. Virtual addresses are translated into new physical\n"
	"      addresses in ascending order at the granularity of the page size.\n"
	"  PeerTransfers = <bool> (Default = transfers)\n"
	"      Whether or not transfers between peer caches are used.\n"
	"\n"
	"Section [Module <name>] defines a generic memory module. This section is used to\n"
	"declare both caches and main memory modules accessible from CPU cores or GPU\n"
	"compute units.\n"
	"\n"
	"  Type = {Cache|MainMemory}  (Required)\n"
	"      Type of the memory module. From the simulation point of view, the\n"
	"      difference between a cache and a main memory module is that the former\n"
	"      contains only a subset of the data located at the memory locations it\n"
	"      serves.\n"
	"  Geometry = <geo>\n"
	"      Cache geometry, defined in a separate section of type [Geometry <geo>].\n"
	"      This variable is required for cache modules.\n"
	"  LowNetwork = <net>\n"
	"      Network connecting the module with other lower-level modules, i.e.,\n"
	"      modules closer to main memory. This variable is mandatory for caches, and\n"
	"      should not appear for main memory modules. Value <net> can refer to an\n"
	"      internal network defined in a [Network <net>] section, or to an external\n"
	"      network defined in the network configuration file.\n"
	"  LowNetworkNode = <node>\n"
	"      If 'LowNetwork' points to an external network, node in the network that\n"
	"      the module is mapped to. For internal networks, this variable should be\n"
	"      omitted.\n"
	"  HighNetwork = <net>\n"
	"      Network connecting the module with other higher-level modules, i.e.,\n"
	"      modules closer to CPU cores or GPU compute units. For highest level\n"
	"      modules accessible by CPU/GPU, this variable should be omitted.\n"
	"  HighNetworkNode = <node>\n"
	"      If 'HighNetwork' points to an external network, node that the module is\n"
	"      mapped to.\n"
	"  LowModules = <mod1> [<mod2> ...]\n"
	"      List of lower-level modules. For a cache module, this variable is required.\n"
	"      If there is only one lower-level module, it serves the entire address\n"
	"      space for the current module. If there are several lower-level modules,\n"
	"      each served a disjoint subset of the address space. This variable should\n"
	"      be omitted for main memory modules.\n"
	"  BlockSize = <size>\n"
	"      Block size in bytes. This variable is required for a main memory module.\n"
	"      It should be omitted for a cache module (in this case, the block size is\n"
	"      specified in the corresponding cache geometry section).\n"
	"  Latency = <cycles>\n"
	"      Memory access latency. This variable is required for a main memory module,\n"
	"      and should be omitted for a cache module (the access latency is specified\n"
	"      in the corresponding cache geometry section in this case).\n"
	"  Ports = <num>\n"
	"      Number of read/write ports. This variable is only allowed for a main memory\n"
	"      module. The number of ports for a cache is specified in a separate cache\n"
	"      geometry section.\n"
	"  DirectorySize <size>\n"
	"      Size of the directory in number of blocks. The size of a directory limits\n"
	"      the number of different blocks that can reside in upper-level caches. If a\n"
	"      cache requests a new block from main memory, and its directory is full, a\n"
	"      previous block must be evicted from the directory, and all its occurrences\n"
	"      in the memory hierarchy need to be first invalidated. This variable is only\n"
	"      allowed for a main memory module.\n"
	"  DirectoryAssoc = <assoc>\n"
	"      Directory associativity in number of ways. This variable is only allowed\n"
	"      for a main memory module.\n"
	"  AddressRange = { BOUNDS <low> <high> | ADDR DIV <div> MOD <mod> EQ <eq> }\n"
	"      Physical address range served by the module. If not specified, the entire\n"
	"      address space is served by the module. There are two possible formats for\n"
	"      the value of 'Range':\n"
	"      With the first format, the user can specify the lowest and highest byte\n"
	"      included in the address range. The value in <low> must be a multiple of\n"
	"      the module block size, and the value in <high> must be a multiple of the\n"
	"      block size minus 1.\n"
	"      With the second format, the address space can be split between different\n"
	"      modules in an interleaved manner. If dividing an address by <div> and\n"
	"      modulo <mod> makes it equal to <eq>, it is served by this module. The\n"
	"      value of <div> must be a multiple of the block size. When a module serves\n"
	"      only a subset of the address space, the user must make sure that the rest\n"
	"      of the modules at the same level serve the remaining address space.\n"
	"\n"
	"Section [CacheGeometry <geo>] defines a geometry for a cache. Caches using this\n"
	"geometry are instantiated [Module <name>] sections.\n"
	"\n"
	"  Sets = <num_sets> (Required)\n"
	"      Number of sets in the cache.\n"
	"  Assoc = <num_ways> (Required)\n"
	"      Cache associativity. The total number of blocks contained in the cache\n"
	"      is given by the product Sets * Assoc.\n"
	"  BlockSize = <size> (Required)\n"
	"      Size of a cache block in bytes. The total size of the cache is given by\n"
	"      the product Sets * Assoc * BlockSize.\n"
	"  Latency = <cycles> (Required)\n"
	"      Hit latency for a cache in number of cycles.\n"
	"  Policy = {LRU|FIFO|Random} (Default = LRU)\n"
	"      Block replacement policy.\n"
	"  MSHR = <size> (Default = 16)\n"
	"      Miss status holding register (MSHR) size in number of entries. This value\n"
	"      determines the maximum number of accesses that can be in flight for the\n"
	"      cache, including the time since the access request is received, until a\n"
	"      potential miss is resolved.\n"
	"  Ports = <num> (Default = 2)\n"
	"      Number of ports. The number of ports in a cache limits the number of\n"
	"      concurrent hits. If an access is a miss, it remains in the MSHR while it\n"
	"      is resolved, but releases the cache port.\n"
	"  DirectoryLatency = <cycles> (Default = 1)\n"
	"      Latency for a directory access in number of cycles.\n"
	"  EnablePrefetcher = {t|f} (Default = False)\n"
	"      Whether the hardware should automatically perform prefetching.\n"
	"      The prefetcher related options below will be ignored if this is not true.\n"
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
	"Section [Network <net>] defines an internal default interconnect, formed of a\n"
	"single switch connecting all modules pointing to the network. For every module\n"
	"in the network, a bidirectional link is created automatically between the module\n"
	"and the switch, together with the suitable input/output buffers in the switch\n"
	"and the module.\n"
	"\n"
	"  DefaultInputBufferSize = <size>\n"
	"      Size of input buffers for end nodes (memory modules) and switch.\n"
	"  DefaultOutputBufferSize = <size>\n"
	"      Size of output buffers for end nodes and switch. \n"
	"  DefaultBandwidth = <bandwidth>\n"
	"      Bandwidth for links and switch crossbar in number of bytes per cycle.\n"
	"\n"
	"Section [Entry <name>] creates an entry into the memory system. An entry is a\n"
	"connection between a CPU core/thread or a GPU compute unit with a module in the\n"
	"memory system.\n"
	"\n"
	"  Arch = { x86 | Evergreen | SouthernIslands | ... }\n"
	"      CPU or GPU architecture affected by this entry.\n"
	"  Core = <core>\n"
	"      CPU core identifier. This is a value between 0 and the number of cores\n"
	"      minus 1, as defined in the CPU configuration file. This variable should be\n"
	"      omitted for GPU entries.\n"
	"  Thread = <thread>\n"
	"      CPU thread identifier. Value between 0 and the number of threads per core\n"
	"      minus 1. Omitted for GPU entries.\n"
	"  ComputeUnit = <id>\n"
	"      GPU compute unit identifier. Value between 0 and the number of compute\n"
	"      units minus 1, as defined in the GPU configuration file. This variable\n"
	"      should be omitted for CPU entries.\n"
	"  DataModule = <mod>\n"
	"  InstModule = <mod>\n"
	"      In architectures supporting separate data/instruction caches, modules used\n"
	"      to access memory for each particular purpose.\n"
	"  Module = <mod>\n"
	"      Module used to access the memory hierarchy. For architectures supporting\n"
	"      separate data/instruction caches, this variable can be used instead of\n"
	"      'DataModule' and 'InstModule' to indicate that data and instruction caches\n"
	"      are unified.\n"
	"\n";



/*
 * Private functions
 */

#define MEM_SYSTEM_MAX_LEVELS  10

static char *err_mem_config_note =
	"\tPlease run 'm2s --mem-help' or consult the Multi2Sim Guide for\n"
	"\ta description of the memory system configuration file format.\n";

static char *err_mem_config_net =
	"\tNetwork identifiers need to be declared either in the cache configuration\n"
	"\tfile, or in the network configuration file (option '--net-config').\n";

static char *err_mem_levels =
	"\tThe path from a cache into main memory exceeds 10 levels of cache.\n"
	"\tThis might be a symptom of a recursive reference in 'LowModules'\n"
	"\tlists. If you really intend to have a high number of cache levels,\n"
	"\tincrease variable MEM_SYSTEM_MAX_LEVELS in '" __FILE__ "'\n";

static char *err_mem_block_size =
	"\tBlock size in a cache must be greater or equal than its lower-level\n"
	"\tcache for correct behavior of directories and coherence protocols.\n";

static char *err_mem_connect =
	"\tAn external network is used that does not provide connectivity between\n"
	"\ta memory module and an associated low/high module. Please add the\n"
	"\tnecessary links in the network configuration file.\n";


static void mem_config_default(struct config_t *config)
{
	struct arch_t *arch;
	int i;

	LIST_FOR_EACH(arch_list, i)
	{
		/* Only for architectures in detailed simulation */
		arch = list_get(arch_list, i);
		if (arch->sim_kind == arch_sim_kind_functional)
			continue;

		/* Architecture must have registered its 'mem_config_default' function */
		if (!arch->mem_config_default_func)
			panic("%s: architecture '%s' didn't register 'mem_config_default'",
				__FUNCTION__, arch->name);

		/* Make call back */
		arch->mem_config_default_func(config);
	}
}


static void mem_config_read_general(struct config_t *config)
{
	char *section;

	/* Section with general parameters */
	section = "General";

	/* Page size */
	mmu_page_size = config_read_int(config, section, "PageSize", mmu_page_size);
	if ((mmu_page_size & (mmu_page_size - 1)))
		fatal("%s: page size must be power of 2.\n%s",
			mem_config_file_name, err_mem_config_note);

	/* Peer transfers */
	mem_system_peer_transfers = config_read_bool(config, section, "PeerTranfers", 1);
}


static void mem_config_read_networks(struct config_t *config)
{
	struct net_t *net;
	int i;

	char buf[MAX_STRING_SIZE];
	char *section;

	/* Create networks */
	mem_debug("Creating internal networks:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *net_name;

		/* Network section */
		if (strncasecmp(section, "Network ", 8))
			continue;
		net_name = section + 8;

		/* Create network */
		net = net_create(net_name);
		mem_debug("\t%s\n", net_name);
		list_add(mem_system->net_list, net);
	}
	mem_debug("\n");

	/* Add network pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections.
	 * Also check integrity of sections. */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		/* Get network section name */
		net = list_get(mem_system->net_list, i);
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));

		/* Add pointer */
		config_write_ptr(config, buf, "ptr", net);

		/* Check section integrity */
		config_var_enforce(config, buf, "DefaultInputBufferSize");
		config_var_enforce(config, buf, "DefaultOutputBufferSize");
		config_var_enforce(config, buf, "DefaultBandwidth");
		config_section_check(config, buf);
	}
}


static void mem_config_insert_module_in_network(struct config_t *config,
	struct mod_t *mod, char *net_name, char *net_node_name,
	struct net_t **net_ptr, struct net_node_t **net_node_ptr)
{
	struct net_t *net;
	struct net_node_t *node;

	int def_input_buffer_size;
	int def_output_buffer_size;

	char buf[MAX_STRING_SIZE];

	/* No network specified */
	*net_ptr = NULL;
	*net_node_ptr = NULL;
	if (!*net_name)
		return;

	/* Try to insert in private network */
	snprintf(buf, sizeof buf, "Network %s", net_name);
	net = config_read_ptr(config, buf, "ptr", NULL);
	if (!net)
		goto try_external_network;

	/* For private networks, 'net_node_name' should be empty */
	if (*net_node_name)
		fatal("%s: %s: network node name should be empty.\n%s",
			mem_config_file_name, mod->name,
			err_mem_config_note);

	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network '%s' already contains module '%s'.\n%s",
			mem_config_file_name, net->name,
			mod->name, err_mem_config_note);

	/* Read buffer sizes from network */
	def_input_buffer_size = config_read_int(config, buf, "DefaultInputBufferSize", 0);
	def_output_buffer_size = config_read_int(config, buf, "DefaultOutputBufferSize", 0);
	if (!def_input_buffer_size)
		fatal("%s: network %s: variable 'DefaultInputBufferSize' missing.\n%s",
			mem_config_file_name, net->name, err_mem_config_note);
	if (!def_output_buffer_size)
		fatal("%s: network %s: variable 'DefaultOutputBufferSize' missing.\n%s",
			mem_config_file_name, net->name, err_mem_config_note);
	if (def_input_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum input buffer size is %d for cache '%s'.\n%s",
			mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_mem_config_note);
	if (def_output_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum output buffer size is %d for cache '%s'.\n%s",
			mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_mem_config_note);

	/* Insert module in network */
	node = net_add_end_node(net, def_input_buffer_size, def_output_buffer_size,
		mod->name, mod);

	/* Return */
	*net_ptr = net;
	*net_node_ptr = node;
	return;


try_external_network:

	/* Get network */
	net = net_find(net_name);
	if (!net)
		fatal("%s: %s: invalid network name.\n%s%s",
			mem_config_file_name, net_name,
			err_mem_config_note, err_mem_config_net);

	/* Node name must be specified */
	if (!*net_node_name)
		fatal("%s: %s: network node name required for external network.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_config_note, err_mem_config_net);

	/* Get node */
	node = net_get_node_by_name(net, net_node_name);
	if (!node)
		fatal("%s: network %s: node %s: invalid node name.\n%s%s",
			mem_config_file_name, net_name, net_node_name,
			err_mem_config_note, err_mem_config_net);

	/* No module must have been assigned previously to this node */
	if (node->user_data)
		fatal("%s: network %s: node '%s' already assigned.\n%s",
			mem_config_file_name, net->name,
			net_node_name, err_mem_config_note);

	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network %s: module '%s' is already present.\n%s",
			mem_config_file_name, net->name,
			mod->name, err_mem_config_note);

	/* Assign module to network node and return */
	node->user_data = mod;
	*net_ptr = net;
	*net_node_ptr = node;
}


static struct mod_t *mem_config_read_cache(struct config_t *config, char *section)
{
	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];

	int num_sets;
	int assoc;
	int block_size;
	int latency;
	int dir_latency;

	char *policy_str;
	enum cache_policy_t policy;

	int mshr_size;
	int num_ports;

	int enable_prefetcher;
	char *prefetcher_type_str;
	enum prefetcher_type_t prefetcher_type;
	int prefetcher_ghb_size;
	int prefetcher_it_size;
	int prefetcher_lookup_depth;

	char *net_name;
	char *net_node_name;

	struct mod_t *mod;
	struct net_t *net;
	struct net_node_t *net_node;

	/* Cache parameters */
	snprintf(buf, sizeof buf, "CacheGeometry %s",
		config_read_string(config, section, "Geometry", ""));
	config_var_enforce(config, section, "Geometry");
	config_section_enforce(config, buf);
	config_var_enforce(config, buf, "Latency");
	config_var_enforce(config, buf, "Sets");
	config_var_enforce(config, buf, "Assoc");
	config_var_enforce(config, buf, "BlockSize");

	/* Read values */
	str_token(mod_name, sizeof mod_name, section, 1, " ");
	num_sets = config_read_int(config, buf, "Sets", 16);
	assoc = config_read_int(config, buf, "Assoc", 2);
	block_size = config_read_int(config, buf, "BlockSize", 256);
	latency = config_read_int(config, buf, "Latency", 1);
	dir_latency = config_read_int(config, buf, "DirectoryLatency", 1);
	policy_str = config_read_string(config, buf, "Policy", "LRU");
	mshr_size = config_read_int(config, buf, "MSHR", 16);
	num_ports = config_read_int(config, buf, "Ports", 2);
	enable_prefetcher = config_read_bool(config, buf, "EnablePrefetcher", 0);
	prefetcher_type_str = config_read_string(config, buf, "PrefetcherType", "GHB_PC_CS");
	prefetcher_ghb_size = config_read_int(config, buf, "PrefetcherGHBSize", 256);
	prefetcher_it_size = config_read_int(config, buf, "PrefetcherITSize", 64);
	prefetcher_lookup_depth = config_read_int(config, buf, "PrefetcherLookupDepth", 2);

	/* Checks */
	policy = str_map_string_case(&cache_policy_map, policy_str);
	if (policy == cache_policy_invalid)
		fatal("%s: cache %s: %s: invalid block replacement policy.\n%s",
			mem_config_file_name, mod_name,
			policy_str, err_mem_config_note);
	if (num_sets < 1 || (num_sets & (num_sets - 1)))
		fatal("%s: cache %s: number of sets must be a power of two greater than 1.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (assoc < 1 || (assoc & (assoc - 1)))
		fatal("%s: cache %s: associativity must be power of two and > 1.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (block_size < 4 || (block_size & (block_size - 1)))
		fatal("%s: cache %s: block size must be power of two and at least 4.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (dir_latency < 1)
		fatal("%s: cache %s: invalid value for variable 'DirectoryLatency'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (latency < 1)
		fatal("%s: cache %s: invalid value for variable 'Latency'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (mshr_size < 1)
		fatal("%s: cache %s: invalid value for variable 'MSHR'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (num_ports < 1)
		fatal("%s: cache %s: invalid value for variable 'Ports'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (enable_prefetcher)
	{
		prefetcher_type = str_map_string_case(&prefetcher_type_map, prefetcher_type_str);
		if (prefetcher_ghb_size < 1 || prefetcher_it_size < 1 ||
		    prefetcher_type == prefetcher_type_invalid || prefetcher_lookup_depth < 2 || 
		    prefetcher_lookup_depth > PREFETCHER_LOOKUP_DEPTH_MAX)
			fatal("%s: cache %s: invalid prefetcher configuration.\n%s",
			      mem_config_file_name, mod_name, err_mem_config_note);
	}

	/* Create module */
	mod = mod_create(mod_name, mod_kind_cache, num_ports,
		block_size, latency);
	
	/* Initialize */
	mod->mshr_size = mshr_size;
	mod->dir_assoc = assoc;
	mod->dir_num_sets = num_sets;
	mod->dir_size = num_sets * assoc;
	mod->dir_latency = dir_latency;

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;

	/* Low network */
	net_name = config_read_string(config, section, "LowNetwork", "");
	net_node_name = config_read_string(config, section, "LowNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->low_net = net;
	mod->low_net_node = net_node;

	/* Create cache */
	mod->cache = cache_create(mod->name, num_sets, block_size, assoc, policy);

	/* Fill in prefetcher parameters */
	if (enable_prefetcher)
		mod->cache->prefetcher = prefetcher_create(prefetcher_ghb_size, prefetcher_it_size, 
							   prefetcher_lookup_depth, prefetcher_type);

	/* Return */
	return mod;
}


static struct mod_t *mem_config_read_main_memory(struct config_t *config, char *section)
{
	char mod_name[MAX_STRING_SIZE];

	int block_size;
	int latency;
	int num_ports;
	int dir_size;
	int dir_assoc;

	char *net_name;
	char *net_node_name;

	struct mod_t *mod;
	struct net_t *net;
	struct net_node_t *net_node;

	/* Read parameters */
	str_token(mod_name, sizeof mod_name, section, 1, " ");
	config_var_enforce(config, section, "Latency");
	config_var_enforce(config, section, "BlockSize");
	block_size = config_read_int(config, section, "BlockSize", 64);
	latency = config_read_int(config, section, "Latency", 1);
	num_ports = config_read_int(config, section, "Ports", 2);
	dir_size = config_read_int(config, section, "DirectorySize", 1024);
	dir_assoc = config_read_int(config, section, "DirectoryAssoc", 8);

	/* Check parameters */
	if (block_size < 1 || (block_size & (block_size - 1)))
		fatal("%s: %s: block size must be power of two.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (latency < 1)
		fatal("%s: %s: invalid value for variable 'Latency'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (num_ports < 1)
		fatal("%s: %s: invalid value for variable 'NumPorts'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (dir_size < 1 || (dir_size & (dir_size - 1)))
		fatal("%s: %s: directory size must be a power of two.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (dir_assoc < 1 || (dir_assoc & (dir_assoc - 1)))
		fatal("%s: %s: directory associativity must be a power of two.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (dir_assoc > dir_size)
		fatal("%s: %s: invalid directory associativity.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);

	/* Create module */
	mod = mod_create(mod_name, mod_kind_main_memory, num_ports,
			block_size, latency);

	/* Store directory size */
	mod->dir_size = dir_size;
	mod->dir_assoc = dir_assoc;
	mod->dir_num_sets = dir_size / dir_assoc;

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
			&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;

	/* Create cache and directory */
	mod->cache = cache_create(mod->name, dir_size / dir_assoc, block_size,
			dir_assoc, cache_policy_lru);

	/* Return */
	return mod;
}


static void mem_config_read_module_address_range(struct config_t *config,
	struct mod_t *mod, char *section)
{
	char *range_str;
	char *token;
	char *delim;

	int err;

	/* Read address range */
	range_str = config_read_string(config, section, "AddressRange", "");
	if (!*range_str)
	{
		mod->range_kind = mod_range_bounds;
		mod->range.bounds.low = 0;
		mod->range.bounds.high = -1;
		return;
	}

	/* Split in tokens */
	range_str = xstrdup(range_str);
	delim = " ";
	token = strtok(range_str, delim);
	if (!token)
		goto invalid_format;

	/* First token - ADDR or BOUNDS */
	if (!strcasecmp(token, "BOUNDS"))
	{
		/* Format is: BOUNDS <low> <high> */
		mod->range_kind = mod_range_bounds;

		/* Low bound */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.bounds.low = str_to_int(token, &err);
		if (err)
			fatal("%s: %s: invalid value '%s' in 'AddressRange'",
				mem_config_file_name, mod->name, token);
		if (mod->range.bounds.low % mod->block_size)
			fatal("%s: %s: low address bound must be a multiple of block size.\n%s",
				mem_config_file_name, mod->name, err_mem_config_note);

		/* High bound */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.bounds.high = str_to_int(token, &err);
		if (err)
			fatal("%s: %s: invalid value '%s' in 'AddressRange'",
				mem_config_file_name, mod->name, token);
		if ((mod->range.bounds.high + 1) % mod->block_size)
			fatal("%s: %s: high address bound must be a multiple of block size minus 1.\n%s",
				mem_config_file_name, mod->name, err_mem_config_note);

		/* No more tokens */
		if ((token = strtok(NULL, delim)))
			goto invalid_format;
	}
	else if (!strcasecmp(token, "ADDR"))
	{
		/* Format is: ADDR DIV <div> MOD <mod> EQ <eq> */
		mod->range_kind = mod_range_interleaved;

		/* Token 'DIV' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "DIV"))
			goto invalid_format;

		/* Field <div> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.div = str_to_int(token, &err);
		if (err)
			fatal("%s: %s: invalid value '%s' in 'AddressRange'",
				mem_config_file_name, mod->name, token);
		if (mod->range.interleaved.div < 1)
			goto invalid_format;
		if (mod->range.interleaved.div % mod->block_size)
			fatal("%s: %s: value for <div> must be a multiple of block size.\n%s",
				mem_config_file_name, mod->name, err_mem_config_note);

		/* Token 'MOD' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "MOD"))
			goto invalid_format;

		/* Field <mod> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.mod = str_to_int(token, &err);
		if (err)
			fatal("%s: %s: invalid value '%s' in 'AddressRange'",
				mem_config_file_name, mod->name, token);
		if (mod->range.interleaved.mod < 1)
			goto invalid_format;

		/* Token 'EQ' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "EQ"))
			goto invalid_format;

		/* Field <eq> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.eq = str_to_int(token, &err);
		if (err)
			fatal("%s: %s: invalid value '%s' in 'AddressRange'",
				mem_config_file_name, mod->name, token);
		if (mod->range.interleaved.eq >= mod->range.interleaved.mod)
			goto invalid_format;

		/* No more tokens */
		if ((token = strtok(NULL, delim)))
			goto invalid_format;
	}
	else
	{
		goto invalid_format;
	}

	/* Free string duplicate */
	free(range_str);
	return;

invalid_format:

	fatal("%s: %s: invalid format for 'AddressRange'.\n%s",
		mem_config_file_name, mod->name, err_mem_config_note);
}


static void mem_config_read_modules(struct config_t *config)
{
	struct mod_t *mod;

	char *section;
	char *mod_type;

	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];
	int i;

	/* Create modules */
	mem_debug("Creating modules:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section for a module */
		if (strncasecmp(section, "Module ", 7))
			continue;

		/* Create module, depending on the type. */
		str_token(mod_name, sizeof mod_name, section, 1, " ");
		mod_type = config_read_string(config, section, "Type", "");
		if (!strcasecmp(mod_type, "Cache"))
			mod = mem_config_read_cache(config, section);
		else if (!strcasecmp(mod_type, "MainMemory"))
			mod = mem_config_read_main_memory(config, section);
		else
			fatal("%s: %s: invalid or missing value for 'Type'.\n%s",
				mem_config_file_name, mod_name,
				err_mem_config_note);

		/* Read module address range */
		mem_config_read_module_address_range(config, mod, section);

		/* Add module */
		list_add(mem_system->mod_list, mod);
		mem_debug("\t%s\n", mod_name);
	}

	/* Debug */
	mem_debug("\n");

	/* Add module pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections.
	 * Also check integrity of sections. */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get module and section */
		mod = list_get(mem_system->mod_list, i);
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", mod);
	}
}


static void mem_config_check_route_to_main_memory(struct mod_t *mod, int block_size, int level)
{
	struct mod_t *low_mod;
	int i;

	/* Maximum level */
	if (level > MEM_SYSTEM_MAX_LEVELS)
		fatal("%s: %s: too many cache levels.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_levels, err_mem_config_note);

	/* Check block size */
	if (mod->block_size < block_size)
		fatal("%s: %s: decreasing block size.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_block_size, err_mem_config_note);
	block_size = mod->block_size;

	/* Dump current module */
	mem_debug("\t");
	for (i = 0; i < level * 2; i++)
		mem_debug(" ");
	mem_debug("%s\n", mod->name);

	/* Check that cache has a way to main memory */
	if (!linked_list_count(mod->low_mod_list) && mod->kind == mod_kind_cache)
		fatal("%s: %s: main memory not accessible from cache.\n%s",
			mem_config_file_name, mod->name,
			err_mem_config_note);

	/* Dump children */
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		mem_config_check_route_to_main_memory(low_mod, block_size, level + 1);
	}
}

static void mem_config_read_low_modules(struct config_t *config)
{
	char buf[MAX_STRING_SIZE];
	char *delim;

	char *low_mod_name;
	char *low_mod_name_list;

	struct mod_t *mod;
	struct mod_t *low_mod;

	int i;

	/* Lower level modules */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get cache module */
		mod = list_get(mem_system->mod_list, i);
		if (mod->kind != mod_kind_cache)
			continue;

		/* Section name */
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));

		/* Low module name list */
		low_mod_name_list = config_read_string(config, buf, "LowModules", "");
		if (!*low_mod_name_list)
			fatal("%s: [ %s ]: missing or invalid value for 'LowModules'.\n%s",
				mem_config_file_name, buf, err_mem_config_note);

		/* For each element in the list */
		low_mod_name_list = xstrdup(low_mod_name_list);
		delim = ", ";
		for (low_mod_name = strtok(low_mod_name_list, delim);
			low_mod_name; low_mod_name = strtok(NULL, delim))
		{
			/* Check valid module name */
			snprintf(buf, sizeof buf, "Module %s", low_mod_name);
			if (!config_section_exists(config, buf))
				fatal("%s: %s: invalid module name in 'LowModules'.\n%s",
					mem_config_file_name, mod->name,
					err_mem_config_note);

			/* Get low cache and assign */
			low_mod = config_read_ptr(config, buf, "ptr", NULL);
			assert(low_mod);
			linked_list_add(mod->low_mod_list, low_mod);
			linked_list_add(low_mod->high_mod_list, mod);
		}

		/* Free copy of low module name list */
		free(low_mod_name_list);
	}

	/* Check paths to main memory */
	mem_debug("Checking paths between caches and main memories:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		mem_config_check_route_to_main_memory(mod, mod->block_size, 1);
	}
	mem_debug("\n");
}


static void mem_config_read_entries(struct config_t *config)
{
	char *section;
	char *entry_name;
	char *arch_name;

	char entry_name_trimmed[MAX_STRING_SIZE];
	char arch_name_trimmed[MAX_STRING_SIZE];
	char arch_list_names[MAX_STRING_SIZE];

	struct arch_t *arch;

	int i;

	/* Debug */
	mem_debug("Processing entries to the memory system:\n");
	mem_debug("\n");

	/* Read all [Entry <name>] sections */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Discard if not an entry section */
		if (strncasecmp(section, "Entry ", 6))
			continue;

		/* Name for the entry */
		entry_name = section + 6;
		str_trim(entry_name_trimmed, sizeof entry_name_trimmed, entry_name);
		if (!entry_name_trimmed[0])
			fatal("%s: section [%s]: invalid entry name.\n%s",
				mem_config_file_name, section, err_mem_config_note);

		/* Check if variable 'Type' is used in the section. This variable was used in
		 * previous versions, now it is replaced with 'Arch'. */
		if (config_var_exists(config, section, "Type"))
			fatal("%s: section [%s]: Variable 'Type' is obsolete, use 'Arch' instead.\n%s",
				mem_config_file_name, section, err_mem_config_note);

		/* Read architecture in variable 'Arch' */ 
		arch_name = config_read_string(config, section, "Arch", NULL);
		if (!arch_name)
			fatal("%s: section [%s]: Variable 'Arch' is missing.\n%s",
				mem_config_file_name, section, err_mem_config_note);

		/* Get architecture */
		str_trim(arch_name_trimmed, sizeof arch_name_trimmed, arch_name);
		arch = arch_list_get(arch_name_trimmed);
		if (!arch)
		{
			arch_list_get_names(arch_list_names, sizeof arch_list_names);
			fatal("%s: section [%s]: '%s' is an invalid value for 'Arch'.\n"
				"\tPossible values are %s.\n%s",
				mem_config_file_name, section, arch_name_trimmed,
				arch_list_names, err_mem_config_note);
		}

		/* An architecture with an entry in the memory configuration file must
		 * undergo a detailed simulation. */
		if (arch->sim_kind == arch_sim_kind_functional)
			fatal("%s: section [%s]: %s architecture not under detailed simulation.\n"
				"\tA CPU/GPU architecture uses functional simulation by default. Please\n"
				"\tactivate detailed simulation for the %s architecture using command-line\n"
				"\toption '--%s-sim detailed' to use this memory entry.\n",
				mem_config_file_name, section, arch->name, arch->name, arch->prefix);

		/* Check that callback functions are valid */
		if (!arch->mem_config_parse_entry_func)
			fatal("%s: section [%s]: %s architecture does not support entries.\n"
				"\tPlease contact development@multi2sim.org to report this problem.\n",
				mem_config_file_name, section, arch->name);

		/* Call function to process entry. Each architecture implements its own ways
		 * to process entries to the memory hierarchy. */
		arch->mem_config_parse_entry_func(config, section);
	}

	/* After processing all [Entry <name>] sections, check that all architectures
	 * satisfy their entries to the memory hierarchy. */
	LIST_FOR_EACH(arch_list, i)
	{
		/* Only architectures with detailed simulation */
		arch = list_get(arch_list, i);
		if (arch->sim_kind == arch_sim_kind_functional)
			continue;

		/* Check that 'mem_config_check' has been registered. */
		if (!arch->mem_config_check_func)
			panic("%s: architecture '%s' didn't register 'mem_config_check'",
				__FUNCTION__, arch->name);

		/* Make call-back */
		arch->mem_config_check_func(config);
	}
}


static void mem_config_create_switches(struct config_t *config)
{
	struct net_t *net;
	struct net_node_t *net_node;
	struct net_node_t *net_switch;

	char buf[MAX_STRING_SIZE];
	char net_switch_name[MAX_STRING_SIZE];

	int def_bandwidth;
	int def_input_buffer_size;
	int def_output_buffer_size;

	int i;
	int j;

	/* For each network, add a switch and create node connections */
	mem_debug("Creating network switches and links for internal networks:\n");
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		/* Get network and lower level cache */
		net = list_get(mem_system->net_list, i);

		/* Get switch bandwidth */
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));
		def_bandwidth = config_read_int(config, buf, "DefaultBandwidth", 0);
		if (def_bandwidth < 1)
			fatal("%s: %s: invalid or missing value for 'DefaultBandwidth'.\n%s",
				mem_config_file_name, net->name, err_mem_config_note);

		/* Get input/output buffer sizes.
		 * Checks for these variables has done before. */
		def_input_buffer_size = config_read_int(config, buf, "DefaultInputBufferSize", 0);
		def_output_buffer_size = config_read_int(config, buf, "DefaultOutputBufferSize", 0);
		assert(def_input_buffer_size > 0);
		assert(def_output_buffer_size > 0);

		/* Create switch */
		snprintf(net_switch_name, sizeof net_switch_name, "Switch");
		net_switch = net_add_switch(net, def_input_buffer_size, def_output_buffer_size,
			def_bandwidth, net_switch_name);
		mem_debug("\t%s.Switch ->", net->name);

		/* Create connections between switch and every end node */
		for (j = 0; j < list_count(net->node_list); j++)
		{
			net_node = list_get(net->node_list, j);
			if (net_node != net_switch)
			{
				net_add_bidirectional_link(net, net_node, net_switch,
					def_bandwidth, 1);
				mem_debug(" %s", net_node->name);
			}
		}

		/* Calculate routes */
		net_routing_table_initiate(net->routing_table);
		net_routing_table_floyd_warshall(net->routing_table);

		/* Debug */
		mem_debug("\n");
	}
	mem_debug("\n");

}


static void mem_config_check_routes(void)
{
	struct mod_t *mod;
	struct net_routing_table_entry_t *entry;
	int i;

	/* For each module, check accessibility to low/high modules */
	mem_debug("Checking accessibility to low and high modules:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		struct mod_t *low_mod;

		/* Get module */
		mod = list_get(mem_system->mod_list, i);
		mem_debug("\t%s\n", mod->name);

		/* List of low modules */
		mem_debug("\t\tLow modules:");
		for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
			linked_list_next(mod->low_mod_list))
		{
			/* Get low module */
			low_mod = linked_list_get(mod->low_mod_list);
			mem_debug(" %s", low_mod->name);

			/* Check that nodes are in the same network */
			if (mod->low_net != low_mod->high_net)
				fatal("%s: %s: low node '%s' is not in the same network.\n%s",
					mem_config_file_name, mod->name, low_mod->name,
					err_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->low_net->routing_table,
				mod->low_net_node, low_mod->high_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					mem_config_file_name, mod->low_net->name,
					mod->name, low_mod->name, err_mem_connect);
		}

		/* List of high modules */
		mem_debug("\n\t\tHigh modules:");
		for (linked_list_head(mod->high_mod_list); !linked_list_is_end(mod->high_mod_list);
			linked_list_next(mod->high_mod_list))
		{
			struct mod_t *high_mod;

			/* Get high module */
			high_mod = linked_list_get(mod->high_mod_list);
			mem_debug(" %s", high_mod->name);

			/* Check that nodes are in the same network */
			if (mod->high_net != high_mod->low_net)
				fatal("%s: %s: high node '%s' is not in the same network.\n%s",
					mem_config_file_name, mod->name, high_mod->name,
					err_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->high_net->routing_table,
				mod->high_net_node, high_mod->low_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					mem_config_file_name, mod->high_net->name,
					mod->name, high_mod->name, err_mem_connect);
		}

		/* Debug */
		mem_debug("\n");
	}

	/* Debug */
	mem_debug("\n");
}


/* Set a color for a module and all its lower-level modules */
static void mem_config_set_mod_color(struct mod_t *mod, int color)
{
	struct mod_t *low_mod;

	/* Already set */
	if (mod->color == color)
		return;

	/* Set color */
	mod->color = color;
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		mem_config_set_mod_color(low_mod, color);
	}
}


/* Check if a module or any of its lower-level modules has a color */
static int mem_config_check_mod_color(struct mod_t *mod, int color)
{
	struct mod_t *low_mod;

	/* This module has the color */
	if (mod->color == color)
		return 1;

	/* Check lower-level modules */
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		if (mem_config_check_mod_color(low_mod, color))
			return 1;
	}

	/* Not found */
	return 0;
}


static void mem_config_check_disjoint(void)
{
	int compute_unit_id;
	int core;
	int thread;

	/* No need if no CPU simulation */
	if (x86_emu_sim_kind == arch_sim_kind_functional)
		return;
	
	/* No need if no GPU simulation */
	switch (x86_emu->gpu_kind)
	{
	case x86_emu_gpu_southern_islands:

		if (si_emu_sim_kind == arch_sim_kind_functional) 
			return;
		break;
	
	case x86_emu_gpu_evergreen:
	
		if (evg_emu_sim_kind == arch_sim_kind_functional)
			return;
		break;
	
	default:
		panic("%s: invalid GPU emulator", __FUNCTION__);
	}

	/* Color CPU modules */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		mem_config_set_mod_color(X86_THREAD.data_mod, 1);
		mem_config_set_mod_color(X86_THREAD.inst_mod, 1);
	}

	/* Check color of GPU modules */
	switch (x86_emu->gpu_kind)
	{

	case x86_emu_gpu_southern_islands:

		SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
		{
			if (mem_config_check_mod_color(
				si_gpu->compute_units[compute_unit_id]->global_memory, 1))
				fatal("%s: non-disjoint CPU/GPU memory hierarchies",
					mem_config_file_name);
		}
		break;
	
	case x86_emu_gpu_evergreen:

		EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
		{
			if (mem_config_check_mod_color(
				evg_gpu->compute_units[compute_unit_id]->global_memory, 1))
				fatal("%s: non-disjoint CPU/GPU memory hierarchies",
					mem_config_file_name);
		}
		break;
	
	default:
		panic("%s: invalid GPU emulation", __FUNCTION__);
	}
}


static void mem_config_calculate_sub_block_sizes(void)
{
	struct mod_t *mod;
	struct mod_t *high_mod;

	int num_nodes;
	int i;

	mem_debug("Creating directories:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get module */
		mod = list_get(mem_system->mod_list, i);

		/* Calculate sub-block size */
		mod->sub_block_size = mod->block_size;
		for (linked_list_head(mod->high_mod_list); !linked_list_is_end(mod->high_mod_list);
			linked_list_next(mod->high_mod_list))
		{
			high_mod = linked_list_get(mod->high_mod_list);
			mod->sub_block_size = MIN(mod->sub_block_size, high_mod->block_size);
		}

		/* Get number of nodes for directory */
		if (mod->high_net && list_count(mod->high_net->node_list))
			num_nodes = list_count(mod->high_net->node_list);
		else
			num_nodes = 1;

		/* Create directory */
		mod->num_sub_blocks = mod->block_size / mod->sub_block_size;
		mod->dir = dir_create(mod->name, mod->dir_num_sets, mod->dir_assoc, mod->num_sub_blocks, num_nodes);
		mem_debug("\t%s - %dx%dx%d (%dx%dx%d effective) - %d entries, %d sub-blocks\n",
			mod->name, mod->dir_num_sets, mod->dir_assoc, num_nodes,
			mod->dir_num_sets, mod->dir_assoc, linked_list_count(mod->high_mod_list),
			mod->dir_size, mod->num_sub_blocks);
	}

	/* Debug */
	mem_debug("\n");
}


static void mem_config_set_mod_level(struct mod_t *mod, int level)
{
	struct mod_t *low_mod;

	/* If level is already set, do nothing */
	if (mod->level >= level)
		return;

	/* Set level of module and lower modules */
	mod->level = level;
	LINKED_LIST_FOR_EACH(mod->low_mod_list)
	{
		low_mod = linked_list_get(mod->low_mod_list);
		mem_config_set_mod_level(low_mod, level + 1);
	}
}


static void mem_config_calculate_mod_levels(void)
{
	int compute_unit_id;
	int core;
	int thread;
	int i;

	struct mod_t *mod;

	/* Color CPU modules */
	if (x86_emu_sim_kind == arch_sim_kind_detailed)
	{
		X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		{
			mem_config_set_mod_level(X86_THREAD.data_mod, 1);
			mem_config_set_mod_level(X86_THREAD.inst_mod, 1);
		}
	}

	/* Check color of GPU modules */
	switch (x86_emu->gpu_kind)
	{
	
	case x86_emu_gpu_southern_islands:

		if (si_emu_sim_kind == arch_sim_kind_detailed)
		{
			SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
				mem_config_set_mod_level(
					si_gpu->compute_units[compute_unit_id]->global_memory, 1);
		}
		break;
	
	case x86_emu_gpu_evergreen:

		if (evg_emu_sim_kind == arch_sim_kind_detailed)
		{
			EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
				mem_config_set_mod_level(
					evg_gpu->compute_units[compute_unit_id]->global_memory, 1);
		}
		break;
	
	default:
		panic("%s: invalid GPU emulation", __FUNCTION__);
	}

	/* Debug */
	mem_debug("Calculating module levels:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		mem_debug("\t%s -> ", mod->name);
		if (mod->level)
			mem_debug("level %d\n", mod->level);
		else
			mem_debug("not accessible\n");
	}
	mem_debug("\n");
}


/* Version of memory system trace producer.
 * See 'src/visual/memory/mem-system.c' for the trace consumer. */

#define MEM_SYSTEM_TRACE_VERSION_MAJOR		1
#define MEM_SYSTEM_TRACE_VERSION_MINOR		678

static void mem_config_trace(void)
{
	struct net_t *net;
	int i;

	/* No need if not tracing */
	if (!mem_tracing())
		return;

	/* Initialization */
	mem_trace_header("mem.init version=\"%d.%d\"\n",
		MEM_SYSTEM_TRACE_VERSION_MAJOR, MEM_SYSTEM_TRACE_VERSION_MINOR);

	/* Internal networks */
	LIST_FOR_EACH(mem_system->net_list, i)
	{
		net = list_get(mem_system->net_list, i);
		mem_trace_header("mem.new_net name=\"%s\" num_nodes=%d\n",
			net->name, net->node_list->count);
	}

	/* External networks */
	for (net = net_find_first(); net; net = net_find_next())
	{
		mem_trace_header("mem.new_net name=\"%s\" num_nodes=%d\n",
			net->name, net->node_list->count);
	}

	/* Modules */
	LIST_FOR_EACH(mem_system->mod_list, i)
	{
		struct mod_t *mod;

		char *high_net_name;
		char *low_net_name;

		int high_net_node_index;
		int low_net_node_index;

		/* Get module */
		mod = list_get(mem_system->mod_list, i);

		/* If module is unreachable, ignore it */
		if (!mod->level)
			continue;

		/* High network */
		high_net_name = mod->high_net ? mod->high_net->name : "";
		high_net_node_index = mod->high_net_node ? mod->high_net_node->index : 0;

		/* Low network */
		low_net_name = mod->low_net ? mod->low_net->name : "";
		low_net_node_index = mod->low_net_node ? mod->low_net_node->index : 0;

		/* Trace header */
		mem_trace_header("mem.new_mod name=\"%s\" num_sets=%d assoc=%d "
			"block_size=%d sub_block_size=%d num_sharers=%d level=%d "
			"high_net=\"%s\" high_net_node=%d low_net=\"%s\" low_net_node=%d\n",
			mod->name, mod->cache->num_sets, mod->cache->assoc,
			mod->cache->block_size, mod->sub_block_size, mod->dir->num_nodes,
			mod->level, high_net_name, high_net_node_index,
			low_net_name, low_net_node_index);
	}
}


static void mem_config_read_commands(struct config_t *config)
{
	char *section = "Commands";
	char *command_line;
	char command_var[MAX_STRING_SIZE];

	int command_var_id;

	/* Check if section is present */
	if (!config_section_exists(config, section))
		return;

	/* Read commands */
	command_var_id = 0;
	while (1)
	{
		/* Get command */
		snprintf(command_var, sizeof command_var, "Command[%d]", command_var_id);
		command_line = config_read_string(config, section, command_var, NULL);
		if (!command_line)
			break;

		/* Schedule event to process command */
		command_line = xstrdup(command_line);
		esim_schedule_event(EV_MEM_SYSTEM_COMMAND, command_line, 1);

		/* Next command */
		command_var_id++;
	}
}




/*
 * Public Functions
 */

void mem_config_read(void)
{
	struct config_t *config;

	/* Load memory system configuration file. If no file name has been given
	 * by the user, create a default configuration for each architecture. */
	config = config_create(mem_config_file_name);
	if (!*mem_config_file_name)
		mem_config_default(config);
	else
		config_load(config);

	/* Read general variables */
	mem_config_read_general(config);

	/* Read networks */
	mem_config_read_networks(config);

	/* Read modules */
	mem_config_read_modules(config);

	/* Read low level caches */
	mem_config_read_low_modules(config);

	/* Read entries from requesting devices (CPUs/GPUs) to memory system entries.
	 * This is presented in [Entry <name>] sections in the configuration file. */
	mem_config_read_entries(config);

	/* Create switches in internal networks */
	mem_config_create_switches(config);

	/* Read commands from the configuration file. Commands are used to artificially
	 * alter the initial state of the memory hierarchy for debugging purposes. */
	mem_config_read_commands(config);

	/* Check that all enforced sections and variables were specified */
	config_check(config);
	config_free(config);

	/* Check routes to low and high modules */
	mem_config_check_routes();

	/* Check for disjoint CPU/GPU memory hierarchies */
	mem_config_check_disjoint();

	/* Compute sub-block sizes, based on high modules. This function also
	 * initializes the directories in modules other than L1. */
	mem_config_calculate_sub_block_sizes();

	/* Compute cache levels relative to the CPU/GPU entry points */
	mem_config_calculate_mod_levels();

	/* Dump configuration to trace file */
	mem_config_trace();
}

