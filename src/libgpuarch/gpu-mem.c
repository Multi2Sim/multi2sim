/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gpuarch.h>
#include <network.h>
#include <esim.h>
#include <cachesystem.h>


/*
 * Global variables
 */

int gpu_mem_debug_category;

char *gpu_mem_report_file_name = "";
char *gpu_mem_config_file_name = "";

char *gpu_mem_config_help = "FIXME\n";

#if 0
	"The GPU memory hierarchy can be configured by using an IniFile formatted file,\n"
	"describing the cache and interconnect properties. This file is passed to\n"
	"Multi2Sim with option '--gpu-cache-config <file>', and should be always used\n"
	"together with option '--gpu-sim detailed' for an architectural GPU simulation.\n"
	"\n"
	"The sections and variables allowed in the cache configuration file are the\n"
	"following:\n"
	"\n"
	"Section '[ CacheGeometry <name> ]': defines a geometry for a cache. Caches\n"
	"using this geometry can be instantiated later.\n"
	"\n"
	"  Sets = <num_sets> (Required)\n"
	"      Number of sets in the cache.\n"
	"  Assoc = <num_ways> (Required)\n"
	"      Cache associativity. The total number of blocks contained in the cache\n"
	"      is given by the product Sets * Assoc.\n"
	"  BlockSize = <size> (Required)\n"
	"      Size of a cache block in bytes. The total size of the cache is given by\n"
	"      the product Sets * Assoc * BlockSize.\n"
	"  Latency = <num_cycles> (Required)\n"
	"      Hit latency for a cache in number of CPU cycles.\n"
	"  Policy = {LRU|FIFO|Random} (Default = LRU)\n"
	"      Block replacement policy.\n"
	"  Banks = <num> (Default = 1)\n"
	"      Number of banks.\n"
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports per bank.\n"
	"  WritePorts = <num> (Default = 2)\n"
	"      Number of write ports per bank.\n"
	"\n"
	"Section '[ Net <name> ]': defines an interconnection network.\n"
	"\n"
	"  UpperLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for bidirectional links connecting upper\n"
	"      level caches with upper input/output buffers of a switch.\n"
	"  LowerLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for the bidirectional link connecting\n"
	"      the switch with the lower-level cache or global memory.\n"
	"  InnerLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for the links connecting input/output\n"
	"      buffers of a switch with the internal crossbar.\n"
	"  InputBufferSize = <bytes> (Default = (B + 8) * 2)\n"
	"      Size of the switch input buffers. This size must be at least B + 8,\n"
	"      being B the block size of the lower level cache (or memory), and 8\n"
	"      the metadata size attached to block transfers. By default, the\n"
	"      input buffer size can hold two maximum-length packages.\n"
	"  OutputBufferSize = <bytes> (Default = (B + 8) * 2)\n"
	"      Size of the switch output buffers. It must be at least the maximum\n"
	"      package size, and it defaults to twice this size.\n"
	"\n"
	"Section '[ Cache <name> ]': instantiates a cache based on a cache geometry\n"
	"defined in a section CacheGeometry.\n"
	"\n"
	"  Geometry = <geometry_name> (Required)\n"
	"      Cache geometry identifier, as specified in a previous section of type\n"
	"      '[ CacheGeometry <geometry_name> ]'.\n"
	"  HiNet = <net_name> (Required only for non-L1 caches)\n"
	"      Upper interconnect where the cache is connected to. <net_name> must\n"
	"      correspond to a network declared in a previous section of type\n"
	"      '[ Net <net_name> ]'.\n"
	"  LoNet = <net_name> (Required)\n"
	"      Lower interconnect where the cache is connected to.\n"
	"\n"
	"Section '[ GlobalMemory ]'\n"
	"\n"
	"  HiNet = <net_name> (Required if there are caches)\n"
	"      Upper interconnect where global memory is connected to.\n"
	"  BlockSize = <size> (Required)\n"
	"      Memory block size in bytes.\n"
	"  Latency = <num_cycles> (Required)\n"
	"      Main memory access latency.\n"
	"  Banks = <num> (Default = 8)\n"
	"      Number of banks.\n"
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports per bank.\n"
	"  WritePorts = <num> (Default = 2)\n"
	"      Number of write ports per bank.\n"
	"\n"
	"Section '[ Node <name> ]': defines an entry to the memory hierarchy from a\n"
	"compute unit in the GPU into an L1 cache or global memory.\n"
	"\n"
	"  ComputeUnit = <id>\n"
	"      Identifier of a compute unit. This must be an integer between 0 and the\n"
	"      number of compute units minus 1.\n"
	"  DataCache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the compute unit reads/writes data.\n"
	"      The cache must be declared in a previous section of type\n"
	"      '[ Cache <cache_name> ]'.\n"
	"\n";
