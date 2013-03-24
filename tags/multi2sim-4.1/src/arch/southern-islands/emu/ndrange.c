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
#include <lib/util/misc.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"



/*
 * Public Functions
 */

struct si_ndrange_t *si_ndrange_create(char *name)
{
	struct si_ndrange_t *ndrange;

	/* Initialize */
	ndrange = xcalloc(1, sizeof(struct si_ndrange_t));
	ndrange->id = si_emu->ndrange_count++;

	/* Insert in ND-Range list of SouthernIslands emulator */
	DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, ndrange, ndrange);

	/* Instruction histogram */
	if (si_emu_report_file)
		ndrange->inst_histogram = xcalloc(SI_INST_COUNT, 
			sizeof(unsigned int));

	/* Return */
	return ndrange;
}


void si_ndrange_free(struct si_ndrange_t *ndrange)
{
	int i;

	/* Run free notify call-back */
	if (ndrange->free_notify_func)
		ndrange->free_notify_func(ndrange->free_notify_data);

	/* Clear all states that affect lists. */
	si_ndrange_clear_status(ndrange, si_ndrange_pending);
	si_ndrange_clear_status(ndrange, si_ndrange_running);
	si_ndrange_clear_status(ndrange, si_ndrange_finished);

	/* Extract from ND-Range list in Southern Islands emulator */
	assert(DOUBLE_LINKED_LIST_MEMBER(si_emu, ndrange, ndrange));
	DOUBLE_LINKED_LIST_REMOVE(si_emu, ndrange, ndrange);

	/* Free work-groups */
	for (i = 0; i < ndrange->work_group_count; i++)
		si_work_group_free(ndrange->work_groups[i]);
	free(ndrange->work_groups);

	/* Free wavefronts */
	for (i = 0; i < ndrange->wavefront_count; i++)
	{
		si_wavefront_free(ndrange->wavefronts[i]);
		si_work_item_free(ndrange->scalar_work_items[i]);
	}
	free(ndrange->wavefronts);
	free(ndrange->scalar_work_items);

	/* Free work-items */
	for (i = 0; i < ndrange->work_item_count; i++)
		si_work_item_free(ndrange->work_items[i]);
	free(ndrange->work_items);

	/* Free instruction histogram */
	if (ndrange->inst_histogram)
		free(ndrange->inst_histogram);

	/* Free instruction buffer */
	if (ndrange->inst_buffer)
		free(ndrange->inst_buffer);

	/* Free ndrange */
	free(ndrange->name);
	free(ndrange);
}


