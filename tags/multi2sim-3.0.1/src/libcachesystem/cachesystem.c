/*
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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


/* Cache system variables */

static int cores = 0;
static int threads = 0;
static int ccache_count = 0;
static int tlb_count = 0;
static int net_count = 0;
static uint64_t access_counter = 0;

struct node_t {
	struct ccache_t *icache;
	struct ccache_t *dcache;
};

static struct node_t *node_array;
static struct ccache_t **ccache_array;
static struct tlb_t **tlb_array;
static struct net_t **net_array;
struct ccache_t *main_memory;  /* last element of ccache_array */




/* Coherent Cache */

static struct repos_t *ccache_access_repos;


struct ccache_t *ccache_create()
{
	struct ccache_t *ccache;
	ccache = calloc(1, sizeof(struct ccache_t));
	ccache->access_list = lnlist_create();
	return ccache;
}


void ccache_free(struct ccache_t *ccache)
{
	/* Stats summary, only hitratio (for full stats, print report) */
	if (ccache->lonet && ccache->accesses)
		fprintf(stderr, "%s.hitratio  %.4f  # Cache hit ratio\n",
			ccache->name, (double) ccache->hits / ccache->accesses);

	/* Free linked list of pending accesses.
	 * Each element is in turn a linked list of access aliases. */
	while (lnlist_count(ccache->access_list)) {
		struct ccache_access_t *a, *n;
		lnlist_head(ccache->access_list);
		for (a = lnlist_get(ccache->access_list); a; a = n) {
			n = a->next;
			repos_free_object(ccache_access_repos, a);
		}
		lnlist_remove(ccache->access_list);
	}
	lnlist_free(ccache->access_list);

	/* Free cache */
	if (ccache->dir)
		dir_free(ccache->dir);
	if (ccache->cache)
		cache_free(ccache->cache);
	free(ccache);
}


/* Look for an in-flight access. If it is found, return the associated
 * ccache_access_t element, and place the linked list pointer into
 * its position within access_list. */
static struct ccache_access_t *ccache_find_access(struct ccache_t *ccache,
	uint32_t addr)
{
	struct ccache_access_t *access;
	addr &= ~(ccache->bsize - 1);
	for (lnlist_head(ccache->access_list); !lnlist_eol(ccache->access_list);
		lnlist_next(ccache->access_list))
	{
		access = lnlist_get(ccache->access_list);
		if (access->address == addr)
			return access;
	}
	return NULL;
}


struct ccache_access_t *ccache_start_access(struct ccache_t *ccache,
	enum cache_access_kind_enum cache_access_kind, uint32_t addr,
	struct lnlist_t *eventq, void *eventq_item)
{
	struct ccache_access_t *access, *alias;

	/* Create access */
	access = repos_create_object(ccache_access_repos);
	access->cache_access_kind = cache_access_kind;
	access->address = addr & ~(ccache->bsize - 1);
	access->eventq = eventq;
	access->eventq_item = eventq_item;

	/* If an alias is found, insert new access at the alias linked list head.
	 * If no alias found, a new entry and id are created. */
	alias = ccache_find_access(ccache, addr);
	if (alias) {
		assert(access->cache_access_kind == cache_access_kind_read);
		assert(alias->cache_access_kind == cache_access_kind_read);
		access->next = alias->next;
		alias->next = access;
		access->id = alias->id;
	} else {
		lnlist_out(ccache->access_list);
		lnlist_insert(ccache->access_list, access);
		access->id = ++access_counter;
		cache_access_kind == cache_access_kind_read ? ccache->pending_reads++
			: ccache->pending_writes++;
		assert(ccache->pending_reads <= ccache->read_ports);
		assert(ccache->pending_writes <= ccache->write_ports);
	}
	return access;
}


