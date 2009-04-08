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

#ifndef CACHESYSTEM_H
#define CACHESYSTEM_H

#include <mhandle.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <lnlist.h>
#include <options.h>
#include <config.h>
#include <network.h>




/* Directory */

struct dir_entry_t {
	int lock;
	struct moesi_stack_t *lock_queue;
	int owner;  /* node owning the block */
	int sharers;  /* number of 1s in next field */
	unsigned char sharer[0];  /* bitmap of sharers (must be last field) */
};

struct dir_t {
	/* Number of possible sharers for a block. This determines
	 * the size of the directory entry bitmap. */
	int nodes;

	/* Width and height of the directory. For caches, it is
	 * useful to have a 2-dim directory. XSize is the number of
	 * sets and YSize is the number of ways of the cache. */
	int xsize, ysize;

	/* Last field. This is an array of elements of type
	 * dir_entry_t, which have likewise variable size. */
	unsigned char data[0];
};

struct dir_t *dir_create(int xsize, int ysize, int nodes);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y);
void dir_entry_set_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_all_sharers(struct dir_t *dir, struct dir_entry_t *dir_entry);
int dir_entry_is_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_dump_sharers(struct dir_t *dir, struct dir_entry_t *dir_entry);

int dir_entry_lock(struct dir_t *dir, struct dir_entry_t *dir_entry,
	int event, struct moesi_stack_t *stack);
void dir_entry_unlock(struct dir_t *dir, struct dir_entry_t *dir_entry);




/* Memory Management Unit */

#define MMU_PAGE_HASH_SIZE	(1 << 10)
#define MMU_PAGE_LIST_SIZE	(1 << 10)
#define MMU_LOG_PAGE_SIZE	(12)
#define MMU_PAGE_SIZE		(1 << MMU_LOG_PAGE_SIZE)
#define MMU_PAGE_MASK		(MMU_PAGE_SIZE - 1)

void mmu_init(void);
void mmu_done(void);
uint32_t mmu_translate(int mid, uint32_t vtladdr);
void mmu_get_dir_entry(uint32_t phaddr, struct dir_t **pdir,
	struct dir_entry_t **pdir_entry);
int mmu_valid_phaddr(uint32_t phaddr);




/* Cache Memory */

struct cache_blk_t {
	struct cache_blk_t *way_next;
	struct cache_blk_t *way_prev;
	uint32_t tag, transient_tag;
	uint32_t way;
	int status;
};

struct cache_set_t {
	struct cache_blk_t *way_head;
	struct cache_blk_t *way_tail;
	struct cache_blk_t *blks;
};

struct cache_t {
	uint32_t nsets;
	uint32_t bsize;
	uint32_t assoc;

	struct cache_set_t *sets;
	uint32_t bmask;
	int logbsize;
};


struct cache_t *cache_create(uint32_t nsets, uint32_t bsize, uint32_t assoc);
void cache_free(struct cache_t *cache);

int cache_log2(uint32_t x);
void cache_decode_address(struct cache_t *cache, uint32_t addr,
	uint32_t *pset, uint32_t *ptag, uint32_t *poffset);
int cache_find_block(struct cache_t *cache, uint32_t addr,
	uint32_t *pset, uint32_t *pway, int *pstatus);
void cache_set_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t tag, int status);
void cache_get_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t *ptag, int *pstatus);
void cache_access_block(struct cache_t *cache, uint32_t set, uint32_t way);
uint32_t cache_replace_block(struct cache_t *cache, uint32_t set);
void cache_set_transient_tag(struct cache_t *cache, uint32_t set, uint32_t way, uint32_t tag);




/* MOESI Protocol */

#define cache_debug(...) debug(cache_debug_category, __VA_ARGS__)
extern int cache_debug_category;

extern int EV_MOESI_FIND_AND_LOCK;
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
	moesi_status_invalid = 0,
	moesi_status_modified,
	moesi_status_owned,
	moesi_status_exclusive,
	moesi_status_shared
};

struct moesi_stack_t {
	uint64_t id;
	struct ccache_t *ccache, *target, *except;
	uint32_t tag, set, way;
	uint32_t ccache_set, ccache_way;
	struct dir_t *dir, *ccache_dir;
	struct dir_entry_t *dir_entry, *ccache_dir_entry;
	int status, response, pending;
	
