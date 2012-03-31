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
 *  You should have received stack copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mem-system.h>


/* Events */

int EV_MOD_LOAD;
int EV_MOD_LOAD_LOCK;
int EV_MOD_LOAD_ACTION;
int EV_MOD_LOAD_MISS;
int EV_MOD_LOAD_UNLOCK;
int EV_MOD_LOAD_FINISH;

int EV_MOD_STORE;
int EV_MOD_STORE_LOCK;
int EV_MOD_STORE_ACTION;
int EV_MOD_STORE_UNLOCK;
int EV_MOD_STORE_FINISH;

int EV_MOD_FIND_AND_LOCK;
int EV_MOD_FIND_AND_LOCK_PORT;
int EV_MOD_FIND_AND_LOCK_ACTION;
int EV_MOD_FIND_AND_LOCK_FINISH;

int EV_MOD_EVICT;
int EV_MOD_EVICT_INVALID;
int EV_MOD_EVICT_ACTION;
int EV_MOD_EVICT_RECEIVE;
int EV_MOD_EVICT_WRITEBACK;
int EV_MOD_EVICT_WRITEBACK_EXCLUSIVE;
int EV_MOD_EVICT_WRITEBACK_FINISH;
int EV_MOD_EVICT_PROCESS;
int EV_MOD_EVICT_REPLY;
int EV_MOD_EVICT_REPLY_RECEIVE;
int EV_MOD_EVICT_FINISH;

int EV_MOD_WRITE_REQUEST;
int EV_MOD_WRITE_REQUEST_RECEIVE;
int EV_MOD_WRITE_REQUEST_ACTION;
int EV_MOD_WRITE_REQUEST_EXCLUSIVE;
int EV_MOD_WRITE_REQUEST_UPDOWN;
int EV_MOD_WRITE_REQUEST_UPDOWN_FINISH;
int EV_MOD_WRITE_REQUEST_DOWNUP;
int EV_MOD_WRITE_REQUEST_DOWNUP_FINISH;
int EV_MOD_WRITE_REQUEST_REPLY;
int EV_MOD_WRITE_REQUEST_FINISH;

int EV_MOD_READ_REQUEST;
int EV_MOD_READ_REQUEST_RECEIVE;
int EV_MOD_READ_REQUEST_ACTION;
int EV_MOD_READ_REQUEST_UPDOWN;
int EV_MOD_READ_REQUEST_UPDOWN_MISS;
int EV_MOD_READ_REQUEST_UPDOWN_FINISH;
int EV_MOD_READ_REQUEST_DOWNUP;
int EV_MOD_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
int EV_MOD_READ_REQUEST_DOWNUP_FINISH;
int EV_MOD_READ_REQUEST_REPLY;
int EV_MOD_READ_REQUEST_FINISH;

int EV_MOD_INVALIDATE;
int EV_MOD_INVALIDATE_FINISH;

int EV_MOD_PEER_SEND;
int EV_MOD_PEER_RECEIVE;
int EV_MOD_PEER_REPLY_ACK;
int EV_MOD_PEER_FINISH;





/* MOESI Protocol */

