/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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
#include <repos.h>
#include <network.h>
#include <esim.h>
#include <cachesystem.h>


/*
 * Global variables
 */

int gpu_cache_debug_category;

char *gpu_cache_config_file_name = "";
char *gpu_cache_config_help =
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
	"  LinkWidth = <width> (Required)\n"
	"      Link bandwidth in bytes per cycle for network links and switch crossbar.\n"
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
	"  DCache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the compute unit reads/writes data.\n"
	"      The cache must be declared in a previous section of type\n"
	"      '[ Cache <cache_name> ]'.\n"
	"  ICache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the compute unit fetches instructions.\n"
	"\n";



/*
 * Public functions
 */

static void gpu_cache_config_default(struct config_t *config)
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
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		sprintf(section, "Cache l1-%d", compute_unit_id);
		config_write_string(config, section, "Geometry", "geo-l1");
		config_write_string(config, section, "LoNet", "net-l1-l2");

		sprintf(section, "Node cu-%d", compute_unit_id);
		sprintf(str, "l1-%d", compute_unit_id);
		config_write_int(config, section, "ComputeUnit", compute_unit_id);
		config_write_string(config, section, "ICache", str);
		config_write_string(config, section, "DCache", str);
	}

	/* L2 cache */
	sprintf(section, "Cache l2");
	config_write_string(config, section, "Geometry", "geo-l2");
	config_write_string(config, section, "HiNet", "net-l1-l2");
	config_write_string(config, section, "LoNet", "net-l2-gm");

	/* Global memory */
	sprintf(section, "GlobalMemory");
	config_write_string(config, section, "HiNet", "net-l2-gm");
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 100);
	config_write_int(config, section, "Banks", 8);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Network connecting L1 caches and L2 */
	sprintf(section, "Net net-l1-l2");
	config_write_int(config, section, "LinkWidth", 32);

	/* Network connecting L2 cache and global memory */
	sprintf(section, "Net net-l2-gm");
	config_write_int(config, section, "LinkWidth", 32);
}


