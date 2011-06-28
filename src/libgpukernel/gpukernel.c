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

#include <gpukernel.h>
#include <gpudisasm.h>
#include <cpukernel.h>
#include <options.h>
#include <hash.h>


/* Global variables */

struct gk_t *gk;

char *gpu_opencl_binary_name = "";
char *gpu_report_file_name = "";
FILE *gpu_report_file = NULL;


/* Architectural parameters introduced in GPU emulator */
int gpu_wavefront_size = 64;
int gpu_max_work_group_size = 256;



/* Initialize GPU kernel */
void gk_init()
{
	struct opencl_device_t *device;

	/* Open report file */
	if (gpu_report_file_name[0]) {
		gpu_report_file = open_write(gpu_report_file_name);
		if (!gpu_report_file)
			fatal("%s: cannot open GPU report file ", gpu_report_file_name);
	}

	/* Initialize kernel */
	gk = calloc(1, sizeof(struct gk_t));
	gk->const_mem = mem_create();
	gk->const_mem->safe = 0;
	gk->global_mem = mem_create();
	gk->global_mem->safe = 0;

	/* Initialize disassembler (decoding tables...) */
	amd_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	gpu_isa_init();

	/* Create platform and device */
	opencl_object_list = lnlist_create();
	opencl_platform = opencl_platform_create();
	device = opencl_device_create();
}


