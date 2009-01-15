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

static int num_cores = 0;
static int num_threads = 0;
static int node_count = 0;  /* num_cores * num_threads */
static int ccache_count = 0;
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

struct ccache_t *ccache_create()
{
	struct ccache_t *ccache;
	ccache = calloc(1, sizeof(struct ccache_t));
	ccache->pending_size = 8;
	ccache->pending = calloc(ccache->pending_size, sizeof(struct ccache_access_t));
	return ccache;
}


void ccache_free(struct ccache_t *ccache)
{
	/* Print stats */
	fprintf(stderr, "%s.accesses  %lld  # Number of accesses to the cache\n",
		ccache->name, (long long) ccache->accesses);
	if (ccache->lonet) {
		fprintf(stderr, "%s.hitratio  %.4f  # Cache hit ratio\n",
			ccache->name, ccache->accesses ? (double) ccache->hits /
			ccache->accesses : 0.0);
	}

	/* Free cache */
	if (ccache->dir)
		dir_free(ccache->dir);
	if (ccache->cache)
		cache_free(ccache->cache);
	free(ccache->pending);
	free(ccache);
}


uint64_t ccache_start_access(struct ccache_t *ccache, uint32_t addr)
{
	int i;

	/* Grow list */
	if (ccache->pending_count >= 1000) /* safety */
		panic("libcachesystem: we should never have more than 1000 concurrent accesses; "
			"if you think we should, modify this panic message.");
	if (ccache->pending_count == ccache->pending_size) {
		ccache->pending_size += ccache->pending_size;
		ccache->pending = realloc(ccache->pending, ccache->pending_size * sizeof(struct ccache_access_t));
		assert(ccache->pending);
		memset(ccache->pending + ccache->pending_count, 0, ccache->pending_count * sizeof(struct ccache_access_t));
	}

	/* Insert new access */
	addr &= ~(cache_block_size - 1);
	for (i = 0; i < ccache->pending_size; i++)
		if (!ccache->pending[i].valid)
			break;
	assert(i < ccache->pending_size);
	ccache->pending[i].address = addr;
	ccache->pending[i].access = ++access_counter;
	ccache->pending[i].valid = 1;
	ccache->pending_count++;
	return ccache->pending[i].access;
}


void ccache_end_access(struct ccache_t *ccache, uint32_t addr)
{
	int i;
	addr &= ~(cache_block_size - 1);
	assert(ccache->pending_count > 0);
	for (i = 0; i < ccache->pending_size; i++)
		if (ccache->pending[i].valid && ccache->pending[i].address == addr)
			break;
	assert(i < ccache->pending_size);
	ccache->pending[i].valid = 0;
	ccache->pending_count--;
}


int ccache_pending_access(struct ccache_t *ccache, uint64_t access)
{
	int i;
	for (i = 0; i < ccache->pending_size; i++)
		if (ccache->pending[i].valid && ccache->pending[i].access == access)
			return 1;
	return 0;
}


int ccache_pending_address(struct ccache_t *ccache, uint32_t addr)
{
	int i;
	addr &= ~(cache_block_size - 1);
	for (i = 0; i < ccache->pending_size; i++)
		if (ccache->pending[i].valid && ccache->pending[i].address == addr)
			return 1;
	return 0;
}


void ccache_get_block(struct ccache_t *ccache, uint32_t set, uint32_t way,
	uint32_t *ptag, int *pstatus,
	struct dir_t **pdir, struct dir_entry_t **pdir_entry)
{
	/* Main memory */
	if (!ccache->lonet) {
		assert(!way);
		if (ptag)
			*ptag = set << cache_log_block_size;
		if (pstatus)
			*pstatus = moesi_status_exclusive;
		mmu_get_dir_entry(set << cache_log_block_size, pdir, pdir_entry);
		return;
	}
	
	/* Caches */
	cache_get_block(ccache->cache, set, way, ptag, pstatus);
	if (pdir)
		*pdir = ccache->dir;
	if (pdir_entry)
		*pdir_entry = dir_entry_get(ccache->dir, set, way);
}


/* Return {set, way, dir, dir_entry} for an address.
 * If the block is not in the cache, return NULL in both
 * fields. For main memory, access the corresponding page in the MMU. A null return
 * value in this case means that 'addr' is not a valid physical address. */
int ccache_find_block(struct ccache_t *ccache, uint32_t tag,
	uint32_t *pset, uint32_t *pway, int *pstatus,
	struct dir_t **pdir, struct dir_entry_t **pdir_entry)
{
	struct cache_t *cache = ccache->cache;
	struct cache_blk_t *blk;
	struct dir_entry_t *dir_entry;
	uint32_t set, way;