#endif


/*
 * Private functions
 */

#define GPU_MEM_MAX_LEVELS  10

static char *err_gpu_mem_config_note =
	"\tPlease run 'm2s --help-gpu-cache-config' or consult the Multi2Sim Guide for\n"
	"\ta description of the GPU cache configuration file format.\n";

static char *err_gpu_mem_config_net =
	"\tNetwork identifiers need to be declared either in the cache configuration\n"
	"\tfile, or in the network configuration file (option '--net-config').\n";

static char *err_gpu_mem_levels =
	"\tThe path from a cache into main memory exceeds 10 levels of cache.\n"
	"\tThis might be a symptom of a recursive reference in 'LowCaches'\n"
	"\tlists. If you really intend to have a high number of cache levels,\n"
	"\tincrease variable GPU_MEM_MAX_LEVELS in '" __FILE__ "'\n";

static char *err_gpu_mem_block_size =
	"\tBlock size in a cache must be greater or equal than its lower-level\n"
	"\tcache for correct behavior of directories and coherence protocols.\n";

static char *err_gpu_mem_ignored_node =
	"\tThis entry in the file will be ignored, because the value for variable\n"
	"\t'ComputeUnit' refers to a non-existent compute unit.\n";

static char *err_gpu_mem_connect =
	"\tAn external network is used that does not provide connectivity between\n"
	"\ta memory module and an associated low/high module. Please add the\n"
	"\tnecessary links in the network configuration file.\n";

