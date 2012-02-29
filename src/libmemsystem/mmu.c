/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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


#include <mem-system.h>


/*
 * Global variables
 */

uint32_t mmu_page_size = 1 << 12;  /* 4KB default page size */
uint32_t mmu_log_page_size;
uint32_t mmu_page_mask;




/*
 * Private variables
 */

/* Local constants */
#define MMU_PAGE_HASH_SIZE  (1 << 10)
#define MMU_PAGE_LIST_SIZE  (1 << 10)


/* Physical memory page */
struct mmu_page_t
{
	struct mmu_page_t *next;

	int mid;  /* Memory map ID */
	uint32_t vtladdr;  /* Virtual address of page */
	uint32_t phaddr;  /* Physical address */
};


struct mmu_t
{
	/* Hash table of pages */
	struct mmu_page_t *page_hash_table[MMU_PAGE_HASH_SIZE];  /* Hash table of pages */
	struct list_t *page_list;  /* List of pages */
};


/* Global memory management unit */
static struct mmu_t *mmu;





/*
 * Public Functions
 */

void mmu_init()
{
	/* Check page size */
	if ((mmu_page_size & (mmu_page_size - 1)))
		fatal("memory page size must be power of 2");
	mmu_log_page_size = log_base2(mmu_page_size);
	mmu_page_mask = mmu_page_size - 1;

	/* Create MMU */
	mmu = calloc(1, sizeof(struct mmu_t));
	if (!mmu)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	mmu->page_list = list_create_with_size(MMU_PAGE_LIST_SIZE);
}


void mmu_done()
{
	int i;

	/* Free pages */
	for (i = 0; i < list_count(mmu->page_list); i++)
		free(list_get(mmu->page_list, i));
	list_free(mmu->page_list);

	/* Free MMU */
	free(mmu);
}


static struct mmu_page_t *mmu_get_page(int mid, uint32_t vtladdr)
{
	struct mmu_page_t *prev, *page;
	uint32_t tag;
	int index;

	/* Look for page */
	index = ((vtladdr >> mmu_log_page_size) + mid * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~mmu_page_mask;
	prev = NULL;
	page = mmu->page_hash_table[index];
	while (page)
	{
		if (page->vtladdr == tag && page->mid == mid)
			break;
		prev = page;
		page = page->next;
	}
	
	/* Not found */
	if (!page)
	{
		/* Create page */
		page = calloc(1, sizeof(struct mmu_page_t));
		if (!page)
			fatal("%s: out of memory", __FUNCTION__);

		/* Initialize */
		page->vtladdr = tag;
		page->mid = mid;
		page->phaddr = list_count(mmu->page_list) << mmu_log_page_size;

		/* Insert in page list */
		list_add(mmu->page_list, page);

		/* Insert in page hash table */
		page->next = mmu->page_hash_table[index];
		mmu->page_hash_table[index] = page;
		prev = NULL;
	}
	
	/* Locate page at the head of the hash table for faster subsequent lookup */
	if (prev)
	{
		prev->next = page->next;
		page->next = mmu->page_hash_table[index];
		mmu->page_hash_table[index] = page;
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


int mmu_valid_phaddr(uint32_t phaddr)
{
	uint32_t index;

	index = phaddr >> mmu_log_page_size;
	return index < mmu->page_list->count;
}
