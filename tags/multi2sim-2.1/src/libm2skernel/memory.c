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

#include <m2skernel.h>



/* Return mem page corresponding to an address. */
static struct mem_page_t *mem_page_get(struct mem_t *mem, uint32_t addr)
{
	uint32_t index, tag;
	struct mem_page_t *prev, *page;

	tag = addr & ~(MEM_PAGESIZE - 1);
	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];
	prev = NULL;
	
	/* Look for page */
	while (page && page->tag != tag) {
		prev = page;
		page = page->next;
	}
	
	/* Place page into list head */
	if (prev && page) {
		prev->next = page->next;
		page->next = mem->pages[index];
		mem->pages[index] = page;
	}
	
	/* Return found page */
	return page;
}


/* Create new mem page */
static struct mem_page_t *mem_page_create(struct mem_t *mem, uint32_t addr, int perm)
{
	uint32_t index, tag;
	struct mem_page_t *page;

	tag = addr & ~(MEM_PAGESIZE - 1);
	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	
	/* Create new page */
	page = calloc(1, sizeof(struct mem_page_t));
	page->tag = tag;
	page->perm = perm;
	
	/* Insert in pages hash table */
	page->next = mem->pages[index];
	mem->pages[index] = page;
	return page;
}


/* Free mem pages */
static void mem_page_free(struct mem_t *mem, uint32_t addr)
{
	uint32_t index, tag;
	struct mem_page_t *prev, *page;
	
	tag = addr & ~(MEM_PAGESIZE - 1);
	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	prev = NULL;

	/* Find page */
	page = mem->pages[index];
	while (page && page->tag != tag) {
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
	free(page);
}


/* Copy memory pages. All parameters must be multiple of the page size.
 * The pages in the source and destination interval must exist. */
void mem_copy(struct mem_t *mem, uint32_t dest, uint32_t src, int size)
{
	struct mem_page_t *page_dest, *page_src;

	/* Restrictions. No overlapping allowed. */
	assert(!(dest & (MEM_PAGESIZE-1)));
	assert(!(src & (MEM_PAGESIZE-1)));
	assert(!(size & (MEM_PAGESIZE-1)));
	if ((src < dest && src + size > dest) ||
		(dest < src && dest + size > src))
		fatal("mem_copy: cannot copy overlapping regions");
	
	/* Copy */
	while (size > 0) {
		page_dest = mem_page_get(mem, dest);
		page_src = mem_page_get(mem, src);
		assert(page_src && page_dest);
		memcpy(&page_dest->data, &page_src->data, MEM_PAGESIZE);
		src += MEM_PAGESIZE;
		dest += MEM_PAGESIZE;
		size -= MEM_PAGESIZE;
	}
}


/* Return the buffer corresponding to address 'addr' in the simulated
 * mem. The returned buffer is null if addr+size exceeds the page
 * boundaries. */
void *mem_get_buffer(struct mem_t *mem, uint32_t addr, int size,
	enum mem_access_enum access)
{
	struct mem_page_t *page;
	uint32_t offset;

	offset = addr & (MEM_PAGESIZE - 1);
	if (offset + size > MEM_PAGESIZE)
		return NULL;
	page = mem_page_get(mem, addr);
	if (!page)
		return NULL;
	if ((page->perm & access) != access && mem->safe)
		fatal("mem_get_buffer: permission denied at 0x%x", addr);
	return page->data + offset;
}


/* Access mem without exceeding page boundaries. */
static void mem_access_page_boundary(struct mem_t *mem, uint32_t addr,
	int size, void *buf, enum mem_access_enum access)
{
	struct mem_page_t *page;
	uint32_t offset;
	void *data;

	/* Find memory page */
	page = mem_page_get(mem, addr);

	/* If page does not exist and we are in unsafe mode, create it on write
	 * and return 0s on read. */
	if (!page && !mem->safe) {
		switch (access) {
		
		/* Return 0s and exit. */
		case mem_access_read:
		case mem_access_exec:
			memset(buf, 0, size);
			return;
		
		/* Create page */
		case mem_access_write:
		case mem_access_init:
			page = mem_page_create(mem, addr, mem_access_read |
				mem_access_write | mem_access_exec |
				mem_access_init);
			break;

		default:
			panic("mem_access: unknown access");
		}
	}

	/* If we are in safe mode, check permissions. */
	if (mem->safe) {
		if (!page)
			fatal("mem_access: segmentation fault accessing 0x%x", addr);
		if ((page->perm & access) != access)
			fatal("mem_access: permission denied at 0x%x", addr);
	}
	
	/* Access */
	offset = addr & (MEM_PAGESIZE - 1);
	assert(offset + size <= MEM_PAGESIZE);
	data = page->data + offset;
	switch (access) {
	case mem_access_read:
	case mem_access_exec:
		memcpy(buf, data, size);
		break;
	case mem_access_write:
	case mem_access_init:
		memcpy(data, buf, size);
		break;
	default:
		panic("mem_access: unknown access");
	}
}


/* Access mem at address 'addr'.
 * This access can cross page boundaries. */
void mem_access(struct mem_t *mem, uint32_t addr, int size, void *buf,
	enum mem_access_enum access)
{
	uint32_t offset;
	int chunksize;

	mem->last_address = addr;
	while (size) {
		offset = addr & (MEM_PAGESIZE - 1);
		chunksize = MIN(size, MEM_PAGESIZE - offset);
		mem_access_page_boundary(mem, addr, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}


/* Creation and destruction */
struct mem_t *mem_create()
{
	struct mem_t *mem;
	mem = calloc(1, sizeof(struct mem_t));
	mem->sharing = 1;
	mem->safe = 1;
	return mem;
}


void mem_free(struct mem_t *mem)
{
	struct mem_page_t *page;
	void *next;
	int i;
	
	/* Free pages */
	for (i = 0; i < MEM_PAGE_COUNT; i++) {
		page = mem->pages[i];
		while (page) {
			next = page->next;
			free(page);
			page = next;
		}
	}

	/* Free mem */
	free(mem);
}


/* This function finds a free memory region to allocate 'size' bytes
 * starting at address 'addr'. */
uint32_t mem_map_space(struct mem_t *mem, uint32_t addr, int size)
{
	uint32_t tag_start, tag_end;

	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag_start = addr;
	tag_end = addr;
	for (;;) {

		/* Address space overflow */
		if (!tag_end)
			return (uint32_t) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_end)) {
			tag_end += MEM_PAGESIZE;
			tag_start = tag_end;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGESIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGESIZE < size);
		
		/* we have a new free page */
		tag_end += MEM_PAGESIZE;
	}


	/* Return the start of the free space */
	return tag_start;
}


uint32_t mem_map_space_down(struct mem_t *mem, uint32_t addr, int size)
{
	uint32_t tag_start, tag_end;

	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag_start = addr;
	tag_end = addr;
	for (;;) {

		/* Address space overflow */
		if (!tag_start)
			return (uint32_t) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_start)) {
			tag_start -= MEM_PAGESIZE;
			tag_end = tag_start;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGESIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGESIZE < size);
		
		/* we have a new free page */
		tag_start -= MEM_PAGESIZE;
	}

	/* Return the start of the free space */
	return tag_start;
}


