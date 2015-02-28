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

#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>

#include "memory.h"
#include "spec-mem.h"


/*
 * Private Functions
 */

static struct spec_mem_page_t *spec_mem_page_get(struct spec_mem_t *spec_mem, unsigned int addr)
{
	struct spec_mem_page_t *page;
	int index;

	/* Get page position */
	addr &= SPEC_MEM_PAGE_MASK;
	index = (addr >> SPEC_MEM_LOG_PAGE_SIZE) % SPEC_MEM_PAGE_TABLE_SIZE;

	/* Look for page */
	page = spec_mem->pages[index];
	while (page && page->addr != addr)
		page = page->next;

	/* Return it (could be NULL) */
	return page;
}


static struct spec_mem_page_t *spec_mem_page_create(struct spec_mem_t *spec_mem, unsigned int addr)
{
	struct mem_t *mem = spec_mem->mem;
	struct spec_mem_page_t *page;
	int index;

	/* Get page position */
	addr &= SPEC_MEM_PAGE_MASK;
	index = (addr >> SPEC_MEM_LOG_PAGE_SIZE) % SPEC_MEM_PAGE_TABLE_SIZE;

	/* Create page */
	page = xcalloc(1, sizeof(struct spec_mem_page_t));
	page->addr = addr;

	/* Read initial contents of page. The read has to be done in unsafe mode,
	 * since we might be speculatively accessing an invalid page. */
	mem->safe = 0;
	mem_read(mem, addr, SPEC_MEM_PAGE_SIZE, page->data);
	mem->safe = mem_safe_mode;

	/* Insert page in hash table */
	page->next = spec_mem->pages[index];
	spec_mem->pages[index] = page;
	spec_mem->page_count++;

	/* Return it */
	return page;
}


static void spec_mem_access_aligned(struct spec_mem_t *spec_mem, unsigned int addr, int size,
	void *buf, enum mem_access_t access)
{
	struct mem_t *mem = spec_mem->mem;
	struct spec_mem_page_t *page;
	unsigned int offset;

	/* Get the memory page */
	page = spec_mem_page_get(spec_mem, addr);
	if (!page)
	{
		/* On a read, an access to a non-existent location in speculative memory
		 * just translates into the corresponding access to non-speculative memory.
		 * This access must be made in unsafe mode, in case the page is invalid. */
		if (access == mem_access_read)
		{
			mem->safe = 0;
			mem_read(mem, addr, size, buf);
			mem->safe = mem_safe_mode;
			return;
		}
		else
		{
			/* On a write, we need to create a new page. If the maximum number
			 * of pages has been reached, the write will be ignored. */
			if (spec_mem->page_count >= SPEC_MEM_MAX_PAGE_COUNT)
				return;

			/* Create page */
			page = spec_mem_page_create(spec_mem, addr);
		}
	}

	/* We have the page, access it. */
	offset = addr & ~SPEC_MEM_PAGE_MASK;
	assert(offset + size <= SPEC_MEM_PAGE_SIZE);
	if (access == mem_access_read)
		memcpy(buf, page->data + offset, size);
	else
		memcpy(page->data + offset, buf, size);
}


static void spec_mem_access(struct spec_mem_t *spec_mem, unsigned int addr, int size,
	void *buf, enum mem_access_t access)
{
	unsigned int offset;
	int chunksize;

	while (size)
	{
		offset = addr & (SPEC_MEM_PAGE_SIZE - 1);
		chunksize = MIN(size, SPEC_MEM_PAGE_SIZE - offset);
		spec_mem_access_aligned(spec_mem, addr, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}




/*
 * Public Functions
 */

struct spec_mem_t *spec_mem_create(struct mem_t *mem)
{
	struct spec_mem_t *spec_mem;

	/* Make sure that the speculative memory page is a divisor of the
	 * non-speculative memory page size. */
	assert(MEM_PAGE_SIZE % SPEC_MEM_PAGE_SIZE == 0);

	/* Create memory */
	spec_mem = xcalloc(1, sizeof(struct spec_mem_t));
	spec_mem->mem = mem;
	return spec_mem;
}


void spec_mem_free(struct spec_mem_t *spec_mem)
{
	spec_mem_clear(spec_mem);
	free(spec_mem);
}


void spec_mem_read(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf)
{
	spec_mem_access(spec_mem, addr, size, buf, mem_access_read);
}


void spec_mem_write(struct spec_mem_t *spec_mem, unsigned int addr, int size, void *buf)
{
	spec_mem_access(spec_mem, addr, size, buf, mem_access_write);
}


void spec_mem_clear(struct spec_mem_t *spec_mem)
{
	struct spec_mem_page_t *page, *next;
	int i;

	for (i = 0; i < SPEC_MEM_PAGE_TABLE_SIZE; i++)
	{
		page = spec_mem->pages[i];
		while (page)
		{
			next = page->next;
			free(page);
			page = next;
		}
		spec_mem->pages[i] = NULL;
	}
	spec_mem->page_count = 0;
}