static void gpu_mem_config_default(struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int compute_unit_id;

	/* Cache geometry for L1 */
	strcpy(section, "CacheGeometry geo-l1");
	config_write_int(config, section, "Sets", 16);
	config_write_int(config, section, "Assoc", 2);
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 1);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Cache geometry for L2 */
	strcpy(section, "CacheGeometry geo-l2");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 4);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* L1 caches and nodes */
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		snprintf(section, sizeof section, "Module l1-%d", compute_unit_id);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "geo-l1");
		config_write_string(config, section, "LowNetwork", "net-l1-l2");
		config_write_string(config, section, "LowCache", "l2");

		snprintf(section, sizeof section, "Node cu-%d", compute_unit_id);
		snprintf(str, sizeof str, "l1-%d", compute_unit_id);
		config_write_int(config, section, "ComputeUnit", compute_unit_id);
		config_write_string(config, section, "DataCache", str);
	}

	/* L2 cache */
	snprintf(section, sizeof section, "Module l2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "geo-l2");
	config_write_string(config, section, "HighNetwork", "net-l1-l2");
	config_write_string(config, section, "LowNetwork", "net-l2-gm");
	config_write_string(config, section, "LowCache", "GlobalMemory");

	/* Global memory */
	snprintf(section, sizeof section, "Module GlobalMemory");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "net-l2-gm");
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 100);
	config_write_int(config, section, "Banks", 8);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Network connecting L1 caches and L2 */
	snprintf(section, sizeof section, "Network net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	/* Network connecting L2 cache and global memory */
	snprintf(section, sizeof section, "Network net-l2-gm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


static void gpu_mem_config_read_networks(struct config_t *config)
{
	struct net_t *net;
	int i;

	char buf[MAX_STRING_SIZE];
	char *section;

	/* Create networks */
	gpu_mem_debug("Creating internal networks:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *net_name;

		/* Network section */
		if (strncasecmp(section, "Network ", 8))
			continue;
		net_name = section + 8;
		
		/* Create network */
		net = net_create(net_name);
		gpu_mem_debug("\t%s\n", net_name);
		list_add(gpu->net_list, net);
	}
	gpu_mem_debug("\n");

	/* Add network pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (i = 0; i < list_count(gpu->net_list); i++)
	{
		net = list_get(gpu->net_list, i);
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", net);
	}

}


static void gpu_mem_config_insert_module_in_network(struct config_t *config,
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
			gpu_mem_config_file_name, mod->name,
			err_gpu_mem_config_note);
	
	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network '%s' already contains module '%s'.\n%s",
			gpu_mem_config_file_name, net->name,
			mod->name, err_gpu_mem_config_note);
	
	/* Read buffer sizes from network */
	def_input_buffer_size = config_read_int(config, buf, "DefaultInputBufferSize", 0);
	def_output_buffer_size = config_read_int(config, buf, "DefaultOutputBufferSize", 0);
	if (!def_input_buffer_size)
		fatal("%s: network %s: variable 'DefaultInputBufferSize' missing.\n%s",
			gpu_mem_config_file_name, net->name, err_gpu_mem_config_note);
	if (!def_output_buffer_size)
		fatal("%s: network %s: variable 'DefaultOutputBufferSize' missing.\n%s",
			gpu_mem_config_file_name, net->name, err_gpu_mem_config_note);
	if (def_input_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum input buffer size is %d for cache '%s'.\n%s",
			gpu_mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_gpu_mem_config_note);
	if (def_output_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum output buffer size is %d for cache '%s'.\n%s",
			gpu_mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_gpu_mem_config_note);
	
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
			gpu_mem_config_file_name, net_name,
			err_gpu_mem_config_note, err_gpu_mem_config_net);
	
	/* Node name must be specified */
	if (!*net_node_name)
		fatal("%s: %s: network node name required for external network.\n%s%s",
			gpu_mem_config_file_name, mod->name,
			err_gpu_mem_config_note, err_gpu_mem_config_net);
	
	/* Get node */
	node = net_get_node_by_name(net, net_node_name);
	if (!node)
		fatal("%s: network %s: node %s: invalid node name.\n%s%s",
			gpu_mem_config_file_name, net_name, net_node_name,
			err_gpu_mem_config_note, err_gpu_mem_config_net);
	
	/* No module must have been assigned previously to this node */
	if (node->user_data)
		fatal("%s: network %s: node '%s' already assigned.\n%s",
			gpu_mem_config_file_name, net->name,
			net_node_name, err_gpu_mem_config_note);
	
	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network %s: module '%s' is already present.\n%s",
			gpu_mem_config_file_name, net->name,
			mod->name, err_gpu_mem_config_note);
	
	/* Assign module to network node and return */
	node->user_data = mod;
	*net_ptr = net;
	*net_node_ptr = node;
}