	/* Flags */
	int err : 1;
	int shared : 1;
	int blocking : 1;
	int writeback : 1;
	int eviction : 1;

	/* Cache block locks */
	int lock_event;
	struct moesi_stack_t *lock_next;

	/* Return event */
	int retevent;
	void *retstack;
};

extern uint64_t moesi_stack_id;

struct moesi_stack_t *moesi_stack_create(uint64_t id, struct ccache_t *ccache,
	uint32_t addr, int retevent, void *retstack);
void moesi_stack_return(struct moesi_stack_t *stack);




/* Coherent Cache */

struct ccache_access_t {
	uint32_t address;
	uint64_t access;  /* id of the access */
	int valid : 1;
};

struct ccache_t {
	
	/* Parameters */
	char name[100];
	int loid;  /* ID in the low interconnect */
	struct net_t *hinet, *lonet;  /* High and low interconnects */
	int lat;
	struct ccache_t *next;  /* Next cache in hierarchy */
	struct cache_t *cache;  /* Cache holding data */
	struct dir_t *dir;

	/* Record of pending accesses */
	struct ccache_access_t *pending;  /* list of pending accesses */
	int pending_count;
	int pending_size;

	/* Stats */
	uint64_t accesses;
	uint64_t reads;
	uint64_t hits;
};

struct ccache_t *ccache_create();
void ccache_free(struct ccache_t *ccache);

int ccache_find_block(struct ccache_t *ccache, uint32_t tag,
	uint32_t *pset, uint32_t *pway, int *pstatus,
	struct dir_t **pdir, struct dir_entry_t **pdir_entry);
void ccache_get_block(struct ccache_t *ccache, uint32_t set, uint32_t way,
	uint32_t *ptag, int *pstatus,
	struct dir_t **pdir, struct dir_entry_t **pdir_entry);



/* Tlb */

struct tlb_t {
	
	/* Parameters */
	char name[100];
	int hitlat;
	int misslat;
	struct cache_t *cache;  /* Cache holding data */

	/* Stats */
	uint64_t accesses;
	uint64_t hits;
};

struct tlb_t *tlb_create();
void tlb_free(struct tlb_t *tlb);




/* Cache System */

extern uint32_t cache_block_size;  /* option */
extern uint32_t cache_log_block_size;  /* derived */
extern struct ccache_t *main_memory;

enum cache_kind_enum {
	cache_kind_inst,
	cache_kind_data
};

enum tlb_kind_enum {
	tlb_kind_data = 0,
	tlb_kind_inst
};

struct cache_system_stack_t {
	int core, thread;
	enum cache_kind_enum cache_kind;
	int read;  /* 0=write, 1=read */
	uint32_t addr;
	int *witness;
	int pending;

	int retevent;
	void *retstack;
};

extern struct repos_t *cache_system_stack_repos;
struct cache_system_stack_t *cache_system_stack_create(int core, int thread, uint32_t addr,
	int retevent, void *retstack);
void cache_system_stack_return(struct cache_system_stack_t *stack);
void cache_system_handler(int event, void *data);

extern int EV_CACHE_SYSTEM_ACCESS;
extern int EV_CACHE_SYSTEM_ACCESS_CACHE;
extern int EV_CACHE_SYSTEM_ACCESS_TLB;
extern int EV_CACHE_SYSTEM_ACCESS_FINISH;

void cache_system_reg_options(void);
void cache_system_init(int def_cores, int def_threads);
void cache_system_done(void);
void cache_system_dump(FILE *f);

/* Return true if cache system can be accesses. If not, it can be due to
 * a lack of ports or that an access to the same block is pending. The
 * parameter cache_kind must be cache_kind_dl1 or cache_kind_il1.*/
int cache_system_can_access(int core, int thread,
	enum cache_kind_enum cache_kind, uint32_t addr);

/* Return true if the access to address addr or with identifier 'access'
 * has completed. Parameter cache_kind must be dl1 or il1. */
int cache_system_pending_address(int core, int thread,
	enum cache_kind_enum cache_kind, uint32_t addr);
int cache_system_pending_access(int core, int thread,
	enum cache_kind_enum cache_kind, uint64_t access);

/* Functions to access cache system */
uint64_t cache_system_read(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, int *witness);
uint64_t cache_system_write(int core, int thread, enum cache_kind_enum cache_kind,
	uint32_t addr, int *witness);


#endif

