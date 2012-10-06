/*
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
 * Memory (for functional simulation)
 */

#define MEM_LOG_PAGE_SIZE  12
#define MEM_PAGE_SHIFT  MEM_LOG_PAGE_SIZE
#define MEM_PAGE_SIZE  (1 << MEM_LOG_PAGE_SIZE)
#define MEM_PAGE_MASK  (~(MEM_PAGE_SIZE - 1))
#define MEM_PAGE_COUNT  1024

enum mem_access_t
{
	mem_access_none   = 0x00,
	mem_access_read   = 0x01,
	mem_access_write  = 0x02,
	mem_access_exec   = 0x04,
	mem_access_init   = 0x08,
	mem_access_modif  = 0x10
};

/* Safe mode */
extern int mem_safe_mode;

/* A 4KB page of memory */
struct mem_page_t
{
	unsigned int tag;
	enum mem_access_t perm;  /* Access permissions; combination of flags */
	struct mem_page_t *next;
	unsigned char *data;
};

struct mem_t
{
	/* Number of extra contexts sharing memory image */
	int num_links;

	/* Memory pages */
	struct mem_page_t *pages[MEM_PAGE_COUNT];

	/* Safe mode */
	int safe;

	/* Heap break for CPU contexts */
	unsigned int heap_break;

	/* Last accessed address */
	unsigned int last_address;
};

extern unsigned long mem_mapped_space;
extern unsigned long mem_max_mapped_space;

struct mem_t *mem_create(void);
void mem_free(struct mem_t *mem);

struct mem_t *mem_link(struct mem_t *mem);
void mem_unlink(struct mem_t *mem);

void mem_clear(struct mem_t *mem);

struct mem_page_t *mem_page_get(struct mem_t *mem, unsigned int addr);
struct mem_page_t *mem_page_get_next(struct mem_t *mem, unsigned int addr);

unsigned int mem_map_space(struct mem_t *mem, unsigned int addr, int size);
unsigned int mem_map_space_down(struct mem_t *mem, unsigned int addr, int size);

void mem_map(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t perm);
void mem_unmap(struct mem_t *mem, unsigned int addr, int size);

void mem_protect(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t perm);
void mem_copy(struct mem_t *mem, unsigned int dest, unsigned int src, int size);

void mem_access(struct mem_t *mem, unsigned int addr, int size, void *buf, enum mem_access_t access);
void mem_read(struct mem_t *mem, unsigned int addr, int size, void *buf);
void mem_write(struct mem_t *mem, unsigned int addr, int size, void *buf);

void mem_zero(struct mem_t *mem, unsigned int addr, int size);
int mem_read_string(struct mem_t *mem, unsigned int addr, int size, char *str);
void mem_write_string(struct mem_t *mem, unsigned int addr, char *str);
void *mem_get_buffer(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t access);

void mem_dump(struct mem_t *mem, char *filename, unsigned int start, unsigned int end);
void mem_load(struct mem_t *mem, char *filename, unsigned int start);

void mem_clone(struct mem_t *dst_mem, struct mem_t *src_mem);





/*
 * Speculative Memory
 *
 * This is a memory used when speculative execution is activated for a context.
 * Speculative memory writes are stored here. A subsequent load during speculative
 * execution will read its value from this memory if it exists, and will load
 * from the actual complete 'mem_t' memory otherwise.
 * When speculative execution ends, the contents of this memory will be just
 * discarded.
 */

/* Number of entries in the hash table of pages */
#define SPEC_MEM_PAGE_TABLE_SIZE  32

/* Page size for memory. Every time a new location is written, a page of this
 * size will be allocated. */
#define SPEC_MEM_LOG_PAGE_SIZE  4
#define SPEC_MEM_PAGE_SIZE  (1 << SPEC_MEM_LOG_PAGE_SIZE)
#define SPEC_MEM_PAGE_MASK  (~(SPEC_MEM_PAGE_SIZE - 1))

/* To prevent an excessive growth of speculative memory, this is a limit of
 * pages. After this limit has reached, no more pages are allocated, reads will
 * be done from the non-speculative memory, and writes will be ignored while
 * in speculative mode. */
#define SPEC_MEM_MAX_PAGE_COUNT  100

struct spec_mem_page_t
{
	unsigned int addr;
	unsigned char data[SPEC_MEM_PAGE_SIZE];
	struct spec_mem_page_t *next;
};

struct spec_mem_t
{
	struct mem_t *mem;  /* Associated non-speculative memory */

