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

#include "prefetch-history.h"
#include "module.h"
#include "cache.h"


int prefetch_history_size;

int prefetch_history_is_redundant(struct prefetch_history_t *ph, struct mod_t *mod, 
				  unsigned int phy_addr)
{
	int i, tag1, tag2, set1, set2;

	if(mod->kind != mod_kind_cache)
	{
		/* Doesn't make much sense to prefetch if the memory being
		 * accessed is not a cache memory. */
		return 1;
	}

	/* A 0 sized table means do not track history. Always issue the prefetch. */
	if (ph->size == 0)
		return 0;

	for (i = 0; i < ph->size; i++)
	{
		cache_decode_address(mod->cache, ph->table[i], &set1, &tag1, NULL);
		cache_decode_address(mod->cache, phy_addr, &set2, &tag2, NULL);

		/* If both accesses refer to the same block, return true */
		if (set1 == set2 && tag1 == tag2)
			return 1;
	}

	return 0;
}

void prefetch_history_record(struct prefetch_history_t *ph, unsigned int phy_addr)
{
	/* A 0 sized table means do not track history. */
	if (ph->size == 0)
		return;

	int index = (++(ph->hindex)) % ph->size;
	ph->table[index] = phy_addr;
	ph->hindex = index;
}

struct prefetch_history_t *prefetch_history_create(void)
{
	struct prefetch_history_t *ph;

	/* Initialize */
	ph = xcalloc(1, sizeof(struct prefetch_history_t));
	ph->hindex = -1;
	assert(prefetch_history_size >= 0);
	ph->size = prefetch_history_size;
	if (prefetch_history_size > 0)
		ph->table = xcalloc(prefetch_history_size, sizeof(unsigned));

	/* Return */
	return ph;
}

void prefetch_history_free(struct prefetch_history_t *pf)
{
	free(pf->table);
	free(pf);
}

