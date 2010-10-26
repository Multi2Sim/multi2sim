/*
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include "cachesystem.h"


/* Debug */

int cache_debug_category;




/* MOESI stack */

static struct repos_t *moesi_stack_repos;
uint64_t moesi_stack_id = 0;

#define CYCLE ((long long) esim_cycle)
#define ID ((long long) stack->id)
#define RETRY_LATENCY (random() % ccache->lat + ccache->lat)

struct moesi_stack_t *moesi_stack_create(uint64_t id, struct ccache_t *ccache,
	uint32_t addr, int retevent, void *retstack)
{
	struct moesi_stack_t *stack;
	stack = repos_create_object(moesi_stack_repos);
	stack->ccache = ccache;
	stack->addr = addr;
	stack->retevent = retevent;
	stack->retstack = retstack;
	stack->id = id;
	return stack;
}


void moesi_stack_return(struct moesi_stack_t *stack)
{
	int retevent = stack->retevent;
	void *retstack = stack->retstack;

	repos_free_object(moesi_stack_repos, stack);
	esim_schedule_event(retevent, retstack, 0);
}




/* Events */

int EV_MOESI_FIND_AND_LOCK;
int EV_MOESI_FIND_AND_LOCK_FINISH;

int EV_MOESI_LOAD;
int EV_MOESI_LOAD_ACTION;
int EV_MOESI_LOAD_MISS;
int EV_MOESI_LOAD_FINISH;

int EV_MOESI_STORE;
int EV_MOESI_STORE_ACTION;
int EV_MOESI_STORE_FINISH;

int EV_MOESI_EVICT;
int EV_MOESI_EVICT_ACTION;
int EV_MOESI_EVICT_RECEIVE;
int EV_MOESI_EVICT_WRITEBACK;
int EV_MOESI_EVICT_WRITEBACK_EXCLUSIVE;
int EV_MOESI_EVICT_WRITEBACK_FINISH;
int EV_MOESI_EVICT_PROCESS;
int EV_MOESI_EVICT_REPLY;
int EV_MOESI_EVICT_REPLY_RECEIVE;
int EV_MOESI_EVICT_FINISH;

int EV_MOESI_WRITE_REQUEST;
int EV_MOESI_WRITE_REQUEST_RECEIVE;
int EV_MOESI_WRITE_REQUEST_ACTION;
int EV_MOESI_WRITE_REQUEST_EXCLUSIVE;
int EV_MOESI_WRITE_REQUEST_UPDOWN;
int EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH;
int EV_MOESI_WRITE_REQUEST_DOWNUP;
int EV_MOESI_WRITE_REQUEST_REPLY;
int EV_MOESI_WRITE_REQUEST_FINISH;

int EV_MOESI_READ_REQUEST;
int EV_MOESI_READ_REQUEST_RECEIVE;
int EV_MOESI_READ_REQUEST_ACTION;
int EV_MOESI_READ_REQUEST_UPDOWN;
int EV_MOESI_READ_REQUEST_UPDOWN_MISS;
int EV_MOESI_READ_REQUEST_UPDOWN_FINISH;
int EV_MOESI_READ_REQUEST_DOWNUP;
int EV_MOESI_READ_REQUEST_DOWNUP_FINISH;
int EV_MOESI_READ_REQUEST_REPLY;
int EV_MOESI_READ_REQUEST_FINISH;

int EV_MOESI_INVALIDATE;
int EV_MOESI_INVALIDATE_FINISH;





/* MOESI Protocol */

void moesi_init()
{
	/* Debug */
	cache_debug_category = debug_new_category();

	/* Events */
	EV_MOESI_FIND_AND_LOCK = esim_register_event(moesi_handler_find_and_lock);
	EV_MOESI_FIND_AND_LOCK_FINISH = esim_register_event(moesi_handler_find_and_lock);

	EV_MOESI_LOAD = esim_register_event(moesi_handler_load);
	EV_MOESI_LOAD_ACTION = esim_register_event(moesi_handler_load);
	EV_MOESI_LOAD_MISS = esim_register_event(moesi_handler_load);
	EV_MOESI_LOAD_FINISH = esim_register_event(moesi_handler_load);

	EV_MOESI_STORE = esim_register_event(moesi_handler_store);
	EV_MOESI_STORE_ACTION = esim_register_event(moesi_handler_store);
	EV_MOESI_STORE_FINISH = esim_register_event(moesi_handler_store);

	EV_MOESI_EVICT = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_ACTION = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_RECEIVE = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_WRITEBACK = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_WRITEBACK_EXCLUSIVE = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_WRITEBACK_FINISH = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_PROCESS = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_REPLY = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_REPLY_RECEIVE = esim_register_event(moesi_handler_evict);
	EV_MOESI_EVICT_FINISH = esim_register_event(moesi_handler_evict);

	EV_MOESI_WRITE_REQUEST = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_RECEIVE = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_ACTION = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_EXCLUSIVE = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_UPDOWN = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_DOWNUP = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_REPLY = esim_register_event(moesi_handler_write_request);
	EV_MOESI_WRITE_REQUEST_FINISH = esim_register_event(moesi_handler_write_request);

	EV_MOESI_READ_REQUEST = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_RECEIVE = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_ACTION = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_UPDOWN = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_UPDOWN_MISS = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_UPDOWN_FINISH = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_DOWNUP = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_DOWNUP_FINISH = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_REPLY = esim_register_event(moesi_handler_read_request);
	EV_MOESI_READ_REQUEST_FINISH = esim_register_event(moesi_handler_read_request);

	EV_MOESI_INVALIDATE = esim_register_event(moesi_handler_invalidate);
	EV_MOESI_INVALIDATE_FINISH = esim_register_event(moesi_handler_invalidate);

	/* Stack repository */
	moesi_stack_repos = repos_create(sizeof(struct moesi_stack_t),
		"moesi_stack_repos");
}


