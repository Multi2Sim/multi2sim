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

enum cache_access_kind_t
{
	cache_access_kind_read = 0,
	cache_access_kind_write
};

extern struct string_map_t cache_policy_map;

enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;
	uint32_t tag, transient_tag;
	uint32_t way;
	int state;
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
	struct mod_stack_t *waiting_list_head, *waiting_list_tail;
	int waiting_count, waiting_max;

};


/* Bank */
struct mod_bank_t
{
	/* Stats */
	uint64_t accesses;

	/* Ports */
	struct mod_port_t ports[0];
};


#define SIZEOF_MOD_BANK(CACHE) (sizeof(struct mod_bank_t) + sizeof(struct mod_port_t) \
	* ((CACHE)->read_port_count + (CACHE)->write_port_count))

#define MOD_BANK_INDEX(CACHE, I)  ((struct mod_bank_t *) ((void *) (CACHE)->banks + SIZEOF_MOD_BANK(CACHE) * (I)))

#define MOD_READ_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(I)])
#define MOD_WRITE_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(CACHE)->read_port_count + (I)])

/* Module types */
enum mod_kind_t
{
	mod_kind_invalid = 0,
	mod_kind_cache,
	mod_kind_main_memory
};

enum mod_range_kind_t
{
	mod_range_invalid = 0,
	mod_range_bounds,
	mod_range_interleaved
};

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

	/* Number of locked read/write ports (adding up all banks) */
	int locked_read_port_count;
	int locked_write_port_count;

	/* Waiting list of events */
	struct mod_stack_t *waiting_list_head, *waiting_list_tail;
	int waiting_count, waiting_max;

	/* Cache structure */
	struct cache_t *cache;

	/* Low and high memory modules */
	struct linked_list_t *high_mod_list;
	struct linked_list_t *low_mod_list;

	/* Interconnects */
	struct net_t *high_net;
	struct net_t *low_net;
	struct net_node_t *high_net_node;
	struct net_node_t *low_net_node;

	/* Stats */
	uint64_t reads;
	uint64_t effective_reads;
	uint64_t effective_read_hits;
	uint64_t writes;
	uint64_t effective_writes;
	uint64_t effective_write_hits;
	uint64_t evictions;
};

struct mod_t *mod_create(char *name, enum mod_kind_t kind,
	int bank_count, int read_port_count, int write_port_count,
	int block_size, int latency);
void mod_free(struct mod_t *mod);
void mod_dump(struct mod_t *mod, FILE *f);

void mod_access(struct mod_t *mod, int access, uint32_t addr, uint32_t size, int *witness_ptr);
struct mod_t *mod_get_low_mod(struct mod_t *mod, uint32_t addr);




/*
 * CPU/GPU Common Event-Driven Simulation
 */


/* Stack */
struct mod_stack_t
{
	uint64_t id;
	int *witness_ptr;

	struct mod_t *mod;
	struct mod_t *target_mod;

	struct mod_bank_t *bank;
	struct mod_port_t *port;
	uint32_t addr;
	uint32_t tag;
	uint32_t set;
	uint32_t way;
	int status;
	uint32_t block_index;
	uint32_t bank_index;
	int read_port_index;
	int write_port_index;
	int pending;
	int hit;

	/* Message sent through interconnect */
	struct net_msg_t *msg;

	/* Linked list for waiting events */
	int waiting_list_event;  /* Event to schedule when stack is waken up */
	struct mod_stack_t *waiting_prev, *waiting_next;

	/* Return stack */
	struct mod_stack_t *ret_stack;
	int ret_event;
};

extern long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	uint32_t addr, int ret_event, void *ret_stack);
void mod_stack_return(struct mod_stack_t *stack);





/*
 * GPU Event-Driven Simulation
 */

extern int EV_GPU_MEM_READ;
extern int EV_GPU_MEM_READ_REQUEST;
extern int EV_GPU_MEM_READ_REQUEST_RECEIVE;
extern int EV_GPU_MEM_READ_REQUEST_REPLY;
extern int EV_GPU_MEM_READ_REQUEST_FINISH;
extern int EV_GPU_MEM_READ_UNLOCK;
extern int EV_GPU_MEM_READ_FINISH;

extern int EV_GPU_MEM_WRITE;
extern int EV_GPU_MEM_WRITE_REQUEST_SEND;
extern int EV_GPU_MEM_WRITE_REQUEST_RECEIVE;
extern int EV_GPU_MEM_WRITE_REQUEST_REPLY;
extern int EV_GPU_MEM_WRITE_REQUEST_REPLY_RECEIVE;
extern int EV_GPU_MEM_WRITE_UNLOCK;
extern int EV_GPU_MEM_WRITE_FINISH;

void gpu_mod_handler_read(int event, void *data);
void gpu_mod_handler_write(int event, void *data);



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
	struct ccache_t *ccache, *target, *except;
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

struct moesi_stack_t *moesi_stack_create(uint64_t id, struct ccache_t *ccache,
	uint32_t addr, int retevent, void *retstack);
void moesi_stack_return(struct moesi_stack_t *stack);




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
	struct list_t *net_list;
	struct list_t *mod_list;
};

extern struct mem_system_t *mem_system;

void mem_system_init(void);
void mem_system_done(void);

void mem_system_config_read(void);
void mem_system_dump_report(void);


#endif

