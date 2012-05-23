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


#include <mem-system.h>


/*
 * Private Functions
 */

static int mod_serves_address(struct mod_t *mod, unsigned int addr)
{
	/* Address bounds */
	if (mod->range_kind == mod_range_bounds)
		return addr >= mod->range.bounds.low &&
			addr <= mod->range.bounds.high;

	/* Interleaved addresses */
	if (mod->range_kind == mod_range_interleaved)
		return (addr / mod->range.interleaved.div) %
			mod->range.interleaved.mod ==
			mod->range.interleaved.eq;

	/* Invalid */
	panic("%s: invalid range kind", __FUNCTION__);
	return 0;
}




/*
 * Public Functions
 */

struct mod_t *mod_create(char *name, enum mod_kind_t kind, int num_ports,
	int block_size, int latency)
{
	struct mod_t *mod;

	/* Allocate */
	mod = calloc(1, sizeof(struct mod_t));
	if (!mod)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	mod->name = strdup(name);
	if (!mod->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	mod->kind = kind;
	mod->latency = latency;

	/* Ports */
	mod->num_ports = num_ports;
	mod->ports = calloc(num_ports, sizeof(struct mod_port_t));
	if (!mod->ports)
		fatal("%s: out of memory", __FUNCTION__);


	/* Lists */
	mod->low_mod_list = linked_list_create();
	mod->high_mod_list = linked_list_create();

	/* Block size */
	mod->block_size = block_size;
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	mod->log_block_size = log_base2(block_size);

	return mod;
}


void mod_free(struct mod_t *mod)
{
	linked_list_free(mod->low_mod_list);
	linked_list_free(mod->high_mod_list);
	if (mod->cache)
		cache_free(mod->cache);
	if (mod->dir)
		dir_free(mod->dir);
	free(mod->ports);
	free(mod->name);
	free(mod);
}


void mod_dump(struct mod_t *mod, FILE *f)
{
}


/* Access a memory module.
 * Variable 'witness', if specified, will be increased when the access completes.
 * The function returns a unique access ID.
 */
long long mod_access(struct mod_t *mod, enum mod_access_kind_t access_kind, 
	unsigned int addr, int *witness_ptr, struct linked_list_t *event_queue,
	void *event_queue_item)
{
	struct mod_stack_t *stack;
	int event;

	/* Create module stack with new ID */
	mod_stack_id++;
	stack = mod_stack_create(mod_stack_id,
		mod, addr, ESIM_EV_NONE, NULL);

	/* Initialize */
	stack->witness_ptr = witness_ptr;
	stack->event_queue = event_queue;
	stack->event_queue_item = event_queue_item;

	/* Select initial CPU/GPU event */
	if (access_kind == mod_access_read)
	{
		event = EV_MOD_LOAD;
	}
	else if (access_kind == mod_access_write)
	{
		event = EV_MOD_STORE;
	}
	else if (access_kind == mod_access_nc_write)
	{
		// TODO Change this to EV_MOD_NC_STORE after updating the protocol
		event = EV_MOD_STORE;
	}
	else
		panic("%s: invalid entry kind", __FUNCTION__);

	/* Schedule */
	esim_execute_event(event, stack);

	/* Return access ID */
	return stack->id;
}


/* Return true if module can be accessed. */
int mod_can_access(struct mod_t *mod, unsigned int addr)
{
	int non_coalesced_accesses;

	/* There must be a free port */
	assert(mod->num_locked_ports <= mod->num_ports);
	if (mod->num_locked_ports == mod->num_ports)
		return 0;

	/* If no MSHR is given, module can be accessed */
	if (!mod->mshr_size)
		return 1;

	/* Module can be accessed if number of non-coalesced in-flight
	 * accesses is smaller than the MSHR size. */
	non_coalesced_accesses = mod->access_list_count -
		mod->access_list_coalesced_count;
	return non_coalesced_accesses < mod->mshr_size;
}


/* Return {set, way, tag, state} for an address.
 * The function returns TRUE on hit, FALSE on miss. */
int mod_find_block(struct mod_t *mod, unsigned int addr, unsigned int *set_ptr,
	unsigned int *way_ptr, unsigned int *tag_ptr, int *state_ptr)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;

	unsigned int set;
	unsigned int way;
	unsigned int tag;

	/* A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->block_mask;
	if (mod->range_kind == mod_range_interleaved)
	{
		unsigned int num_mods = mod->range.interleaved.mod;
		set = ((tag >> cache->log_block_size) / num_mods) % cache->num_sets;
	}
	else if (mod->range_kind == mod_range_bounds)
	{
		set = (tag >> cache->log_block_size) % cache->num_sets;
	}
	else 
	{
		panic("%s: invalid range kind (%d)", __FUNCTION__, mod->range_kind);
	}

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
		PTR_ASSIGN(set_ptr, set);
		PTR_ASSIGN(tag_ptr, tag);
		PTR_ASSIGN(way_ptr, 0);
		PTR_ASSIGN(state_ptr, 0);
		return 0;
	}

	/* Hit */
	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(tag_ptr, tag);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}