static void gpu_mem_config_read_cache(struct config_t *config, char *section)
{
	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];

	int sets;
	int assoc;
	int block_size;
	int latency;

	char *policy_str;
	enum cache_policy_t policy;

	int bank_count;
	int read_port_count;
	int write_port_count;

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
	sets = config_read_int(config, buf, "Sets", 16);
	assoc = config_read_int(config, buf, "Assoc", 2);
	block_size = config_read_int(config, buf, "BlockSize", 256);
	latency = config_read_int(config, buf, "Latency", 1);
	policy_str = config_read_string(config, buf, "Policy", "LRU");
	bank_count = config_read_int(config, buf, "Banks", 1);
	read_port_count = config_read_int(config, buf, "ReadPorts", 2);
	write_port_count = config_read_int(config, buf, "WritePorts", 1);

	/* Checks */
	policy = map_string_case(&cache_policy_map, policy_str);
	if (policy == cache_policy_invalid)
		fatal("%s: cache '%s': %s: invalid block replacement policy.\n%s",
			gpu_mem_config_file_name, mod_name,
			policy_str, err_gpu_mem_config_note);
	if (sets < 1 || (sets & (sets - 1)))
		fatal("%s: cache '%s': number of sets must be a power of two greater than 1.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (assoc < 1 || (assoc & (assoc - 1)))
		fatal("%s: cache '%s': associativity must be power of two and > 1.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (block_size < 4 || (block_size & (block_size - 1)))
		fatal("%s: cache '%s': block size must be power of two and at least 4.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (latency < 1)
		fatal("%s: cache '%s': invalid value for variable 'Latency'.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (bank_count < 1 || (bank_count & (bank_count - 1)))
		fatal("%s: cache '%s': number of banks must be a power of two greater than 1.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (read_port_count < 1)
		fatal("%s: cache '%s': invalid value for variable 'ReadPorts'.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);
	if (write_port_count < 1)
		fatal("%s: cache '%s': invalid value for variable 'WritePorts'.\n%s",
			gpu_mem_config_file_name, mod_name, err_gpu_mem_config_note);

	/* Create cache */
	mod = mod_create(mod_name, mod_kind_cache,
		bank_count, read_port_count, write_port_count,
		block_size, latency);
	list_add(gpu->mod_list, mod);

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	gpu_mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;

	/* Low network */
	net_name = config_read_string(config, section, "LowNetwork", "");
	net_node_name = config_read_string(config, section, "LowNetworkNode", "");
	gpu_mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->low_net = net;
	mod->low_net_node = net_node;

	/* Create cache */
	mod->cache = cache_create(sets, block_size, assoc, policy);
}


static void gpu_mem_config_read_main_memory(struct config_t *config, char *section)
{
	char mod_name[MAX_STRING_SIZE];

	int block_size;
	int latency;
	int bank_count;
	int read_port_count;
	int write_port_count;

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
	bank_count = config_read_int(config, section, "Banks", 4);
	read_port_count = config_read_int(config, section, "ReadPorts", 2);
	write_port_count = config_read_int(config, section, "WritePorts", 2);

	/* Check parameters */
	if (block_size < 1 || (block_size & (block_size - 1)))
		fatal("%s: global memory: block size must be power of two and > 1.\n%s",
			gpu_mem_config_file_name, err_gpu_mem_config_note);
	if (latency < 1)
		fatal("%s: global memory: invalid value for variable 'Latency'.\n%s",
			gpu_mem_config_file_name, err_gpu_mem_config_note);
	if (bank_count < 1 || (bank_count & (bank_count - 1)))
		fatal("%s: global_memory: number of banks must be a power of two greater than 1.\n%s",
			gpu_mem_config_file_name, err_gpu_mem_config_note);
	if (read_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'ReadPorts'.\n%s",
			gpu_mem_config_file_name, err_gpu_mem_config_note);
	if (write_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'WritePorts'.\n%s",
			gpu_mem_config_file_name, err_gpu_mem_config_note);

	/* Create module */
	mod = mod_create(mod_name, mod_kind_main_memory,
			bank_count, read_port_count, write_port_count,
			block_size, latency);
	gpu->global_memory = mod;
	list_add(gpu->mod_list, mod);

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	gpu_mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
			&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;
}


static void gpu_mem_config_read_modules(struct config_t *config)
{
	struct mod_t *mod;

	char *section;
	char *mod_type;

	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];
	int i;

	/* Create modules */
	gpu_mem_debug("Creating modules:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section for a module */
		if (strncasecmp(section, "Module ", 7))
			continue;

		/* Ignore if module is not a cache */
		str_token(mod_name, sizeof mod_name, section, 1, " ");
		mod_type = config_read_string(config, section, "Type", "");
		if (!strcasecmp(mod_type, "Cache"))
			gpu_mem_config_read_cache(config, section);
		else if (!strcasecmp(mod_type, "MainMemory"))
			gpu_mem_config_read_main_memory(config, section);
		else
			fatal("%s: %s: invalid or missing value for 'Type'.\n%s",
				gpu_mem_config_file_name, mod_name,
				err_gpu_mem_config_note);

		/* Debug */
		gpu_mem_debug("\t%s\n", mod_name);
	}

	/* Debug */
	gpu_mem_debug("\n");

	/* Add module pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (i = 0; i < list_count(gpu->mod_list); i++)
	{
		mod = list_get(gpu->mod_list, i);
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", mod);
	}
}


static void gpu_mem_config_check_route_to_main_memory(struct mod_t *mod, int block_size, int level)
{
	struct mod_t *low_mod;
	int i;

	/* Maximum level */
	if (level > GPU_MEM_MAX_LEVELS)
		fatal("%s: %s: too many cache levels.\n%s%s",
			gpu_mem_config_file_name, mod->name,
			err_gpu_mem_levels, err_gpu_mem_config_note);

	/* Check block size */
	if (mod->block_size < block_size)
		fatal("%s: %s: decreasing block size.\n%s%s",
			gpu_mem_config_file_name, mod->name,
			err_gpu_mem_block_size, err_gpu_mem_config_note);
	block_size = mod->block_size;

	/* Dump current module */
	gpu_mem_debug("\t");
	for (i = 0; i < level * 2; i++)
		gpu_mem_debug(" ");
	gpu_mem_debug("%s\n", mod->name);

	/* Check that cache has a way to main memory */
	if (!linked_list_count(mod->low_mod_list) && mod->kind == mod_kind_cache)
		fatal("%s: %s: main memory not accessible from cache.\n%s",
			gpu_mem_config_file_name, mod->name,
			err_gpu_mem_config_note);

	/* Dump children */
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		gpu_mem_config_check_route_to_main_memory(low_mod, block_size, level + 1);
	}
}

static void gpu_mem_config_read_low_modules(struct config_t *config)
{
	char buf[MAX_STRING_SIZE];
	char *delim;

	char *low_mod_name;
	char *low_mod_name_list;

	struct mod_t *mod;
	struct mod_t *low_mod;

	int i;

	/* Lower level modules */
	for (i = 0; i < list_count(gpu->mod_list); i++)
	{
		/* Get cache module */
		mod = list_get(gpu->mod_list, i);
		if (mod->kind != mod_kind_cache)
			continue;

		/* Section name */
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));

		/* Low module name list */
		low_mod_name_list = config_read_string(config, buf, "LowCaches", "");
		if (!*low_mod_name_list)
			continue;

		/* Create copy of low module name list */
		low_mod_name_list = strdup(low_mod_name_list);
		if (!low_mod_name_list)
			fatal("%s: out of memory", __FUNCTION__);

		/* For each element in the list */
		delim = ", ";
		for (low_mod_name = strtok(low_mod_name_list, delim);
			low_mod_name; low_mod_name = strtok(NULL, delim))
		{
			/* Check valid module name */
			snprintf(buf, sizeof buf, "Module %s", low_mod_name);
			if (!config_section_exists(config, buf))
				fatal("%s: %s: invalid module name in 'LowCaches'.\n%s",
					gpu_mem_config_file_name, mod->name,
					err_gpu_mem_config_note);

			/* Get low cache and assign */
			low_mod = config_read_ptr(config, buf, "ptr", NULL);
			assert(low_mod);
			linked_list_add(mod->low_mod_list, low_mod);
			linked_list_add(low_mod->high_mod_list, mod);
		}

		/* Free copy of low module name list */
		free(low_mod_name_list);
	}

	/* Check paths to global memory */
	gpu_mem_debug("Creating paths to global memory:\n");
	for (i = 0; i < list_count(gpu->mod_list); i++)
	{
		mod = list_get(gpu->mod_list, i);
		gpu_mem_config_check_route_to_main_memory(mod, mod->block_size, 1);
	}
	gpu_mem_debug("\n");
}


static void gpu_mem_config_read_nodes(struct config_t *config)
{
	char *section;
	int compute_unit_id;
	struct gpu_compute_unit_t *compute_unit;

	char buf[MAX_STRING_SIZE];
	char *node_name;
	char *value;

	/* Nodes */
	gpu_mem_debug("Creating access points to memory hierarchy:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section is a node */
		if (strncasecmp(section, "Node ", 5))
			continue;
		node_name = section + 5;

		/* Get compute unit */
		if (!config_var_exists(config, section, "ComputeUnit"))
			fatal("%s: node %s: variable 'ComputeUnit' not specified.\n%s",
				gpu_mem_config_file_name, node_name, err_gpu_mem_config_note);
		compute_unit_id = config_read_int(config, section, "ComputeUnit", 0);
		if (compute_unit_id < 0)
			fatal("%s: node %s: invalid value for variable 'ComputeUnit'.\n%s",
				gpu_mem_config_file_name, node_name, err_gpu_mem_config_note);
		if (compute_unit_id >= gpu_num_compute_units)
		{
			warning("%s: node %s: section ignored.\n%s",
				gpu_mem_config_file_name, node_name, err_gpu_mem_ignored_node);
			config_var_allow(config, section, "DataCache");
			continue;
		}
		compute_unit = gpu->compute_units[compute_unit_id];

		/* Entry module for node */
		value = config_read_string(config, section, "DataCache", "");
		if (!*value)
			fatal("%s: node '%s': variable 'DataCache' not specified.\n%s",
				gpu_mem_config_file_name, node_name, err_gpu_mem_config_note);

		/* Get module */
		snprintf(buf, sizeof buf, "Module %s", value);
		if (!config_section_exists(config, buf))
			fatal("%s: node '%s': invalid cache name for variable 'DataCache'.\n%s",
				gpu_mem_config_file_name, node_name, err_gpu_mem_config_note);

		/* Assign entry */
		compute_unit->data_cache = config_read_ptr(config, buf, "ptr", NULL);
		assert(compute_unit->data_cache);
		gpu_mem_debug("\tcu[%d] -> %s\n", compute_unit_id,
			compute_unit->data_cache->name);
	}
	gpu_mem_debug("\n");

	/* Check that all compute units have an entry to the global memory hierarchy */
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = gpu->compute_units[compute_unit_id];
		if (!compute_unit->data_cache)
			fatal("%s: missing entry point for compute unit %d.\n%s",
				gpu_mem_config_file_name, compute_unit_id,
				err_gpu_mem_config_note);
	}
}