static void si_ndrange_setup_arrays(struct si_ndrange_t *ndrange)
{
	struct si_work_group_t *work_group;
	struct si_wavefront_t *wavefront;
	struct si_work_item_t *work_item;

	int gidx, gidy, gidz;  /* 3D work-group ID iterators */
	int lidx, lidy, lidz;  /* 3D work-item local ID iterators */

	int tid;  /* Global ID iterator */
	int gid;  /* Group ID iterator */
	int wid;  /* Wavefront ID iterator */
	int lid;  /* Local ID iterator */

	/* Array of work-groups */
	ndrange->work_group_count = ndrange->group_count;
	ndrange->work_group_id_first = 0;
	ndrange->work_group_id_last = ndrange->work_group_count - 1;
	ndrange->work_groups = xcalloc(ndrange->work_group_count, sizeof(void *));
	for (gid = 0; gid < ndrange->group_count; gid++)
	{
		ndrange->work_groups[gid] = si_work_group_create();
		work_group = ndrange->work_groups[gid];
	}

	/* Array of wavefronts */
	ndrange->wavefronts_per_work_group = 
		(ndrange->local_size + si_emu_wavefront_size - 1) /
		si_emu_wavefront_size;
	ndrange->wavefront_count = ndrange->wavefronts_per_work_group * 
		ndrange->work_group_count;
	ndrange->wavefront_id_first = 0;
	ndrange->wavefront_id_last = ndrange->wavefront_count - 1;
	assert(ndrange->wavefronts_per_work_group > 0 && 
		ndrange->wavefront_count > 0);
	ndrange->wavefronts = xcalloc(ndrange->wavefront_count, sizeof(void *));
	ndrange->scalar_work_items = xcalloc(ndrange->wavefront_count, 
		sizeof(void *));

	for (wid = 0; wid < ndrange->wavefront_count; wid++)
	{
		gid = wid / ndrange->wavefronts_per_work_group;
		ndrange->wavefronts[wid] = si_wavefront_create();
		wavefront = ndrange->wavefronts[wid];
		work_group = ndrange->work_groups[gid];

		wavefront->id = wid;
		wavefront->id_in_work_group = wid % 
			ndrange->wavefronts_per_work_group;
		wavefront->ndrange = ndrange;
		wavefront->work_group = work_group;
		DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, running, wavefront);

		/* Initialize the scalar work item */
		ndrange->scalar_work_items[wid] = si_work_item_create();
		wavefront->scalar_work_item = ndrange->scalar_work_items[wid];
		ndrange->scalar_work_items[wid]->wavefront = wavefront;
		ndrange->scalar_work_items[wid]->work_group = work_group;
		ndrange->scalar_work_items[wid]->ndrange = ndrange;
	}

	/* Array of work-items */
	ndrange->work_item_count = ndrange->global_size;
	ndrange->work_item_id_first = 0;
	ndrange->work_item_id_last = ndrange->work_item_count - 1;
	ndrange->work_items = xcalloc(ndrange->work_item_count, sizeof(void *));
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < ndrange->group_count3[2]; gidz++)
	{
		for (gidy = 0; gidy < ndrange->group_count3[1]; gidy++)
		{
			for (gidx = 0; gidx < ndrange->group_count3[0]; gidx++)
			{
				/* Assign work-group ID */
				work_group = ndrange->work_groups[gid];
				work_group->ndrange = ndrange;
				work_group->id_3d[0] = gidx;
				work_group->id_3d[1] = gidy;
				work_group->id_3d[2] = gidz;
				work_group->id = gid;
				si_work_group_set_status(work_group, si_work_group_pending);

				/* First, last, and number of work-items in work-group */
				work_group->work_item_id_first = tid;
				work_group->work_item_id_last = tid + ndrange->local_size;
				work_group->work_item_count = ndrange->local_size;
				work_group->work_items = &ndrange->work_items[tid];
				snprintf(work_group->name, sizeof(work_group->name), "work-group[i%d-i%d]",
					work_group->work_item_id_first, work_group->work_item_id_last);

				/* First ,last, and number of wavefronts in work-group */
				work_group->wavefront_id_first = gid * ndrange->wavefronts_per_work_group;
				work_group->wavefront_id_last = work_group->wavefront_id_first + ndrange->wavefronts_per_work_group - 1;
				work_group->wavefront_count = ndrange->wavefronts_per_work_group;
				work_group->wavefronts = &ndrange->wavefronts[work_group->wavefront_id_first];
				/* Iterate through work-items */
				lid = 0;
				for (lidz = 0; lidz < ndrange->local_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < ndrange->local_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < ndrange->local_size3[0]; lidx++)
						{
							/* Wavefront ID */
							wid = gid * ndrange->wavefronts_per_work_group +
								lid / si_emu_wavefront_size;
							assert(wid < ndrange->wavefront_count);
							wavefront = ndrange->wavefronts[wid];
							
							/* Create work-item */
							ndrange->work_items[tid] = si_work_item_create();
							work_item = ndrange->work_items[tid];
							work_item->ndrange = ndrange;

							/* Global IDs */
							work_item->id_3d[0] = gidx * ndrange->local_size3[0] + lidx;
							work_item->id_3d[1] = gidy * ndrange->local_size3[1] + lidy;
							work_item->id_3d[2] = gidz * ndrange->local_size3[2] + lidz;
							work_item->id = tid;

							/* Local IDs */
							work_item->id_in_work_group_3d[0] = lidx;
							work_item->id_in_work_group_3d[1] = lidy;
							work_item->id_in_work_group_3d[2] = lidz;
							work_item->id_in_work_group = lid;

							/* Other */
							work_item->id_in_wavefront = work_item->id_in_work_group % si_emu_wavefront_size;
							work_item->work_group = ndrange->work_groups[gid];
							work_item->wavefront = ndrange->wavefronts[wid];

							/* First, last, and number of work-items in wavefront */
							if (!wavefront->work_item_count)
							{
								wavefront->work_item_id_first = tid;
								wavefront->work_items = &ndrange->work_items[tid];
							}
							wavefront->work_item_count++;
							wavefront->work_item_id_last = tid;

							/* Next work-item */
							tid++;
							lid++;
						}
					}
				}

				/* Next work-group */
				gid++;
			}
		}
	}

	/* Initialize the wavefronts */
	for (wid = 0; wid < ndrange->wavefront_count; wid++)
	{
		/* Assign names to wavefronts */
		wavefront = ndrange->wavefronts[wid];
		snprintf(wavefront->name, sizeof(wavefront->name),
			"wavefront[i%d-i%d]",
			wavefront->work_item_id_first,
			wavefront->work_item_id_last);
	}

	/* Debug */
	si_isa_debug("local_size = %d (%d,%d,%d)\n", ndrange->local_size,
		ndrange->local_size3[0], ndrange->local_size3[1],
		ndrange->local_size3[2]);
	si_isa_debug("global_size = %d (%d,%d,%d)\n", ndrange->global_size,
		ndrange->global_size3[0], ndrange->global_size3[1],
		ndrange->global_size3[2]);
	si_isa_debug("group_count = %d (%d,%d,%d)\n", ndrange->group_count,
		ndrange->group_count3[0], ndrange->group_count3[1],
		ndrange->group_count3[2]);
	si_isa_debug("wavefront_count = %d\n", ndrange->wavefront_count);
	si_isa_debug("wavefronts_per_work_group = %d\n",
		ndrange->wavefronts_per_work_group);
	si_isa_debug("\n");
}


