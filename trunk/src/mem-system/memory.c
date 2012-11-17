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
#include <lib/util/debug.h>

#include "memory.h"


/* Total space allocated for memory pages */
unsigned long mem_mapped_space = 0;
unsigned long mem_max_mapped_space = 0;

/* Safe mode */
int mem_safe_mode = 1;


/* Return mem page corresponding to an address. */
struct mem_page_t *mem_page_get(struct mem_t *mem, unsigned int addr)
{
	unsigned int index, tag;
	struct mem_page_t *prev, *page;

	tag = addr & ~(MEM_PAGE_SIZE - 1);
	index = (addr >> MEM_LOG_PAGE_SIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];
	prev = NULL;
	
	/* Look for page */
	while (page && page->tag != tag)
	{
		prev = page;
		page = page->next;
	}
	
	/* Place page into list head */
	if (prev && page)
	{
		prev->next = page->next;
		page->next = mem->pages[index];
		mem->pages[index] = page;
	}
	
	/* Return found page */
	return page;
}


/* Return the memory page following addr in the current memory map. This function
 * is useful to reconstruct consecutive ranges of mapped pages. */
struct mem_page_t *mem_page_get_next(struct mem_t *mem, unsigned int addr)
{
	unsigned int tag, index, mintag;
	struct mem_page_t *page, *minpage;

	/* Get tag of the page just following addr */
	tag = (addr + MEM_PAGE_SIZE) & ~(MEM_PAGE_SIZE - 1);
	if (!tag)
		return NULL;
	index = (tag >> MEM_LOG_PAGE_SIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];

	/* Look for a page exactly following addr. If it is found, return it. */
	while (page && page->tag != tag)
		page = page->next;
	if (page)
		return page;
	
	/* Page following addr is not found, so check all memory pages to find
	 * the one with the lowest tag following addr. */
	mintag = 0xffffffff;
	minpage = NULL;
	for (index = 0; index < MEM_PAGE_COUNT; index++)
	{
		for (page = mem->pages[index]; page; page = page->next)
		{
			if (page->tag > tag && page->tag < mintag)
			{
				mintag = page->tag;
				minpage = page;
			}
		}
	}

	/* Return the found page (or NULL) */
	return minpage;
}


/* Create new mem page */
static struct mem_page_t *mem_page_create(struct mem_t *mem, unsigned int addr, int perm)
{
	unsigned int index, tag;
	struct mem_page_t *page;

	/* Initialize */
	page = xcalloc(1, sizeof(struct mem_page_t));
	tag = addr & ~(MEM_PAGE_SIZE - 1);
	index = (addr >> MEM_LOG_PAGE_SIZE) % MEM_PAGE_COUNT;
	page->tag = tag;
	page->perm = perm;
	
	/* Insert in pages hash table */
	page->next = mem->pages[index];
	mem->pages[index] = page;
	mem_mapped_space += MEM_PAGE_SIZE;
	mem_max_mapped_space = MAX(mem_max_mapped_space, mem_mapped_space);

	/* Return */
	return page;
}


/* Free mem pages */
static void mem_page_free(struct mem_t *mem, unsigned int addr)
{
	unsigned int index, tag;
	struct mem_page_t *prev, *page;
	
	tag = addr & ~(MEM_PAGE_SIZE - 1);
	index = (addr >> MEM_LOG_PAGE_SIZE) % MEM_PAGE_COUNT;
	prev = NULL;

	/* Find page */
	page = mem->pages[index];
	while (page && page->tag != tag)
	{
		prev = page;
		page = page->next;
	}
	if (!page)
		return;
	
	/* Free page */
	if (prev)
		prev->next = page->next;
	else
		mem->pages[index] = page->next;
	mem_mapped_space -= MEM_PAGE_SIZE;
	if (page->data)
		free(page->data);
	free(page);
}


/* Copy memory pages. All parameters must be multiple of the page size.
 * The pages in the source and destination interval must exist. */