static void gpu_mem_config_create_switches(struct config_t *config)
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
	gpu_mem_debug("Creating network switches and links for internal networks:\n");
	for (i = 0; i < list_count(gpu->net_list); i++)
	{
		/* Get network and lower level cache */
		net = list_get(gpu->net_list, i);

		/* Get switch bandwidth */
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));
		def_bandwidth = config_read_int(config, buf, "DefaultBandwidth", 0);
		if (def_bandwidth < 1)
			fatal("%s: %s: invalid or missing value for 'DefaultBandwidth'.\n%s",
				gpu_mem_config_file_name, net->name, err_gpu_mem_config_note);

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
		gpu_mem_debug("\t%s.Switch ->", net->name);

		/* Create connections between switch and every end node */
		for (j = 0; j < list_count(net->node_list); j++)
		{
			net_node = list_get(net->node_list, j);
			if (net_node != net_switch)
			{
				net_add_bidirectional_link(net, net_node, net_switch,
					def_bandwidth);
				gpu_mem_debug(" %s", net_node->name);
			}
		}

		/* Calculate routes */
		net_routing_table_calculate(net->routing_table);

		/* Debug */
		gpu_mem_debug("\n");
	}
	gpu_mem_debug("\n");

}


void gpu_mem_config_check_routes(void)
{
	struct mod_t *mod;
	struct net_routing_table_entry_t *entry;
	int i;

	/* For each module, check accessibility to low/high modules */
	gpu_mem_debug("Checking accessibility to low and high modules:\n");
	for (i = 0; i < list_count(gpu->mod_list); i++)
	{
		struct mod_t *low_mod;

		/* Get module */
		mod = list_get(gpu->mod_list, i);
		gpu_mem_debug("\t%s\n", mod->name);

		/* List of low modules */
		gpu_mem_debug("\t\tLow modules:");
		for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
			linked_list_next(mod->low_mod_list))
		{
			/* Get low module */
			low_mod = linked_list_get(mod->low_mod_list);
			gpu_mem_debug(" %s", low_mod->name);
			
			/* Check that nodes are in the same network */
			if (mod->low_net != low_mod->high_net)
				fatal("%s: %s: low node '%s' is not in the same network.\n%s",
					gpu_mem_config_file_name, mod->name, low_mod->name,
					err_gpu_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->low_net->routing_table,
				mod->low_net_node, low_mod->high_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					gpu_mem_config_file_name, mod->low_net->name,
					mod->name, low_mod->name, err_gpu_mem_connect);
		}

		/* List of high modules */
		gpu_mem_debug("\n\t\tHigh modules:");
		for (linked_list_head(mod->high_mod_list); !linked_list_is_end(mod->high_mod_list);
			linked_list_next(mod->high_mod_list))
		{
			struct mod_t *high_mod;

			/* Get high module */
			high_mod = linked_list_get(mod->high_mod_list);
			gpu_mem_debug(" %s", high_mod->name);
			
			/* Check that nodes are in the same network */
			if (mod->high_net != high_mod->low_net)
				fatal("%s: %s: high node '%s' is not in the same network.\n%s",
					gpu_mem_config_file_name, mod->name, high_mod->name,
					err_gpu_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->high_net->routing_table,
				mod->high_net_node, high_mod->low_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					gpu_mem_config_file_name, mod->high_net->name,
					mod->name, high_mod->name, err_gpu_mem_connect);
		}

		/* Debug */
		gpu_mem_debug("\n");
	}

	/* Debug */
	gpu_mem_debug("\n");
}