void ccache_end_access(struct ccache_t *ccache, uint32_t addr)
{
	struct ccache_access_t *access, *alias;

	/* Find access */
	addr &= ~(ccache->bsize - 1);
	access = ccache_find_access(ccache, addr);
	assert(access && access->address == addr);

	/* Finish actions - insert eventq_item into eventq for all aliases */
	for (alias = access; alias; alias = alias->next) {
		if (alias->eventq) {
			assert(alias->eventq_item);
			lnlist_head(alias->eventq);
			lnlist_insert(alias->eventq, alias->eventq_item);
		}
	}

	/* Free access and all aliases. */
	access->cache_access_kind == cache_access_kind_read ? ccache->pending_reads--
		: ccache->pending_writes--;
	assert(ccache->pending_reads >= 0);
	assert(ccache->pending_writes >= 0);
	while (access) {
		alias = access->next;
		repos_free_object(ccache_access_repos, access);
		access = alias;
	}
	lnlist_remove(ccache->access_list);
}


int ccache_pending_access(struct ccache_t *ccache, uint64_t id)
{
	struct ccache_access_t *access;
	for (lnlist_head(ccache->access_list); !lnlist_eol(ccache->access_list);
		lnlist_next(ccache->access_list))
	{
		access = lnlist_get(ccache->access_list);
		if (access->id == id)
			return 1;
	}
	return 0;
}


int ccache_pending_address(struct ccache_t *ccache, uint32_t addr)
{
	struct ccache_access_t *access;
	access = ccache_find_access(ccache, addr);
	return access != NULL;
}


void ccache_get_block(struct ccache_t *ccache, uint32_t set, uint32_t way,
	uint32_t *ptag, int *pstatus)
{
	/* Main memory */
	if (!ccache->lonet) {
		assert(!way);
		PTR_ASSIGN(ptag, set << ccache->logbsize);
		PTR_ASSIGN(pstatus, moesi_status_exclusive);
		return;
	}
	
	/* Caches */
	cache_get_block(ccache->cache, set, way, ptag, pstatus);
}