/* Finalize GPU kernel */
void gk_done()
{
	/* GPU report */
	if (gpu_report_file)
		fclose(gpu_report_file);

	/* Free OpenCL objects */
	opencl_object_free_all();
	lnlist_free(opencl_object_list);

	/* Finalize disassembler */
	amd_disasm_done();

	/* Finalize ISA */
	gpu_isa_done();

	/* Finalize GPU kernel */
	mem_free(gk->const_mem);
	mem_free(gk->global_mem);
	free(gk);
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void gk_libopencl_redirect(char *fullpath, int size)
{
	char fullpath_original[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	char *relpath, *filename;
	int length;
	FILE *f;

	/* Get path length */
	strncpy(fullpath_original, fullpath, MAX_PATH_SIZE);
	length = strlen(fullpath);
	relpath = rindex(fullpath, '/');
	assert(relpath && *relpath == '/');
	filename = relpath + 1;

	/* Detect an attempt to open 'libm2s-opencl' and record it */
	if (!strcmp(filename, "libm2s-opencl.so")) {
		f = fopen(fullpath, "r");
		if (f) {
			fclose(f);
			isa_ctx->libopencl_open_attempt = 0;
		} else
			isa_ctx->libopencl_open_attempt = 1;
	}

	/* Translate libOpenCL -> libm2s-opencl */
	if (!strcmp(filename, "libOpenCL.so") || !strncmp(filename, "libOpenCL.so.", 13)) {
		
		/* Translate name in same path */
		fullpath[length - 13] = '\0';
		snprintf(buf, MAX_STRING_SIZE, "%s/libm2s-opencl.so", fullpath);
		strncpy(fullpath, buf, size);
		f = fopen(fullpath, "r");

		/* If attempt failed, translate name into current working directory */
		if (!f) {
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(fullpath, "%s/libm2s-opencl.so", buf);
			f = fopen(fullpath, "r");
		}

		/* End of attempts */
		if (f) {
			fclose(f);
			warning("path '%s' has been redirected to '%s'\n"
				"\tYour application is trying to access the default OpenCL library, which is being\n"
				"\tredirected by Multi2Sim to its own provided library. Though this should work,\n"
				"\tthe safest way to simulate an OpenCL program is by linking it initially with\n"
				"\t'libm2s-opencl.so'. See the Multi2Sim Guide for further details (www.multi2sim.org).\n",
				fullpath_original, fullpath);
			isa_ctx->libopencl_open_attempt = 0;
		} else
			isa_ctx->libopencl_open_attempt = 1;
	}
}


/* Dump a warning about failed attempts of context to access OpenCL library */
void gk_libopencl_failed(int pid)
{
	warning("context %d finished after failing to access OpenCL library.\n"
		"\tMulti2Sim has detected several attempts to access 'libm2s-opencl.so' by your\n"
		"\tapplication's dynamic linker. Please, make sure that this file is available\n"
		"\teither in any shared library path, in the current working directory, or in\n"
		"\tany directory pointed by the environment variable LD_LIBRARY_PATH. See the\n"
		"\tMulti2Sim Guide for further details (www.multi2sim.org).\n",
		pid);
}




/*
 * GPU ND-Range
 */

struct gpu_ndrange_t *gpu_ndrange_create(struct opencl_kernel_t *kernel)
{
	struct gpu_ndrange_t *ndrange;

	ndrange = calloc(1, sizeof(struct gpu_ndrange_t));
	ndrange->kernel = kernel;
	ndrange->local_mem_top = kernel->func_mem_local;
	return ndrange;
}


void gpu_ndrange_free(struct gpu_ndrange_t *ndrange)
{
	int i;

	/* Free work-groups */
	for (i = 0; i < ndrange->work_group_count; i++)
		gpu_work_group_free(ndrange->work_groups[i]);
	free(ndrange->work_groups);

	/* Free wavefronts */
	for (i = 0; i < ndrange->wavefront_count; i++)
		gpu_wavefront_free(ndrange->wavefronts[i]);
	free(ndrange->wavefronts);

	/* Free work-items */
	for (i = 0; i < ndrange->work_item_count; i++)
		gpu_work_item_free(ndrange->work_items[i]);
	free(ndrange->work_items);

	/* Free ndrange */
	free(ndrange);
}


void gpu_ndrange_setup_work_items(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;

	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	struct gpu_work_item_t *work_item;

	int gidx, gidy, gidz;  /* 3D work-group ID iterators */
	int lidx, lidy, lidz;  /* 3D work-item local ID iterators */

	int tid;  /* Global ID iterator */
	int gid;  /* Group ID iterator */
	int wid;  /* Wavefront ID iterator */
	int lid;  /* Local ID iterator */

	/* Array of work-groups */
	strcpy(ndrange->name, kernel->name);
	ndrange->work_group_count = kernel->group_count;
	ndrange->work_group_id_first = 0;
	ndrange->work_group_id_last = ndrange->work_group_count - 1;
	ndrange->work_groups = calloc(ndrange->work_group_count, sizeof(void *));
	for (gid = 0; gid < kernel->group_count; gid++) {
		ndrange->work_groups[gid] = gpu_work_group_create();
		work_group = ndrange->work_groups[gid];
	}
	
	/* Array of wavefronts */
	ndrange->wavefronts_per_work_group = (kernel->local_size + gpu_wavefront_size - 1) / gpu_wavefront_size;
	ndrange->wavefront_count = ndrange->wavefronts_per_work_group * ndrange->work_group_count;
	ndrange->wavefront_id_first = 0;
	ndrange->wavefront_id_last = ndrange->wavefront_count - 1;
	assert(ndrange->wavefronts_per_work_group > 0 && ndrange->wavefront_count > 0);
	ndrange->wavefronts = calloc(ndrange->wavefront_count, sizeof(void *));
	for (wid = 0; wid < ndrange->wavefront_count; wid++) {
		gid = wid / ndrange->wavefronts_per_work_group;
		ndrange->wavefronts[wid] = gpu_wavefront_create();
		wavefront = ndrange->wavefronts[wid];
		work_group = ndrange->work_groups[gid];

		wavefront->id = wid;
		wavefront->id_in_work_group = wid % ndrange->wavefronts_per_work_group;
		wavefront->ndrange = ndrange;
		wavefront->work_group = work_group;
		DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, running, wavefront);
	}
	
	/* Array of work-items */
	ndrange->work_item_count = kernel->global_size;
	ndrange->work_item_id_first = 0;
	ndrange->work_item_id_last = ndrange->work_item_count - 1;
	ndrange->work_items = calloc(ndrange->work_item_count, sizeof(void *));
	tid = 0;
	gid = 0;
	for (gidz = 0; gidz < kernel->group_count3[2]; gidz++) {
		for (gidy = 0; gidy < kernel->group_count3[1]; gidy++) {
			for (gidx = 0; gidx < kernel->group_count3[0]; gidx++) {
				
				/* Assign work-group ID */
				work_group = ndrange->work_groups[gid];
				work_group->ndrange = ndrange;
				work_group->id_3d[0] = gidx;
				work_group->id_3d[1] = gidy;
				work_group->id_3d[2] = gidz;
				work_group->id = gid;
				gpu_work_group_set_status(work_group, gpu_work_group_pending);

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
				for (lidz = 0; lidz < kernel->local_size3[2]; lidz++) {
					for (lidy = 0; lidy < kernel->local_size3[1]; lidy++) {
						for (lidx = 0; lidx < kernel->local_size3[0]; lidx++) {
							
							/* Wavefront ID */
							wid = gid * ndrange->wavefronts_per_work_group +
								lid / gpu_wavefront_size;
							assert(wid < ndrange->wavefront_count);
							wavefront = ndrange->wavefronts[wid];
							
							/* Create work-item */
							ndrange->work_items[tid] = gpu_work_item_create();
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
							work_item->id_in_wavefront = work_item->id_in_work_group % gpu_wavefront_size;
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

							/* Save local IDs in register R0 */
							work_item->gpr[0].elem[0] = lidx;  /* R0.x */
							work_item->gpr[0].elem[1] = lidy;  /* R0.y */
							work_item->gpr[0].elem[2] = lidz;  /* R0.z */

							/* Save work-group IDs in register R1 */
							work_item->gpr[1].elem[0] = gidx;  /* R1.x */
							work_item->gpr[1].elem[1] = gidy;  /* R1.y */
							work_item->gpr[1].elem[2] = gidz;  /* R1.z */

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
	for (wid = 0; wid < ndrange->wavefront_count; wid++) {
		wavefront = ndrange->wavefronts[wid];
		snprintf(wavefront->name, sizeof(wavefront->name), "wavefront[i%d-i%d]",
			wavefront->work_item_id_first, wavefront->work_item_id_last);

		/* Initialize wavefront program counter */
		if (!kernel->cal_abi->text_buffer)
			fatal("%s: cannot load kernel code", __FUNCTION__);
		wavefront->cf_buf_start = kernel->cal_abi->text_buffer;
		wavefront->cf_buf = kernel->cal_abi->text_buffer;
		wavefront->clause_kind = GPU_CLAUSE_CF;
		wavefront->emu_time_start = ke_timer();
	}

	/* Debug */
	gpu_isa_debug("local_size = %d (%d,%d,%d)\n", kernel->local_size, kernel->local_size3[0],
		kernel->local_size3[1], kernel->local_size3[2]);
	gpu_isa_debug("global_size = %d (%d,%d,%d)\n", kernel->global_size, kernel->global_size3[0],
		kernel->global_size3[1], kernel->global_size3[2]);
	gpu_isa_debug("group_count = %d (%d,%d,%d)\n", kernel->group_count, kernel->group_count3[0],
		kernel->group_count3[1], kernel->group_count3[2]);
	gpu_isa_debug("wavefront_count = %d\n", ndrange->wavefront_count);
	gpu_isa_debug("wavefronts_per_work_group = %d\n", ndrange->wavefronts_per_work_group);
	gpu_isa_debug(" tid tid2 tid1 tid0   gid gid2 gid1 gid0   lid lid2 lid1 lid0  wavefront            work-group\n");
	for (tid = 0; tid < ndrange->work_item_count; tid++) {
		work_item = ndrange->work_items[tid];
		wavefront = work_item->wavefront;
		work_group = work_item->work_group;
		gpu_isa_debug("%4d %4d %4d %4d  ", work_item->id, work_item->id_3d[2],
			work_item->id_3d[1], work_item->id_3d[0]);
		gpu_isa_debug("%4d %4d %4d %4d  ", work_group->id, work_group->id_3d[2],
			work_group->id_3d[1], work_group->id_3d[0]);
		gpu_isa_debug("%4d %4d %4d %4d  ", work_item->id_in_work_group, work_item->id_in_work_group_3d[2],
			work_item->id_in_work_group_3d[1], work_item->id_in_work_group_3d[0]);
		gpu_isa_debug("%20s.%-4d  ", wavefront->name, work_item->id_in_wavefront);
		gpu_isa_debug("%20s.%-4d\n", work_group->name, work_item->id_in_work_group);
	}

}


/* Write initial values in constant buffer 0 (CB0) */
/* FIXME: constant memory should be member of 'gk' or 'ndrange'? */
void gpu_ndrange_setup_const_mem(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	uint32_t zero = 0;
	float f;

	/* CB0[0]
	 * x,y,z: global work size for the {x,y,z} dimensions.
	 * w: number of work dimensions.  */
	gpu_isa_const_mem_write(0, 0, 0, &kernel->global_size3[0]);
	gpu_isa_const_mem_write(0, 0, 1, &kernel->global_size3[1]);
	gpu_isa_const_mem_write(0, 0, 2, &kernel->global_size3[2]);
	gpu_isa_const_mem_write(0, 0, 3, &kernel->work_dim);

	/* CB0[1]
	 * x,y,z: local work size for the {x,y,z} dimensions.
	 * w: 0  */
	gpu_isa_const_mem_write(0, 1, 0, &kernel->local_size3[0]);
	gpu_isa_const_mem_write(0, 1, 1, &kernel->local_size3[1]);
	gpu_isa_const_mem_write(0, 1, 2, &kernel->local_size3[2]);

	/* CB0[2]
	 * x,y,z: global work size {x,y,z} / local work size {x,y,z}
	 * w: 0  */
	gpu_isa_const_mem_write(0, 2, 0, &kernel->group_count3[0]);
	gpu_isa_const_mem_write(0, 2, 1, &kernel->group_count3[1]);
	gpu_isa_const_mem_write(0, 2, 2, &kernel->group_count3[2]);

	/* CB0[3]
	 * x: Offset to private memory ring (0 if private memory is not emulated).
	 * y: Private memory allocated per work_item.
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
	gpu_isa_const_mem_write(0, 5, 0, &f);
	f = 0.5f;
	gpu_isa_const_mem_write(0, 5, 1, &f);
	f = 1.0f;
	gpu_isa_const_mem_write(0, 5, 2, &f);
	f = 2.0f;
	gpu_isa_const_mem_write(0, 5, 3, &f);

	/* CB0[6]
	 * x,y,z: Global offset for the {x,y,z} dimension of the work_item spawn.
	 * z: Global single dimension flat offset: x * y * z. */
	gpu_isa_const_mem_write(0, 6, 0, &zero);
	gpu_isa_const_mem_write(0, 6, 1, &zero);
	gpu_isa_const_mem_write(0, 6, 2, &zero);
	gpu_isa_const_mem_write(0, 6, 3, &zero);

	/* CB0[7]
	 * x,y,z: Group offset for the {x,y,z} dimensions of the work_item spawn.
	 * w: Group single dimension flat offset, x * y * z.  */
	gpu_isa_const_mem_write(0, 7, 0, &zero);
	gpu_isa_const_mem_write(0, 7, 1, &zero);
	gpu_isa_const_mem_write(0, 7, 2, &zero);
	gpu_isa_const_mem_write(0, 7, 3, &zero);

	/* CB0[8]
	 * x: Offset in the global buffer where data segment exists.
	 * y: Offset in buffer for printf support.
	 * z: Size of the printf buffer. */
}


void gpu_ndrange_setup_args(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	struct opencl_kernel_arg_t *arg;
	int i;

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++) {

		arg = list_get(kernel->arg_list, i);
		assert(arg);

		/* Check that argument was set */
		if (!arg->set)
			fatal("kernel '%s': argument '%s' has not been assigned with 'clKernelSetArg'.",
				kernel->name, arg->name);

		/* Process argument depending on its type */
		switch (arg->kind) {

		case OPENCL_KERNEL_ARG_KIND_VALUE: {
			
			/* Value copied directly into device constant memory */
			gpu_isa_const_mem_write(1, i, 0, &arg->value);
			opencl_debug("    arg %d: value '0x%x' loaded\n", i, arg->value);
			break;
		}

		case OPENCL_KERNEL_ARG_KIND_POINTER:
		{
			switch (arg->mem_scope) {

			case OPENCL_MEM_SCOPE_GLOBAL:
			{
				struct opencl_mem_t *mem;

				/* Pointer in __global scope.
				 * Argument value is a pointer to an 'opencl_mem' object.
				 * It is translated first into a device memory pointer. */
				mem = opencl_object_get(OPENCL_OBJ_MEM, arg->value);
				gpu_isa_const_mem_write(1, i, 0, &mem->device_ptr);
				opencl_debug("    arg %d: opencl_mem id 0x%x loaded, device_ptr=0x%x\n",
					i, arg->value, mem->device_ptr);
				break;
			}

			case OPENCL_MEM_SCOPE_LOCAL:
			{
				/* Pointer in __local scope.
				 * Argument value is always NULL, just assign space for it. */
				gpu_isa_const_mem_write(1, i, 0, &ndrange->local_mem_top);
				opencl_debug("    arg %d: %d bytes reserved in local memory at 0x%x\n",
					i, arg->size, ndrange->local_mem_top);
				ndrange->local_mem_top += arg->size;
				break;
			}

			default:
				fatal("%s: argument in memory scope %d not supported",
					__FUNCTION__, arg->mem_scope);
			}
			break;
		}

		default:
			fatal("%s: argument type not recognized", __FUNCTION__);
		}
	}
}


void gpu_ndrange_run(struct gpu_ndrange_t *ndrange)
{
	struct gpu_work_group_t *work_group, *work_group_next;
	struct gpu_wavefront_t *wavefront, *wavefront_next;

	/* Set all ready work-groups to running */
	while ((work_group = ndrange->pending_list_head)) {
		gpu_work_group_clear_status(work_group, gpu_work_group_pending);
		gpu_work_group_set_status(work_group, gpu_work_group_running);
	}

	/* Execution loop */
	while (ndrange->running_list_head)
	{
		/* Execute an instruction from each work-group */
		for (work_group = ndrange->running_list_head; work_group; work_group = work_group_next) {
			
			/* Save next running work-group */
			work_group_next = work_group->running_next;

			/* Run an instruction from each wavefront */
			for (wavefront = work_group->running_list_head; wavefront; wavefront = wavefront_next) {
				
				/* Save next running wavefront */
				wavefront_next = wavefront->running_next;

				/* Execute instruction in wavefront */
				gpu_wavefront_execute(wavefront);
			}
		}
	}

	/* Dump stats */
	gpu_ndrange_dump(ndrange, gpu_report_file);
}


void gpu_ndrange_dump(struct gpu_ndrange_t *ndrange, FILE *f)
{
	struct gpu_work_group_t *work_group;
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
	fprintf(f, "\n");

	/* Work-groups */
	FOREACH_WORK_GROUP_IN_NDRANGE(ndrange, work_group_id) {
		work_group = ndrange->work_groups[work_group_id];
		gpu_work_group_dump(work_group, f);
	}
}




/*
 * GPU Work-Group
 */


struct gpu_work_group_t *gpu_work_group_create()
{
	struct gpu_work_group_t *work_group;

	work_group = calloc(1, sizeof(struct gpu_work_group_t));
	work_group->local_mem = mem_create();
	work_group->local_mem->safe = 0;
	return work_group;
}


void gpu_work_group_free(struct gpu_work_group_t *work_group)
{
	mem_free(work_group->local_mem);
	free(work_group);
}


int gpu_work_group_get_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status)
{
	return (work_group->status & status) > 0;
}


void gpu_work_group_set_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status)
{
	struct gpu_ndrange_t *ndrange = work_group->ndrange;

	/* Get only the new bits */
	status &= ~work_group->status;

	/* Add work-group to lists */
	if (status & gpu_work_group_pending)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, pending, work_group);
	if (status & gpu_work_group_running)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, running, work_group);
	if (status & gpu_work_group_finished)
		DOUBLE_LINKED_LIST_INSERT_TAIL(ndrange, finished, work_group);

	/* Update it */
	work_group->status |= status;
}


void gpu_work_group_clear_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status)
{
	struct gpu_ndrange_t *ndrange = work_group->ndrange;

	/* Get only the bits that are set */
	status &= work_group->status;

	/* Remove work-group from lists */
	if (status & gpu_work_group_pending)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, pending, work_group);
	if (status & gpu_work_group_running)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, running, work_group);
	if (status & gpu_work_group_finished)
		DOUBLE_LINKED_LIST_REMOVE(ndrange, finished, work_group);
	
	/* Update status */
	work_group->status &= ~status;
}


