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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Public Functions
 */

struct si_work_group_t *si_work_group_create(unsigned int work_group_id, 
	struct si_ndrange_t *ndrange)
{
	struct si_bin_enc_user_element_t *user_elements;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	struct si_work_item_t *work_item;

	int i;
	int lid;
	int lidx, lidy, lidz;
	int tid;
	int user_element_count;
	int wavefront_id;
	int wavefront_offset;
	int work_item_id;
	int work_item_gidx_start;
	int work_item_gidy_start;
	int work_item_gidz_start;

	/* Number of in work-items in work-group */
	unsigned int work_items_per_group = ndrange->local_size3[0] * 
		ndrange->local_size3[1] * ndrange->local_size3[2];
	assert(work_items_per_group > 0);

	/* Number of wavefronts in work-group */
	unsigned int wavefronts_per_group = (work_items_per_group + 
		(si_emu_wavefront_size - 1)) / si_emu_wavefront_size;
	assert(wavefronts_per_group > 0);

	/* Initialize */
	work_group = xcalloc(1, sizeof(struct si_work_group_t));
	work_group->id = work_group_id;
	work_group->ndrange = ndrange;

	/* Create LDS */
	work_group->lds_module = mem_create();
	work_group->lds_module->safe = 0;

	/* Allocate pointers for work-items (will actually be created when
	 * wavefronts are created) */
	work_group->work_items = xcalloc(si_emu_wavefront_size * 
		wavefronts_per_group, sizeof(void *));
	work_group->wavefronts = xcalloc(wavefronts_per_group, sizeof(void *));
	work_group->wavefront_count = wavefronts_per_group;

	/* Allocate wavefronts and work-items */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		work_group->wavefronts[wavefront_id] = si_wavefront_create(
			work_group->id * wavefronts_per_group + wavefront_id,
			work_group);

		wavefront = work_group->wavefronts[wavefront_id];

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			wavefront_offset = wavefront_id * si_emu_wavefront_size;

