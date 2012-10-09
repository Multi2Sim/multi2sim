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

#include <arch/x86/emu/emu.h>

#include "southern-islands-emu.h"


/*
 * GPU ND-Range
 */

struct si_ndrange_t *si_ndrange_create(struct si_opencl_kernel_t *kernel)
{
	struct si_ndrange_t *ndrange;

	/* Allocate */
	ndrange = calloc(1, sizeof(struct si_ndrange_t));
	if (!ndrange)
		fatal("%s: out of memory", __FUNCTION__);

	/* Insert in ND-Range list of SouthernIslands emulator */
	DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, ndrange, ndrange);

	/* Name */
	ndrange->name = strdup(kernel->name);
	if (!ndrange->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	ndrange->kernel = kernel;
	ndrange->local_mem_top = kernel->func_mem_local;
	ndrange->id = si_emu->ndrange_count++;

	/* Create the UAV-to-physical-address lookup lists */
	ndrange->uav_list = list_create();

	/* Instruction histogram */
	if (evg_emu_report_file)
	{
		ndrange->inst_histogram = calloc(EVG_INST_COUNT, sizeof(unsigned int));
		if (!ndrange->inst_histogram)
			fatal("%s: out of memory", __FUNCTION__);
	}

	/* Return */
	return ndrange;
}


void si_ndrange_free(struct si_ndrange_t *ndrange)
{
	/* Set event status to complete if an event was set. */
	if(ndrange->event)
		ndrange->event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;

	int i;

	/* Clear task from command queue */
	if (ndrange->command_queue && ndrange->command)
	{
		si_opencl_command_queue_complete(ndrange->command_queue, ndrange->command);
		si_opencl_command_free(ndrange->command);
	}

	/* Clear all states that affect lists. */
	si_ndrange_clear_status(ndrange, si_ndrange_pending);
	si_ndrange_clear_status(ndrange, si_ndrange_running);
	si_ndrange_clear_status(ndrange, si_ndrange_finished);

	/* Extract from ND-Range list in Southern Islands emulator */
	assert(DOUBLE_LINKED_LIST_MEMBER(si_emu, ndrange, ndrange));
	DOUBLE_LINKED_LIST_REMOVE(si_emu, ndrange, ndrange);

	/* Free lists */
	list_free(ndrange->uav_list);

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

	/* Free ndrange */
	free(ndrange->name);
	free(ndrange);
}