void si_ndrange_setup_size(struct si_ndrange_t *ndrange,
		unsigned int *global_size,
		unsigned int *local_size,
		int work_dim)
{
	int i;

	/* Default value */
	ndrange->global_size3[1] = 1;
	ndrange->global_size3[2] = 1;
	ndrange->local_size3[1] = 1;
	ndrange->local_size3[2] = 1;

	/* Global work sizes */
	for (i = 0; i < work_dim; i++)
		ndrange->global_size3[i] = global_size[i];
	ndrange->global_size = ndrange->global_size3[0] *
			ndrange->global_size3[1] * ndrange->global_size3[2];

	/* Local work sizes */
	for (i = 0; i < work_dim; i++)
	{
		ndrange->local_size3[i] = local_size[i];
		if (ndrange->local_size3[i] < 1)
			fatal("%s: local work size must be greater than 0",
					__FUNCTION__);
	}
	ndrange->local_size = ndrange->local_size3[0] * ndrange->local_size3[1] * ndrange->local_size3[2];

	/* Check valid global/local sizes */
	if (ndrange->global_size3[0] < 1 || ndrange->global_size3[1] < 1
			|| ndrange->global_size3[2] < 1)
		fatal("%s: invalid global size", __FUNCTION__);
	if (ndrange->local_size3[0] < 1 || ndrange->local_size3[1] < 1
			|| ndrange->local_size3[2] < 1)
		fatal("%s: invalid local size", __FUNCTION__);

	/* Check divisibility of global by local sizes */
	if ((ndrange->global_size3[0] % ndrange->local_size3[0])
			|| (ndrange->global_size3[1] % ndrange->local_size3[1])
			|| (ndrange->global_size3[2] % ndrange->local_size3[2]))
		fatal("%s: global work sizes must be multiples of local sizes",
				__FUNCTION__);

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
		ndrange->group_count3[i] = ndrange->global_size3[i] / ndrange->local_size3[i];
	ndrange->group_count = ndrange->group_count3[0] * ndrange->group_count3[1] * ndrange->group_count3[2];

	/* Allocate work-group, wavefront, and work-item arrays */
	si_ndrange_setup_arrays(ndrange);
}


