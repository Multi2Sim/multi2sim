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
	for (i = 0; i < mod->bank_count; i++) {
		fprintf(f, "  bank %d:\n", i);
		bank = MOD_BANK_INDEX(mod, i);
		for (j = 0; j < mod->read_port_count; j++) {
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
long long mod_access(struct mod_t *mod, int mod_type, enum mod_access_kind_t access_kind,
	uint32_t addr, int *witness_ptr, struct linked_list_t *event_queue, void *event_queue_item)
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

	/* FIXME - Select CPU/GPU event */
	if (mod_type == 1)
	{
		event = access_kind == mod_access_read ?
			EV_MOD_LOAD : EV_MOD_STORE;
	}
	else if (mod_type == 2)
	{
		event = access_kind == mod_access_read ?
			EV_MOD_GPU_LOAD : EV_MOD_GPU_STORE;
	}

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


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, uint32_t addr)
{
	/* FIXME - for now, just return first module in list */
	/* Check that there is at least one lower-level module */
	if (mod->kind != mod_kind_cache)
		panic("%s: invalid module kind", __FUNCTION__);
	if (!mod->low_mod_list->count)
		fatal("%s: no low memory module", mod->name);

	/* Return first element */
	linked_list_head(mod->low_mod_list);
	return linked_list_get(mod->low_mod_list);
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