/* Return {set, way, tag, status} for an address. */
int ccache_find_block(struct ccache_t *ccache, uint32_t addr,
	uint32_t *pset, uint32_t *pway, uint32_t *ptag, int *pstatus)
{
	struct cache_t *cache = ccache->cache;
	struct cache_blk_t *blk;
	struct dir_lock_t *dir_lock;
	uint32_t set, way, tag;

	/* Main memory */
	if (!ccache->lonet) {
		PTR_ASSIGN(pset, addr >> ccache->logbsize);
		PTR_ASSIGN(pway, 0);
		PTR_ASSIGN(ptag, addr & ~(ccache->bsize - 1));
		PTR_ASSIGN(pstatus, moesi_status_exclusive);
		return 1;
	}
	
	/* Cache. A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->bmask;
	set = (tag >> cache->logbsize) % cache->nsets;
	for (way = 0; way < cache->assoc; way++) {
		blk = &cache->sets[set].blks[way];
		if (blk->tag == tag && blk->status)
			break;
		if (blk->transient_tag == tag) {
			dir_lock = dir_lock_get(ccache->dir, set, way);
			if (dir_lock->lock)
				break;
		}
	}

	/* Miss */
	if (way == cache->assoc) {
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
	PTR_ASSIGN(pstatus, cache->sets[set].blks[way].status);
	return 1;
}


/* Return directory corresponding to a ccache (either cache or main memory). In
 * the latter case, the physical address is needed to obtain the proper
 * directory. */
struct dir_t *ccache_get_dir(struct ccache_t *ccache, uint32_t phaddr)
{
	struct dir_t *dir;
	dir = ccache->lonet ? ccache->dir : mmu_get_dir(phaddr);
	assert(dir);
	return dir;
}


/* Return the directory entry corresponding to a set, way, and subblock
 * within a ccache. It ccache is main memory, set refers to a physical
 * address divided by the main memory block size. */
struct dir_entry_t *ccache_get_dir_entry(struct ccache_t *ccache,
	uint32_t set, uint32_t way, uint32_t subblk)
{
	struct dir_t *dir;

	/* Get directory first */
	dir = ccache_get_dir(ccache, set << ccache->logbsize);

	/* Main memory */
	if (!ccache->lonet) {
		assert(way == 0);
		assert(subblk < main_memory->bsize / cache_min_block_size);
		set = set % dir->xsize;
		return dir_entry_get(dir, set, 0, subblk);
	}
	
	/* Cache */
	dir = ccache->dir;
	assert(dir);
	assert(set < dir->xsize);
	assert(way < dir->ysize);
	assert(subblk < dir->zsize);
	return dir_entry_get(dir, set, way, subblk);
}


/* Return a directory lock. There is a directory lock per
 * block, and not per directory entry, so the subblock is not passed as
 * parameter. */
struct dir_lock_t *ccache_get_dir_lock(struct ccache_t *ccache,
	uint32_t set, uint32_t way)
{
	struct dir_t *dir;
	
	dir = ccache_get_dir(ccache, set << main_memory->logbsize);

	/* Main memory */
	if (!ccache->lonet) {
		set = set % dir->xsize;
		return dir_lock_get(dir, set, way);
	}

	/* Cache */
	return dir_lock_get(dir, set, way);
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
	/* Print stats */
	if (tlb->accesses)
		fprintf(stderr, "%s.hitratio  %.4g  # TLB hit ratio\n",
			tlb->name, (double) tlb->hits / tlb->accesses);
	
	/* Free */
	if (tlb->cache)
		cache_free(tlb->cache);
	free(tlb);
}




/* Cache System Stack */

struct repos_t *cache_system_stack_repos;


struct cache_system_stack_t *cache_system_stack_create(int core, int thread, uint32_t addr,
	int retevent, void *retstack)
{
	struct cache_system_stack_t *stack;
	stack = repos_create_object(cache_system_stack_repos);
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

	repos_free_object(cache_system_stack_repos, stack);
	esim_schedule_event(retevent, retstack, 0);
}




/* Cache system */

int EV_CACHE_SYSTEM_ACCESS;
int EV_CACHE_SYSTEM_ACCESS_CACHE;
int EV_CACHE_SYSTEM_ACCESS_TLB;
int EV_CACHE_SYSTEM_ACCESS_FINISH;

char *cache_config_file = "";
char *cache_system_report_file = "";
struct config_t *cache_config;
int cache_min_block_size = 0;
int cache_max_block_size = 0;
uint32_t mem_latency = 200;
static int iperfect = 0;
static int dperfect = 0;


void cache_system_reg_options(void)
{
	opt_reg_string("-cacheconfig", "Cache configuration file", &cache_config_file);
	opt_reg_bool("-iperfect", "Perfect instruction cache {t|f}", &iperfect);
	opt_reg_bool("-dperfect", "Perfect data cache {t|f}", &dperfect);

	/* Other options */
	mmu_reg_options();
}


/* Check that a section exists */
static void cache_config_section(char *section)
{
	if (!config_section_exists(cache_config, section))
		fatal("%s: section '%s' not present", cache_config_file, section);
}


/* Check that a key exists in configuration file */
static void cache_config_key(char *section, char *key)
{
	if (!config_key_exists(cache_config, section, key))
		fatal("%s: section '%s': key '%s' not present",
			cache_config_file, section, key);
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
	struct tlb_t *dtlb, *itlb;
	char *section, *value;
	int core, thread, curr;
	int nsets, bsize, assoc;
	int read_ports, write_ports;
	struct net_t *net = NULL;
	struct ccache_t *ccache;
	struct node_t *node;
	char buf[200];
	enum cache_policy_enum policy;
	char *policy_str;

	/* Try to open report file */
	if (cache_system_report_file[0] && !can_open_write(cache_system_report_file))
		fatal("%s: cannot open cache system report file",
			cache_system_report_file);

	/* Initializations */
	cores = _cores;
	threads = _threads;
	mmu_init();
	moesi_init();

	/* Repositories */
	cache_system_stack_repos = repos_create(sizeof(struct cache_system_stack_t),
		"cache_system_stack_repos");
	ccache_access_repos = repos_create(sizeof(struct ccache_access_t),
		"ccache_access_repos");
	
	/* Events */
	EV_CACHE_SYSTEM_ACCESS = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_CACHE = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_TLB = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_FINISH = esim_register_event(cache_system_handler);

	/* Load cache configuration file */
	cache_config = config_create(cache_config_file);
	if (!*cache_config_file)
		cache_config_default();
	else if (!config_load(cache_config))
		fatal("%s: cannot load cache configuration file", cache_config_file);
	
	/* Create array of ccaches and networks. */
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (!strncasecmp(section, "Cache ", 6) || !strcasecmp(section, "MainMemory"))
			ccache_count++;
		else if (!strncasecmp(section, "Net ", 4))
			net_count++;
	}
	if (ccache_count < 1)
		fatal("%s: no cache", cache_config_file);
	/*if (net_count < 1)
		fatal("%s: no network", cache_config_file);*/
	ccache_array = calloc(ccache_count, sizeof(void *));
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
		config_write_ptr(cache_config, section, "ptr", net);
	}
	assert(curr == net_count);

	/* Create ccache_array */
	curr = 0;
	for (section = config_section_first(cache_config); curr < ccache_count && section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "Cache ", 6))
			continue;

		/* Create cache */
		ccache = ccache_array[curr++] = ccache_create();
		config_write_ptr(cache_config, section, "ptr", ccache);
		strcpy(ccache->name, section + 6);
		if (!strcasecmp(ccache->name, "MainMemory"))
			fatal("'%s' is not a valid name for a cache", ccache->name);

		/* High network */
		value = config_read_string(cache_config, section, "HiNet", "");
		sprintf(buf, "net %s", value);
		ccache->hinet = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (!ccache->hinet && *value)
			fatal("%s: network specified in HiNet does not exist", ccache->name);

		/* Low network */
		value = config_read_string(cache_config, section, "LoNet", "");
		sprintf(buf, "net %s", value);
		ccache->lonet = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (!ccache->lonet && *value)
			fatal("%s: network specified in LoNet does not exist", ccache->name);
		if (!*value)
			fatal("%s: cache must be connected to a lower network (use LoNet)", ccache->name);

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
		ccache->lat = config_read_int(cache_config, buf, "Latency", 0);
		ccache->bsize = bsize;
		ccache->logbsize = log_base2(bsize);
		ccache->read_ports = read_ports;
		ccache->write_ports = write_ports;
		ccache->cache = cache_create(nsets, bsize, assoc, policy);
		cache_min_block_size = cache_min_block_size ? MIN(cache_min_block_size, bsize) : bsize;
		cache_max_block_size = cache_max_block_size ? MAX(cache_max_block_size, bsize) : bsize;
		if (bsize > mmu_page_size)
			fatal("%s: cache block size greater than memory page size", ccache->name);
		if (read_ports < 1 || write_ports < 1)
			fatal("%s: number of read/write ports must be at least 1", ccache->name);
	}

	/* Main memory */
	section = "MainMemory";
	ccache = main_memory = ccache_array[curr++] = ccache_create();
	assert(curr == ccache_count);
	strcpy(ccache->name, "mm");
	sprintf(buf, "Net %s", config_read_string(cache_config, section, "HiNet", ""));
	cache_config_section(section);
	cache_config_key(section, "Latency");
	cache_config_key(section, "BlockSize");

	read_ports = config_read_int(cache_config, section, "ReadPorts", 2);
	write_ports = config_read_int(cache_config, section, "WritePorts", 1);
	if (read_ports < 1 || write_ports < 1)
		fatal("%s: number of read/write ports must be at least 1", ccache->name);
	bsize = config_read_int(cache_config, section, "BlockSize", 0);
	if (bsize & (bsize - 1))
		fatal("block size for main memory is not a power of 2");
	if (bsize > mmu_page_size)
		fatal("main memory block size cannot be greater than page size");
	ccache->bsize = bsize;
	ccache->logbsize = log_base2(bsize);
	cache_min_block_size = cache_min_block_size ? MIN(cache_min_block_size, bsize) : bsize;

	ccache->lat = config_read_int(cache_config, section, "Latency", 0);
	ccache->bsize = config_read_int(cache_config, section, "BlockSize", 0);
	ccache->read_ports = read_ports;
	ccache->write_ports = write_ports;
	ccache->hinet = config_read_ptr(cache_config, buf, "ptr", NULL);
	if (cache_min_block_size < 1)
		fatal("cache block size must be >= 1");

	/* Nodes */
	node_array = calloc(cores * threads, sizeof(struct node_t));
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "node ", 5))
			continue;

		core = config_read_int(cache_config, section, "Core", -1);
		thread = config_read_int(cache_config, section, "Thread", -1);
		if (core < 0 || thread < 0)
			fatal("cache config: wrong section '%s'", section);
		if (core >= cores) {
			warning("%s: section '[ %s ]' ignored, since it refers to an unexisting core (core %d); "
				"the number of cores in the current configuration is %d",
				cache_config_file, section, core, cores);
			continue;
		}
		if (thread >= threads) {
			warning("%s: section '[ %s ]' ignored, since it refers to an unexisting thread (thread %d); "
				"the number of threads in the current configuration is %d",
				cache_config_file, section, core, cores);
			continue;
		}
		node = &node_array[core * threads + thread];

		/* Instruction cache for node */
		cache_config_key(section, "ICache");
		value = config_read_string(cache_config, section, "ICache", "");
		if (!strcasecmp(value, "MainMemory")) {
			node->icache = main_memory;
		} else {
			sprintf(buf, "Cache %s", value);
			cache_config_section(buf);
			node->icache = config_read_ptr(cache_config, buf, "ptr", NULL);
			assert(node->icache);
		}

		/* Data cache for node */
		cache_config_key(section, "DCache");
		value = config_read_string(cache_config, section, "DCache", "");
		if (!strcasecmp(value, "MainMemory")) {
			node->dcache = main_memory;
		} else {
			sprintf(buf, "Cache %s", value);
			cache_config_section(buf);
			node->dcache = config_read_ptr(cache_config, buf, "ptr", NULL);
			assert(node->dcache);
		}
	}

	/* Check that all nodes have an entry points to the memory hierarchy */
	for (core = 0; core < cores; core++) {
		for (thread = 0; thread < threads; thread++) {
			node = &node_array[core * threads + thread];
			if (!node->icache)
				fatal("core/thread %d/%d does not have an entry into the memory hierarchy "
					"for fetching instructions (instruction cache); "
					"please write a '[ Node <name> ]' section in the cache configuration file for it, "
					"including an 'ICache = <cache>' entry.",
					core, thread);
			if (!node->dcache)
				fatal("core/thread %d/%d does not have an entry into the memory hierarchy "
					"to read or write data (data cache); "
					"please write a '[ Node <name> ]' section in the cache configuration file for it, "
					"including a 'DCache = <cache>' entry.",
					core, thread);
		}
	}

	/* Add lower node_array to networks. */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];
		net = ccache->hinet;
		if (!net)
			continue;
		if (net->node_count)
			fatal("network '%s' has more than one lower node", net->name);
		net_new_node(net, ccache->name, ccache);
	}

	/* Add upper node_array to networks. Update 'next' attributes for ccache_array. */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];
		net = ccache->lonet;
		if (!net)
			continue;
		ccache->loid = net_new_node(net, ccache->name, ccache);
		ccache->next = net_get_node_data(net, 0);
	}

	/* Check that block sizes are equal or larger while we descend through the
	 * memory hierarchy. */
	for (curr = 0; curr < ccache_count; curr++) {
		bsize = 0;
		for (ccache = ccache_array[curr]; ccache; ccache = ccache->next) {
			if (ccache->bsize < bsize)
				fatal("cache %s has a smaller block size than some "
					"of its upper level caches", ccache->name);
			bsize = ccache->bsize;
		}
	}

	/* For each network, add a switch and create node connections.
	 * Then calculate routes between nodes. */
	for (i = 0; i < net_count; i++) {
		int maxmsg, sw;
		int bandwidth;

		/* Get the maximum message size for network, which is equals to
		 * the block size of the lower cache plus 8 (moesi msg) */
		net = net_array[i];
		ccache = net_get_node_data(net, 0);
		if (!ccache)
			continue;
		maxmsg = ccache->bsize + 8;

		/* Create switch and connections. By default, each i/o buffer has
		 * space for two maximum-length messages, and 8 bytes/cycle bandwidth. */
		bandwidth = 8;  /* Bandwidth for links and switch xbar. */
		snprintf(buf, sizeof(buf), "%s.sw", net->name);
		sw = net_new_switch(net, net->end_node_count, maxmsg * 2,
			net->end_node_count, maxmsg * 2, bandwidth, buf, NULL);
		for (j = 0; j < net->end_node_count; j++)
			net_new_bidirectional_link(net, j, sw, bandwidth);

		net_calculate_routes(net);
	}

	/* Directories */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];

		/* Main memory */
		if (!ccache->lonet)
			continue;

		/* Level 1 cache */
		if (!ccache->hinet) {
			ccache->dir = dir_create(ccache->cache->nsets, ccache->cache->assoc,
				ccache->bsize / cache_min_block_size, 1);
			continue;
		}

		/* Other level ccache_array */
		ccache->dir = dir_create(ccache->cache->nsets, ccache->cache->assoc,
			ccache->bsize / cache_min_block_size, ccache->hinet->end_node_count);
	}

	/* Create TLBs (one dtlb and one itlb per thread) */
	section = "Tlb";
	tlb_count = cores * threads * 2;
	tlb_array = calloc(tlb_count, sizeof(void *));
	for (core = 0; core < cores; core++) {
		for (thread = 0; thread < threads; thread++) {
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
}


void cache_system_dump_report()
{
	struct ccache_t *ccache;
	struct tlb_t *tlb;
	FILE *f;
	int curr;

	/* Open file */
	f = open_write(cache_system_report_file);
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
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];
		fprintf(f, "[ %s ]\n\n", ccache->name);
		fprintf(f, "Accesses = %lld\n", (long long) ccache->accesses);
		fprintf(f, "Hits = %lld\n", (long long) ccache->hits);
		fprintf(f, "Misses = %lld\n", (long long) (ccache->accesses - ccache->hits));
		fprintf(f, "HitRatio = %.4g\n", ccache->accesses ?
			(double) ccache->hits / ccache->accesses : 0.0);
		fprintf(f, "Evictions = %lld\n", (long long) ccache->evictions);
		fprintf(f, "Retries = %lld\n", (long long) (ccache->read_retries + ccache->write_retries));
		fprintf(f, "ReadRetries = %lld\n", (long long) ccache->read_retries);
		fprintf(f, "WriteRetries = %lld\n", (long long) ccache->write_retries);
		fprintf(f, "\n");
		fprintf(f, "NoRetryAccesses = %lld\n", (long long) ccache->no_retry_accesses);
		fprintf(f, "NoRetryHits = %lld\n", (long long) ccache->no_retry_hits);
		fprintf(f, "NoRetryMisses = %lld\n", (long long) (ccache->no_retry_accesses -
			ccache->no_retry_hits));
		fprintf(f, "NoRetryHitRatio = %.4g\n", ccache->no_retry_accesses ?
			(double) ccache->no_retry_hits / ccache->no_retry_accesses : 0.0);
		fprintf(f, "NoRetryReads = %lld\n", (long long) ccache->no_retry_reads);
		fprintf(f, "NoRetryReadHits = %lld\n", (long long) ccache->no_retry_read_hits);
		fprintf(f, "NoRetryReadMisses = %lld\n", (long long) (ccache->no_retry_reads -
			ccache->no_retry_read_hits));
		fprintf(f, "NoRetryWrites = %lld\n", (long long) ccache->no_retry_writes);
		fprintf(f, "NoRetryWriteHits = %lld\n", (long long) ccache->no_retry_write_hits);
		fprintf(f, "NoRetryWriteMisses = %lld\n", (long long) (ccache->no_retry_writes -
			ccache->no_retry_write_hits));
		fprintf(f, "\n");
		fprintf(f, "Reads = %lld\n", (long long) ccache->reads);
		fprintf(f, "BlockingReads = %lld\n", (long long) ccache->blocking_reads);
		fprintf(f, "NonBlockingReads = %lld\n", (long long) ccache->non_blocking_reads);
		fprintf(f, "ReadHits = %lld\n", (long long) ccache->read_hits);
		fprintf(f, "ReadMisses = %lld\n", (long long) (ccache->reads - ccache->read_hits));
		fprintf(f, "\n");
		fprintf(f, "Writes = %lld\n", (long long) ccache->writes);
		fprintf(f, "BlockingWrites = %lld\n", (long long) ccache->blocking_writes);
		fprintf(f, "NonBlockingWrites = %lld\n", (long long) ccache->non_blocking_writes);
		fprintf(f, "WriteHits = %lld\n", (long long) ccache->write_hits);
		fprintf(f, "WriteMisses = %lld\n", (long long) (ccache->writes - ccache->write_hits));
		fprintf(f, "\n\n");
	}

	/* Report for each TLB */
	for (curr = 0; curr < tlb_count; curr++) {
		tlb = tlb_array[curr];
		fprintf(f, "[ %s ]\n\n", tlb->name);
		fprintf(f, "Accesses = %lld\n", (long long) tlb->accesses);
		fprintf(f, "Hits = %lld\n", (long long) tlb->hits);
		fprintf(f, "Misses = %lld\n", (long long) (tlb->accesses - tlb->hits));
		fprintf(f, "HitRatio = %.4g\n", tlb->accesses ?
			(double) tlb->hits / tlb->accesses : 0.0);
		fprintf(f, "Evictions = %lld\n", (long long) tlb->evictions);
		fprintf(f, "\n\n");
	}

	/* Done */
	fclose(f);
}