void si_ndrange_setup_inst_mem(struct si_ndrange_t *ndrange, void *buf, 
	int size, unsigned int pc)
{
	struct si_wavefront_t *wavefront;
	int wid;

	/* Sanity */
	if (ndrange->inst_buffer || ndrange->inst_buffer_size)
		panic("%s: instruction buffer already set up", __FUNCTION__);
	if (!size || pc >= size)
		panic("%s: invalid value for size/pc", __FUNCTION__);

	/* Allocate memory buffer */
	assert(size);
	ndrange->inst_buffer = xmalloc(size);
	ndrange->inst_buffer_size = size;
	memcpy(ndrange->inst_buffer, buf, size);

	/* Initialize PC for all wavefronts */
	/* Initialize the wavefronts */
	for (wid = 0; wid < ndrange->wavefront_count; wid++)
	{
		wavefront = ndrange->wavefronts[wid];
		wavefront->pc = pc;
	}
}


void si_ndrange_dump(struct si_ndrange_t *ndrange, FILE *f)
{
	struct si_work_group_t *work_group;
	int work_group_id;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d] ]\n\n", ndrange->id);
	fprintf(f, "Name = %s\n", ndrange->name);
	fprintf(f, "WorkGroupFirst = %d\n", ndrange->work_group_id_first);
	fprintf(f, "WorkGroupLast = %d\n", ndrange->work_group_id_last);
	fprintf(f, "WorkGroupCount = %d\n", ndrange->work_group_count);
	fprintf(f, "WaveFrontFirst = %d\n", ndrange->wavefront_id_first);
	fprintf(f, "WaveFrontLast = %d\n", ndrange->wavefront_id_last);
	fprintf(f, "WaveFrontCount = %d\n", ndrange->wavefront_count);
	fprintf(f, "WorkItemFirst = %d\n", ndrange->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", ndrange->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", ndrange->work_item_count);

	/* Work-groups */
	SI_FOREACH_WORK_GROUP_IN_NDRANGE(ndrange, work_group_id)
	{
		work_group = ndrange->work_groups[work_group_id];
		si_work_group_dump(work_group, f);
	}
}


void si_ndrange_set_free_notify_func(struct si_ndrange_t *ndrange,
		void (*func)(void *), void *user_data)
{
	ndrange->free_notify_func = func;
	ndrange->free_notify_data = user_data;
}


int si_ndrange_get_status(struct si_ndrange_t *ndrange, 
	enum si_ndrange_status_t status)
{
	return (ndrange->status & status) > 0;
}


void si_ndrange_set_status(struct si_ndrange_t *ndrange, 
	enum si_ndrange_status_t status)
{
	/* Get only the new bits */
	status &= ~ndrange->status;

	/* Add ND-Range to lists */
	if (status & si_ndrange_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, pending_ndrange, 
			ndrange);
	if (status & si_ndrange_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, running_ndrange, 
			ndrange);
	if (status & si_ndrange_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, finished_ndrange, 
			ndrange);

	/* Start/stop Southern Islands timer depending on ND-Range states */
	if (si_emu->running_ndrange_list_count)
		m2s_timer_start(si_emu->timer);
	else
		m2s_timer_stop(si_emu->timer);

	/* Update it */
	ndrange->status |= status;
}


void si_ndrange_clear_status(struct si_ndrange_t *ndrange, 
	enum si_ndrange_status_t status)
{
	/* Get only the bits that are set */
	status &= ndrange->status;

	/* Remove ND-Range from lists */
	if (status & si_ndrange_pending)
		DOUBLE_LINKED_LIST_REMOVE(si_emu, pending_ndrange, ndrange);
	if (status & si_ndrange_running)
		DOUBLE_LINKED_LIST_REMOVE(si_emu, running_ndrange, ndrange);
	if (status & si_ndrange_finished)
		DOUBLE_LINKED_LIST_REMOVE(si_emu, finished_ndrange, ndrange);