void gpu_cache_config_read(void)
{
	struct config_t *config;
	struct gpu_cache_t *gpu_cache;
	struct net_t *net;

	char buf[MAX_STRING_SIZE];
	char *section;
	char *value;

	int curr;
	int i, j;

	int sets, assoc, block_size, latency;
	int bank_count, read_port_count, write_port_count;
	char *policy_str;
	enum cache_policy_enum policy;

	int compute_unit_id;
	struct gpu_compute_unit_t *compute_unit;

	char *err_note =
		"\tPlease run 'm2s --help-gpu-cache-config' or consult the Multi2Sim Guide for\n"
		"\ta description of the GPU cache configuration file format.";

	/* Load cache configuration file */
	config = config_create(gpu_cache_config_file_name);
	if (!*gpu_cache_config_file_name)
		gpu_cache_config_default(config);
	else if (!config_load(config))
		fatal("%s: cannot load GPU cache configuration file", gpu_cache_config_file_name);
	
	/* Create array of caches and networks */
	for (section = config_section_first(config); section; section = config_section_next(config)) {
		if (!strncasecmp(section, "Cache ", 6) || !strcasecmp(section, "GlobalMemory"))
			gpu->gpu_cache_count++;
		else if (!strncasecmp(section, "Net ", 4))
			gpu->network_count++;
	}
	if (!gpu->gpu_cache_count)
		fatal("%s: no cache defined.\n%s", cache_system_config_file_name, err_note);
	if (!gpu->network_count)
		fatal("%s: no network defined.\n%s", cache_system_config_file_name, err_note);
	gpu->gpu_caches = calloc(gpu->gpu_cache_count, sizeof(void *));
	gpu->networks = calloc(gpu->network_count, sizeof(void *));
	gpu_cache_debug("gpu_caches: array of %d caches allocated\n", gpu->gpu_cache_count);
	gpu_cache_debug("networks: array of %d networks allocated\n", gpu->network_count);
	
	/* Create networks */
	gpu_cache_debug("creating networks:");
	curr = 0;
	for (section = config_section_first(config); section; section = config_section_next(config)) {
		if (strncasecmp(section, "Net ", 4))
			continue;
		net = net_create(section + 4);
		gpu_cache_debug(" '%s'", net->name);
		gpu->networks[curr++] = net;
	}
	gpu_cache_debug("\n");
	assert(curr == gpu->network_count);

	/* Add network pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (curr = 0; curr < gpu->network_count; curr++)
	{
		net = gpu->networks[curr];
		sprintf(buf, "Net %s", net->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", net);
	}

	/* Create caches */
	gpu_cache_debug("creating caches:");
	curr = 0;
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		if (strncasecmp(section, "Cache ", 6))
			continue;

		/* Cache parameters */
		sprintf(buf, "CacheGeometry %s", config_read_string(config, section, "Geometry", ""));
		config_var_enforce(config, section, "Geometry");
		config_section_enforce(config, buf);
		config_var_enforce(config, buf, "Latency");
		config_var_enforce(config, buf, "Sets");
		config_var_enforce(config, buf, "Assoc");
		config_var_enforce(config, buf, "BlockSize");

		/* Read values */
		sets = config_read_int(config, buf, "Sets", 16);
		assoc = config_read_int(config, buf, "Assoc", 2);
		block_size = config_read_int(config, buf, "BlockSize", 256);
		latency = config_read_int(config, buf, "Latency", 1);
		policy_str = config_read_string(config, buf, "Policy", "LRU");
		bank_count = config_read_int(config, buf, "Banks", 1);
		read_port_count = config_read_int(config, buf, "ReadPorts", 2);
		write_port_count = config_read_int(config, buf, "WritePorts", 1);

		/* Create cache */
		assert(curr < gpu->gpu_cache_count - 1);
		gpu_cache = gpu_cache_create(bank_count, read_port_count, write_port_count);
		gpu->gpu_caches[curr++] = gpu_cache;
		snprintf(gpu_cache->name, sizeof(gpu_cache->name), "%s", section + 6);
		gpu_cache_debug(" '%s'", gpu_cache->name);
		if (!strcasecmp(gpu_cache->name, "GlobalMemory"))
			fatal("%s: '%s' is not a valid name for a cache.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);

		/* High network */
		value = config_read_string(config, section, "HiNet", "");
		sprintf(buf, "net %s", value);
		gpu_cache->net_hi = config_read_ptr(config, buf, "ptr", NULL);
		if (!gpu_cache->net_hi && *value)
			fatal("%s: cache '%s': invalid network name for variable HiNet.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);

		/* Low network */
		value = config_read_string(config, section, "LoNet", "");
		sprintf(buf, "net %s", value);
		gpu_cache->net_lo = config_read_ptr(config, buf, "ptr", NULL);
		if (!gpu_cache->net_lo && *value)
			fatal("%s: cache '%s': invalid network name for variable LoNet.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (!*value)
			fatal("%s: cache '%s': lower network must be specified (use LoNet).\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);

		/* Checks */
		policy = map_string_case(&cache_policy_map, policy_str);
		if (policy == cache_policy_invalid)
			fatal("%s: cache '%s': %s: invalid block replacement policy.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, policy_str, err_note);
		if (sets < 1 || (sets & (sets - 1)))
			fatal("%s: cache '%s': number of sets must be a power of two greater than 1.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (assoc < 1 || (assoc & (assoc - 1)))
			fatal("%s: cache '%s': associativity must be power of two and > 1.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (block_size < 1 || (block_size & (block_size - 1)))
			fatal("%s: cache '%s': block size must be power of two and > 1.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (latency < 1)
			fatal("%s: cache '%s': invalid value for variable 'Latency'.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (bank_count < 1 || (bank_count & (bank_count - 1)))
			fatal("%s: cache '%s': number of banks must be a power of two greater than 1.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (read_port_count < 1)
			fatal("%s: cache '%s': invalid value for variable 'ReadPorts'.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		if (write_port_count < 1)
			fatal("%s: cache '%s': invalid value for variable 'WritePorts'.\n%s",
				gpu_cache_config_file_name, gpu_cache->name, err_note);
		
		/* Assign values and create cache */
		gpu_cache->block_size = block_size;
		gpu_cache->log_block_size = log_base2(block_size);
		gpu_cache->latency = latency;
		gpu_cache->cache = cache_create(sets, block_size, assoc, policy);
	}
	gpu_cache_debug("\n");
	assert(curr == gpu->gpu_cache_count - 1);

	/* Add cache pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections.
	 * Do not handle last element (gpu->gpu_cache_count - 1), which is GlobalMemory. */
	for (curr = 0; curr < gpu->gpu_cache_count - 1; curr++)
	{
		gpu_cache = gpu->gpu_caches[curr];
		sprintf(buf, "Cache %s", gpu_cache->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", gpu_cache);
	}

	/* Global memory */
	gpu_cache_debug("creating global memory\n");
	section = "GlobalMemory";
	sprintf(buf, "Net %s", config_read_string(config, section, "HiNet", ""));
	config_section_enforce(config, buf);
	config_var_enforce(config, section, "Latency");
	config_var_enforce(config, section, "BlockSize");

	/* Global memory - read parameters */
	block_size = config_read_int(config, section, "BlockSize", 64);
	latency = config_read_int(config, section, "Latency", 1);
	bank_count = config_read_int(config, section, "Banks", 4);
	read_port_count = config_read_int(config, section, "ReadPorts", 2);
	write_port_count = config_read_int(config, section, "WritePorts", 2);

	/* Global memory - create cache */
	gpu_cache = gpu_cache_create(bank_count, read_port_count, write_port_count);
	gpu->global_memory = gpu_cache;
	gpu->gpu_caches[gpu->gpu_cache_count - 1] = gpu_cache;
	strcpy(gpu_cache->name, "GlobalMemory");

	/* Global memory - high network */
	value = config_read_string(config, section, "HiNet", "");
	sprintf(buf, "net %s", value);
	gpu_cache->net_hi = config_read_ptr(config, buf, "ptr", NULL);
	if (!gpu_cache->net_hi && *value)
		fatal("%s: global memory: invalid network name for variable HiNet.\n%s",
			gpu_cache_config_file_name, err_note);

	/* Global memory - check parameters */
	if (block_size < 1 || (block_size & (block_size - 1)))
		fatal("%s: global memory: block size must be power of two and > 1.\n%s",
			gpu_cache_config_file_name, err_note);
	if (latency < 1)
		fatal("%s: global memory: invalid value for variable 'Latency'.\n%s",
			gpu_cache_config_file_name, err_note);
	if (bank_count < 1 || (bank_count & (bank_count - 1)))
		fatal("%s: global_memory: number of banks must be a power of two greater than 1.\n%s",
			gpu_cache_config_file_name, err_note);
	if (read_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'ReadPorts'.\n%s",
			gpu_cache_config_file_name, err_note);
	if (write_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'WritePorts'.\n%s",
			gpu_cache_config_file_name, err_note);

	/* Global memory - assign parameters */
	gpu_cache->block_size = block_size;
	gpu_cache->log_block_size = log_base2(block_size);
	gpu_cache->latency = latency;
	
	/* Nodes */
	gpu_cache_debug("creating access points to memory hierarchy:");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *node_name;

		if (strncasecmp(section, "Node ", 5))
			continue;
		node_name = section + 5;

		if (!config_var_exists(config, section, "ComputeUnit"))
			fatal("%s: node '%s': variable 'ComputeUnit' not specified.\n%s",
				gpu_cache_config_file_name, node_name, err_note);
		compute_unit_id = config_read_int(config, section, "ComputeUnit", 0);
		if (compute_unit_id < 0)
			fatal("%s: node '%s': invalid value for variable 'ComputeUnit'.\n%s",
				gpu_cache_config_file_name, node_name, err_note);
		if (compute_unit_id >= gpu_num_compute_units) {
			warning("%s: node '%s': section ignored.\n"
				"\tThis entry in the file will be ignored, because the value for variable\n"
				"\t'ComputeUnit' (%d) refers to a non-existent compute unit (the number of\n"
				"\tavailable compute units was set to %d).",
				gpu_cache_config_file_name, node_name, compute_unit_id, gpu_num_compute_units);
			config_var_allow(config, section, "ICache");
			config_var_allow(config, section, "DCache");
			continue;
		}
		compute_unit = gpu->compute_units[compute_unit_id];

		/* Instruction cache for node */
		value = config_read_string(config, section, "ICache", "");
		if (!*value)
			fatal("%s: node '%s': variable 'ICache' not specified.\n%s",
				gpu_cache_config_file_name, node_name, err_note);
		if (!strcasecmp(value, "GlobalMemory")) {
			compute_unit->gpu_cache_inst = gpu->global_memory;
		} else {
			sprintf(buf, "Cache %s", value);
			if (!config_section_exists(config, buf))
				fatal("%s: node '%s': invalid cache name for variable 'ICache'.\n%s",
					gpu_cache_config_file_name, node_name, err_note);
			compute_unit->gpu_cache_inst = config_read_ptr(config, buf, "ptr", NULL);
		}
		assert(compute_unit->gpu_cache_inst);
		
		/* Data cache for node */
		value = config_read_string(config, section, "DCache", "");
		if (!*value)
			fatal("%s: node '%s': variable 'DCache' not specified.\n%s",
				gpu_cache_config_file_name, node_name, err_note);
		if (!strcasecmp(value, "GlobalMemory")) {
			compute_unit->gpu_cache_data = gpu->global_memory;
		} else {
			sprintf(buf, "Cache %s", value);
			if (!config_section_exists(config, buf))
				fatal("%s: node '%s': invalid cache name for variable 'DCache'.\n%s",
					gpu_cache_config_file_name, node_name, err_note);
			compute_unit->gpu_cache_data = config_read_ptr(config, buf, "ptr", NULL);
		}
		assert(compute_unit->gpu_cache_data);

		/* Debug */
		gpu_cache_debug(" cu[%d].{inst='%s',data='%s'}", compute_unit_id,
			compute_unit->gpu_cache_inst->name, compute_unit->gpu_cache_data->name);
	}
	gpu_cache_debug("\n");

	/* Check that all compute units have an entry to the global memory hierarchy, both
	 * for data and instruction caches. */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		compute_unit = gpu->compute_units[compute_unit_id];
		if (!compute_unit->gpu_cache_inst || !compute_unit->gpu_cache_data)
			fatal("%s: GPU cache configuration file does not specify a valid entry point\n"
				"\tto the memory hierarchy for compute unit %d. Please make sure that a valid\n"
				"\t'[ Node <name> ]' section is used, including variables 'ICache' and 'DCache'.\n%s",
				gpu_cache_config_file_name, compute_unit_id, err_note);
	}

	/* Add lower cache to networks. */
	gpu_cache_debug("adding lower cache to networks:");
	for (curr = 0; curr < gpu->gpu_cache_count; curr++) {
		gpu_cache = gpu->gpu_caches[curr];
		net = gpu_cache->net_hi;
		if (!net)
			continue;
		if (net->node_count)
			fatal("%s: network '%s': only one lower node allowed.\n%s",
				gpu_cache_config_file_name, net->name, err_note);
		net_new_node(net, gpu_cache->name, gpu_cache);
		gpu_cache_debug(" '%s'.node[0]='%s'", net->name, gpu_cache->name);
	}
	gpu_cache_debug("\n");

	/* Check that all networks got assigned a lower node. */
	for (curr = 0; curr < gpu->network_count; curr++) {
		net = gpu->networks[curr];
		assert(net->node_count <= 1);
		if (!net->node_count)
			fatal("%s: network '%s': no lower node leading to global memory.\n%s",
				gpu_cache_config_file_name, net->name, err_note);
	}

	/* Add upper caches to networks. Update 'gpu_cache_next' attributes for caches. */
	gpu_cache_debug("adding upper caches to networks:");
	for (curr = 0; curr < gpu->gpu_cache_count; curr++) {
		gpu_cache = gpu->gpu_caches[curr];
		net = gpu_cache->net_lo;
		if (!net)
			continue;
		gpu_cache->id_lo = net_new_node(net, gpu_cache->name, gpu_cache);
		gpu_cache->gpu_cache_next = net_get_node_data(net, 0);
		gpu_cache_debug(" '%s'.node[%d]='%s'", net->name, gpu_cache->id_lo, gpu_cache->name);
	}
	gpu_cache_debug("\n");

	/* Check that block sizes are equal or larger while we descend through the
	 * memory hierarchy. */
	for (curr = 0; curr < gpu->gpu_cache_count; curr++) {
		block_size = 0;
		for (gpu_cache = gpu->gpu_caches[curr]; gpu_cache; gpu_cache = gpu_cache->gpu_cache_next) {
			if (gpu_cache->block_size < block_size)
				fatal("%s: cache '%s': non-growing block size.\n"
					"\tA cache has been found in the GPU memory hierarchy with a block size larger\n"
					"\tthan some of its lower-level caches, which is not allowed.\n%s",
					gpu_cache_config_file_name, gpu_cache->name, err_note);
			block_size = gpu_cache->block_size;
		}
	}

	/* For each network, add a switch and create node connections.
	 * Then calculate routes between nodes. */
	gpu_cache_debug("creating network switches and links:");
	for (i = 0; i < gpu->network_count; i++)
	{
		int maxmsg, sw;
		int link_width;

		/* Get the maximum message size for network, which is equals to
		 * the block size of the lower cache plus 8 (control message) */
		net = gpu->networks[i];
		gpu_cache = net_get_node_data(net, 0);
		if (!gpu_cache)
			continue;
		maxmsg = gpu_cache->block_size + 8;

		/* Get network link width */
		sprintf(buf, "Net %s", net->name);
		config_var_enforce(config, buf, "LinkWidth");
		link_width = config_read_int(config, buf, "LinkWidth", 8);
		if (link_width < 1)
			fatal("%s: network '%s': invalid value for variable 'LinkWidth'.\n%s",
				gpu_cache_config_file_name, net->name, err_note);

		/* Create switch and connections. By default, each i/o buffer has
		 * space for two maximum-length messages. */
		snprintf(buf, sizeof(buf), "%s.sw", net->name);
		sw = net_new_switch(net, net->end_node_count, maxmsg * 2,
			net->end_node_count, maxmsg * 2, link_width, buf, NULL);
		gpu_cache_debug(" '%s'.node[%d]='switch'", net->name, sw);
		for (j = 0; j < net->end_node_count; j++) {
			net_new_bidirectional_link(net, j, sw, link_width);
			gpu_cache = net_get_node_data(net, j);
			gpu_cache_debug(" '%s'.connect('%s','switch')", net->name, gpu_cache->name);
		}

		net_calculate_routes(net);
	}
	gpu_cache_debug("\n");

	/* Check that all enforced sections and variables were specified */
	config_check(config);
	config_free(config);
}


void gpu_cache_init(void)
{
	/* Events */
	EV_GPU_CACHE_READ = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_REPLY = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_FINISH = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_UNLOCK = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_FINISH = esim_register_event(gpu_cache_handler_read);

	EV_GPU_CACHE_WRITE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_UNLOCK = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_FINISH = esim_register_event(gpu_cache_handler_write);

	/* Repository of GPU cache stacks */
	gpu_cache_stack_repos = repos_create(sizeof(struct gpu_cache_stack_t), "gpu_cache_stack_repos");
	
	/* Read cache configuration file */
	gpu_cache_config_read();
}


void gpu_cache_done(void)
{
	int i;
	
	/* Free caches and cache array */
	for (i = 0; i < gpu->gpu_cache_count; i++)
		gpu_cache_free(gpu->gpu_caches[i]);
	free(gpu->gpu_caches);

	/* Free networks */
	for (i = 0; i < gpu->network_count; i++)
		net_free(gpu->networks[i]);
	free(gpu->networks);

	/* GPU cache stack repository */
	repos_free(gpu_cache_stack_repos);
}


struct gpu_cache_t *gpu_cache_create(int bank_count, int read_port_count, int write_port_count)
{
	struct gpu_cache_t *gpu_cache;
	
	gpu_cache = calloc(1, sizeof(struct gpu_cache_t));
	gpu_cache->bank_count = bank_count;
	gpu_cache->read_port_count = read_port_count;
	gpu_cache->write_port_count = write_port_count;
	gpu_cache->banks = calloc(1, gpu_cache->bank_count + SIZEOF_GPU_CACHE_BANK(gpu_cache));
	return gpu_cache;
}


void gpu_cache_free(struct gpu_cache_t *gpu_cache)
{
	if (gpu_cache->cache)
		cache_free(gpu_cache->cache);
	free(gpu_cache->banks);
	free(gpu_cache);
}


void gpu_cache_read(int compute_unit_id, uint32_t addr, uint32_t size)
{
	struct gpu_cache_stack_t *stack;

	gpu_cache_stack_id++;
	assert(IN_RANGE(compute_unit_id, 0, gpu_num_compute_units));
	stack = gpu_cache_stack_create(gpu_cache_stack_id,
		gpu->gpu_caches[compute_unit_id], addr,
		ESIM_EV_NONE, NULL);
	esim_schedule_event(EV_GPU_CACHE_READ, stack, 0);
}


void gpu_cache_write(int compute_unit_id, uint32_t addr, uint32_t size)
{
	struct gpu_cache_stack_t *stack;

	gpu_cache_stack_id++;
	assert(IN_RANGE(compute_unit_id, 0, gpu_num_compute_units));
	stack = gpu_cache_stack_create(gpu_cache_stack_id,
		gpu->gpu_caches[compute_unit_id], addr,
		ESIM_EV_NONE, NULL);
	esim_schedule_event(EV_GPU_CACHE_WRITE, stack, 0);
}




/*
 * Event-driven simulation
 */


/* Events */

int EV_GPU_CACHE_READ;
int EV_GPU_CACHE_READ_REQUEST;
int EV_GPU_CACHE_READ_REQUEST_RECEIVE;
int EV_GPU_CACHE_READ_REQUEST_REPLY;
int EV_GPU_CACHE_READ_REQUEST_FINISH;
int EV_GPU_CACHE_READ_UNLOCK;
int EV_GPU_CACHE_READ_FINISH;

int EV_GPU_CACHE_WRITE;
int EV_GPU_CACHE_WRITE_REQUEST_RECEIVE;
int EV_GPU_CACHE_WRITE_UNLOCK;
int EV_GPU_CACHE_WRITE_FINISH;

struct repos_t *gpu_cache_stack_repos;
uint64_t gpu_cache_stack_id;


struct gpu_cache_stack_t *gpu_cache_stack_create(uint64_t id, struct gpu_cache_t *gpu_cache,
	uint32_t addr, int ret_event, void *ret_stack)
{
	struct gpu_cache_stack_t *stack;
	stack = repos_create_object(gpu_cache_stack_repos);
	stack->id = id;
	stack->gpu_cache = gpu_cache;
	stack->addr = addr;
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;
	return stack;
}


void gpu_cache_stack_return(struct gpu_cache_stack_t *stack)
{
	int ret_event = stack->ret_event;
	void *ret_stack = stack->ret_stack;

	repos_free_object(gpu_cache_stack_repos, stack);
	esim_schedule_event(ret_event, ret_stack, 0);
}


void gpu_cache_stack_wait_in_cache(struct gpu_cache_stack_t *stack, int event)
{
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_cache, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_cache, waiting, stack);
}


/* Enqueue stack in waiting list of 'stack->port' */
void gpu_cache_stack_wait_in_port(struct gpu_cache_stack_t *stack, int event)
{
	struct gpu_cache_port_t *port = stack->port;

	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


/* Wake up accesses in 'gpu_cache->waiting_list' */
void gpu_cache_wakeup(struct gpu_cache_t *gpu_cache)
{
	struct gpu_cache_stack_t *stack;
	int event;

	while (gpu_cache->waiting_list_head) {
		stack = gpu_cache->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(gpu_cache, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/* Wake up accesses in 'port->waiting_list' */
void gpu_cache_port_wakeup(struct gpu_cache_port_t *port)
{
	struct gpu_cache_stack_t *stack;
	int event;

	while (port->waiting_list_head) {
		stack = port->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(port, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


#define CYCLE ((long long) esim_cycle)
#define ID ((long long) stack->id)


void gpu_cache_handler_read(int event, void *data)
{
	struct gpu_cache_stack_t *stack = data, *newstack;
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	if (event == EV_GPU_CACHE_READ)
	{
		struct gpu_cache_port_t *port;
		int i;

		/* If there is any pending access in the cache, this access should
		 * be enqueued in the waiting list, since all accesses need to be
		 * done in order. */
		if (gpu_cache->waiting_list_head) {
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_READ);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(gpu_cache->block_size - 1);
		stack->block_index = stack->tag >> gpu_cache->log_block_size;
		stack->bank_index = stack->block_index % gpu_cache->bank_count;
		stack->bank = GPU_CACHE_BANK_INDEX(gpu_cache, stack->bank_index);

		/* If any read port in bank is processing the same tag starting
		 * in the current cycle, the accesses are coalesced. */
		for (i = 0; i < gpu_cache->read_port_count; i++) {
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, stack->bank, i);
			if (port->locked && port->lock_when == esim_cycle && port->stack->tag == stack->tag) {
				gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->read_port_index = i;
				stack->port = port;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d read_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->read_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_READ_FINISH);
				return;
			}
		}

		/* Look for a free read port */
		for (i = 0; i < gpu_cache->read_port_count; i++) {
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked) {
				stack->read_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free read port, enqueue in cache waiting list. */
		if (!stack->port) {
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
			gpu_cache_debug("  %lld %lld wait why=\"no_read_port\"\n", CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_READ);
			return;
		}

		/* Lock read port */
		port = stack->port;
		assert(!port->locked);
		assert(gpu_cache->locked_read_port_count < gpu_cache->read_port_count * gpu_cache->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		gpu_cache->locked_read_port_count++;
		gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->read_port_index);
	
		/* If there is no cache, assume hit */
		if (!gpu_cache->cache) {
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);
			return;
		}

		/* Get block from cache, consuming 'latency' cycles. */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);
		else {
			stack->way = cache_replace_block(gpu_cache->cache, stack->set);
			esim_schedule_event(EV_GPU_CACHE_READ_REQUEST, stack, gpu_cache->latency);
		}

		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST)
	{
		struct net_t *net = gpu_cache->net_lo;
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		assert(net);
		assert(target);
		gpu_cache_debug("  %lld %lld read_request src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, gpu_cache->name, target->name, net->name);
		net_send_ev(net, gpu_cache->id_lo, target->id_hi, 8, EV_GPU_CACHE_READ_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		gpu_cache_debug("  %lld %lld read_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);

		newstack = gpu_cache_stack_create(stack->id,
			gpu_cache->gpu_cache_next, stack->tag,
			EV_GPU_CACHE_READ_REQUEST_REPLY, stack);
		esim_schedule_event(EV_GPU_CACHE_READ, newstack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_REPLY)
	{
		struct net_t *net = gpu_cache->net_lo;
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		assert(net && target);
		gpu_cache_debug("  %lld %lld read_request_reply src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, target->name, gpu_cache->name, net->name);
		net_send_ev(net, target->id_hi, gpu_cache->id_lo, gpu_cache->block_size + 8,
			EV_GPU_CACHE_READ_REQUEST_FINISH, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_FINISH)
	{
		gpu_cache_debug("  %lld %lld read_request_finish\n", CYCLE, ID);
		assert(gpu_cache->cache);

		/* Set tag and state of the new block.
		 * A set other than 0 means that the block is valid. */
		cache_set_block(gpu_cache->cache, stack->set, stack->way, stack->tag, 1);
		esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_UNLOCK)
	{
		struct gpu_cache_port_t *port = stack->port;

		gpu_cache_debug("  %lld %lld read_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (gpu_cache->cache)
			cache_access_block(gpu_cache->cache, stack->set, stack->way);

		/* Unlock port */
		assert(port->locked);
		assert(gpu_cache->locked_read_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		gpu_cache->locked_read_port_count--;

		/* Wake up accesses in waiting lists */
		gpu_cache_port_wakeup(port);
		gpu_cache_wakeup(gpu_cache);

		esim_schedule_event(EV_GPU_CACHE_READ_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_FINISH)
	{
		gpu_cache_debug("  %lld %lld read_finish\n", CYCLE, ID);

		/* Return */
		gpu_cache_stack_return(stack);
		return;

	}

	abort();
}


void gpu_cache_handler_write(int event, void *data)
{
	struct gpu_cache_stack_t *stack = data;
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	if (event == EV_GPU_CACHE_WRITE)
	{
		struct gpu_cache_t *target;
		struct gpu_cache_port_t *port;
		struct net_t *net;
		int i;

		/* If there is any pending access in the cache, access gets enqueued. */
		if (gpu_cache->waiting_list_head) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* If there is any locked read port in the cache, the write is stalled.
		 * The reason is that a write must wait for all reads to be complete, since
		 * writes could be faster than reads in the memory hierarchy. */
		if (gpu_cache->locked_read_port_count) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"write_after_read\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(gpu_cache->block_size - 1);
		stack->block_index = stack->tag >> gpu_cache->log_block_size;
		stack->bank_index = stack->block_index % gpu_cache->bank_count;
		stack->bank = GPU_CACHE_BANK_INDEX(gpu_cache, stack->bank_index);

		/* If any write port in bank is processing the same tag starting
		 * in the current cycle, the accesses are coalesced. */
		for (i = 0; i < gpu_cache->write_port_count; i++) {
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
			if (port->locked && port->lock_when == esim_cycle && port->stack->tag == stack->tag) {
				gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->write_port_index = i;
				stack->port = port;
				stack->pending++;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d write_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->write_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_WRITE_FINISH);
				return;
			}
		}

		/* Look for a free write port */
		for (i = 0; i < gpu_cache->write_port_count; i++) {
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked) {
				stack->write_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free write port, enqueue in cache waiting list. */
		if (!stack->port) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
			gpu_cache_debug("  %lld %lld wait why=\"no_write_port\"\n", CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Lock write port */
		port = stack->port;
		assert(!port->locked);
		assert(gpu_cache->locked_write_port_count < gpu_cache->write_port_count * gpu_cache->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		gpu_cache->locked_write_port_count++;
		gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d write_port=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->write_port_index);
	
		/* If this is main memory, access block */
		if (!gpu_cache->cache) {
			stack->pending++;
			esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);
			return;
		}

		/* Access cache to write on block (write actually occurs only if block is present). */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		stack->pending++;
		esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);

		/* Access lower level cache */
		net = gpu_cache->net_lo;
		target = gpu_cache->gpu_cache_next;
		assert(target);
		assert(net);
		stack->pending++;
		gpu_cache_debug("  %lld %lld write_request src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, target->name, gpu_cache->name, net->name);
		net_send_ev(net, gpu_cache->id_lo, target->id_hi, 8, EV_GPU_CACHE_WRITE_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;
		struct gpu_cache_stack_t *newstack;

		gpu_cache_debug("  %lld %lld write_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);
		newstack = gpu_cache_stack_create(stack->id, target, stack->tag,
			EV_GPU_CACHE_WRITE_FINISH, stack);
		esim_schedule_event(EV_GPU_CACHE_WRITE, newstack, 0);
		return;
	}
	
	if (event == EV_GPU_CACHE_WRITE_UNLOCK)
	{
		struct gpu_cache_port_t *port = stack->port;

		gpu_cache_debug("  %lld %lld write_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (stack->hit) {
			assert(gpu_cache->cache);
			cache_access_block(gpu_cache->cache, stack->set, stack->way);
		}

		/* Unlock port */
		assert(port->locked);
		assert(gpu_cache->locked_write_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		gpu_cache->locked_write_port_count--;

		/* Wake up accesses in waiting lists */
		gpu_cache_port_wakeup(port);
		gpu_cache_wakeup(gpu_cache);

		esim_schedule_event(EV_GPU_CACHE_WRITE_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_FINISH)
	{
		
		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Return */
		gpu_cache_debug("  %lld %lld write_finish\n", CYCLE, ID);
		gpu_cache_stack_return(stack);
		return;
	}

	abort();
}

