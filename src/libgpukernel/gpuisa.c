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

#include <gpukernel-local.h>
#include <gpudisasm.h>
#include <repos.h>


/* Some globals */

struct gpu_work_item_t *gpu_isa_work_item;  /* Current work-item */
struct gpu_wavefront_t *gpu_isa_wavefront;  /* Current wavefront */
struct gpu_work_group_t *gpu_isa_work_group;  /* Current work-group */
struct amd_inst_t *gpu_isa_inst;  /* Current instruction */
struct amd_inst_t *gpu_isa_cf_inst;  /* Current CF instruction */
struct amd_alu_group_t *gpu_isa_alu_group;  /* Current ALU group */

struct gpu_work_item_t **gpu_isa_work_items;  /* Array of kernel work-items */
struct gpu_wavefront_t **gpu_isa_wavefronts;  /* Array of wavefronts */
int gpu_wavefront_count;  /* Number of wavefronts */
struct gpu_work_group_t **gpu_isa_work_groups;

/* Repository of deferred tasks */
struct repos_t *gpu_isa_write_task_repos;

/* Instruction execution table */
amd_inst_impl_t *amd_inst_impl;

/* Debug */
int gpu_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void gpu_isa_init()
{
	/* Initialize instruction execution table */
	amd_inst_impl = calloc(AMD_INST_COUNT, sizeof(amd_inst_impl_t));
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	extern void amd_inst_##_name##_impl(); \
	amd_inst_impl[AMD_INST_##_name] = amd_inst_##_name##_impl;
#include <gpudisasm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
	gpu_isa_write_task_repos = repos_create(sizeof(struct gpu_isa_write_task_t),
		"gpu_isa_write_task_repos");
}


void gpu_isa_done()
{
	/* Instruction execution table */
	free(amd_inst_impl);

	/* Repository of deferred tasks */
	repos_free(gpu_isa_write_task_repos);
}




/*
 * Constant Memory
 */

void gpu_isa_const_mem_write(int bank, int vector, int elem, void *pvalue)
{
	uint32_t addr;

	/* Mark CB0[0..8].{x,y,z,w} positions as initialized */
	if (!bank && vector < 9)
		gk->const_mem_cb0_init[vector * 4 + elem] = 1;

	/* Write */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_write(gk->const_mem, addr, 4, pvalue);
}


void gpu_isa_const_mem_read(int bank, int vector, int elem, void *pvalue)
{
	uint32_t addr;

	/* Warn if a position within CB[0..8].{x,y,z,w} is used uninitialized */
	if (!bank && vector < 9 && !gk->const_mem_cb0_init[vector * 4 + elem])
		warning("CB0[%d].%c is used uninitialized", vector, "xyzw"[elem]);
	
	/* Read */
	addr = bank * 16384 + vector * 16 + elem * 4;
	mem_read(gk->const_mem, addr, 4, pvalue);
}




/*
 * Main loop
 */


/* Write initial values in constant buffer 0 (CB0) */
void gpu_isa_init_const_mem(struct opencl_kernel_t *kernel)
{
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


void gpu_isa_init_work_items(struct opencl_kernel_t *kernel)
{
	int i;
	int global_id;
	int local_id;

	int wavefronts_per_work_group;
	int wavefront_index;
	int work_group_index;

	int gidx, gidy, gidz;
	int lidx, lidy, lidz;
	gpu_isa_work_items = calloc(kernel->global_size, sizeof(void *));
	global_id = 0;
	for (gidz = 0; gidz < kernel->group_count3[2]; gidz++) {
		for (gidy = 0; gidy < kernel->group_count3[1]; gidy++) {
			for (gidx = 0; gidx < kernel->group_count3[0]; gidx++) {
				local_id = 0;
				for (lidz = 0; lidz < kernel->local_size3[2]; lidz++) {
					for (lidy = 0; lidy < kernel->local_size3[1]; lidy++) {
						for (lidx = 0; lidx < kernel->local_size3[0]; lidx++) {
							gpu_isa_work_items[global_id] = gpu_work_item_create();
							gpu_isa_work_item = gpu_isa_work_items[global_id];

							WORK_ITEM.group_id3[0] = GPU_GPR_X(1) = gidx;
							WORK_ITEM.group_id3[1] = GPU_GPR_Y(1) = gidy;
							WORK_ITEM.group_id3[2] = GPU_GPR_Z(1) = gidz;
							WORK_ITEM.group_id = gidz * kernel->group_count3[1] * kernel->group_count3[0]
								+ gidy * kernel->group_count3[0] + gidx;

							WORK_ITEM.local_id3[0] = GPU_GPR_X(0) = lidx;
							WORK_ITEM.local_id3[1] = GPU_GPR_Y(0) = lidy;
							WORK_ITEM.local_id3[2] = GPU_GPR_Z(0) = lidz;
							WORK_ITEM.local_id = local_id;

							WORK_ITEM.global_id3[0] = gidx * kernel->local_size3[0] + lidx;
							WORK_ITEM.global_id3[1] = gidy * kernel->local_size3[1] + lidy;
							WORK_ITEM.global_id3[2] = gidz * kernel->local_size3[2] + lidz;
							WORK_ITEM.global_id = global_id;

							global_id++;
							local_id++;
						}
					}
				}
			}
		}
	}

	/* Array of work-groups */
	gpu_isa_work_groups = calloc(kernel->group_count, sizeof(void *));
	for (i = 0; i < kernel->group_count; i++)
		gpu_isa_work_groups[i] = gpu_work_group_create();
	
	/* Assign work-items for work-groups */
	for (i = 0; i < kernel->global_size; i++) {
		gpu_isa_work_item = gpu_isa_work_items[i];
		work_group_index = i / kernel->local_size;
		assert(work_group_index < kernel->group_count);
		gpu_isa_work_group = gpu_isa_work_groups[work_group_index];

		WORK_ITEM.work_group = gpu_isa_work_group;
		if (!WORK_ITEM.local_id) {
			WORK_GROUP.work_items = &gpu_isa_work_items[i];
			WORK_GROUP.work_item_count = kernel->local_size;
			WORK_GROUP.global_id = i;
			WORK_GROUP.global_id_last = i + kernel->local_size - 1;
			snprintf(WORK_GROUP.name, sizeof(WORK_GROUP.name), "work-group[i%d-i%d]",
				i, i + kernel->local_size - 1);
		}
	}

	/* Array of wavefronts */
	wavefronts_per_work_group = (kernel->local_size + gpu_wavefront_size - 1) / gpu_wavefront_size;
	gpu_wavefront_count = wavefronts_per_work_group * kernel->group_count;
	assert(wavefronts_per_work_group > 0 && gpu_wavefront_count > 0);
	gpu_isa_wavefronts = calloc(gpu_wavefront_count, sizeof(void *));
	for (i = 0; i < gpu_wavefront_count; i++)
		gpu_isa_wavefronts[i] = gpu_wavefront_create();
	
	/* Assign work-items to wavefronts */
	for (i = 0; i < kernel->global_size; i++) {
		gpu_isa_work_item = gpu_isa_work_items[i];
		wavefront_index = WORK_ITEM.group_id * wavefronts_per_work_group + WORK_ITEM.local_id / gpu_wavefront_size;
		assert(wavefront_index < gpu_wavefront_count);
		gpu_isa_wavefront = gpu_isa_wavefronts[wavefront_index];

		WORK_ITEM.wavefront = gpu_isa_wavefront;
		WORK_ITEM.wavefront_id = WORK_ITEM.local_id % gpu_wavefront_size;
		
		/* Work-item is initially active */
		bit_map_set(WAVEFRONT.active_stack, WORK_ITEM.wavefront_id, 1, 1);
		if (!WORK_ITEM.wavefront_id) {
			WAVEFRONT.work_items = &gpu_isa_work_items[i];
			WAVEFRONT.global_id = i;
		}
		WAVEFRONT.global_id_last = i;
		WAVEFRONT.work_item_count = i - WAVEFRONT.global_id + 1;
	}

	/* Assign names to wavefronts */
	for (i = 0; i < gpu_wavefront_count; i++) {
		gpu_isa_wavefront = gpu_isa_wavefronts[i];
		snprintf(WAVEFRONT.name, sizeof(WAVEFRONT.name), "wavefront[i%d..i%d]",
			WAVEFRONT.global_id, WAVEFRONT.global_id_last);
	}

	/* Assign wavefronts to work-groups */
	for (i = 0; i < gpu_wavefront_count; i++) {
		gpu_isa_wavefront = gpu_isa_wavefronts[i];
		work_group_index = i / wavefronts_per_work_group;
		assert(work_group_index < kernel->group_count);
		gpu_isa_work_group = gpu_isa_work_groups[work_group_index];

		WAVEFRONT.work_group = gpu_isa_work_group;
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_isa_work_group, wavefront, gpu_isa_wavefront);
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_isa_work_group, running, gpu_isa_wavefront);
		if (i % wavefronts_per_work_group == 0) {
			WORK_GROUP.wavefronts = &gpu_isa_wavefronts[i];
			WORK_GROUP.wavefront_count = wavefronts_per_work_group;
		}
	}

	/* Debug */
	gpu_isa_debug("local_size = %d (%d,%d,%d)\n", kernel->local_size, kernel->local_size3[0],
		kernel->local_size3[1], kernel->local_size3[2]);
	gpu_isa_debug("global_size = %d (%d,%d,%d)\n", kernel->global_size, kernel->global_size3[0],
		kernel->global_size3[1], kernel->global_size3[2]);
	gpu_isa_debug("group_count = %d (%d,%d,%d)\n", kernel->group_count, kernel->group_count3[0],
		kernel->group_count3[1], kernel->group_count3[2]);
	gpu_isa_debug("wavefront_count = %d\n", gpu_wavefront_count);
	gpu_isa_debug("wavefronts_per_work_group = %d\n", wavefronts_per_work_group);
	gpu_isa_debug(" tid tid2 tid1 tid0   gid gid2 gid1 gid0   lid lid2 lid1 lid0  wavefront            work-group\n");
	for (i = 0; i < kernel->global_size; i++) {
		gpu_isa_work_item = gpu_isa_work_items[i];
		gpu_isa_wavefront = WORK_ITEM.wavefront;
		gpu_isa_work_group = WORK_ITEM.work_group;
		gpu_isa_debug("%4d %4d %4d %4d  ", WORK_ITEM.global_id, WORK_ITEM.global_id3[2],
			WORK_ITEM.global_id3[1], WORK_ITEM.global_id3[0]);
		gpu_isa_debug("%4d %4d %4d %4d  ", WORK_ITEM.group_id, WORK_ITEM.group_id3[2],
			WORK_ITEM.group_id3[1], WORK_ITEM.group_id3[0]);
		gpu_isa_debug("%4d %4d %4d %4d  ", WORK_ITEM.local_id, WORK_ITEM.local_id3[2],
			WORK_ITEM.local_id3[1], WORK_ITEM.local_id3[0]);
		gpu_isa_debug("%20s.%-4d  ", WAVEFRONT.name, WORK_ITEM.wavefront_id);
		gpu_isa_debug("%20s.%-4d\n", WORK_GROUP.name, WORK_ITEM.local_id);
	}

}