void mod_handler_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_cycle, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" addr=0x%x\n",
			stack->id, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_read);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_read, stack->addr, stack);
		if (master_stack)
		{
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_LOAD_FINISH);
			return;
		}

		/* Next event */
		esim_schedule_event(EV_MOD_LOAD_LOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_LOAD_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s load lock\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_LOAD_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_LOAD_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->read = 1;
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_LOAD_ACTION)
	{
		int retry_lat;
		mem_debug("  %lld %lld 0x%x %s load action\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_LOAD_LOCK, stack, retry_lat);
			return;
		}

		/* Hit */
		if (stack->state)
		{
			esim_schedule_event(EV_MOD_LOAD_UNLOCK, stack, 0);
			return;
		}

		/* Miss */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
			EV_MOD_LOAD_MISS, stack);
		new_stack->peer = mod;
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_READ_REQUEST, new_stack, 0);
		return;
	}

	if (event == EV_MOD_LOAD_MISS)
	{
		int retry_lat;
		mem_debug("  %lld %lld 0x%x %s load miss\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Error on read request. Unlock block and retry load. */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_lock_unlock(stack->dir_lock);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_LOAD_LOCK, stack, retry_lat);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
			stack->shared ? cache_block_shared : cache_block_exclusive);

		/* Continue */
		esim_schedule_event(EV_MOD_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Unlock directory entry */
		dir_lock_unlock(stack->dir_lock);

		/* Continue */
		esim_schedule_event(EV_MOD_LOAD_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_LOAD_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s load finish\n", esim_cycle, stack->id,
			stack->addr, mod->name);
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


void mod_handler_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s store\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Record access */
		mod_access_start(mod, stack, mod_access_write);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_write, stack->addr, stack);
		if (master_stack)
		{
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_STORE_FINISH);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_STORE_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* If there is any older access, wait for it */
		older_stack = stack->access_list_prev;
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_STORE_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->read = 0;
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_STORE_ACTION)
	{
		int retry_lat;
		mem_debug("  %lld %lld 0x%x %s store action\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Hit - state=M/E */
		if (stack->state == cache_block_modified ||
			stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_STORE_UNLOCK, stack, 0);
			return;
		}

		/* Miss - state=O/S/I */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
			EV_MOD_STORE_UNLOCK, stack);
		new_stack->peer = mod;
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_WRITE_REQUEST, new_stack, 0);
		return;
	}

	if (event == EV_MOD_STORE_UNLOCK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store unlock\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Error in write request, unlock block and retry store. */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_lock_unlock(stack->dir_lock);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Update tag/state and unlock */
		cache_set_block(mod->cache, stack->set, stack->way,
			stack->tag, cache_block_modified);
		dir_lock_unlock(stack->dir_lock);

		/* Continue */
		esim_schedule_event(EV_MOD_STORE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_cycle, stack->id,
			stack->addr, mod->name);

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


void mod_handler_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
			esim_cycle, stack->id, stack->addr, mod->name, stack->blocking);

		/* Default return values */
		ret->err = 0;
		ret->set = 0;
		ret->way = 0;
		ret->state = 0;
		ret->tag = 0;

		/* Get a port */
		mod_lock_port(mod, stack, EV_MOD_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_FIND_AND_LOCK_PORT)
	{
		struct mod_port_t *port = stack->port;

		assert(stack->port);
		mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_cycle, stack->id,
			stack->addr, mod->name);

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
				map_value(&cache_block_state_map, stack->state));

		/* Statistics */
		mod->accesses++;
		if (stack->hit)
			mod->hits++;
		if (stack->read)
		{
			mod->reads++;
			stack->blocking ? mod->blocking_reads++ : mod->non_blocking_reads++;
			if (stack->hit)
				mod->read_hits++;
		}
		else
		{
			mod->writes++;
			stack->blocking ? mod->blocking_writes++ : mod->non_blocking_writes++;
			if (stack->hit)
				mod->write_hits++;
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
			else
			{
				mod->no_retry_writes++;
				if (stack->hit)
					mod->no_retry_write_hits++;
			}
		}

		/* Miss */
		if (!stack->hit)
		{
			/* Find victim */
			stack->way = cache_replace_block(mod->cache, stack->set);
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
				stack->set, stack->way));
			mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way,
				map_value(&cache_block_state_map, stack->state));
		}

		/* Get directory entry */
		stack->dir_lock = dir_lock_get(mod->dir, stack->set, stack->way);

		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
		if (stack->dir_lock->lock && !stack->blocking)
		{
			mem_debug("    %lld 0x%x %s block already locked: set=%d, way=%d\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way);
			ret->err = 1;
			mod_unlock_port(mod, port, stack);
			mod_stack_return(stack);
			return;
		}

		/* Lock directory entry */
		if (!dir_lock_lock(stack->dir_lock, EV_MOD_FIND_AND_LOCK_PORT, stack))
			return;

		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought.
		 * Also, update LRU counters here. */
		cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
		cache_access_block(mod->cache, stack->set, stack->way);

		/* Access latency */
		esim_schedule_event(EV_MOD_FIND_AND_LOCK_ACTION, stack, mod->latency);
		return;
	}

	if (event == EV_MOD_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_cycle, stack->id,
			stack->tag, mod->name);

		/* Release port */
		mod_unlock_port(mod, port, stack);

		/* On miss, evict if victim is a valid block. */
		if (!stack->hit && stack->state)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod, 0,
				EV_MOD_FIND_AND_LOCK_FINISH, stack);
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			esim_schedule_event(EV_MOD_EVICT, new_stack, 0);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_cycle, stack->id,
			stack->tag, mod->name, stack->err);

		/* If evict produced err, return err */
		if (stack->err)
		{
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state);
			assert(stack->eviction);
			ret->err = 1;
			dir_lock_unlock(stack->dir_lock);
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
		ret->dir_lock = stack->dir_lock;
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_evict(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_EVICT)
	{
		/* Default return value */
		ret->err = 0;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
			stack->set, stack->way));
		mem_debug("  %lld %lld 0x%x %s evict (set=%d, way=%d, state=%s)\n", esim_cycle, stack->id,
			stack->tag, mod->name, stack->set, stack->way,
			map_value(&cache_block_state_map, stack->state));

		/* Save some data */
		stack->src_set = stack->set;
		stack->src_way = stack->way;
		stack->src_tag = stack->tag;
		stack->target_mod = mod_get_low_mod(mod, stack->tag);
		target_mod = stack->target_mod;

		/* Send write request to all sharers */
		new_stack = mod_stack_create(stack->id, mod, 0,
			EV_MOD_EVICT_INVALID, stack);
		new_stack->except_mod = NULL;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		esim_schedule_event(EV_MOD_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_INVALID)
	{
		mem_debug("  %lld %lld 0x%x %s evict invalid\n", esim_cycle, stack->id,
			stack->tag, mod->name);

		/* If module is main memory, no writeback.
		 * We just need to set the block as invalid, and finish. */
		if (mod->kind == mod_kind_main_memory)
		{
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
				0, cache_block_invalid);
			esim_schedule_event(EV_MOD_EVICT_FINISH, stack, 0);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_EVICT_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_ACTION)
	{
		struct mod_t *low_mod;
		struct net_node_t *low_node;

		mem_debug("  %lld %lld 0x%x %s evict action\n", esim_cycle, stack->id,
			stack->tag, mod->name);

		/* Get low node */
		low_mod = stack->target_mod;
		low_node = low_mod->high_net_node;
		assert(low_mod != mod);
		assert(low_mod == mod_get_low_mod(mod, stack->tag));
		assert(low_node && low_node->user_data == low_mod);
		
		/* State = I */
		if (stack->state == cache_block_invalid)
		{
			esim_schedule_event(EV_MOD_EVICT_FINISH, stack, 0);
			return;
		}

		/* State = M/O */
		if (stack->state == cache_block_modified ||
			stack->state == cache_block_owned)
		{
			/* Send message */
			stack->msg = net_try_send_ev(mod->low_net, mod->low_net_node,
				low_node, mod->block_size + 8, EV_MOD_EVICT_RECEIVE, stack,
				event, stack);
			stack->writeback = 1;
			return;
		}

		/* State = S/E */
		if (stack->state == cache_block_shared ||
			stack->state == cache_block_exclusive)
		{
			/* Send message */
			stack->msg = net_try_send_ev(mod->low_net, mod->low_net_node,
				low_node, 8, EV_MOD_EVICT_RECEIVE, stack, event, stack);
			return;
		}

		/* Shouldn't get here */
		panic("%s: invalid moesi state", __FUNCTION__);
		return;
	}

	if (event == EV_MOD_EVICT_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict receive\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Receive message */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
			EV_MOD_EVICT_WRITEBACK, stack);
		new_stack->blocking = 0;
		new_stack->read = 0;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_WRITEBACK)
	{
		mem_debug("  %lld %lld 0x%x %s evict writeback\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Error locking block */
		if (stack->err)
		{
			ret->err = 1;
			esim_schedule_event(EV_MOD_EVICT_REPLY, stack, 0);
			return;
		}

		/* No writeback */
		if (!stack->writeback)
		{
			esim_schedule_event(EV_MOD_EVICT_PROCESS, stack, 0);
			return;
		}

		/* Writeback */
		new_stack = mod_stack_create(stack->id, target_mod, 0,
			EV_MOD_EVICT_WRITEBACK_EXCLUSIVE, stack);
		new_stack->except_mod = mod;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		esim_schedule_event(EV_MOD_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_WRITEBACK_EXCLUSIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict writeback exclusive\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* State = O/S/I */
		assert(stack->state != cache_block_invalid);
		if (stack->state == cache_block_owned || stack->state ==
			cache_block_shared)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_EVICT_WRITEBACK_FINISH, stack);
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_WRITE_REQUEST, new_stack, 0);
			return;
		}

		/* State = M/E */
		esim_schedule_event(EV_MOD_EVICT_WRITEBACK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_WRITEBACK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s evict writeback finish\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Error in write request */
		if (stack->err)
		{
			ret->err = 1;
			dir_lock_unlock(stack->dir_lock);
			esim_schedule_event(EV_MOD_EVICT_REPLY, stack, 0);
			return;
		}

		/* Set tag and state */
		if (target_mod->cache)
			cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
				cache_block_modified);
		esim_schedule_event(EV_MOD_EVICT_PROCESS, stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_PROCESS)
	{
		mem_debug("  %lld %lld 0x%x %s evict process\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Remove sharer, owner, and unlock */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->src_tag || dir_entry_tag >= stack->src_tag + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			if (dir_entry->owner == mod->low_net_node->index)
				dir_entry_set_owner(dir, stack->set, stack->way, z, DIR_ENTRY_OWNER_NONE);
		}
		dir_lock_unlock(stack->dir_lock);

		esim_schedule_event(EV_MOD_EVICT_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_REPLY)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Send message */
		stack->msg = net_try_send_ev(target_mod->high_net, target_mod->high_net_node,
			mod->low_net_node, 8, EV_MOD_EVICT_REPLY_RECEIVE, stack,
			event, stack);
		return;

	}

	if (event == EV_MOD_EVICT_REPLY_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply receive\n", esim_cycle, stack->id,
			stack->tag, mod->name);

		/* Receive message */
		net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Invalidate block if there was no error. */
		if (!stack->err)
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
				0, cache_block_invalid);
		assert(!dir_entry_group_shared_or_owned(mod->dir,
			stack->src_set, stack->src_way));
		esim_schedule_event(EV_MOD_EVICT_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_EVICT_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s evict finish\n", esim_cycle, stack->id,
			stack->tag, mod->name);
		
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_read_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;

	if (event == EV_MOD_READ_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request\n", esim_cycle, stack->id,
			stack->addr, mod->name);

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
			EV_MOD_READ_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s read request receive\n", esim_cycle, stack->id,
			stack->addr, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_READ_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->read = 1;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s read request action\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Check block locking error. If read request is down-up, there should not
		 * have been any error while locking. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			ret->reply = reply_ACK_ERROR;
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_READ_REQUEST_REPLY, stack, 0);
			return;
		}
		esim_schedule_event(stack->request_dir == mod_request_up_down ?
			EV_MOD_READ_REQUEST_UPDOWN : EV_MOD_READ_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_UPDOWN)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request updown\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);
		stack->pending = 1;

		/* Set the initial reply message and size.  This will be adjusted later if
		 * a transfer occur between peers. */
		stack->reply_size = mod->block_size + 8;
		stack->reply = reply_ACK_DATA;
		
		if (stack->state)
		{
			/* Status = M/O/E/S
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
					EV_MOD_READ_REQUEST_UPDOWN_FINISH, stack);
				new_stack->peer = stack->mod;
				new_stack->target_mod = owner;
				new_stack->request_dir = mod_request_down_up;
				esim_schedule_event(EV_MOD_READ_REQUEST, new_stack, 0);
			}
			esim_schedule_event(EV_MOD_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		}
		else
		{
			/* State = I */
			assert(!dir_entry_group_shared_or_owned(target_mod->dir,
				stack->set, stack->way));
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_READ_REQUEST_UPDOWN_MISS, stack);
			/* Peer is NULL since we keep going up-down */
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_READ_REQUEST, new_stack, 0);
		}
		return;
	}

	if (event == EV_MOD_READ_REQUEST_UPDOWN_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s read request updown miss\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);
		
		/* Check error */
		if (stack->err)
		{
			dir_lock_unlock(stack->dir_lock);
			ret->err = 1;
			ret->reply = reply_ACK_ERROR;
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_READ_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on the return value 'shared'
		 * that comes from a read request into the next cache level.
		 * Also set the tag of the block. */
		cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
			stack->shared ? cache_block_shared : cache_block_exclusive);
		esim_schedule_event(EV_MOD_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_UPDOWN_FINISH)
	{
		int shared;

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		mem_debug("  %lld %lld 0x%x %s read request updown finish\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data or simply ACK */
		if (stack->reply_size == 8) 
		{
			ret->reply = reply_ACK;
		}
		else if (stack->reply_size > 8)
		{
			ret->reply = reply_ACK_DATA;
		}
		else 
		{
			panic("Invalid reply size\n");
		}

		/* Set owner to 0 for all directory entries not owned by mod. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			if (dir_entry->owner != mod->low_net_node->index)
				dir_entry_set_owner(dir, stack->set, stack->way, z, DIR_ENTRY_OWNER_NONE);
		}

		/* For each sub-block requested by mod, set mod as sharer, and
		 * check whether there is other cache sharing it. */
		shared = 0;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			if (dir_entry->num_sharers > 1)
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

		dir_lock_unlock(stack->dir_lock);
		esim_schedule_event(EV_MOD_READ_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_DOWNUP)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request downup\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Check: state must not be invalid or shared.
		 * By default, only one pending request.
		 * Response depends on state */
		assert(stack->state != cache_block_invalid);
		assert(stack->state != cache_block_shared);
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
				EV_MOD_READ_REQUEST_DOWNUP_FINISH, stack);
			new_stack->target_mod = owner;
			new_stack->request_dir = mod_request_down_up;
			esim_schedule_event(EV_MOD_READ_REQUEST, new_stack, 0);
		}

		/* Set up the reply that will take place after all read
		 * requests for blocks have returned */
		if (stack->state == cache_block_exclusive) 
		{
			/* Exclusive state only sends an ACK */
			stack->reply_size = 8;
			stack->reply = reply_ACK;
		}
		else if (stack->peer == NULL) 
		{
			/* State is M/O and no peer exists, so data is returned to mod */
			stack->reply_size = target_mod->block_size + 8;
			stack->reply = reply_ACK_DATA;
		}
		else 
		{
			/* State is M/O and peer exists, so data is sent directly to peer */
			stack->reply_size = 8;
			stack->reply = reply_ACK_DATA_SENT_TO_PEER;

			/* Decrease the amount of data that mod will have to send back
			 * to its higher level cache */
			ret->reply_size -= target_mod->block_size;
			assert(ret->reply_size >= 8);
		}

		esim_schedule_event(EV_MOD_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_DOWNUP_WAIT_FOR_REQS)
	{
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		mem_debug("  %lld %lld 0x%x %s read request downup wait for reqs\n", 
			esim_cycle, stack->id, stack->tag, target_mod->name);

		if (stack->reply == reply_ACK_DATA_SENT_TO_PEER)
		{
			/* Send this block (or subblock) to the peer */
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_READ_REQUEST_DOWNUP_FINISH, stack);
			new_stack->peer = stack->peer;
			new_stack->target_mod = stack->target_mod;
			esim_schedule_event(EV_MOD_PEER_SEND, new_stack, 0);
		}
		else 
		{
			/* No data to send to peer, so finish */
			esim_schedule_event(EV_MOD_READ_REQUEST_DOWNUP_FINISH, stack, 0);
		}

		return;
	}

	if (event == EV_MOD_READ_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request downup finish\n", 
			esim_cycle, stack->id, stack->tag, target_mod->name);

		/* Modified states become owned */
		if (stack->state == cache_block_modified) 
		{
			cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
				cache_block_owned);
		}
		/* Exclusive states become shared */
		else if (stack->state == cache_block_exclusive) 
		{
			/* If state was changed to shared, set owner of sub-blocks to 0. */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size);
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_set_owner(dir, stack->set, stack->way, z, DIR_ENTRY_OWNER_NONE);
			}
			cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
				cache_block_shared);
		}	
		/* Owned states remain owned */
		else if (stack->state == cache_block_owned) 
		{
			/* Nothing to do */
		}
		/* No other states are allowed */
		else 
		{
			panic("Invalid cache block state: %d", stack->state);
		}

		dir_lock_unlock(stack->dir_lock);
		esim_schedule_event(EV_MOD_READ_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request reply\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->tag) == target_mod ||
			mod_get_low_mod(target_mod, stack->tag) == mod);

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
			EV_MOD_READ_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_READ_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request finish\n", esim_cycle, stack->id,
			stack->tag, mod->name);

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


