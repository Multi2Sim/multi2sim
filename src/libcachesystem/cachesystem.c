/*
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

#include "cachesystem.h"

/* Help message */
char *cache_system_config_help =
	"The cache system configuration file is plain text file in the IniFile format,\n"
	"describing the model for the memory hierarchy and interconnection networks.\n"
	"This file is passed to Multi2Sim with option '--cpu-cache-config <file>', and\n"
	"should be used together with option '--cpu-sim detailed' to perform an\n"
	"architectural simulation.\n"
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
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports.\n"
	"  WritePorts = <num> (Default = 1)\n"
	"      Number of write ports.\n"
	"\n"
	"Section '[ Net <name> ]': defines an interconnection network.\n"
	"\n"
	"  Topology = {Bus|P2P} (Required)\n"
	"      Interconnection network topology.\n"
	"  LinkWidth = <width> (Required)\n"
	"      Link bandwidth in bytes per cycle.\n"
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
	"Section '[ MainMemory ]'\n"
	"\n"
	"  HiNet = <net_name> (Required if there are caches)\n"
	"      Upper interconnect where main memory is connected to.\n"
	"  Latency = <num_cycles> (Required)\n"
	"      Main memory access latency.\n"
	"  BlockSize = <size> (Required)\n"
	"      Memory block size in bytes.\n"
	"\n"
	"Section '[ Node <name> ]': defines an entry to the memory hierarchy from a\n"
	"processing node in the CPU, identified as a pair {core, thread}.\n"
	"\n"
	"  Core = <core> (Required)\n"
	"      Core ID in the CPU.\n"
	"  Thread = <thread> (Required)\n"
	"      Hardware Thread ID in the CPU.\n"
	"  DCache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the processing node reads/writes data.\n"
	"      The cache must be declared in a previous section of type\n"
	"      '[ Cache <cache_name> ]'.\n"
	"  ICache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the processing node fetches instructions.\n"
	"\n"
	"Section '[ TLB <name> ]': defines the parameters of the TLBs. There will be\n"
	"one instruction TLB and one data TLB per hardware thread. This section is\n"
	"optional.\n"
	"\n"
	"  Sets = <num_sets> (Default = 64)\n"
	"      Number of sets of data and instruction TLBs\n"
	"  Assoc = <num_ways> (Default = 4)\n"
	"      Associativity of TLBs.\n"
	"  HitLatency = <cycles> (Default = 2)\n"
	"      Access latency upon a hit.\n"
	"  MissLatency = <cycles> (Default = 30)\n"
	"      TLB access latency upon a lookup miss.\n"
	"\n";

char *err_cachesystem_icache =
	"\tA node described in the cache configuration file does not\n"
	"\thave an entry into the memory hierarchy for fetching instructions\n"
	"\t(instruction cache). Please write a '[ Node <name> ]' section in\n"
	"\tthe cache configuration file for it, including an\n"
	"\t'ICache = <cache>' entry.\n";

char *err_cachesystem_dcache =
	"\tA node described in the cache configuration file does not\n"
	"\thave an entry into the memory hierarchy for data\n"
	"\t(data cache). Please write a '[ Node <name> ]' section in\n"
	"\tthe cache configuration file for it, including a\n"
	"\t'DCache = <cache>' entry.\n";

char *err_cachesystem_ignored =
	"\tA '[ Node <id> ]' section is ignored in the cache configuration\n"
	"\tfile if it refers to an unused core/thread, as specified in the\n"
	"\tCPU configuration file.\n";


/* Cache system variables */

static int cores = 0;
static int threads = 0;
static int mod_count = 0;
static int tlb_count = 0;
static int net_count = 0;
static uint64_t access_counter = 0;

struct node_t
{
	struct mod_t *icache;
	struct mod_t *dcache;
};

static struct node_t *node_array;
static struct mod_t **mod_array;
static struct tlb_t **tlb_array;
static struct net_t **net_array;




/* Coherent Cache */

struct mod_t *__mod_create(char *name, enum mod_kind_t kind)
{
	struct mod_t *mod;

	mod = calloc(1, sizeof(struct mod_t));
	mod->kind = kind;
	mod->name = strdup(name);
	mod->access_list = linked_list_create();
	mod->high_mod_list = linked_list_create();
	mod->low_mod_list = linked_list_create();
	return mod;
}


void __mod_free(struct mod_t *mod)
{
	/* Free linked list of pending accesses.
	 * Each element is in turn a linked list of access aliases. */
	while (linked_list_count(mod->access_list))
	{
		struct ccache_access_t *a, *n;
		linked_list_head(mod->access_list);
		for (a = linked_list_get(mod->access_list); a; a = n)
		{
			n = a->next;
			free(a);
		}
		linked_list_remove(mod->access_list);
	}
	linked_list_free(mod->access_list);

	/* Free cache */
	if (mod->dir)
		dir_free(mod->dir);
	if (mod->cache)
		cache_free(mod->cache);
	linked_list_free(mod->high_mod_list);
	linked_list_free(mod->low_mod_list);
	free(mod->name);
	free(mod);
}


/* Look for an in-flight access. If it is found, return the associated
 * ccache_access_t element, and place the linked list pointer into
 * its position within access_list. */
static struct ccache_access_t *__mod_find_access(struct mod_t *mod,
	uint32_t addr)
{
	struct ccache_access_t *access;
	addr &= ~(mod->block_size - 1);
	for (linked_list_head(mod->access_list); !linked_list_is_end(mod->access_list);
		linked_list_next(mod->access_list))
	{
		access = linked_list_get(mod->access_list);
		if (access->address == addr)
			return access;
	}
	return NULL;
}


struct ccache_access_t *__mod_start_access(struct mod_t *mod,
	enum mod_access_kind_t access_kind, uint32_t addr,
	struct linked_list_t *eventq, void *eventq_item)
{
	struct ccache_access_t *access, *alias;

