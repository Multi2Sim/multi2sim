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


void mmu_init()
{
	mmu = calloc(1, sizeof(struct mmu_t));
	mmu->page_list = calloc(MMU_PAGE_LIST_SIZE, sizeof(void *));
	mmu->page_list_size = MMU_PAGE_LIST_SIZE;
}


void mmu_done()
{
	struct mmu_page_t *page;
	int i;

	/* Memory usage stat */
	fprintf(stderr, "mmu.phys_mem  %u  # Physical memory allocated\n",
		mmu->page_count * MMU_PAGE_SIZE);

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
	int idx;

	/* Look for page */
	idx = ((vtladdr >> MMU_LOG_PAGE_SIZE) + mid * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~(MMU_PAGE_SIZE - 1);
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
		page->dir = dir_create(MMU_PAGE_SIZE / cache_block_size, 1,
			main_memory->hinet->num_nodes);
		page->vtladdr = tag;
		page->mid = mid;
		page->phaddr = mmu->page_count << MMU_LOG_PAGE_SIZE;

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

	offs = vtladdr & (MMU_PAGE_SIZE - 1);
	page = mmu_get_page(mid, vtladdr);
	assert(page);
	phaddr = page->phaddr | offs;
	return phaddr;
}


void mmu_get_dir_entry(uint32_t phaddr, struct dir_t **pdir,
	struct dir_entry_t **pdir_entry)
{
	uint32_t idx;
	struct mmu_page_t *page;

	idx = phaddr >> MMU_LOG_PAGE_SIZE;
	*pdir = NULL;
	*pdir_entry = NULL;
	if (idx >= mmu->page_count)
		return;
	page = mmu->page_list[idx];

	/* Return values */
	*pdir = page->dir;
	idx = (phaddr & MMU_PAGE_MASK) >> cache_log_block_size;
	*pdir_entry = dir_entry_get(page->dir, idx, 0);
}


int mmu_valid_phaddr(uint32_t phaddr)
{
	uint32_t idx;
	idx = phaddr >> MMU_LOG_PAGE_SIZE;
	return idx < mmu->page_count;
}

