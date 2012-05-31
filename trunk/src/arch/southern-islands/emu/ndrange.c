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

	/* Free scalar work-item */
	si_work_item_free(ndrange->scalar_work_item);

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
	}

	/* Initialize the scalar work item */
	ndrange->scalar_work_item = si_work_item_create();
	ndrange->scalar_work_item->ndrange = ndrange;
	ndrange->scalar_work_item->id = -1;
	
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
							bit_map_set(wavefront->active_stack, work_item->id_in_wavefront, 1, 1);

/* FIXME Update for SI */
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

	/* Assign names to wavefronts */
	for (wid = 0; wid < ndrange->wavefront_count; wid++)
	{
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
		si_isa_debug("%4d %4d %4d %4d  ", work_item->id_in_work_group, work_item->id_in_work_group_3d[2],
			work_item->id_in_work_group_3d[1], work_item->id_in_work_group_3d[0]);
		si_isa_debug("%20s.%-4d  ", wavefront->name, work_item->id_in_wavefront);
		si_isa_debug("%20s.%-4d\n", work_group->name, work_item->id_in_work_group);
	}

}


/* FIXME: constant memory should be member of 'si_emu' or 'ndrange'? */
void si_ndrange_setup_const_mem(struct si_ndrange_t *ndrange)
{

}


void si_ndrange_setup_args(struct si_ndrange_t *ndrange)
{
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
			printf("work group %d\n", work_group->id); 
			/* Save next running work-group */
			work_group_next = work_group->running_list_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; wavefront; wavefront = wavefront_next)
			{
				printf("wavefront id %d\n", wavefront->id); 
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
	int work_item_id, last_work_item_id;
	uint32_t branch_digest, last_branch_digest;
	int branch_digest_count;

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

	/* Branch digests */
	assert(ndrange->work_item_count);
	branch_digest_count = 0;
	last_work_item_id = 0;
	last_branch_digest = ndrange->work_items[0]->branch_digest;
	for (work_item_id = 1; work_item_id <= ndrange->work_item_count; work_item_id++)
	{
		branch_digest = work_item_id < ndrange->work_item_count ? ndrange->work_items[work_item_id]->branch_digest : 0;
		if (work_item_id == ndrange->work_item_count || branch_digest != last_branch_digest)
		{
			fprintf(f, "BranchDigest[%d] = %d %d %08x\n", branch_digest_count,
				last_work_item_id, work_item_id - 1, last_branch_digest);
			last_work_item_id = work_item_id;
			last_branch_digest = branch_digest;
			branch_digest_count++;
		}
	}
	fprintf(f, "BranchDigestCount = %d\n", branch_digest_count);
	fprintf(f, "\n");

	/* Work-groups */
	SI_FOR_EACH_WORK_GROUP_IN_NDRANGE(ndrange, work_group_id)
	{
		work_group = ndrange->work_groups[work_group_id];
		si_work_group_dump(work_group, f);
	}
}
