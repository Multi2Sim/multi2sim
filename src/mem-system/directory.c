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
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"


#define DIR_ENTRY_SHARERS_SIZE ((dir->num_nodes + 7) / 8)
#define DIR_ENTRY_SIZE (sizeof(struct dir_entry_t) + DIR_ENTRY_SHARERS_SIZE)
#define DIR_ENTRY(X, Y, Z) ((struct dir_entry_t *) (((void *) &dir->data) + DIR_ENTRY_SIZE * \
	((X) * dir->ysize * dir->zsize + (Y) * dir->zsize + (Z))))


struct dir_t *dir_create(char *name, int xsize, int ysize, int zsize, int num_nodes)
{
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	int dir_size;
	int dir_entry_size;

	int x;
	int y;
	int z;
	
	/* Calculate sizes */
	assert(num_nodes > 0);
	dir_entry_size = sizeof(struct dir_entry_t) + (num_nodes + 7) / 8;
	dir_size = sizeof(struct dir_t) + dir_entry_size * xsize * ysize * zsize;

	/* Initialize */
	dir = xcalloc(1, dir_size);
	dir->name = xstrdup(name);
	dir->dir_lock = xcalloc(xsize * ysize, sizeof(struct dir_lock_t));
	dir->num_nodes = num_nodes;
	dir->xsize = xsize;
	dir->ysize = ysize;
	dir->zsize = zsize;

	/* Reset all owners */
	for (x = 0; x < xsize; x++)
	{
		for (y = 0; y < ysize; y++)
		{
			for (z = 0; z < zsize; z++)
			{
				dir_entry = dir_entry_get(dir, x, y, z);
				dir_entry->owner = DIR_ENTRY_OWNER_NONE;
			}
		}
	}

	/* Return */
	return dir;
}


void dir_free(struct dir_t *dir)
{
	free(dir->name);
	free(dir->dir_lock);
	free(dir);
}


struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z)
{
	assert(IN_RANGE(x, 0, dir->xsize - 1));
	assert(IN_RANGE(y, 0, dir->ysize - 1));
	assert(IN_RANGE(z, 0, dir->zsize - 1));
	return DIR_ENTRY(x, y, z);
}


void dir_entry_dump_sharers(struct dir_t *dir, int x, int y, int z)
{
	struct dir_entry_t *dir_entry;
	int i;

	dir_entry = dir_entry_get(dir, x, y, z);
	mem_debug("  %d sharers: { ", dir_entry->num_sharers);
	for (i = 0; i < dir->num_nodes; i++)
		if (dir_entry_is_sharer(dir, x, y, z, i))
			mem_debug("%d ", i);
	mem_debug("}\n");
}


void dir_entry_set_owner(struct dir_t *dir, int x, int y, int z, int node)
{
	struct dir_entry_t *dir_entry;

	/* Set owner */
	assert(node == DIR_ENTRY_OWNER_NONE || IN_RANGE(node, 0, dir->num_nodes - 1));
	dir_entry = dir_entry_get(dir, x, y, z);
	dir_entry->owner = node;

	/* Trace */
	mem_trace("mem.set_owner dir=\"%s\" x=%d y=%d z=%d owner=%d\n",
		dir->name, x, y, z, node);
}


void dir_entry_set_sharer(struct dir_t *dir, int x, int y, int z, int node)
{
	struct dir_entry_t *dir_entry;

	/* Nothing if sharer was already set */
	assert(IN_RANGE(node, 0, dir->num_nodes - 1));
	dir_entry = dir_entry_get(dir, x, y, z);
	if (dir_entry->sharer[node / 8] & (1 << (node % 8)))
		return;

	/* Set sharer */
	dir_entry->sharer[node / 8] |= 1 << (node % 8);
	dir_entry->num_sharers++;
	assert(dir_entry->num_sharers <= dir->num_nodes);

	/* Debug */
	mem_trace("mem.set_sharer dir=\"%s\" x=%d y=%d z=%d sharer=%d\n",
		dir->name, x, y, z, node);
}


void dir_entry_clear_sharer(struct dir_t *dir, int x, int y, int z, int node)
{
	struct dir_entry_t *dir_entry;

	/* Nothing if sharer is not set */
	dir_entry = dir_entry_get(dir, x, y, z);
	assert(IN_RANGE(node, 0, dir->num_nodes - 1));
	if (!(dir_entry->sharer[node / 8] & (1 << (node % 8))))
		return;

	/* Clear sharer */
	dir_entry->sharer[node / 8] &= ~(1 << (node % 8));
	assert(dir_entry->num_sharers > 0);
	dir_entry->num_sharers--;

	/* Debug */
	mem_trace("mem.clear_sharer dir=\"%s\" x=%d y=%d z=%d sharer=%d\n",
		dir->name, x, y, z, node);
}


void dir_entry_clear_all_sharers(struct dir_t *dir, int x, int y, int z)
{
	struct dir_entry_t *dir_entry;
	int i;

	/* Clear sharers */
	dir_entry = dir_entry_get(dir, x, y, z);
	dir_entry->num_sharers = 0;
	for (i = 0; i < DIR_ENTRY_SHARERS_SIZE; i++)
		dir_entry->sharer[i] = 0;

	/* Debug */
	mem_trace("mem.clear_all_sharers dir=\"%s\" x=%d y=%d z=%d\n",
		dir->name, x, y, z);
}