	/* Create access */
	access = calloc(1, sizeof(struct ccache_access_t));
	access->access_kind = access_kind;
	access->address = addr & ~(mod->block_size - 1);
	access->eventq = eventq;
	access->eventq_item = eventq_item;

	/* If an alias is found, insert new access at the alias linked list head.
	 * If no alias found, a new entry and id are created. */
	alias = __mod_find_access(mod, addr);
	if (alias) {
		assert(access->access_kind == mod_access_kind_read);
		assert(alias->access_kind == mod_access_kind_read);
		access->next = alias->next;
		alias->next = access;
		access->id = alias->id;
	} else {
		linked_list_out(mod->access_list);
		linked_list_insert(mod->access_list, access);
		access->id = ++access_counter;
		access_kind == mod_access_kind_read ? mod->pending_reads++
			: mod->pending_writes++;
		assert(mod->pending_reads <= mod->read_port_count);
		assert(mod->pending_writes <= mod->write_port_count);
	}
	return access;
}


void __mod_end_access(struct mod_t *mod, uint32_t addr)
{
	struct ccache_access_t *access, *alias;

	/* Find access */
	addr &= ~(mod->block_size - 1);
	access = __mod_find_access(mod, addr);
	assert(access && access->address == addr);

	/* Finish actions - insert eventq_item into eventq for all aliases */
	for (alias = access; alias; alias = alias->next)
	{
		if (alias->eventq)
		{
			assert(alias->eventq_item);
			linked_list_head(alias->eventq);
			linked_list_insert(alias->eventq, alias->eventq_item);
		}
	}

	/* Free access and all aliases. */
	access->access_kind == mod_access_kind_read ? mod->pending_reads--
		: mod->pending_writes--;
	assert(mod->pending_reads >= 0);
	assert(mod->pending_writes >= 0);
	while (access)
	{
		alias = access->next;
		free(access);
		access = alias;
	}
	linked_list_remove(mod->access_list);
}


int __mod_pending_access(struct mod_t *mod, uint64_t id)
{
	struct ccache_access_t *access;
	for (linked_list_head(mod->access_list); !linked_list_is_end(mod->access_list);
		linked_list_next(mod->access_list))
	{
		access = linked_list_get(mod->access_list);
		if (access->id == id)
			return 1;
	}
	return 0;
}


int __mod_pending_address(struct mod_t *mod, uint32_t addr)
{
	struct ccache_access_t *access;
	access = __mod_find_access(mod, addr);
	return access != NULL;
}


/* Return {set, way, tag, state} for an address. */
int __mod_find_block(struct mod_t *mod, uint32_t addr,
	uint32_t *pset, uint32_t *pway, uint32_t *ptag, int *pstatus)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;
	uint32_t set, way, tag;

	/* Cache. A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->block_mask;
	set = (tag >> cache->log_block_size) % cache->num_sets;
	for (way = 0; way < cache->assoc; way++)
	{
		blk = &cache->sets[set].blocks[way];
		if (blk->tag == tag && blk->state)
			break;
		if (blk->transient_tag == tag)
		{
			dir_lock = dir_lock_get(mod->dir, set, way);
			if (dir_lock->lock)
				break;
		}
	}

	/* Miss */
	if (way == cache->assoc)
	{
		PTR_ASSIGN(pset, set);
		PTR_ASSIGN(ptag, tag);
		PTR_ASSIGN(pway, 0);
		PTR_ASSIGN(pstatus, 0);
		return 0;
	}
	
	/* Hit */
	PTR_ASSIGN(pset, set);
	PTR_ASSIGN(pway, way);
	PTR_ASSIGN(ptag, tag);
	PTR_ASSIGN(pstatus, cache->sets[set].blocks[way].state);
	return 1;
}


void __mod_dump(struct mod_t *mod, FILE *f)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;
	int i, j;
	
	if (!cache)
		return;

	for (i = 0; i < cache->num_sets; i++)
	{
		fprintf(f, "Set %03d:", i);
		for (j = 0; j < cache->assoc; j++)
		{
			dir_lock = dir_lock_get(mod->dir, i, j);
			blk = &cache->sets[i].blocks[j];
			if (!blk->state)
				fprintf(f, " %d:I", j);
			else
				fprintf(f, " %d:0x%x", j, blk->tag);
			if (dir_lock->lock)
				fprintf(f, "*LOCK->0x%x*", blk->transient_tag);
		}
		fprintf(f, "\n");
	}
}


/* Get lower node */
struct mod_t *__mod_get_low_mod(struct mod_t *mod)
{
	linked_list_head(mod->low_mod_list);
	return linked_list_get(mod->low_mod_list);
}




/* TLB */

struct tlb_t *tlb_create()
{
	struct tlb_t *tlb;
	tlb = calloc(1, sizeof(struct tlb_t));
	return tlb;
}


void tlb_free(struct tlb_t *tlb)
{
	/* Free */
	if (tlb->cache)
		cache_free(tlb->cache);
	free(tlb);
}




/* Cache System Stack */

struct cache_system_stack_t *cache_system_stack_create(int core, int thread, uint32_t addr,
	int retevent, void *retstack)
{
	struct cache_system_stack_t *stack;
	stack = calloc(1, sizeof(struct cache_system_stack_t));
	stack->core = core;
	stack->thread = thread;
	stack->addr = addr;
	stack->retevent = retevent;
	stack->retstack = retstack;
	return stack;
}


void cache_system_stack_return(struct cache_system_stack_t *stack)
{
	int retevent = stack->retevent;
	void *retstack = stack->retstack;

	free(stack);
	esim_schedule_event(retevent, retstack, 0);
}




/* Cache system */