/* Lock a port, and schedule event when done.
 * If there is no free port, the access is enqueued in the port
 * waiting list, and it will retry once a port becomes available with a
 * call to 'mod_unlock_port'. */
void mod_lock_port(struct mod_t *mod, struct mod_stack_t *stack, int event)
{
	struct mod_port_t *port = NULL;
	int i;

	/* No free port */
	if (mod->num_locked_ports >= mod->num_ports)
	{
		assert(!DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, port_waiting, stack);
		stack->port_waiting_list_event = event;
		return;
	}

	/* Get free port */
	for (i = 0; i < mod->num_ports; i++)
	{
		port = &mod->ports[i];
		if (!port->stack)
			break;
	}

	/* Lock port */
	assert(port && i < mod->num_ports);
	port->stack = stack;
	stack->port = port;
	mod->num_locked_ports++;

	/* Debug */
	mem_debug("  %lld %lld %s port %d locked\n", esim_cycle,
		stack->id, mod->name, i);

	/* Schedule event */
	esim_schedule_event(event, stack, 0);
}


void mod_unlock_port(struct mod_t *mod, struct mod_port_t *port,
	struct mod_stack_t *stack)
{
	int event;

	/* Checks */
	assert(mod->num_locked_ports > 0);
	assert(stack->port == port && port->stack == stack);
	assert(stack->mod == mod);

	/* Unlock port */
	stack->port = NULL;
	port->stack = NULL;
	mod->num_locked_ports--;

	/* Debug */
	mem_debug("  %lld %lld %s port unlocked\n", esim_cycle,
		stack->id, mod->name);

	/* Check if there was any access waiting for free port */
	if (!mod->port_waiting_list_count)
		return;

	/* Wake up one access waiting for a free port */
	stack = mod->port_waiting_list_head;
	event = stack->port_waiting_list_event;
	assert(DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));
	DOUBLE_LINKED_LIST_REMOVE(mod, port_waiting, stack);
	mod_lock_port(mod, stack, event);

}


void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack,
	enum mod_access_kind_t access_kind)
{
	int index;

	/* Record access kind */
	stack->access_kind = access_kind;

	/* Insert in access list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, access, stack);

	/* Insert in write access list */
	if (access_kind == mod_access_write)
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, write_access, stack);

	/* Insert in access hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_INSERT_TAIL(&mod->access_hash_table[index], bucket, stack);
}


void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack)
{
	int index;

	/* Remove from access list */
	DOUBLE_LINKED_LIST_REMOVE(mod, access, stack);

	/* Remove from write access list */
	assert(stack->access_kind);
	if (stack->access_kind == mod_access_write)
		DOUBLE_LINKED_LIST_REMOVE(mod, write_access, stack);

	/* Remove from hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_REMOVE(&mod->access_hash_table[index], bucket, stack);

	/* If this was a coalesced access, update counter */
	if (stack->coalesced)
	{
		assert(mod->access_list_coalesced_count > 0);
		mod->access_list_coalesced_count--;
	}
}


/* Return true if the access with identifier 'id' is in flight.
 * The address of the access is passed as well because this lookup is done on the
 * access truth table, indexed by the access address.
 */
