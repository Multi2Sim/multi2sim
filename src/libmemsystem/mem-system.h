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

#ifndef MEM_SYSTEM_H
#define MEM_SYSTEM_H

#include <mhandle.h>
#include <stdint.h>
#include <stdio.h>
#include <debug.h>
#include <network.h>
#include <list.h>
#include <linked-list.h>
#include <misc.h>




/*
 * Cache Memory
 */

extern struct string_map_t cache_policy_map;

enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

enum cache_block_state_t
{
	cache_block_invalid = 0,
	cache_block_non_coherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared
};

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	uint32_t tag;
	uint32_t transient_tag;
	uint32_t way;

	enum cache_block_state_t state;
};

struct cache_set_t
{
	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;
};

struct cache_t
{
	uint32_t num_sets;
	uint32_t block_size;
	uint32_t assoc;
	enum cache_policy_t policy;

	struct cache_set_t *sets;
	uint32_t block_mask;
	int log_block_size;
};


struct cache_t *cache_create(uint32_t num_sets, uint32_t block_size, uint32_t assoc,
	enum cache_policy_t policy);
void cache_free(struct cache_t *cache);

void cache_decode_address(struct cache_t *cache, uint32_t addr,
	uint32_t *set_ptr, uint32_t *tag_ptr, uint32_t *offset_ptr);
int cache_find_block(struct cache_t *cache, uint32_t addr,
	uint32_t *set_ptr, uint32_t *pway, int *state_ptr);
void cache_set_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t tag, int state);
void cache_get_block(struct cache_t *cache, uint32_t set, uint32_t way,
	uint32_t *tag_ptr, int *state_ptr);
void cache_access_block(struct cache_t *cache, uint32_t set, uint32_t way);
uint32_t cache_replace_block(struct cache_t *cache, uint32_t set);
void cache_set_transient_tag(struct cache_t *cache, uint32_t set, uint32_t way, uint32_t tag);



/*
 * Directory
 */


struct dir_lock_t
{
	int lock;
	struct mod_stack_t *lock_queue;
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
	int num_nodes;

	/* Width, height and depth of the directory. For caches, it is
	 * useful to have a 3-dim directory. XSize is the number of
	 * sets, YSize is the number of ways of the cache, and ZSize
	 * is the number of sub-blocks of size 'cache_min_block_size'
	 * that fit within a block. */
	int xsize, ysize, zsize;

	/* Array of xsize * ysize locks. Each lock corresponds to a
	 * block, i.e. a set of zsize directory entries */
	struct dir_lock_t *dir_lock;

	/* Last field. This is an array of xsize*ysize*zsize elements of type
	 * dir_entry_t, which have likewise variable size. */
	unsigned char data[0];
};

struct dir_t *dir_create(int xsize, int ysize, int zsize, int num_nodes);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z);
void dir_entry_set_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_all_sharers(struct dir_t *dir, volatile struct dir_entry_t *dir_entry);
int dir_entry_is_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node);
void dir_entry_dump_sharers(struct dir_t *dir, struct dir_entry_t *dir_entry);
int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y);

struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y);
int dir_lock_lock(struct dir_lock_t *dir_lock, int event, struct mod_stack_t *stack);
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
 * Memory Module
 */

/* Port */
struct mod_port_t
{
	/* Port lock status */
	int locked;
	uint64_t lock_when;
	struct mod_stack_t *stack;  /* Current access */

	/* Waiting list */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;

};


/* Bank */
struct mod_bank_t
{
	/* Stats */
	long long accesses;

	/* Ports */
	struct mod_port_t ports[0];
};


#define SIZEOF_MOD_BANK(CACHE) (sizeof(struct mod_bank_t) + sizeof(struct mod_port_t) \
	* ((CACHE)->read_port_count + (CACHE)->write_port_count))