void gpu_work_group_dump(struct gpu_work_group_t *work_group, FILE *f)
{
	struct gpu_ndrange_t *ndrange = work_group->ndrange;
	struct gpu_wavefront_t *wavefront;
	int wavefront_id;

	if (!f)
		return;
	
	fprintf(f, "[ NDRange[%d].WorkGroup[%d] ]\n\n", ndrange->id, work_group->id);
	fprintf(f, "Name = %s\n", work_group->name);
	fprintf(f, "WaveFrontFirst = %d\n", work_group->wavefront_id_first);
	fprintf(f, "WaveFrontLast = %d\n", work_group->wavefront_id_last);
	fprintf(f, "WaveFrontCount = %d\n", work_group->wavefront_count);
	fprintf(f, "WorkItemFirst = %d\n", work_group->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", work_group->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", work_group->work_item_count);
	fprintf(f, "\n");

	/* Dump wavefronts */
	FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id) {
		wavefront = ndrange->wavefronts[wavefront_id];
		gpu_wavefront_dump(wavefront, f);
	}
}




/*
 * GPU wavefront
 */


struct gpu_wavefront_t *gpu_wavefront_create()
{
	struct gpu_wavefront_t *wavefront;

	wavefront = calloc(1, sizeof(struct gpu_wavefront_t));
	wavefront->active_stack = bit_map_create(GPU_MAX_STACK_SIZE * gpu_wavefront_size);
	wavefront->pred = bit_map_create(gpu_wavefront_size);
	return wavefront;
}


