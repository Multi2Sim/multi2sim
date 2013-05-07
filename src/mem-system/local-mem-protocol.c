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
 *  You should have received stack copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>

#include "mem-system.h"
#include "mod-stack.h"


/* Events */

int EV_MOD_LOCAL_MEM_LOAD;
int EV_MOD_LOCAL_MEM_LOAD_LOCK;
int EV_MOD_LOCAL_MEM_LOAD_FINISH;

int EV_MOD_LOCAL_MEM_STORE;
int EV_MOD_LOCAL_MEM_STORE_LOCK;
int EV_MOD_LOCAL_MEM_STORE_FINISH;

int EV_MOD_LOCAL_MEM_FIND_AND_LOCK;
int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT;
int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION;
int EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH;




/* Protocol for Local (Scratchpad) Memory */

void mod_handler_local_mem_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_LOCAL_MEM_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("  %lld %lld 0x%x %s load\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" "
			"state=\"%s:load\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_load);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_load, stack->addr, stack);
		if (master_stack)
		{
			mod->reads++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_LOCAL_MEM_LOAD_FINISH);
			return;
		}

		esim_schedule_event(EV_MOD_LOCAL_MEM_LOAD_LOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_LOCAL_MEM_LOAD_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s load lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock\"\n",
			stack->id, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_LOCAL_MEM_LOAD_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_LOCAL_MEM_LOAD_LOCK);
			return;
		}

		/* Call find and lock to lock the port */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_LOCAL_MEM_LOAD_FINISH, stack);
		new_stack->read = 1;
		esim_schedule_event(EV_MOD_LOCAL_MEM_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_LOCAL_MEM_LOAD_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s load finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n", stack->id);

		/* Increment witness variable */
                if (stack->witness_ptr) {
                        (*stack->witness_ptr)++;
		}

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_local_mem_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_LOCAL_MEM_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s store\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
			"state=\"%s:store\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_store);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_store, stack->addr, stack);
		if (master_stack)
		{
			mod->writes++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_LOCAL_MEM_STORE_FINISH);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_LOCAL_MEM_STORE_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_LOCAL_MEM_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_lock\"\n",
			stack->id, mod->name);

		/* If there is any older access, wait for it */
		older_stack = stack->access_list_prev;
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_LOCAL_MEM_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_LOCAL_MEM_STORE_FINISH, stack);
		new_stack->read = 0;
		new_stack->witness_ptr = stack->witness_ptr;
		esim_schedule_event(EV_MOD_LOCAL_MEM_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_LOCAL_MEM_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n", stack->id);

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_local_mem_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_LOCAL_MEM_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock\n",
			esim_time, stack->id, stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
			stack->id, mod->name);

		/* Get a port */
		mod_lock_port(mod, stack, EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_port\"\n",
			stack->id, mod->name);

		/* Set parent stack flag expressing that port has already been locked.
		 * This flag is checked by new writes to find out if it is already too
		 * late to coalesce. */
		ret->port_locked = 1;

		/* Statistics */
		mod->accesses++;
		if (stack->read)
		{
			mod->reads++;
			mod->effective_reads++;
		}
		else
		{
			mod->writes++;
			mod->effective_writes++;

			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}
		}

		/* Access latency */
		esim_schedule_event(EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION, stack, mod->latency);
		return;
	}

	if (event == EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_action\"\n",
			stack->id, mod->name);

		/* Release port */
		mod_unlock_port(mod, port, stack);

		/* Continue */
		esim_schedule_event(EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_time, stack->id,
			stack->tag, mod->name, stack->err);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_finish\"\n",
			stack->id, mod->name);

		mod_stack_return(stack);
		return;
	}

	abort();
}
