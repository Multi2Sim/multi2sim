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
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <network/network.h>
#include <network/node.h>

#include "cache.h"
#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "prefetcher.h"


/* Events */

int EV_MOD_NMOESI_LOAD;
int EV_MOD_NMOESI_LOAD_LOCK;
int EV_MOD_NMOESI_LOAD_ACTION;
int EV_MOD_NMOESI_LOAD_MISS;
int EV_MOD_NMOESI_LOAD_UNLOCK;
int EV_MOD_NMOESI_LOAD_FINISH;

int EV_MOD_NMOESI_STORE;
int EV_MOD_NMOESI_STORE_LOCK;
int EV_MOD_NMOESI_STORE_ACTION;
int EV_MOD_NMOESI_STORE_UNLOCK;
int EV_MOD_NMOESI_STORE_FINISH;

int EV_MOD_NMOESI_PREFETCH;
int EV_MOD_NMOESI_PREFETCH_LOCK;
int EV_MOD_NMOESI_PREFETCH_ACTION;
int EV_MOD_NMOESI_PREFETCH_MISS;
int EV_MOD_NMOESI_PREFETCH_UNLOCK;
int EV_MOD_NMOESI_PREFETCH_FINISH;

int EV_MOD_NMOESI_NC_STORE;
int EV_MOD_NMOESI_NC_STORE_LOCK;
int EV_MOD_NMOESI_NC_STORE_WRITEBACK;
int EV_MOD_NMOESI_NC_STORE_ACTION;
int EV_MOD_NMOESI_NC_STORE_MISS;
int EV_MOD_NMOESI_NC_STORE_UNLOCK;
int EV_MOD_NMOESI_NC_STORE_FINISH;

int EV_MOD_NMOESI_FIND_AND_LOCK;
int EV_MOD_NMOESI_FIND_AND_LOCK_PORT;
int EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
int EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;

int EV_MOD_NMOESI_EVICT;
int EV_MOD_NMOESI_EVICT_INVALID;
int EV_MOD_NMOESI_EVICT_ACTION;
int EV_MOD_NMOESI_EVICT_RECEIVE;
int EV_MOD_NMOESI_EVICT_PROCESS;
int EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
int EV_MOD_NMOESI_EVICT_REPLY;
int EV_MOD_NMOESI_EVICT_REPLY_RECEIVE;
int EV_MOD_NMOESI_EVICT_FINISH;

int EV_MOD_NMOESI_WRITE_REQUEST;
int EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
int EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
int EV_MOD_NMOESI_WRITE_REQUEST_FINISH;

int EV_MOD_NMOESI_READ_REQUEST;
int EV_MOD_NMOESI_READ_REQUEST_RECEIVE;
int EV_MOD_NMOESI_READ_REQUEST_ACTION;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_REPLY;
int EV_MOD_NMOESI_READ_REQUEST_FINISH;

int EV_MOD_NMOESI_INVALIDATE;
int EV_MOD_NMOESI_INVALIDATE_FINISH;

int EV_MOD_NMOESI_PEER_SEND;
int EV_MOD_NMOESI_PEER_RECEIVE;
int EV_MOD_NMOESI_PEER_REPLY;
int EV_MOD_NMOESI_PEER_FINISH;

int EV_MOD_NMOESI_MESSAGE;
int EV_MOD_NMOESI_MESSAGE_RECEIVE;
int EV_MOD_NMOESI_MESSAGE_ACTION;
int EV_MOD_NMOESI_MESSAGE_REPLY;
int EV_MOD_NMOESI_MESSAGE_FINISH;





/* NMOESI Protocol */