void gpu_isa_init_kernel_args(struct opencl_kernel_t *kernel)
{
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
				gpu_isa_const_mem_write(1, i, 0, &kernel->local_mem_top);
				opencl_debug("    arg %d: %d bytes reserved in local memory at 0x%x\n",
					i, arg->size, kernel->local_mem_top);
				kernel->local_mem_top += arg->size;
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


/* Execute one instruction from 'gpu_isa_wavefront' */
void gpu_isa_wavefront_exec(struct opencl_kernel_t *kernel)
{
	int global_id;
	int i;

	switch (WAVEFRONT.clause_kind) {

	case GPU_CLAUSE_CF:
	{
		int inst_num;

		/* Decode CF instruction */
		inst_num = (WAVEFRONT.cf_buf - kernel->cal_abi->text_buffer) / 8;
		WAVEFRONT.cf_buf = amd_inst_decode_cf(WAVEFRONT.cf_buf, &WAVEFRONT.cf_inst);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_inst_dump(&WAVEFRONT.cf_inst, inst_num, 0,
				debug_file(gpu_isa_debug_category));
		}

		/* Execute once in wavefront */
		gpu_isa_inst = &WAVEFRONT.cf_inst;
		gpu_isa_cf_inst = &WAVEFRONT.cf_inst;
		gpu_isa_work_item = NULL;
		(*amd_inst_impl[gpu_isa_inst->info->inst])();

		/* If instruction updates the work_item's active mask, update digests */
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_ACT_MASK) {
			for (global_id = WAVEFRONT.global_id; global_id < WAVEFRONT.global_id +
				WAVEFRONT.work_item_count; global_id++)
			{
				gpu_isa_work_item = gpu_isa_work_items[global_id];
				gpu_work_item_update_branch_digest(gpu_isa_work_item, WAVEFRONT.cf_inst_count, inst_num);
			}
		}

		/* Stats */
		WAVEFRONT.emu_inst_count++;
		WAVEFRONT.inst_count++;
		WAVEFRONT.cf_inst_count++;
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_MEM) {
			WAVEFRONT.global_mem_inst_count++;
			WAVEFRONT.cf_inst_global_mem_write_count++;  /* CF inst accessing memory is a write */
		}

		break;
	}

	case GPU_CLAUSE_ALU:
	{
		/* Decode ALU group */
		WAVEFRONT.clause_buf = amd_inst_decode_alu_group(WAVEFRONT.clause_buf,
			WAVEFRONT.alu_group_count, &WAVEFRONT.alu_group);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_alu_group_dump(&WAVEFRONT.alu_group, 0, debug_file(gpu_isa_debug_category));
		}

		/* Execute group for each work_item in wavefront */
		gpu_isa_cf_inst = &WAVEFRONT.cf_inst;
		gpu_isa_alu_group = &WAVEFRONT.alu_group;
		for (global_id = WAVEFRONT.global_id; global_id < WAVEFRONT.global_id +
			WAVEFRONT.work_item_count; global_id++)
		{
			gpu_isa_work_item = gpu_isa_work_items[global_id];
			for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
				gpu_isa_inst = &gpu_isa_alu_group->inst[i];
				(*amd_inst_impl[gpu_isa_inst->info->inst])();
			}
			gpu_isa_write_task_commit();
		}
		
		/* Stats */
		WAVEFRONT.inst_count += gpu_isa_alu_group->inst_count;
		WAVEFRONT.alu_inst_count += gpu_isa_alu_group->inst_count;
		WAVEFRONT.alu_group_count++;
		WAVEFRONT.emu_inst_count += gpu_isa_alu_group->inst_count * WAVEFRONT.work_item_count;
		assert(gpu_isa_alu_group->inst_count > 0 && gpu_isa_alu_group->inst_count < 6);
		WAVEFRONT.alu_group_size[gpu_isa_alu_group->inst_count - 1]++;
		for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
			gpu_isa_inst = &gpu_isa_alu_group->inst[i];
			if (gpu_isa_inst->info->flags & AMD_INST_FLAG_LDS) {
				WAVEFRONT.local_mem_inst_count++;
				WAVEFRONT.alu_inst_local_mem_count++;
			}
		}

		/* End of clause reached */
		assert(WAVEFRONT.clause_buf <= WAVEFRONT.clause_buf_end);
		if (WAVEFRONT.clause_buf >= WAVEFRONT.clause_buf_end) {
			gpu_isa_alu_clause_end();
			WAVEFRONT.clause_kind = GPU_CLAUSE_CF;
		}

		break;
	}

	case GPU_CLAUSE_TC:
	{
		/* Decode TEX inst */
		WAVEFRONT.clause_buf = amd_inst_decode_tc(WAVEFRONT.clause_buf,
			&WAVEFRONT.tc_inst);

		/* Debug */
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("\n\n");
			amd_inst_dump(&WAVEFRONT.tc_inst, 0, 0, debug_file(gpu_isa_debug_category));
		}

		/* Execute in all work_items */
		gpu_isa_inst = &WAVEFRONT.tc_inst;
		gpu_isa_cf_inst = &WAVEFRONT.cf_inst;
		for (global_id = WAVEFRONT.global_id; global_id < WAVEFRONT.global_id +
			WAVEFRONT.work_item_count; global_id++)
		{
			gpu_isa_work_item = gpu_isa_work_items[global_id];
			(*amd_inst_impl[gpu_isa_inst->info->inst])();
		}

		/* Stats */
		WAVEFRONT.emu_inst_count += WAVEFRONT.work_item_count;
		WAVEFRONT.inst_count++;
		WAVEFRONT.tc_inst_count++;
		if (gpu_isa_inst->info->flags & AMD_INST_FLAG_MEM) {
			WAVEFRONT.global_mem_inst_count++;
			WAVEFRONT.tc_inst_global_mem_read_count++;  /* Memory instructions in TC are reads */
		}

		/* End of clause reached */
		assert(WAVEFRONT.clause_buf <= WAVEFRONT.clause_buf_end);
		if (WAVEFRONT.clause_buf == WAVEFRONT.clause_buf_end) {
			gpu_isa_tc_clause_end();
			WAVEFRONT.clause_kind = GPU_CLAUSE_CF;
		}

		break;
	}

	default:
		abort();
	}
}