	int page_count;  /* Number of words currently written */
	struct spec_mem_page_t *pages[SPEC_MEM_PAGE_TABLE_SIZE];  /* Hash table */
};

struct spec_mem_t *spec_mem_create(struct mem_t *mem);
void spec_mem_free(struct spec_mem_t *spec_mem);

void spec_mem_read(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf);
void spec_mem_write(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf);

void spec_mem_clear(struct spec_mem_t *spec_mem);




/*
 * Cache Memory
 */

extern struct string_map_t cache_policy_map;
extern struct string_map_t cache_block_state_map;

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
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared
};

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	int tag;
	int transient_tag;
	int way;

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
	char *name;

	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	enum cache_policy_t policy;

	struct cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;
};


struct cache_t *cache_create(char *name, unsigned int num_sets, unsigned int block_size,
	unsigned int assoc, enum cache_policy_t policy);
void cache_free(struct cache_t *cache);

void cache_decode_address(struct cache_t *cache, unsigned int addr,
	int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
int cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *pway, 
	int *state_ptr);
void cache_set_block(struct cache_t *cache, int set, int way, int tag, int state);
void cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);

void cache_access_block(struct cache_t *cache, int set, int way);
int cache_replace_block(struct cache_t *cache, int set);
void cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag);




/*
 * Directory
 */


struct dir_lock_t
{
	int lock;
	long long stack_id;
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
	char *name;

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

struct dir_t *dir_create(char *name, int xsize, int ysize, int zsize, int num_nodes);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z);

void dir_entry_set_owner(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_set_sharer(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_clear_sharer(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_clear_all_sharers(struct dir_t *dir, int x, int y, int z);
int dir_entry_is_sharer(struct dir_t *dir, int x, int y, int z, int node);
int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y);

void dir_entry_dump_sharers(struct dir_t *dir, int x, int y, int z);

struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y);
int dir_entry_lock(struct dir_t *dir, int x, int y, int event, struct mod_stack_t *stack);
void dir_entry_unlock(struct dir_t *dir, int x, int y);




/*
 * Memory Management Unit
 */

enum mmu_access_t
{
	mmu_access_invalid = 0,
	mmu_access_read,
	mmu_access_write,
	mmu_access_execute
};

extern char *mmu_report_file_name;

extern unsigned int mmu_page_size;
extern unsigned int mmu_page_mask;
extern unsigned int mmu_log_page_size;

void mmu_init(void);
void mmu_done(void);
void mmu_dump_report(void);

int mmu_address_space_new(void);
unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr);
int mmu_valid_phy_addr(unsigned int phy_addr);

void mmu_access_page(unsigned int phy_addr, enum mmu_access_t access);




/*
 * Memory Module
 */

/* Port */
struct mod_port_t
{
	/* Port lock status */
	int locked;
	long long lock_when;  /* Cycle when it was locked */
	struct mod_stack_t *stack;  /* Access locking port */

	/* Waiting list */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;
};

/* String map for access type */
extern struct string_map_t mod_access_kind_map;

/* Access type */
enum mod_access_kind_t
{
	mod_access_invalid = 0,
	mod_access_load,
	mod_access_store,
	mod_access_nc_store
};

/* Module types */
enum mod_kind_t
{
	mod_kind_invalid = 0,
	mod_kind_cache,
	mod_kind_main_memory,
	mod_kind_local_memory
};

/* Type of address range */
enum mod_range_kind_t
{
	mod_range_invalid = 0,
	mod_range_bounds,
	mod_range_interleaved
};

#define MOD_ACCESS_HASH_TABLE_SIZE  17

/* Memory module */
struct mod_t
{
	/* Parameters */
	enum mod_kind_t kind;
	char *name;
	int block_size;
	int log_block_size;
	int latency;
	int dir_latency;
	int mshr_size;

	/* Module level starting from entry points */
	int level;

	/* Address range served by module */
	enum mod_range_kind_t range_kind;
	union
	{
		/* For range_kind = mod_range_bounds */
		struct
		{
			unsigned int low;
			unsigned int high;
		} bounds;

		/* For range_kind = mod_range_interleaved */
		struct
		{
			unsigned int mod;
			unsigned int div;
			unsigned int eq;
		} interleaved;
	} range;

	/* Ports */
	struct mod_port_t *ports;
	int num_ports;
	int num_locked_ports;

	/* Accesses waiting to get a port */
	struct mod_stack_t *port_waiting_list_head;
	struct mod_stack_t *port_waiting_list_tail;
	int port_waiting_list_count;
	int port_waiting_list_max;

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