void mem_copy(struct mem_t *mem, unsigned int dest, unsigned int src, int size)
{
	struct mem_page_t *page_dest, *page_src;

	/* Restrictions. No overlapping allowed. */
	assert(!(dest & (MEM_PAGE_SIZE-1)));
	assert(!(src & (MEM_PAGE_SIZE-1)));
	assert(!(size & (MEM_PAGE_SIZE-1)));
	if ((src < dest && src + size > dest) ||
		(dest < src && dest + size > src))
		fatal("mem_copy: cannot copy overlapping regions");
	
	/* Copy */
	while (size > 0)
	{
		/* Get source and destination pages */
		page_dest = mem_page_get(mem, dest);
		page_src = mem_page_get(mem, src);
		assert(page_src && page_dest);
		
		/* Different actions depending on whether source and
		 * destination page data are allocated. */
		if (page_src->data)
		{
			if (!page_dest->data)
				page_dest->data = xmalloc(MEM_PAGE_SIZE);
			memcpy(page_dest->data, page_src->data, MEM_PAGE_SIZE);
		}
		else
		{
			if (page_dest->data)
				memset(page_dest->data, 0, MEM_PAGE_SIZE);
		}

		/* Advance pointers */
		src += MEM_PAGE_SIZE;
		dest += MEM_PAGE_SIZE;
		size -= MEM_PAGE_SIZE;
	}
}


/* Return the buffer corresponding to address 'addr' in the simulated
 * mem. The returned buffer is null if addr+size exceeds the page
 * boundaries. */
void *mem_get_buffer(struct mem_t *mem, unsigned int addr, int size,
	enum mem_access_t access)
{
	struct mem_page_t *page;
	unsigned int offset;

	/* Get page offset and check page bounds */
	offset = addr & (MEM_PAGE_SIZE - 1);
	if (offset + size > MEM_PAGE_SIZE)
		return NULL;
	
	/* Look for page */
	page = mem_page_get(mem, addr);
	if (!page)
		return NULL;
	
	/* Check page permissions */
	if ((page->perm & access) != access && mem->safe)
		fatal("mem_get_buffer: permission denied at 0x%x", addr);
	
	/* Allocate and initialize page data if it does not exist yet. */
	if (!page->data)
		page->data = xcalloc(1, MEM_PAGE_SIZE);
	
	/* Return pointer to page data */
	return page->data + offset;
}


/* Access memory without exceeding page boundaries. */
static void mem_access_page_boundary(struct mem_t *mem, unsigned int addr,
	int size, void *buf, enum mem_access_t access)
{
	struct mem_page_t *page;
	unsigned int offset;

	/* Find memory page and compute offset. */
	page = mem_page_get(mem, addr);
	offset = addr & (MEM_PAGE_SIZE - 1);
	assert(offset + size <= MEM_PAGE_SIZE);

	/* On nonexistent page, raise segmentation fault in safe mode,
	 * or create page with full privileges for writes in unsafe mode. */
	if (!page)
	{
		if (mem->safe)
			fatal("illegal access at 0x%x: page not allocated", addr);
		if (access == mem_access_read || access == mem_access_exec)
		{
			memset(buf, 0, size);
			return;
		}
		if (access == mem_access_write || access == mem_access_init)
		{
			page = mem_page_create(mem, addr, mem_access_read |
				mem_access_write | mem_access_exec |
				mem_access_init);
		}
	}
	assert(page);

	/* If it is a write access, set the 'modified' flag in the page
	 * attributes (perm). This is not done for 'initialize' access. */
	if (access == mem_access_write)
		page->perm |= mem_access_modif;

	/* Check permissions in safe mode */
	if (mem->safe && (page->perm & access) != access)
		fatal("mem_access: permission denied at 0x%x", addr);

	/* Read/execute access */
	if (access == mem_access_read || access == mem_access_exec)
	{
		if (page->data)
			memcpy(buf, page->data + offset, size);
		else
			memset(buf, 0, size);
		return;
	}

	/* Write/initialize access */
	if (access == mem_access_write || access == mem_access_init)
	{
		if (!page->data)
			page->data = xcalloc(1, MEM_PAGE_SIZE);
		memcpy(page->data + offset, buf, size);
		return;
	}

	/* Shouldn't get here. */
	abort();
}