	/* Update status */
	ndrange->status &= ~status;
}


void si_ndrange_insert_buffer_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int uav)
{
        assert(uav < SI_EMU_MAX_NUM_UAVS);
        assert(sizeof(*buf_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

        /* Write the buffer resource descriptor into the UAV table */
        mem_write(si_emu->global_mem, ndrange->uav_table +
                uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*buf_desc),
                buf_desc);

        ndrange->uav_table_entries[uav].valid = 1;
        ndrange->uav_table_entries[uav].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
        ndrange->uav_table_entries[uav].size = sizeof(*buf_desc);
}

void si_ndrange_insert_buffer_into_const_buf_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int const_buf_num)
{
        assert(const_buf_num < SI_EMU_MAX_NUM_CONST_BUFS);
        assert(sizeof(*buf_desc) <= SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE);

        /* Write the buffer resource descriptor into the UAV table */
        mem_write(si_emu->global_mem, ndrange->const_buf_table +
                const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(*buf_desc), buf_desc);

        ndrange->const_buf_table_entries[const_buf_num].valid = 1;
        ndrange->const_buf_table_entries[const_buf_num].kind = 
		SI_TABLE_ENTRY_KIND_BUFFER_DESC;
        ndrange->uav_table_entries[const_buf_num].size = sizeof(*buf_desc);
}

void si_ndrange_insert_image_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_image_desc_t *image_desc, unsigned int uav)
{
        assert(uav < SI_EMU_MAX_NUM_UAVS);
        assert(sizeof(*image_desc) <= SI_EMU_UAV_TABLE_ENTRY_SIZE);

        /* Write the buffer resource descriptor into the UAV table */
        mem_write(si_emu->global_mem, ndrange->uav_table +
                uav*SI_EMU_UAV_TABLE_ENTRY_SIZE, sizeof(*image_desc),
                image_desc);

        ndrange->uav_table_entries[uav].valid = 1;
        ndrange->uav_table_entries[uav].kind = SI_TABLE_ENTRY_KIND_IMAGE_DESC;
        ndrange->uav_table_entries[uav].size = sizeof(*image_desc);
}

void si_ndrange_const_buf_write(struct si_ndrange_t *ndrange, 
	int const_buf_num, int offset, void *pvalue, unsigned int size)
{
        unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

        /* Sanity check */
        assert(const_buf_num < 2);
        if (const_buf_num == 0)
        {
                assert(offset + size < SI_EMU_CONST_BUF_0_SIZE);
        }
        else if (const_buf_num == 1)
        {
                assert(offset + size < SI_EMU_CONST_BUF_1_SIZE);
        }

	mem_read(si_emu->global_mem, ndrange->const_buf_table + 
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

        addr = (unsigned int)buffer_desc.base_addr;
        addr += offset;

        /* Write */
        mem_write(si_emu->global_mem, addr, size, pvalue);
}

void si_ndrange_const_buf_read(struct si_ndrange_t *ndrange, int const_buf_num, 	int offset, void *pvalue, unsigned int size)
{
        unsigned int addr;

	struct si_buffer_desc_t buffer_desc;

        /* Sanity check */
        assert(const_buf_num < 2);
        if (const_buf_num == 0)
        {
                assert(offset + size < SI_EMU_CONST_BUF_0_SIZE);
        }
        else if (const_buf_num == 1)
        {
                assert(offset + size < SI_EMU_CONST_BUF_1_SIZE);
        }

	mem_read(si_emu->global_mem, ndrange->const_buf_table + 
		const_buf_num*SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE, 
		sizeof(buffer_desc), &buffer_desc);

        addr = (unsigned int)buffer_desc.base_addr;
        addr += offset;

        /* Read */
        mem_read(si_emu->global_mem, addr, size, pvalue);
}
