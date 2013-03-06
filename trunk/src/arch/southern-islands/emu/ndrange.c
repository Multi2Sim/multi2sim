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

#include <arch/southern-islands/asm/bin-file.h>
#include <arch/southern-islands/emu/work-item.h>
#include <driver/opencl-old/southern-islands/command-queue.h>
#include <driver/opencl-old/southern-islands/event.h>
#include <driver/opencl-old/southern-islands/kernel.h>
#include <driver/opencl-old/southern-islands/mem.h>
#include <driver/opencl-old/southern-islands/repo.h>
#include <driver/opencl-old/southern-islands/opencl.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"

/*
 * Private
 */

static void si_ndrange_debug_array(int nelem, int *array)
{
	char *comma = "";
	int i;

	si_opencl_debug("{");
	for (i = 0; i < nelem; i++)
	{
		si_opencl_debug("%s%d", comma, array[i]);
		comma = ", ";
	}
	si_opencl_debug("}");
}




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

	/* Set event status to complete if an event was set. */
	if (ndrange->event)
		ndrange->event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;

	/* Clear task from command queue */
	if (ndrange->command_queue && ndrange->command)
	{
		si_opencl_command_queue_complete(ndrange->command_queue, 
			ndrange->command);
		si_opencl_command_free(ndrange->command);
	}

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

	/* Debug */
	si_opencl_debug("\tglobal_work_size=");
	si_ndrange_debug_array(work_dim, ndrange->global_size3);
	si_opencl_debug("\n");

	/* Local work sizes */
	for (i = 0; i < work_dim; i++)
	{
		ndrange->local_size3[i] = local_size[i];
		if (ndrange->local_size3[i] < 1)
			fatal("%s: local work size must be greater than 0.\n%s",
					__FUNCTION__, si_err_opencl_param_note);
	}
	ndrange->local_size = ndrange->local_size3[0] * ndrange->local_size3[1] * ndrange->local_size3[2];

	/* Debug */
	si_opencl_debug("\tlocal_work_size=");
	si_ndrange_debug_array(work_dim, ndrange->local_size3);
	si_opencl_debug("\n");

	/* Check valid global/local sizes */
	if (ndrange->global_size3[0] < 1 || ndrange->global_size3[1] < 1
			|| ndrange->global_size3[2] < 1)
		fatal("%s: invalid global size.\n%s", __FUNCTION__, si_err_opencl_param_note);
	if (ndrange->local_size3[0] < 1 || ndrange->local_size3[1] < 1
			|| ndrange->local_size3[2] < 1)
		fatal("%s: invalid local size.\n%s", __FUNCTION__, si_err_opencl_param_note);

	/* Check divisibility of global by local sizes */
	if ((ndrange->global_size3[0] % ndrange->local_size3[0])
			|| (ndrange->global_size3[1] % ndrange->local_size3[1])
			|| (ndrange->global_size3[2] % ndrange->local_size3[2]))
		fatal("%s: global work sizes must be multiples of local sizes.\n%s",
				__FUNCTION__, si_err_opencl_param_note);

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
		ndrange->group_count3[i] = ndrange->global_size3[i] / ndrange->local_size3[i];
	ndrange->group_count = ndrange->group_count3[0] * ndrange->group_count3[1] * ndrange->group_count3[2];

	/* Debug */
	si_opencl_debug("\tgroup_count=");
	si_ndrange_debug_array(work_dim, ndrange->group_count3);
	si_opencl_debug("\n");

	/* Allocate work-group, wavefront, and work-item arrays */
	si_ndrange_setup_arrays(ndrange);
}


void si_ndrange_setup_kernel(struct si_ndrange_t *ndrange, struct si_opencl_kernel_t *kernel)
{
	/* Initialize */
	ndrange->kernel = kernel;
}