	/* Access list */
	struct mod_stack_t *access_list_head;
	struct mod_stack_t *access_list_tail;
	int access_list_count;
	int access_list_max;

	/* Write access list */
	struct mod_stack_t *write_access_list_head;
	struct mod_stack_t *write_access_list_tail;
	int write_access_list_count;
	int write_access_list_max;

	/* Number of in-flight coalesced accesses. This is a number
	 * between 0 and 'access_list_count' at all times. */
	int access_list_coalesced_count;

	/* Hash table of accesses */
	struct
	{
		struct mod_stack_t *bucket_list_head;
		struct mod_stack_t *bucket_list_tail;
		int bucket_list_count;
		int bucket_list_max;
	} access_hash_table[MOD_ACCESS_HASH_TABLE_SIZE];

	/* For coloring algorithm used to check collisions between CPU and GPU
	 * memory hierarchies. Remove when fused. */
	int color;

	/* Statistics */
	long long accesses;
	long long hits;

	long long reads;
	long long effective_reads;
	long long effective_read_hits;
	long long writes;
	long long effective_writes;
	long long effective_write_hits;
	long long nc_writes;
	long long effective_nc_writes;
	long long effective_nc_write_hits;
	long long evictions;

	long long blocking_reads;
	long long non_blocking_reads;
	long long read_hits;
	long long blocking_writes;
	long long non_blocking_writes;
	long long write_hits;
	long long blocking_nc_writes;
	long long non_blocking_nc_writes;
	long long nc_write_hits;

	long long read_retries;
	long long write_retries;
	long long nc_write_retries;

	long long no_retry_accesses;
	long long no_retry_hits;
	long long no_retry_reads;
	long long no_retry_read_hits;
	long long no_retry_writes;
	long long no_retry_write_hits;
	long long no_retry_nc_writes;
	long long no_retry_nc_write_hits;
};

struct mod_t *mod_create(char *name, enum mod_kind_t kind, int num_ports,
	int block_size, int latency);
void mod_free(struct mod_t *mod);
void mod_dump(struct mod_t *mod, FILE *f);
void mod_stack_set_reply(struct mod_stack_t *stack, int reply);
struct mod_t *mod_stack_set_peer(struct mod_t *peer, int state);

long long mod_access(struct mod_t *mod, enum mod_access_kind_t access_kind, 
	unsigned int addr, int *witness_ptr, struct linked_list_t *event_queue,
	void *event_queue_item);
int mod_can_access(struct mod_t *mod, unsigned int addr);

int mod_find_block(struct mod_t *mod, unsigned int addr, int *set_ptr, int *way_ptr, 
	int *tag_ptr, int *state_ptr);

void mod_lock_port(struct mod_t *mod, struct mod_stack_t *stack, int event);
void mod_unlock_port(struct mod_t *mod, struct mod_port_t *port,
	struct mod_stack_t *stack);

void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack,
	enum mod_access_kind_t access_kind);
void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack);

int mod_in_flight_access(struct mod_t *mod, long long id, unsigned int addr);
struct mod_stack_t *mod_in_flight_address(struct mod_t *mod, unsigned int addr,
	struct mod_stack_t *older_than_stack);
struct mod_stack_t *mod_in_flight_write(struct mod_t *mod,
	struct mod_stack_t *older_than_stack);

int mod_serves_address(struct mod_t *mod, unsigned int addr);
struct mod_t *mod_get_low_mod(struct mod_t *mod, unsigned int addr);

int mod_get_retry_latency(struct mod_t *mod);

struct mod_stack_t *mod_can_coalesce(struct mod_t *mod,
	enum mod_access_kind_t access_kind, unsigned int addr,
	struct mod_stack_t *older_than_stack);
void mod_coalesce(struct mod_t *mod, struct mod_stack_t *master_stack,
	struct mod_stack_t *stack);




/*
 * CPU/GPU Common Event-Driven Simulation
 */

/* Local Memory */

extern int EV_MOD_LOCAL_MEM_LOAD;
extern int EV_MOD_LOCAL_MEM_LOAD_LOCK;
extern int EV_MOD_LOCAL_MEM_LOAD_FINISH;

extern int EV_MOD_LOCAL_MEM_STORE;
extern int EV_MOD_LOCAL_MEM_STORE_LOCK;
extern int EV_MOD_LOCAL_MEM_STORE_FINISH;

extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION;
extern int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH;

/* NMOESI Event-Driven Simulation */

