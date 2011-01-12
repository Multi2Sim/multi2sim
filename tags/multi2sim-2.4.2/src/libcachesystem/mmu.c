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


/* Global variables */
uint32_t mmu_page_size = 1 << 12;
uint32_t mmu_log_page_size;
uint32_t mmu_page_mask;


/* Local constants */
#define MMU_PAGE_HASH_SIZE	(1 << 10)
#define MMU_PAGE_LIST_SIZE	(1 << 10)


/* Physical memory page */
struct mmu_page_t {
	struct mmu_page_t *next;
	struct dir_t *dir;
	int mid;  /* memory map id */
	uint32_t vtladdr;  /* virtual address of page */
	uint32_t phaddr;  /* corresponding physical address */
};


struct mmu_t
{
	/* Hash table of pages */
	struct mmu_page_t *page_hash[MMU_PAGE_HASH_SIZE];  /* hash table of pages */
	struct mmu_page_t **page_list;  /* list of pages */
	uint32_t page_count;  /* number of allocated pages */
	uint32_t page_list_size;  /* size of pages list */
};


/* Global memory management unit */
static struct mmu_t *mmu;


void mmu_reg_options()
{
	opt_reg_uint32("-page_size", "Memory page size", &mmu_page_size);
}


void mmu_init()
{
	/* Check page size */
	if ((mmu_page_size & (mmu_page_size - 1)))
		fatal("memory page size must be power of 2");
	mmu_log_page_size = log_base2(mmu_page_size);
	mmu_page_mask = mmu_page_size - 1;

	/* Create mmu */
	mmu = calloc(1, sizeof(struct mmu_t));
	mmu->page_list = calloc(MMU_PAGE_LIST_SIZE, sizeof(void *));
	mmu->page_list_size = MMU_PAGE_LIST_SIZE;
}


void mmu_done()
{
	struct mmu_page_t *page;
	int i;

	/* Free */
	for (i = 0; i < mmu->page_count; i++) {
		page = mmu->page_list[i];
		dir_free(page->dir);
		free(page);
	}
	free(mmu->page_list);
	free(mmu);
}


static struct mmu_page_t *mmu_get_page(int mid, uint32_t vtladdr)
{
	struct mmu_page_t *prev, *page;
	uint32_t tag;
	int idx, node_count;

	/* Look for page */
	idx = ((vtladdr >> mmu_log_page_size) + mid * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~mmu_page_mask;
	prev = NULL;
	page = mmu->page_hash[idx];
	while (page) {
		if (page->vtladdr == tag && page->mid == mid)
			break;
		prev = page;
		page = page->next;
	}
	
	/* Not found */
	if (!page) {
		
		/* Create page */
		page = calloc(1, sizeof(struct mmu_page_t));
		node_count = main_memory->hinet ? main_memory->hinet->end_node_count : 1;
		page->dir = dir_create(mmu_page_size / main_memory->bsize, 1,
			main_memory->bsize / cache_min_block_size, node_count);
		page->vtladdr = tag;
		page->mid = mid;
		page->phaddr = mmu->page_count << mmu_log_page_size;

		/* Insert in page list */
		if (mmu->page_count == mmu->page_list_size) {
			mmu->page_list_size += mmu->page_list_size;
			mmu->page_list = realloc(mmu->page_list,
				mmu->page_list_size * sizeof(void *));
			if (!mmu->page_list)
				abort();
		}
		mmu->page_list[mmu->page_count++] = page;
		page->next = mmu->page_hash[idx];
		mmu->page_hash[idx] = page;
		prev = NULL;
	}
	
	/* Locate at the head of the hash table */
	if (prev) {
		prev->next = page->next;
		page->next = mmu->page_hash[idx];
		mmu->page_hash[idx] = page;
	}

	/* Return it */
	return page;
}


uint32_t mmu_translate(int mid, uint32_t vtladdr)
{
	struct mmu_page_t *page;
	uint32_t offs, phaddr;

	offs = vtladdr & mmu_page_mask;
	page = mmu_get_page(mid, vtladdr);
	assert(page);
	phaddr = page->phaddr | offs;
	return phaddr;
}


struct dir_t *mmu_get_dir(uint32_t phaddr)
{
	uint32_t idx;
	struct mmu_page_t *page;

	/* Get memory page */
	idx = phaddr >> mmu_log_page_size;
	if (idx >= mmu->page_count)
		return NULL;
	page = mmu->page_list[idx];
	return page->dir;
}


int mmu_valid_phaddr(uint32_t phaddr)
{
	uint32_t idx;
	idx = phaddr >> mmu_log_page_size;
	return idx < mmu->page_count;
}