void mod_handler_write_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_WRITE_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request\n", esim_cycle, stack->id,
			stack->addr, mod->name);

		/* Default return values */
		ret->err = 0;

		/* For write requests, we need to set the initial reply size because
		 * in updown, peer transfers must be allowed to decrease this value
		 * (during invalidate). If the request turns out to be downup, then 
		 * these values will get overwritten. */
		stack->reply_size = mod->block_size + 8;
		stack->reply = reply_ACK_DATA;

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
			EV_MOD_WRITE_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request receive\n", esim_cycle, stack->id,
			stack->addr, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_WRITE_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->read = 0;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s write request action\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Check lock error. If write request is down-up, there should
		 * have been no error. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Invalidate the rest of upper level sharers */
		new_stack = mod_stack_create(stack->id, target_mod, 0,
			EV_MOD_WRITE_REQUEST_EXCLUSIVE, stack);
		new_stack->except_mod = mod;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		new_stack->peer = stack->peer;
		esim_schedule_event(EV_MOD_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_EXCLUSIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request exclusive\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		if (stack->request_dir == mod_request_up_down)
			esim_schedule_event(EV_MOD_WRITE_REQUEST_UPDOWN, stack, 0);
		else
			esim_schedule_event(EV_MOD_WRITE_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_UPDOWN)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* state = M/E */
		if (stack->state == cache_block_modified ||
			stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);
		}
		/* state = O/S/I */
		else if (stack->state == cache_block_owned || stack->state == cache_block_shared ||
			stack->state == cache_block_invalid)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_WRITE_REQUEST_UPDOWN_FINISH, stack);
			new_stack->peer = mod;
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_WRITE_REQUEST, new_stack, 0);
		}
		else 
		{
			panic("Unknown cache block state\n");
		}

		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_UPDOWN_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown finish\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Error in write request to next cache level */
		if (stack->err)
		{
			ret->err = 1;
			ret->reply = reply_ACK_ERROR;
			stack->reply_size = 8;
			dir_lock_unlock(stack->dir_lock);
			esim_schedule_event(EV_MOD_WRITE_REQUEST_REPLY, stack, 0);
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

		/* Set state: M->M, O/E/S/I->E */
		if (target_mod->cache && stack->state != cache_block_modified)
			cache_set_block(target_mod->cache, stack->set, stack->way,
				stack->tag, cache_block_exclusive);

		/* Unlock, reply_size is the data of the size of the requester's block. */
		dir_lock_unlock(stack->dir_lock);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data up or simply ACK */
		if (stack->reply_size == 8) 
		{
			ret->reply = reply_ACK;
		}
		else if (stack->reply_size > 8)
		{
			ret->reply = reply_ACK_DATA;
		}
		else 
		{
			panic("Invalid reply size\n");
		}

		esim_schedule_event(EV_MOD_WRITE_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_DOWNUP)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		assert(stack->state != cache_block_invalid);
		assert(!dir_entry_group_shared_or_owned(target_mod->dir, stack->set, stack->way));

		/* Compute reply size */	
		if (stack->state == cache_block_exclusive || 
			stack->state == cache_block_shared) 
		{
			/* Exclusive and shared states send an ACK */
			stack->reply_size = 8;
			stack->reply = reply_ACK;
		}
		else if (stack->state == cache_block_modified || 
			stack->state == cache_block_owned)
		{
			if (stack->peer) 
			{
				/* Modified or owned entries send data directly to peer if it exists */
				stack->reply = reply_ACK_DATA_SENT_TO_PEER;
				stack->reply_size = 8;

				/* This control path uses an intermediate stack that disappears, so 
				 * we have to update the return stack of the return stack */
				ret->ret_stack->reply_size -= target_mod->block_size;
				assert(ret->ret_stack->reply_size >= 8);

				/* Send data to the peer */
				new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_WRITE_REQUEST_DOWNUP_FINISH, stack);
				new_stack->peer = stack->peer;
				new_stack->target_mod = stack->target_mod;

				esim_schedule_event(EV_MOD_PEER_SEND, new_stack, 0);
				return;
			}	
			else 
			{
				/* If peer does not exist, data is returned to mod */
				stack->reply = reply_ACK_DATA;
				stack->reply_size = target_mod->block_size + 8;
			}
		}
		else 
		{
			panic("Invalid cache block state: %d\n", stack->state);
		}

		esim_schedule_event(EV_MOD_WRITE_REQUEST_DOWNUP_FINISH, stack, 0);

		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup complete\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Set state to I, unlock*/
		cache_set_block(target_mod->cache, stack->set, stack->way, 0, cache_block_invalid);
		dir_lock_unlock(stack->dir_lock);
		
		esim_schedule_event(EV_MOD_WRITE_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request reply\n", esim_cycle, stack->id,
			stack->tag, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(mod_get_low_mod(mod, stack->tag) == target_mod ||
			mod_get_low_mod(target_mod, stack->tag) == mod);

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
			EV_MOD_WRITE_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_WRITE_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request finish\n", esim_cycle, stack->id,
			stack->tag, mod->name);

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