void si_ndrange_setup_opencl_state(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;
	struct si_wavefront_t *wavefront;
	struct si_work_item_t *work_item;

	int work_item_id;
	int wavefront_id;
	int i;

	unsigned int user_sgpr;
	unsigned int user_element_count;
	struct si_bin_enc_user_element_t *user_elements;

	/* Save local IDs in registers */
	SI_FOREACH_WORK_ITEM_IN_NDRANGE(ndrange, work_item_id)
	{
		work_item = ndrange->work_items[work_item_id];
		work_item->vreg[0].as_int = work_item->id_in_work_group_3d[0];  /* V0 */
		work_item->vreg[1].as_int = work_item->id_in_work_group_3d[1];  /* V1 */
		work_item->vreg[2].as_int = work_item->id_in_work_group_3d[2];  /* V2 */
	}

	/* Initialize the wavefronts */
	SI_FOREACH_WAVEFRONT_IN_NDRANGE(ndrange, wavefront_id)
	{
		/* Get wavefront */
		wavefront = ndrange->wavefronts[wavefront_id];

		/* Save work-group IDs in registers */
		user_sgpr = kernel->bin_file->
			enc_dict_entry_southern_islands->
			compute_pgm_rsrc2->user_sgpr;
		wavefront->sreg[user_sgpr].as_int = 
			wavefront->work_group->id_3d[0];
		wavefront->sreg[user_sgpr + 1].as_int = 
			wavefront->work_group->id_3d[1];
		wavefront->sreg[user_sgpr + 2].as_int = 
			wavefront->work_group->id_3d[2];

		/* Initialize sreg pointers to internal data structures */
		user_element_count = kernel->bin_file->enc_dict_entry_southern_islands->
			userElementCount;
		user_elements = kernel->bin_file->enc_dict_entry_southern_islands->
			userElements;
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

		/* Initialize the execution mask */
		wavefront->sreg[SI_EXEC].as_int = 0xFFFFFFFF;
		wavefront->sreg[SI_EXEC + 1].as_int = 0xFFFFFFFF;
		wavefront->sreg[SI_EXECZ].as_int = 0;
	}
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


void si_ndrange_setup_const_mem(struct si_ndrange_t *ndrange)
{
	unsigned int zero = 0;
	float f;
	
	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	si_isa_const_mem_write(0, 0, &ndrange->global_size3[0]);
	si_isa_const_mem_write(0, 4, &ndrange->global_size3[1]);
	si_isa_const_mem_write(0, 8, &ndrange->global_size3[2]);

	/* Number of work dimensions */
	si_isa_const_mem_write(0, 12, &ndrange->work_dim);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	si_isa_const_mem_write(0, 16, &ndrange->local_size3[0]);
	si_isa_const_mem_write(0, 20, &ndrange->local_size3[1]);
	si_isa_const_mem_write(0, 24, &ndrange->local_size3[2]);

	/* 0  */
	si_isa_const_mem_write(0, 28, &zero);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	si_isa_const_mem_write(0, 32, &ndrange->group_count3[0]);
	si_isa_const_mem_write(0, 36, &ndrange->group_count3[1]);
	si_isa_const_mem_write(0, 40, &ndrange->group_count3[2]);

	/* 0  */
	si_isa_const_mem_write(0, 44, &zero);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is 
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	si_isa_const_mem_write(0, 56, &zero);

	/* 0  */
	si_isa_const_mem_write(0, 60, &zero);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is 
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	si_isa_const_mem_write(0, 72, &zero);

	/* FIXME Pointer to location in global buffer where math library 
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	si_isa_const_mem_write(0, 80, &f);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	si_isa_const_mem_write(0, 84, &f);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	si_isa_const_mem_write(0, 88, &f);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	si_isa_const_mem_write(0, 92, &f);
	
	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	si_isa_const_mem_write(0, 96, &zero);
	si_isa_const_mem_write(0, 100, &zero);
	si_isa_const_mem_write(0, 104, &zero);

	/* Global single dimension flat offset: x * y * z */
	si_isa_const_mem_write(0, 108, &zero);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	si_isa_const_mem_write(0, 112, &zero);
	si_isa_const_mem_write(0, 116, &zero);
	si_isa_const_mem_write(0, 120, &zero);

	/* Group single dimension flat offset, x * y * z */
	si_isa_const_mem_write(0, 124, &zero);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
}

void si_ndrange_setup_args(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;
	struct si_opencl_kernel_arg_t *arg;
	struct si_opencl_mem_t *mem_obj;

	int i;

	/* Initial top of local memory is determined by the static local memory
	 * specified in the kernel binary. */
	ndrange->local_mem_top = kernel->mem_size_local;

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant 
			 * memory */
			assert(arg->size);
			si_isa_const_mem_write_size(
				arg->value.constant_buffer_num, 
				arg->value.constant_offset,
				arg->value.value, 
				arg->size);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			if (arg->pointer.mem_type == 
				SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign
				 * space for it. */
				si_isa_const_mem_write(
					arg->pointer.constant_buffer_num,
					arg->pointer.constant_offset, 
					&ndrange->local_mem_top);

				si_opencl_debug("    arg %d: %d bytes reserved"
					" in local memory at 0x%x\n", i, 
					arg->size, ndrange->local_mem_top);

				ndrange->local_mem_top += arg->size;
			}
			else
			{
				/* XXX Need to figure out what value goes in
				 * CB1 and what value goes in 
				 * buf_desc.base_addr. For now, putting UAV
				 * offset in CB1 and setting base_addr to 0 */
				mem_obj = si_opencl_repo_get_object(
						si_emu->opencl_repo,
						si_opencl_object_mem, 
						arg->pointer.mem_obj_id);
				si_isa_const_mem_write(
					arg->pointer.constant_buffer_num, 
					arg->pointer.constant_offset, 
					&mem_obj->device_ptr);
			}
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_IMAGE:
		{
			assert(0);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_SAMPLER:
		{
			assert(0);
			break;
		}

		default:
		{
			fatal("%s: argument type not reconized", 
				__FUNCTION__);
		}

		}
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

void si_ndrange_dump_initialized_state(struct si_ndrange_t *ndrange)
{
	int i;
	struct si_buffer_desc_t buf_desc;
	struct si_opencl_kernel_t *kernel;
	struct si_opencl_kernel_arg_t *arg;

	kernel = ndrange->kernel;

	si_isa_debug("\n");
	si_isa_debug("================ Initialization Summary ================"
		"\n");
	si_isa_debug("\n");

	/* Dump address ranges */
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\t| Memory Space  |    Start   |     End    |\n");
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\t| UAV table     | %10u | %10u |\n", 
		SI_EMU_UAV_TABLE_START,
		SI_EMU_UAV_TABLE_START+SI_EMU_UAV_TABLE_SIZE-1);
	si_isa_debug("\t| CB table      | %10u | %10u |\n", 
		SI_EMU_CONSTANT_BUFFER_TABLE_START,
		SI_EMU_CONSTANT_BUFFER_TABLE_START+
		SI_EMU_CONSTANT_BUFFER_TABLE_SIZE-1);
	si_isa_debug("\t| RSRC table    | %10u | %10u |\n", 
		SI_EMU_RESOURCE_TABLE_START,
		SI_EMU_RESOURCE_TABLE_START+
		SI_EMU_RESOURCE_TABLE_SIZE-1);
	si_isa_debug("\t| Constant mem  | %10u | %10u |\n", 
		SI_EMU_CONSTANT_MEMORY_START,
		SI_EMU_CONSTANT_MEMORY_START+SI_EMU_CONSTANT_MEMORY_SIZE-1);
	si_isa_debug("\t| Global mem    | %10u | %10u |\n", 
		SI_EMU_GLOBAL_MEMORY_START, 0xFFFFFFFFU);
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\n");

	/* Dump SREG initialization */
	unsigned int userElementCount = 
		kernel->bin_file->enc_dict_entry_southern_islands->
		userElementCount;
	struct si_bin_enc_user_element_t* userElements = 
		kernel->bin_file->enc_dict_entry_southern_islands->
		userElements;
	si_isa_debug("Scalar register initialization prior to execution:\n");
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\t|  Registers  |   Initialization Value    |\n");
	si_isa_debug("\t-------------------------------------------\n");
	for (int i = 0; i < userElementCount; i++)
	{
		if (userElements[i].dataClass == IMM_CONST_BUFFER)
		{

			if (userElements[i].userRegCount > 1)
			{
				/* FIXME Replace CB calculation with value
				 * from CB table once it's implemented */
				si_isa_debug("\t| SREG[%2d:%2d] |  CB%1d "
					"desc   (%10u)  |\n", 
					userElements[i].startUserReg,  
					userElements[i].startUserReg + 
					userElements[i].userRegCount - 1, 
					userElements[i].apiSlot, 
					SI_EMU_CALC_CB_ADDR(
						userElements[i].apiSlot));  
			}
			else
			{
				/* FIXME Replace CB calculation with value
				 * from CB table once it's implemented */
				si_isa_debug("\t| SREG[%2d]    |  CB%1d "
					"desc   (%10u)  |\n", 
					userElements[i].startUserReg,  
					userElements[i].apiSlot, 
					SI_EMU_CALC_CB_ADDR(
						userElements[i].apiSlot));  
			}
		}
		else if (userElements[i].dataClass == IMM_UAV)
		{
			si_isa_debug("\t| SREG[%2d:%2d] |  UAV%-2d "
				"desc (%10u)  |\n", 
				userElements[i].startUserReg,  
				userElements[i].startUserReg + 
				userElements[i].userRegCount - 1, 
				userElements[i].apiSlot, 
				si_emu_get_uav_base_addr(
					userElements[i].apiSlot));
		}
		else if (userElements[i].dataClass == 
			PTR_CONST_BUFFER_TABLE)
		{
			si_isa_debug("\t| SREG[%2d:%2d] |  CONSTANT BUFFER "
				"TABLE    |\n", 
				userElements[i].startUserReg,  
				userElements[i].startUserReg + 
				userElements[i].userRegCount - 1);
		}
		else if (userElements[i].dataClass == PTR_UAV_TABLE)
		{
			si_isa_debug("\t| SREG[%2d:%2d] |  UAV "
				"TABLE                |\n", 
				userElements[i].startUserReg,  
				userElements[i].startUserReg + 
				userElements[i].userRegCount - 1);
		}
		else 
		{
			assert(0);
		}
	}
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\n");

	/* Dump constant buffer 1 (argument mapping) */
	si_isa_debug("Constant buffer 1 initialization (kernel arguments):\n");
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\t| CB1 Idx | Arg # |   Size   |    Name    |\n");
	si_isa_debug("\t-------------------------------------------\n");
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant 
			 * memory */
			assert(arg->size);
			si_isa_debug("\t| CB1[%2d] | %5d | %8d | %-10s |\n", 
				arg->pointer.constant_offset/4, i, arg->size,
				arg->name);

			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			if (arg->pointer.mem_type != 
				SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
			{
				si_isa_debug("\t| CB1[%2d] | %5d | %8d | %-10s"
					" |\n", arg->pointer.constant_offset/4,
					i, arg->size,
					arg->name);
			}
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_IMAGE:
		{
			assert(0);
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_SAMPLER:
		{
			assert(0);
			break;
		}

		default:
		{
			fatal("%s: argument type not reconized", 
				__FUNCTION__);
		}

		}
	}	
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\n");

	/* Dump constant buffers */
	si_isa_debug("Constant buffer mappings into global memory:\n");
	si_isa_debug("\t--------------------------------------\n");
	si_isa_debug("\t|  CB  |        Address Range        |\n");
	si_isa_debug("\t--------------------------------------\n");
	for (i = 0; i < si_emu_num_mapped_const_buffers; i++)
	{
		si_isa_debug("\t| CB%-2d |   [%10u:%10u]   |\n", i,
			SI_EMU_CALC_CB_ADDR(i),
			SI_EMU_CALC_CB_ADDR(i)+SI_EMU_CONSTANT_BUFFER_SIZE-1);
	}
	si_isa_debug("\t--------------------------------------\n");
	si_isa_debug("\n");

	/* Dump UAVs */
	si_isa_debug("Initialized UAVs:\n");
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\t|  UAV  |    Address   |    Arg Name      |\n");
	si_isa_debug("\t-------------------------------------------\n");
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
		{
			fatal("kernel '%s': argument '%s' has not been "
				"assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);
		}

		/* Process argument depending on its type */
		if (arg->kind == SI_OPENCL_KERNEL_ARG_KIND_POINTER &&
			arg->pointer.mem_type != 
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
		{
			buf_desc = si_emu_get_uav_table_entry(
				arg->pointer.buffer_num);

			si_isa_debug("\t| UAV%-2d | %10u   |    %-12s  |\n",
				arg->pointer.buffer_num, 
				(unsigned int)buf_desc.base_addr,
				arg->name);
		}
	}	
	si_isa_debug("\t-------------------------------------------\n");
	si_isa_debug("\n");
	si_isa_debug("========================================================"
		"\n");
}