void cache_system_done()
{
	int i;

	/* Dump report */
	cache_system_dump_report();

	/* Free ccache_array */
	for (i = 0; i < ccache_count; i++)
		ccache_free(ccache_array[i]);
	free(ccache_array);

	/* Free tlbs */
	for (i = 0; i < cores * threads; i++)
		tlb_free(tlb_array[i]);
	free(tlb_array);

	/* Free networks */
	for (i = 0; i < net_count; i++)
		net_free(net_array[i]);
	free(net_array);

	/* Other */
	free(node_array);
	repos_free(cache_system_stack_repos);
	repos_free(ccache_access_repos);
	
	/* Finalizations */
	moesi_done();
	mmu_done();
}


/* Return the entry point in the cache hierarchy depending on the core-thread pair
 * and the type of block accessed (if data=1, data cache, otherwise, instruction cache) */
static struct ccache_t *cache_system_get_ccache(int core, int thread, enum cache_kind_enum cache_kind)
{
	int index;
	assert(core < cores && thread < threads);
	index = core * threads + thread;
	return cache_kind == cache_kind_data ? node_array[index].dcache : node_array[index].icache;
}


/* Return the associated itlb/dtlb */
static struct tlb_t *cache_system_get_tlb(int core, int thread, enum cache_kind_enum cache_kind)
{
	int index;
	assert(core < cores && thread < threads);
	index = (core * threads + thread) * 2 + (cache_kind == cache_kind_data ? 0 : 1);
	return tlb_array[index];
}


