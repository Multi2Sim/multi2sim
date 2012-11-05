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

#ifndef MEM_SYSTEM_PREFETCHER_H
#define MEM_SYSTEM_PREFETCHER_H


/* 
 * This file implements a global history buffer
 * based prefetcher. Refer to the 2005 paper by
 * Nesbit and Smith. 
 */


extern int prefetcher_ghb_size;
extern int prefetcher_it_size;

/* Global history buffer. */
struct prefetcher_ghb_t
{
	/* The global address miss this entry corresponds to */
	unsigned int addr;
	/* Next element in the linked list : -1 implies none */
	int next;
	/* Previous element in the linked list : -1 implies none */
	int prev;
	/* Whether the previous element is a GHB entry or a pointer
	 * to the index table. */
	enum prefetcher_ptr_t
	{
		prefetcher_ptr_ghb = 0,
		prefetcher_ptr_it,
	} prev_it_ghb;
};

/* Index table. */
struct prefetcher_it_t
{
	/* The tag to compare to before indexing into the table */
	unsigned int tag;
	/* Pointer into the GHB. -1 implies no entry in GHB. */
   	int ptr;
};

/* The main prefetcher object */
struct prefetcher_t
{
	int ghb_size;
	int it_size;

	struct prefetcher_ghb_t *ghb;
	struct prefetcher_it_t *index_table;

	int ghb_head;
};

struct mod_stack_t;
struct mod_t;

struct prefetcher_t *prefetcher_create(int prefetcher_ghb_size, int prefetcher_it_size);
void prefetcher_free(struct prefetcher_t *pref);

void prefetcher_access_miss(struct mod_stack_t *stack, struct mod_t *mod);

#endif
