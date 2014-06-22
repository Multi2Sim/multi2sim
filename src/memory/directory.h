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

#ifndef MEM_SYSTEM_DIRECTORY_H
#define MEM_SYSTEM_DIRECTORY_H


struct dir_lock_t
{
	int lock;
	long long stack_id;
	struct mod_stack_t *lock_queue;
};

#define DIR_ENTRY_OWNER_NONE  (-1)
#define DIR_ENTRY_VALID_OWNER(dir_entry)  ((dir_entry)->owner >= 0)

struct dir_entry_t
{
	int owner;  /* Node owning the block (-1 = No owner)*/
	int num_sharers;  /* Number of 1s in next field */
	unsigned char sharer[0];  /* Bitmap of sharers (must be last field) */
};

struct dir_t
{
	char *name;

	/* Number of possible sharers for a block. This determines
	 * the size of the directory entry bitmap. */
	int num_nodes;

	/* Width, height and depth of the directory. For caches, it is
	 * useful to have a 3-dim directory. XSize is the number of
	 * sets, YSize is the number of ways of the cache, and ZSize
	 * is the number of sub-blocks of size 'cache_min_block_size'
	 * that fit within a block. */
	int xsize, ysize, zsize;

	/* Array of xsize * ysize locks. Each lock corresponds to a
	 * block, i.e. a set of zsize directory entries */
	struct dir_lock_t *dir_lock;

	/* Last field. This is an array of xsize*ysize*zsize elements of type
	 * dir_entry_t, which have likewise variable size. */
	unsigned char data[0];
};

struct dir_t *dir_create(char *name, int xsize, int ysize, int zsize, int num_nodes);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z);

void dir_entry_set_owner(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_set_sharer(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_clear_sharer(struct dir_t *dir, int x, int y, int z, int node);
void dir_entry_clear_all_sharers(struct dir_t *dir, int x, int y, int z);
int dir_entry_is_sharer(struct dir_t *dir, int x, int y, int z, int node);
int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y);

void dir_entry_dump_sharers(struct dir_t *dir, int x, int y, int z);

struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y);
int dir_entry_lock(struct dir_t *dir, int x, int y, int event, struct mod_stack_t *stack);
void dir_entry_unlock(struct dir_t *dir, int x, int y);


#endif