	/* Main memory */
	if (!ccache->lonet) {
		mmu_get_dir_entry(tag, pdir, pdir_entry);
		if (pset)
			*pset = tag >> cache_log_block_size;
		if (pway)
			*pway = 1;
		if (pstatus)
			*pstatus = moesi_status_exclusive;
		return 1;
	}
	
	/* Cache. A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag &= ~cache->bmask;
	set = (tag >> cache->logbsize) % cache->nsets;
	for (way = 0; way < cache->assoc; way++) {
		blk = &cache->sets[set].blks[way];
		dir_entry = dir_entry_get(ccache->dir, set, way);
		if (blk->tag == tag && blk->status)
			break;
		if (blk->transient_tag == tag && dir_entry->lock)
			break;
	}

	/* Miss */
	if (way == cache->assoc)
		return 0;
	
	/* Hit */
	if (pset)
		*pset = set;
	if (pway)
		*pway = way;
	if (pdir)
		*pdir = ccache->dir;
	if (pdir_entry)
		*pdir_entry = dir_entry_get(ccache->dir, set, way);
	if (pstatus)
		*pstatus = cache->sets[set].blks[way].status;
	return 1;
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
	fprintf(stderr, "%s.accesses  %lld  # Number of accesses to the tlb\n",
		tlb->name, (long long) tlb->accesses);
	fprintf(stderr, "%s.hitratio  %.4f  # Cache hit ratio\n",
		tlb->name, tlb->accesses ? (double) tlb->hits /
		tlb->accesses : 0.0);
	
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
struct config_t *cache_config;
uint32_t cache_block_size;
uint32_t cache_log_block_size;
uint32_t mem_latency = 200;
static uint32_t iports = 8;
static int iperfect = 0;
static uint32_t dports = 8;
static int dperfect = 0;


void cache_system_reg_options(void)
{
	opt_reg_string("-cacheconfig", "Cache configuration file", &cache_config_file);
	opt_reg_uint32("-iports", "Ports of level 1 instruction caches", &iports);
	opt_reg_uint32("-dports", "Ports of level 1 data caches", &dports);
	opt_reg_bool("-iperfect", "Perfect instruction cache {t|f}", &iperfect);
	opt_reg_bool("-dperfect", "Perfect data cache {t|f}", &dperfect);
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
static void cache_config_default(int def_cores, int def_threads)
{
	char section[100], buf[100];
	int core, thread, i;
	int def_nodes = def_cores * def_threads;

	/* Write cache topologies */
	SECTION("CacheTopology L1");
	KEY_INT("Sets", 256);
	KEY_INT("Assoc", 2);
	KEY_INT("Blocksize", 64);
	KEY_INT("Latency", 2);

	SECTION("CacheTopology L2");
	KEY_INT("Sets", 1024);
	KEY_INT("Assoc", 8);
	KEY_INT("Blocksize", 64);
	KEY_INT("Latency", 20);

	/* Main memory */
	SECTION("MainMemory");
	sprintf(buf, "net-%d", def_cores);
	KEY_STRING("HiNet", buf);
	KEY_INT("Latency", 200);

	/* Tlb */
	SECTION("Tlb");
	KEY_INT("Sets", 64);
	KEY_INT("Assoc", 4);
	KEY_INT("HitLatency", 2);
	KEY_INT("MissLatency", 2);

	/* Nodes */
	for (core = 0; core < def_cores; core++) {
		for (thread = 0; thread < def_threads; thread++) {
			sprintf(buf, "Node %d.%d", core, thread);
			SECTION(buf);
			KEY_INT("Core", core);
			KEY_INT("Thread", thread);
			sprintf(buf, "il1-%d", core * def_threads + thread);
			KEY_STRING("ICache", buf);
			sprintf(buf, "dl1-%d", core * def_threads + thread);
			KEY_STRING("DCache", buf);
		}
	}

	/* Level 1 Caches */
	for (i = 0; i < def_nodes; i++) {
		sprintf(buf, "Cache dl1-%d", i);
		SECTION(buf);
		KEY_STRING("Topology", "L1");
		KEY_STRING("HiNet", "");
		sprintf(buf, "net-%d", i / def_threads);
		KEY_STRING("LoNet", buf);

		sprintf(buf, "Cache il1-%d", i);
		SECTION(buf);
		KEY_STRING("Topology", "L1");
		KEY_STRING("HiNet", "");
		sprintf(buf, "net-%d", i / def_threads);
		KEY_STRING("LoNet", buf);
	}

	/* Level 2 Caches */
	for (core = 0; core < def_cores; core++) {
		sprintf(buf, "Cache l2-%d", core);
		SECTION(buf);
		KEY_STRING("Topology", "L2");
		sprintf(buf, "net-%d", core);
		KEY_STRING("HiNet", buf);
		sprintf(buf, "net-%d", def_cores);
		KEY_STRING("LoNet", buf);
	}

	/* Interconnects */
	for (core = 0; core <= def_cores; core++) {
		sprintf(buf, "Net net-%d", core);
		SECTION(buf);
		KEY_STRING("Topology", "FatTree");
		KEY_INT("LinkWidth", 32);
	}
}
#undef KEY_INT
#undef KEY_STRING
#undef SECTION


void cache_system_init(int def_cores, int def_threads)
{
	int i;
	struct tlb_t *tlb;
	char *section;
	int core, thread, curr;
	int nsets, bsize, assoc;
	struct net_t *net = NULL;
	struct ccache_t *ccache;
	struct node_t *node;
	char buf[200];

	/* Initializations */
	mmu_init();
	moesi_init();

	/* Repositories */
	cache_system_stack_repos = repos_create(sizeof(struct cache_system_stack_t),
		"cache_system_stack_repos");
	
	/* Events */
	EV_CACHE_SYSTEM_ACCESS = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_CACHE = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_TLB = esim_register_event(cache_system_handler);
	EV_CACHE_SYSTEM_ACCESS_FINISH = esim_register_event(cache_system_handler);

	/* Load cache configuration file */
	cache_config = config_create(cache_config_file);
	if (!*cache_config_file)
		cache_config_default(def_cores, def_threads);
	else if (!config_load(cache_config))
		fatal("%s: cannot load cache configuration file", cache_config_file);
	
	/* Explore sections for first time to detect number of cores/threads,
	 * number of ccache_array, and number of interconnects. */
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (!strncasecmp(section, "Cache ", 6) || !strcasecmp(section, "MainMemory"))
			ccache_count++;
		else if (!strncasecmp(section, "Net ", 4))
			net_count++;
		else if (!strncasecmp(section, "Node ", 5)) {
			cache_config_key(section, "Core");
			cache_config_key(section, "Thread");
			core = config_read_int(cache_config, section, "Core", 0);
			thread = config_read_int(cache_config, section, "Thread", 0);
			num_cores = MAX(num_cores, core + 1);
			num_threads = MAX(num_threads, thread + 1);
		}
	}
	node_count = num_cores * num_threads;

	/* Create arrays */
	if (num_cores < 1 || num_threads < 1)
		fatal("cache config file: wrong number of cores/threads");
	node_array = calloc(node_count, sizeof(struct node_t));
	ccache_array = calloc(ccache_count, sizeof(void *));
	net_array = calloc(net_count, sizeof(void *));

	/* Create networks */
	curr = 0;
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "Net ", 4))
			continue;

		cache_config_key(section, "Topology");
		cache_config_key(section, "LinkWidth");
		strcpy(buf, config_read_string(cache_config, section, "Topology", ""));
		if (!strcasecmp(buf, "Bus"))
			net = net_create_bus();
		else if (!strcasecmp(buf, "FatTree"))
			net = net_create();
		else
			fatal("bad network topology in section '%s'", section);
		net->link_width = config_read_int(cache_config, section, "LinkWidth", 8);
		net_array[curr++] = net;
		config_write_ptr(cache_config, section, "ptr", net);
		strcpy(net->name, section + 4);
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

		/* High network */
		sprintf(buf, "net %s", config_read_string(cache_config, section, "HiNet", ""));
		ccache->hinet = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (ccache->hinet)
			ccache->hinet->max_nodes++;

		/* Low network */
		sprintf(buf, "net %s", config_read_string(cache_config, section, "LoNet", ""));
		ccache->lonet = config_read_ptr(cache_config, buf, "ptr", NULL);
		if (ccache->lonet)
			ccache->lonet->max_nodes++;

		/* Cache parameters */
		sprintf(buf, "CacheTopology %s", config_read_string(cache_config, section, "Topology", ""));
		cache_config_key(section, "Topology");
		cache_config_section(buf);
		cache_config_key(buf, "Latency");
		cache_config_key(buf, "Sets");
		cache_config_key(buf, "Assoc");
		cache_config_key(buf, "BlockSize");
		ccache->lat = config_read_int(cache_config, buf, "Latency", 0);
		nsets = config_read_int(cache_config, buf, "Sets", 0);
		assoc = config_read_int(cache_config, buf, "Assoc", 0);
		bsize = config_read_int(cache_config, buf, "BlockSize", 0);
		ccache->cache = cache_create(nsets, bsize, assoc);

		/* Block size */
		if (cache_block_size && cache_block_size != bsize)
			fatal("block size of all caches must be the same");
		if (bsize > MMU_PAGE_SIZE)
			fatal("cache block size cannot be greater than the page size (%d bytes)",
				MMU_PAGE_SIZE);
		cache_block_size = bsize;
	}

	/* Main memory */
	section = "MainMemory";
	ccache = main_memory = ccache_array[curr++] = ccache_create();
	assert(curr == ccache_count);
	strcpy(ccache->name, "mm");
	sprintf(buf, "Net %s", config_read_string(cache_config, section, "HiNet", ""));
	cache_config_section(section);
	cache_config_key(section, "HiNet");
	cache_config_key(section, "Latency");
	cache_config_section(buf);
	ccache->lat = config_read_int(cache_config, section, "Latency", 0);
	ccache->hinet = config_read_ptr(cache_config, buf, "ptr", NULL);
	if (ccache->hinet)
		ccache->hinet->max_nodes++;

	/* Nodes */
	for (section = config_section_first(cache_config); section;
		section = config_section_next(cache_config))
	{
		if (strncasecmp(section, "node ", 5))
			continue;

		core = config_read_int(cache_config, section, "Core", -1);
		thread = config_read_int(cache_config, section, "Thread", -1);
		if (core < 0 || thread < 0)
			fatal("cache config: wrong section '%s'", section);
		node = &node_array[core * num_threads + thread];

		/* Instruction cache for node */
		sprintf(buf, "Cache %s", config_read_string(cache_config, section, "ICache", ""));
		cache_config_key(section, "ICache");
		cache_config_section(buf);
		node->icache = config_read_ptr(cache_config, buf, "ptr", NULL);
		assert(node->icache);

		/* Data cache for node */
		sprintf(buf, "Cache %s", config_read_string(cache_config, section, "DCache", ""));
		cache_config_key(section, "DCache");
		cache_config_section(buf);
		node->dcache = config_read_ptr(cache_config, buf, "ptr", NULL);
		assert(node->dcache);
	}

	/* Initialize node_array of networks */
	for (curr = 0; curr < net_count; curr++) {
		net = net_array[curr];
		net_init_nodes(net, net->max_nodes);
	}

	/* Add lower node_array to networks. */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];
		net = ccache->hinet;
		if (!net)
			continue;
		if (net->num_nodes)
			fatal("network '%s' has more than one lower node", net->name);
		net_add_node(net, net_node_kind_main, ccache);
	}

	/* Add upper node_array to networks. Update 'next' attributes for ccache_array.
	 * If the network is a fat tree, add connection. */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];
		net = ccache->lonet;
		if (!net)
			continue;
		ccache->loid = net_add_node(net, net_node_kind_cache, ccache);
		ccache->next = net_get_node(net, 0);
		if (!net->bus)
			net_add_con(net, 0, ccache->loid);
	}

	/* Calculate routes for network nodes */
	for (i = 0; i < net_count; i++) {
		net = net_array[i];
		net_calculate_routes(net);
	}

	/* Directories */
	for (curr = 0; curr < ccache_count; curr++) {
		ccache = ccache_array[curr];

		/* Level 1 cache */
		if (!ccache->hinet) {
			ccache->dir = dir_create(ccache->cache->nsets, ccache->cache->assoc, 1);
			continue;
		}

		/* Main memory */
		if (!ccache->lonet)
			continue;

		/* Other level ccache_array */
		ccache->dir = dir_create(ccache->cache->nsets, ccache->cache->assoc, ccache->hinet->num_nodes);
	}

	/* Check configuration */
	if (cache_block_size < 1 || (cache_block_size & (cache_block_size - 1)))
		fatal("cache block size must be a power of two and >= 1");
	cache_log_block_size = cache_log2(cache_block_size);

	/* Create tlbs */
	section = "Tlb";
	cache_config_section(section);
	cache_config_key(section, "HitLatency");
	cache_config_key(section, "MissLatency");
	cache_config_key(section, "Sets");
	cache_config_key(section, "Assoc");
	tlb_array = calloc(node_count, sizeof(void *));
	for (i = 0; i < node_count; i++) {
		tlb = tlb_array[i] = tlb_create();
		sprintf(tlb->name, "tlb.%d.%d", i / num_threads, i % num_threads);
		tlb->hitlat = config_read_int(cache_config, section, "HitLatency", 0);
		tlb->misslat = config_read_int(cache_config, section, "MissLatency", 0);
		nsets = config_read_int(cache_config, section, "Sets", 0);
		assoc = config_read_int(cache_config, section, "Assoc", 0);
		tlb->cache = cache_create(nsets, MMU_PAGE_SIZE, assoc);
	}
}


