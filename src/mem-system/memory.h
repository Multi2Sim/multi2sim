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

#ifndef MEM_SYSTEM_MEMORY_H
#define MEM_SYSTEM_MEMORY_H


#define MEM_LOG_PAGE_SIZE  12
#define MEM_PAGE_SHIFT  MEM_LOG_PAGE_SIZE
#define MEM_PAGE_SIZE  (1 << MEM_LOG_PAGE_SIZE)
#define MEM_PAGE_MASK  (~(MEM_PAGE_SIZE - 1))
#define MEM_PAGE_COUNT  1024

enum mem_access_t
{
	mem_access_none   = 0x00,
	mem_access_read   = 0x01,
	mem_access_write  = 0x02,
	mem_access_exec   = 0x04,
	mem_access_init   = 0x08,
	mem_access_modif  = 0x10
};

/* Safe mode */
extern int mem_safe_mode;

/* A 4KB page of memory */
struct mem_page_t
{
	unsigned int tag;
	enum mem_access_t perm;  /* Access permissions; combination of flags */
	struct mem_page_t *next;
	unsigned char *data;
};

struct mem_t
{
	/* Number of extra contexts sharing memory image */
	int num_links;

	/* Memory pages */
	struct mem_page_t *pages[MEM_PAGE_COUNT];

	/* Safe mode */
	int safe;

	/* Heap break for CPU contexts */
	unsigned int heap_break;

	/* Last accessed address */
	unsigned int last_address;
};

extern unsigned long mem_mapped_space;
extern unsigned long mem_max_mapped_space;

struct mem_t *mem_create(void);
void mem_free(struct mem_t *mem);

struct mem_t *mem_link(struct mem_t *mem);
void mem_unlink(struct mem_t *mem);

void mem_clear(struct mem_t *mem);

struct mem_page_t *mem_page_get(struct mem_t *mem, unsigned int addr);
struct mem_page_t *mem_page_get_next(struct mem_t *mem, unsigned int addr);

unsigned int mem_map_space(struct mem_t *mem, unsigned int addr, int size);
unsigned int mem_map_space_down(struct mem_t *mem, unsigned int addr, int size);

void mem_map(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t perm);
void mem_unmap(struct mem_t *mem, unsigned int addr, int size);

void mem_protect(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t perm);
void mem_copy(struct mem_t *mem, unsigned int dest, unsigned int src, int size);

void mem_access(struct mem_t *mem, unsigned int addr, int size, void *buf, enum mem_access_t access);
void mem_read(struct mem_t *mem, unsigned int addr, int size, void *buf);
void mem_write(struct mem_t *mem, unsigned int addr, int size, void *buf);

void mem_zero(struct mem_t *mem, unsigned int addr, int size);
int mem_read_string(struct mem_t *mem, unsigned int addr, int size, char *str);
void mem_write_string(struct mem_t *mem, unsigned int addr, char *str);
void *mem_get_buffer(struct mem_t *mem, unsigned int addr, int size, enum mem_access_t access);

void mem_dump(struct mem_t *mem, char *filename, unsigned int start, unsigned int end);
void mem_load(struct mem_t *mem, char *filename, unsigned int start);

void mem_clone(struct mem_t *dst_mem, struct mem_t *src_mem);
#endif