void moesi_done()
{
	repos_free(moesi_stack_repos);
}


void moesi_handler_find_and_lock(int event, void *data)
{
	struct moesi_stack_t *stack = data, *ret = stack->retstack, *newstack;
	struct ccache_t *ccache = stack->ccache;

	if (event == EV_MOESI_FIND_AND_LOCK)
	{
		int hit;
		cache_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n", CYCLE, ID,
			stack->addr, ccache->name, stack->blocking);

		/* Default return values */
		ret->err = 0;
		ret->set = 0;
		ret->way = 0;
		ret->status = 0;
		ret->tag = 0;

		/* Look for block. */
		hit = ccache_find_block(ccache, stack->addr, &stack->set,
			&stack->way, &stack->tag, &stack->status);
		if (hit)
			cache_debug("    %lld 0x%x %s hit: set=%d, way=%d, status=%d\n", ID,
				stack->tag, ccache->name, stack->set, stack->way, stack->status);

		/* Stats */
		ccache->accesses++;
		if (hit)
			ccache->hits++;
		if (stack->read) {
			ccache->reads++;
			stack->blocking ? ccache->blocking_reads++ : ccache->non_blocking_reads++;
			if (hit)
				ccache->read_hits++;
		} else {
			ccache->writes++;
			stack->blocking ? ccache->blocking_writes++ : ccache->non_blocking_writes++;
			if (hit)
				ccache->write_hits++;
		}
		if (!stack->retry) {
			ccache->no_retry_accesses++;
			if (hit)
				ccache->no_retry_hits++;
			if (stack->read) {
				ccache->no_retry_reads++;
				if (hit)
					ccache->no_retry_read_hits++;
			} else {
				ccache->no_retry_writes++;
				if (hit)
					ccache->no_retry_write_hits++;
			}
		}

		/* Miss */
		if (!hit) {
			
			assert(!stack->blocking);
			assert(ccache != main_memory);

			/* Find victim */
			stack->way = cache_replace_block(ccache->cache, stack->set);
			cache_get_block(ccache->cache, stack->set, stack->way, NULL, &stack->status);
			assert(stack->status || !dir_entry_group_shared_or_owned(ccache->dir,
				stack->set, stack->way));
			cache_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, status=%d\n",
				ID, stack->tag, ccache->name, stack->set, stack->way, stack->status);
		}

		/* Lock entry */
		stack->dir_lock = ccache_get_dir_lock(ccache, stack->set, stack->way);
		if (stack->dir_lock->lock && !stack->blocking) {
			cache_debug("    %lld 0x%x %s block already locked: set=%d, way=%d\n",
				ID, stack->tag, ccache->name, stack->set, stack->way);
			ret->err = 1;
			moesi_stack_return(stack);
			return;
		}
		if (!dir_lock_lock(stack->dir_lock, EV_MOESI_FIND_AND_LOCK, stack))
			return;

		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought. */
		if (ccache->cache)
			cache_set_transient_tag(ccache->cache, stack->set, stack->way, stack->tag);

		/* On miss, evict if victim is a valid block. */
		if (!hit && stack->status) {
			stack->eviction = 1;
			newstack = moesi_stack_create(stack->id, ccache, 0,
				EV_MOESI_FIND_AND_LOCK_FINISH, stack);
			newstack->set = stack->set;
			newstack->way = stack->way;
			esim_schedule_event(EV_MOESI_EVICT, newstack, ccache->lat);
			return;
		}

		/* Access latency */
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK_FINISH, stack, ccache->lat);
		return;
	}

	if (event == EV_MOESI_FIND_AND_LOCK_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", CYCLE, ID,
			stack->tag, ccache->name, stack->err);

		/* If evict produced err, return err */
		if (stack->err) {
			cache_get_block(ccache->cache, stack->set, stack->way, NULL, &stack->status);
			assert(stack->status);
			assert(stack->eviction);
			ret->err = 1;
			dir_lock_unlock(stack->dir_lock);
			moesi_stack_return(stack);
			return;
		}

		/* Eviction */
		if (stack->eviction) {
			ccache->evictions++;
			cache_get_block(ccache->cache, stack->set, stack->way, NULL, &stack->status);
			assert(!stack->status);
		}

		/* Return */
		ret->err = 0;
		ret->set = stack->set;
		ret->way = stack->way;
		ret->status = stack->status;
		ret->tag = stack->tag;
		ret->dir_lock = stack->dir_lock;
		moesi_stack_return(stack);
		return;
	}

	abort();
}


