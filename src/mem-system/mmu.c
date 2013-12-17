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
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "mmu.h"


/*
 * Global variables
 */

unsigned int mmu_page_size = 1 << 12;  /* 4KB default page size */




/*
 * Private variables
 */

/* Local constants */

/* Physical memory page */
struct mmu_page_t
{
	struct mmu_page_t *next;

	int address_space_index;  /* Memory map ID */
	unsigned int vtl_addr;  /* Virtual address of page */
	unsigned int phy_addr;  /* Physical address */

	/* Statistics */
	long long num_read_accesses;
	long long num_write_accesses;
	long long num_execute_accesses;
};

/*
 * Private Functions
 */

static int MMUPageCompare(const void *ptr1, const void *ptr2)
{
	struct mmu_page_t *page1 = (struct mmu_page_t *) ptr1;
	struct mmu_page_t *page2 = (struct mmu_page_t *) ptr2;

	long long num_accesses1;
	long long num_accesses2;

	num_accesses1 = page1->num_read_accesses + page1->num_write_accesses +
		page1->num_execute_accesses;
	num_accesses2 = page2->num_read_accesses + page2->num_write_accesses +
		page2->num_execute_accesses;

	if (num_accesses1 < num_accesses2)
	{
		return 1;
	}
	else if (num_accesses1 == num_accesses2)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static void MMUDumpReport(MMU *self)
{
	struct mmu_page_t *page;

	FILE *f;
	int i;

	long long num_accesses;

	/* Report file */
	f = self->report_file;
	if (!f)
		return;

	/* If MMU is read-only, the address range is bloated with 
	 * NULL pages */
	if (self->read_only)
	{
		int list_entries = list_count(self->page_list);
		int list_index = 0;
		for (i = 0; i < list_entries; i++)
		{
			if (!list_get(self->page_list, list_index))
			{
				list_remove_at(self->page_list, list_index);
				continue;
			}
			list_index++;
		}
	}

	/* Sort list of pages it as per access count */
	list_sort(self->page_list, MMUPageCompare);

	/* Header */
	fprintf(f, "%5s %5s %9s %9s %10s %10s %10s %10s\n", "Idx", "MemID", 
		"VtlAddr", "PhyAddr", "Accesses", "Read", "Write", "Exec");
	for (i = 0; i < 77; i++)
		fprintf(f, "-");
	fprintf(f, "\n");

	/* Dump */
	for (i = 0; i < list_count(self->page_list); i++)
	{
		page = list_get(self->page_list, i);
		if (!page)
			continue;

		num_accesses = page->num_read_accesses + 
			page->num_write_accesses + 
			page->num_execute_accesses;
		fprintf(f, "%5d %5d %9x %9x %10lld %10lld %10lld %10lld\n",
			i + 1, page->address_space_index, page->vtl_addr, 
			page->phy_addr, num_accesses, page->num_read_accesses,
			page->num_write_accesses, page->num_execute_accesses);
	}
}

static struct mmu_page_t *MMUGetPage(MMU *self, int address_space_index, 
	unsigned int vtladdr)
{
	struct mmu_page_t *prev, *page;
	unsigned int tag;
	int index;

	/* Look for page */
	index = ((vtladdr >> self->log_page_size) + address_space_index * 23) % 
		MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~(self->page_mask);
	prev = NULL;
	page = self->page_hash_table[index];
	while (page)
	{
		if (page->vtl_addr == tag && 
			page->address_space_index == address_space_index)
		{
			break;
		}
		prev = page;
		page = page->next;
	}
	
	/* Not found */
	if (!page)
	{
		/* If page is not found, it will be allocated.  If the MMU
		 * is in read-only mode, it is likely that the simulator is
		 * using a unified memory configuration and the GPU is trying
		 * to access a page that has not been initialized by its 
		 * driver */
		if (self->read_only)
		{
			if (self->report_file)
				MMUDumpReport(self);
			
			fatal("%s: MMU was trying to allocate page 0x%x "
				"(asid %d) in read-only mode.", __FUNCTION__, 
				vtladdr, address_space_index);
		}

		/* Initialize */
		page = xcalloc(1, sizeof(struct mmu_page_t));
		page->vtl_addr = tag;
		page->address_space_index = address_space_index;
		page->phy_addr = list_count(self->page_list) << 
			self->log_page_size;

		/* Insert in page list */
		list_add(self->page_list, page);

		/* Insert in page hash table */
		page->next = self->page_hash_table[index];
		self->page_hash_table[index] = page;
		prev = NULL;
	}
	
	/* Locate page at the head of the hash table for faster 
	 * subsequent lookup */
	if (prev)
	{
		prev->next = page->next;
		page->next = self->page_hash_table[index];
		self->page_hash_table[index] = page;
	}

	/* Return it */
	return page;
}


/*
 * Public Functions
 */
void MMUCreate(MMU *self, char *report_file_name)
{

	/* Variables derived from page size */
	self->page_size = mmu_page_size;
	self->log_page_size = log_base2(mmu_page_size);
	self->page_mask = mmu_page_size - 1;

	self->address_space_index = 0;

	self->page_list = list_create_with_size(MMU_PAGE_LIST_SIZE);

	/* Open report file */
	if (*report_file_name)
	{
		self->report_file = file_open_for_write(report_file_name);
		if (!self->report_file)
			fatal("%s: cannot open report file for MMU", 
				report_file_name);
	}

}

void MMUDestroy(MMU *self)
{
	int i;

	/* Dump report */
	MMUDumpReport(self);

	/* Close the report file */
	if (self->report_file)
		fclose(self->report_file);

	/* Free pages */
	for (i = 0; i < list_count(self->page_list); i++)
		free(list_get(self->page_list, i));
	list_free(self->page_list);
}

int MMUAddressSpaceNew(MMU *self)
{
	return self->address_space_index++;
}

unsigned int MMUTranslate(MMU *self, int address_space_index, 
	unsigned int vtl_addr)
{
	struct mmu_page_t *page;

	unsigned int offset;
	unsigned int phy_addr;

	offset = vtl_addr & self->page_mask;
	page = MMUGetPage(self, address_space_index, vtl_addr);
	assert(page);
	phy_addr = page->phy_addr | offset;

	return phy_addr;
}

int MMUValidPhysicalAddr(MMU *self, unsigned int phy_addr)
{
	int index;

	index = phy_addr >> self->log_page_size;
	return index < self->page_list->count;
}

void MMUAccessPage(MMU *self, unsigned int phy_addr, enum mmu_access_t access)
{
	struct mmu_page_t *page;
	int index;

	if (!self->report_file)
		return;

	/* Get page */
	index = phy_addr >> self->log_page_size;
	page = list_get(self->page_list, index);
	if (!page)
	{
		if (self->report_file)
			MMUDumpReport(self);

		fatal("%s: accessing non-allocated page (addr 0x%x)", 
			__FUNCTION__, phy_addr);
	}

	/* Record access */
	switch (access)
	{
	case mmu_access_read:
		page->num_read_accesses++;
		break;

	case mmu_access_write:
		page->num_write_accesses++;
		break;

	case mmu_access_execute:
		page->num_execute_accesses++;
		break;

	default:
		panic("%s: invalid access", __FUNCTION__);
	}
}

void MMUCopyTranslation(MMU *self, int self_address_space_index, MMU *other, 
	int other_address_space_index, unsigned int vtl_addr, unsigned int size)
{
	assert(self->read_only);
	assert(!other->read_only);
	assert(self->page_size = other->page_size);
	assert(self->page_mask = other->page_mask);

	struct mmu_page_t *self_page, *other_page;

	int vtl_index;
	int phy_index;

	unsigned int addr = vtl_addr & ~self->page_mask;

	/* Map all pages in range */
	while (addr <= (vtl_addr + size))
	{
		/* Find existing page in other MMU */
		other_page = MMUGetPage(other, other_address_space_index, addr);

		/* Insert in page list (may need to insert empty pages
		 * in the list first) */
		phy_index = other_page->phy_addr >> self->log_page_size;

		if (list_count(self->page_list) <= phy_index)
		{
			/* Page doesn't exist yet.  Create NULL pages
			 * up to and including the page to be created. */
			while (list_count(self->page_list) <= phy_index+1)
				list_add(self->page_list, NULL);

			/* Create page in self */
			self_page = xcalloc(1, sizeof(struct mmu_page_t));
			self_page->vtl_addr = addr & ~(self->page_mask);
			self_page->phy_addr = other_page->phy_addr;
			self_page->address_space_index = 
				self_address_space_index;

			/* Add page to page-list */
			list_set(self->page_list, phy_index, self_page);
		}
		else if (list_get(self->page_list, phy_index))
		{
			/* Page exists. Continue using it to keep the stats
			 * correct. */
			self_page = list_get(self->page_list, phy_index);
		}
		else
		{
			/* Page location exists, but page is NULL */

			/* Create page in self */
			self_page = xcalloc(1, sizeof(struct mmu_page_t));
			self_page->vtl_addr = addr & ~(self->page_mask);
			self_page->phy_addr = other_page->phy_addr;
			self_page->address_space_index = 
				self_address_space_index;

			/* Set page to page-list */
			list_set(self->page_list, phy_index, self_page);
		}
		if (self->page_list->error_code)
			fatal("%s: error initializing GPU MMU", __FUNCTION__);

		/* Insert in page hash table */
		vtl_index = ((addr >> self->log_page_size) + 
			self_address_space_index * 23) % MMU_PAGE_HASH_SIZE;
		self_page->next = self->page_hash_table[vtl_index];
		self->page_hash_table[vtl_index] = self_page;

		addr += self->page_size;
	}
}