/*
 * Public Functions
 */

void gpu_mem_init(void)
{
	/* Try to open report file */
	if (gpu_mem_report_file_name[0] && !can_open_write(gpu_mem_report_file_name))
		fatal("%s: cannot open GPU cache report file",
			gpu_mem_report_file_name);

	/* Create network and module list */
	gpu->net_list = list_create();
	gpu->mod_list = list_create();

	/* Events */
	EV_GPU_MEM_READ = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_REQUEST = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_REQUEST_RECEIVE = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_REQUEST_REPLY = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_REQUEST_FINISH = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_UNLOCK = esim_register_event(mod_handler_read);
	EV_GPU_MEM_READ_FINISH = esim_register_event(mod_handler_read);

	EV_GPU_MEM_WRITE = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_REQUEST_SEND = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_REQUEST_RECEIVE = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_REQUEST_REPLY = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_REQUEST_REPLY_RECEIVE = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_UNLOCK = esim_register_event(mod_handler_write);
	EV_GPU_MEM_WRITE_FINISH = esim_register_event(mod_handler_write);

	/* Read cache configuration file */
	gpu_mem_config_read();
}


void gpu_mem_done(void)
{
	int i;
	
	/* Dump report */
	gpu_mem_dump_report();

	/* Free memory modules */
	for (i = 0; i < list_count(gpu->mod_list); i++)
		mod_free(list_get(gpu->mod_list, i));
	list_free(gpu->mod_list);

	/* Free networks */
	for (i = 0; i < list_count(gpu->net_list); i++)
		net_free(list_get(gpu->net_list, i));
	list_free(gpu->net_list);
}