static void cache_system_dump_route(int core, int thread, enum cache_kind_enum kind, FILE *f)
{
	struct ccache_t *ccache;

	ccache = cache_system_get_ccache(core, thread, kind);
	while (ccache) {
		fprintf(f, "    %s loid=%d\n",
			ccache->name, ccache->loid);
		if (ccache->lonet)
			fprintf(f, "    %s\n", ccache->lonet->name);
		ccache = ccache->next;
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
	enum cache_kind_enum cache_kind, uint32_t addr)
{
	struct ccache_t *ccache;
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	return ccache_pending_address(ccache, addr);
}


int cache_system_pending_access(int core, int thread,
	enum cache_kind_enum cache_kind, uint64_t access)
{
	struct ccache_t *ccache;
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	return ccache_pending_access(ccache, access);
}


int cache_system_block_size(int core, int thread,
	enum cache_kind_enum cache_kind)
{
	struct ccache_t *ccache;
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	return ccache->bsize;
}


int cache_system_can_access(int core, int thread, enum cache_kind_enum cache_kind,
	enum cache_access_kind_enum cache_access_kind, uint32_t addr)
{
	struct ccache_t *ccache;
	struct ccache_access_t *access;

	/* Find cache and an in-flight access to the same address. */
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	access = ccache_find_access(ccache, addr);

	/* If there is no matching access, we just need a free port. */
	if (!access)
		return cache_access_kind == cache_access_kind_read ?
			ccache->pending_reads < ccache->read_ports :
			ccache->pending_writes < ccache->write_ports;
	
	/* If either the matching or the current access is a write,
	 * concurrency is not allowed. */
	if (cache_access_kind == cache_access_kind_write ||
		access->cache_access_kind == cache_access_kind_write)
		return 0;
	
	/* Both current and matching accesses are loads, so the current
	 * access can get the result of the in-flight one. */
	return 1;
}


static uint64_t cache_system_access(int core, int thread, enum cache_kind_enum cache_kind,
	enum cache_access_kind_enum cache_access_kind, uint32_t addr,
	struct lnlist_t *eventq, void *eventq_item)
{
	struct cache_system_stack_t *newstack;
	struct ccache_t *ccache;
	struct ccache_access_t *access, *alias;

	/* Check that the physical address is valid for the MMU, i.e.,
	 * it belongs to an allocated physical page. */
	if (!mmu_valid_phaddr(addr)) {
		fprintf(stderr, "fatal: cache_system_access: not allocated physical page\n");
		abort();
	}

	/* Record immediately a new access */
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	alias = ccache_find_access(ccache, addr);
	access = ccache_start_access(ccache, cache_access_kind, addr,
		eventq, eventq_item);

	/* If there was no alias, start cache access */
	if (!alias) {
		newstack = cache_system_stack_create(core, thread, addr,
			ESIM_EV_NONE, NULL);
		newstack->cache_kind = cache_kind;
		newstack->cache_access_kind = cache_access_kind;
		newstack->eventq = eventq;
		newstack->eventq_item = eventq_item;
		esim_schedule_event(EV_CACHE_SYSTEM_ACCESS, newstack, 0);
	}

	/* Return access identifier */
	return access->id;
}


uint64_t cache_system_write(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, struct lnlist_t *eventq, void *eventq_item)
{
	assert(cache_kind == cache_kind_data);
	assert(cache_system_can_access(core, thread, cache_kind,
		cache_access_kind_write, addr));
	return cache_system_access(core, thread, cache_kind, cache_access_kind_write,
		addr, eventq, eventq_item);
}


uint64_t cache_system_read(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, struct lnlist_t *eventq, void *eventq_item)
{
	assert(cache_system_can_access(core, thread, cache_kind,
		cache_access_kind_read, addr));
	return cache_system_access(core, thread, cache_kind, cache_access_kind_read,
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
		if ((stack->cache_kind == cache_kind_data && !dperfect) ||
			(stack->cache_kind == cache_kind_inst && !iperfect))
		{
			esim_schedule_event(EV_CACHE_SYSTEM_ACCESS_CACHE, stack, 0);
			stack->pending++;
		}
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_CACHE)
	{
		struct ccache_t *ccache;
		struct moesi_stack_t *newstack;

		ccache = cache_system_get_ccache(stack->core, stack->thread,
			stack->cache_kind);
		newstack = moesi_stack_create(moesi_stack_id++, ccache, stack->addr,
			EV_CACHE_SYSTEM_ACCESS_FINISH, stack);
		esim_schedule_event(stack->cache_access_kind == cache_access_kind_read ?
			EV_MOESI_LOAD : EV_MOESI_STORE, newstack, 0);
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_TLB) {
		struct tlb_t *tlb;
		uint32_t set, way, tag;
		int status, hit;

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
			cache_get_block(tlb->cache, set, way, NULL, &status);
			if (status)
				tlb->evictions++;
			cache_set_block(tlb->cache, set, way, tag, 1);
		}

		/* Schedule finish of access after latency. */
		cache_access_block(tlb->cache, set, way);
		esim_schedule_event(EV_CACHE_SYSTEM_ACCESS_FINISH, stack,
			hit ? tlb->hitlat : tlb->misslat);
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_FINISH) {
		struct ccache_t *ccache;

		stack->pending--;
		if (stack->pending)
			return;

		ccache = cache_system_get_ccache(stack->core, stack->thread, stack->cache_kind);
		ccache_end_access(ccache, stack->addr);
		cache_system_stack_return(stack);
		return;
	}

	/* Not handled event */
	abort();
}