#define MOD_BANK_INDEX(CACHE, I)  ((struct mod_bank_t *) ((void *) (CACHE)->banks + SIZEOF_MOD_BANK(CACHE) * (I)))

#define MOD_READ_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(I)])
#define MOD_WRITE_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(CACHE)->read_port_count + (I)])

/* Access type */
enum mod_access_kind_t
{
	mod_access_invalid = 0,
	mod_access_read,
	mod_access_write,
	mod_access_nc_write
};

/* Module types */
enum mod_kind_t
{
	mod_kind_invalid = 0,
	mod_kind_cache,
	mod_kind_main_memory
};

/* Type of address range */
enum mod_range_kind_t
{
	mod_range_invalid = 0,
	mod_range_bounds,
	mod_range_interleaved
};

#define MOD_ACCESS_HASH_TABLE_SIZE  7

/* Memory module */
struct mod_t
{
	/* Parameters */
	enum mod_kind_t kind;
	char *name;
	int block_size;
	int log_block_size;
	int latency;

	/* Address range */
	enum mod_range_kind_t range_kind;
	union
	{
		/* For range_kind = mod_range_bounds */
		struct
		{
			uint32_t low;
			uint32_t high;
		} bounds;

		/* For range_kind = mod_range_interleaved */
		struct
		{
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

	/* Number of locked read/write ports (adding up all banks) */
	int locked_read_port_count;
	int locked_write_port_count;

	/* Directory */
	struct dir_t *dir;
	int dir_size;
	int dir_assoc;
	int dir_num_sets;

	/* Waiting list of events */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;

	/* Cache structure */
	struct cache_t *cache;

	/* Low and high memory modules */
	struct linked_list_t *high_mod_list;
	struct linked_list_t *low_mod_list;

	/* Smallest block size of high nodes. When there is no high node, the
	 * sub-block size is equal to the block size. */
	int sub_block_size;
	int num_sub_blocks;  /* block_size / sub_block_size */

	/* Interconnects */
	struct net_t *high_net;
	struct net_t *low_net;
	struct net_node_t *high_net_node;
	struct net_node_t *low_net_node;

	/* Linked list of accesses */
	struct mod_stack_t *access_list_head;
	struct mod_stack_t *access_list_tail;
	int access_list_count;
	int access_list_max;

	/* Hash table of accesses */
	struct
	{
		struct mod_stack_t *bucket_list_head;
		struct mod_stack_t *bucket_list_tail;
		int bucket_list_count;
		int bucket_list_max;
	} access_hash_table[MOD_ACCESS_HASH_TABLE_SIZE];

	/* FIXME: remove */
	struct linked_list_t *__access_list;  /* Elements of type ccache_access_t */
	int __pending_reads;
	int __pending_writes;

	/* FIXME: remove */
	int color;  /* For coloring algorithm */

	/* Stats */
	long long accesses;
	long long hits;

	long long reads;
	long long effective_reads;
	long long effective_read_hits;
	long long writes;
	long long effective_writes;
	long long effective_write_hits;
	long long evictions;

	long long blocking_reads;
	long long non_blocking_reads;
	long long read_hits;
	long long blocking_writes;
	long long non_blocking_writes;
	long long write_hits;

	long long read_retries;
	long long write_retries;

	long long no_retry_accesses;
	long long no_retry_hits;
	long long no_retry_reads;
	long long no_retry_read_hits;
	long long no_retry_writes;
	long long no_retry_write_hits;
};

struct mod_t *mod_create(char *name, enum mod_kind_t kind,
	int bank_count, int read_port_count, int write_port_count,
	int block_size, int latency);
void mod_free(struct mod_t *mod);
void mod_dump(struct mod_t *mod, FILE *f);

/* Access a module.
 * FIXME: mod_type: 1 - CPU, 2 - GPU. Remove once the coherence protocol is
 * shared between them. Currently, this determines the first event to
 * schedule (EV_MOD_xxx or EV_GPU_MOD_xxx). */
long long mod_access(struct mod_t *mod, int mod_type, enum mod_access_kind_t access_kind,
	uint32_t addr, int *witness_ptr, struct linked_list_t *event_queue, void *event_queue_item);
int mod_can_access(struct mod_t *mod, uint32_t addr);

void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack);
void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack);
int mod_access_in_flight(struct mod_t *mod, long long id, uint32_t addr);

