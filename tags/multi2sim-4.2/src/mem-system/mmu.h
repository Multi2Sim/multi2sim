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


enum mmu_access_t
{
	mmu_access_invalid = 0,
	mmu_access_read,
	mmu_access_write,
	mmu_access_execute
};

extern char *mmu_report_file_name;

extern unsigned int mmu_page_size;
extern unsigned int mmu_page_mask;
extern unsigned int mmu_log_page_size;

void mmu_init(void);
void mmu_done(void);
void mmu_dump_report(void);

int mmu_address_space_new(void);
unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr);
int mmu_valid_phy_addr(unsigned int phy_addr);

void mmu_access_page(unsigned int phy_addr, enum mmu_access_t access);


#endif