void gpu_mem_dump_report(void)
{
	FILE *f;
	int i;

	struct mod_t *mod;
	struct cache_t *cache;

	/* Open file */
	f = open_write(gpu_mem_report_file_name);
	if (!f)
		return;

	/* Intro */
	fprintf(f, "; Report for the GPU global memory hierarchy.\n");
	fprintf(f, ";    Accesses - Total number of accesses requested from a compute unit or upper-level cache\n");
	fprintf(f, ";    Reads - Number of read requests received from a compute unit or upper-level cache\n");
	fprintf(f, ";    Writes - Number of write requests received from a compute unit or upper-level cache\n");
	fprintf(f, ";    CoalescedReads - Number of reads that were coalesced with previous accesses (discarded)\n");
	fprintf(f, ";    CoalescedWrites - Number of writes coalesced with previous accesses\n");
	fprintf(f, ";    EffectiveReads - Number of reads actually performed (= Reads - CoalescedReads)\n");
	fprintf(f, ";    EffectiveReadHits - Number of effective reads producing cache hit\n");
	fprintf(f, ";    EffectiveReadMisses - Number of effective reads missing in the cache\n");
	fprintf(f, ";    EffectiveWrites - Number of writes actually performed (= Writes - CoalescedWrites)\n");
	fprintf(f, ";    EffectiveWriteHits - Number of effective writes that found the block in the cache\n");
	fprintf(f, ";    EffectiveWriteMisses - Number of effective writes missing in the cache\n");
	fprintf(f, ";    Evictions - Number of valid blocks replaced in the cache\n");
	fprintf(f, "\n\n");

	/* Print cache statistics */
	for (i = 0; i < list_count(gpu->mod_list); i++)
	{
		/* Get cache */
		mod = list_get(gpu->mod_list, i);
		cache = mod->cache;
		fprintf(f, "[ %s ]\n\n", mod->name);

		/* Configuration */
		if (cache)
		{
			fprintf(f, "Sets = %d\n", cache->nsets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", mod->block_size);
		fprintf(f, "Latency = %d\n", mod->latency);
		fprintf(f, "Banks = %d\n", mod->bank_count);
		fprintf(f, "ReadPorts = %d\n", mod->read_port_count);
		fprintf(f, "WritePorts = %d\n", mod->write_port_count);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", (long long) (mod->reads + mod->writes));
		fprintf(f, "Reads = %lld\n", (long long) mod->reads);
		fprintf(f, "Writes = %lld\n", (long long) mod->writes);
		fprintf(f, "CoalescedReads = %lld\n", (long long) (mod->reads
			- mod->effective_reads));
		fprintf(f, "CoalescedWrites = %lld\n", (long long) (mod->writes
			- mod->effective_writes));
		fprintf(f, "EffectiveReads = %lld\n", (long long) mod->effective_reads);
		fprintf(f, "EffectiveReadHits = %lld\n", (long long) mod->effective_read_hits);
		fprintf(f, "EffectiveReadMisses = %lld\n", (long long) (mod->effective_reads
			- mod->effective_read_hits));
		fprintf(f, "EffectiveWrites = %lld\n", (long long) mod->effective_writes);
		fprintf(f, "EffectiveWriteHits = %lld\n", (long long) mod->effective_write_hits);
		fprintf(f, "EffectiveWriteMisses = %lld\n", (long long) (mod->effective_writes
			- mod->effective_write_hits));
		fprintf(f, "Evictions = %lld\n", (long long) mod->evictions);
		fprintf(f, "\n\n");
	}
	
	
	/* Dump report for networks */
	for (i = 0; i < list_count(gpu->net_list); i++)
		net_dump_report(list_get(gpu->net_list, i), f);

	/* Close */
	fclose(f);
}


void gpu_mem_config_read(void)
{
	struct config_t *config;

	/* Load cache configuration file */
	config = config_create(gpu_mem_config_file_name);
	if (!*gpu_mem_config_file_name)
		gpu_mem_config_default(config);
	else if (!config_load(config))
		fatal("%s: cannot load GPU cache configuration file", gpu_mem_config_file_name);
	
	/* Read networks */
	gpu_mem_config_read_networks(config);

	/* Read modules */
	gpu_mem_config_read_modules(config);

	/* Read low level caches */
	gpu_mem_config_read_low_modules(config);

	/* Read nodes */
	gpu_mem_config_read_nodes(config);

	/* Create switches in internal networks */
	gpu_mem_config_create_switches(config);

	/* Check routes to low and high modules */
	gpu_mem_config_check_routes();

	/* Check that all enforced sections and variables were specified */
	config_check(config);
	config_free(config);
}
