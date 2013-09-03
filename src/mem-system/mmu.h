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

#ifndef MEM_SYSTEM_MMU_H
#define MEM_SYSTEM_MMU_H

#include <lib/class/class.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#define MMU_PAGE_HASH_SIZE  (1 << 10)
#define MMU_PAGE_LIST_SIZE  (1 << 10)

enum mmu_access_t
{
	mmu_access_invalid = 0,
	mmu_access_read,
	mmu_access_write,
	mmu_access_execute
};

/*
 * Class 'MMU'
 */
CLASS_BEGIN(MMU, Object)

	/* List of pages */
	struct list_t *page_list;

	/* Hash table of pages */
	struct mmu_page_t *page_hash_table[MMU_PAGE_HASH_SIZE];

	/* Report file */
	FILE *report_file;

	unsigned int page_size;
	unsigned int log_page_size;
	unsigned int page_mask;
	int address_space_index;

	unsigned int read_only : 1;

CLASS_END(MMU)

void MMUCreate(MMU *self, char *report_file_name);
void MMUDestroy(MMU *self);

void MMUAccessPage(MMU *self, unsigned int phy_addr, enum mmu_access_t access);
int MMUAddressSpaceNew(MMU *self);
void MMUCopyTranslation(MMU *self, int self_address_space_index, MMU *other, 
	int other_address_space_index, unsigned int vtl_addr, 
	unsigned int size);
unsigned int MMUTranslate(MMU *self, int address_space_index, 
	unsigned int vtl_addr);
int MMUValidPhysicalAddr(MMU *self, unsigned int phy_addr);

extern unsigned int mmu_page_size;

#endif

