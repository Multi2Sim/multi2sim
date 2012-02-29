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

#ifndef CACHESYSTEM_H
#define CACHESYSTEM_H

#include <mhandle.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <network.h>
#include <misc.h>
#include <esim.h>

#include <config.h>
#include <mem-system.h>
#include <debug.h>
#include <linked-list.h>




/*
 * Global variables
 */


extern char *cache_system_config_help;




/*
 * Coherent Cache
 */

struct ccache_access_t
{
	enum mod_access_kind_t access_kind;  /* Read or write */
	uint32_t address;  /* Block address */
	uint64_t id;  /* Access identifier */
	struct linked_list_t *eventq;  /* Event queue to modify when access finishes */
	void *eventq_item;  /* Item to enqueue when finished */
	struct ccache_access_t *next;  /* Alias (same address/access, but different eventq_item */
};

#if 0
struct ccache_t
{
	enum mod_kind_t kind;
	char *name;
	uint32_t block_size;
	int log_block_size;
	int latency;

	/* Directory */
	struct dir_t *dir;
	uint32_t dir_size;
	uint32_t dir_num_sets;
	uint32_t dir_assoc;

	/* Address range */
	enum mod_range_kind_t range_kind;
	union {
		/* For range_kind = mod_range_bounds */
		struct {
			uint32_t low;
			uint32_t high;
		} bounds;

		/* For range_kind = mod_range_interleaved */
		struct {
			uint32_t mod;
			uint32_t div;
			uint32_t eq;
		} interleaved;
	} range;

	/* Banks and ports */
	struct mod_bank_t *banks;
	int bank_count;
	int read_port_count;  /* Number of read ports (per bank) */
	int write_port_count;  /* Number of write ports (per bank) */

	/* Low and high memory modules */
	struct linked_list_t *high_mod_list;
	struct linked_list_t *low_mod_list;

	/* Associated structures */
	struct cache_t *cache;  /* Cache holding data */

	/* Networks */
	struct net_t *high_net;
	struct net_t *low_net;
	struct net_node_t *high_net_node;  /* Node in upper network */
	struct net_node_t *low_net_node;  /* Node in lower network */

	/* List of in-flight accesses */
	struct linked_list_t *access_list;  /* Elements of type ccache_access_t */
	int pending_reads;  /* Non-aliasing reads in access_list */
	int pending_writes;  /* Writes in access_list */

	/* Stats */
	uint64_t accesses;
	uint64_t hits;
	uint64_t evictions;
	uint64_t reads;
	uint64_t blocking_reads;
	uint64_t non_blocking_reads;
	uint64_t read_hits;
	uint64_t writes;
	uint64_t blocking_writes;
	uint64_t non_blocking_writes;
	uint64_t write_hits;

	uint64_t read_retries;
	uint64_t write_retries;

	uint64_t no_retry_accesses;
	uint64_t no_retry_hits;
	uint64_t no_retry_reads;
	uint64_t no_retry_read_hits;
	uint64_t no_retry_writes;
	uint64_t no_retry_write_hits;
};
#endif

struct mod_t *__mod_create(char *name, enum mod_kind_t kind);
void __mod_free(struct mod_t *mod);

int __mod_find_block(struct mod_t *mod, uint32_t addr,
	uint32_t *pset, uint32_t *pway, uint32_t *ptag, int *pstatus);
void __mod_dump(struct mod_t *mod, FILE *f);
struct mod_t *__mod_get_low_mod(struct mod_t *mod);


/*
 * CPU Event-Driven Simulation
 */

#define cache_debug(...) debug(cache_debug_category, __VA_ARGS__)
extern int cache_debug_category;

/*
 * TLB
 */

struct tlb_t
{
	/* Parameters */
	char name[100];
	int hitlat;
	int misslat;
	struct cache_t *cache;  /* Cache holding data */

	/* Stats */
	long long accesses;
	long long hits;
	long long evictions;
};

struct tlb_t *tlb_create();
void tlb_free(struct tlb_t *tlb);




/*
 * Cache System
 */

extern char *cache_system_config_file_name;
extern char *cache_system_report_file_name;

extern int cache_system_iperfect;
extern int cache_system_dperfect;

extern int cache_min_block_size;
extern int cache_max_block_size;

enum cache_kind_t
{
	cache_kind_inst,
	cache_kind_data
};

enum tlb_kind_t
{
	tlb_kind_data = 0,
	tlb_kind_inst
};

struct cache_system_stack_t
{
	int core, thread;
	enum cache_kind_t cache_kind;
	enum mod_access_kind_t access_kind;
	uint32_t addr;
	int pending;
	struct linked_list_t *eventq;
	void *eventq_item;

	int retevent;
	void *retstack;
};

struct cache_system_stack_t *cache_system_stack_create(int core, int thread, uint32_t addr,
	int retevent, void *retstack);
void cache_system_stack_return(struct cache_system_stack_t *stack);
void cache_system_handler(int event, void *data);

extern int EV_CACHE_SYSTEM_ACCESS;
extern int EV_CACHE_SYSTEM_ACCESS_CACHE;
extern int EV_CACHE_SYSTEM_ACCESS_TLB;
extern int EV_CACHE_SYSTEM_ACCESS_FINISH;

void cache_system_init(int def_cores, int def_threads);
void cache_system_done(void);

void cache_system_print_stats(FILE *f);
void cache_system_dump(FILE *f);

/* Return block size of the first cache when accessing the cache system
 * by a given core-thread and cache kind. */
int cache_system_block_size(int core, int thread,
	enum cache_kind_t cache_kind);

/* Return true if cache system can be accesses. */
int cache_system_can_access(int core, int thread, enum cache_kind_t cache_kind,
	enum mod_access_kind_t access_kind, uint32_t addr);

/* Return true if the access to address addr or with identifier 'access'
 * has completed. Parameter cache_kind must be dl1 or il1. */
int cache_system_pending_address(int core, int thread,
	enum cache_kind_t cache_kind, uint32_t addr);
int cache_system_pending_access(int core, int thread,
	enum cache_kind_t cache_kind, uint64_t access);

/* Functions to access cache system */
uint64_t cache_system_read(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *item);
uint64_t cache_system_write(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *eventq_item);


#endif