/* Allocate (if not already allocated) all necessary memory pages to
 * access 'size' bytes at 'addr'. These two fields do not need to be
 * aligned to page boundaries.
 * If some page already exists, add permissions. */
void mem_map(struct mem_t *mem, uint32_t addr, int size,
	enum mem_access_enum perm)
{
	uint32_t tag1, tag2, tag;
	struct mem_page_t *page;

	/* Calculate page boundaries */
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE) {
		page = mem_page_get(mem, tag);
		if (!page)
			page = mem_page_create(mem, tag, perm);
		page->perm |= perm;
	}
}


/* Deallocate memory pages. The addr and size parameters must be both
 * multiple of the page size. If some page was not allocated, no action
 * is done for that specific page. */
void mem_unmap(struct mem_t *mem, uint32_t addr, int size)
{
	uint32_t tag1, tag2, tag;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE)
		if (mem_page_get(mem, tag))
			mem_page_free(mem, tag);
}


/* Assign protection attributes to pages */
void mem_protect(struct mem_t *mem, uint32_t addr, int size, enum mem_access_enum perm)
{
	uint32_t tag1, tag2, tag;
	struct mem_page_t *page;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE) {
		page = mem_page_get(mem, tag);
		if (page)
			page->perm = perm;
	}
}


void mem_write_string(struct mem_t *mem, uint32_t addr, char *str)
{
	mem_access(mem, addr, strlen(str) + 1, str, mem_access_write);
}


/* Read a string from memory and return the length of the read string.
 * If the return length is equal to max_size, it means that the string did not
 * fit in the destination buffer. */
int mem_read_string(struct mem_t *mem, uint32_t addr, int size, char *str)
{
	int i;
	for (i = 0; i < size; i++) {
		mem_access(mem, addr + i, 1, str + i, mem_access_read);
		if (!str[i])
			break;
	}
	return i;
}


void mem_zero(struct mem_t *mem, uint32_t addr, int size)
{
	unsigned char zero = 0;
	while (size--)
		mem_access(mem, addr++, 0, &zero, mem_access_write);
}


void mem_dump(struct mem_t *mem, char *filename, uint32_t start, uint32_t end)
{
	FILE *f;
	uint32_t size;
	uint8_t buf[MEM_PAGESIZE];

	f = fopen(filename, "wb");
	if (!f)
		fatal("mem_dump: cannot open file '%s'", filename);
	
	/* Set unsafe mode and dump */
	mem->safe = 0;
	while (start < end) {
		size = MIN(MEM_PAGESIZE, end - start);
		mem_access(mem, start, size, buf, mem_access_read);
		fwrite(buf, size, 1, f);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = 1;
	fclose(f);
}


void mem_load(struct mem_t *mem, char *filename, uint32_t start)
{
	FILE *f;
	uint32_t size;
	uint8_t buf[MEM_PAGESIZE];
	
	f = fopen(filename, "rb");
	if (!f)
		fatal("mem_load: cannot open file '%s'", filename);
	
	/* Set unsafe mode and load */
	mem->safe = 0;
	for (;;) {
		size = fread(buf, 1, MEM_PAGESIZE, f);
		if (!size)
			break;
		mem_access(mem, start, size, buf, mem_access_write);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = 1;
	fclose(f);
}