/* Access mem at address 'addr'.
 * This access can cross page boundaries. */
void mem_access(struct mem_t *mem, unsigned int addr, int size, void *buf,
	enum mem_access_t access)
{
	unsigned int offset;
	int chunksize;

	mem->last_address = addr;
	while (size)
	{
		offset = addr & (MEM_PAGE_SIZE - 1);
		chunksize = MIN(size, MEM_PAGE_SIZE - offset);
		mem_access_page_boundary(mem, addr, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}


void mem_read(struct mem_t *mem, unsigned int addr, int size, void *buf)
{
	mem_access(mem, addr, size, buf, mem_access_read);
}


void mem_write(struct mem_t *mem, unsigned int addr, int size, void *buf)
{
	mem_access(mem, addr, size, buf, mem_access_write);
}


/* Creation and destruction */
struct mem_t *mem_create()
{
	struct mem_t *mem;

	/* Initialize */
	mem = xcalloc(1, sizeof(struct mem_t));
	mem->safe = mem_safe_mode;

	/* Return */
	return mem;
}


void mem_free(struct mem_t *mem)
{
	assert(!mem->num_links);
	mem_clear(mem);
	free(mem);
}


struct mem_t *mem_link(struct mem_t *mem)
{
	mem->num_links++;
	return mem;
}


void mem_unlink(struct mem_t *mem)
{
	assert(mem->num_links >= 0);
	if (mem->num_links)
		mem->num_links--;
	else
		mem_free(mem);
}


/* Clear memory */
void mem_clear(struct mem_t *mem)
{
	int i;
	
	for (i = 0; i < MEM_PAGE_COUNT; i++)
		while (mem->pages[i])
			mem_page_free(mem, mem->pages[i]->tag);

}


/* This function finds a free memory region to allocate 'size' bytes
 * starting at address 'addr'. */
unsigned int mem_map_space(struct mem_t *mem, unsigned int addr, int size)
{
	unsigned int tag_start;
	unsigned int tag_end;

	assert(!(addr & (MEM_PAGE_SIZE - 1)));
	assert(!(size & (MEM_PAGE_SIZE - 1)));

	tag_start = addr;
	tag_end = addr;

	for (;;)
	{
		/* Address space overflow */
		if (!tag_end)
			return (unsigned int) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_end))
		{
			tag_end += MEM_PAGE_SIZE;
			tag_start = tag_end;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGE_SIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGE_SIZE < size);
		
		/* we have a new free page */
		tag_end += MEM_PAGE_SIZE;
	}


	/* Return the start of the free space */
	return tag_start;
}


unsigned int mem_map_space_down(struct mem_t *mem, unsigned int addr, int size)
{
	unsigned int tag_start;
	unsigned int tag_end;

	assert(!(addr & (MEM_PAGE_SIZE - 1)));
	assert(!(size & (MEM_PAGE_SIZE - 1)));
	tag_start = addr;
	tag_end = addr;

	for (;;)
	{
		/* Address space overflow */
		if (!tag_start)
			return (unsigned int) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_start))
		{
			tag_start -= MEM_PAGE_SIZE;
			tag_end = tag_start;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGE_SIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGE_SIZE < size);
		
		/* we have a new free page */
		tag_start -= MEM_PAGE_SIZE;
	}

	/* Return the start of the free space */
	return tag_start;
}


/* Allocate (if not already allocated) all necessary memory pages to
 * access 'size' bytes at 'addr'. These two fields do not need to be
 * aligned to page boundaries.
 * If some page already exists, add permissions. */
void mem_map(struct mem_t *mem, unsigned int addr, int size,
	enum mem_access_t perm)
{
	unsigned int tag1, tag2, tag;
	struct mem_page_t *page;

	/* Calculate page boundaries */
	tag1 = addr & ~(MEM_PAGE_SIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGE_SIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGE_SIZE)
	{
		page = mem_page_get(mem, tag);
		if (!page)
			page = mem_page_create(mem, tag, perm);
		page->perm |= perm;
	}
}


