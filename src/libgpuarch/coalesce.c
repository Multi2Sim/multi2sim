/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpuarch.h>
#include <repos.h>


struct repos_t *gpu_mem_access_repos;


void gpu_mem_access_list_dump(struct lnlist_t *access_list, FILE *f)
{
	struct gpu_mem_access_t *access;
	char *comma = "";

	for (lnlist_head(access_list); !lnlist_eol(access_list); lnlist_next(access_list)) {
		access = lnlist_get(access_list);
		fprintf(f, "%s%u (%u)", comma, access->addr, access->size);
		comma = ", ";
	}
	fprintf(f, "\n");
}


void gpu_mem_access_list_create_from_subwavefront(struct lnlist_t *access_list,
	struct gpu_uop_t *uop, int subwavefront_id)
{
	struct gpu_mem_access_t *access;
	struct gpu_wavefront_t *wavefront = uop->wavefront;
	struct gpu_ndrange_t *ndrange = wavefront->ndrange;
	struct gpu_work_item_uop_t *work_item_uop;
	struct gpu_work_item_t *work_item;
	int work_item_id;

	assert(!lnlist_count(access_list));
	FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(wavefront, subwavefront_id, work_item_id) {
		work_item = ndrange->work_items[work_item_id];
		work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];

		access = repos_create_object(gpu_mem_access_repos);
		access->addr = work_item_uop->global_mem_access_addr;
		access->size = work_item_uop->global_mem_access_size;

		lnlist_insert(access_list, access);
		lnlist_next(access_list);
	}
	printf("WF %d, SubWF %d: ", wavefront->id, subwavefront_id);
	gpu_mem_access_list_dump(access_list, stdout);
}


void gpu_mem_access_list_coalesce(struct lnlist_t *access_list, uint32_t width)
{
	struct gpu_mem_access_t *access, *prev = NULL;

	/* 'width' must be power of 2 */
	assert(!(width & (width - 1)));

	/* First pass.
	 * Group continuous or overlapping accesses */
	lnlist_head(access_list);
	for (;;) {
		
		/* Get current access; stop if no more accesses. */
		access = lnlist_get(access_list);
		if (!access)
			break;

		/* Align access */
		access->addr = ROUND_DOWN(access->addr, width);
		access->size = ROUND_UP(access->size, width);

		/* No previous access */
		if (!prev) {
			prev = access;
			lnlist_next(access_list);
			continue;
		}

		/* If previous and current accesses overlap or are contiguous,
		 * combine them into a single coalesced access. */
		if (IN_RANGE(prev->addr, access->addr, access->addr + access->size) ||
			IN_RANGE(access->addr, prev->addr, prev->addr + prev->size))
		{
			uint32_t end;

			end = MAX(prev->addr + prev->size, access->addr + access->size);
			prev->addr = MIN(prev->addr, access->addr);
			prev->size = end - prev->addr;

			lnlist_remove(access_list);
			repos_free_object(gpu_mem_access_repos, access);
			continue;
		}

		/* Advance to next access */
		prev = access;
		lnlist_next(access_list);
		continue;
	}

	printf("\t1st pass: "); ///
	gpu_mem_access_list_dump(access_list, stdout); ///
	fflush(stdout); ///

	/* Second pass.
	 * Split accesses into groups of 'width' bytes */
	lnlist_head(access_list);
	for (;;) {
		
		/* Get current access. Stop if no more. */
		access = lnlist_get(access_list);
		if (!access)
			break;

		/* If access size is equals to 'width', do nothing. */
		assert(access->size >= width);
		if (access->size == width) {
			lnlist_next(access_list);
			continue;
		}

		/* Remove this access from the list */
		lnlist_remove(access_list);
		prev = access;

		/* Insert several accesses of 'width' size */
		while (prev->size) {
			access = repos_create_object(gpu_mem_access_repos);
			access->addr = prev->addr;
			access->size = width;

			lnlist_insert(access_list, access);
			lnlist_next(access_list);

			prev->addr += width;
			prev->size -= width;
		}

		/* Free original access */
		repos_free_object(gpu_mem_access_repos, prev);
	}
	
	printf("\t2nd pass: "); ///
	gpu_mem_access_list_dump(access_list, stdout); ///
}