			work_group->work_items[wavefront_offset+work_item_id] = 
				wavefront->work_items[work_item_id];
			work_group->work_items[wavefront_offset+work_item_id]->
				work_group = work_group;
		}
	}

	/* Initialize work-group and work-item metadata */
	work_group->id_3d[0] = work_group_id % ndrange->group_count3[0];
	work_group->id_3d[1] = (work_group_id / ndrange->group_count3[0]) % 
		ndrange->group_count3[1];
	work_group->id_3d[2] = work_group_id / (ndrange->group_count3[0] * 
		ndrange->group_count3[1]);
	work_group->id = work_group_id;

	/* Number of work-items in work-group */
	work_group->work_item_count = ndrange->local_size3[2] *
		ndrange->local_size3[1] * ndrange->local_size3[0];

	/* Global ID of work-item (0,0,0) within the work group */
	work_item_gidx_start = work_group->id_3d[0] * ndrange->local_size3[0];
	work_item_gidy_start = work_group->id_3d[1] * ndrange->local_size3[1];
	work_item_gidz_start = work_group->id_3d[2] * ndrange->local_size3[2];

	/* Initialize work-item metadata */
	lid = 0;
	tid = work_group->id * work_items_per_group;
	for (lidz = 0; lidz < ndrange->local_size3[2]; lidz++)
	{
		for (lidy = 0; lidy < ndrange->local_size3[1]; lidy++)
		{
			for (lidx = 0; lidx < ndrange->local_size3[0]; lidx++)
			{
				work_item = work_group->work_items[lid];

				/* Global IDs */
				work_item->id_3d[0] = work_item_gidx_start + 
					lidx;
				work_item->id_3d[1] = work_item_gidy_start + 
					lidy;
				work_item->id_3d[2] = work_item_gidz_start + 
					lidz;
				work_item->id = tid;

				/* Local IDs */
				work_item->id_in_work_group_3d[0] = lidx;
				work_item->id_in_work_group_3d[1] = lidy;
				work_item->id_in_work_group_3d[2] = lidz;
				work_item->id_in_work_group = lid;


				/* First, last, and number of work-items 
				 * in wavefront */
				wavefront = work_item->wavefront;
				if (!wavefront->work_item_count)
				{
					wavefront->work_item_id_first = tid;
				}
				wavefront->work_item_id_last = tid;
				wavefront->work_item_count++;

				/* Next work-item */
				tid++;
				lid++;

				/* Initialize the execution mask */
				if (work_item->id_in_wavefront < 32)
				{
					wavefront->sreg[SI_EXEC].as_uint |= 
						1 << work_item->id_in_wavefront;
				}
				else if (work_item->id_in_wavefront < 64)
				{
					wavefront->sreg[SI_EXEC + 1].as_uint |= 
						1 << work_item->id_in_wavefront;
				}
				else 
				{
					fatal("%s: invalid work-item id (%d)",
						__FUNCTION__, 
						work_item->id_in_wavefront);
				}
			}
		}
	}

	/* Intialize wavefront state */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = work_group->wavefronts[wavefront_id];

		/* Set PC */
		wavefront->pc = 0;

		/* Save work-group IDs in scalar registers */
		wavefront->sreg[ndrange->wg_id_sgpr].as_int =
			wavefront->work_group->id_3d[0];
		wavefront->sreg[ndrange->wg_id_sgpr + 1].as_int =
			wavefront->work_group->id_3d[1];
		wavefront->sreg[ndrange->wg_id_sgpr + 2].as_int =
			wavefront->work_group->id_3d[2];

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			/* Store work-item IDs in vector registers */
			work_item = wavefront->work_items[work_item_id];

			/* V0 */
			work_item->vreg[0].as_int = 
				work_item->id_in_work_group_3d[0];  
			/* V1 */
			work_item->vreg[1].as_int = 
				work_item->id_in_work_group_3d[1]; 
			/* V2 */
			work_item->vreg[2].as_int = 
				work_item->id_in_work_group_3d[2];

		}

		/* Initialize sreg pointers to internal data structures */
		user_element_count = ndrange->userElementCount;
		user_elements = ndrange->userElements;
		for (i = 0; i < user_element_count; i++)
		{
			if (user_elements[i].dataClass == IMM_CONST_BUFFER)
			{
				/* Store CB pointer in sregs */
				si_wavefront_init_sreg_with_cb(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass == IMM_UAV)
			{
				/* Store UAV pointer in sregs */
				si_wavefront_init_sreg_with_uav(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount,
					user_elements[i].apiSlot);
			}
			else if (user_elements[i].dataClass ==
				PTR_CONST_BUFFER_TABLE)
			{
				/* Store CB table in sregs */
				si_wavefront_init_sreg_with_cb_table(wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == PTR_UAV_TABLE)
			{
				/* Store UAV table in sregs */
				si_wavefront_init_sreg_with_uav_table(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == PTR_VERTEX_BUFFER_TABLE)
			{
				/* Store VB table in sregs */
				si_wavefront_init_sreg_with_vertex_buffer_table(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == SUB_PTR_FETCH_SHADER)
			{
				/* Store Fetch Shader pointer in sregs */
				si_wavefront_init_sreg_with_fetch_shader(
					wavefront,
					user_elements[i].startUserReg,
					user_elements[i].userRegCount);
			}
			else if (user_elements[i].dataClass == IMM_SAMPLER)
			{
				/* Store sampler in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				PTR_RESOURCE_TABLE)
			{
				/* Store resource table in sregs */
				assert(0);
			}
			else if (user_elements[i].dataClass ==
				PTR_INTERNAL_GLOBAL_TABLE)
			{
				fatal("%s: PTR_INTERNAL_GLOBAL_TABLE not "
					"supported", __FUNCTION__);
			}
			else
			{
				fatal("%s: Unimplemented User Element: "
					"dataClass:%d", __FUNCTION__,
					user_elements[i].dataClass);
			}
		}
	}

	/* Statistics */
	si_emu->work_group_count++;

	/* Return */
	return work_group;
}

void si_work_group_free(struct si_work_group_t *work_group)
{
	struct si_wavefront_t *wavefront;

	int wavefront_id;
	int work_item_id; 

	assert(work_group);

	/* Free wavefronts and work-items */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = work_group->wavefronts[wavefront_id];

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			si_work_item_free(wavefront->work_items[work_item_id]);
		}

		si_work_item_free(wavefront->scalar_work_item);
		si_wavefront_free(wavefront);
	}
	free(work_group->wavefronts);
	free(work_group->work_items);

	/* Free LDS memory module */
	mem_free(work_group->lds_module);

	/* Free work-group */
	memset(work_group, 0, sizeof(struct si_work_group_t));
	free(work_group);
	work_group = NULL;
}
