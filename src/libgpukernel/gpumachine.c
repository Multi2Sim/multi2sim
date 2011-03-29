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
#include <math.h>

char *err_gpu_machine_note =
	"\tThe AMD Evergreen instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	gpu_isa_inst->info->name, err_gpu_machine_note)


/*
 * CF Instructions
 */

#define W0  CF_ALU_WORD0
#define W1  CF_ALU_WORD1
void amd_inst_ALU_impl()
{
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.alt_const, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	/* FIXME: block constant cache sets */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */

	/* Start ALU clause */
	gpu_isa_warp->clause_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
	gpu_isa_warp->clause_buf_end = gpu_isa_warp->clause_buf + (W1.count + 1) * 8;
	gpu_isa_warp->clause_kind = GPU_CLAUSE_ALU;
	gpu_isa_alu_clause_start();
}
#undef W0
#undef W1


void amd_inst_ALU_BREAK_impl()
{
	/* Same behavior as ALU */
	/* FIXME: what's the difference? */
	amd_inst_ALU_impl();
}


void amd_inst_ALU_POP_AFTER_impl()
{
	/* Same behavior as ALU */
	amd_inst_ALU_impl();
}


void amd_inst_ALU_PUSH_BEFORE_impl()
{
	/* Same behavior as ALU inst */
	amd_inst_ALU_impl();
}


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_ELSE_impl()
{
	int active, active_last, active_new;
	int active_count = 0;
	int i;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.count, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* Debug */
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:act=", gpu_isa_warp->name);
		bit_map_dump(gpu_isa_warp->active_stack, gpu_isa_warp->stack_top *
			gpu_isa_warp->thread_count, gpu_isa_warp->thread_count,
			debug_file(gpu_isa_debug_category));
	}

	/* Invert active mask */
	if (!gpu_isa_warp->stack_top)
		fatal("ELSE: cannot execute for stack_top=0");
	for (i = 0; i < gpu_isa_warp->thread_count; i++) {
		active = bit_map_get(gpu_isa_warp->active_stack, gpu_isa_warp->stack_top *
			gpu_isa_warp->thread_count + i, 1);
		active_last = bit_map_get(gpu_isa_warp->active_stack, (gpu_isa_warp->stack_top - 1) *
			gpu_isa_warp->thread_count + i, 1);
		active_new = !active && active_last;
		active_count += active_new;
		bit_map_set(gpu_isa_warp->active_stack, gpu_isa_warp->stack_top *
			gpu_isa_warp->thread_count + i, 1, active_new);
	}
	
	/* Debug */
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:invert(act)=", gpu_isa_warp->name);
		bit_map_dump(gpu_isa_warp->active_stack, gpu_isa_warp->stack_top *
			gpu_isa_warp->thread_count, gpu_isa_warp->thread_count,
			debug_file(gpu_isa_debug_category));
	}

	/* If all pixels are inactive, pop stack and jump */
	if (!active_count) {
		gpu_warp_stack_pop(gpu_isa_warp, W1.pop_count);
		gpu_isa_warp->cf_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_JUMP_impl()
{
	int active_count;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.count, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* If all pixels are inactive, pop stack and jump */
	active_count = bit_map_count_ones(gpu_isa_warp->active_stack,
		gpu_isa_warp->stack_top * gpu_isa_warp->thread_count, gpu_isa_warp->thread_count);
	if (!active_count) {
		gpu_warp_stack_pop(gpu_isa_warp, W1.pop_count);
		gpu_isa_warp->cf_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_LOOP_END_impl()
{
	int active_count;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.pop_count, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if any pixel is active */

	/* FIXME: Update loop state and check if index is 0 */

	/* Dump current loop state */
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:act=", gpu_isa_warp->name);
		bit_map_dump(gpu_isa_warp->active_stack, gpu_isa_warp->stack_top *
			gpu_isa_warp->thread_count, gpu_isa_warp->thread_count,
			debug_file(gpu_isa_debug_category));
	}

	/* If any pixel is active, jump back */
	active_count = bit_map_count_ones(gpu_isa_warp->active_stack,
		gpu_isa_warp->stack_top * gpu_isa_warp->thread_count, gpu_isa_warp->thread_count);
	if (active_count) {
		gpu_isa_warp->cf_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
		return;
	}

	/* FIXME: pop loop state */

	/* Pop stack once */
	gpu_warp_stack_pop(gpu_isa_warp, 1);
}
#undef W0
#undef W1


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_LOOP_START_DX10_impl()
{
	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cf_const, 0);  /* FIXME: what does it refer to? */
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.count, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if all pixels fail. */
	/* W0.pop_count: pop if all pixels fail. */
	/* FIXME: W1.cf_const: used to set up initial loop state... but how? */

	/* FIXME: if initial condition fails, jump to 'addr' */

	/* FIXME: Push loop state */

	/* FIXME: Set up new loop state */

	/* FIXME: Push active mask? */
	gpu_warp_stack_push(gpu_isa_warp);///
}
#undef W0
#undef W1


#define W0  CF_ALLOC_EXPORT_WORD0_RAT
#define W1  CF_ALLOC_EXPORT_WORD1_BUF
void amd_inst_MEM_RAT_CACHELESS_impl()
{
	int global_id;

	switch (W0.rat_inst) {

	/* STORE_RAW */
	case 2: {
		uint32_t value, addr;
		float value_float;
		int i;

		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.rat_id, 1);  /* FIXME: what does rat_id mean? */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.rat_index_mode, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.elem_size, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.burst_count, 0);

		/* Values for W0.type:
		 *   0  EXPORT_WRITE: Write to the memory buffer.
		 *   1  EXPORT_WRITE_IND: write to memory buffer, use offset in INDEX_GPR.
		 *   2  EXPORT_WRITE_ACK: write to memory buffer, request an ACK when write is
		 *      committed to memory. For UAV, ACK guarantees return value has been writ-
		 *      ten to memory.
		 *   3  EXPORT_WRITE_IND_ACK: write to memory buffer with
		 *      offset in INDEX_GPR, get an ACK when done. For UAV, ACK guarantees return
		 *      value has been written to memory.
		 *      FIXME: What is the ACK?
		 */
		/* Only 1 and 3 supported */
		if (W0.type != 1 && W0.type != 3)
			GPU_PARAM_NOT_SUPPORTED(W0.type);


		for (global_id = gpu_isa_warp->global_id; global_id < gpu_isa_warp->global_id
			+ gpu_isa_warp->thread_count; global_id++)
		{
			gpu_isa_thread = gpu_isa_threads[global_id];

			/* If VPM is set, do not export for inactive pixels. */
			if (W1.valid_pixel_mode && !gpu_thread_get_active(gpu_isa_thread))
				continue;

			/* W0.rw_gpr: GPR register from which to read data */
			/* W0.rw_rel: relative/absolute rw_gpr */
			/* W0.index_gpr: GPR containing buffer coordinates. It is multiplied by (elem_size+1) */
			/* W0.elem_size: number of doublewords per array element, minus one */
			/* W1.array_size: array size (elem-size units) */

			addr = gpu_isa_read_gpr(W0.index_gpr, 0, 0, 0) * 4;  /* FIXME: only 1D - X coordinate, FIXME: x4? */
			gpu_isa_debug("  t%d:write(0x%x)", GPU_THR.global_id, addr);

			for (i = 0; i < 4; i++) {
				if (!(W1.comp_mask & (1 << i)))
					continue;
				value = gpu_isa_read_gpr(W0.rw_gpr, W0.rr, i, 0);
				value_float = * (float *) &value;
				/* FIXME: leave gaps when intermediate 'comp_mask' bits are not set? */
				mem_write(gk->global_mem, addr + i * 4, 4, &value);
				gpu_isa_debug(",");
				if (debug_status(gpu_isa_debug_category))
					amd_inst_dump_gpr(W0.rw_gpr, W0.rr, i, 0, debug_file(gpu_isa_debug_category));
				gpu_isa_debug("=(0x%x,%gf)", value, value_float);
			}
			/* FIXME: array_size: ignored now, because 'burst_count' = 0 */
			/* FIXME: rat_id */
			/* FIXME: mark - mark memory write to be acknowledged by the next write-ack */
			/* FIXME: barrier */
		}
		break;
	}

	default:
		GPU_PARAM_NOT_SUPPORTED(W0.rat_inst);
	}
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].cf_word0
#define W1 gpu_isa_inst->words[1].cf_word1
void amd_inst_NOP_CF_impl()
{
}
#undef W0
#undef W1


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_POP_impl()
{
	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.count, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* Pop 'pop_count' from stack and jump to 'addr' */
	gpu_warp_stack_pop(gpu_isa_warp, W1.pop_count);
	gpu_isa_warp->cf_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
}
#undef W0
#undef W1