void gpu_wavefront_free(struct gpu_wavefront_t *wavefront)
{
	/* Free wavefront */
	bit_map_free(wavefront->active_stack);
	bit_map_free(wavefront->pred);
	free(wavefront);
}


/* Comparison function to sort list */
static int gpu_wavefront_divergence_compare(const void *elem1, const void *elem2)
{
	int count1 = * (int *) elem1;
	int count2 = * (int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


void gpu_wavefront_divergence_dump(struct gpu_wavefront_t *wavefront, FILE *f)
{
	struct gpu_work_item_t *work_item;
	struct elem_t {
		int count;  /* 1st field hardcoded for comparison */
		int list_index;
		uint32_t branch_digest;
	};
	struct elem_t *elem;
	struct list_t *list;
	struct hashtable_t *ht;
	char str[10];
	int i, j;

	/* Create list and hash table */
	list = list_create(20);
	ht = hashtable_create(20, 1);

	/* Create one 'elem' for each work_item with a different branch digest, and
	 * store it into the hash table and list. */
	for (i = 0; i < wavefront->work_item_count; i++) {
		work_item = wavefront->work_items[i];
		sprintf(str, "%08x", work_item->branch_digest);
		elem = hashtable_get(ht, str);
		if (!elem) {
			elem = calloc(1, sizeof(struct elem_t));
			hashtable_insert(ht, str, elem);
			elem->list_index = list_count(list);
			elem->branch_digest = work_item->branch_digest;
			list_add(list, elem);
		}
		elem->count++;
	}

	/* Sort divergence groups as per size */
	list_sort(list, gpu_wavefront_divergence_compare);
	fprintf(f, "DivergenceGroups = %d\n", list_count(list));
	
	/* Dump size of groups with */
	fprintf(f, "DivergenceGroupsSize =");
	for (i = 0; i < list_count(list); i++) {
		elem = list_get(list, i);
		fprintf(f, " %d", elem->count);
	}
	fprintf(f, "\n\n");

	/* Dump work_item ids contained in each work_item divergence group */
	for (i = 0; i < list_count(list); i++) {
		elem = list_get(list, i);
		fprintf(f, "DivergenceGroup[%d] =", i);

		for (j = 0; j < wavefront->work_item_count; j++) {
			int first, last;
			first = wavefront->work_items[j]->branch_digest == elem->branch_digest &&
				(j == 0 || wavefront->work_items[j - 1]->branch_digest != elem->branch_digest);
			last = wavefront->work_items[j]->branch_digest == elem->branch_digest &&
				(j == wavefront->work_item_count - 1 || wavefront->work_items[j + 1]->branch_digest != elem->branch_digest);
			if (first)
				fprintf(f, " %d", j);
			else if (last)
				fprintf(f, "-%d", j);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");

	/* Free 'elem's and structures */
	for (i = 0; i < list_count(list); i++)
		free(list_get(list, i));
	list_free(list);
	hashtable_free(ht);
}


void gpu_wavefront_dump(struct gpu_wavefront_t *wavefront, FILE *f)
{
	struct gpu_ndrange_t *ndrange = wavefront->ndrange;
	struct gpu_work_group_t *work_group = wavefront->work_group;
	int i;

	if (!f)
		return;
	
	/* Dump wavefront statistics in GPU report */
	fprintf(f, "[ NDRange[%d].Wavefront[%d] ]\n\n", ndrange->id, wavefront->id);

	fprintf(f, "Name = %s\n", wavefront->name);
	fprintf(f, "WorkGroup = %d\n", work_group->id);
	fprintf(f, "WorkItemFirst = %d\n", wavefront->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", wavefront->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", wavefront->work_item_count);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", (long long) wavefront->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", (long long) wavefront->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", (long long) wavefront->local_mem_inst_count);
	fprintf(f, "\n");

	fprintf(f, "CF_Inst_Count = %lld\n", (long long) wavefront->cf_inst_count);
	fprintf(f, "CF_Inst_Global_Mem_Write_Count = %lld\n", (long long) wavefront->cf_inst_global_mem_write_count);
	fprintf(f, "\n");

	fprintf(f, "ALU_Clause_Count = %lld\n", (long long) wavefront->alu_clause_count);
	fprintf(f, "ALU_Group_Count = %lld\n", (long long) wavefront->alu_group_count);
	fprintf(f, "ALU_Group_Size =");
	for (i = 0; i < 5; i++)
		fprintf(f, " %lld", (long long) wavefront->alu_group_size[i]);
	fprintf(f, "\n");
	fprintf(f, "ALU_Inst_Count = %lld\n", (long long) wavefront->alu_inst_count);
	fprintf(f, "ALU_Inst_Local_Mem_Count = %lld\n", (long long) wavefront->alu_inst_local_mem_count);
	fprintf(f, "\n");

	fprintf(f, "TC_Clause_Count = %lld\n", (long long) wavefront->tc_clause_count);
	fprintf(f, "TC_Inst_Count = %lld\n", (long long) wavefront->tc_inst_count);
	fprintf(f, "TC_Inst_Global_Mem_Read_Count = %lld\n", (long long) wavefront->tc_inst_global_mem_read_count);
	fprintf(f, "\n");

	gpu_wavefront_divergence_dump(wavefront, f);

	fprintf(f, "\n");
}


void gpu_wavefront_stack_push(struct gpu_wavefront_t *wavefront)
{
	if (wavefront->stack_top == GPU_MAX_STACK_SIZE - 1)
		fatal("%s: stack overflow", wavefront->cf_inst.info->name);
	wavefront->stack_top++;
	bit_map_copy(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
		wavefront->active_stack, (wavefront->stack_top - 1) * wavefront->work_item_count,
		wavefront->work_item_count);
	gpu_isa_debug("  %s:push", wavefront->name);
	wavefront->active_mask_push = 1;
}


void gpu_wavefront_stack_pop(struct gpu_wavefront_t *wavefront, int count)
{
	if (!count)
		return;
	if (wavefront->stack_top < count)
		fatal("%s: stack underflow", wavefront->cf_inst.info->name);
	wavefront->stack_top -= count;
	wavefront->active_mask_pop = 1;
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:pop(%d),act=", wavefront->name, count);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
			wavefront->work_item_count, debug_file(gpu_isa_debug_category));
	}
}


/* Execute one instruction in the wavefront */
void gpu_wavefront_execute(struct gpu_wavefront_t *wavefront)
{
	extern struct gpu_ndrange_t *gpu_isa_ndrange;
	extern struct gpu_work_group_t *gpu_isa_work_group;
	extern struct gpu_wavefront_t *gpu_isa_wavefront;
	extern struct gpu_work_item_t *gpu_isa_work_item;
	extern struct amd_inst_t *gpu_isa_cf_inst;
	extern struct amd_inst_t *gpu_isa_inst;
	extern struct amd_alu_group_t *gpu_isa_alu_group;

	struct gpu_ndrange_t *ndrange = wavefront->ndrange;
	struct opencl_kernel_t *kernel = ndrange->kernel;

	int work_item_id;

	/* Get current work-group */
	gpu_isa_ndrange = wavefront->ndrange;
	gpu_isa_wavefront = wavefront;
	gpu_isa_work_group = wavefront->work_group;
	gpu_isa_work_item = NULL;
	gpu_isa_cf_inst = NULL;
	gpu_isa_inst = NULL;
	gpu_isa_alu_group = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_isa_work_group, finished, gpu_isa_wavefront));

	/* Reset instruction flags */
	wavefront->global_mem_write = 0;
	wavefront->global_mem_read = 0;
	wavefront->local_mem_write = 0;
	wavefront->local_mem_read = 0;
	wavefront->pred_mask_update = 0;
	wavefront->active_mask_update = 0;
	wavefront->active_mask_push = 0;
	wavefront->active_mask_pop = 0;

	switch (wavefront->clause_kind) {

	case GPU_CLAUSE_CF:
	{
		int inst_num;

		/* Decode CF instruction */
		inst_num = (gpu_isa_wavefront->cf_buf - kernel->cal_abi->text_buffer) / 8;
		gpu_isa_wavefront->cf_buf = amd_inst_decode_cf(gpu_isa_wavefront->cf_buf, &gpu_isa_wavefront->cf_inst);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_inst_dump(&gpu_isa_wavefront->cf_inst, inst_num, 0,
				debug_file(gpu_isa_debug_category));
		}

		/* Execute once in wavefront */
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		gpu_isa_inst = &gpu_isa_wavefront->cf_inst;
		(*amd_inst_impl[gpu_isa_inst->info->inst])();

		/* If instruction updates the work_item's active mask, update digests */
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_ACT_MASK) {
			FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
				gpu_isa_work_item = ndrange->work_items[work_item_id];
				gpu_work_item_update_branch_digest(gpu_isa_work_item, gpu_isa_wavefront->cf_inst_count, inst_num);
			}
		}

		/* Stats */
		gpu_isa_wavefront->emu_inst_count++;
		gpu_isa_wavefront->inst_count++;
		gpu_isa_wavefront->cf_inst_count++;
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_MEM) {
			gpu_isa_wavefront->global_mem_inst_count++;
			gpu_isa_wavefront->cf_inst_global_mem_write_count++;  /* CF inst accessing memory is a write */
		}

		break;
	}

	case GPU_CLAUSE_ALU:
	{
		int i;

		/* Decode ALU group */
		gpu_isa_wavefront->clause_buf = amd_inst_decode_alu_group(gpu_isa_wavefront->clause_buf,
			gpu_isa_wavefront->alu_group_count, &gpu_isa_wavefront->alu_group);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_alu_group_dump(&gpu_isa_wavefront->alu_group, 0, debug_file(gpu_isa_debug_category));
		}

		/* Execute group for each work_item in wavefront */
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		gpu_isa_alu_group = &gpu_isa_wavefront->alu_group;
		FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
			gpu_isa_work_item = ndrange->work_items[work_item_id];
			for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
				gpu_isa_inst = &gpu_isa_alu_group->inst[i];
				(*amd_inst_impl[gpu_isa_inst->info->inst])();
			}
			gpu_isa_write_task_commit();
		}
		
		/* Stats */
		gpu_isa_wavefront->inst_count += gpu_isa_alu_group->inst_count;
		gpu_isa_wavefront->alu_inst_count += gpu_isa_alu_group->inst_count;
		gpu_isa_wavefront->alu_group_count++;
		gpu_isa_wavefront->emu_inst_count += gpu_isa_alu_group->inst_count * gpu_isa_wavefront->work_item_count;
		assert(gpu_isa_alu_group->inst_count > 0 && gpu_isa_alu_group->inst_count < 6);
		gpu_isa_wavefront->alu_group_size[gpu_isa_alu_group->inst_count - 1]++;
		for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
			gpu_isa_inst = &gpu_isa_alu_group->inst[i];
			if (gpu_isa_inst->info->flags & AMD_INST_FLAG_LDS) {
				gpu_isa_wavefront->local_mem_inst_count++;
				gpu_isa_wavefront->alu_inst_local_mem_count++;
			}
		}

		/* End of clause reached */
		assert(gpu_isa_wavefront->clause_buf <= gpu_isa_wavefront->clause_buf_end);
		if (gpu_isa_wavefront->clause_buf >= gpu_isa_wavefront->clause_buf_end) {
			gpu_isa_alu_clause_end();
			gpu_isa_wavefront->clause_kind = GPU_CLAUSE_CF;
		}

		break;
	}

	case GPU_CLAUSE_TC:
	{
		/* Decode TEX inst */
		gpu_isa_wavefront->clause_buf = amd_inst_decode_tc(gpu_isa_wavefront->clause_buf,
			&gpu_isa_wavefront->tc_inst);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_inst_dump(&gpu_isa_wavefront->tc_inst, 0, 0, debug_file(gpu_isa_debug_category));
		}

		/* Execute in all work_items */
		gpu_isa_inst = &gpu_isa_wavefront->tc_inst;
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
			gpu_isa_work_item = ndrange->work_items[work_item_id];
			(*amd_inst_impl[gpu_isa_inst->info->inst])();
		}

		/* Stats */
		gpu_isa_wavefront->emu_inst_count += gpu_isa_wavefront->work_item_count;
		gpu_isa_wavefront->inst_count++;
		gpu_isa_wavefront->tc_inst_count++;
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_MEM) {
			gpu_isa_wavefront->global_mem_inst_count++;
			gpu_isa_wavefront->tc_inst_global_mem_read_count++;  /* Memory instructions in TC are reads */
		}

		/* End of clause reached */
		assert(gpu_isa_wavefront->clause_buf <= gpu_isa_wavefront->clause_buf_end);
		if (gpu_isa_wavefront->clause_buf == gpu_isa_wavefront->clause_buf_end) {
			gpu_isa_tc_clause_end();
			gpu_isa_wavefront->clause_kind = GPU_CLAUSE_CF;
		}

		break;
	}

	default:
		abort();
	}

	/* Check if wavefront finished kernel execution */
	if (gpu_isa_wavefront->clause_kind == GPU_CLAUSE_CF && !gpu_isa_wavefront->cf_buf) {
		assert(DOUBLE_LINKED_LIST_MEMBER(gpu_isa_work_group, running, gpu_isa_wavefront));
		assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_isa_work_group, finished, gpu_isa_wavefront));
		DOUBLE_LINKED_LIST_REMOVE(gpu_isa_work_group, running, gpu_isa_wavefront);
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_isa_work_group, finished, gpu_isa_wavefront);

		/* Check if work-group finished kernel execution */
		if (gpu_isa_work_group->finished_count == gpu_isa_work_group->wavefront_count) {
			assert(DOUBLE_LINKED_LIST_MEMBER(ndrange, running, gpu_isa_work_group));
			assert(!DOUBLE_LINKED_LIST_MEMBER(ndrange, finished, gpu_isa_work_group));
			gpu_work_group_clear_status(gpu_isa_work_group, gpu_work_group_running);
			gpu_work_group_set_status(gpu_isa_work_group, gpu_work_group_finished);
		}
	}
}