void cache_system_done()
{
	int i;

	/* Free ccache_array */
	for (i = 0; i < ccache_count; i++)
		ccache_free(ccache_array[i]);
	free(ccache_array);

	/* Free tlbs */
	for (i = 0; i < node_count; i++)
		tlb_free(tlb_array[i]);
	free(tlb_array);

	/* Free networks */
	for (i = 0; i < net_count; i++)
		net_free(net_array[i]);
	free(net_array);

	/* Other */
	free(node_array);
	repos_free(cache_system_stack_repos);
	
	/* Finalizations */
	moesi_done();
	mmu_done();
}


/* Return the entry point in the cache hierarchy depending on the core-thread pair
 * and the type of block accessed (if data=1, data cache, otherwise, instruction cache) */
static struct ccache_t *cache_system_get_ccache(int core, int thread, enum cache_kind_enum cache_kind)
{
	int index;
	if (core >= num_cores || thread >= num_threads)
		panic("core.thread out of range");
	index = core * num_threads + thread;
	return cache_kind == cache_kind_data ? node_array[index].dcache : node_array[index].icache;
}


static struct tlb_t *cache_system_get_tlb(int core, int thread)
{
	int index;
	if (core >= num_cores || thread >= num_threads)
		panic("core.thread out of range");
	index = core * num_threads + thread;
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
	for (core = 0; core < num_cores; core++) {
		for (thread = 0; thread < num_threads; thread++) {
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


int cache_system_can_access(int core, int thread, enum cache_kind_enum cache_kind, uint32_t addr)
{
	struct ccache_t *ccache;
	int ports;
	
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	ports = cache_kind == cache_kind_data ? dports : iports;
	assert(ccache->pending_count <= ports);
	return ccache->pending_count < ports &&
		!cache_system_pending_address(core, thread, cache_kind, addr);
}


static uint64_t cache_system_access(int core, int thread, uint32_t addr, int *witness,
	int read, enum cache_kind_enum cache_kind)
{
	struct cache_system_stack_t *newstack;
	struct ccache_t *ccache;
	uint64_t access;

	/* Check that the physical address is valid for the MMU, i.e.,
	 * it belongs to an allocated physical page. */
	if (!mmu_valid_phaddr(addr)) {
		fprintf(stderr, "fatal: cache_system_access: not allocated physical page\n");
		abort();
	}

	/* Record immediately a new access */
	ccache = cache_system_get_ccache(core, thread, cache_kind);
	access = ccache_start_access(ccache, addr);

	/* Schedule event */
	newstack = cache_system_stack_create(core, thread, addr,
		ESIM_EV_NONE, NULL);
	newstack->read = read;
	newstack->cache_kind = cache_kind;
	newstack->witness = witness;
	esim_schedule_event(EV_CACHE_SYSTEM_ACCESS, newstack, 0);
	return access;
}


uint64_t cache_system_write(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, int *witness)
{
	addr &= ~(cache_block_size - 1);
	assert(cache_kind == cache_kind_data);
	assert(cache_system_can_access(core, thread, cache_kind, addr));
	return cache_system_access(core, thread, addr, witness, 0, cache_kind);
}


uint64_t cache_system_read(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, int *witness)
{
	addr &= ~(cache_block_size - 1);
	assert(cache_system_can_access(core, thread, cache_kind, addr));
	return cache_system_access(core, thread, addr, witness, 1, cache_kind);
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
		esim_schedule_event(stack->read ? EV_MOESI_LOAD :
			EV_MOESI_STORE, newstack, 0);
		return;
	}

	if (event == EV_CACHE_SYSTEM_ACCESS_TLB) {
		struct tlb_t *tlb;
		uint32_t set, way, tag;
		int hit;

		/* Access tlb */
		tlb = cache_system_get_tlb(stack->core, stack->thread);
		hit = cache_find_block(tlb->cache, stack->addr, &set, &way, NULL);

		/* Stats */
		tlb->accesses++;
		if (hit)
			tlb->hits++;

		/* On a miss, replace an entry */
		if (!hit) {
			cache_decode_address(tlb->cache, stack->addr, &set, &tag, NULL);
			way = cache_replace_block(tlb->cache, set);
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
		if (stack->witness)
			(*stack->witness)++;
		cache_system_stack_return(stack);
		return;
	}

	/* Not handled event */
	abort();
}