/* Deallocate memory pages. The addr and size parameters must be both
 * multiple of the page size.
 * If some page was not allocated, the corresponding address range is skipped.
 * If a host mapping is caught in the range, it is deallocated with a call
 * to 'mem_unmap_host'. */
void mem_unmap(struct mem_t *mem, unsigned int addr, int size)
{
	unsigned int tag1, tag2, tag;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGE_SIZE - 1)));
	assert(!(size & (MEM_PAGE_SIZE - 1)));
	tag1 = addr & ~(MEM_PAGE_SIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGE_SIZE-1);

	/* Deallocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGE_SIZE)
		mem_page_free(mem, tag);
}


/* Assign protection attributes to pages */
void mem_protect(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t perm)
{
	unsigned int tag1, tag2, tag;
	struct mem_page_t *page;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGE_SIZE - 1)));
	assert(!(size & (MEM_PAGE_SIZE - 1)));
	tag1 = addr & ~(MEM_PAGE_SIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGE_SIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGE_SIZE)
	{
		page = mem_page_get(mem, tag);
		if (!page)
			continue;

		/* Set page new protection flags */
		page->perm = perm;
	}
}


void mem_write_string(struct mem_t *mem, unsigned int addr, char *str)
{
	mem_access(mem, addr, strlen(str) + 1, str, mem_access_write);
}


/* Read a string from memory and return the length of the read string.
 * If the return length is equal to max_size, it means that the string did not
 * fit in the destination buffer. */
int mem_read_string(struct mem_t *mem, unsigned int addr, int size, char *str)
{
	int i;
	for (i = 0; i < size; i++)
	{
		mem_access(mem, addr + i, 1, str + i, mem_access_read);
		if (!str[i])
			break;
	}
	return i;
}


void mem_zero(struct mem_t *mem, unsigned int addr, int size)
{
	unsigned char zero = 0;
	while (size--)
		mem_access(mem, addr++, 0, &zero, mem_access_write);
}


void mem_dump(struct mem_t *mem, char *filename, unsigned int start, unsigned int end)
{
	FILE *f;
	unsigned int size;
	unsigned char buf[MEM_PAGE_SIZE];

	f = fopen(filename, "wb");
	if (!f)
		fatal("mem_dump: cannot open file '%s'", filename);
	
	/* Set unsafe mode and dump */
	mem->safe = 0;
	while (start < end)
	{
		size = MIN(MEM_PAGE_SIZE, end - start);
		mem_access(mem, start, size, buf, mem_access_read);
		fwrite(buf, size, 1, f);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = mem_safe_mode;
	fclose(f);
}


void mem_load(struct mem_t *mem, char *file_name, unsigned int start)
{
	FILE *f;
	unsigned int size;
	unsigned char buf[MEM_PAGE_SIZE];
	
	f = fopen(file_name, "rb");
	if (!f)
		fatal("mem_load: cannot open file '%s'", file_name);
	
	/* Set unsafe mode and load */
	mem->safe = 0;
	for (;;)
	{
		size = fread(buf, 1, MEM_PAGE_SIZE, f);
		if (!size)
			break;
		mem_access(mem, start, size, buf, mem_access_write);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = mem_safe_mode;
	fclose(f);
}


/* Copy the entire content of a memory image into another. Any previously existing
 * content in the destination memory image is removed. */
void mem_clone(struct mem_t *dst_mem, struct mem_t *src_mem)
{
	struct mem_page_t *page;

	int i;

	/* Clear destination memory */
	mem_clear(dst_mem);

	/* Copy pages */
	dst_mem->safe = 0;
	for (i = 0; i < MEM_PAGE_COUNT; i++)
	{
		for (page = src_mem->pages[i]; page; page = page->next)
		{
			mem_page_create(dst_mem, page->tag, page->perm);
			if (page->data)
				mem_access(dst_mem, page->tag, MEM_PAGE_SIZE,
					page->data, mem_access_init);
		}
	}

	/* Copy other fields */
	dst_mem->safe = src_mem->safe;
	dst_mem->heap_break = src_mem->heap_break;
}
