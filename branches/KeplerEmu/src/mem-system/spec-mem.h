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

#ifndef MEM_SYSTEM_SPEC_MEM_H
#define MEM_SYSTEM_SPEC_MEM_H

/*
 * Speculative Memory
 *
 * This is a memory used when speculative execution is activated for a context.
 * Speculative memory writes are stored here. A subsequent load during speculative
 * execution will read its value from this memory if it exists, and will load
 * from the actual complete 'mem_t' memory otherwise.
 * When speculative execution ends, the contents of this memory will be just
 * discarded.
 */

/* Number of entries in the hash table of pages */
#define SPEC_MEM_PAGE_TABLE_SIZE  32

/* Page size for memory. Every time a new location is written, a page of this
 * size will be allocated. */
#define SPEC_MEM_LOG_PAGE_SIZE  4
#define SPEC_MEM_PAGE_SIZE  (1 << SPEC_MEM_LOG_PAGE_SIZE)
#define SPEC_MEM_PAGE_MASK  (~(SPEC_MEM_PAGE_SIZE - 1))

/* To prevent an excessive growth of speculative memory, this is a limit of
 * pages. After this limit has reached, no more pages are allocated, reads will
 * be done from the non-speculative memory, and writes will be ignored while
 * in speculative mode. */
#define SPEC_MEM_MAX_PAGE_COUNT  100

struct spec_mem_page_t
{
	unsigned int addr;
	unsigned char data[SPEC_MEM_PAGE_SIZE];
	struct spec_mem_page_t *next;
};

struct spec_mem_t
{
	struct mem_t *mem;  /* Associated non-speculative memory */

	int page_count;  /* Number of words currently written */
	struct spec_mem_page_t *pages[SPEC_MEM_PAGE_TABLE_SIZE];  /* Hash table */
};

struct spec_mem_t *spec_mem_create(struct mem_t *mem);
void spec_mem_free(struct spec_mem_t *spec_mem);

void spec_mem_read(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf);
void spec_mem_write(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf);

void spec_mem_clear(struct spec_mem_t *spec_mem);


#endif