void gpu_isa_run(struct opencl_kernel_t *kernel)
{
	struct opencl_program_t *program;
	void *code_buffer;
	int i;

	/* Record one more kernel execution and dump report */
	gk_kernel_execution_count++;
	if (gk_report_file) {
		fprintf(gk_report_file, "[ KernelExecution %d ]\n\n", gk_kernel_execution_count - 1);
		fprintf(gk_report_file, "KernelName = %s\n", kernel->name);
		fprintf(gk_report_file, "\n\n");
	}

	/* Get program */
	program = opencl_object_get(OPENCL_OBJ_PROGRAM, kernel->program_id);

	/* Initialize work_items and wavefronts */
	gpu_isa_init_work_items(kernel);

	/* Initialize constant memory */
	gpu_isa_init_const_mem(kernel);

	/* Create local memories */
	gk->local_mem = calloc(kernel->group_count, sizeof(struct mem_t *));
	for (i = 0; i < kernel->group_count; i++) {
		gk->local_mem[i] = mem_create();
		gk->local_mem[i]->safe = 0;
	}


	/* Kernel arguments */
	gpu_isa_init_kernel_args(kernel);

	/* Load kernel code */
	code_buffer = kernel->cal_abi->text_buffer;
	if (!code_buffer)
		fatal("%s: cannot load kernel code", __FUNCTION__);

	/* Initialize wavefronts */
	for (i = 0; i < gpu_wavefront_count; i++) {
		gpu_isa_wavefront = gpu_isa_wavefronts[i];
		WAVEFRONT.cf_buf_start = code_buffer;
		WAVEFRONT.cf_buf = code_buffer;
		WAVEFRONT.clause_kind = GPU_CLAUSE_CF;
		WAVEFRONT.emu_time_start = ke_timer();
	}

	/* Execution loop */
	for (;;) {
		int finished_count = 0;

		for (i = 0; i < gpu_wavefront_count; i++) {
			gpu_isa_wavefront = gpu_isa_wavefronts[i];
			if (WAVEFRONT.clause_kind == GPU_CLAUSE_CF && !WAVEFRONT.cf_buf) {
				finished_count++;
				continue;
			}
			gpu_isa_wavefront_exec(kernel);
		}
		if (finished_count == gpu_wavefront_count)
			break;  /* FIXME: implement with linked lists */
	}

	/* Record finish time */
	WAVEFRONT.emu_time_end = ke_timer();

	/* Dump wavefront report */
	gpu_wavefront_dump(gpu_isa_wavefront, gk_report_file);

	/* Free work-groups */
	for (i = 0; i < kernel->group_count; i++)
		gpu_work_group_free(gpu_isa_work_groups[i]);
	free(gpu_isa_work_groups);

	/* Free wavefronts */
	for (i = 0; i < gpu_wavefront_count; i++)
		gpu_wavefront_free(gpu_isa_wavefronts[i]);
	free(gpu_isa_wavefronts);

	/* Free work-items */
	for (i = 0; i < kernel->global_size; i++)
		gpu_work_item_free(gpu_isa_work_items[i]);
	free(gpu_isa_work_items);

	/* Free local memories */
	for (i = 0; i < kernel->group_count; i++)
		mem_free(gk->local_mem[i]);
	free(gk->local_mem);
}




