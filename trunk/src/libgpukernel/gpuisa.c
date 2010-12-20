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

/* Some globals */
struct gpu_thread_t *gpu_isa_thread;  /* Active thread */
struct gpu_thread_t **gpu_isa_threads;  /* Array of kernel threads */
struct amd_inst_t *gpu_isa_inst;  /* Current instruction */
struct amd_inst_t *gpu_isa_cf_inst;  /* Current CF instruction */
struct amd_alu_group_t *gpu_isa_alu_group;  /* Current ALU group */


/* Instruction execution table */
amd_inst_impl_t *amd_inst_impl;


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
}


void gpu_isa_done()
{
	/* Instruction execution table */
	free(amd_inst_impl);
}


void gpu_isa_run(struct opencl_kernel_t *kernel)
{
	struct opencl_program_t *program;
	uint32_t cf_ip, clause_ip;

	void *cf_buf;
	struct amd_inst_t cf_inst;

	void *alu_buf, *alu_buf_end;
	struct amd_alu_group_t alu_group;
	int alu_group_count;

	int i, x, y, z;
	int global_id;

	/* Get program */
	program = opencl_object_get(OPENCL_OBJ_PROGRAM, kernel->program_id);

	/* Create threads */
	gpu_isa_threads = calloc(kernel->global_size, sizeof(void *));
	for (x = 0; x < kernel->global_size3[0]; x++) {
		for (y = 0; y < kernel->global_size3[1]; y++) {
			for (z = 0; z < kernel->global_size3[2]; z++) {
				global_id = z * kernel->global_size3[1] * kernel->global_size3[0]
					+ y * kernel->global_size3[0] + x;
				gpu_isa_threads[global_id] = gpu_thread_create();
				gpu_isa_thread = gpu_isa_threads[global_id];

				GPU_THR.global_id3[0] = x;
				GPU_THR.global_id3[1] = y;
				GPU_THR.global_id3[2] = z;
				GPU_THR.global_id = global_id;

				GPU_THR.group_id3[0] = GPU_GPR_X(1) = x / kernel->local_size3[0];
				GPU_THR.group_id3[1] = GPU_GPR_Y(1) = y / kernel->local_size3[1];
				GPU_THR.group_id3[2] = GPU_GPR_Z(1) = z / kernel->local_size3[2];
				GPU_THR.group_id = GPU_THR.group_id3[2] * kernel->group_count3[1] * kernel->group_count3[0]
					+ GPU_THR.group_id3[1] * kernel->group_count3[0]
					+ GPU_THR.group_id3[0];

				GPU_THR.local_id3[0] = GPU_GPR_X(0) = x % kernel->local_size3[0];
				GPU_THR.local_id3[1] = GPU_GPR_Y(0) = y % kernel->local_size3[1];
				GPU_THR.local_id3[2] = GPU_GPR_Z(0) = z % kernel->local_size3[2];
				GPU_THR.local_id = GPU_THR.local_id3[2] * kernel->local_size3[1] * kernel->local_size3[0]
					+ GPU_THR.local_id3[1] * kernel->local_size3[0]
					+ GPU_THR.local_id3[0];
			}
		}
	}

	/* Initialize constant memory */
	mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(0, 0, 3), 4, &kernel->work_dim);  /* CB0[0].w */
	mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(0, 0, 0), 12, kernel->global_size3);  /* CB0[0].{x,y,z} */
	mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(0, 1, 0), 12, kernel->local_size3);  /* CB0[1].{x,y,z} */
	mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(0, 2, 0), 12, kernel->group_count3);  /* CB0[2].{x,y,z} */

	/* Kernel arguments */
	for (i = 0; i < list_count(kernel->arg_list); i++) {

		struct opencl_kernel_arg_t *arg;
		arg = list_get(kernel->arg_list, i);
		assert(arg);

		switch (arg->kind) {

		case OPENCL_KERNEL_ARG_KIND_VALUE: {
			
			/* Value copied directly into device constant memory */
			mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(1, i, 0), 4, &arg->value);
			opencl_debug("    arg %d: value '0x%x' loaded\n", i, arg->value);
			break;
		}

		case OPENCL_KERNEL_ARG_KIND_POINTER: {
			struct opencl_mem_t *mem;

			/* Argument value is a pointer to an 'opencl_mem' object.
			 * It is translated first into a device memory pointer. */
			mem = opencl_object_get(OPENCL_OBJ_MEM, arg->value);
			mem_write(gk->const_mem, GPU_CONST_MEM_ADDR(1, i, 0), 4, &mem->device_ptr);
			opencl_debug("    arg %d: opencl_mem id 0x%x loaded, device_ptr=0x%x\n",
				i, arg->value, mem->device_ptr);
			break;
		}

		default:
			fatal("%s: argument type not recognized", __FUNCTION__);
		}
	}
	
	/* Execution loop */
	cf_ip = 0;
	clause_ip = 0;
	cf_buf = program->code;
	while (cf_buf) {
		
		/* Decode CF instruction */
		cf_buf = amd_inst_decode_cf(cf_buf, &cf_inst);
		amd_inst_dump(&cf_inst, 0, 0, stdout);

		/* Execute in all threads */
		gpu_isa_inst = gpu_isa_cf_inst = &cf_inst;
		for (global_id = 0; global_id < kernel->global_size; global_id++) {
			gpu_isa_thread = gpu_isa_threads[global_id];
			(*amd_inst_impl[gpu_isa_inst->info->inst])();
		}
		
		/* ALU clause */
		if (cf_inst.info->fmt[0] == FMT_CF_ALU_WORD0) {

			alu_buf = program->code + cf_inst.words[0].cf_alu_word0.addr * 8;
			alu_buf_end = alu_buf + (cf_inst.words[1].cf_alu_word1.count + 1) * 8;
			alu_group_count = 0;
			assert(IN_RANGE(alu_buf, program->code, program->code + program->code_size)
				&& IN_RANGE(alu_buf_end, program->code, program->code + program->code_size));
			while (alu_buf < alu_buf_end) {
				
				/* Decode ALU group */
				alu_buf = amd_inst_decode_alu_group(alu_buf, alu_group_count, &alu_group);
				amd_alu_group_dump(&alu_group, 0, stdout);
				alu_group_count++;

				/* Execute group in all threads */
				gpu_isa_alu_group = &alu_group;
				for (global_id = 0; global_id < kernel->global_size; global_id++) {
					gpu_isa_thread = gpu_isa_threads[global_id];
					for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
						gpu_isa_inst = &gpu_isa_alu_group->inst[i];
						(*amd_inst_impl[gpu_isa_inst->info->inst])();
					}
					gpu_alu_group_commit();
				}
			}
		}

		/* Fetch through a Texture Cache Clause */
		if (cf_inst.info->inst == AMD_INST_TC) {
			
			void *tex_buf, *tex_buf_end;
			struct amd_inst_t tex_inst;

			tex_buf = program->code + cf_inst.words[0].cf_word0.addr * 8;
			tex_buf_end = tex_buf + (cf_inst.words[1].cf_word1.count + 1) * 16;
			assert(IN_RANGE(tex_buf, program->code, program->code + program->code_size)
				&& IN_RANGE(tex_buf_end, program->code, program->code + program->code_size));
			while (tex_buf < tex_buf_end) {
				
				/* Decode TEX inst */
				tex_buf = amd_inst_decode_tc(tex_buf, &tex_inst);
				amd_inst_dump(&tex_inst, 0, 0, stdout);

				/* Execute in all threads */
				gpu_isa_inst = &tex_inst;
				for (global_id = 0; global_id < kernel->global_size; global_id++) {
					gpu_isa_thread = gpu_isa_threads[global_id];
					(*amd_inst_impl[gpu_isa_inst->info->inst])();
				}
			}
		}
	}
	
	/* Free threads */
	for (i = 0; i < kernel->global_size; i++)
		gpu_thread_free(gpu_isa_threads[i]);
	free(gpu_isa_threads);
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


