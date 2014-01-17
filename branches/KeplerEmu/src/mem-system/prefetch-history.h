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

#ifndef MEM_SYSTEM_PREFETCH_HISTORY_H
#define MEM_SYSTEM_PREFETCH_HISTORY_H


/* 
 * This file specifies a prefetch history structure
 * that can be used by CPU/GPUs to keep track of
 * prefetches that they have initiated to the memory.
 * Strictly speaking, this is not a part of the memory
 * system, but keeping it here so that all archs can
 * access it.
 *
 * At present, the structure is a simple table that
 * remembers recent prefetches and suggests not to
 * prefetch them. This probably needs improvement.
 */

struct mod_t;
extern int prefetch_history_size;

struct prefetch_history_t
{
	unsigned int *table;
	int size;
	int hindex;
};

int prefetch_history_is_redundant(struct prefetch_history_t *ph, struct mod_t *mod, 
				  unsigned int phy_addr);
void prefetch_history_record(struct prefetch_history_t *ph, unsigned int phy_addr);
struct prefetch_history_t *prefetch_history_create();
void prefetch_history_free(struct prefetch_history_t *pf);

#endif
