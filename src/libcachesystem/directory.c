/*
 *  Multi2Sim
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "cachesystem.h"


#define DIR_ENTRY_SHARERS_SIZE ((dir->nodes + 7) / 8)
#define DIR_ENTRY_SIZE (sizeof(struct dir_entry_t) + DIR_ENTRY_SHARERS_SIZE)
#define DIR_ENTRY(X, Y, Z) ((struct dir_entry_t *) (((void *) &dir->data) + DIR_ENTRY_SIZE * \
	((X) * dir->ysize * dir->zsize + (Y) * dir->zsize + (Z))))


struct dir_t *dir_create(int xsize, int ysize, int zsize, int nodes)
{
	int dir_entry_size, dir_size;
	struct dir_t *dir;
	
	assert(nodes);
	dir_entry_size = sizeof(struct dir_entry_t) + (nodes + 7) / 8;
	dir_size = sizeof(struct dir_t) + dir_entry_size * xsize * ysize * zsize;

	dir = calloc(1, dir_size);
	dir->dir_lock = calloc(xsize * ysize, sizeof(struct dir_lock_t));
	dir->nodes = nodes;
	dir->xsize = xsize;
	dir->ysize = ysize;
	dir->zsize = zsize;
	return dir;
}


void dir_free(struct dir_t *dir)
{
	free(dir->dir_lock);
	free(dir);
}


struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z)
{
	assert(x < dir->xsize && y < dir->ysize && z < dir->zsize);
	return DIR_ENTRY(x, y, z);
}


void dir_entry_dump_sharers(struct dir_t *dir, struct dir_entry_t *dir_entry)
{
	int i;
	cache_debug("  %d sharers: { ", dir_entry->sharers);
	for (i = 0; i < dir->nodes; i++)
		if (dir_entry_is_sharer(dir, dir_entry, i))
			printf("%d ", i);
	cache_debug("}\n");
}


void dir_entry_set_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node)
{
	assert(node > 0 && node < dir->nodes);
	if (dir_entry->sharer[node / 8] & (1 << (node % 8)))
		return;
	dir_entry->sharer[node / 8] |= 1 << (node % 8);
	dir_entry->sharers++;
	assert(dir_entry->sharers <= dir->nodes);
}


void dir_entry_clear_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node)
{
	assert(node > 0 && node < dir->nodes);
	if (!(dir_entry->sharer[node / 8] & (1 << (node % 8))))
		return;
	dir_entry->sharer[node / 8] &= ~(1 << (node % 8));
	assert(dir_entry->sharers > 0);
	dir_entry->sharers--;
}


void dir_entry_clear_all_sharers(struct dir_t *dir, volatile struct dir_entry_t *dir_entry)
{
	memset(&dir_entry->sharer, 0, DIR_ENTRY_SHARERS_SIZE);
	dir_entry->sharers = 0;
}


int dir_entry_is_sharer(struct dir_t *dir, struct dir_entry_t *dir_entry, int node)
{
	assert(node >= 0 && node < dir->nodes);
	return (dir_entry->sharer[node / 8] & (1 << (node % 8))) > 0;
}


int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y)
{
	struct dir_entry_t *dir_entry;
	int z;
	for (z = 0; z < dir->zsize; z++) {
		dir_entry = DIR_ENTRY(x, y, z);
		if (dir_entry->sharers || dir_entry->owner)
			return 1;
	}
	return 0;
}


struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y)
{
	struct dir_lock_t *dir_lock;
	assert(x < dir->xsize && y < dir->ysize);
	dir_lock = &dir->dir_lock[x * dir->ysize + y];
	cache_debug("  %lld dir_lock %p - retrieve\n", (long long) esim_cycle, dir_lock);
	return dir_lock;
}


int dir_lock_lock(struct dir_lock_t *dir_lock, int event, struct moesi_stack_t *stack)
{
	cache_debug("  dir_lock %p - lock\n", dir_lock);

	/* If the entry is already locked, enqueue a new waiter and
	 * return failure to lock. */
	if (dir_lock->lock) {
		stack->lock_next = dir_lock->lock_queue;
		stack->lock_event = event;
		dir_lock->lock_queue = stack;
		cache_debug("    0x%x access suspended\n", stack->tag);
		return 0;
	}

	/* Lock entry */
	dir_lock->lock = 1;
	return 1;
}


void dir_lock_unlock(struct dir_lock_t *dir_lock)
{
	cache_debug("  dir_lock %p - unlock\n", dir_lock);

	/* Wake up all waiters */
	while (dir_lock->lock_queue) {
		esim_schedule_event(dir_lock->lock_queue->lock_event, dir_lock->lock_queue, 1);
		cache_debug("    0x%x access resumed\n", dir_lock->lock_queue->tag);
		dir_lock->lock_queue = dir_lock->lock_queue->lock_next;
	}

	/* Unlock entry */
	dir_lock->lock = 0;
}