/*
 * ALU Clauses
 */

/* Called before and ALU clause starts in wavefront */
void gpu_isa_alu_clause_start()
{
	/* Copy 'active' mask at the top of the stack to 'pred' mask */
	bit_map_copy(WAVEFRONT.pred, 0, WAVEFRONT.active_stack,
		WAVEFRONT.stack_top * WAVEFRONT.work_item_count, WAVEFRONT.work_item_count);
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:pred=", WAVEFRONT.name);
		bit_map_dump(WAVEFRONT.pred, 0, WAVEFRONT.work_item_count,
			debug_file(gpu_isa_debug_category));
	}

	/* Flag 'push_before_done' will be set by the first PRED_SET* inst */
	WAVEFRONT.push_before_done = 0;

	/* Stats */
	WAVEFRONT.alu_clause_count++;
}


/* Called after an ALU clause completed in a wavefront */
void gpu_isa_alu_clause_end()
{
	/* If CF inst was ALU_POP_AFTER, pop the stack */
	if (gpu_isa_cf_inst->info->inst == AMD_INST_ALU_POP_AFTER)
		gpu_wavefront_stack_pop(gpu_isa_wavefront, 1);
}




/*
 * TEX Clauses
 */

/* Called before and TEX clause starts in wavefront */
void gpu_isa_tc_clause_start()
{
	/* Stats */
	WAVEFRONT.tc_clause_count++;
}