extern int EV_MOD_NMOESI_LOAD;
extern int EV_MOD_NMOESI_LOAD_LOCK;
extern int EV_MOD_NMOESI_LOAD_ACTION;
extern int EV_MOD_NMOESI_LOAD_MISS;
extern int EV_MOD_NMOESI_LOAD_UNLOCK;
extern int EV_MOD_NMOESI_LOAD_FINISH;

extern int EV_MOD_NMOESI_STORE;
extern int EV_MOD_NMOESI_STORE_LOCK;
extern int EV_MOD_NMOESI_STORE_ACTION;
extern int EV_MOD_NMOESI_STORE_UNLOCK;
extern int EV_MOD_NMOESI_STORE_FINISH;

extern int EV_MOD_NMOESI_NC_STORE;
extern int EV_MOD_NMOESI_NC_STORE_LOCK;
extern int EV_MOD_NMOESI_NC_STORE_WRITEBACK;
extern int EV_MOD_NMOESI_NC_STORE_ACTION;
extern int EV_MOD_NMOESI_NC_STORE_MISS;
extern int EV_MOD_NMOESI_NC_STORE_UNLOCK;
extern int EV_MOD_NMOESI_NC_STORE_FINISH;

extern int EV_MOD_NMOESI_FIND_AND_LOCK;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_PORT;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
extern int EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;

extern int EV_MOD_NMOESI_EVICT;
extern int EV_MOD_NMOESI_EVICT_INVALID;
extern int EV_MOD_NMOESI_EVICT_ACTION;
extern int EV_MOD_NMOESI_EVICT_RECEIVE;
extern int EV_MOD_NMOESI_EVICT_PROCESS;
extern int EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
extern int EV_MOD_NMOESI_EVICT_REPLY;
extern int EV_MOD_NMOESI_EVICT_REPLY_RECEIVE;
extern int EV_MOD_NMOESI_EVICT_FINISH;

extern int EV_MOD_NMOESI_WRITE_REQUEST;
extern int EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE;
extern int EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
extern int EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
extern int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN;
extern int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
extern int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
extern int EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
extern int EV_MOD_NMOESI_WRITE_REQUEST_FINISH;

extern int EV_MOD_NMOESI_READ_REQUEST;
extern int EV_MOD_NMOESI_READ_REQUEST_RECEIVE;
extern int EV_MOD_NMOESI_READ_REQUEST_ACTION;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS;
extern int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
extern int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
extern int EV_MOD_NMOESI_READ_REQUEST_REPLY;
extern int EV_MOD_NMOESI_READ_REQUEST_FINISH;

extern int EV_MOD_NMOESI_INVALIDATE;
extern int EV_MOD_NMOESI_INVALIDATE_FINISH;

extern int EV_MOD_NMOESI_PEER_SEND;
extern int EV_MOD_NMOESI_PEER_RECEIVE;
extern int EV_MOD_NMOESI_PEER_REPLY;
extern int EV_MOD_NMOESI_PEER_FINISH;

extern int EV_MOD_NMOESI_MESSAGE;
extern int EV_MOD_NMOESI_MESSAGE_RECEIVE;
extern int EV_MOD_NMOESI_MESSAGE_ACTION;
extern int EV_MOD_NMOESI_MESSAGE_REPLY;
extern int EV_MOD_NMOESI_MESSAGE_FINISH;

/* Current identifier for stack */
extern long long mod_stack_id;

/* Read/write request direction */
enum mod_request_dir_t
{
	mod_request_invalid = 0,
	mod_request_up_down,
	mod_request_down_up
};

/* ACK types */
enum mod_reply_type_t
{
	reply_none = 0,
	reply_ack ,
	reply_ack_data,
	reply_ack_data_sent_to_peer,
	reply_ack_error
};

/* Message types */
enum mod_message_type_t
{
	message_none = 0,
	message_clear_owner
};

/* Stack */
struct mod_stack_t
{
	long long id;
	enum mod_access_kind_t access_kind;
	int *witness_ptr;

	struct linked_list_t *event_queue;
	void *event_queue_item;

	struct mod_t *mod;
	struct mod_t *target_mod;
	struct mod_t *except_mod;
	struct mod_t *peer;

	struct mod_port_t *port;

	unsigned int addr;
	int tag;
	int set;
	int way;
	int state;

	int src_set;
	int src_way;
	int src_tag;

	enum mod_request_dir_t request_dir;
	enum mod_message_type_t message;
	enum mod_reply_type_t reply;
	int reply_size;
	int retain_owner;
	int pending;