#define W0  CF_WORD0
#define W1  CF_WORD1
void amd_inst_TC_impl()
{
	int active;
	int i;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);  /* ignored for this instruction */
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.pop_count, 0);  /* number of entries to pop from stack */
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.cond, 0);  /* how to evaluate condition test for pixels (ACTIVE) */
	/* FIXME: valid_pixel_mode */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */
	
	/* Start TC clause */
	gpu_isa_warp->clause_buf = gpu_isa_warp->cf_buf_start + W0.addr * 8;
	gpu_isa_warp->clause_buf_end = gpu_isa_warp->clause_buf + (W1.count + 1) * 16;
	gpu_isa_warp->clause_kind = GPU_CLAUSE_TC;
	gpu_isa_tc_clause_start();

	/* If VPM is set, copy 'active' mask at the top of the stack to 'pred' mask.
	 * This will make all fetches within the clause happen only for active pixels.
	 * If VPM is clear, copy a mask set to ones. */
	for (i = 0; i < gpu_isa_warp->thread_count; i++) {
		active = W1.valid_pixel_mode ? bit_map_get(gpu_isa_warp->active_stack,
			gpu_isa_warp->stack_top * gpu_isa_warp->thread_count + i, 1) : 1;
		bit_map_set(gpu_isa_warp->pred, i, 1, active);
	}
}
#undef W0
#undef W1