/* Called after a TEX clause completed in a wavefront */
void gpu_isa_tc_clause_end()
{
}




/*
 * Instruction operands
 */

/* Dump a destination value depending on the format of the destination operand
 * in the current instruction, as specified by its flags. */
void gpu_isa_dest_value_dump(void *pvalue, FILE *f)
{
	if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_INT)
		fprintf(f, "%d", * (int *) pvalue);
	
	else if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_UINT)
		fprintf(f, "0x%x", * (unsigned int *) pvalue);
	
	else if (gpu_isa_inst->info->flags & AMD_INST_FLAG_DST_FLOAT)
		fprintf(f, "%gf", * (float *) pvalue);
	
	else
		fprintf(f, "(0x%x,%gf)", * (unsigned int *) pvalue, * (float *) pvalue);
}


/* Read source GPR */
uint32_t gpu_isa_read_gpr(int gpr, int rel, int chan, int im)
{
	GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 4);
	GPU_PARAM_NOT_SUPPORTED_OOR(gpr, 0, 127);
	GPU_PARAM_NOT_SUPPORTED_NEQ(rel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(im, 0);
	return GPU_GPR_ELEM(gpr, chan);
}


/* Read source GPR in float format */
float gpu_isa_read_gpr_float(int gpr, int rel, int chan, int im)
{
	uint32_t value;
	float value_float;

	value = gpu_isa_read_gpr(gpr, rel, chan, im);
	value_float = * (float *) &value;
	return value_float;
}