struct mod_t *mod_get_low_mod(struct mod_t *mod, uint32_t addr);




/*
 * CPU/GPU Common Event-Driven Simulation
 */

extern int EV_MOD_GPU_LOAD;
extern int EV_MOD_GPU_LOAD_FINISH;

extern int EV_MOD_GPU_STORE;
extern int EV_MOD_GPU_STORE_FINISH;

extern int EV_MOD_GPU_READ;
extern int EV_MOD_GPU_READ_REQUEST;
extern int EV_MOD_GPU_READ_REQUEST_RECEIVE;
extern int EV_MOD_GPU_READ_REQUEST_REPLY;
extern int EV_MOD_GPU_READ_REQUEST_FINISH;
extern int EV_MOD_GPU_READ_UNLOCK;
extern int EV_MOD_GPU_READ_FINISH;

extern int EV_MOD_GPU_WRITE;
extern int EV_MOD_GPU_WRITE_REQUEST_SEND;
extern int EV_MOD_GPU_WRITE_REQUEST_RECEIVE;
extern int EV_MOD_GPU_WRITE_REQUEST_REPLY;
extern int EV_MOD_GPU_WRITE_REQUEST_REPLY_RECEIVE;
extern int EV_MOD_GPU_WRITE_UNLOCK;
extern int EV_MOD_GPU_WRITE_FINISH;



/*
 * CPU Event-Driven Simulation
 */

extern int EV_MOD_LOAD;
extern int EV_MOD_LOAD_LOCK;
extern int EV_MOD_LOAD_ACTION;
extern int EV_MOD_LOAD_MISS;
extern int EV_MOD_LOAD_FINISH;

extern int EV_MOD_STORE;
extern int EV_MOD_STORE_LOCK;
extern int EV_MOD_STORE_ACTION;
extern int EV_MOD_STORE_FINISH;

extern int EV_MOD_FIND_AND_LOCK;
extern int EV_MOD_FIND_AND_LOCK_ACTION;
extern int EV_MOD_FIND_AND_LOCK_FINISH;

extern int EV_MOD_EVICT;
extern int EV_MOD_EVICT_INVALID;
extern int EV_MOD_EVICT_ACTION;
extern int EV_MOD_EVICT_RECEIVE;
extern int EV_MOD_EVICT_WRITEBACK;
extern int EV_MOD_EVICT_WRITEBACK_EXCLUSIVE;
extern int EV_MOD_EVICT_WRITEBACK_FINISH;
extern int EV_MOD_EVICT_PROCESS;
extern int EV_MOD_EVICT_REPLY;
extern int EV_MOD_EVICT_REPLY_RECEIVE;
extern int EV_MOD_EVICT_FINISH;

extern int EV_MOD_WRITE_REQUEST;
extern int EV_MOD_WRITE_REQUEST_RECEIVE;
extern int EV_MOD_WRITE_REQUEST_ACTION;
extern int EV_MOD_WRITE_REQUEST_EXCLUSIVE;
extern int EV_MOD_WRITE_REQUEST_UPDOWN;
extern int EV_MOD_WRITE_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_WRITE_REQUEST_DOWNUP;
extern int EV_MOD_WRITE_REQUEST_REPLY;
extern int EV_MOD_WRITE_REQUEST_FINISH;

