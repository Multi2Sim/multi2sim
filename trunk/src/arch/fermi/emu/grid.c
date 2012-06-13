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

#include <fermi-emu.h>
#include <x86-emu.h>


/*
 * CUDA Grid
 */

struct frm_grid_t *frm_grid_create(struct frm_cuda_kernel_t *kernel)
{
	struct frm_grid_t *grid;

	/* Allocate */
	grid = calloc(1, sizeof(struct frm_grid_t));
	if (!grid)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	grid->name = strdup(kernel->name);
	if (!grid->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	grid->kernel = kernel;
	grid->local_mem_top = kernel->func_mem_local;
	grid->id = frm_emu->grid_count++;

	/* Return */
	return grid;
}


void frm_grid_free(struct frm_grid_t *grid)
{
	int i;

	/* Free work-groups */
	for (i = 0; i < grid->threadblock_count; i++)
		frm_threadblock_free(grid->threadblocks[i]);
	free(grid->threadblocks);

	/* Free warps */
	for (i = 0; i < grid->warp_count; i++)
		frm_warp_free(grid->warps[i]);
	free(grid->warps);

	/* Free work-items */
	for (i = 0; i < grid->thread_count; i++)
		frm_thread_free(grid->threads[i]);
	free(grid->threads);

	/* Free grid */
	free(grid->name);
	free(grid);
}


void frm_grid_setup_threads(struct frm_grid_t *grid)
{
	struct frm_cuda_kernel_t *kernel = grid->kernel;

	struct frm_threadblock_t *threadblock;
	struct frm_warp_t *warp;
	struct frm_thread_t *thread;

	int gidx, gidy, gidz;  /* 3D work-group ID iterators */
	int lidx, lidy, lidz;  /* 3D work-item local ID iterators */

	int tid;  /* Global ID iterator */
	int gid;  /* Group ID iterator */
	int wid;  /* Wavefront ID iterator */
	int lid;  /* Local ID iterator */

	/* Array of work-groups */
	grid->threadblock_count = kernel->group_count;
	grid->threadblock_id_first = 0;
	grid->threadblock_id_last = grid->threadblock_count - 1;
	grid->threadblocks = calloc(grid->threadblock_count, sizeof(void *));
	for (gid = 0; gid < kernel->group_count; gid++)
	{
		grid->threadblocks[gid] = frm_threadblock_create();
		threadblock = grid->threadblocks[gid];
	}
	
	/* Array of warps */
	grid->warps_per_threadblock = (kernel->local_size + frm_emu_warp_size - 1) / frm_emu_warp_size;
	grid->warp_count = grid->warps_per_threadblock * grid->threadblock_count;
	grid->warp_id_first = 0;
	grid->warp_id_last = grid->warp_count - 1;
	assert(grid->warps_per_threadblock > 0 && grid->warp_count > 0);
	grid->warps = calloc(grid->warp_count, sizeof(void *));
	for (wid = 0; wid < grid->warp_count; wid++)
	{
		gid = wid / grid->warps_per_threadblock;
		grid->warps[wid] = frm_warp_create();
		warp = grid->warps[wid];
		threadblock = grid->threadblocks[gid];

		warp->id = wid;
		warp->id_in_threadblock = wid % grid->warps_per_threadblock;
		warp->grid = grid;
		warp->threadblock = threadblock;
		DOUBLE_LINKED_LIST_INSERT_TAIL(threadblock, running, warp);
	}
	
	/* Array of work-items */
	grid->thread_count = kernel->global_size;
	grid->thread_id_first = 0;
	grid->thread_id_last = grid->thread_count - 1;
	grid->threads = calloc(grid->thread_count, sizeof(void *));
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < kernel->group_count3[2]; gidz++)
	{
		for (gidy = 0; gidy < kernel->group_count3[1]; gidy++)
		{
			for (gidx = 0; gidx < kernel->group_count3[0]; gidx++)
			{
				/* Assign work-group ID */
				threadblock = grid->threadblocks[gid];
				threadblock->grid = grid;
				threadblock->id_3d[0] = gidx;
				threadblock->id_3d[1] = gidy;
				threadblock->id_3d[2] = gidz;
				threadblock->id = gid;
				frm_threadblock_set_status(threadblock, frm_threadblock_pending);

				/* First, last, and number of work-items in work-group */
				threadblock->thread_id_first = tid;
				threadblock->thread_id_last = tid + kernel->local_size;
				threadblock->thread_count = kernel->local_size;
				threadblock->threads = &grid->threads[tid];
				snprintf(threadblock->name, sizeof(threadblock->name), "work-group[i%d-i%d]",
					threadblock->thread_id_first, threadblock->thread_id_last);

				/* First ,last, and number of warps in work-group */
				threadblock->warp_id_first = gid * grid->warps_per_threadblock;
				threadblock->warp_id_last = threadblock->warp_id_first + grid->warps_per_threadblock - 1;
				threadblock->warp_count = grid->warps_per_threadblock;
				threadblock->warps = &grid->warps[threadblock->warp_id_first];

				/* Iterate through work-items */
				lid = 0;
				for (lidz = 0; lidz < kernel->local_size3[2]; lidz++)
				{
					for (lidy = 0; lidy < kernel->local_size3[1]; lidy++)
					{
						for (lidx = 0; lidx < kernel->local_size3[0]; lidx++)
						{
							/* Wavefront ID */
							wid = gid * grid->warps_per_threadblock +
								lid / frm_emu_warp_size;
							assert(wid < grid->warp_count);
							warp = grid->warps[wid];
							
							/* Create work-item */
							grid->threads[tid] = frm_thread_create();
							thread = grid->threads[tid];
							thread->grid = grid;

							/* Global IDs */
							thread->id_3d[0] = gidx * kernel->local_size3[0] + lidx;
							thread->id_3d[1] = gidy * kernel->local_size3[1] + lidy;
							thread->id_3d[2] = gidz * kernel->local_size3[2] + lidz;
							thread->id = tid;

							/* Local IDs */
							thread->id_in_threadblock_3d[0] = lidx;
							thread->id_in_threadblock_3d[1] = lidy;
							thread->id_in_threadblock_3d[2] = lidz;
							thread->id_in_threadblock = lid;

							/* Other */
							thread->id_in_warp = thread->id_in_threadblock % frm_emu_warp_size;
							thread->threadblock = grid->threadblocks[gid];
							thread->warp = grid->warps[wid];

							/* First, last, and number of work-items in warp */
							if (!warp->thread_count) {
								warp->thread_id_first = tid;
								warp->threads = &grid->threads[tid];
							}
							warp->thread_count++;
							warp->thread_id_last = tid;
							bit_map_set(warp->active_stack, thread->id_in_warp, 1, 1);

							/* Save local IDs in register R0 */
							thread->gpr[0].elem[0] = lidx;  /* R0.x */
							thread->gpr[0].elem[1] = lidy;  /* R0.y */
							thread->gpr[0].elem[2] = lidz;  /* R0.z */

							/* Save work-group IDs in register R1 */
							thread->gpr[1].elem[0] = gidx;  /* R1.x */
							thread->gpr[1].elem[1] = gidy;  /* R1.y */
							thread->gpr[1].elem[2] = gidz;  /* R1.z */

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

	/* Assign names to warps */
	for (wid = 0; wid < grid->warp_count; wid++)
	{
		warp = grid->warps[wid];
		snprintf(warp->name, sizeof(warp->name), "warp[i%d-i%d]",
			warp->thread_id_first, warp->thread_id_last);

		/* Initialize warp program counter */
		if (!kernel->bin_file->enc_dict_entry_evergreen->sec_text_buffer.size)
			fatal("%s: cannot load kernel code", __FUNCTION__);
		warp->cf_buf_start = kernel->bin_file->enc_dict_entry_evergreen->sec_text_buffer.ptr;
		warp->cf_buf = warp->cf_buf_start;
		warp->clause_kind = FRM_CLAUSE_CF;
		warp->emu_time_start = x86_emu_timer();
	}

	/* Debug */
	frm_isa_debug("local_size = %d (%d,%d,%d)\n", kernel->local_size, kernel->local_size3[0],
		kernel->local_size3[1], kernel->local_size3[2]);
	frm_isa_debug("global_size = %d (%d,%d,%d)\n", kernel->global_size, kernel->global_size3[0],
		kernel->global_size3[1], kernel->global_size3[2]);
	frm_isa_debug("group_count = %d (%d,%d,%d)\n", kernel->group_count, kernel->group_count3[0],
		kernel->group_count3[1], kernel->group_count3[2]);
	frm_isa_debug("warp_count = %d\n", grid->warp_count);
	frm_isa_debug("warps_per_threadblock = %d\n", grid->warps_per_threadblock);
	frm_isa_debug(" tid tid2 tid1 tid0   gid gid2 gid1 gid0   lid lid2 lid1 lid0  warp            work-group\n");
	for (tid = 0; tid < grid->thread_count; tid++)
	{
		thread = grid->threads[tid];
		warp = thread->warp;
		threadblock = thread->threadblock;
		frm_isa_debug("%4d %4d %4d %4d  ", thread->id, thread->id_3d[2],
			thread->id_3d[1], thread->id_3d[0]);
		frm_isa_debug("%4d %4d %4d %4d  ", threadblock->id, threadblock->id_3d[2],
			threadblock->id_3d[1], threadblock->id_3d[0]);
		frm_isa_debug("%4d %4d %4d %4d  ", thread->id_in_threadblock, thread->id_in_threadblock_3d[2],
			thread->id_in_threadblock_3d[1], thread->id_in_threadblock_3d[0]);
		frm_isa_debug("%20s.%-4d  ", warp->name, thread->id_in_warp);
		frm_isa_debug("%20s.%-4d\n", threadblock->name, thread->id_in_threadblock);
	}

}


/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'frm_emu' or 'grid'? */
void frm_grid_setup_const_mem(struct frm_grid_t *grid)
{
	struct frm_cuda_kernel_t *kernel = grid->kernel;
	uint32_t zero = 0;
	float f;

	/* CB0[0]
	 * x,y,z: global work size for the {x,y,z} dimensions.
	 * w: number of work dimensions.  */
	frm_isa_const_mem_write(0, 0, 0, &kernel->global_size3[0]);
	frm_isa_const_mem_write(0, 0, 1, &kernel->global_size3[1]);
	frm_isa_const_mem_write(0, 0, 2, &kernel->global_size3[2]);
	frm_isa_const_mem_write(0, 0, 3, &kernel->work_dim);

	/* CB0[1]
	 * x,y,z: local work size for the {x,y,z} dimensions.
	 * w: 0  */
	frm_isa_const_mem_write(0, 1, 0, &kernel->local_size3[0]);
	frm_isa_const_mem_write(0, 1, 1, &kernel->local_size3[1]);
	frm_isa_const_mem_write(0, 1, 2, &kernel->local_size3[2]);
	frm_isa_const_mem_write(0, 1, 3, &zero);

	/* CB0[2]
	 * x,y,z: global work size {x,y,z} / local work size {x,y,z}
	 * w: 0  */
	frm_isa_const_mem_write(0, 2, 0, &kernel->group_count3[0]);
	frm_isa_const_mem_write(0, 2, 1, &kernel->group_count3[1]);
	frm_isa_const_mem_write(0, 2, 2, &kernel->group_count3[2]);
	frm_isa_const_mem_write(0, 2, 3, &zero);

	/* CB0[3]
	 * x: Offset to private memory ring (0 if private memory is not emulated).
	 * y: Private memory allocated per thread.
	 * z,w: 0  */
	/* FIXME */
	
	/* CB0[4]
	 * x: Offset to local memory ring (0 if local memory is not emulated).
	 * y: Local memory allocated per group.
	 * z: 0
	 * w: Pointer to location in global buffer where math library tables start. */
	/* FIXME */

	/* CB[5]
	 * x: 0.0 as IEEE-32bit float - required for math library.
	 * y: 0.5 as IEEE-32bit float - required for math library.
	 * z: 1.0 as IEEE-32bit float - required for math library.
	 * w: 2.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	frm_isa_const_mem_write(0, 5, 0, &f);
	f = 0.5f;
	frm_isa_const_mem_write(0, 5, 1, &f);
	f = 1.0f;
	frm_isa_const_mem_write(0, 5, 2, &f);
	f = 2.0f;
	frm_isa_const_mem_write(0, 5, 3, &f);

	/* CB0[6]
	 * x,y,z: Global offset for the {x,y,z} dimension of the thread spawn.
	 * z: Global single dimension flat offset: x * y * z. */
	frm_isa_const_mem_write(0, 6, 0, &zero);
	frm_isa_const_mem_write(0, 6, 1, &zero);
	frm_isa_const_mem_write(0, 6, 2, &zero);
	frm_isa_const_mem_write(0, 6, 3, &zero);

	/* CB0[7]
	 * x,y,z: Group offset for the {x,y,z} dimensions of the thread spawn.
	 * w: Group single dimension flat offset, x * y * z.  */
	frm_isa_const_mem_write(0, 7, 0, &zero);
	frm_isa_const_mem_write(0, 7, 1, &zero);
	frm_isa_const_mem_write(0, 7, 2, &zero);
	frm_isa_const_mem_write(0, 7, 3, &zero);

	/* CB0[8]
	 * x: Offset in the global buffer where data segment exists.
	 * y: Offset in buffer for printf support.
	 * z: Size of the printf buffer. */
}


void frm_grid_setup_args(struct frm_grid_t *grid)
{
	struct frm_cuda_kernel_t *kernel = grid->kernel;
	struct frm_cuda_kernel_arg_t *arg;
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

		case FRM_CUDA_KERNEL_ARG_KIND_VALUE:
		{
			/* Value copied directly into device constant memory */
			frm_isa_const_mem_write(1, cb_index, 0, &arg->value);
			frm_cuda_debug("    arg %d: value '0x%x' loaded into CB1[%d]\n", i, 
					arg->value, cb_index);
			cb_index++;
			break;
		}

		case FRM_CUDA_KERNEL_ARG_KIND_IMAGE:

			switch (arg->mem_scope) 
			{

			case FRM_CUDA_MEM_SCOPE_GLOBAL:
			{
				struct frm_cuda_mem_t *mem;

				/* Image type
				 * Images really take up two slots, but for now we'll
				 * just copy the pointer into both. */
				mem = frm_cuda_object_get(FRM_CUDA_OBJ_MEM, arg->value);
				frm_isa_const_mem_write(1, cb_index, 0, &mem->device_ptr);
				frm_cuda_debug("    arg %d: cuda_mem id 0x%x loaded into CB1[%d]," 
						" device_ptr=0x%x\n", i, arg->value, cb_index,
						mem->device_ptr);
				frm_isa_const_mem_write(1, cb_index+1, 0, &mem->device_ptr);
				cb_index += 2;
				break;
			}

			default:
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}
			break;

		case FRM_CUDA_KERNEL_ARG_KIND_POINTER:
		{
			switch (arg->mem_scope)
			{

			case FRM_CUDA_MEM_SCOPE_GLOBAL:
			{
				struct frm_cuda_mem_t *mem;

				/* Pointer in __global scope.
				 * Argument value is a pointer to an 'cuda_mem' object.
				 * It is translated first into a device memory pointer. */
				mem = frm_cuda_object_get(FRM_CUDA_OBJ_MEM, arg->value);
				frm_isa_const_mem_write(1, cb_index, 0, &mem->device_ptr);
				frm_cuda_debug("    arg %d: cuda_mem id 0x%x loaded into CB1[%d]," 
						" device_ptr=0x%x\n", i, arg->value, cb_index,
						mem->device_ptr);
				cb_index++;
				break;
			}

			case FRM_CUDA_MEM_SCOPE_LOCAL:
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign space for it. */
				frm_isa_const_mem_write(1, cb_index, 0, &grid->local_mem_top);
				frm_cuda_debug("    arg %d: %d bytes reserved in local memory at 0x%x\n",
					i, arg->size, grid->local_mem_top);
				grid->local_mem_top += arg->size;
				cb_index++;
				break;
			}

			default:
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}
			break;
		}

		case FRM_CUDA_KERNEL_ARG_KIND_SAMPLER:
		{
			frm_cuda_debug("    arg %d: sampler at CB1[%d]\n", i, cb_index);
			cb_index++;
			break;
		}

		default:
			fatal("%s: argument type not recognized", __FUNCTION__);
		}
	}
}


void frm_grid_run(struct frm_grid_t *grid)
{
	struct frm_threadblock_t *threadblock, *threadblock_next;
	struct frm_warp_t *warp, *warp_next;
	uint64_t cycle = 0;

	/* Set all ready threadblocks to running */
	while ((threadblock = grid->pending_list_head))
	{
		frm_threadblock_clear_status(threadblock, frm_threadblock_pending);
		frm_threadblock_set_status(threadblock, frm_threadblock_running);
	}

	/* Start GPU timer */
	frm_emu_timer_start();

	/* Execution loop */
	while (grid->running_list_head)
	{
		/* Stop if maximum number of GPU cycles exceeded */
		if (frm_emu_max_cycles && cycle >= frm_emu_max_cycles)
			x86_emu_finish = x86_emu_finish_max_gpu_cycles;

		/* Stop if maximum number of GPU instructions exceeded */
		if (frm_emu_max_inst && frm_emu->inst_count >= frm_emu_max_inst)
			x86_emu_finish = x86_emu_finish_max_gpu_inst;

		/* Stop if any reason met */
		if (x86_emu_finish)
			break;

		/* Next cycle */
		cycle++;

		/* Execute an instruction from each work-group */
		for (threadblock = grid->running_list_head; threadblock; threadblock = threadblock_next)
		{
			/* Save next running work-group */
			threadblock_next = threadblock->running_list_next;

			/* Run an instruction from each warp */
			for (warp = threadblock->running_list_head; warp; warp = warp_next)
			{
				/* Save next running warp */
				warp_next = warp->running_list_next;

				/* Execute instruction in warp */
				frm_warp_execute(warp);
			}
		}
	}

	/* Stop GPU timer */
	frm_emu_timer_stop();

	/* Dump stats */
	frm_grid_dump(grid, frm_emu_report_file);

	/* Stop if maximum number of kernels reached */
	if (frm_emu_max_kernels && frm_emu->grid_count >= frm_emu_max_kernels)
		x86_emu_finish = x86_emu_finish_max_gpu_kernels;
}


void frm_grid_dump(struct frm_grid_t *grid, FILE *f)
{
	struct frm_threadblock_t *threadblock;
	int threadblock_id;
	int thread_id, last_thread_id;
	uint32_t branch_digest, last_branch_digest;
	int branch_digest_count;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d] ]\n\n", grid->id);
	fprintf(f, "Name = %s\n", grid->name);
	fprintf(f, "WorkGroupFirst = %d\n", grid->threadblock_id_first);
	fprintf(f, "WorkGroupLast = %d\n", grid->threadblock_id_last);
	fprintf(f, "WorkGroupCount = %d\n", grid->threadblock_count);
	fprintf(f, "WaveFrontFirst = %d\n", grid->warp_id_first);
	fprintf(f, "WaveFrontLast = %d\n", grid->warp_id_last);
	fprintf(f, "WaveFrontCount = %d\n", grid->warp_count);
	fprintf(f, "WorkItemFirst = %d\n", grid->thread_id_first);
	fprintf(f, "WorkItemLast = %d\n", grid->thread_id_last);
	fprintf(f, "WorkItemCount = %d\n", grid->thread_count);

	/* Branch digests */
	assert(grid->thread_count);
	branch_digest_count = 0;
	last_thread_id = 0;
	last_branch_digest = grid->threads[0]->branch_digest;
	for (thread_id = 1; thread_id <= grid->thread_count; thread_id++)
	{
		branch_digest = thread_id < grid->thread_count ? grid->threads[thread_id]->branch_digest : 0;
		if (thread_id == grid->thread_count || branch_digest != last_branch_digest)
		{
			fprintf(f, "BranchDigest[%d] = %d %d %08x\n", branch_digest_count,
				last_thread_id, thread_id - 1, last_branch_digest);
			last_thread_id = thread_id;
			last_branch_digest = branch_digest;
			branch_digest_count++;
		}
	}
	fprintf(f, "BranchDigestCount = %d\n", branch_digest_count);
	fprintf(f, "\n");

	/* Work-groups */
	FRM_FOR_EACH_THREADBLOCK_IN_GRID(grid, threadblock_id)
	{
		threadblock = grid->threadblocks[threadblock_id];
		frm_threadblock_dump(threadblock, f);
	}
}