void mod_handler_nmoesi_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_time, stack->id,
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
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_LOAD_FINISH);
			return;
		}

		/* Next event */
		esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_LOCK)
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
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_LOAD_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->read = 1;
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load action\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_action\"\n",
			stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);
			return;
		}

		/* Hit */
		if (stack->state)
		{
			esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			prefetcher_access_hit(stack, mod);

			return;
		}

		/* Miss */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
			EV_MOD_NMOESI_LOAD_MISS, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

		/* The prefetcher may be interested in this miss */
		prefetcher_access_miss(stack, mod);

		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load miss\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_miss\"\n",
			stack->id, mod->name);

		/* Error on read request. Unlock block and retry load. */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
			stack->shared ? cache_block_shared : cache_block_exclusive);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_unlock\"\n",
			stack->id, mod->name);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Impose the access latency before continuing */
		esim_schedule_event(EV_MOD_NMOESI_LOAD_FINISH, stack, 
			mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s load finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_STORE)
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
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_STORE_FINISH);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_NMOESI_STORE_LOCK)
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
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_STORE_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->write = 1;
		new_stack->retry = stack->retry;
		new_stack->witness_ptr = stack->witness_ptr;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store action\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_action\"\n",
			stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Hit - state=M/E */
		if (stack->state == cache_block_modified ||
			stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_NMOESI_STORE_UNLOCK, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			prefetcher_access_hit(stack, mod);

			return;
		}

		/* Miss - state=O/S/I/N */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
			EV_MOD_NMOESI_STORE_UNLOCK, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);

		/* The prefetcher may be interested in this miss */
		prefetcher_access_miss(stack, mod);

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_UNLOCK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_unlock\"\n",
			stack->id, mod->name);

		/* Error in write request, unlock block and retry store. */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Update tag/state and unlock */
		cache_set_block(mod->cache, stack->set, stack->way,
			stack->tag, cache_block_modified);
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Impose the access latency before continuing */
		esim_schedule_event(EV_MOD_NMOESI_STORE_FINISH, stack, 
			mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_nc_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_NC_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s nc store\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"nc_store\" "
			"state=\"%s:nc store\" addr=0x%x\n", stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_nc_store);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_nc_store, stack->addr, stack);
		if (master_stack)
		{
			mod->nc_writes++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_NC_STORE_FINISH);
			return;
		}

		/* Next event */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s nc store lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_lock\"\n",
			stack->id, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_NC_STORE_WRITEBACK, stack);
		new_stack->blocking = 1;
		new_stack->nc_write = 1;
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_WRITEBACK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store writeback\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_writeback\"\n",
			stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* If the block has modified data, evict it so that the lower-level cache will
		 * have the latest copy */
		if (stack->state == cache_block_modified || stack->state == cache_block_owned)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod, 0,
				EV_MOD_NMOESI_NC_STORE_ACTION, stack);
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);
			return;
		}

		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store action\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_action\"\n",
			stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Main memory modules are a special case */
		if (mod->kind == mod_kind_main_memory)
		{
			/* For non-coherent stores, finding an E or M for the state of
			 * a cache block in the directory still requires a message to 
			 * the lower-level module so it can update its owner field.
			 * These messages should not be sent if the module is a main
			 * memory module. */
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
			return;
		}

		/* N/S are hit */
		if (stack->state == cache_block_shared || stack->state == cache_block_noncoherent)
		{
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		}
		/* E state must tell the lower-level module to remove this module as an owner */
		else if (stack->state == cache_block_exclusive)
		{
			new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_NC_STORE_MISS, stack);
			new_stack->message = message_clear_owner;
			new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE, new_stack, 0);
		}
		/* Modified and Owned states need to call read request because we've already
		 * evicted the block so that the lower-level cache will have the latest value
		 * before it becomes non-coherent */
		else
		{
			new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_NC_STORE_MISS, stack);
			new_stack->peer = mod_stack_set_peer(mod, stack->state);
			new_stack->nc_write = 1;
			new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store miss\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_miss\"\n",
			stack->id, mod->name);

		/* Error on read request. Unlock block and retry nc store. */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s nc store unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_unlock\"\n",
			stack->id, mod->name);

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
			cache_block_noncoherent);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Impose the access latency before continuing */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_FINISH, stack, 
			mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s nc store finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_prefetch(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_PREFETCH)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
			"state=\"%s:prefetch\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_prefetch);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_prefetch, stack->addr, stack);
		if (master_stack)
		{
			/* Doesn't make sense to prefetch as the block is already being fetched */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - already being fetched\n",
				  esim_time, stack->id, stack->addr, mod->name);

			mod->useless_prefetches++;
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_NMOESI_PREFETCH_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s prefetch lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_lock\"\n",
			stack->id, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_PREFETCH_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_PREFETCH_ACTION, stack);
		new_stack->blocking = 0;
		new_stack->prefetch = 1;
		new_stack->retry = 0;
		new_stack->witness_ptr = stack->witness_ptr;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch action\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_action\"\n",
			stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if getting a lock failed. 
			Effectively this means that prefetches are of low priority.
			This can be improved to not retry only when the current lock
			holder is writing to the block. */
			mod->prefetch_aborts++;
			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Hit */
		if (stack->state)
		{
			/* block already in the cache */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - cache hit\n",
				  esim_time, stack->id, stack->addr, mod->name);

			mod->useless_prefetches++;
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
			return;
		}

		/* Miss */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
			EV_MOD_NMOESI_PREFETCH_MISS, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		new_stack->prefetch = 1;
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		return;
	}
	if (event == EV_MOD_NMOESI_PREFETCH_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch miss\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_miss\"\n",
			stack->id, mod->name);

		/* Error on read request. Unlock block and abort. */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if read request failed. 
			 * Effectively this means that prefetches are of low priority.
			 * This can be improved depending on the reason for read request fail */
			mod->prefetch_aborts++;
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
			stack->shared ? cache_block_shared : cache_block_exclusive);

		/* Mark the prefetched block as prefetched. This is needed to let the 
		 * prefetcher know about an actual access to this block so that it
		 * is aware of all misses as they would be without the prefetcher. 
		 * TODO: The lower caches that will be filled because of this prefetch
		 * do not know if it was a prefetch or not. Need to have a way to mark
		 * them as prefetched too. */
		mod_block_set_prefetched(mod, stack->addr, 1);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_unlock\"\n",
			stack->id, mod->name);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
			esim_time, stack->id, stack->addr, mod->name, stack->blocking);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
			stack->id, mod->name);

		/* Default return values */
		ret->err = 0;

		/* If this stack has already been assigned a way, keep using it */
		stack->way = ret->way;

		/* Get a port */
		mod_lock_port(mod, stack, EV_MOD_NMOESI_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_PORT)
	{
		struct mod_port_t *port = stack->port;
		struct dir_lock_t *dir_lock;

		assert(stack->port);
		mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_port\"\n",
			stack->id, mod->name);

		/* Set parent stack flag expressing that port has already been locked.
		 * This flag is checked by new writes to find out if it is already too
		 * late to coalesce. */
		ret->port_locked = 1;

		/* Look for block. */
		stack->hit = mod_find_block(mod, stack->addr, &stack->set,
			&stack->way, &stack->tag, &stack->state);

		/* Debug */
		if (stack->hit)
			mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, state=%s\n", stack->id,
				stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));

		/* Statistics */
		mod->accesses++;
		if (stack->hit)
			mod->hits++;

		if (stack->read)
		{
			mod->reads++;
			mod->effective_reads++;
			stack->blocking ? mod->blocking_reads++ : mod->non_blocking_reads++;
			if (stack->hit)
				mod->read_hits++;
		}
		else if (stack->prefetch)
		{
			mod->prefetches++;
		}
		else if (stack->nc_write)  /* Must go after read */
		{
			mod->nc_writes++;
			mod->effective_nc_writes++;
			stack->blocking ? mod->blocking_nc_writes++ : mod->non_blocking_nc_writes++;
			if (stack->hit)
				mod->nc_write_hits++;
		}
		else if (stack->write)
		{
			mod->writes++;
			mod->effective_writes++;
			stack->blocking ? mod->blocking_writes++ : mod->non_blocking_writes++;

			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}

			if (stack->hit)
				mod->write_hits++;
		}
		else if (stack->message)
		{
			/* FIXME */
		}
		else 
		{
			fatal("Unknown memory operation type");
		}

		if (!stack->retry)
		{
			mod->no_retry_accesses++;
			if (stack->hit)
				mod->no_retry_hits++;
			
			if (stack->read)
			{
				mod->no_retry_reads++;
				if (stack->hit)
					mod->no_retry_read_hits++;
			}
			else if (stack->nc_write)  /* Must go after read */
			{
				mod->no_retry_nc_writes++;
				if (stack->hit)
					mod->no_retry_nc_write_hits++;
			}
			else if (stack->write)
			{
				mod->no_retry_writes++;
				if (stack->hit)
					mod->no_retry_write_hits++;
			}
			else if (stack->prefetch)
			{
				/* No retries currently for prefetches */
			}
			else if (stack->message)
			{
				/* FIXME */
			}
			else 
			{
				fatal("Unknown memory operation type");
			}
		}

		if (!stack->hit)
		{
			/* Find victim */
			if (stack->way < 0) 
			{
				stack->way = cache_replace_block(mod->cache, stack->set);
			}
		}
		assert(stack->way >= 0);

		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
		dir_lock = dir_lock_get(mod->dir, stack->set, stack->way);
		if (dir_lock->lock && !stack->blocking)
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - aborting\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			ret->err = 1;
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;
			mod_stack_return(stack);
			return;
		}

		/* Lock directory entry. If lock fails, port needs to be released to prevent 
		 * deadlock.  When the directory entry is released, locking port and 
		 * directory entry will be retried. */
		if (!dir_entry_lock(mod->dir, stack->set, stack->way, EV_MOD_NMOESI_FIND_AND_LOCK, 
			stack))
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - waiting\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;
			return;
		}

		/* Miss */
		if (!stack->hit)
		{
			/* Find victim */
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
				stack->set, stack->way));
			mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));
		}


		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought.
		 * Also, update LRU counters here. */
		cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
		cache_access_block(mod->cache, stack->set, stack->way);

		/* Access latency */
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_ACTION, stack, mod->dir_latency);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_action\"\n",
			stack->id, mod->name);

		/* Release port */
		mod_unlock_port(mod, port, stack);
		ret->port_locked = 0;

		/* On miss, evict if victim is a valid block. */
		if (!stack->hit && stack->state)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod, 0,
				EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack);
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_time, stack->id,
			stack->tag, mod->name, stack->err);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_finish\"\n",
			stack->id, mod->name);

		/* If evict produced err, return err */
		if (stack->err)
		{
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state);
			assert(stack->eviction);
			ret->err = 1;
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mod_stack_return(stack);
			return;
		}

		/* Eviction */
		if (stack->eviction)
		{
			mod->evictions++;
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(!stack->state);
		}

		/* If this is a main memory, the block is here. A previous miss was just a miss
		 * in the directory. */
		if (mod->kind == mod_kind_main_memory && !stack->state)
		{
			stack->state = cache_block_exclusive;
			cache_set_block(mod->cache, stack->set, stack->way,
				stack->tag, stack->state);
		}

		/* Return */
		ret->err = 0;
		ret->set = stack->set;
		ret->way = stack->way;
		ret->state = stack->state;
		ret->tag = stack->tag;
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_evict(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_NMOESI_EVICT)
	{
		/* Default return value */
		ret->err = 0;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
			stack->set, stack->way));
		mem_debug("  %lld %lld 0x%x %s evict (set=%d, way=%d, state=%s)\n", esim_time, stack->id,
			stack->tag, mod->name, stack->set, stack->way,
			str_map_value(&cache_block_state_map, stack->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict\"\n",
			stack->id, mod->name);

		/* Save some data */
		stack->src_set = stack->set;
		stack->src_way = stack->way;
		stack->src_tag = stack->tag;
		stack->target_mod = mod_get_low_mod(mod, stack->tag);

		/* Send write request to all sharers */
		new_stack = mod_stack_create(stack->id, mod, 0, EV_MOD_NMOESI_EVICT_INVALID, stack);
		new_stack->except_mod = NULL;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_INVALID)
	{
		mem_debug("  %lld %lld 0x%x %s evict invalid\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_invalid\"\n",
			stack->id, mod->name);

		/* If module is main memory, we just need to set the block as invalid, 
		 * and finish. */
		if (mod->kind == mod_kind_main_memory)
		{
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
				0, cache_block_invalid);
			esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_EVICT_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_ACTION)
	{
		struct mod_t *low_mod;
		struct net_node_t *low_node;
		int msg_size;

		mem_debug("  %lld %lld 0x%x %s evict action\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_action\"\n",
			stack->id, mod->name);

		/* Get low node */
		low_mod = stack->target_mod;
		low_node = low_mod->high_net_node;
		assert(low_mod != mod);
		assert(low_mod == mod_get_low_mod(mod, stack->tag));
		assert(low_node && low_node->user_data == low_mod);

		/* Update the cache state since it may have changed after its 
		 * higher-level modules were invalidated */
		cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
		
		/* State = I */
		if (stack->state == cache_block_invalid)
		{
			esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* If state is M/O/N, data must be sent to lower level mod */
		if (stack->state == cache_block_modified || stack->state == cache_block_owned ||
			stack->state == cache_block_noncoherent)
		{
			/* Need to transmit data to low module */
			msg_size = 8 + mod->block_size;
			stack->reply = reply_ack_data;
		}
		/* If state is E/S, just an ack needs to be sent */
		else 
		{
			msg_size = 8;
			stack->reply = reply_ack;
		}

		/* Send message */
		stack->msg = net_try_send_ev(mod->low_net, mod->low_net_node,
			low_node, msg_size, EV_MOD_NMOESI_EVICT_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict receive\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Find and lock */
		if (stack->state == cache_block_noncoherent)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
				EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT, stack);
		}
		else 
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
				EV_MOD_NMOESI_EVICT_PROCESS, stack);
		}

		/* FIXME It's not guaranteed to be a write */
		new_stack->blocking = 0;
		new_stack->write = 1;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS)
	{
		mem_debug("  %lld %lld 0x%x %s evict process\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process\"\n",
			stack->id, target_mod->name);

		/* Error locking block */
		if (stack->err)
		{
			ret->err = 1;
			esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack)
		{
			/* Nothing to do */
		}
		else if (stack->reply == reply_ack_data)
		{
			if (stack->state == cache_block_exclusive)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_modified);
			}
			else if (stack->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
					stack->state);
			}
		}
		else 
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
				stack->state);
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->src_tag || 
				dir_entry_tag >= stack->src_tag + mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, stack->set, stack->way, z, 
				mod->low_net_node->index);
			if (dir_entry->owner == mod->low_net_node->index)
			{
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
					DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, target_mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT)
	{
		mem_debug("  %lld %lld 0x%x %s evict process noncoherent\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process_noncoherent\"\n",
			stack->id, target_mod->name);

		/* Error locking block */
		if (stack->err)
		{
			ret->err = 1;
			esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack_data)
		{
			if (stack->state == cache_block_exclusive)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_modified);
			}
			else if (stack->state == cache_block_owned || 
				stack->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else if (stack->state == cache_block_shared ||
				stack->state == cache_block_noncoherent)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_noncoherent);
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
					stack->state);
			}
		}
		else 
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
				stack->state);
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->src_tag || 
				dir_entry_tag >= stack->src_tag + mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, stack->set, stack->way, z, 
				mod->low_net_node->index);
			if (dir_entry->owner == mod->low_net_node->index)
			{
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
					DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, target_mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply\"\n",
			stack->id, target_mod->name);

		/* Send message */
		stack->msg = net_try_send_ev(target_mod->high_net, target_mod->high_net_node,
			mod->low_net_node, 8, EV_MOD_NMOESI_EVICT_REPLY_RECEIVE, stack,
			event, stack);
		return;

	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply receive\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply_receive\"\n",
			stack->id, mod->name);

		/* Receive message */
		net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Invalidate block if there was no error. */
		if (!stack->err)
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
				0, cache_block_invalid);

		assert(!dir_entry_group_shared_or_owned(mod->dir, stack->src_set, stack->src_way));
		esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s evict finish\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_finish\"\n",
			stack->id, mod->name);

		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_read_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;

	if (event == EV_MOD_NMOESI_READ_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request\"\n",
			stack->id, mod->name);

		/* Default return values*/
		ret->shared = 0;
		ret->err = 0;

		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
			stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == mod ||
			stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
			EV_MOD_NMOESI_READ_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s read request receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_READ_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->read = 1;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s read request action\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_action\"\n",
			stack->id, target_mod->name);

		/* Check block locking error. If read request is down-up, there should not
		 * have been any error while locking. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}
		esim_schedule_event(stack->request_dir == mod_request_up_down ?
			EV_MOD_NMOESI_READ_REQUEST_UPDOWN : EV_MOD_NMOESI_READ_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request updown\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown\"\n",
			stack->id, target_mod->name);

		stack->pending = 1;

		/* Set the initial reply message and size.  This will be adjusted later if
		 * a transfer occur between peers. */
		stack->reply_size = mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);
		
		if (stack->state)
		{
			/* Status = M/O/E/S/N
			 * Check: address is a multiple of requester's block_size
			 * Check: no sub-block requested by mod is already owned by mod */
			assert(stack->addr % mod->block_size == 0);
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size);
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
					continue;
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				assert(dir_entry->owner != mod->low_net_node->index);
			}

			/* TODO If there is only sharers, should one of them
			 *      send the data to mod instead of having target_mod do it? */

			/* Send read request to owners other than mod for all sub-blocks. */
			for (z = 0; z < dir->zsize; z++)
			{
				struct net_node_t *node;

				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;

				/* No owner */
				if (!DIR_ENTRY_VALID_OWNER(dir_entry))
					continue;

				/* Owner is mod */
				if (dir_entry->owner == mod->low_net_node->index)
					continue;

				/* Get owner mod */
				node = list_get(target_mod->high_net->node_list, dir_entry->owner);
				assert(node->kind == net_node_end);
				owner = node->user_data;
				assert(owner);

				/* Not the first sub-block */
				if (dir_entry_tag % owner->block_size)
					continue;

				/* Send read request */
				stack->pending++;
				new_stack = mod_stack_create(stack->id, target_mod, dir_entry_tag,
					EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack);
				/* Only set peer if its a subblock that was requested */
				if (dir_entry_tag >= stack->addr && 
					dir_entry_tag < stack->addr + mod->block_size)
				{
					new_stack->peer = mod_stack_set_peer(mod, stack->state);
				}
				new_stack->target_mod = owner;
				new_stack->request_dir = mod_request_down_up;
				esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
			}
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. 
			 * TODO: I'm not sure how relavant this is here for all states. */
			prefetcher_access_hit(stack, target_mod);
		}
		else
		{
			/* State = I */
			assert(!dir_entry_group_shared_or_owned(target_mod->dir,
				stack->set, stack->way));
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS, stack);
			/* Peer is NULL since we keep going up-down */
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

			/* The prefetcher may be interested in this miss */
			prefetcher_access_miss(stack, target_mod);

		}
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s read request updown miss\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_miss\"\n",
			stack->id, target_mod->name);

		/* Check error */
		if (stack->err)
		{
			dir_entry_unlock(target_mod->dir, stack->set, stack->way);
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on the return value 'shared'
		 * that comes from a read request into the next cache level.
		 * Also set the tag of the block. */
		cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
			stack->shared ? cache_block_shared : cache_block_exclusive);
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH)
	{
		int shared;

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Trace */
		mem_debug("  %lld %lld 0x%x %s read request updown finish\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_finish\"\n",
			stack->id, target_mod->name);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data or simply ack */
		if (stack->reply_size == 8) 
		{
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->reply_size > 8)
		{
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else 
		{
			fatal("Invalid reply size: %d", stack->reply_size);
		}

		dir = target_mod->dir;

		shared = 0;
		/* With the Owned state, the directory entry may remain owned by the sender */
		if (!stack->retain_owner)
		{
			/* Set owner to 0 for all directory entries not owned by mod. */
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				if (dir_entry->owner != mod->low_net_node->index)
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
			}
		}

		/* For each sub-block requested by mod, set mod as sharer, and
		 * check whether there is other cache sharing it. */
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			if (dir_entry->num_sharers > 1 || stack->nc_write || stack->shared)
				shared = 1;

			/* If the block is owned, non-coherent, or shared,  
			 * mod (the higher-level cache) should never be exclusive */
			if (stack->state == cache_block_owned || 
				stack->state == cache_block_noncoherent ||
				stack->state == cache_block_shared )
				shared = 1;
		}

		/* If no sub-block requested by mod is shared by other cache, set mod
		 * as owner of all of them. Otherwise, notify requester that the block is
		 * shared by setting the 'shared' return value to true. */
		ret->shared = shared;
		if (!shared)
		{
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
					continue;
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_set_owner(dir, stack->set, stack->way, z, mod->low_net_node->index);
			}
		}

		dir_entry_unlock(dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request downup\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup\"\n",
			stack->id, target_mod->name);

		/* Check: state must not be invalid or shared.
		 * By default, only one pending request.
		 * Response depends on state */
		assert(stack->state != cache_block_invalid);
		assert(stack->state != cache_block_shared);
		assert(stack->state != cache_block_noncoherent);
		stack->pending = 1;

		/* Send a read request to the owner of each subblock. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			struct net_node_t *node;

			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);

			/* No owner */
			if (!DIR_ENTRY_VALID_OWNER(dir_entry))
				continue;

			/* Get owner mod */
			node = list_get(target_mod->high_net->node_list, dir_entry->owner);
			assert(node && node->kind == net_node_end);
			owner = node->user_data;

			/* Not the first sub-block */
			if (dir_entry_tag % owner->block_size)
				continue;

			stack->pending++;
			new_stack = mod_stack_create(stack->id, target_mod, dir_entry_tag,
				EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack);
			new_stack->target_mod = owner;
			new_stack->request_dir = mod_request_down_up;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		}

		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS)
	{
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		mem_debug("  %lld %lld 0x%x %s read request downup wait for reqs\n", 
			esim_time, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_wait_for_reqs\"\n",
			stack->id, target_mod->name);

		if (stack->peer)
		{
			/* Send this block (or subblock) to the peer */
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack);
			new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
			new_stack->target_mod = stack->target_mod;
			esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
		}
		else 
		{
			/* No data to send to peer, so finish */
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack, 0);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request downup finish\n", 
			esim_time, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_finish\"\n",
			stack->id, target_mod->name);

		if (stack->reply == reply_ack_data)
		{
			/* If data was received, it was owned or modified by a higher level cache.
			 * We need to continue to propagate it up until a peer is found */

			if (stack->peer) 
			{
				/* Peer was found, so this directory entry should be changed 
				 * to owned */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_owned);

				/* Higher-level cache changed to shared, set owner of 
				 * sub-blocks to NONE. */
				dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size);
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);

				/* Let the lower-level cache know not to delete the owner */
				ret->retain_owner = 1;
			}
			else
			{
				/* Set state to shared */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_shared);

				/* State is changed to shared, set owner of sub-blocks to 0. */
				dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size);
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = target_mod->block_size + 8;
				mod_stack_set_reply(ret, reply_ack_data);
			}
		}
		else if (stack->reply == reply_ack)
		{
			/* Higher-level cache was exclusive with no modifications above it */
			stack->reply_size = 8;

			/* Set state to shared */
			cache_set_block(target_mod->cache, stack->set, stack->way, 
				stack->tag, cache_block_shared);

			/* State is changed to shared, set owner of sub-blocks to 0. */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size);
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
					DIR_ENTRY_OWNER_NONE);
			}

			if (stack->peer)
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);
			}
			else
			{
				mod_stack_set_reply(ret, reply_ack);
				stack->reply_size = 8;
			}
		}
		else if (stack->reply == reply_none)
		{
			/* This block is not present in any higher level caches */

			if (stack->peer) 
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->sub_block_size;
				assert(ret->reply_size >= 8);

				if (stack->state == cache_block_modified || 
					stack->state == cache_block_owned)
				{
					/* Let the lower-level cache know not to delete the owner */
					ret->retain_owner = 1;

					/* Set block to owned */
					cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_owned);
				}
				else 
				{
					/* Set block to shared */
					cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_shared);
				}
			}
			else 
			{
				if (stack->state == cache_block_exclusive || 
					stack->state == cache_block_shared)
				{
					stack->reply_size = 8;
					mod_stack_set_reply(ret, reply_ack);

				}
				else if (stack->state == cache_block_owned ||
					stack->state == cache_block_modified || 
					stack->state == cache_block_noncoherent)
				{
					/* No peer exists, so data is returned to mod */
					stack->reply_size = target_mod->sub_block_size + 8;
					mod_stack_set_reply(ret, reply_ack_data);
				}
				else 
				{
					fatal("Invalid cache block state: %d\n", stack->state);
				}

				/* Set block to shared */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_shared);
			}
		}
		else 
		{
			fatal("Unexpected reply type: %d\n", stack->reply);
		}


		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_reply\"\n",
			stack->id, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == mod);

		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = mod->low_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = mod->high_net_node;
		}

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_READ_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request finish\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_finish\"\n",
			stack->id, mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
		else
			net_receive(mod->high_net, mod->high_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_write_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_NMOESI_WRITE_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request\"\n",
			stack->id, mod->name);

		/* Default return values */
		ret->err = 0;

		/* For write requests, we need to set the initial reply size because
		 * in updown, peer transfers must be allowed to decrease this value
		 * (during invalidate). If the request turns out to be downup, then 
		 * these values will get overwritten. */
		stack->reply_size = mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);

		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
			stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == mod ||
			stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
			EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_WRITE_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->write = 1;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s write request action\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_action\"\n",
			stack->id, target_mod->name);

		/* Check lock error. If write request is down-up, there should
		 * have been no error. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Invalidate the rest of upper level sharers */
		new_stack = mod_stack_create(stack->id, target_mod, 0,
			EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE, stack);
		new_stack->except_mod = mod;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request exclusive\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_exclusive\"\n",
			stack->id, target_mod->name);

		if (stack->request_dir == mod_request_up_down)
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN, stack, 0);
		else
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown\"\n",
			stack->id, target_mod->name);

		/* state = M/E */
		if (stack->state == cache_block_modified ||
			stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);
		}
		/* state = O/S/I/N */
		else if (stack->state == cache_block_owned || stack->state == cache_block_shared ||
			stack->state == cache_block_invalid || stack->state == cache_block_noncoherent)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack);
			new_stack->peer = mod_stack_set_peer(mod, stack->state);
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);

			if (stack->state == cache_block_invalid)
			{
				/* The prefetcher may be interested in this miss */
				prefetcher_access_miss(stack, target_mod);
			}
		}
		else 
		{
			fatal("Invalid cache block state: %d\n", stack->state);
		}

		if (stack->state != cache_block_invalid)
		{
			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may been a miss. 
			 * TODO: I'm not sure how relavant this is here for all states. */
			prefetcher_access_hit(stack, target_mod);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown finish\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown_finish\"\n",
			stack->id, target_mod->name);

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Error in write request to next cache level */
		if (stack->err)
		{
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			dir_entry_unlock(target_mod->dir, stack->set, stack->way);
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Check that addr is a multiple of mod.block_size.
		 * Set mod as sharer and owner. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			assert(stack->addr % mod->block_size == 0);
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			dir_entry_set_owner(dir, stack->set, stack->way, z, mod->low_net_node->index);
			assert(dir_entry->num_sharers == 1);
		}

		/* Set state to exclusive */
		cache_set_block(target_mod->cache, stack->set, stack->way,
			stack->tag, cache_block_exclusive);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data up or simply ack */
		if (stack->reply_size == 8) 
		{
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->reply_size > 8)
		{
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else 
		{
			fatal("Invalid reply size: %d", stack->reply_size);
		}

		/* Unlock, reply_size is the data of the size of the requester's block. */
		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup\"\n",
			stack->id, target_mod->name);

		assert(stack->state != cache_block_invalid);
		assert(!dir_entry_group_shared_or_owned(target_mod->dir, stack->set, stack->way));

		/* Compute reply size */	
		if (stack->state == cache_block_exclusive || 
			stack->state == cache_block_shared) 
		{
			/* Exclusive and shared states send an ack */
			stack->reply_size = 8;
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->state == cache_block_noncoherent)
		{
			/* Non-coherent state sends data */
			stack->reply_size = target_mod->block_size + 8;
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else if (stack->state == cache_block_modified || 
			stack->state == cache_block_owned)
		{
			if (stack->peer) 
			{
				/* Modified or owned entries send data directly to peer 
				 * if it exists */
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);
				stack->reply_size = 8;

				/* This control path uses an intermediate stack that disappears, so 
				 * we have to update the return stack of the return stack */
				ret->ret_stack->reply_size -= target_mod->block_size;
				assert(ret->ret_stack->reply_size >= 8);

				/* Send data to the peer */
				new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack);
				new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
				new_stack->target_mod = stack->target_mod;

				esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
				return;
			}	
			else 
			{
				/* If peer does not exist, data is returned to mod */
				mod_stack_set_reply(ret, reply_ack_data);
				stack->reply_size = target_mod->block_size + 8;
			}
		}
		else 
		{
			fatal("Invalid cache block state: %d\n", stack->state);
		}

		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup complete\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup_finish\"\n",
			stack->id, target_mod->name);

		/* Set state to I, unlock*/
		cache_set_block(target_mod->cache, stack->set, stack->way, 0, cache_block_invalid);
		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = ret->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_reply\"\n",
			stack->id, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == mod);

		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = mod->low_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = mod->high_net_node;
		}

		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_WRITE_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request finish\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_finish\"\n",
			stack->id, mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
		{
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
		}
		else
		{
			net_receive(mod->high_net, mod->high_net_node, stack->msg);
		}
		

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_peer(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_t *src = stack->target_mod;
	struct mod_t *peer = stack->peer;

	if (event == EV_MOD_NMOESI_PEER_SEND) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer send\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer\"\n",
			stack->id, src->name);

		/* Send message from src to peer */
		stack->msg = net_try_send_ev(src->low_net, src->low_net_node, peer->low_net_node, 
			src->block_size + 8, EV_MOD_NMOESI_PEER_RECEIVE, stack, event, stack);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_RECEIVE) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer receive\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_receive\"\n",
			stack->id, peer->name);

		/* Receive message from src */
		net_receive(peer->low_net, peer->low_net_node, stack->msg);

		esim_schedule_event(EV_MOD_NMOESI_PEER_REPLY, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_REPLY) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer reply ack\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_reply_ack\"\n",
			stack->id, peer->name);

		/* Send ack from peer to src */
		stack->msg = net_try_send_ev(peer->low_net, peer->low_net_node, src->low_net_node, 
				8, EV_MOD_NMOESI_PEER_FINISH, stack, event, stack); 

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_FINISH) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer finish\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_finish\"\n",
			stack->id, src->name);

		/* Receive message from src */
		net_receive(src->low_net, src->low_net_node, stack->msg);

		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_invalidate(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag;
	uint32_t z;

	if (event == EV_MOD_NMOESI_INVALIDATE)
	{
		struct mod_t *sharer;
		int i;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		mem_debug("  %lld %lld 0x%x %s invalidate (set=%d, way=%d, state=%s)\n", esim_time, stack->id,
			stack->tag, mod->name, stack->set, stack->way,
			str_map_value(&cache_block_state_map, stack->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate\"\n",
			stack->id, mod->name);

		/* At least one pending reply */
		stack->pending = 1;
		
		/* Send write request to all upper level sharers except 'except_mod' */
		dir = mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + mod->block_size);
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			for (i = 0; i < dir->num_nodes; i++)
			{
				struct net_node_t *node;
				
				/* Skip non-sharers and 'except_mod' */
				if (!dir_entry_is_sharer(dir, stack->set, stack->way, z, i))
					continue;

				node = list_get(mod->high_net->node_list, i);
				sharer = node->user_data;
				if (sharer == stack->except_mod)
					continue;

				/* Clear sharer and owner */
				dir_entry_clear_sharer(dir, stack->set, stack->way, z, i);
				if (dir_entry->owner == i)
					dir_entry_set_owner(dir, stack->set, stack->way, z, DIR_ENTRY_OWNER_NONE);

				/* Send write request upwards if beginning of block */
				if (dir_entry_tag % sharer->block_size)
					continue;
				new_stack = mod_stack_create(stack->id, mod, dir_entry_tag,
					EV_MOD_NMOESI_INVALIDATE_FINISH, stack);
				new_stack->target_mod = sharer;
				new_stack->request_dir = mod_request_down_up;

				esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);
				stack->pending++;
			}
		}
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_INVALIDATE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s invalidate finish\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate_finish\"\n",
			stack->id, mod->name);

		if (stack->reply == reply_ack_data)
			cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
				cache_block_modified);

		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_message(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t z;

	if (event == EV_MOD_NMOESI_MESSAGE)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message\n", esim_time, stack->id,
			stack->addr, mod->name);

		stack->reply_size = 8;
		stack->reply = reply_ack;

		/* Default return values*/
		ret->err = 0;

		/* Checks */
		assert(stack->message);

		/* Get source and destination nodes */
		net = mod->low_net;
		src_node = mod->low_net_node;
		dst_node = target_mod->high_net_node;

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
			EV_MOD_NMOESI_MESSAGE_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s message receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);

		/* Receive message */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_MESSAGE_ACTION, stack);
		new_stack->message = stack->message;
		new_stack->blocking = 0;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s clear owner action\n", esim_time, stack->id,
			stack->tag, target_mod->name);

		assert(stack->message);

		/* Check block locking error. */
		mem_debug("stack err = %u\n", stack->err);
		if (stack->err)
		{
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
			return;
		}

		if (stack->message == message_clear_owner)
		{
			/* Remove owner */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				/* Skip other subblocks */
				if (stack->addr == stack->tag + z * target_mod->sub_block_size)
				{
					/* Clear the owner */
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					assert(dir_entry->owner == mod->low_net_node->index);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
				}
			}

		}
		else
		{
			fatal("Unexpected message");
		}

		/* Unlock the directory entry */
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);

		/* Checks */
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == mod);

		/* Get network and nodes */
		net = mod->low_net;
		src_node = target_mod->high_net_node;
		dst_node = mod->low_net_node;

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_MESSAGE_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s message finish\n", esim_time, stack->id,
			stack->tag, mod->name);

		/* Receive message */
		net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}
