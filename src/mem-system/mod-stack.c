/*
 *  Multi2Sim
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

#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include "cache.h"
#include "mem-system.h"
#include "mod-stack.h"


long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	unsigned int addr, int ret_event, struct mod_stack_t *ret_stack)
{
	struct mod_stack_t *stack;

	/* Initialize */
	stack = xcalloc(1, sizeof(struct mod_stack_t));
	stack->id = id;
	stack->mod = mod;
	stack->addr = addr;
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;
	if (ret_stack != NULL)
		stack->client_info = ret_stack->client_info;
	stack->way = -1;
	stack->set = -1;
	stack->tag = -1;

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
	mem_debug("  %lld %lld 0x%x wake up accesses:", esim_time,
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

/* Set a reply value that has a precedence order.  This is required
 * when multiple subblocks all return replies.  An alternative would
 * be to store each reply and scan them all before deciding an action. */
void mod_stack_set_reply(struct mod_stack_t *stack, int reply)
{
	if (reply > stack->reply)
	{
		stack->reply = reply;
	}
}

/* Peer-peer transfers are always used when a block is in the owned state,
 * otherwise it is based on a configuration argument */
struct mod_t *mod_stack_set_peer(struct mod_t *peer, int state)
{
	struct mod_t *ret = NULL;

	if (state == cache_block_owned || mem_peer_transfers)
		ret = peer;	

	return ret;
}