void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value)
{
	GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 4);
	GPU_PARAM_NOT_SUPPORTED_OOR(gpr, 0, 127);
	GPU_PARAM_NOT_SUPPORTED_NEQ(rel, 0);
	GPU_GPR_ELEM(gpr, chan) = value;
	//printf("thread %d: GPR(%d).%d set to %d\n", gpu_isa_thread->global_id, gpr, chan, value), fflush(stdout); ////
}


/* Read source operand in ALU instruction */
uint32_t gpu_isa_read_op_src(int src_idx)
{
	int sel, rel, chan, neg, abs;
	int32_t value = 0;  /* Signed, for negative constants and abs operations */

	/* Get the source operand parameters */
	amd_inst_get_op_src(gpu_isa_inst, src_idx, &sel, &rel, &chan, &neg, &abs);

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127)) {
		int im;  /* index_mode */
		im = gpu_isa_inst->words[0].alu_word0.im;
		value = gpu_isa_read_gpr(sel, rel, chan, im);
		goto end;
	}

	/* 128..159: Kcache 0 constant */
	if (IN_RANGE(sel, 128, 159)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;
		uint32_t addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kb0;
		kcache_mode = gpu_isa_cf_inst->words[0].cf_alu_word0.km0;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr0;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		addr = GPU_CONST_MEM_ADDR(kcache_bank, kcache_addr * 16 + sel - 128, chan);
		mem_read(gk->const_mem, addr, 4, &value);
		goto end;
	}

	/* 160..191: Kcache 1 constant */
	if (IN_RANGE(sel, 160, 191)) {

		uint32_t kcache_bank, kcache_mode, kcache_addr;
		uint32_t addr;

		assert(gpu_isa_cf_inst->info->fmt[0] == FMT_CF_ALU_WORD0 && gpu_isa_cf_inst->info->fmt[1] == FMT_CF_ALU_WORD1);
		kcache_bank = gpu_isa_cf_inst->words[0].cf_alu_word0.kb1;
		kcache_mode = gpu_isa_cf_inst->words[1].cf_alu_word1.km1;
		kcache_addr = gpu_isa_cf_inst->words[1].cf_alu_word1.kcache_addr1;

		GPU_PARAM_NOT_SUPPORTED_NEQ(kcache_mode, 1);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		addr = GPU_CONST_MEM_ADDR(kcache_bank, kcache_addr * 16 + sel - 160, chan);
		mem_read(gk->const_mem, addr, 4, &value);
		goto end;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253) {
		assert(gpu_isa_inst->alu_group);
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = * (uint32_t *) &gpu_isa_inst->alu_group->literal[chan];
		goto end;
	}

	/* ALU_SRC_PV */
	if (sel == 254) {
		GPU_PARAM_NOT_SUPPORTED_OOR(chan, 0, 3);
		value = GPU_THR.pv.elem[chan];
		goto end;
	}

	/* ALU_SRC_PS */
	if (sel == 255) {
		value = GPU_THR.pv.elem[4];
		goto end;
	}

	/* ALU_SRC_1_INT */
	if (sel == 250) {
		value = 1;
		goto end;
	}

	/* ALU_SRC_M_1_INT */
	if (sel == 251) {
		value = -1;
		goto end;
	}

	fatal("gpu_isa_read_op_src: src_idx=%d, not implemented for sel=%d", src_idx, sel);