int mod_in_flight_access(struct mod_t *mod, long long id, unsigned int addr)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for access */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack; stack = stack->bucket_list_next)
		if (stack->id == id)
			return 1;

	/* Not found */
	return 0;
}


/* Return the youngest in-flight access older than 'older_than_stack' to block containing 'addr'.
 * If 'older_than_stack' is NULL, return the youngest in-flight access containing 'addr'.
 * The function returns NULL if there is no in-flight access to block containing 'addr'.
 */
struct mod_stack_t *mod_in_flight_address(struct mod_t *mod, unsigned int addr,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for address */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack;
		stack = stack->bucket_list_next)
	{
		/* This stack is not older than 'older_than_stack' */
		if (older_than_stack && stack->id >= older_than_stack->id)
			continue;

		/* Address matches */
		if (stack->addr >> mod->log_block_size == addr >> mod->log_block_size)
			return stack;
	}

	/* Not found */
	return NULL;
}


/* Return the youngest in-flight write older than 'older_than_stack'. If 'older_than_stack'
 * is NULL, return the youngest in-flight write. Return NULL if there is no in-flight write.
 */
struct mod_stack_t *mod_in_flight_write(struct mod_t *mod,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;

	/* No 'older_than_stack' given, return youngest write */
	if (!older_than_stack)
		return mod->write_access_list_tail;

	/* Search */
	for (stack = older_than_stack->access_list_prev; stack;
		stack = stack->access_list_prev)
		if (stack->access_kind == mod_access_write)
			return stack;

	/* Not found */
	return NULL;
}


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, unsigned int addr)
{
	struct mod_t *low_mod;
	struct mod_t *server_mod;

	/* Main memory does not have a low module */
	assert(mod_serves_address(mod, addr));
	if (mod->kind == mod_kind_main_memory)
	{
		assert(!linked_list_count(mod->low_mod_list));
		return NULL;
	}

	/* Check which low module serves address */
	server_mod = NULL;
	LINKED_LIST_FOR_EACH(mod->low_mod_list)
	{
		/* Get new low module */
		low_mod = linked_list_get(mod->low_mod_list);
		if (!mod_serves_address(low_mod, addr))
			continue;

		/* Address served by more than one module */
		if (server_mod)
			fatal("%s: low modules %s and %s both serve address 0x%x",
				mod->name, server_mod->name, low_mod->name, addr);

		/* Assign server */
		server_mod = low_mod;
	}

	/* Error if no low module serves address */
	if (!server_mod)
		fatal("module %s: no lower module serves address 0x%x",
			mod->name, addr);

	/* Return server module */
	return server_mod;
}


int mod_get_retry_latency(struct mod_t *mod)
{
	return random() % mod->latency + mod->latency;
}


/* Check if an access to a module can be coalesced with another access older
 * than 'older_than_stack'. If 'older_than_stack' is NULL, check if it can
 * be coalesced with any in-flight access.
 * If it can, return the access that it would be coalesced with. Otherwise,
 * return NULL. */
struct mod_stack_t *mod_can_coalesce(struct mod_t *mod,
	enum mod_access_kind_t access_kind, unsigned int addr,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	struct mod_stack_t *tail;

	/* For efficiency, first check in the hash table of accesses
	 * whether there is an access in flight to the same block. */
	assert(access_kind);
	if (!mod_in_flight_address(mod, addr, older_than_stack))
		return NULL;

	/* Get youngest access older than 'older_than_stack' */
	tail = older_than_stack ? older_than_stack->access_list_prev :
		mod->access_list_tail;

	/* Coalesce depending on access kind */
	switch (access_kind)
	{

	case mod_access_read:
	{
		for (stack = tail; stack; stack = stack->access_list_prev)
		{
			/* Only coalesce with groups of reads at the tail */
			if (stack->access_kind != mod_access_read)
				return NULL;

			if (stack->addr >> mod->log_block_size ==
				addr >> mod->log_block_size)
				return stack->master_stack ? stack->master_stack : stack;
		}
		break;
	}

	case mod_access_write:
	{
		/* Only coalesce with last access */
		stack = tail;
		if (!stack)
			return NULL;

		/* Only if it is a write */
		if (stack->access_kind != mod_access_write)
			return NULL;

		/* Only if it is an access to the same block */
		if (stack->addr >> mod->log_block_size != addr >> mod->log_block_size)
			return NULL;

		/* Only if previous write has not started yet */
		if (stack->port_locked)
			return NULL;

		/* Coalesce */
		return stack->master_stack ? stack->master_stack : stack;
	}

	default:
		panic("%s: invalid access type", __FUNCTION__);
		break;
	}

	/* No access found */
	return NULL;
}