int EV_CACHE_SYSTEM_ACCESS;
int EV_CACHE_SYSTEM_ACCESS_CACHE;
int EV_CACHE_SYSTEM_ACCESS_TLB;
int EV_CACHE_SYSTEM_ACCESS_FINISH;

char *cache_system_config_file_name = "";
char *cache_system_report_file_name = "";
struct config_t *cache_config;
int cache_min_block_size = 0;
int cache_max_block_size = 0;
uint32_t mem_latency = 200;
int cache_system_iperfect = 0;
int cache_system_dperfect = 0;


/* Check that a section exists */
static void cache_config_section(char *section)
{
	if (!config_section_exists(cache_config, section))
		fatal("%s: section '%s' not present", cache_system_config_file_name, section);
}


/* Check that a key exists in configuration file */
static void cache_config_key(char *section, char *key)
{
	if (!config_var_exists(cache_config, section, key))
		fatal("%s: section '%s': key '%s' not present",
			cache_system_config_file_name, section, key);
}


/* Create default configuration file */
#define SECTION(V) strcpy(section, (V))
#define KEY_INT(K, V) config_write_int(cache_config, section, (K), (V))
#define KEY_STRING(K, V) config_write_string(cache_config, section, (K), (V))

static void cache_config_default(void)
{
	char section[100], buf[100];
	int core, thread;

	/* Write cache topologies */
	SECTION("CacheGeometry L1");
	KEY_INT("Sets", 256);
	KEY_INT("Assoc", 2);
	KEY_INT("BlockSize", 64);
	KEY_INT("Latency", 2);

	SECTION("CacheGeometry L2");
	KEY_INT("Sets", 1024);
	KEY_INT("Assoc", 8);
	KEY_INT("BlockSize", 64);
	KEY_INT("Latency", 20);

	/* Main memory */
	SECTION("MainMemory");
	sprintf(buf, "net-%d", cores);
	KEY_STRING("HiNet", buf);
	KEY_INT("BlockSize", 64);
	KEY_INT("DirectorySize", 1024);
	KEY_INT("DirectoryAssoc", 8);
	KEY_INT("Latency", 200);

	/* Nodes */
	for (core = 0; core < cores; core++) {
		for (thread = 0; thread < threads; thread++) {
			sprintf(buf, "Node %d.%d", core, thread);
			SECTION(buf);
			KEY_INT("Core", core);
			KEY_INT("Thread", thread);
			sprintf(buf, "il1-%d", core * threads + thread);
			KEY_STRING("ICache", buf);
			sprintf(buf, "dl1-%d", core * threads + thread);
			KEY_STRING("DCache", buf);
		}
	}

	/* Level 1 Caches */
	for (core = 0; core < cores; core++) {
		for (thread = 0; thread < threads; thread++) {
			sprintf(buf, "Cache dl1-%d", core * threads + thread);
			SECTION(buf);
			KEY_STRING("Geometry", "L1");
			KEY_STRING("HiNet", "");
			sprintf(buf, "net-%d", core);
			KEY_STRING("LoNet", buf);

			sprintf(buf, "Cache il1-%d", core * threads + thread);
			SECTION(buf);
			KEY_STRING("Geometry", "L1");
			KEY_STRING("HiNet", "");
			sprintf(buf, "net-%d", core);
			KEY_STRING("LoNet", buf);
		}
	}

	/* Level 2 Caches */
	for (core = 0; core < cores; core++) {
		sprintf(buf, "Cache l2-%d", core);
		SECTION(buf);
		KEY_STRING("Geometry", "L2");
		sprintf(buf, "net-%d", core);
		KEY_STRING("HiNet", buf);
		sprintf(buf, "net-%d", cores);
		KEY_STRING("LoNet", buf);
	}

	/* Interconnects */
	for (core = 0; core <= cores; core++) {
		sprintf(buf, "Net net-%d", core);
		SECTION(buf);
		KEY_STRING("Topology", "P2P");
	}
}

#undef KEY_INT
#undef KEY_STRING
#undef SECTION