void amd_inst_WAIT_ACK_impl()
{
	/* FIXME: wait for Write ACKs */
}


void amd_inst_ADD_impl()
{
	float src0, src1, dst;

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	dst = src0 + src1;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_MUL_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_IEEE_impl()
{
	float src0, src1, dst;

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	dst = src0 * src1;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_MAX_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_DX10_impl()
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	dst = src0 > src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_MIN_DX10_impl()
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	dst = src0 < src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_SETE_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_DX10_impl() {
	NOT_IMPL();
}


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETGT_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


void amd_inst_SETGE_DX10_impl() {
	NOT_IMPL();
}


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETNE_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


void amd_inst_FRACT_impl() {
	NOT_IMPL();
}


void amd_inst_TRUNC_impl()
{
	float src, dst;

	src = gpu_isa_read_op_src_float(0);
	dst = truncf(src);
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_CEIL_impl() {
	NOT_IMPL();
}


void amd_inst_RNDNE_impl() {
	NOT_IMPL();
}


void amd_inst_FLOOR_impl() {
	NOT_IMPL();
}


void amd_inst_ASHR_INT_impl()
{
	int32_t src0, dst;
	uint32_t src1;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	if (src1 > 31)
		dst = src0 < 0 ? -1 : 0;
	else
		dst = src0 >> src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_LSHR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 >> src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_LSHL_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 << src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MOV_impl()
{
	uint32_t value;
	value = gpu_isa_read_op_src(0);
	gpu_isa_enqueue_write_dest(value);
}


void amd_inst_NOP_ALU_impl()
{
}


void amd_inst_MUL_64_impl() {
	NOT_IMPL();
}


void amd_inst_FLT64_TO_FLT32_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_INV_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_POP_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_CLR_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_RESTORE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_KILLE_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_impl() {
	NOT_IMPL();
}


void amd_inst_KILLNE_impl() {
	NOT_IMPL();
}


void amd_inst_AND_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 & src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_OR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 | src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_XOR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 ^ src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_NOT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 + src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_SUB_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 - src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MAX_INT_impl()
{
	int32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 > src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MIN_INT_impl()
{
	int32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 < src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MAX_UINT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 > src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MIN_UINT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 < src1 ? src0 : src1;
	gpu_isa_enqueue_write_dest(dst);
}


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETE_INT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 == src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETGT_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETGE_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETNE_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1




#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETGT_UINT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_SETGE_UINT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	gpu_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


void amd_inst_KILLGT_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_UINT_impl() {
	NOT_IMPL();
}


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_PREDE_INT_impl()
{
	int32_t src0, src1;
	float dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 == src1;
	dst = cond ? 0.0f : 1.0f;
	gpu_isa_enqueue_write_dest(* (uint32_t *) &dst);

	/* Active masks */
	gpu_isa_enqueue_push_before();
	gpu_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


void amd_inst_PRED_SETGE_INT_impl() {
	NOT_IMPL();
}


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_PRED_SETGT_INT_impl()
{
	int32_t src0, src1;
	float dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 > src1;
	dst = cond ? 0.0f : 1.0f;
	gpu_isa_enqueue_write_dest(* (uint32_t *) &dst);

	/* Active masks */
	gpu_isa_enqueue_push_before();
	gpu_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 gpu_isa_inst->words[0].alu_word0
#define W1 gpu_isa_inst->words[1].alu_word1_op2
void amd_inst_PRED_SETNE_INT_impl()
{
	int32_t src0, src1;
	float dst;
	int cond;

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.omod, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	cond = src0 != src1;
	dst = cond ? 0.0f : 1.0f;
	gpu_isa_enqueue_write_dest(* (uint32_t *) &dst);

	/* Active masks */
	gpu_isa_enqueue_push_before();
	gpu_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


void amd_inst_KILLE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLNE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETLT_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETLE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_impl()
{
	float src;
	int32_t dst;
	
	src = gpu_isa_read_op_src_float(0);
	if (isinf(src) == 1)
		dst = INT32_MAX;
	else if (isinf(src) == -1)
		dst = INT32_MIN;
	else if (isnan(src))
		dst = 0;
	else
		dst = src;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_BFREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_ADDC_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SUBB_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_GROUP_BARRIER_impl() {
	/* FIXME */
	//printf("thread %d - BARRIER\n", gpu_isa_thread->global_id);
}


void amd_inst_GROUP_SEQ_BEGIN_impl() {
	NOT_IMPL();
}


void amd_inst_GROUP_SEQ_END_impl() {
	NOT_IMPL();
}


void amd_inst_SET_MODE_impl() {
	NOT_IMPL();
}


void amd_inst_SET_CF_IDX0_impl() {
	NOT_IMPL();
}


void amd_inst_SET_CF_IDX1_impl() {
	NOT_IMPL();
}


void amd_inst_SET_LDS_SIZE_impl() {
	NOT_IMPL();
}


void amd_inst_EXP_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_LOG_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_LOG_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_FF_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_IEEE_impl()
{
	float src, dst;

	src = gpu_isa_read_op_src_float(0);
	dst = 1.0f / src;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_RECIPSQRT_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_FF_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_SQRT_IEEE_impl()
{
	float src, dst;

	src = gpu_isa_read_op_src_float(0);
	dst = sqrtf(src);
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_SIN_impl() {
	NOT_IMPL();
}


void amd_inst_COS_impl() {
	NOT_IMPL();
}


void amd_inst_MULLO_INT_impl()
{
	int64_t src0, src1, dst;

	src0 = (int32_t) gpu_isa_read_op_src(0);
	src1 = (int32_t) gpu_isa_read_op_src(1);
	dst = src0 * src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MULHI_INT_impl()
{
	int64_t src0, src1, dst;

	src0 = (int32_t) gpu_isa_read_op_src(0);
	src1 = (int32_t) gpu_isa_read_op_src(1);
	dst = src0 * src1;
	gpu_isa_enqueue_write_dest(dst >> 32);
}


void amd_inst_MULLO_UINT_impl()
{
	uint64_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 * src1;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_MULHI_UINT_impl()
{
	uint64_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 * src1;
	gpu_isa_enqueue_write_dest(dst >> 32);
}


void amd_inst_RECIP_INT_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_UINT_impl()
{
	uint32_t src;
	uint32_t dst;

	src = gpu_isa_read_op_src(0);
	dst = 0xffffffff / src;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_RECIP_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_CLAMPED_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_CLAMPED_64_impl() {
	NOT_IMPL();
}


void amd_inst_SQRT_64_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_UINT_impl()
{
	float src;
	uint32_t dst;
	
	src = gpu_isa_read_op_src_float(0);
	if (isinf(src) == 1)
		dst = UINT32_MAX;
	else if (isinf(src) == -1 || isnan(src))
		dst = 0;
	else
		dst = src;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_INT_TO_FLT_impl()
{
	int32_t src0;
	float dst;

	src0 = gpu_isa_read_op_src(0);
	dst = (float) src0;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_UINT_TO_FLT_impl()
{
	uint32_t src0;
	float dst;

	src0 = gpu_isa_read_op_src(0);
	dst = (float) src0;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_BFM_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT16_impl() {
	NOT_IMPL();
}


void amd_inst_FLT16_TO_FLT32_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE0_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE1_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE2_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE3_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_BCNT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBH_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBL_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_UINT4_impl() {
	NOT_IMPL();
}


void amd_inst_DOT_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_RPI_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_FLOOR_impl() {
	NOT_IMPL();
}


void amd_inst_MULHI_UINT24_impl() {
	NOT_IMPL();
}


void amd_inst_MBCNT_32HI_INT_impl() {
	NOT_IMPL();
}


void amd_inst_OFFSET_TO_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_UINT24_impl() {
	NOT_IMPL();
}


void amd_inst_BCNT_ACCUM_PREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MBCNT_32LO_ACCUM_PREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_64_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_64_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_64_impl() {
	NOT_IMPL();
}


void amd_inst_DOT4_impl() {
	NOT_IMPL();
}


void amd_inst_DOT4_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_CUBE_impl() {
	NOT_IMPL();
}


void amd_inst_MAX4_impl() {
	NOT_IMPL();
}


void amd_inst_FREXP_64_impl() {
	NOT_IMPL();
}


void amd_inst_LDEXP_64_impl() {
	NOT_IMPL();
}


void amd_inst_FRACT_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_64_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_64_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_64_impl() {
	NOT_IMPL();
}


void amd_inst_MOVA_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT64_TO_FLT32_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT64_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_PREV_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_DOT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_IEEE_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_PREV_impl()
{
	NOT_IMPL();
}


void amd_inst_MULADD_IEEE_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_XY_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_ZW_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_X_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_Z_impl() {
	NOT_IMPL();
}


void amd_inst_STORE_FLAGS_impl() {
	NOT_IMPL();
}


void amd_inst_LOAD_STORE_FLAGS_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_1A_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_1A1D_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_2A_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P0_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P10_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P20_impl() {
	NOT_IMPL();
}


void amd_inst_BFE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_BFE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_BFI_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FMA_impl() {
	NOT_IMPL();
}


void amd_inst_CNDNE_64_impl() {
	NOT_IMPL();
}


void amd_inst_FMA_64_impl() {
	NOT_IMPL();
}


void amd_inst_LERP_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_BIT_ALIGN_INT_impl() {
	NOT_IMPL();
}


void amd_inst_BYTE_ALIGN_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_HI_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_UINT24_impl() {
	NOT_IMPL();
}


#define W0 gpu_isa_inst->words[0].alu_word0_lds_idx_op
#define W1 gpu_isa_inst->words[1].alu_word1_lds_idx_op
void amd_inst_LDS_IDX_OP_impl()
{
	struct mem_t *local_mem;
	unsigned int idx_offset;
	uint32_t op0, op1, op2;

	/* Get local memory */
	local_mem = gk->local_mem[gpu_isa_thread->group_id];
	assert(local_mem);

	/* Recompose 'idx_offset' field */
	idx_offset = (W0.idx_offset_5 << 5) | (W0.idx_offset_4 << 4) |
		(W1.idx_offset_3 << 3) | (W1.idx_offset_2 << 2) |
		(W1.idx_offset_1 << 1) | W1.idx_offset_0;
	
	/* Read operands */
	op0 = gpu_isa_read_op_src(0);
	op1 = gpu_isa_read_op_src(1);
	op2 = gpu_isa_read_op_src(2);

	/* Process LDS instruction */
	switch (W1.lds_op) {

	/* DS_INST_WRITE: 1A1D WRITE(dst,src) : DS(dst) = src */
	case 13:
	{
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(op2, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(idx_offset, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_chan, 0);
		/* FIXME: dst_chan? Does address need to be multiplied? */
		gpu_isa_enqueue_write_lds(op0, op1);
		break;
	}


	/* DS_INST_WRITE_REL: 1A2D WRITEREL(dst,src0,src1)
	 *   tmp = dst + DS_idx_offset (offset in dwords)
	 *   DS(dst) = src0
	 *   DS(tmp) = src1 */
	case 14:
	{
		uint32_t dst, src0, src1, tmp;

		dst = op0;
		src0 = op1;
		src1 = op2;
		tmp = dst + idx_offset * 4;

		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_chan, 0);
		gpu_isa_enqueue_write_lds(dst, src0);
		gpu_isa_enqueue_write_lds(tmp, src1);  /* FIXME: correct? */
		break;
	}


	/* DS_INST_READ_RET: 1A READ(dst) : OQA = DS(dst) */
	case 50:
	{
		uint32_t *pvalue;

		pvalue = malloc(4);
		mem_read(local_mem, op0, 4, pvalue);
		list_enqueue(gpu_isa_thread->lds_oqa, pvalue);
		gpu_isa_debug("  t%d:LDS[0x%x]=(%u,%gf)=>OQA", GPU_THR.global_id, op0, *pvalue, * (float *) pvalue);
		break;
	}

	
	/* DS_INST_READ2_RET: 2A READ2(dst0,dst1)
	 *   OQA=DS(dst0)
	 *   OQB=DS(dst1) */
	case 52: {
		uint32_t *pvalue;

		pvalue = malloc(4);
		mem_read(local_mem, op0, 4, pvalue);
		list_enqueue(gpu_isa_thread->lds_oqa, pvalue);

		pvalue = malloc(4);
		mem_read(local_mem, op1, 4, pvalue);
		list_enqueue(gpu_isa_thread->lds_oqb, pvalue);
		break;
	}


	default:
		GPU_PARAM_NOT_SUPPORTED(W1.lds_op);
	}
}
#undef W0
#undef W1


void amd_inst_MULADD_impl()
{
	float src0, src1, src2, dst;

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	src2 = gpu_isa_read_op_src_float(2);
	dst = src0 * src1 + src2;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_MULADD_M2_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_M4_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_D2_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_IEEE_impl()
{
	float src0, src1, src2, dst;

	src0 = gpu_isa_read_op_src_float(0);
	src1 = gpu_isa_read_op_src_float(1);
	src2 = gpu_isa_read_op_src_float(2);
	dst = src0 * src1 + src2;
	gpu_isa_enqueue_write_dest_float(dst);
}


void amd_inst_CNDE_impl() {
	NOT_IMPL();
}


void amd_inst_CNDGT_impl() {
	NOT_IMPL();
}


void amd_inst_CNDGE_impl() {
	NOT_IMPL();
}


void amd_inst_CNDE_INT_impl()
{
	uint32_t src0, src1, src2, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	src2 = gpu_isa_read_op_src(2);
	dst = src0 == 0 ? src1 : src2;
	gpu_isa_enqueue_write_dest(dst);
}


void amd_inst_CMNDGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_CMNDGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_LIT_impl() {
	NOT_IMPL();
}


#define W0  VTX_WORD0
#define W1  VTX_WORD1_GPR
#define W2  VTX_WORD2
void amd_inst_FETCH_impl()
{
	uint32_t addr;
	int data_format;
	int i;

	//amd_inst_words_dump(gpu_isa_inst, stdout);///

	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.fetch_type, 2);  /* NO_INDEX_OFFSET */
	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.fetch_whole_quad, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W0.buffer_id, 156);  /* FIXME: what is that? */
	GPU_PARAM_NOT_SUPPORTED_NEQ(W2.offset, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W2.endian_swap, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W2.const_buf_no_stride, 0);
	GPU_PARAM_NOT_SUPPORTED_NEQ(W2.mega_fetch, 1);

	/* W0.src_gpr: source GPR to get fetch address from */
	/* W0.sr (src_rel): relative or absolute src_gpr */
	/* W0.ssx (src_sel_x): src_gpr component to use (x,y,z,w) */
	/* W0.mfc (mega_fetch_count): for mega-fetch, number of bytes - 1 to fetch at once */

	/* W1.dst_gpr: destination GPR to write result */
	/* W1.dr (dst_rel): relative or absolute dst_gpr */
	/* W1.{dsx,dsy,dsz,dsw}: specified which element of result to write into dst_gpr.{x,y,z,w} */
	/* W1.use_const_fields: use format given in fetch constant or in this instr */
	/* W1.data_format: ignored for 'use_const_fields'=1 */
	/* W1.num_format_all: ignored for 'use_const_fields'=1 */
	/* W1.format_comp_all: ignored for 'use_const_fields'=1 */
	/* W1.srf_mode_all: ignored for 'use_const_fields'=1 */

	/* W2.offset: offset to be reading from (byte aligned) */
	/* W2.endian_swap */
	/* W2.const_buf_no_stride: force stride to 0 */
	/* W2.mf (mega_fetch) */

	/* FIXME: buffer_id - what does it mean? */
	
	/* Do not fetch for inactive threads */
	if (!gpu_thread_get_pred(gpu_isa_thread))
		return;
	
	/* Use constant fields */
	if (W1.use_const_fields) {
		/* FIXME: format can be found in constant. See table 2.10 in Evergreen manual.
		 * No details are given about this. */
		data_format = 13;  /* DATA_FORMAT_32 */
	} else {
		data_format = W1.data_format;
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.num_format_all, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.format_comp_all, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.srf_mode_all, 0);
	}
	
	/* Address */
	addr = gpu_isa_read_gpr(W0.src_gpr, W0.src_rel, W0.src_sel_x, 0) * 4;  /* FIXME: x4? */
	gpu_isa_debug("  t%d:read(0x%x)", GPU_THR.global_id, addr);

	/* Fetch */
	switch (data_format) {

	case 13:  /* DATA_FORMAT_32 */
	{
		uint32_t value;

		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_x, 0);  /* SEL_X */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_y, 7);  /* SEL_MASK */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_z, 7);  /* SEL_MASK */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_w, 7);  /* SEL_MASK */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.mega_fetch_count, 3);  /* 4-byte fetch */

		mem_read(gk->global_mem, addr + W2.offset, 4, &value);
		gpu_isa_write_gpr(W1.dst_gpr, W1.dst_rel, 0, value);
		if (debug_status(gpu_isa_debug_category)) {
			gpu_isa_debug("=(%d,%gf)=>", value, * (float *) &value);
			amd_inst_dump_gpr(W1.dst_gpr, W1.dst_rel, 0, 0, debug_file(gpu_isa_debug_category));
		}
		break;
	}
	
	case 35:  /* DATA_FORMAT_32_32_32_32_FLOAT */
	{
		float value[4];

		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_x, 0);  /* SEL_X */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_y, 1);  /* SEL_Y */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_z, 2);  /* SEL_Z */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.dst_sel_w, 3);  /* SEL_W */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.mega_fetch_count, 15);  /* 15-byte fetch */

		mem_read(gk->global_mem, addr + W2.offset, 16, value);
		for (i = 0; i < 4; i++) {
			gpu_isa_write_gpr_float(W1.dst_gpr, W1.dst_rel, i, value[i]);
			gpu_isa_debug(",");
			if (debug_status(gpu_isa_debug_category))
				amd_inst_dump_gpr(W1.dst_gpr, W1.dst_rel, i, 0, debug_file(gpu_isa_debug_category));
			gpu_isa_debug("=%gf", value[i]);
		}
		break;
	}

	default:
		GPU_PARAM_NOT_SUPPORTED(W1.data_format);
	}

}
#undef W0
#undef W1
#undef W2


void amd_inst_GET_BUFFER_RESINFO_impl() {
	NOT_IMPL();
}


void amd_inst_SEMANTIC_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_C_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_C_O_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_O_impl() {
	NOT_IMPL();
}


void amd_inst_GET_GRADIENTS_H_impl() {
	NOT_IMPL();
}


void amd_inst_GET_GRADIENTS_V_impl() {
	NOT_IMPL();
}


void amd_inst_GET_LOD_impl() {
	NOT_IMPL();
}


void amd_inst_GET_NUMBER_OF_SAMPLES_impl() {
	NOT_IMPL();
}


void amd_inst_GET_TEXTURE_RESINFO_impl() {
	NOT_IMPL();
}


void amd_inst_KEEP_GRADIENTS_impl() {
	NOT_IMPL();
}


void amd_inst_LD_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_G_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_G_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_L_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_LZ_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_G_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_G_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_L_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_LZ_impl() {
	NOT_IMPL();
}


void amd_inst_SET_GRADIENTS_H_impl() {
	NOT_IMPL();
}


void amd_inst_SET_GRADIENTS_V_impl() {
	NOT_IMPL();
}


void amd_inst_SET_TEXTURE_OFFSETS_impl() {
	NOT_IMPL();
}