void mod_coalesce(struct mod_t *mod, struct mod_stack_t *master_stack,
	struct mod_stack_t *stack)
{
	/* Debug */
	mem_debug("  %lld %lld 0x%x %s coalesce with %lld\n", esim_cycle,
		stack->id, stack->addr, mod->name, master_stack->id);

	/* Master stack must not have a parent. We only want one level of
	 * coalesced accesses. */
	assert(!master_stack->master_stack);

	/* Access must have been recorded already, which sets the access
	 * kind to a valid value. */
	assert(stack->access_kind);

	/* Set slave stack as a coalesced access */
	stack->coalesced = 1;
	stack->master_stack = master_stack;
	assert(mod->access_list_coalesced_count <= mod->access_list_count);

	/* Record in-flight coalesced access in module */
	mod->access_list_coalesced_count++;
}




/*
 * Memory module access stack (for event-driven simulation)
 */

long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	unsigned int addr, int ret_event, void *ret_stack)
{
	struct mod_stack_t *stack;

	/* Create stack */
	stack = calloc(1, sizeof(struct mod_stack_t));
	if (!stack)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	stack->id = id;
	stack->mod = mod;
	stack->addr = addr;
	stack->peer = NULL;
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;
	stack->reply = reply_NO_REPLY;

	/* Return */
	return stack;
}


void mod_stack_return(struct mod_stack_t *stack)
{
	int ret_event = stack->ret_event;
	void *ret_stack = stack->ret_stack;

	/* Wake up dependent accesses */
	mod_stack_wakeup_stack(stack);

	/* Free */
	free(stack);
	esim_schedule_event(ret_event, ret_stack, 0);
}


/* Enqueue access in module wait list. */
void mod_stack_wait_in_mod(struct mod_stack_t *stack,
	struct mod_t *mod, int event)
{
	assert(mod == stack->mod);
	assert(!DOUBLE_LINKED_LIST_MEMBER(mod, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, waiting, stack);
}


/* Wake up accesses waiting in module wait list. */
void mod_stack_wakeup_mod(struct mod_t *mod)
{
	struct mod_stack_t *stack;
	int event;

	while (mod->waiting_list_head)
	{
		stack = mod->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(mod, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/* Enqueue access in port wait list. */
void mod_stack_wait_in_port(struct mod_stack_t *stack,
	struct mod_port_t *port, int event)
{
	assert(port == stack->port);
	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


/* Wake up accesses waiting in a port wait list. */
void mod_stack_wakeup_port(struct mod_port_t *port)
{
	struct mod_stack_t *stack;
	int event;

	while (port->waiting_list_head)
	{
		stack = port->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(port, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/* Enqueue access in stack wait list. */
void mod_stack_wait_in_stack(struct mod_stack_t *stack,
	struct mod_stack_t *master_stack, int event)
{
	assert(master_stack != stack);
	assert(!DOUBLE_LINKED_LIST_MEMBER(master_stack, waiting, stack));

	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(master_stack, waiting, stack);
}


/* Wake up access waiting in a stack's wait list. */
void mod_stack_wakeup_stack(struct mod_stack_t *master_stack)
{
	struct mod_stack_t *stack;
	int event;

	/* No access to wake up */
	if (!master_stack->waiting_list_count)
		return;

	/* Debug */
	mem_debug("  %lld %lld 0x%x wake up accesses:", esim_cycle,
		master_stack->id, master_stack->addr);

	/* Wake up all coalesced accesses */
	while (master_stack->waiting_list_head)
	{
		stack = master_stack->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(master_stack, waiting, stack);
		esim_schedule_event(event, stack, 0);
		mem_debug(" %lld", stack->id);
	}

	/* Debug */
	mem_debug("\n");
}