void cache_system_init(int _cores, int _threads)
{
	int i, j;

	struct tlb_t *dtlb;
	struct tlb_t *itlb;
	
	char *section;
	char *value;

	int core, thread;
	int curr;

	int nsets;
	int bsize;
	int assoc;
	int read_ports;
	int write_ports;

	struct net_t *net = NULL;
	struct mod_t *mod;
	struct node_t *node;
	char buf[200];
	enum cache_policy_t policy;
	char *policy_str;

	int net_msg_size;
	int net_bandwidth;
	int net_buffer_size;

	/* Try to open report file */
	if (cache_system_report_file_name[0] && !can_open_write(cache_system_report_file_name))
		fatal("%s: cannot open cache system report file",
			cache_system_report_file_name);

	/* Initializations */
	cores = _cores;
	threads = _threads;
	mmu_init();

	/* Events */
	EV_CACHE_SYSTEM_ACCESS = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_CACHE = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_TLB = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_FINISH = esim_register_event(cache_system_handler);

	/* Load cache configuration file */
	cache_config = config_create(cache_system_config_file_name);
	if (!*cache_system_config_file_name)
		cache_config_default();
	else if (!config_load(cache_config))
		fatal("%s: cannot load cache configuration file", cache_system_config_file_name);
	
	/* Create array of ccaches and networks. */
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (!strncasecmp(section, "Cache ", 6) || !strcasecmp(section, "MainMemory"))
			mod_count++;
		else if (!strncasecmp(section, "Net ", 4))
			net_count++;
	}
	if (mod_count < 1)
		fatal("%s: no cache", cache_system_config_file_name);
	if (net_count < 1)
		fatal("%s: no network", cache_system_config_file_name);
	if (!config_section_exists(cache_config, "MainMemory"))
		fatal("%s: section [ MainMemory ] is missing", cache_system_config_file_name);
	mod_array = calloc(mod_count, sizeof(void *));
	net_array = calloc(net_count, sizeof(void *));

	/* Create networks */
	curr = 0;
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "Net ", 4))
			continue;

		net = net_create(section + 4);
		net_array[curr++] = net;
	}
	assert(curr == net_count);

	/* Add network pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (curr = 0; curr < net_count; curr++)
	{
		net = net_array[curr];
		sprintf(buf, "Net %s", net->name);
		assert(config_section_exists(cache_config, buf));
		config_write_ptr(cache_config, buf, "ptr", net);
	}

	/* Create array of modules */
	curr = 0;
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "Cache ", 6))
			continue;

		/* Create cache */
		assert(curr < mod_count - 1);
		mod = __mod_create(section + 6, mod_kind_cache);
		mod_array[curr] = mod;
		curr++;
		if (!strcasecmp(mod->name, "MainMemory"))
			fatal("'%s' is not a valid name for a cache", mod->name);

		/* High network */
		value = config_read_string(cache_config, section, "HiNet", "");
		sprintf(buf, "net %s", value);
		mod->high_net = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (!mod->high_net && *value)
			fatal("%s: network specified in HiNet does not exist", mod->name);

		/* Low network */
		value = config_read_string(cache_config, section, "LoNet", "");
		sprintf(buf, "net %s", value);
		mod->low_net = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (!mod->low_net && *value)
			fatal("%s: network specified in LoNet does not exist", mod->name);
		if (!*value)
			fatal("%s: cache must be connected to a lower network (use LoNet)", mod->name);

		/* Cache parameters */
		sprintf(buf, "CacheGeometry %s", config_read_string(cache_config, section, "Geometry", ""));
		cache_config_key(section, "Geometry");
		cache_config_section(buf);
		cache_config_key(buf, "Latency");
		cache_config_key(buf, "Sets");
		cache_config_key(buf, "Assoc");
		cache_config_key(buf, "BlockSize");
		nsets = config_read_int(cache_config, buf, "Sets", 0);
		assoc = config_read_int(cache_config, buf, "Assoc", 0);
		bsize = config_read_int(cache_config, buf, "BlockSize", 0);
		read_ports = config_read_int(cache_config, buf, "ReadPorts", 2);
		write_ports = config_read_int(cache_config, buf, "WritePorts", 1);
		policy_str = config_read_string(cache_config, buf, "Policy", "LRU");
		policy = map_string_case(&cache_policy_map, policy_str);
		if (policy == cache_policy_invalid)
			fatal("%s: invalid block replacement policy", policy_str);
		mod->latency = config_read_int(cache_config, buf, "Latency", 0);
		mod->block_size = bsize;
		mod->log_block_size = log_base2(bsize);
		mod->read_port_count = read_ports;
		mod->write_port_count = write_ports;
		mod->cache = cache_create(nsets, bsize, assoc, policy);
		cache_min_block_size = cache_min_block_size ? MIN(cache_min_block_size, bsize) : bsize;
		cache_max_block_size = cache_max_block_size ? MAX(cache_max_block_size, bsize) : bsize;
		if (bsize > mmu_page_size)
			fatal("%s: cache block size greater than memory page size", mod->name);
		if (read_ports < 1 || write_ports < 1)
			fatal("%s: number of read/write ports must be at least 1", mod->name);
	}
	assert(curr == mod_count - 1);

	/* Add module pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections.
	 * Do not handle last element (ccache_count - 1), which is MainMemory. */
	for (curr = 0; curr < mod_count - 1; curr++)
	{
		mod = mod_array[curr];
		sprintf(buf, "Cache %s", mod->name);
		assert(config_section_exists(cache_config, buf));
		config_write_ptr(cache_config, buf, "ptr", mod);
	}

	/* Main memory */
	section = "MainMemory";
	mod = __mod_create("mm", mod_kind_main_memory);
	mod_array[mod_count - 1] = mod;
	sprintf(buf, "Net %s", config_read_string(cache_config, section, "HiNet", ""));
	cache_config_section(section);
	cache_config_key(section, "Latency");
	cache_config_key(section, "BlockSize");

	read_ports = config_read_int(cache_config, section, "ReadPorts", 2);
	write_ports = config_read_int(cache_config, section, "WritePorts", 1);
	if (read_ports < 1 || write_ports < 1)
		fatal("%s: number of read/write ports must be at least 1", mod->name);
	bsize = config_read_int(cache_config, section, "BlockSize", 0);
	if (bsize & (bsize - 1))
		fatal("block size for main memory is not a power of 2");
	if (bsize > mmu_page_size)
		fatal("main memory block size cannot be greater than page size");
	mod->block_size = bsize;
	mod->log_block_size = log_base2(bsize);
	cache_min_block_size = cache_min_block_size ? MIN(cache_min_block_size, bsize) : bsize;

	mod->latency = config_read_int(cache_config, section, "Latency", 0);
	mod->block_size = config_read_int(cache_config, section, "BlockSize", 0);
	mod->read_port_count = read_ports;
	mod->write_port_count = write_ports;
	mod->high_net = config_read_ptr(cache_config, buf, "ptr", NULL);
	if (cache_min_block_size < 1)
		fatal("cache block size must be >= 1");
	
	mod->dir_size = config_read_int(cache_config, section, "DirectorySize", 1024);
	mod->dir_assoc = config_read_int(cache_config, section, "DirectoryAssoc", 8);
	if (mod->dir_size & (mod->dir_size - 1))
		fatal("main directory size is not a power of 2");
	if (mod->dir_assoc & (mod->dir_assoc - 1))
		fatal("main directory associativity is not a power of 2");
	mod->dir_num_sets = mod->dir_size / mod->dir_assoc;
	
	/* Calculate default network buffer sizes and bandwidth, based on the
	 * maximum message size (block_size + 8). */
	net_msg_size = 64 + 8;
	net_bandwidth = net_msg_size;  /* One message in one cycle */
	net_buffer_size = net_msg_size * 8;  /* Two messages in a buffer */

	/* Nodes */
	node_array = calloc(cores * threads, sizeof(struct node_t));
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "node ", 5))
			continue;

		core = config_read_int(cache_config, section, "Core", -1);
		thread = config_read_int(cache_config, section, "Thread", -1);
		if (core < 0)
			fatal("%s: section '[ %s ]': invalid or missing value for 'Core'",
				cache_system_config_file_name, section);
		if (thread < 0)
			fatal("%s: section '[ %s ]': invalid or missing value for 'Thread'",
				cache_system_config_file_name, section);
		if (core >= cores)
		{
			warning("%s: section '[ %s ]' ignored.\n%s", cache_system_config_file_name,
				section, err_cachesystem_ignored);
			continue;
		}
		if (thread >= threads)
		{
			warning("%s: section '[ %s ]' ignored.\n%s", cache_system_config_file_name,
				section, err_cachesystem_ignored);
			continue;
		}
		node = &node_array[core * threads + thread];

		/* Instruction cache for node */
		cache_config_key(section, "ICache");
		value = config_read_string(cache_config, section, "ICache", "");
		sprintf(buf, "Cache %s", value);
		cache_config_section(buf);
		node->icache = config_read_ptr(cache_config, buf, "ptr", NULL);
		assert(node->icache);

		/* Data cache for node */
		cache_config_key(section, "DCache");
		value = config_read_string(cache_config, section, "DCache", "");
		sprintf(buf, "Cache %s", value);
		cache_config_section(buf);
		node->dcache = config_read_ptr(cache_config, buf, "ptr", NULL);
		assert(node->dcache);
	}

	/* Check that all nodes have an entry points to the memory hierarchy */
	for (core = 0; core < cores; core++)
	{
		for (thread = 0; thread < threads; thread++)
		{
			node = &node_array[core * threads + thread];
			if (!node->icache)
				fatal("core/thread %d/%d missing ICache.\n%s",
					core, thread, err_cachesystem_icache);
			if (!node->dcache)
				fatal("core/thread %d/%d missing DCache.\n%s",
					core, thread, err_cachesystem_dcache);
		}
	}

	/* Add lower node_array to networks. */
	for (curr = 0; curr < mod_count; curr++)
	{
		mod = mod_array[curr];
		net = mod->high_net;
		if (!net)
			continue;
		if (net->node_count)
			fatal("network '%s' has more than one lower node", net->name);
		mod->high_net_node = net_add_end_node(net, net_buffer_size, net_buffer_size,
			mod->name, mod);
	}

	/* Check that all networks got assigned a lower node. */
	for (i = 0; i < net_count; i++)
	{
		net = net_array[i];
		assert(net->node_count <= 1);
		if (!net->node_count)
			fatal("network '%s' has no lower node leading to main memory", net->name);
	}

	/* Add upper node_array to networks. Update 'next' attributes for array of modules. */
	for (curr = 0; curr < mod_count; curr++)
	{
		struct net_node_t *lower_node;
		struct mod_t *low_mod;

		mod = mod_array[curr];
		net = mod->low_net;
		if (!net)
			continue;

		/* Create node */
		mod->low_net_node = net_add_end_node(net, net_buffer_size,
			net_buffer_size, mod->name, mod);

		/* Associate with lower node */
		lower_node = list_get(net->node_list, 0);
		assert(lower_node && lower_node->user_data);
		low_mod = lower_node->user_data;
		assert(!linked_list_count(mod->low_mod_list));
		linked_list_add(mod->low_mod_list, low_mod);
		linked_list_add(low_mod->high_mod_list, mod);
	}

	/* Check that block sizes are equal or larger while we descend through the
	 * memory hierarchy. */
	for (curr = 0; curr < mod_count; curr++)
	{
		bsize = 0;
		for (mod = mod_array[curr]; mod; )
		{
			if (mod->block_size < bsize)
				fatal("cache %s has a smaller block size than some "
					"of its upper level caches", mod->name);
			bsize = mod->block_size;
			linked_list_head(mod->low_mod_list);
			mod = linked_list_get(mod->low_mod_list);
		}
	}

	/* For each network, add a switch and create node connections.
	 * Then calculate routes between nodes. */
	for (i = 0; i < net_count; i++)
	{
		struct net_node_t *lower_node;
		struct net_node_t *switch_node;
		struct net_node_t *node;

		/* Get lower node */
		net = net_array[i];
		lower_node = list_get(net->node_list, 0);
		assert(lower_node && lower_node->user_data);

		/* Create switch */
		snprintf(buf, sizeof(buf), "%s.sw", net->name);
		switch_node = net_add_switch(net, net_buffer_size, net_buffer_size,
			net_bandwidth, buf);

		/* Create connections */
		for (j = 0; j < net->end_node_count; j++)
		{
			node = list_get(net->node_list, j);
			net_add_bidirectional_link(net, node, switch_node, net_bandwidth);
		}

		/* Routing table */
		net_routing_table_calculate(net->routing_table);
	}

	/* Directories */
	for (curr = 0; curr < mod_count; curr++)
	{
		mod = mod_array[curr];

		/* Main memory */
		if (mod->kind == mod_kind_main_memory)
		{
			mod->dir = dir_create(mod->dir_num_sets, mod->dir_assoc,
				mod->block_size / cache_min_block_size, mod->high_net->node_count);
			mod->cache = cache_create(mod->dir_num_sets, mod->block_size,
				mod->dir_assoc, cache_policy_lru);
			continue;
		}

		/* Level 1 cache */
		if (!mod->high_net)
		{
			mod->dir_size = mod->cache->num_sets * mod->cache->assoc;
			mod->dir_num_sets = mod->cache->num_sets;
			mod->dir_assoc = mod->cache->assoc;
			mod->dir = dir_create(mod->cache->num_sets, mod->cache->assoc,
				mod->block_size / cache_min_block_size, 1);
			continue;
		}

		/* Other level array of modules */
		mod->dir_size = mod->cache->num_sets * mod->cache->assoc;
		mod->dir_num_sets = mod->cache->num_sets;
		mod->dir_assoc = mod->cache->assoc;
		mod->dir = dir_create(mod->cache->num_sets, mod->cache->assoc,
			mod->block_size / cache_min_block_size, mod->high_net->node_count);
	}

	/* Create TLBs (one dtlb and one itlb per thread) */
	section = "Tlb";
	tlb_count = cores * threads * 2;
	tlb_array = calloc(tlb_count, sizeof(void *));
	for (core = 0; core < cores; core++)
	{
		for (thread = 0; thread < threads; thread++)
		{
			dtlb = tlb_array[(core * threads + thread) * 2] = tlb_create();
			itlb = tlb_array[(core * threads + thread) * 2 + 1] = tlb_create();
			sprintf(dtlb->name, "dtlb.%d.%d", core, thread);
			sprintf(itlb->name, "itlb.%d.%d", core, thread);
			dtlb->hitlat = itlb->hitlat =
				config_read_int(cache_config, section, "HitLatency", 2);
			dtlb->misslat = itlb->misslat =
				config_read_int(cache_config, section, "MissLatency", 30);
			nsets = config_read_int(cache_config, section, "Sets", 64);
			assoc = config_read_int(cache_config, section, "Assoc", 4);
			dtlb->cache = cache_create(nsets, mmu_page_size, assoc, cache_policy_lru);
			itlb->cache = cache_create(nsets, mmu_page_size, assoc, cache_policy_lru);
		}
	}

	/* Free configuration file */
	config_free(cache_config);
}