extern int EV_MOD_READ_REQUEST;
extern int EV_MOD_READ_REQUEST_RECEIVE;
extern int EV_MOD_READ_REQUEST_ACTION;
extern int EV_MOD_READ_REQUEST_UPDOWN;
extern int EV_MOD_READ_REQUEST_UPDOWN_MISS;
extern int EV_MOD_READ_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_READ_REQUEST_DOWNUP;
extern int EV_MOD_READ_REQUEST_DOWNUP_FINISH;
extern int EV_MOD_READ_REQUEST_REPLY;
extern int EV_MOD_READ_REQUEST_FINISH;

extern int EV_MOD_INVALIDATE;
extern int EV_MOD_INVALIDATE_FINISH;


/* Current identifier for stack */
extern long long mod_stack_id;


/* Stack */
struct mod_stack_t
{
	long long id;
	int *witness_ptr;

	struct linked_list_t *event_queue;
	void *event_queue_item;

	struct mod_t *mod;
	struct mod_t *target_mod;
	struct mod_t *except_mod;

	struct mod_bank_t *bank;
	struct mod_port_t *port;
	uint32_t addr;
	uint32_t tag;
	uint32_t set;
	uint32_t way;
	int state;

	uint32_t src_set;
	uint32_t src_way;
	uint32_t src_tag;

	uint32_t block_index;
	uint32_t bank_index;
	int read_port_index;
	int write_port_index;

	struct dir_lock_t *dir_lock;
	int reply_size;
	int pending;

	/* Linked list of accesses in 'mod' */
	struct mod_stack_t *access_list_prev;
	struct mod_stack_t *access_list_next;

	/* Bucket list of accesses in hash table in 'mod' */
	struct mod_stack_t *bucket_list_prev;
	struct mod_stack_t *bucket_list_next;

	/* Flags */
	int hit : 1;
	int err : 1;
	int shared : 1;
	int read : 1;
	int blocking : 1;
	int writeback : 1;
	int eviction : 1;
	int retry : 1;

	/* Message sent through interconnect */
	struct net_msg_t *msg;

	/* Linked list for waiting events */
	int waiting_list_event;  /* Event to schedule when stack is waken up */
	struct mod_stack_t *waiting_list_prev;
	struct mod_stack_t *waiting_list_next;

	/* Events wiating in directory lock */
	int dir_lock_event;
	struct mod_stack_t *dir_lock_next;

	/* Return stack */
	struct mod_stack_t *ret_stack;
	int ret_event;
};

extern long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	uint32_t addr, int ret_event, void *ret_stack);
void mod_stack_return(struct mod_stack_t *stack);

void mod_stack_wait_in_mod(struct mod_stack_t *stack, int event);
void mod_stack_wakeup_mod(struct mod_t *mod);

void mod_stack_wait_in_port(struct mod_stack_t *stack, int event);
void mod_stack_wakeup_port(struct mod_port_t *port);

void mod_handler_gpu_load(int event, void *data);
void mod_handler_gpu_store(int event, void *data);
void mod_handler_gpu_read(int event, void *data);
void mod_handler_gpu_write(int event, void *data);

void mod_handler_find_and_lock(int event, void *data);
void mod_handler_load(int event, void *data);
void mod_handler_store(int event, void *data);
void mod_handler_evict(int event, void *data);
void mod_handler_write_request(int event, void *data);
void mod_handler_read_request(int event, void *data);
void mod_handler_invalidate(int event, void *data);




/*
 * Memory System
 */

extern char *mem_config_file_name;
extern char *mem_config_help;

extern char *mem_report_file_name;

#define mem_debugging() debug_status(mem_debug_category)
#define mem_debug(...) debug(mem_debug_category, __VA_ARGS__)
extern int mem_debug_category;

struct mem_system_t
{
	/* List of modules and networks */
	struct list_t *mod_list;
	struct list_t *net_list;
};

extern struct mem_system_t *mem_system;

void mem_system_init(void);
void mem_system_done(void);

void mem_system_config_read(void);
void mem_system_dump_report(void);


#endif

