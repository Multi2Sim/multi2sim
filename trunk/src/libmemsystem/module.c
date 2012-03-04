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
 * Public Functions
 */

struct mod_t *mod_create(char *name, enum mod_kind_t kind,
	int bank_count, int read_port_count, int write_port_count,
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
	mod->bank_count = bank_count;
	mod->read_port_count = read_port_count;
	mod->write_port_count = write_port_count;
	mod->banks = calloc(1, mod->bank_count * SIZEOF_MOD_BANK(mod));
	mod->latency = latency;

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
	free(mod->banks);
	free(mod->name);
	free(mod);
}


void mod_dump(struct mod_t *mod, FILE *f)
{
	struct mod_bank_t *bank;
	struct mod_port_t *port;
	struct mod_stack_t *stack;
	int i, j;

	/* Read ports */
	fprintf(f, "module '%s'\n", mod->name);
	for (i = 0; i < mod->bank_count; i++)
	{
		fprintf(f, "  bank %d:\n", i);
		bank = MOD_BANK_INDEX(mod, i);
		for (j = 0; j < mod->read_port_count; j++)
		{
			port = MOD_READ_PORT_INDEX(mod, bank, j);
			fprintf(f, "  read port %d: ", j);

			/* Waiting list */
			fprintf(f, "waiting={");
			for (stack = port->waiting_list_head; stack; stack = stack->waiting_list_next)
				fprintf(f, " %lld", stack->id);
			fprintf(f, " }\n");
		}
	}
}


/* Access a memory module.
 * Variable 'witness', if specified, will be increased when the access completes.
 * The function returns a unique access ID.
 */
long long mod_access(struct mod_t *mod, enum mod_entry_kind_t entry_kind,
	enum mod_access_kind_t access_kind, uint32_t addr, int *witness_ptr,
	struct linked_list_t *event_queue, void *event_queue_item)
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
	if (entry_kind == mod_entry_cpu)
	{
		event = access_kind == mod_access_read ?
			EV_MOD_LOAD : EV_MOD_STORE;
	}
	else if (entry_kind == mod_entry_gpu)
	{
		event = access_kind == mod_access_read ?
			EV_MOD_GPU_LOAD : EV_MOD_GPU_STORE;
	}
	else
		panic("%s: invalid entry kind", __FUNCTION__);

	/* Schedule */
	esim_execute_event(event, stack);

	/* Return access ID */
	return stack->id;
}


/* Return true if module can be accessed. */
int mod_can_access(struct mod_t *mod, uint32_t addr)
{
	/* FIXME */
	return mod->access_list_count < 4;
}


/* Return {set, way, tag, state} for an address.
 * The function returns TRUE on hit, FALSE on miss. */
int mod_find_block(struct mod_t *mod, uint32_t addr, uint32_t *set_ptr,
	uint32_t *way_ptr, uint32_t *tag_ptr, int *state_ptr)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;

	uint32_t set;
	uint32_t way;
	uint32_t tag;

	/* A transient tag is considered a hit if the block is
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


void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack)
{
	int index;

	/* Insert in access list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, access, stack);

	/* Insert in access hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_INSERT_TAIL(&mod->access_hash_table[index], bucket, stack);
}


void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack)
{
	int index;

	/* Remove from access list */
	DOUBLE_LINKED_LIST_REMOVE(mod, access, stack);

	/* Remove from hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_REMOVE(&mod->access_hash_table[index], bucket, stack);
}


/* Return true if the access with identifier 'id' is in flight.
 * The address of the access is passed as well because this lookup is done on the
 * access truth table, indexed by the access address.
 */
int mod_access_in_flight(struct mod_t *mod, long long id, uint32_t addr)
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


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, uint32_t addr)
{
	/* Main memory does not have a low module */
	if (mod->kind == mod_kind_main_memory)
	{
		assert(!linked_list_count(mod->low_mod_list));
		return NULL;
	}

	/* FIXME - not supported for more than one lower module */
	if (linked_list_count(mod->low_mod_list) != 1)
		panic("%s: not supported for more than 1 low node", __FUNCTION__);

	/* FIXME - return the only lower module */
	linked_list_head(mod->low_mod_list);
	return linked_list_get(mod->low_mod_list);
}


int mod_get_retry_latency(struct mod_t *mod)
{
	return random() % mod->latency + mod->latency;
}




/*
 * Memory module access stack (for event-driven simulation)
 */

long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	uint32_t addr, int ret_event, void *ret_stack)
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
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;

	/* Return */
	return stack;
}


void mod_stack_return(struct mod_stack_t *stack)
{
	int ret_event = stack->ret_event;
	void *ret_stack = stack->ret_stack;

	free(stack);
	esim_schedule_event(ret_event, ret_stack, 0);
}


/* Enqueue stack in waiting list of 'stack->mod' */
void mod_stack_wait_in_mod(struct mod_stack_t *stack, int event)
{
	struct mod_t *mod = stack->mod;

	assert(!DOUBLE_LINKED_LIST_MEMBER(mod, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, waiting, stack);
}


/* Wake up accesses from 'mod->waiting_list' */
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


/* Enqueue stack in waiting list of 'stack->port' */
void mod_stack_wait_in_port(struct mod_stack_t *stack, int event)
{
	struct mod_port_t *port = stack->port;

	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


/* Wake up accesses from 'port->waiting_list' */
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