void cache_system_print_stats(FILE *f)
{
	struct mod_t *mod;
	struct tlb_t *tlb;
	int curr;

	fprintf(f, "[ CacheSystemSummary ]\n");

	/* Show hit ratio for each cache */
	for (curr = 0; curr < mod_count; curr++)
	{
		mod = mod_array[curr];
		fprintf(f, "Cache[%s].HitRatio = %.4g\n", mod->name, mod->accesses ?
			(double) mod->hits / mod->accesses : 0.0);
	}

	/* Show hit ratio for each TLB */
	for (curr = 0; curr < tlb_count; curr++)
	{
		tlb = tlb_array[curr];
		fprintf(f, "Cache[%s].HitRatio = %.4g\n", tlb->name, tlb->accesses ?
			(double) tlb->hits / tlb->accesses : 0.0);
	}
	
	fprintf(f, "\n");
}


void cache_system_dump_report()
{
	struct mod_t *mod;
	struct cache_t *cache;
	struct tlb_t *tlb;
	FILE *f;
	int curr;
	int i;

	/* Open file */
	f = open_write(cache_system_report_file_name);
	if (!f)
		return;
	
	/* Intro */
	fprintf(f, "; Report for caches, TLBs, and main memory\n");
	fprintf(f, ";    Accesses - Total number of accesses\n");
	fprintf(f, ";    Hits, Misses - Accesses resulting in hits/misses\n");
	fprintf(f, ";    HitRatio - Hits divided by accesses\n");
	fprintf(f, ";    Evictions - Invalidated or replaced cache blocks\n");
	fprintf(f, ";    Retries - For L1 caches, accesses that were retried\n");
	fprintf(f, ";    ReadRetries, WriteRetries - Read/Write retried accesses\n");
	fprintf(f, ";    NoRetryAccesses - Number of accesses that were not retried\n");
	fprintf(f, ";    NoRetryHits, NoRetryMisses - Hits and misses for not retried accesses\n");
	fprintf(f, ";    NoRetryHitRatio - NoRetryHits divided by NoRetryAccesses\n");
	fprintf(f, ";    NoRetryReads, NoRetryWrites - Not retried reads and writes\n");
	fprintf(f, ";    Reads, Writes - Total read/write accesses\n");
	fprintf(f, ";    BlockingReads, BlockingWrites - Reads/writes coming from lower-level cache\n");
	fprintf(f, ";    NonBlockingReads, NonBlockingWrites - Coming from upper-level cache\n");
	fprintf(f, "\n\n");
	
	/* Report for each cache */
	for (curr = 0; curr < mod_count; curr++)
	{
		mod = mod_array[curr];
		cache = mod->cache;
		fprintf(f, "[ %s ]\n", mod->name);
		fprintf(f, "\n");

		/* Configuration */
		if (cache) {
			fprintf(f, "Sets = %d\n", cache->num_sets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", mod->block_size);
		fprintf(f, "Latency = %d\n", mod->latency);
		fprintf(f, "ReadPorts = %d\n", mod->read_port_count);
		fprintf(f, "WritePorts = %d\n", mod->write_port_count);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", mod->accesses);
		fprintf(f, "Hits = %lld\n", mod->hits);
		fprintf(f, "Misses = %lld\n", mod->accesses - mod->hits);
		fprintf(f, "HitRatio = %.4g\n", mod->accesses ?
			(double) mod->hits / mod->accesses : 0.0);
		fprintf(f, "Evictions = %lld\n", mod->evictions);
		fprintf(f, "Retries = %lld\n", mod->read_retries + mod->write_retries);
		fprintf(f, "ReadRetries = %lld\n", mod->read_retries);
		fprintf(f, "WriteRetries = %lld\n", mod->write_retries);
		fprintf(f, "\n");
		fprintf(f, "NoRetryAccesses = %lld\n", mod->no_retry_accesses);
		fprintf(f, "NoRetryHits = %lld\n", mod->no_retry_hits);
		fprintf(f, "NoRetryMisses = %lld\n", mod->no_retry_accesses -
			mod->no_retry_hits);
		fprintf(f, "NoRetryHitRatio = %.4g\n", mod->no_retry_accesses ?
			(double) mod->no_retry_hits / mod->no_retry_accesses : 0.0);
		fprintf(f, "NoRetryReads = %lld\n", mod->no_retry_reads);
		fprintf(f, "NoRetryReadHits = %lld\n", mod->no_retry_read_hits);
		fprintf(f, "NoRetryReadMisses = %lld\n", mod->no_retry_reads -
			mod->no_retry_read_hits);
		fprintf(f, "NoRetryWrites = %lld\n", mod->no_retry_writes);
		fprintf(f, "NoRetryWriteHits = %lld\n", mod->no_retry_write_hits);
		fprintf(f, "NoRetryWriteMisses = %lld\n", mod->no_retry_writes -
			mod->no_retry_write_hits);
		fprintf(f, "\n");
		fprintf(f, "Reads = %lld\n", mod->reads);
		fprintf(f, "BlockingReads = %lld\n", mod->blocking_reads);
		fprintf(f, "NonBlockingReads = %lld\n", mod->non_blocking_reads);
		fprintf(f, "ReadHits = %lld\n", mod->read_hits);
		fprintf(f, "ReadMisses = %lld\n", mod->reads - mod->read_hits);
		fprintf(f, "\n");
		fprintf(f, "Writes = %lld\n", mod->writes);
		fprintf(f, "BlockingWrites = %lld\n", mod->blocking_writes);
		fprintf(f, "NonBlockingWrites = %lld\n", mod->non_blocking_writes);
		fprintf(f, "WriteHits = %lld\n", mod->write_hits);
		fprintf(f, "WriteMisses = %lld\n", mod->writes - mod->write_hits);
		fprintf(f, "\n\n");
	}

	/* Report for each TLB */
	for (curr = 0; curr < tlb_count; curr++)
	{
		tlb = tlb_array[curr];
		cache = tlb->cache;
		fprintf(f, "[ %s ]\n", tlb->name);
		fprintf(f, "\n");

		/* Configuration */
		fprintf(f, "HitLatency = %d\n", tlb->hitlat);
		fprintf(f, "MissLatency = %d\n", tlb->misslat);
		fprintf(f, "Sets = %d\n", cache->num_sets);
		fprintf(f, "Assoc = %d\n", cache->assoc);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", tlb->accesses);
		fprintf(f, "Hits = %lld\n", tlb->hits);
		fprintf(f, "Misses = %lld\n", tlb->accesses - tlb->hits);
		fprintf(f, "HitRatio = %.4g\n", tlb->accesses ?
			(double) tlb->hits / tlb->accesses : 0.0);
		fprintf(f, "Evictions = %lld\n", tlb->evictions);
		fprintf(f, "\n\n");
	}

	/* Dump report for networks */
	for (i = 0; i < net_count; i++)
		net_dump_report(net_array[i], f);
	
	/* Done */
	fclose(f);
}


void cache_system_done()
{
	int i;

	/* Dump report */
	cache_system_dump_report();

	/* Free array of modules */
	for (i = 0; i < mod_count; i++) {
		__mod_free(mod_array[i]);
	}
	free(mod_array);

	/* Free tlbs */
	for (i = 0; i < tlb_count; i++)
		tlb_free(tlb_array[i]);
	free(tlb_array);

	/* Free networks */
	for (i = 0; i < net_count; i++)
		net_free(net_array[i]);
	free(net_array);

	/* Other */
	free(node_array);
	
	/* Finalizations */
	mmu_done();
}


/* Return the entry point in the cache hierarchy depending on the core-thread pair
 * and the type of block accessed (if data=1, data cache, otherwise, instruction cache) */
static struct mod_t *cache_system_get_mod(int core, int thread, enum cache_kind_t cache_kind)
{
	int index;
	assert(core < cores && thread < threads);
	index = core * threads + thread;
	return cache_kind == cache_kind_data ? node_array[index].dcache : node_array[index].icache;
}


/* Return the associated itlb/dtlb */
static struct tlb_t *cache_system_get_tlb(int core, int thread, enum cache_kind_t cache_kind)
{
	int index;
	assert(core < cores && thread < threads);
	index = (core * threads + thread) * 2 + (cache_kind == cache_kind_data ? 0 : 1);
	return tlb_array[index];
}


static void cache_system_dump_route(int core, int thread, enum cache_kind_t kind, FILE *f)
{
	struct mod_t *mod;

	mod = cache_system_get_mod(core, thread, kind);
	while (mod)
	{
		fprintf(f, "    %s loid=%d\n",
			mod->name, mod->low_net_node->index);
		if (mod->low_net)
			fprintf(f, "    %s\n", mod->low_net->name);
		linked_list_head(mod->low_mod_list);
		mod = linked_list_get(mod->low_mod_list);
	}
}


void cache_system_dump(FILE *f)
{
	int core, thread;
	for (core = 0; core < cores; core++) {
		for (thread = 0; thread < threads; thread++) {
			fprintf(f, "core %d - thread %d\n  data route\n", core, thread);
			cache_system_dump_route(core, thread, cache_kind_data, f);
			fprintf(f, "  instruction route\n");
			cache_system_dump_route(core, thread, cache_kind_inst, f);
		}
	}
}


int cache_system_pending_address(int core, int thread,
	enum cache_kind_t cache_kind, uint32_t addr)
{
	struct mod_t *mod;
	mod = cache_system_get_mod(core, thread, cache_kind);
	return __mod_pending_address(mod, addr);
}


int cache_system_pending_access(int core, int thread,
	enum cache_kind_t cache_kind, uint64_t access)
{
	struct mod_t *mod;
	mod = cache_system_get_mod(core, thread, cache_kind);
	return __mod_pending_access(mod, access);
}


int cache_system_block_size(int core, int thread,
	enum cache_kind_t cache_kind)
{
	struct mod_t *mod;
	mod = cache_system_get_mod(core, thread, cache_kind);
	return mod->block_size;
}


int cache_system_can_access(int core, int thread, enum cache_kind_t cache_kind,
	enum mod_access_kind_t access_kind, uint32_t addr)
{
	struct mod_t *mod;
	struct ccache_access_t *access;

	/* Find cache and an in-flight access to the same address. */
	mod = cache_system_get_mod(core, thread, cache_kind);
	access = __mod_find_access(mod, addr);

	/* If there is no matching access, we just need a free port. */
	if (!access)
		return access_kind == mod_access_kind_read ?
			mod->pending_reads < mod->read_port_count :
			mod->pending_writes < mod->write_port_count;
	
	/* If either the matching or the current access is a write,
	 * concurrency is not allowed. */
	if (access_kind == mod_access_kind_write ||
		access->access_kind == mod_access_kind_write)
		return 0;
	
	/* Both current and matching accesses are loads, so the current
	 * access can get the result of the in-flight one. */
	return 1;
}


static uint64_t cache_system_access(int core, int thread, enum cache_kind_t cache_kind,
	enum mod_access_kind_t access_kind, uint32_t addr,
	struct linked_list_t *eventq, void *eventq_item)
{
	struct cache_system_stack_t *newstack;
	struct mod_t *mod;
	struct ccache_access_t *access, *alias;

	/* Check that the physical address is valid for the MMU, i.e.,
	 * it belongs to an allocated physical page. */
	if (!mmu_valid_phaddr(addr))
		panic("%s: page not allocated", __FUNCTION__);

	/* Record immediately a new access */
	mod = cache_system_get_mod(core, thread, cache_kind);
	alias = __mod_find_access(mod, addr);
	access = __mod_start_access(mod, access_kind, addr,
		eventq, eventq_item);

	/* If there was no alias, start cache access */
	if (!alias)
	{
		newstack = cache_system_stack_create(core, thread, addr,
			ESIM_EV_NONE, NULL);
		newstack->cache_kind = cache_kind;
		newstack->access_kind = access_kind;
		newstack->eventq = eventq;
		newstack->eventq_item = eventq_item;
		esim_schedule_event(EV_CACHE_SYSTEM_ACCESS, newstack, 0);
	}

	/* Return access identifier */
	return access->id;
}


uint64_t cache_system_write(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *eventq_item)
{
	assert(cache_kind == cache_kind_data);
	assert(cache_system_can_access(core, thread, cache_kind,
		mod_access_kind_write, addr));
	return cache_system_access(core, thread, cache_kind, mod_access_kind_write,
		addr, eventq, eventq_item);
}


uint64_t cache_system_read(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *eventq_item)
{
	assert(cache_system_can_access(core, thread, cache_kind,
		mod_access_kind_read, addr));
	return cache_system_access(core, thread, cache_kind, mod_access_kind_read,
		addr, eventq, eventq_item);
}

void cache_system_handler(int event, void *data)
{
	struct cache_system_stack_t *stack = data;

	if (event == EV_CACHE_SYSTEM_ACCESS)
	{
		/* Access to TLB */
		esim_schedule_event(EV_CACHE_SYSTEM_ACCESS_TLB, stack, 0);
		stack->pending++;

		/* Access to cache if not perfect */
		if ((stack->cache_kind == cache_kind_data && !cache_system_dperfect) ||
			(stack->cache_kind == cache_kind_inst && !cache_system_iperfect))
		{
			esim_schedule_event(EV_CACHE_SYSTEM_ACCESS_CACHE, stack, 0);
			stack->pending++;
		}
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_CACHE)
	{
		struct mod_t *mod;
		struct moesi_stack_t *newstack;

		mod = cache_system_get_mod(stack->core, stack->thread,
			stack->cache_kind);
		newstack = moesi_stack_create(moesi_stack_id++, mod, stack->addr,
			EV_CACHE_SYSTEM_ACCESS_FINISH, stack);
		esim_schedule_event(stack->access_kind == mod_access_kind_read ?
			EV_MOESI_LOAD : EV_MOESI_STORE, newstack, 0);
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_TLB)
	{
		struct tlb_t *tlb;
		uint32_t set, way, tag;
		int state, hit;

		/* Access tlb */
		tlb = cache_system_get_tlb(stack->core, stack->thread, stack->cache_kind);
		hit = cache_find_block(tlb->cache, stack->addr, &set, &way, NULL);

		/* Stats */
		tlb->accesses++;
		if (hit)
			tlb->hits++;

		/* On a miss, replace an entry */
		if (!hit) {
			cache_decode_address(tlb->cache, stack->addr, &set, &tag, NULL);
			way = cache_replace_block(tlb->cache, set);
			cache_get_block(tlb->cache, set, way, NULL, &state);
			if (state)
				tlb->evictions++;
			cache_set_block(tlb->cache, set, way, tag, 1);
		}

		/* Schedule finish of access after latency. */
		cache_access_block(tlb->cache, set, way);
		esim_schedule_event(EV_CACHE_SYSTEM_ACCESS_FINISH, stack,
			hit ? tlb->hitlat : tlb->misslat);
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_FINISH)
	{
		struct mod_t *mod;

		stack->pending--;
		if (stack->pending)
			return;

		mod = cache_system_get_mod(stack->core, stack->thread, stack->cache_kind);
		__mod_end_access(mod, stack->addr);
		cache_system_stack_return(stack);
		return;
	}

	/* Not handled event */
	abort();
}