void moesi_handler_load(int event, void *data)
{
	struct moesi_stack_t *stack = data, *newstack;
	struct ccache_t *ccache = stack->ccache;

	if (event == EV_MOESI_LOAD)
	{
		cache_debug("%lld %lld 0x%x %s load\n", CYCLE, ID,
			stack->addr, ccache->name);

		/* Call find and lock */
		newstack = moesi_stack_create(stack->id, ccache, stack->addr,
			EV_MOESI_LOAD_ACTION, stack);
		newstack->blocking = 0;
		newstack->read = 1;
		newstack->retry = stack->retry;
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK, newstack, 0);
		return;
	}

	if (event == EV_MOESI_LOAD_ACTION)
	{
		int retry_lat;
		cache_debug("  %lld %lld 0x%x %s load action\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Error locking */
		if (stack->err) {
			ccache->read_retries++;
			retry_lat = RETRY_LATENCY;
			cache_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOESI_LOAD, stack, retry_lat);
			return;
		}

		/* Hit */
		if (stack->status) {
			esim_schedule_event(EV_MOESI_LOAD_FINISH, stack, 0);
			return;
		}

		/* Miss */
		newstack = moesi_stack_create(stack->id, ccache, stack->tag,
			EV_MOESI_LOAD_MISS, stack);
		newstack->target = ccache->next;
		esim_schedule_event(EV_MOESI_READ_REQUEST, newstack, 0);
		return;
	}

	if (event == EV_MOESI_LOAD_MISS)
	{
		int retry_lat;
		cache_debug("  %lld %lld 0x%x %s load miss\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Error on read request. Unlock block and retry load. */
		if (stack->err) {
			ccache->read_retries++;
			retry_lat = RETRY_LATENCY;
			dir_lock_unlock(stack->dir_lock);
			cache_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOESI_LOAD, stack, retry_lat);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(ccache->cache, stack->set, stack->way, stack->tag,
			stack->shared ? moesi_status_shared : moesi_status_exclusive);

		/* Continue */
		esim_schedule_event(EV_MOESI_LOAD_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_LOAD_FINISH)
	{
		cache_debug("%lld %lld 0x%x %s load finish\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Update LRU, unlock, and return. */
		if (ccache->cache)
			cache_access_block(ccache->cache, stack->set, stack->way);
		dir_lock_unlock(stack->dir_lock);
		moesi_stack_return(stack);
		return;
	}

	abort();
}


void moesi_handler_store(int event, void *data)
{
	struct moesi_stack_t *stack = data, *newstack;
	struct ccache_t *ccache = stack->ccache;

	if (event == EV_MOESI_STORE)
	{
		cache_debug("%lld %lld 0x%x %s store\n", CYCLE, ID,
			stack->addr, ccache->name);

		/* Call find and lock */
		newstack = moesi_stack_create(stack->id, ccache, stack->addr,
			EV_MOESI_STORE_ACTION, stack);
		newstack->blocking = 0;
		newstack->read = 0;
		newstack->retry = stack->retry;
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK, newstack, 0);
		return;
	}

	if (event == EV_MOESI_STORE_ACTION)
	{
		int retry_lat;
		cache_debug("  %lld %lld 0x%x %s store action\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Error locking */
		if (stack->err) {
			ccache->write_retries++;
			retry_lat = RETRY_LATENCY;
			cache_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOESI_STORE, stack, retry_lat);
			return;
		}

		/* Hit - status=M/E */
		if (stack->status == moesi_status_modified ||
			stack->status == moesi_status_exclusive)
		{
			esim_schedule_event(EV_MOESI_STORE_FINISH, stack, 0);
			return;
		}

		/* Miss - status=O/S/I */
		newstack = moesi_stack_create(stack->id, ccache, stack->tag,
			EV_MOESI_STORE_FINISH, stack);
		newstack->target = ccache->next;
		esim_schedule_event(EV_MOESI_WRITE_REQUEST, newstack, 0);
		return;
	}

	if (event == EV_MOESI_STORE_FINISH)
	{
		int retry_lat;
		cache_debug("%lld %lld 0x%x %s store finish\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Error in write request, unlock block and retry store. */
		if (stack->err) {
			ccache->write_retries++;
			retry_lat = RETRY_LATENCY;
			dir_lock_unlock(stack->dir_lock);
			cache_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOESI_STORE, stack, retry_lat);
			return;
		}

		/* Update LRU, tag/status, unlock, and return. */
		if (ccache->cache) {
			cache_access_block(ccache->cache, stack->set, stack->way);
			cache_set_block(ccache->cache, stack->set, stack->way,
				stack->tag, moesi_status_modified);
		}
		dir_lock_unlock(stack->dir_lock);
		moesi_stack_return(stack);
		return;
	}

	abort();
}


void moesi_handler_evict(int event, void *data)
{
	struct moesi_stack_t *stack = data, *ret = stack->retstack, *newstack;
	struct ccache_t *ccache = stack->ccache, *target = stack->target;
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t dir_entry_tag, z;

	if (event == EV_MOESI_EVICT)
	{
		/* Default ret value */
		ret->err = 0;

		/* Get block info */
		ccache_get_block(ccache, stack->set, stack->way, &stack->tag, &stack->status);
		assert(stack->status || !dir_entry_group_shared_or_owned(ccache->dir,
			stack->set, stack->way));
		cache_debug("  %lld %lld 0x%x %s evict (set=%d, way=%d, status=%d)\n", CYCLE, ID,
			stack->tag, ccache->name, stack->set, stack->way, stack->status);
	
		/* Save some data */
		stack->src_set = stack->set;
		stack->src_way = stack->way;
		stack->src_tag = stack->tag;
		stack->target = target = ccache->next;

		/* Send write request to all sharers */
		newstack = moesi_stack_create(stack->id, ccache, 0,
			EV_MOESI_EVICT_ACTION, stack);
		newstack->except = NULL;
		newstack->set = stack->set;
		newstack->way = stack->way;
		esim_schedule_event(EV_MOESI_INVALIDATE, newstack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_ACTION)
	{
		cache_debug("  %lld %lld 0x%x %s evict action\n", CYCLE, ID,
			stack->tag, ccache->name);
		
		/* status = I */
		if (stack->status == moesi_status_invalid) {
			esim_schedule_event(EV_MOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* status = M/O */
		if (stack->status == moesi_status_modified ||
			stack->status == moesi_status_owned) {
			net_send_ev(ccache->lonet, ccache->loid, 0,
				ccache->bsize + 8, EV_MOESI_EVICT_RECEIVE, stack);
			stack->writeback = 1;
			return;
		}

		/* status = S/E */
		net_send_ev(ccache->lonet, ccache->loid, 0, 8,
			EV_MOESI_EVICT_RECEIVE, stack);
		return;
	}

	if (event == EV_MOESI_EVICT_RECEIVE) {
		cache_debug("  %lld %lld 0x%x %s evict receive\n", CYCLE, ID,
			stack->tag, target->name);
		
		/* Find and lock */
		newstack = moesi_stack_create(stack->id, target, stack->src_tag,
			EV_MOESI_EVICT_WRITEBACK, stack);
		newstack->blocking = 0;
		newstack->read = 0;
		newstack->retry = 0;
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK, newstack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_WRITEBACK)
	{
		cache_debug("  %lld %lld 0x%x %s evict writeback\n", CYCLE, ID,
			stack->tag, target->name);

		/* Error locking block */
		if (stack->err) {
			ret->err = 1;
			esim_schedule_event(EV_MOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* No writeback */
		if (!stack->writeback) {
			esim_schedule_event(EV_MOESI_EVICT_PROCESS, stack, 0);
			return;
		}

		/* Writeback */
		newstack = moesi_stack_create(stack->id, target, 0,
			EV_MOESI_EVICT_WRITEBACK_EXCLUSIVE, stack);
		newstack->except = ccache;
		newstack->set = stack->set;
		newstack->way = stack->way;
		esim_schedule_event(EV_MOESI_INVALIDATE, newstack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_WRITEBACK_EXCLUSIVE)
	{
		cache_debug("  %lld %lld 0x%x %s evict writeback exclusive\n", CYCLE, ID,
			stack->tag, target->name);

		/* Status = O/S/I */
		assert(stack->status != moesi_status_invalid);
		if (stack->status == moesi_status_owned || stack->status ==
			moesi_status_shared)
		{
			newstack = moesi_stack_create(stack->id, target, stack->tag,
				EV_MOESI_EVICT_WRITEBACK_FINISH, stack);
			newstack->target = target->next;
			esim_schedule_event(EV_MOESI_WRITE_REQUEST, newstack, 0);
			return;
		}

		/* Status = M/E */
		esim_schedule_event(EV_MOESI_EVICT_WRITEBACK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_WRITEBACK_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s evict writeback finish\n", CYCLE, ID,
			stack->tag, target->name);

		/* Error in write request */
		if (stack->err) {
			ret->err = 1;
			dir_lock_unlock(stack->dir_lock);
			esim_schedule_event(EV_MOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* Set tag, status and lru */
		if (target->cache) {
			cache_set_block(target->cache, stack->set, stack->way, stack->tag,
				moesi_status_modified);
			cache_access_block(target->cache, stack->set, stack->way);
		}
		esim_schedule_event(EV_MOESI_EVICT_PROCESS, stack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_PROCESS)
	{

		cache_debug("  %lld %lld 0x%x %s evict process\n", CYCLE, ID,
			stack->tag, target->name);

		/* Remove sharer, owner, and unlock */
		dir = ccache_get_dir(target, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			if (dir_entry_tag < stack->src_tag || dir_entry_tag >= stack->src_tag + ccache->bsize)
				continue;
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, dir_entry, ccache->loid);
			if (dir_entry->owner == ccache->loid)
				dir_entry->owner = 0;
		}
		dir_lock_unlock(stack->dir_lock);

		esim_schedule_event(EV_MOESI_EVICT_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_REPLY)
	{
		cache_debug("  %lld %lld 0x%x %s evict reply\n", CYCLE, ID,
			stack->tag, target->name);
		
		net_send_ev(target->hinet, 0, ccache->loid, 8,
			EV_MOESI_EVICT_REPLY_RECEIVE, stack);
		return;

	}

	if (event == EV_MOESI_EVICT_REPLY_RECEIVE)
	{
		cache_debug("  %lld %lld 0x%x %s evict reply receive\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Invalidate block if there was no error. */
		if (!stack->err)
			cache_set_block(ccache->cache, stack->src_set, stack->src_way,
				0, moesi_status_invalid);
		assert(!dir_entry_group_shared_or_owned(ccache->dir,
			stack->src_set, stack->src_way));
		esim_schedule_event(EV_MOESI_EVICT_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_EVICT_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s evict finish\n", CYCLE, ID,
			stack->tag, ccache->name);
		
		moesi_stack_return(stack);
		return;
	}

	abort();
}


void moesi_handler_read_request(int event, void *data)
{
	struct moesi_stack_t *stack = data, *ret = stack->retstack, *newstack;
	struct ccache_t *ccache = stack->ccache, *target = stack->target;
	uint32_t dir_entry_tag, z;
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	if (event == EV_MOESI_READ_REQUEST)
	{
		struct net_t *net;
		int src, dest;
		cache_debug("  %lld %lld 0x%x %s read request\n", CYCLE, ID,
			stack->addr, ccache->name);

		/* Default return values*/
		ret->shared = 0;
		ret->err = 0;

		/* Send request to target */
		assert(ccache->next == target || target->next == ccache);
		net = ccache->next == target ? ccache->lonet : ccache->hinet;
		src = ccache->next == target ? ccache->loid : 0;
		dest = ccache->next == target ? 0 : target->loid;
		net_send_ev(net, src, dest, 8, EV_MOESI_READ_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_RECEIVE)
	{
		cache_debug("  %lld %lld 0x%x %s read request receive\n", CYCLE, ID,
			stack->addr, target->name);
		
		/* Find and lock */
		newstack = moesi_stack_create(stack->id, target, stack->addr,
			EV_MOESI_READ_REQUEST_ACTION, stack);
		newstack->blocking = target->next == ccache;
		newstack->read = 1;
		newstack->retry = 0;
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK, newstack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_ACTION)
	{
		cache_debug("  %lld %lld 0x%x %s read request action\n", CYCLE, ID,
			stack->tag, target->name);

		/* Check block locking error. If read request is down-up, there should not
		 * have been any error while locking. */
		if (stack->err) {
			assert(ccache->next == target);
			ret->err = 1;
			stack->response = 8;
			esim_schedule_event(EV_MOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}
		esim_schedule_event(ccache->next == target ? EV_MOESI_READ_REQUEST_UPDOWN :
			EV_MOESI_READ_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_UPDOWN)
	{
		struct ccache_t *owner;

		cache_debug("  %lld %lld 0x%x %s read request updown\n", CYCLE, ID,
			stack->tag, target->name);
		stack->pending = 1;
		
		if (stack->status) {
			
			/* Status = M/O/E/S
			 * Check: addr multiple of requester's bsize
			 * Check: no subblock requested by ccache is already owned by ccache */
			assert(stack->addr % ccache->bsize == 0);
			dir = ccache_get_dir(target, stack->tag);
			for (z = 0; z < dir->zsize; z++) {
				dir_entry_tag = stack->tag + z * cache_min_block_size;
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + ccache->bsize)
					continue;
				dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
				assert(dir_entry->owner != ccache->loid);
			}

			/* Send read request to owners other than ccache for all subblocks. */
			for (z = 0; z < dir->zsize; z++) {
				dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
				dir_entry_tag = stack->tag + z * cache_min_block_size;
				if (!dir_entry->owner) /* no owner */
					continue;
				if (dir_entry->owner == ccache->loid) /* owner is ccache */
					continue;
				owner = net_get_node_data(target->hinet, dir_entry->owner);
				if (dir_entry_tag % owner->bsize) /* not the first owner subblock */
					continue;

				/* Send read request */
				stack->pending++;
				newstack = moesi_stack_create(stack->id, target, dir_entry_tag,
					EV_MOESI_READ_REQUEST_UPDOWN_FINISH, stack);
				newstack->target = owner;
				esim_schedule_event(EV_MOESI_READ_REQUEST, newstack, 0);
			}
			esim_schedule_event(EV_MOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);

		} else {
			
			/* Status = I */
			assert(!dir_entry_group_shared_or_owned(target->dir,
				stack->set, stack->way));
			newstack = moesi_stack_create(stack->id, target, stack->tag,
				EV_MOESI_READ_REQUEST_UPDOWN_MISS, stack);
			newstack->target = target->next;
			esim_schedule_event(EV_MOESI_READ_REQUEST, newstack, 0);
		}
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_UPDOWN_MISS)
	{
		cache_debug("  %lld %lld 0x%x %s read request updown miss\n", CYCLE, ID,
			stack->tag, target->name);
		
		/* Check error */
		if (stack->err) {
			dir_lock_unlock(stack->dir_lock);
			ret->err = 1;
			stack->response = 8;
			esim_schedule_event(EV_MOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on the return value 'shared'
		 * that comes from a read request into the next cache level.
		 * Also set the tag of the block. */
		cache_set_block(target->cache, stack->set, stack->way, stack->tag,
			stack->shared ? moesi_status_shared : moesi_status_exclusive);
		esim_schedule_event(EV_MOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_UPDOWN_FINISH)
	{
		int shared;

		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		cache_debug("  %lld %lld 0x%x %s read request updown finish\n", CYCLE, ID,
			stack->tag, target->name);

		/* Set owner to 0 for all directory entries not owned by ccache. */
		dir = ccache_get_dir(target, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			if (dir_entry->owner != ccache->loid)
				dir_entry->owner = 0;
		}

		/* For each subblock requested by ccache, set ccache as sharer, and
		 * check whether there is other cache sharing it. */
		shared = 0;
		for (z = 0; z < dir->zsize; z++) {
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + ccache->bsize)
				continue;
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, dir_entry, ccache->loid);
			if (dir_entry->sharers > 1)
				shared = 1;
		}

		/* If no subblock requested by ccache is shared by other cache, set ccache
		 * as owner of all of them. Otherwise, notify requester that the block is
		 * shared by setting the 'shared' return value to true. */
		ret->shared = shared;
		if (!shared) {
			for (z = 0; z < dir->zsize; z++) {
				dir_entry_tag = stack->tag + z * cache_min_block_size;
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + ccache->bsize)
					continue;
				dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
				dir_entry->owner = ccache->loid;
			}
		}

		/* Respond with data, update LRU, unlock */
		stack->response = ccache->bsize + 8;
		if (target->cache)
			cache_access_block(target->cache, stack->set, stack->way);
		dir_lock_unlock(stack->dir_lock);
		esim_schedule_event(EV_MOESI_READ_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_DOWNUP)
	{
		struct ccache_t *owner;

		cache_debug("  %lld %lld 0x%x %s read request downup\n", CYCLE, ID,
			stack->tag, target->name);

		/* Check: status must not be invalid.
		 * By default, only one pending request.
		 * Response depends on status */
		assert(stack->status != moesi_status_invalid);
		stack->pending = 1;
		stack->response = stack->status == moesi_status_exclusive ||
			stack->status == moesi_status_shared ?
			8 : target->bsize + 8;

		/* Send a read request to the owner of each subblock. */
		dir = ccache_get_dir(target, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			if (!dir_entry->owner)  /* no owner */
				continue;
			owner = net_get_node_data(target->hinet, dir_entry->owner);
			if (dir_entry_tag % owner->bsize)  /* not the first subblock */
				continue;
			stack->pending++;
			stack->response = target->bsize + 8;
			newstack = moesi_stack_create(stack->id, target, dir_entry_tag,
				EV_MOESI_READ_REQUEST_DOWNUP_FINISH, stack);
			newstack->target = owner;
			esim_schedule_event(EV_MOESI_READ_REQUEST, newstack, 0);
		}

		esim_schedule_event(EV_MOESI_READ_REQUEST_DOWNUP_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_DOWNUP_FINISH)
	{
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		cache_debug("  %lld %lld 0x%x %s read request downup finish\n", CYCLE, ID,
			stack->tag, target->name);

		/* Set owner of subblocks to 0. */
		dir = ccache_get_dir(target, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			dir_entry->owner = 0;
		}

		/* Set status to S, update LRU, unlock */
		cache_set_block(target->cache, stack->set, stack->way, stack->tag,
			moesi_status_shared);
		cache_access_block(target->cache, stack->set, stack->way);
		dir_lock_unlock(stack->dir_lock);
		esim_schedule_event(EV_MOESI_READ_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_REPLY)
	{
		struct net_t *net;
		int src, dest;
		cache_debug("  %lld %lld 0x%x %s read request reply\n", CYCLE, ID,
			stack->tag, target->name);

		assert(stack->response);
		assert(ccache->next == target || target->next == ccache);
		net = ccache->next == target ? ccache->lonet : ccache->hinet;
		src = ccache->next == target ? 0 : target->loid;
		dest = ccache->next == target ? ccache->loid : 0;
		net_send_ev(net, src, dest, stack->response,
			EV_MOESI_READ_REQUEST_FINISH, stack);
		return;
	}

	if (event == EV_MOESI_READ_REQUEST_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s read request finish\n", CYCLE, ID,
			stack->tag, ccache->name);

		moesi_stack_return(stack);
		return;
	}

	abort();
}


void moesi_handler_write_request(int event, void *data)
{
	struct moesi_stack_t *stack = data, *ret = stack->retstack, *newstack;
	struct ccache_t *ccache = stack->ccache, *target = stack->target;
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t dir_entry_tag, z;


	if (event == EV_MOESI_WRITE_REQUEST)
	{
		struct net_t *net;
		int src, dest;
		cache_debug("  %lld %lld 0x%x %s write request\n", CYCLE, ID,
			stack->addr, ccache->name);

		/* Default return values */
		ret->err = 0;

		/* Send request to target */
		assert(ccache->next == target || target->next == ccache);
		net = ccache->next == target ? ccache->lonet : ccache->hinet;
		src = ccache->next == target ? ccache->loid : 0;
		dest = ccache->next == target ? 0 : target->loid;
		net_send_ev(net, src, dest, 8, EV_MOESI_WRITE_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_RECEIVE)
	{
		cache_debug("  %lld %lld 0x%x %s write request receive\n", CYCLE, ID,
			stack->addr, target->name);
		
		/* Find and lock */
		newstack = moesi_stack_create(stack->id, target, stack->addr,
			EV_MOESI_WRITE_REQUEST_ACTION, stack);
		newstack->blocking = target->next == ccache;
		newstack->read = 0;
		newstack->retry = 0;
		esim_schedule_event(EV_MOESI_FIND_AND_LOCK, newstack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_ACTION)
	{
		cache_debug("  %lld %lld 0x%x %s write request action\n", CYCLE, ID,
			stack->tag, target->name);

		/* Check lock error. If write request is down-up, there should
		 * have been no error. */
		if (stack->err) {
			assert(ccache->next == target);
			ret->err = 1;
			stack->response = 8;
			esim_schedule_event(EV_MOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Invalidate the rest of upper level sharers */
		newstack = moesi_stack_create(stack->id, target, 0,
			EV_MOESI_WRITE_REQUEST_EXCLUSIVE, stack);
		newstack->except = ccache;
		newstack->set = stack->set;
		newstack->way = stack->way;
		esim_schedule_event(EV_MOESI_INVALIDATE, newstack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_EXCLUSIVE)
	{
		cache_debug("  %lld %lld 0x%x %s write request exclusive\n", CYCLE, ID,
			stack->tag, target->name);

		if (ccache->next == target)
			esim_schedule_event(EV_MOESI_WRITE_REQUEST_UPDOWN, stack, 0);
		else
			esim_schedule_event(EV_MOESI_WRITE_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_UPDOWN)
	{
		cache_debug("  %lld %lld 0x%x %s write request updown\n", CYCLE, ID,
			stack->tag, target->name);

		/* status = M/E */
		if (stack->status == moesi_status_modified ||
			stack->status == moesi_status_exclusive) {
			esim_schedule_event(EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);
			return;
		}

		/* status = O/S/I */
		newstack = moesi_stack_create(stack->id, target, stack->tag,
			EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH, stack);
		newstack->target = target->next;
		esim_schedule_event(EV_MOESI_WRITE_REQUEST, newstack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_UPDOWN_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s write request updown finish\n", CYCLE, ID,
			stack->tag, target->name);

		/* Error in write request to next cache level */
		if (stack->err) {
			ret->err = 1;
			stack->response = 8;
			dir_lock_unlock(stack->dir_lock);
			esim_schedule_event(EV_MOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Check that addr is a multiple of ccache.bsize.
		 * Set ccache as sharer and owner. */
		dir = ccache_get_dir(target, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			assert(stack->addr % ccache->bsize == 0);
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + ccache->bsize)
				continue;
			dir_entry = ccache_get_dir_entry(target, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, dir_entry, ccache->loid);
			dir_entry->owner = ccache->loid;
			assert(dir_entry->sharers == 1);
		}

		/* Update LRU, set status: M->M, O/E/S/I->E */
		if (target->cache) {
			cache_access_block(target->cache, stack->set, stack->way);
			if (stack->status != moesi_status_modified)
				cache_set_block(target->cache, stack->set, stack->way,
					stack->tag, moesi_status_exclusive);
		}

		/* Unlock, response is the data of the size of the requester's block. */
		dir_lock_unlock(stack->dir_lock);
		stack->response = ccache->bsize + 8;
		esim_schedule_event(EV_MOESI_WRITE_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_DOWNUP)
	{
		cache_debug("  %lld %lld 0x%x %s write request downup\n", CYCLE, ID,
			stack->tag, target->name);

		/* Compute response, set status to I, unlock */
		assert(stack->status != moesi_status_invalid);
		assert(!dir_entry_group_shared_or_owned(target->dir, stack->set, stack->way));
		stack->response = stack->status == moesi_status_modified || stack->status
			== moesi_status_owned ? target->bsize + 8 : 8;
		cache_set_block(target->cache, stack->set, stack->way, 0, moesi_status_invalid);
		dir_lock_unlock(stack->dir_lock);
		esim_schedule_event(EV_MOESI_WRITE_REQUEST_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_REPLY)
	{
		struct net_t *net;
		int src, dest;
		cache_debug("  %lld %lld 0x%x %s write request reply\n", CYCLE, ID,
			stack->tag, target->name);

		assert(stack->response);
		assert(ccache->next == target || target->next == ccache);
		net = ccache->next == target ? ccache->lonet : ccache->hinet;
		src = ccache->next == target ? 0 : target->loid;
		dest = ccache->next == target ? ccache->loid : 0;
		net_send_ev(net, src, dest, stack->response,
			EV_MOESI_WRITE_REQUEST_FINISH, stack);
		return;
	}

	if (event == EV_MOESI_WRITE_REQUEST_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s write request finish\n", CYCLE, ID,
			stack->tag, ccache->name);

		moesi_stack_return(stack);
		return;
	}

	abort();
}



void moesi_handler_invalidate(int event, void *data)
{
	struct moesi_stack_t *stack = data, *newstack;
	struct ccache_t *ccache = stack->ccache;
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t dir_entry_tag, z;

	if (event == EV_MOESI_INVALIDATE)
	{
		int node_count, i;
		struct ccache_t *sharer;

		/* Get block info */
		ccache_get_block(ccache, stack->set, stack->way, &stack->tag, &stack->status);
		cache_debug("  %lld %lld 0x%x %s invalidate (set=%d, way=%d, status=%d)\n", CYCLE, ID,
			stack->tag, ccache->name, stack->set, stack->way, stack->status);
		stack->pending = 1;

		/* Send write request to all upper level sharers but ccache */
		dir = ccache_get_dir(ccache, stack->tag);
		for (z = 0; z < dir->zsize; z++) {
			dir_entry_tag = stack->tag + z * cache_min_block_size;
			dir_entry = ccache_get_dir_entry(ccache, stack->set, stack->way, z);
			node_count = ccache->hinet ? ccache->hinet->end_node_count : 0;
			for (i = 1; i < node_count; i++) {
				
				/* Skip non-sharers and 'except' */
				if (!dir_entry_is_sharer(dir, dir_entry, i))
					continue;
				sharer = net_get_node_data(ccache->hinet, i);
				if (sharer == stack->except)
					continue;

				/* Clear sharer and owner */
				dir_entry_clear_sharer(dir, dir_entry, i);
				if (dir_entry->owner == i)
					dir_entry->owner = 0;

				/* Send write request upwards if beginning of block */
				if (dir_entry_tag % sharer->bsize)
					continue;
				newstack = moesi_stack_create(stack->id, ccache, dir_entry_tag,
					EV_MOESI_INVALIDATE_FINISH, stack);
				newstack->target = sharer;
				esim_schedule_event(EV_MOESI_WRITE_REQUEST, newstack, 0);
				stack->pending++;
			}
		}
		esim_schedule_event(EV_MOESI_INVALIDATE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOESI_INVALIDATE_FINISH)
	{
		cache_debug("  %lld %lld 0x%x %s invalidate finish\n", CYCLE, ID,
			stack->tag, ccache->name);

		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		moesi_stack_return(stack);
		return;
	}

	abort();
}