	/* Linked list of accesses in 'mod' */
	struct mod_stack_t *access_list_prev;
	struct mod_stack_t *access_list_next;

	/* Linked list of write accesses in 'mod' */
	struct mod_stack_t *write_access_list_prev;
	struct mod_stack_t *write_access_list_next;

	/* Bucket list of accesses in hash table in 'mod' */
	struct mod_stack_t *bucket_list_prev;
	struct mod_stack_t *bucket_list_next;

	/* Flags */
	int hit : 1;
	int err : 1;
	int shared : 1;
	int read : 1;
	int write : 1;
	int nc_write : 1;
	int blocking : 1;
	int writeback : 1;
	int eviction : 1;
	int retry : 1;
	int coalesced : 1;
	int port_locked : 1;

	/* Message sent through interconnect */
	struct net_msg_t *msg;

	/* Linked list for waiting events */
	int waiting_list_event;  /* Event to schedule when stack is waken up */
	struct mod_stack_t *waiting_list_prev;
	struct mod_stack_t *waiting_list_next;

	/* Waiting list for locking a port. */
	int port_waiting_list_event;
	struct mod_stack_t *port_waiting_list_prev;
	struct mod_stack_t *port_waiting_list_next;

	/* Waiting list. Contains other stacks waiting for this one to finish.
	 * Waiting stacks corresponds to slave coalesced accesses waiting for
	 * the current one to finish. */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;

	/* Master stack that the current access has been coalesced with.
	 * This field has a value other than NULL only if 'coalesced' is TRUE. */
	struct mod_stack_t *master_stack;

	/* Events waiting in directory lock */
	int dir_lock_event;
	struct mod_stack_t *dir_lock_next;

	/* Return stack */
	struct mod_stack_t *ret_stack;
	int ret_event;
};

extern long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	unsigned int addr, int ret_event, void *ret_stack);
void mod_stack_return(struct mod_stack_t *stack);

void mod_stack_wait_in_mod(struct mod_stack_t *stack,
	struct mod_t *mod, int event);
void mod_stack_wakeup_mod(struct mod_t *mod);

void mod_stack_wait_in_port(struct mod_stack_t *stack,
	struct mod_port_t *port, int event);
void mod_stack_wakeup_port(struct mod_port_t *port);

void mod_stack_wait_in_stack(struct mod_stack_t *stack,
	struct mod_stack_t *master_stack, int event);
void mod_stack_wakeup_stack(struct mod_stack_t *master_stack);

/* NMOESI */
void mod_handler_nmoesi_find_and_lock(int event, void *data);
void mod_handler_nmoesi_load(int event, void *data);
void mod_handler_nmoesi_store(int event, void *data);
void mod_handler_nmoesi_nc_store(int event, void *data);
void mod_handler_nmoesi_evict(int event, void *data);
void mod_handler_nmoesi_write_request(int event, void *data);
void mod_handler_nmoesi_read_request(int event, void *data);
void mod_handler_nmoesi_invalidate(int event, void *data);
void mod_handler_nmoesi_peer(int event, void *data);
void mod_handler_nmoesi_message(int event, void *data);

/* Local memory */
void mod_handler_local_mem_load(int event, void *data);
void mod_handler_local_mem_store(int event, void *data);
void mod_handler_local_mem_find_and_lock(int event, void *data);

extern int mem_system_peer_transfers;


/*
 * Command
 *
 * Event to handle memory hierarchy commands that initialize memory modules to a
 * certain state. These commands are used only for debugging purposes.
 */

extern int EV_MEM_SYSTEM_COMMAND;
extern int EV_MEM_SYSTEM_END_COMMAND;

void mem_system_command_handler(int event, void *data);
void mem_system_end_command_handler(int event, void *data);




/*
 * Memory System
 */

extern char *mem_config_file_name;
extern char *mem_config_help;

extern char *mem_report_file_name;

#define mem_debugging() debug_status(mem_debug_category)
#define mem_debug(...) debug(mem_debug_category, __VA_ARGS__)
extern int mem_debug_category;

#define mem_tracing() trace_status(mem_trace_category)
#define mem_trace(...) trace(mem_trace_category, __VA_ARGS__)
#define mem_trace_header(...) trace_header(mem_trace_category, __VA_ARGS__)
extern int mem_trace_category;

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

struct mod_t *mem_system_get_mod(char *mod_name);
struct net_t *mem_system_get_net(char *net_name);


#endif