/*
 * GPU work_item
 */

struct gpu_work_item_t *gpu_work_item_create()
{
	struct gpu_work_item_t *work_item;
	work_item = calloc(1, sizeof(struct gpu_work_item_t));
	work_item->write_task_list = lnlist_create();
	work_item->lds_oqa = list_create(5);
	work_item->lds_oqb = list_create(5);
	return work_item;
}


void gpu_work_item_free(struct gpu_work_item_t *work_item)
{
	/* Empty LDS output queues */
	while (list_count(work_item->lds_oqa))
		free(list_dequeue(work_item->lds_oqa));
	while (list_count(work_item->lds_oqb))
		free(list_dequeue(work_item->lds_oqb));
	list_free(work_item->lds_oqa);
	list_free(work_item->lds_oqb);
	lnlist_free(work_item->write_task_list);

	/* Free work_item */
	free(work_item);
}



void gpu_work_item_set_active(struct gpu_work_item_t *work_item, int active)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1, !!active);
	wavefront->active_mask_update = 1;
}


int gpu_work_item_get_active(struct gpu_work_item_t *work_item)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1);
}


void gpu_work_item_set_pred(struct gpu_work_item_t *work_item, int pred)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	bit_map_set(wavefront->pred, work_item->id_in_wavefront, 1, !!pred);
	wavefront->pred_mask_update = 1;
}


int gpu_work_item_get_pred(struct gpu_work_item_t *work_item)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->id_in_wavefront >= 0 && work_item->id_in_wavefront < wavefront->work_item_count);
	return bit_map_get(wavefront->pred, work_item->id_in_wavefront, 1);
}


/* Based on an instruction counter, instruction address, and work_item mask,
 * update (xor) branch_digest with a random number */
void gpu_work_item_update_branch_digest(struct gpu_work_item_t *work_item, uint64_t inst_count, uint32_t inst_addr)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	uint32_t mask = 0;

	/* Update branch digest only if work_item is active */
	if (!bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->id_in_wavefront, 1))
		return;

	/* Update mask with inst_count */
	mask = (uint32_t) inst_count * 0x4919f71f;  /* Multiply by prime number to generate sparse mask */

	/* Update mask with inst_addr */
	mask += inst_addr * 0x31f2e73b;

	/* Update branch digest */
	work_item->branch_digest ^= mask;
}



