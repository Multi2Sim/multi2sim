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
 * Directory
 */


struct dir_lock_t
{
	int lock;
	struct moesi_stack_t *lock_queue;
};

#define DIR_ENTRY_OWNER_NONE  (-1)
#define DIR_ENTRY_VALID_OWNER(dir_entry)  ((dir_entry)->owner >= 0)

struct dir_entry_t
{
	int owner;  /* Node owning the block (-1 = No owner)*/
	int num_sharers;  /* Number of 1s in next field */
	unsigned char sharer[0];  /* Bitmap of sharers (must be last field) */
};

struct dir_t
{
	/* Number of possible sharers for a block. This determines
	 * the size of the directory entry bitmap. */
	int nodes;

	/* Width, height and depth of the directory. For caches, it is
	 * useful to have a 3-dim directory. XSize is the number of
	 * sets, YSize is the number of ways of the cache, and ZSize
	 * is the number of subblocks of size 'cache_min_block_size'
	 * that fit within a block. */
	int xsize, ysize, zsize;

	/* Array of xsize * ysize locks. Each lock corresponds to a
	 * block, i.e. a set of zsize directory entries */
	struct dir_lock_t *dir_lock;

	/* Last field. This is an array of xsize*ysize*zsize elements of type
	 * dir_entry_t, which have likewise variable size. */
	unsigned char data[0];
};

struct dir_t *dir_create(int xsize, int ysize, int zsize, int nodes);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z);
void dir_entry_set_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_all_sharers(struct dir_t *dir, volatile struct dir_entry_t *dir_entry);
int dir_entry_is_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_dump_sharers(struct dir_t *dir, struct dir_entry_t *dir_entry);
int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y);

struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y);
int dir_lock_lock(struct dir_lock_t *dir_lock, int event, struct moesi_stack_t *stack);
void dir_lock_unlock(struct dir_lock_t *dir_lock);
void dir_unlock(struct dir_t *dir, int x, int y);




/*
 * Memory Management Unit
 */

extern uint32_t mmu_page_size;
extern uint32_t mmu_page_mask;
extern uint32_t mmu_log_page_size;

void mmu_init(void);
void mmu_done(void);
uint32_t mmu_translate(int mid, uint32_t vtladdr);
int mmu_valid_phaddr(uint32_t phaddr);




/*
 * Coherent Cache
 */

struct ccache_access_t
{
	enum cache_access_kind_t cache_access_kind;  /* Read or write */
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

extern int EV_MOESI_FIND_AND_LOCK;
extern int EV_MOESI_FIND_AND_LOCK_ACTION;
extern int EV_MOESI_FIND_AND_LOCK_FINISH;

extern int EV_MOESI_LOAD;
extern int EV_MOESI_LOAD_ACTION;
extern int EV_MOESI_LOAD_MISS;
extern int EV_MOESI_LOAD_FINISH;

extern int EV_MOESI_STORE;
extern int EV_MOESI_STORE_ACTION;
extern int EV_MOESI_STORE_FINISH;

extern int EV_MOESI_EVICT;
extern int EV_MOESI_EVICT_ACTION;
extern int EV_MOESI_EVICT_RECEIVE;
extern int EV_MOESI_EVICT_WRITEBACK;
extern int EV_MOESI_EVICT_WRITEBACK_EXCLUSIVE;
extern int EV_MOESI_EVICT_WRITEBACK_FINISH;
extern int EV_MOESI_EVICT_PROCESS;
extern int EV_MOESI_EVICT_REPLY;
extern int EV_MOESI_EVICT_REPLY_RECEIVE;
extern int EV_MOESI_EVICT_FINISH;

extern int EV_MOESI_WRITE_REQUEST;
extern int EV_MOESI_WRITE_REQUEST_RECEIVE;
extern int EV_MOESI_WRITE_REQUEST_ACTION;
extern int EV_MOESI_WRITE_REQUEST_EXCLUSIVE;
extern int EV_MOESI_WRITE_REQUEST_UPDOWN;
extern int EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH;
extern int EV_MOESI_WRITE_REQUEST_DOWNUP;
extern int EV_MOESI_WRITE_REQUEST_REPLY;
extern int EV_MOESI_WRITE_REQUEST_FINISH;

extern int EV_MOESI_READ_REQUEST;
extern int EV_MOESI_READ_REQUEST_RECEIVE;
extern int EV_MOESI_READ_REQUEST_ACTION;
extern int EV_MOESI_READ_REQUEST_UPDOWN;
extern int EV_MOESI_READ_REQUEST_UPDOWN_MISS;
extern int EV_MOESI_READ_REQUEST_UPDOWN_FINISH;
extern int EV_MOESI_READ_REQUEST_DOWNUP;
extern int EV_MOESI_READ_REQUEST_DOWNUP_FINISH;
extern int EV_MOESI_READ_REQUEST_REPLY;
extern int EV_MOESI_READ_REQUEST_FINISH;

extern int EV_MOESI_INVALIDATE;
extern int EV_MOESI_INVALIDATE_FINISH;

void moesi_handler_find_and_lock(int event, void *data);
void moesi_handler_load(int event, void *data);
void moesi_handler_store(int event, void *data);
void moesi_handler_evict(int event, void *data);
void moesi_handler_write_request(int event, void *data);
void moesi_handler_read_request(int event, void *data);
void moesi_handler_invalidate(int event, void *data);


void moesi_init(void);
void moesi_done(void);

enum {
	moesi_state_invalid = 0,
	moesi_state_modified,
	moesi_state_owned,
	moesi_state_exclusive,
	moesi_state_shared
};

struct moesi_stack_t
{
	uint64_t id;
	struct mod_t *mod, *target, *except;
	uint32_t addr, set, way, tag;
	uint32_t src_set, src_way, src_tag;
	struct dir_lock_t *dir_lock;
	int status, response, pending;
	int hit;

	/* Message sent to the network */
	struct net_msg_t *msg;

	/* Flags */
	int err : 1;
	int shared : 1;
	int read : 1;
	int blocking : 1;
	int writeback : 1;
	int eviction : 1;
	int retry : 1;

	/* Cache block lock */
	int lock_event;
	struct moesi_stack_t *lock_next;

	/* Return event */
	int retevent;
	void *retstack;
};

extern uint64_t moesi_stack_id;

struct moesi_stack_t *moesi_stack_create(uint64_t id, struct mod_t *mod,
	uint32_t addr, int retevent, void *retstack);
void moesi_stack_return(struct moesi_stack_t *stack);



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
	uint64_t accesses;
	uint64_t hits;
	uint64_t evictions;
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
	enum cache_access_kind_t cache_access_kind;
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
	enum cache_access_kind_t cache_access_kind, uint32_t addr);

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