void si_ndrange_setup_work_items(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;

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
	ndrange->work_group_count = kernel->group_count;
	ndrange->work_group_id_first = 0;
	ndrange->work_group_id_last = ndrange->work_group_count - 1;
	ndrange->work_groups = calloc(ndrange->work_group_count, sizeof(void *));
	for (gid = 0; gid < kernel->group_count; gid++)
	{
		ndrange->work_groups[gid] = si_work_group_create();
		work_group = ndrange->work_groups[gid];
	}

	/* Array of wavefronts */
	ndrange->wavefronts_per_work_group = (kernel->local_size + si_emu_wavefront_size - 1) / si_emu_wavefront_size;
	ndrange->wavefront_count = ndrange->wavefronts_per_work_group * ndrange->work_group_count;
	ndrange->wavefront_id_first = 0;
	ndrange->wavefront_id_last = ndrange->wavefront_count - 1;
	assert(ndrange->wavefronts_per_work_group > 0 && ndrange->wavefront_count > 0);
	ndrange->wavefronts = calloc(ndrange->wavefront_count, sizeof(void *));
	ndrange->scalar_work_items = calloc(ndrange->wavefront_count, sizeof(void *));
	for (wid = 0; wid < ndrange->wavefront_count; wid++)
	{
		gid = wid / ndrange->wavefronts_per_work_group;
		ndrange->wavefronts[wid] = si_wavefront_create();
		wavefront = ndrange->wavefronts[wid];
		work_group = ndrange->work_groups[gid];

		wavefront->id = wid;
		wavefront->id_in_work_group = wid % ndrange->wavefronts_per_work_group;
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
	ndrange->work_item_count = kernel->global_size;
	ndrange->work_item_id_first = 0;
	ndrange->work_item_id_last = ndrange->work_item_count - 1;
	ndrange->work_items = calloc(ndrange->work_item_count, sizeof(void *));
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < kernel->group_count3[2]; gidz++)
	{
		for (gidy = 0; gidy < kernel->group_count3[1]; gidy++)
		{
			for (gidx = 0; gidx < kernel->group_count3[0]; gidx++)
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
				work_group->work_item_id_last = tid + kernel->local_size;
				work_group->work_item_count = kernel->local_size;
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
				for (lidz = 0; lidz < kernel->local_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < kernel->local_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < kernel->local_size3[0]; lidx++)
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
							work_item->id_3d[0] = gidx * kernel->local_size3[0] + lidx;
							work_item->id_3d[1] = gidy * kernel->local_size3[1] + lidy;
							work_item->id_3d[2] = gidz * kernel->local_size3[2] + lidz;
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
							if (!wavefront->work_item_count) {
								wavefront->work_item_id_first = tid;
								wavefront->work_items = &ndrange->work_items[tid];
							}
							wavefront->work_item_count++;
							wavefront->work_item_id_last = tid;

							/* Save local IDs in registers */
							work_item->vreg[0].as_int = lidx;  /* V0 */
							work_item->vreg[1].as_int = lidy;  /* V1 */
							work_item->vreg[2].as_int = lidz;  /* V2 */

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
		snprintf(wavefront->name, sizeof(wavefront->name), "wavefront[i%d-i%d]",
			wavefront->work_item_id_first, wavefront->work_item_id_last);

		/* Initialize wavefront program counter */
		if (!kernel->bin_file->enc_dict_entry_southern_islands->sec_text_buffer.size)
			fatal("%s: cannot load kernel code", __FUNCTION__);
		wavefront->inst_buf_start = kernel->bin_file->enc_dict_entry_southern_islands->sec_text_buffer.ptr;
		wavefront->inst_buf = kernel->bin_file->enc_dict_entry_southern_islands->sec_text_buffer.ptr;

		/* Save work-group IDs in registers */
		unsigned int user_sgpr = kernel->bin_file->enc_dict_entry_southern_islands->compute_pgm_rsrc2->user_sgpr;
		wavefront->sreg[user_sgpr].as_int = wavefront->work_group->id_3d[0];
		wavefront->sreg[user_sgpr + 1].as_int = wavefront->work_group->id_3d[1];
		wavefront->sreg[user_sgpr + 2].as_int = wavefront->work_group->id_3d[2];

		/* Initialize Constant Buffers */
		unsigned int userElementCount = kernel->bin_file->enc_dict_entry_southern_islands->userElementCount;
		struct si_bin_enc_user_element_t* userElements = kernel->bin_file->enc_dict_entry_southern_islands->userElements;
		for (int i = 0; i < userElementCount; i++)
		{
			if (userElements[i].dataClass == IMM_CONST_BUFFER)
			{
				si_wavefront_init_sreg_with_cb(wavefront, userElements[i].startUserReg, userElements[i].userRegCount, userElements[i].apiSlot);
			}
			else if (userElements[i].dataClass == IMM_UAV)
			{
				si_wavefront_init_sreg_with_cb(wavefront, userElements[i].startUserReg, userElements[i].userRegCount, userElements[i].apiSlot);
			}
			else if (userElements[i].dataClass == PTR_CONST_BUFFER_TABLE)
			{
				si_wavefront_init_sreg_with_uav_table(wavefront, userElements[i].startUserReg, userElements[i].userRegCount);
			}
			else if (userElements[i].dataClass == PTR_UAV_TABLE)
			{
				si_wavefront_init_sreg_with_uav_table(wavefront, userElements[i].startUserReg, userElements[i].userRegCount);
			}
			else
			{
				fatal("Unimplemented User Element: dataClass:%d", userElements[i].dataClass);
			}
		}

		/* Initialize the execution mask */
		wavefront->sreg[SI_EXEC].as_int = 0xFFFFFFFF;
		wavefront->sreg[SI_EXEC + 1].as_int = 0xFFFFFFFF;
		wavefront->sreg[SI_EXECZ].as_int = 0;
	}

	/* Debug */
	si_isa_debug("local_size = %d (%d,%d,%d)\n", kernel->local_size, kernel->local_size3[0],
		kernel->local_size3[1], kernel->local_size3[2]);
	si_isa_debug("global_size = %d (%d,%d,%d)\n", kernel->global_size, kernel->global_size3[0],
		kernel->global_size3[1], kernel->global_size3[2]);
	si_isa_debug("group_count = %d (%d,%d,%d)\n", kernel->group_count, kernel->group_count3[0],
		kernel->group_count3[1], kernel->group_count3[2]);
	si_isa_debug("wavefront_count = %d\n", ndrange->wavefront_count);
	si_isa_debug("wavefronts_per_work_group = %d\n", ndrange->wavefronts_per_work_group);
	si_isa_debug(" tid tid2 tid1 tid0   gid gid2 gid1 gid0   lid lid2 lid1 lid0  wavefront            work-group\n");
	for (tid = 0; tid < ndrange->work_item_count; tid++)
	{
		work_item = ndrange->work_items[tid];
		wavefront = work_item->wavefront;
		work_group = work_item->work_group;
		si_isa_debug("%4d %4d %4d %4d  ", work_item->id, work_item->id_3d[2],
			work_item->id_3d[1], work_item->id_3d[0]);
		si_isa_debug("%4d %4d %4d %4d  ", work_group->id, work_group->id_3d[2],
			work_group->id_3d[1], work_group->id_3d[0]);
		si_isa_debug("%4d %4d %4d %4d  ", work_item->id_in_work_group, 
			work_item->id_in_work_group_3d[2], work_item->id_in_work_group_3d[1], 
			work_item->id_in_work_group_3d[0]);
		si_isa_debug("%20s.%-4d  ", wavefront->name, work_item->id_in_wavefront);
		si_isa_debug("%20s.%-4d\n", work_group->name, work_item->id_in_work_group);
	}

}



void si_ndrange_setup_const_mem(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;
	uint32_t zero = 0;
	float f;
	
       	/* CB0 bytes 0:15 */
        
	/* Global work size for the {x,y,z} dimensions */
	si_isa_const_mem_write(0, 0, &kernel->global_size3[0]);
	si_isa_const_mem_write(0, 4, &kernel->global_size3[1]);
	si_isa_const_mem_write(0, 8, &kernel->global_size3[2]);

        /* Number of work dimensions */
	si_isa_const_mem_write(0, 12, &kernel->work_dim);

       	/* CB0 bytes 16:31 */

        /* Local work size for the {x,y,z} dimensions */
	si_isa_const_mem_write(0, 16, &kernel->local_size3[0]);
	si_isa_const_mem_write(0, 20, &kernel->local_size3[1]);
	si_isa_const_mem_write(0, 24, &kernel->local_size3[2]);

	/* 0  */
	si_isa_const_mem_write(0, 28, &zero);

       	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	si_isa_const_mem_write(0, 32, &kernel->group_count3[0]);
	si_isa_const_mem_write(0, 36, &kernel->group_count3[1]);
	si_isa_const_mem_write(0, 40, &kernel->group_count3[2]);

	/* 0  */
	si_isa_const_mem_write(0, 44, &zero);

       	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	si_isa_const_mem_write(0, 56, &zero);

	/* 0  */
	si_isa_const_mem_write(0, 60, &zero);

       	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	si_isa_const_mem_write(0, 72, &zero);

	/* FIXME Pointer to location in global buffer where math library tables start. */
        	
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

void si_ndrange_init_uav_table(struct si_ndrange_t *ndrange)
{
	int i;
	uint32_t buffer_addr;
	struct si_opencl_mem_t *mem_obj;
	struct si_buffer_resource_t buf_desc;

	/* Zero-out the buffer resource descriptor */
	memset(&buf_desc, 0, 16);

	for (i = 0; i < list_count(ndrange->uav_list); i++)
	{
		   /* Get the memory object for the buffer */
		   mem_obj = list_get(ndrange->uav_list, i);

		   /* Get the address of the buffer in global memory */
		   buffer_addr = mem_obj->device_ptr;

		   /* Initialize the buffer resource descriptor for this UAV */
		   buf_desc.base_addr = buffer_addr;

		   assert(UAV_TABLE_START + 320 + i*32 <= UAV_TABLE_START + UAV_TABLE_SIZE - 32);

		   /* Write the buffer resource descriptor into the UAV table at offset 320
			* with 32 bytes spacing */
		   mem_write(si_emu->global_mem, UAV_TABLE_START + 320 + i*32, 16, &buf_desc);
	}
}

void si_ndrange_setup_args(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;
	struct si_opencl_kernel_arg_t *arg;
	int i;
	int j;
	int cb_index = 0;

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
			fatal("kernel '%s': argument '%s' has not been assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);

		/* Process argument depending on its type */
		switch (arg->kind)
		{

		case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant memory */
			for (j = 0; j < arg->size/4; j++)
			{
				si_isa_const_mem_write(1, (cb_index*4)*4+j, &arg->data.value[j]);
				si_opencl_debug("    arg %d: value '0x%x' loaded into "
						"CB1[%d][%d]\n", i, arg->data.value[j], 
						cb_index, j);
			}
			cb_index++;
			break;
		}

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			switch (arg->mem_scope)
			{

			case SI_OPENCL_MEM_SCOPE_CONSTANT:
			case SI_OPENCL_MEM_SCOPE_GLOBAL:
			{
				struct si_opencl_mem_t *mem;

				/* Pointer in __global scope.
				 * Argument value is a pointer to an 'opencl_mem' object.
				 * It is translated first into a device memory pointer. */
				mem = si_opencl_repo_get_object(si_emu->opencl_repo,
					si_opencl_object_mem, arg->data.ptr);
				si_isa_const_mem_write(1, (cb_index*4)*4, &mem->device_ptr);
				si_opencl_debug("    arg %d: opencl_mem id 0x%x loaded into CB1[%d],"
					" device_ptr=0x%x\n", i, arg->data.ptr, cb_index,
					mem->device_ptr);
				cb_index++;
				break;
			}

			case SI_OPENCL_MEM_SCOPE_LOCAL:
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign space for it. */
				si_isa_const_mem_write(1, (cb_index*4)*4, &ndrange->local_mem_top);
				si_opencl_debug("    arg %d: %d bytes reserved in local memory at 0x%x\n",
					i, arg->size, ndrange->local_mem_top);
				ndrange->local_mem_top += arg->size;
				cb_index++;
				break;
			}

			default:
			{
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}

			}

			break;
		}

		default:
		{
			fatal("%s: argument type not reconized", __FUNCTION__);
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
	SI_FOR_EACH_WORK_GROUP_IN_NDRANGE(ndrange, work_group_id)
	{
		work_group = ndrange->work_groups[work_group_id];
		si_work_group_dump(work_group, f);
	}
}

int si_ndrange_get_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status)
{
	return (ndrange->status & status) > 0;
}


void si_ndrange_set_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status)
{
	/* Get only the new bits */
	status &= ~ndrange->status;

	/* Add ND-Range to lists */
	if (status & si_ndrange_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, pending_ndrange, ndrange);
	if (status & si_ndrange_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, running_ndrange, ndrange);
	if (status & si_ndrange_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_emu, finished_ndrange, ndrange);

	/* Start/stop Evergreen timer depending on ND-Range states */
	if (si_emu->running_ndrange_list_count)
		m2s_timer_start(si_emu->timer);
	else
		m2s_timer_stop(si_emu->timer);

	/* Update it */
	ndrange->status |= status;
}


void si_ndrange_clear_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status)
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