int dir_entry_is_sharer(struct dir_t *dir, int x, int y, int z, int node)
{
	struct dir_entry_t *dir_entry;

	assert(IN_RANGE(node, 0, dir->num_nodes - 1));
	dir_entry = dir_entry_get(dir, x, y, z);
	return (dir_entry->sharer[node / 8] & (1 << (node % 8))) > 0;
}


int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y)
{
	struct dir_entry_t *dir_entry;
	int z;
	for (z = 0; z < dir->zsize; z++)
	{
		dir_entry = DIR_ENTRY(x, y, z);
		if (dir_entry->num_sharers || DIR_ENTRY_VALID_OWNER(dir_entry))
			return 1;
	}
	return 0;
}


struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y)
{
	struct dir_lock_t *dir_lock;

	assert(x < dir->xsize && y < dir->ysize);
	dir_lock = &dir->dir_lock[x * dir->ysize + y];
	mem_debug("  dir_lock retrieve\n");
	return dir_lock;
}


int dir_entry_lock(struct dir_t *dir, int x, int y, int event, struct mod_stack_t *stack)
{
	struct dir_lock_t *dir_lock;
	struct mod_stack_t *lock_queue_iter;

	/* Get lock */
	assert(x < dir->xsize && y < dir->ysize);
	dir_lock = &dir->dir_lock[x * dir->ysize + y];

	/* If the entry is already locked, enqueue a new waiter and
	 * return failure to lock. */
	if (dir_lock->lock)
	{
		/* Enqueue the stack to the end of the lock queue */
		stack->dir_lock_next = NULL;
		stack->dir_lock_event = event;
		stack->ret_stack->way = stack->way;

		if (!dir_lock->lock_queue)
		{
			/* Special case: queue is empty */
			dir_lock->lock_queue = stack;
		}
		else 
		{
			lock_queue_iter = dir_lock->lock_queue;

			/* FIXME - Code below is the queue insertion algorithm based on stack id.
			 * This causes a deadlock when, for example, A-10 keeps retrying an up-down access and
			 * gets always priority over A-20, which is waiting to finish a down-up access. */
#if 0
			while (stack->id > lock_queue_iter->id)
			{
				if (!lock_queue_iter->dir_lock_next)
					break;

				lock_queue_iter = lock_queue_iter->dir_lock_next;
			}
#endif
			/* ------------------------------------------------------------------------ */
			/* FIXME - Replaced with code below, just inserting at the end of the queue.
			 * But this seems to be what this function was doing before, isn't it? Why
			 * weren't we happy with this policy? */
			while (lock_queue_iter->dir_lock_next)
				lock_queue_iter = lock_queue_iter->dir_lock_next;
			/* ------------------------------------------------------------------------ */

			if (!lock_queue_iter->dir_lock_next) 
			{
				/* Stack goes at end of queue */
				lock_queue_iter->dir_lock_next = stack;
			}
			else 
			{
				/* Stack goes in front or middle of queue */
				stack->dir_lock_next = lock_queue_iter->dir_lock_next;
				lock_queue_iter->dir_lock_next = stack;
			}
		}
		mem_debug("    0x%x access suspended\n", stack->tag);
		return 0;
	}

	/* Trace */
	mem_trace("mem.new_access_block cache=\"%s\" access=\"A-%lld\" set=%d way=%d\n",
		dir->name, stack->id, x, y);

	/* Lock entry */
	dir_lock->lock = 1;
	dir_lock->stack_id = stack->id;
	return 1;
}


void dir_entry_unlock(struct dir_t *dir, int x, int y)
{
	struct dir_lock_t *dir_lock;
	struct mod_stack_t *stack;
	FILE *f;

	/* Get lock */
	assert(x < dir->xsize && y < dir->ysize);
	dir_lock = &dir->dir_lock[x * dir->ysize + y];

	/* Wake up first waiter */
	if (dir_lock->lock_queue)
	{
		/* Debug */
		f = debug_file(mem_debug_category);
		if (f)
		{
			mem_debug("    A-%lld resumed", dir_lock->lock_queue->id);
			if (dir_lock->lock_queue->dir_lock_next)
			{
				mem_debug(" - {");
				for (stack = dir_lock->lock_queue->dir_lock_next; stack;
						stack = stack->dir_lock_next)
					mem_debug(" A-%lld", stack->id);
				mem_debug(" } still waiting");
			}
			mem_debug("\n");
		}

		/* Wake up access */
		esim_schedule_event(dir_lock->lock_queue->dir_lock_event, dir_lock->lock_queue, 1);
		dir_lock->lock_queue = dir_lock->lock_queue->dir_lock_next;
	}

	/* Trace */
	mem_trace("mem.end_access_block cache=\"%s\" access=\"A-%lld\" set=%d way=%d\n",
		dir->name, dir_lock->stack_id, x, y);

	/* Unlock entry */
	dir_lock->lock = 0;
}