void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value)
{
	GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 4);
	GPU_PARAM_NOT_SUPPORTED_OOR(gpr, 0, 127);
	GPU_PARAM_NOT_SUPPORTED_NEQ(rel, 0);
	GPU_GPR_ELEM(gpr, chan) = value;
}


void gpu_isa_write_gpr_float(int gpr, int rel, int chan, float value_float)
{
	uint32_t value;

	value = * (uint32_t *) &value_float;
	gpu_isa_write_gpr(gpr, rel, chan, value);
}


/* Read source operand in ALU instruction.
 * This is a common function for both integer and float formats. */
static uint32_t gpu_isa_read_op_src_common(int src_idx, int *neg_ptr, int *abs_ptr)
{
	int sel, rel, chan;
	int32_t value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	amd_inst_get_op_src(gpu_isa_inst, src_idx, &sel, &rel, &chan, neg_ptr, abs_ptr);

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127)) {
		int index_mode;
		index_mode = gpu_isa_inst->words[0].alu_word0.index_mode;
		value = gpu_isa_read_gpr(sel, rel, chan, index_mode);
		return value;
	}

	/* 128..159: Kcache 0 constant */
	if (IN_RANGE(sel, 128, 159)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_bank0;
		kcache_mode = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_mode0;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr0;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		gpu_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 128, chan, &value);
		return value;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kcache_bank1;
		kcache_mode = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_mode1;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr1;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		gpu_isa_const_mem_read(kcache_bank, kcache_addr * 16 + sel - 160, chan, &value);
		return value;
	}

	/* QA and QA.pop */
	if (sel == 219 || sel == 221) {
		uint32_t *pvalue;
		pvalue = (uint32_t *) list_dequeue(WORK_ITEM.lds_oqa);
		if (!pvalue)
			fatal("%s: LDS queue A is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 219)
			list_enqueue(WORK_ITEM.lds_oqa, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* QB and QB.pop */
	if (sel == 220 || sel == 222) {
		uint32_t *pvalue;
		pvalue = (uint32_t *) list_dequeue(WORK_ITEM.lds_oqb);
		if (!pvalue)
			fatal("%s: LDS queue B is empty", __FUNCTION__);
		value = *pvalue;
		if (sel == 220)
			list_enqueue(WORK_ITEM.lds_oqb, pvalue);
		else
			free(pvalue);
		return value;
	}

	/* ALU_SRC_0 */
	if (sel == 248) {
		value = 0;
		return value;
	}

	/* ALU_SRC_1 */
	if (sel == 249) {
		float f = 1.0f;
		value = * (uint32_t *) &f;
		return value;
	}

	/* ALU_SRC_1_INT */
	if (sel == 250) {
		value = 1;
		return value;
	}

	/* ALU_SRC_M_1_INT */
	if (sel == 251) {
		value = -1;
		return value;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253) {
		assert(gpu_isa_inst->alu_group);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = * (uint32_t *) &gpu_isa_inst->alu_group->literal[chan];
		return value;
	}

	/* ALU_SRC_PV */
	if (sel == 254) {
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = WORK_ITEM.pv.elem[chan];
		return value;
	}

	/* ALU_SRC_PS */
	if (sel == 255) {
		value = WORK_ITEM.pv.elem[4];
		return value;
	}

	/* Not implemented 'sel' field */
	fatal("gpu_isa_read_op_src: src_idx=%d, not implemented for sel=%d", src_idx, sel);
	return 0;
}


uint32_t gpu_isa_read_op_src_int(int src_idx)
{
	int neg, abs;
	int32_t value; /* Signed */

	value = gpu_isa_read_op_src_common(src_idx, &neg, &abs);

	/* Absolute value and negation */
	if (abs && value < 0)
		value = -value;
	if (neg)
		value = -value;
	
	/* Return as unsigned */
	return value;
}


float gpu_isa_read_op_src_float(int src_idx)
{
	uint32_t value;
	float value_float;
	int neg, abs;

	value = gpu_isa_read_op_src_common(src_idx, &neg, &abs);
	value_float = * (float *) &value;

	/* Absolute value and negation */
	if (abs && value_float < 0.0)
		value_float = -value_float;
	if (neg)
		value_float = -value_float;
	return value_float;
}




/*
 * Deferred tasks for ALU group
 */


void gpu_isa_enqueue_write_lds(uint32_t addr, uint32_t value)
{
	struct gpu_isa_write_task_t *wt;

	/* Inactive pixel not enqueued */
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;
	
	/* Create task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_WRITE_LDS;
	wt->inst = gpu_isa_inst;
	wt->lds_addr = addr;
	wt->lds_value = value;

	/* Enqueue task */
	lnlist_add(WORK_ITEM.write_task_list, wt);
}


/* Write to destination operand in ALU instruction */
void gpu_isa_enqueue_write_dest(uint32_t value)
{
	struct gpu_isa_write_task_t *wt;

	/* If pixel is inactive, do not enqueue the task */
	assert(gpu_isa_inst->info->fmt[0] == FMT_ALU_WORD0);
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * ALU_WORD1_OP2 and ALU_WORD1_OP3 formats. */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_WRITE_DEST;
	wt->inst = gpu_isa_inst;
	wt->gpr = ALU_WORD1_OP2.dst_gpr;
	wt->rel = ALU_WORD1_OP2.dst_rel;
	wt->chan = ALU_WORD1_OP2.dst_chan;
	wt->index_mode = ALU_WORD0.index_mode;
	wt->value = value;

	/* For ALU_WORD1_OP2, check 'write_mask' field */
	wt->write_mask = 1;
	if (gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && !ALU_WORD1_OP2.write_mask)
		wt->write_mask = 0;

	/* Enqueue task */
	lnlist_add(WORK_ITEM.write_task_list, wt);
}


void gpu_isa_enqueue_write_dest_float(float value_float)
{
	uint32_t value;

	value = * (uint32_t *) &value_float;
	gpu_isa_enqueue_write_dest(value);
}


void gpu_isa_enqueue_push_before(void)
{
	struct gpu_isa_write_task_t *wt;

	/* Do only if instruction initiating ALU clause is ALU_PUSH_BEFORE */
	if (gpu_isa_cf_inst->info->inst != AMD_INST_ALU_PUSH_BEFORE)
		return;

	/* Create and enqueue task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_PUSH_BEFORE;
	wt->inst = gpu_isa_inst;
	lnlist_add(WORK_ITEM.write_task_list, wt);
}


void gpu_isa_enqueue_pred_set(int cond)
{
	struct gpu_isa_write_task_t *wt;

	/* If pixel is inactive, predicate is not changed */
	assert(gpu_isa_inst->info->fmt[0] == FMT_ALU_WORD0);
	assert(gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2);
	if (!gpu_work_item_get_pred(gpu_isa_work_item))
		return;
	
	/* Create and enqueue task */
	wt = repos_create_object(gpu_isa_write_task_repos);
	wt->kind = GPU_ISA_WRITE_TASK_SET_PRED;
	wt->inst = gpu_isa_inst;
	wt->cond = cond;
	lnlist_add(WORK_ITEM.write_task_list, wt);
}


void gpu_isa_write_task_commit(void)
{
	struct lnlist_t *task_list = WORK_ITEM.write_task_list;
	struct gpu_isa_write_task_t *wt;

	/* Process first tasks of type:
	 *  - GPU_ISA_WRITE_TASK_WRITE_DEST
	 *  - GPU_ISA_WRITE_TASK_WRITE_LDS
	 */
	for (lnlist_head(task_list); !lnlist_eol(task_list); )
	{

		/* Get task */
		wt = lnlist_get(task_list);
		gpu_isa_inst = wt->inst;

		switch (wt->kind) {
		
		case GPU_ISA_WRITE_TASK_WRITE_DEST:
		{
			if (wt->write_mask)
				gpu_isa_write_gpr(wt->gpr, wt->rel, wt->chan, wt->value);
			WORK_ITEM.pv.elem[wt->inst->alu] = wt->value;

			/* Debug */
			if (gpu_isa_debugging()) {
				gpu_isa_debug("  i%d:%s", WORK_ITEM.global_id,
					map_value(&amd_pv_map, wt->inst->alu));
				if (wt->write_mask) {
					gpu_isa_debug(",");
					amd_inst_dump_gpr(wt->gpr, wt->rel, wt->chan, 0,
						debug_file(gpu_isa_debug_category));
				}
				gpu_isa_debug("<=");
				gpu_isa_dest_value_dump(&wt->value, debug_file(gpu_isa_debug_category));
			}

			break;
		}

		case GPU_ISA_WRITE_TASK_WRITE_LDS:
		{
			struct mem_t *local_mem;

			local_mem = gk->local_mem[WORK_ITEM.group_id];
			assert(local_mem);
			mem_write(local_mem, wt->lds_addr, 4, &wt->lds_value);
			gpu_isa_debug("  i%d:LDS[0x%x]<=(%u,%gf)", WORK_ITEM.global_id, wt->lds_addr,
				wt->lds_value, * (float *) &wt->lds_value);
			break;
		}

		default:
			lnlist_next(task_list);
			continue;
		}

		/* Done with this task */
		repos_free_object(gpu_isa_write_task_repos, wt);
		lnlist_remove(task_list);
	}

	/* Process PUSH_BEFORE, PRED_SET */
	for (lnlist_head(task_list); !lnlist_eol(task_list); ) {
		
		/* Get task */
		wt = lnlist_get(task_list);
		gpu_isa_inst = wt->inst;

		/* Process */
		switch (wt->kind) {

		case GPU_ISA_WRITE_TASK_PUSH_BEFORE:
		{
			if (!WAVEFRONT.push_before_done)
				gpu_wavefront_stack_push(gpu_isa_wavefront);
			WAVEFRONT.push_before_done = 1;
			break;
		}

		case GPU_ISA_WRITE_TASK_SET_PRED:
		{
			int update_pred = ALU_WORD1_OP2.update_pred;
			int update_exec_mask = ALU_WORD1_OP2.update_exec_mask;

			assert(gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2);
			if (update_pred)
				gpu_work_item_set_pred(gpu_isa_work_item, wt->cond);
			if (update_exec_mask)
				gpu_work_item_set_active(gpu_isa_work_item, wt->cond);

			/* Debug */
			if (debug_status(gpu_isa_debug_category)) {
				if (update_pred && update_exec_mask)
					gpu_isa_debug("  i%d:act/pred<=%d", WORK_ITEM.global_id, wt->cond);
				else if (update_pred)
					gpu_isa_debug("  i%d:pred=%d", WORK_ITEM.global_id, wt->cond);
				else if (update_exec_mask)
					gpu_isa_debug("  i%d:pred=%d", WORK_ITEM.global_id, wt->cond);
			}
			break;
		}

		default:
			abort();
		}
		
		/* Done with task */
		repos_free_object(gpu_isa_write_task_repos, wt);
		lnlist_remove(task_list);
	}

	/* List should be empty */
	assert(!lnlist_count(task_list));
}