end:
	/* Absolute value and negation */
	if (abs && value < 0)
		value = -value;
	if (neg)
		value = -value;
	return value;
}


/* Write to dest operand in ALU instruction */
#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void gpu_isa_write_op_dst(uint32_t value)
{
	struct gpu_write_task_t *wt;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * ALU_WORD1_OP2 and ALU_WORD1_OP3 formats. */
	assert(gpu_isa_inst->info->fmt[0] == FMT_ALU_WORD0);
	assert(gpu_isa_thread->write_task_count < GPU_MAX_WRITE_TASKS);
	wt = &gpu_isa_thread->write_tasks[gpu_isa_thread->write_task_count];
	wt->alu = gpu_isa_inst->alu;
	wt->gpr = W1.dst_gpr;
	wt->rel = W1.dr;
	wt->chan = W1.dc;
	wt->im = W0.im;
	wt->value = value;

	/* For ALU_WORD1_OP2, check 'write_mask' field */
	wt->wm = 1;
	if (gpu_isa_inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && !gpu_isa_inst->words[1].alu_word1_op2.wm)
		wt->wm = 0;

	/* One more write task */
	gpu_isa_thread->write_task_count++;
}
#undef W0
#undef W1


void gpu_alu_group_commit(void)
{
	int i;
	struct gpu_write_task_t *wt;

	for (i = 0; i < gpu_isa_thread->write_task_count; i++) {
		wt = &gpu_isa_thread->write_tasks[i];
		if (wt->wm)
			gpu_isa_write_gpr(wt->gpr, wt->rel, wt->chan, wt->value);
		GPU_THR.pv.elem[wt->alu] = wt->value;
		//printf("thread %d: PV.%d set to %d\n", gpu_isa_thread->global_id, wt->chan, wt->value), fflush(stdout); ////
	}
	gpu_isa_thread->write_task_count = 0;
}