void mod_handler_peer(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_t *src = stack->target_mod;
	struct mod_t *peer = stack->peer;

	if (event == EV_MOD_PEER_SEND) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer send\n", esim_cycle, stack->id,
			stack->tag, src->name, peer->name);

		/* Send message from src to peer */
		stack->msg = net_try_send_ev(src->low_net, src->low_net_node, peer->low_net_node, 
				src->block_size + 8, EV_MOD_PEER_RECEIVE, stack, event, stack); 

		return;
	}

	if (event == EV_MOD_PEER_RECEIVE) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer receive\n", esim_cycle, stack->id,
			stack->tag, src->name, peer->name);

		/* Receive message from src */
		net_receive(peer->low_net, peer->low_net_node, stack->msg);

		esim_schedule_event(EV_MOD_PEER_REPLY_ACK, stack, 0);

		return;
	}

	if (event == EV_MOD_PEER_REPLY_ACK) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer reply ack\n", esim_cycle, stack->id,
			stack->tag, src->name, peer->name);

		/* Send ack from peer to src */
		stack->msg = net_try_send_ev(peer->low_net, peer->low_net_node, src->low_net_node, 
				8, EV_MOD_PEER_FINISH, stack, event, stack); 

		return;
	}

	if (event == EV_MOD_PEER_FINISH) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer finish\n", esim_cycle, stack->id,
			stack->tag, src->name, peer->name);

		/* Receive message from src */
		net_receive(src->low_net, src->low_net_node, stack->msg);

		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_invalidate(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag;
	uint32_t z;

	if (event == EV_MOD_INVALIDATE)
	{
		struct mod_t *sharer;
		int i;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		mem_debug("  %lld %lld 0x%x %s invalidate (set=%d, way=%d, state=%s)\n", esim_cycle, stack->id,
			stack->tag, mod->name, stack->set, stack->way,
			map_value(&cache_block_state_map, stack->state));
		stack->pending = 1;
		
		/* Send write request to all upper level sharers except 'except_mod' */
		dir = mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			int first_sharer = 1;
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
					EV_MOD_INVALIDATE_FINISH, stack);
				new_stack->target_mod = sharer;
				new_stack->request_dir = mod_request_down_up;

				/* If there is a peer and this is a block that he is interested in,
				 * and this sharer is the first sharer that is capable of 
				 * sending data, send the data directly to the peer. */
				if (stack->peer && first_sharer)
                                {
					new_stack->peer = stack->peer;
					first_sharer = 0;
				}
				esim_schedule_event(EV_MOD_WRITE_REQUEST, new_stack, 0);
				stack->pending++;
			}
		}
		esim_schedule_event(EV_MOD_INVALIDATE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_INVALIDATE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s invalidate finish\n", esim_cycle, stack->id,
			stack->tag, mod->name);

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

