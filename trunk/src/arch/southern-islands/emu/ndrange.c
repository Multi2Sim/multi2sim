/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <southern-islands-emu.h>
#include <x86-emu.h>


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

	/* Name */
	ndrange->name = strdup(kernel->name);
	if (!ndrange->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	ndrange->kernel = kernel;
	ndrange->local_mem_top = kernel->func_mem_local;
	ndrange->id = si_emu->ndrange_count++;

	/* Return */
	return ndrange;
}


void si_ndrange_free(struct si_ndrange_t *ndrange)
{
	int i;

	/* Free work-groups */
	for (i = 0; i < ndrange->work_group_count; i++)
		si_work_group_free(ndrange->work_groups[i]);
	free(ndrange->work_groups);

	/* Free wavefronts */
	for (i = 0; i < ndrange->wavefront_count; i++)
		si_wavefront_free(ndrange->wavefronts[i]);
	free(ndrange->wavefronts);

	/* Free work-items */
	for (i = 0; i < ndrange->work_item_count; i++)
		si_work_item_free(ndrange->work_items[i]);
	free(ndrange->work_items);

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

		/* Initialize SGPRs */
		/* FIXME These values need to be determined from the binary */
		si_wavefront_init_sreg_with_cb(wavefront, 4, 4, 0);
		si_wavefront_init_sreg_with_cb(wavefront, 8, 4, 1);
		si_wavefront_init_sreg_with_uav_table(wavefront, 2, 2);
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

#if 0
							/* Save local IDs in register R0 */
							work_item->gpr[0].elem[0] = lidx;  /* R0.x */
							work_item->gpr[0].elem[1] = lidy;  /* R0.y */
							work_item->gpr[0].elem[2] = lidz;  /* R0.z */

							/* Save work-group IDs in register R1 */
							work_item->gpr[1].elem[0] = gidx;  /* R1.x */
							work_item->gpr[1].elem[1] = gidy;  /* R1.y */
							work_item->gpr[1].elem[2] = gidz;  /* R1.z */
#endif

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
		wavefront->emu_time_start = x86_emu_timer();
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


void si_ndrange_setup_args(struct si_ndrange_t *ndrange)
{
	struct si_opencl_kernel_t *kernel = ndrange->kernel;
	struct si_opencl_kernel_arg_t *arg;
	int i;
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

#if 0
                case SI_OPENCL_KERNEL_ARG_KIND_VALUE:
                {
                        /* Value copied directly into device constant memory */
                        si_isa_const_mem_write(1, cb_index, 0, &arg->value);
                        si_opencl_debug("    arg %d: value '0x%x' loaded into CB1[%d]\n", i,
                                        arg->value, cb_index);
                        cb_index++;
                        break;
                }
#endif

		case SI_OPENCL_KERNEL_ARG_KIND_POINTER:
		{
                        switch (arg->mem_scope)
			{

                        case SI_OPENCL_MEM_SCOPE_GLOBAL:
                        {
                                struct si_opencl_mem_t *mem;
				/* Base address is address of CB1 */

                                /* Pointer in __global scope.
                                 * Argument value is a pointer to an 'opencl_mem' object.
                                 * It is translated first into a device memory pointer. */
                                mem = si_opencl_object_get(SI_OPENCL_OBJ_MEM, arg->value);
                                si_isa_const_mem_write(1, (cb_index*4)*4, &mem->device_ptr);
                                si_opencl_debug("    arg %d: opencl_mem id 0x%x loaded into CB1[%d],"
                                                " device_ptr=0x%x\n", i, arg->value, cb_index,
                                                mem->device_ptr);
                                cb_index++;
                                break;
                        }

                        default:
                                fatal("%s: argument in memory scope %d not supported",
                                        __FUNCTION__, arg->mem_scope);
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


void si_ndrange_run(struct si_ndrange_t *ndrange)
{
	struct si_work_group_t *work_group, *work_group_next;
	struct si_wavefront_t *wavefront, *wavefront_next;
	uint64_t cycle = 0;

	/* Set all ready work-groups to running */
	while ((work_group = ndrange->pending_list_head))
	{
		si_work_group_clear_status(work_group, si_work_group_pending);
		si_work_group_set_status(work_group, si_work_group_running);
	}

	/* Start GPU timer */
	si_emu_timer_start();

	/* Execution loop */
	while (ndrange->running_list_head)
	{
		/* Stop if maximum number of GPU cycles exceeded */
		if (si_emu_max_cycles && cycle >= si_emu_max_cycles)
			x86_emu_finish = x86_emu_finish_max_gpu_cycles;

		/* Stop if maximum number of GPU instructions exceeded */
		if (si_emu_max_inst && si_emu->inst_count >= si_emu_max_inst)
			x86_emu_finish = x86_emu_finish_max_gpu_inst;

		/* Stop if any reason met */
		if (x86_emu_finish)
			break;

		/* Next cycle */
		cycle++;

		/* Execute an instruction from each work-group */
		for (work_group = ndrange->running_list_head; work_group; work_group = work_group_next)
		{
			/* Save next running work-group */
			work_group_next = work_group->running_list_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; wavefront; 
				wavefront = wavefront_next)
			{
				/* Save next running wavefront */
				wavefront_next = wavefront->running_list_next;

				/* Execute instruction in wavefront */
				si_wavefront_execute(wavefront);
			}
		}
	}

	/* Stop GPU timer */
	si_emu_timer_stop();

	/* Dump stats */
	si_ndrange_dump(ndrange, si_emu_report_file);

	/* Stop if maximum number of kernels reached */
	if (si_emu_max_kernels && si_emu->ndrange_count >= si_emu_max_kernels)
		x86_emu_finish = x86_emu_finish_max_gpu_kernels;
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
