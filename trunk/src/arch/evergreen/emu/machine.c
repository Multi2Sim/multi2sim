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

#include <math.h>

#include <driver/opencl-old/evergreen/bin-file.h>
#include <driver/opencl-old/evergreen/kernel.h>
#include <driver/opencl-old/evergreen/mem.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "ndrange.h"
#include "work-item.h"
#include "work-group.h"


char *evg_err_isa_note =
	"\tThe AMD Evergreen instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define __EVG_NOT_IMPL__ \
	fatal("%s: Evergreen instruction '%s' not implemented\n%s", \
		__FUNCTION__, inst->info->name, evg_err_isa_note);


/*
 * CF Instructions
 */

#define W0  EVG_CF_ALU_WORD0
#define W1  EVG_CF_ALU_WORD1
void evg_isa_ALU_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.alt_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	/* FIXME: block constant cache sets */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */

	/* Start ALU clause */
	wavefront->clause_buf_start = wavefront->cf_buf_start + W0.addr * 8;
	wavefront->clause_buf_end = wavefront->clause_buf_start + (W1.count + 1) * 8;
	wavefront->clause_buf = wavefront->clause_buf_start;
	wavefront->clause_kind = EVG_CLAUSE_ALU;
	evg_isa_alu_clause_start(wavefront);
}
#undef W0
#undef W1


void evg_isa_ALU_BREAK_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Same behavior as ALU */
	/* FIXME: what's the difference? */
	evg_isa_ALU_impl(work_item, inst);
}


void evg_isa_ALU_POP_AFTER_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Same behavior as ALU */
	evg_isa_ALU_impl(work_item, inst);
}

void evg_isa_ALU_POP2_AFTER_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Initiates an ALU clause, and pops the stack twice after the clause completes execution. */
	__EVG_NOT_IMPL__
}

void evg_isa_ALU_PUSH_BEFORE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Same behavior as ALU inst */
	evg_isa_ALU_impl(work_item, inst);
}

void evg_isa_CALL_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Execute a subroutine call (push call variables onto stack) */
	__EVG_NOT_IMPL__
}

void evg_isa_CALL_FS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Call Fetch Subroutine */
	__EVG_NOT_IMPL__
}

void evg_isa_CUT_VERTEX_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* End Primitive Strip, Start New Primitive Strip */
	__EVG_NOT_IMPL__
}

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_ELSE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int active;
	int active_last;
	int active_new;
	int active_count = 0;

	int i;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* Debug */
	if (debug_status(evg_isa_debug_category))
	{
		evg_isa_debug("  %s:act=", wavefront->name);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count, wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Invert active mask */
	if (!wavefront->stack_top)
		fatal("ELSE: cannot execute for stack_top=0");
	for (i = 0; i < wavefront->work_item_count; i++)
	{
		active = bit_map_get(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count + i, 1);
		active_last = bit_map_get(wavefront->active_stack, (wavefront->stack_top - 1) *
			wavefront->work_item_count + i, 1);
		active_new = !active && active_last;
		active_count += active_new;
		bit_map_set(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count + i, 1, active_new);
	}
	
	/* Debug */
	if (debug_status(evg_isa_debug_category))
	{
		evg_isa_debug("  %s:invert(act)=", wavefront->name);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count, wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* If all pixels are inactive, pop stack and jump */
	if (!active_count)
	{
		evg_wavefront_stack_pop(wavefront, W1.pop_count);
		wavefront->cf_buf = wavefront->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1


void evg_isa_EMIT_CUT_VERTEX_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Emit Vertex, End Primitive Strip */
	__EVG_NOT_IMPL__
}


void evg_isa_EMIT_VERTEX_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Vertex Exported to Memory */
	__EVG_NOT_IMPL__
}


void evg_isa_EXPORT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Export from VS or PS */
	__EVG_NOT_IMPL__
}


void evg_isa_EXPORT_DONE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Export Last Data */
	__EVG_NOT_IMPL__
}


void evg_isa_GDS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Global Data Share */
	__EVG_NOT_IMPL__
}


void evg_isa_GWS_BARRIER_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Global Wavefront Barrier */
	__EVG_NOT_IMPL__
}


void evg_isa_GWS_INIT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Global Wavefront Resource Initialization */
	__EVG_NOT_IMPL__
}


void evg_isa_GWS_SEMA_P_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Global Wavefront Sync Semaphore P */
	__EVG_NOT_IMPL__
}

void evg_isa_GWS_SEMA_V_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Global Wavefront Sync Semaphore V */
	__EVG_NOT_IMPL__
}

void evg_isa_HALT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Halt Wavefront Execution */
	__EVG_NOT_IMPL__
}


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_JUMP_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int active_count;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* If all pixels are inactive, pop stack and jump */
	active_count = bit_map_count_ones(wavefront->active_stack,
		wavefront->stack_top * wavefront->work_item_count, wavefront->work_item_count);
	if (!active_count)
	{
		evg_wavefront_stack_pop(wavefront, W1.pop_count);
		wavefront->cf_buf = wavefront->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_JUMPTABLE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Executes a jump through a jump table. */
	__EVG_NOT_IMPL__
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_KILL_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Kill (prevent rendering of) pixels that pass a condition test. */
	__EVG_NOT_IMPL__
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_END_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int active_count;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if any pixel is active */

	/* Increment the trip count */
	++wavefront->loop_trip_count;

	/* Dump current loop state */
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:act=", wavefront->name);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count, wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Decrement loop index */
	wavefront->loop_index -= wavefront->loop_step;

	/* If any pixel is active and loop index is not zero, jump back */
	active_count = bit_map_count_ones(wavefront->active_stack,
		wavefront->stack_top * wavefront->work_item_count, wavefront->work_item_count);
	if (active_count && (wavefront->loop_index == 0) && 
		(wavefront->loop_trip_count != wavefront->loop_max_trip_count)) 
	{
		wavefront->cf_buf = wavefront->cf_buf_start + W0.addr * 8;
		return;
	}

	/* Pop stack once */
	evg_wavefront_stack_pop(wavefront, 1);

	/* FIXME: Get rid of this once loop state is pushed on the stack */
	--wavefront->loop_depth;
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_START_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);  /* FIXME: what does it refer to? */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if all pixels fail. */
	/* W0.pop_count: pop if all pixels fail. */

	/* Initialize the loop state for the wavefront */
	wavefront->loop_max_trip_count =
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][0];
	wavefront->loop_start =
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][1];
	wavefront->loop_step =
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][2];

	wavefront->loop_index = wavefront->loop_start;
	wavefront->loop_trip_count = 0;


	/* FIXME: if initial condition fails, jump to 'addr' */

	/* FIXME: Push loop state */

	/* FIXME: Set up new loop state */

	/* FIXME: Remove this once loop state is part of stack */
	++wavefront->loop_depth;

	/* FIXME: Push active mask? */
	evg_wavefront_stack_push(wavefront);
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_START_NO_AL_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);

	/* FIXME: Remove this once loop state is part of stack */
	++wavefront->loop_depth;
	if (wavefront->loop_depth > 1)
		fatal("%s: nested loops not supported", __FUNCTION__);

	/* Initialize the loop state for the wavefront */
	wavefront->loop_max_trip_count = 
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][0];
	wavefront->loop_start = 
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][1];
	wavefront->loop_step = 
		ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][2];

	wavefront->loop_index = wavefront->loop_start;
	wavefront->loop_trip_count = 0;

	evg_wavefront_stack_push(wavefront);
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_CONTINUE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_BREAK_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int active_count;
	int active;
	int i;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_RANGE(W1.cond, 0, 1);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
 
	/* W0.addr: jump if all pixels are disabled  */

	/* Dump current loop state */
	if (debug_status(evg_isa_debug_category))
	{
		evg_isa_debug("  %s:act=", wavefront->name);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count, wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Mark active work items as inactive */
	for (i = 0; i < wavefront->work_item_count; i++)
	{
		active = bit_map_get(wavefront->active_stack, wavefront->stack_top *
			wavefront->work_item_count + i, 1);

		if (active) 
		{
			/* Pixel is active, so mark it as inactive */
			bit_map_set(wavefront->active_stack, wavefront->stack_top *
				wavefront->work_item_count + i, 1, 0);
		}
	}

	/* If no pixels are active, jump to addr */
	active_count = bit_map_count_ones(wavefront->active_stack,
		wavefront->stack_top * wavefront->work_item_count, wavefront->work_item_count);
	if (!active_count)
	{
		wavefront->cf_buf = wavefront->cf_buf_start + W0.addr * 8;
		return;
	}

	/* Pop stack */
	evg_wavefront_stack_pop(wavefront, W1.pop_count);

	/* FIXME: Get rid of this once loop state is pushed on the stack */
	wavefront->loop_depth -= W1.pop_count;
}
#undef W0
#undef W1


void evg_isa_MEM_EXPORT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Performs a memory read or write on the scatter buffer. */
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_EXPORT_COMBINED_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Performs a memory read or write on the scatter buffer. */
	__EVG_NOT_IMPL__
}


#define W0  EVG_CF_ALLOC_EXPORT_WORD0_RAT
#define W1  EVG_CF_ALLOC_EXPORT_WORD1_BUF
void evg_isa_MEM_RAT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	switch (W0.rat_inst)
	{

	/* STORE_RAW */
	case 1:
		/* FIXME Need to support multiple elements (multi-channel images) */
	case 2:
	{
		uint32_t addr;
		union evg_reg_t value;

		int work_item_id;
		int i;
		int uav;
		struct evg_opencl_mem_t *mem;
		uint32_t base_addr;

		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.rat_index_mode, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.elem_size, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.burst_count, 0);

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
			EVG_ISA_ARG_NOT_SUPPORTED(W0.type);

		/* Record access */
		wavefront->global_mem_write = 1;

		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];

			/* If VPM is set, do not export for inactive pixels. */
			if (W1.valid_pixel_mode && !evg_work_item_get_active(work_item))
				continue;

			/* W0.rw_gpr: GPR register from which to read data */
			/* W0.rr: relative/absolute rw_gpr */
			/* W0.index_gpr: GPR containing buffer coordinates. It is multiplied by (elem_size+1) */
			/* W0.elem_size: number of doublewords per array element, minus one */
			/* W1.array_size: array size (elem-size units) */

			/* This is a write, so we need to get the correct address based on the UAV number */
			uav = W0.rat_id;

			/* Otherwise, we have an image, and we need to provide a base address */
			mem = list_get(ndrange->kernel->uav_write_list, uav);
			base_addr = mem->device_ptr;
			addr = base_addr + evg_isa_read_gpr(work_item, W0.index_gpr,
				0, 0, 0) * 4;  /* FIXME: only 1D - X coordinate, FIXME: x4? */
			evg_isa_debug("  t%d:write(0x%x)", work_item->id, addr);

			/* Record access */
			work_item->global_mem_access_addr = addr;
			work_item->global_mem_access_size = 0;

			for (i = 0; i < 4; i++)
			{
				/* If component is masked, skip */
				if (!(W1.comp_mask & (1 << i)))
					continue;

				/* FIXME We only support single-channel images, and don't know
				 * how the ISA knows which components to write */
				if (i) 
					continue;

				/* Record size for memory access (warning: this is done inaccurately by assuming a
				 * baseline access and a contiguous set of accessed elements */
				work_item->global_mem_access_size += 4;

				/* Access */
				value.as_uint = evg_isa_read_gpr(work_item, W0.rw_gpr, W0.rr, i, 0);

				/* FIXME: leave gaps when intermediate 'comp_mask' bits are not set? */
				mem_write(evg_emu->global_mem, addr + i * 4, 4, &value);
				evg_isa_debug(",");
				if (debug_status(evg_isa_debug_category))
					evg_inst_dump_gpr(W0.rw_gpr, W0.rr, i, 0, debug_file(evg_isa_debug_category));
				evg_isa_debug("=(0x%x,%gf)", value.as_uint, value.as_float);
			}
			/* FIXME: array_size: ignored now, because 'burst_count' = 0 */
			/* FIXME: rat_id */
			/* FIXME: mark - mark memory write to be acknowledged by the next write-ack */
			/* FIXME: barrier */
		}
		break;
	}

	default:
		EVG_ISA_ARG_NOT_SUPPORTED(W0.rat_inst);
	}
}
#undef W0
#undef W1


#define W0  EVG_CF_ALLOC_EXPORT_WORD0_RAT
#define W1  EVG_CF_ALLOC_EXPORT_WORD1_BUF
void evg_isa_MEM_RAT_CACHELESS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	switch (W0.rat_inst)
	{

	/* STORE_RAW */
	case 2:
	{
		union evg_reg_t value;
		uint32_t addr;

		int work_item_id;
		int i;

		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.rat_index_mode, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.elem_size, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.burst_count, 0);

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
			EVG_ISA_ARG_NOT_SUPPORTED(W0.type);

		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];

			/* If VPM is set, do not export for inactive pixels. */
			if (W1.valid_pixel_mode && !evg_work_item_get_active(work_item))
				continue;

			/* W0.rw_gpr: GPR register from which to read data */
			/* W0.rr: relative/absolute rw_gpr */
			/* W0.index_gpr: GPR containing buffer coordinates. It is multiplied by (elem_size+1) */
			/* W0.elem_size: number of doublewords per array element, minus one */
			/* W1.array_size: array size (elem-size units) */

			/* This is a write, so we need to get the correct address based on the UAV number */
			addr = evg_isa_read_gpr(work_item, W0.index_gpr, 0, 0, 0) * 4;  /* FIXME: only 1D - X coordinate, FIXME: x4? */
			evg_isa_debug("  t%d:write(0x%x)", work_item->id, addr);

			/* Record access */
			wavefront->global_mem_write = 1;
			work_item->global_mem_access_addr = addr;
			work_item->global_mem_access_size = 0;

			for (i = 0; i < 4; i++)
			{
				/* If component is masked, skip */
				if (!(W1.comp_mask & (1 << i)))
					continue;

				/* Record size for memory access (warning: this is done inaccurately by assuming a
				 * baseline access and a contiguous set of accessed elements */
				work_item->global_mem_access_size += 4;

				/* Access */
				value.as_uint = evg_isa_read_gpr(work_item, W0.rw_gpr, W0.rr, i, 0);

				/* FIXME: leave gaps when intermediate 'comp_mask' bits are not set? */
				mem_write(evg_emu->global_mem, addr + i * 4, 4, &value);
				evg_isa_debug(",");
				if (debug_status(evg_isa_debug_category))
					evg_inst_dump_gpr(W0.rw_gpr, W0.rr, i, 0, debug_file(evg_isa_debug_category));
				evg_isa_debug("=(0x%x,%gf)", value.as_uint, value.as_float);
			}
			/* FIXME: array_size: ignored now, because 'burst_count' = 0 */
			/* FIXME: rat_id */
			/* FIXME: mark - mark memory write to be acknowledged by the next write-ack */
			/* FIXME: barrier */
		}
		break;
	}

	default:
		EVG_ISA_ARG_NOT_SUPPORTED(W0.rat_inst);
	}
}
#undef W0
#undef W1


void evg_isa_MEM_RAT_COMBINED_CACHELESS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_RING_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_RING1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_RING2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_RING3_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM0_BUF0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM0_BUF1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM0_BUF2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM0_BUF3_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM1_BUF0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM1_BUF1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM1_BUF2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM1_BUF3_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM2_BUF0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM2_BUF1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM2_BUF2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM2_BUF3_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM3_BUF0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM3_BUF1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM3_BUF2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_STREAM3_BUF3_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MEM_WR_SCRATCH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_NOP_CF_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
}


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_POP_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* Pop 'pop_count' from stack and jump to 'addr' */
	evg_wavefront_stack_pop(wavefront, W1.pop_count);
	wavefront->cf_buf = wavefront->cf_buf_start + W0.addr * 8;
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_PUSH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Push State To Stack */
	__EVG_NOT_IMPL__
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_RETURN_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Return From Subroutine */
	__EVG_NOT_IMPL__
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_TC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;

	int active;
	int i;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);  /* ignored for this instruction */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);  /* number of entries to pop from stack */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);  /* how to evaluate condition test for pixels (ACTIVE) */
	/* FIXME: valid_pixel_mode */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */
	
	/* Start TC clause */
	wavefront->clause_buf_start = wavefront->cf_buf_start + W0.addr * 8;
	wavefront->clause_buf_end = wavefront->clause_buf_start + (W1.count + 1) * 16;
	wavefront->clause_buf = wavefront->clause_buf_start;
	wavefront->clause_kind = EVG_CLAUSE_TEX;
	evg_isa_tc_clause_start(wavefront);

	/* If VPM is set, copy 'active' mask at the top of the stack to 'pred' mask.
	 * This will make all fetches within the clause happen only for active pixels.
	 * If VPM is clear, copy a mask set to ones. */
	for (i = 0; i < wavefront->work_item_count; i++)
	{
		active = W1.valid_pixel_mode ? bit_map_get(wavefront->active_stack,
			wavefront->stack_top * wavefront->work_item_count + i, 1) : 1;
		bit_map_set(wavefront->pred, i, 1, active);
	}
}
#undef W0
#undef W1


void evg_isa_TC_ACK_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Fetch Clause Through Texture Cache With ACK */
	__EVG_NOT_IMPL__
}


void evg_isa_VC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Initiate Clause of Vertex or Constant Fetches Through Vertex Cache */
	__EVG_NOT_IMPL__
}


void evg_isa_VC_ACK_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* Fetch Clause Through Vertex Cache With ACK */
	__EVG_NOT_IMPL__
}

void evg_isa_WAIT_ACK_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	/* FIXME: wait for Write ACKs */
}


void evg_isa_ADD_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	dst = src0 + src1;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MUL_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MUL_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MAX_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MIN_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MAX_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MIN_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 == src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 > src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 >= src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETNE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 != src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETE_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 == src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGT_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGE_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETNE_DX10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


void evg_isa_FRACT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = fmodf(src, 1.0f);
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_TRUNC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = truncf(src);
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_CEIL_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RNDNE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLOOR_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	
	dst = trunc(src0);
	if((src0 < 0.0f) && (src0 != dst))
		dst += -1.0f;

	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_ASHR_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, dst;
	uint32_t src1;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	if (src1 > 31)
		dst = src0 < 0 ? -1 : 0;
	else
		dst = src0 >> src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_LSHR_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 >> src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_LSHL_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 << src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MOV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t value;
	value = evg_isa_read_op_src_int(work_item, inst, 0);
	evg_isa_enqueue_write_dest(work_item, inst, value);
}


void evg_isa_NOP_ALU_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
}


void evg_isa_MUL_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT64_TO_FLT32_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT32_TO_FLT64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGT_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGE_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 == src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETGT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 > src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETGE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 >= src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETNE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	cond = src0 != src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


void evg_isa_PRED_SET_INV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SET_POP_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SET_CLR_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SET_RESTORE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETE_PUSH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGT_PUSH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGE_PUSH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETNE_PUSH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLGT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLGE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLNE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_AND_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 & src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_OR_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 | src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_XOR_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 ^ src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_NOT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_ADD_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 + src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_SUB_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 - src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MAX_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MIN_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MAX_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MIN_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 == src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETNE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1




#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGT_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_SETGE_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}
#undef W0
#undef W1


void evg_isa_KILLGT_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLGE_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PREDE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 == src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETGE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 >= src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETGT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 > src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_PRED_SETNE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	cond = src0 != src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(work_item, inst, dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before(work_item, inst);
	evg_isa_enqueue_pred_set(work_item, inst, cond);
}
#undef W0
#undef W1


void evg_isa_KILLE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLGT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLGE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KILLNE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETE_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGT_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGE_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETNE_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETLT_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETLE_PUSH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT_TO_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src;
	int32_t dst;
	
	src = evg_isa_read_op_src_float(work_item, inst, 0);
	if (isinf(src) == 1)
		dst = INT32_MAX;
	else if (isinf(src) == -1)
		dst = INT32_MIN;
	else if (isnan(src))
		dst = 0;
	else
		dst = src;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_BFREV_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_ADDC_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SUBB_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GROUP_BARRIER_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_work_group_t *work_group = work_item->work_group;

	/* Only the first work-item in a wavefront handles barriers */
	if (work_item->id_in_wavefront)
		return;
	
	/* Suspend current wavefront at the barrier */
	assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
	DOUBLE_LINKED_LIST_REMOVE(work_group, running, wavefront);
	DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, barrier, wavefront);
	evg_isa_debug("%s (gid=%d) reached barrier (%d reached, %d left)\n",
		wavefront->name, work_group->id, work_group->barrier_list_count,
		work_group->wavefront_count - work_group->barrier_list_count);
	
	/* If all wavefronts in work-group reached the barrier, wake them up */
	if (work_group->barrier_list_count == work_group->wavefront_count)
	{
		struct evg_wavefront_t *wavefront;
		while (work_group->barrier_list_head)
		{
			wavefront = work_group->barrier_list_head;
			DOUBLE_LINKED_LIST_REMOVE(work_group, barrier, wavefront);
			DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, running, wavefront);
		}
		assert(work_group->running_list_count == work_group->wavefront_count);
		assert(work_group->barrier_list_count == 0);
		evg_isa_debug("%s completed barrier, waking up wavefronts\n",
			work_group->name);
	}
}


void evg_isa_GROUP_SEQ_BEGIN_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GROUP_SEQ_END_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_MODE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_CF_IDX0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_CF_IDX1_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_LDS_SIZE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_EXP_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LOG_CLAMPED_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LOG_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIP_CLAMPED_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIP_FF_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIP_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = 1.0f / src;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_RECIPSQRT_CLAMPED_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIPSQRT_FF_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIPSQRT_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SQRT_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = sqrtf(src);
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_SIN_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = sinf(src);
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_COS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src, dst;

	src = evg_isa_read_op_src_float(work_item, inst, 0);
	dst = cosf(src);
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MULLO_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MULHI_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int64_t src0, src1, dst;

	src0 = (int32_t) evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = (int32_t) evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst >> 32);
}


void evg_isa_MULLO_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	unsigned int src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MULHI_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint64_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(work_item, inst, dst >> 32);
}


void evg_isa_RECIP_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIP_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src;
	uint32_t dst;

	src = evg_isa_read_op_src_int(work_item, inst, 0);
	dst = 0xffffffff / src;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_RECIP_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIP_CLAMPED_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIPSQRT_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_RECIPSQRT_CLAMPED_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SQRT_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT_TO_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src;
	uint32_t dst;
	
	src = evg_isa_read_op_src_float(work_item, inst, 0);
	if (isinf(src) == 1)
		dst = UINT32_MAX;
	else if (isinf(src) == -1 || isnan(src))
		dst = 0;
	else
		dst = src;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_INT_TO_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int32_t src0;
	float dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	dst = (float) src0;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_UINT_TO_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0;
	float dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	dst = (float) src0;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_BFM_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT32_TO_FLT16_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT16_TO_FLT32_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_UBYTE0_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_UBYTE1_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_UBYTE2_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_UBYTE3_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_BCNT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FFBH_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FFBL_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FFBH_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT_TO_UINT4_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_DOT_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT_TO_INT_RPI_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT_TO_INT_FLOOR_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULHI_UINT24_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MBCNT_32HI_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_OFFSET_TO_FLT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MUL_UINT24_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_BCNT_ACCUM_PREV_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MBCNT_32LO_ACCUM_PREV_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SETE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SETNE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SETGT_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SETGE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MIN_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MAX_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_DOT4_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_DOT4_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_CUBE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MAX4_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FREXP_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LDEXP_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FRACT_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGT_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_PRED_SETGE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_MUL_64_VEC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_alu_group_t *alu_group = inst->alu_group;
	struct evg_inst_t *inst_slot[4];

	enum evg_alu_enum alu;

	int i;

	union
	{
		double as_double;
		unsigned int as_uint[2];
	} src0, src1, dst;

	/* Only slot X runs this instruction */
	alu = inst->alu;
	if (alu % 4)
		return;

	/* Get multi-slot instruction */
	for (i = 0; i < 4; i++)
	{
		inst_slot[i] = evg_isa_get_alu_inst(alu_group, i);
		if (!inst_slot[i])
			fatal("%s: unexpected empty VLIW slots", __FUNCTION__);
		if (inst_slot[i]->info->inst != inst->info->inst)
			fatal("%s: invalid multi-slot instruction", __FUNCTION__);
	}

	/* Get low-order operand from slot 2 */
	src0.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[2], 0);
	src1.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[2], 1);

	/* Get high-order operand from slot 3 */
	src0.as_uint[1] = evg_isa_read_op_src_int(work_item, inst_slot[3], 0);
	src1.as_uint[1] = evg_isa_read_op_src_int(work_item, inst_slot[3], 1);

	/* Operation */
	dst.as_double = src0.as_double * src1.as_double;

	/* Store low-order result to slots 0 and 2 */
	evg_isa_enqueue_write_dest(work_item, inst_slot[0], dst.as_uint[0]);
	evg_isa_enqueue_write_dest(work_item, inst_slot[2], dst.as_uint[0]);

	/* Store high-order result to slots 1 and 3 */
	evg_isa_enqueue_write_dest(work_item, inst_slot[1], dst.as_uint[1]);
	evg_isa_enqueue_write_dest(work_item, inst_slot[1], dst.as_uint[1]);
}
#undef W0
#undef W1


#define W0 EVG_ALU_WORD0
#define W1 EVG_ALU_WORD1_OP2
void evg_isa_ADD_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_alu_group_t *alu_group = inst->alu_group;
	struct evg_inst_t *inst_slot[2];

	enum evg_alu_enum alu;

	union
	{
		double as_double;
		unsigned int as_uint[2];
	} src0, src1, dst;

	int i;

	/* Only slots X or Z run this instruction */
	alu = inst->alu;
	if (alu % 2)
		return;

	/* Get multi-slot instruction */
	for (i = 0; i < 2; i++)
	{
		inst_slot[i] = evg_isa_get_alu_inst(alu_group, alu / 2 * 2 + i);
		if (!inst_slot[i])
			fatal("%s: unexpected empty VLIW slots", __FUNCTION__);
		if (inst_slot[i]->info->inst != inst->info->inst)
			fatal("%s: invalid multi-slot instruction", __FUNCTION__);
	}
	
	/* Get low-order operands from slot 0 */
	src0.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[0], 0);
	src1.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[0], 1);

	/* Get high-order operands from slot 1 */
	src0.as_uint[1] = evg_isa_read_op_src_int(work_item, inst_slot[1], 0);
	src1.as_uint[1] = evg_isa_read_op_src_int(work_item, inst_slot[1], 1);

	/* Operation */
	dst.as_double = src0.as_double + src1.as_double;

	/* Store low-order operand to slot 0, high-order to slot 1 */
	evg_isa_enqueue_write_dest(work_item, inst_slot[0], dst.as_uint[0]);
	evg_isa_enqueue_write_dest(work_item, inst_slot[1], dst.as_uint[1]);
}
#undef W0
#undef W1


void evg_isa_MOVA_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FLT64_TO_FLT32_VEC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_alu_group_t *alu_group = inst->alu_group;
	struct evg_inst_t *inst_slot[2];

	enum evg_alu_enum alu;

	union
	{
		double as_double;
		unsigned int as_uint[2];
		float as_float[2];
	} src, dst;

	int i;

	/* Only slots X or Z run this instruction */
	alu = inst->alu;
	if (alu % 2)
		return;

	/* Get multi-slot instruction */
	for (i = 0; i < 2; i++)
	{
		inst_slot[i] = evg_isa_get_alu_inst(alu_group, alu / 2 * 2 + i);
		if (!inst_slot[i])
			fatal("%s: unexpected empty VLIW slots", __FUNCTION__);
		if (inst_slot[i]->info->inst != inst->info->inst)
			fatal("%s: invalid multi-slot instruction", __FUNCTION__);
	}
	
	/* Get low-order operand from slot 0, high-order from slot 1 */
	src.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[0], 0);
	src.as_uint[1] = evg_isa_read_op_src_int(work_item, inst_slot[1], 0);

	/* Operation */
	dst.as_float[0] = src.as_double;
	dst.as_uint[1] = 0;

	/* Store low-order operand to slot 0, high-order to slot 1 */
	evg_isa_enqueue_write_dest(work_item, inst_slot[0], dst.as_uint[0]);
	evg_isa_enqueue_write_dest(work_item, inst_slot[1], dst.as_uint[1]);
}


void evg_isa_FLT32_TO_FLT64_VEC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_alu_group_t *alu_group = inst->alu_group;
	struct evg_inst_t *inst_slot[2];

	enum evg_alu_enum alu;

	union
	{
		double as_double;
		unsigned int as_uint[2];
		float as_float[2];
	} src, dst;

	int i;

	/* Only slots X or Z run this instruction */
	alu = inst->alu;
	if (alu % 2)
		return;

	/* Get multi-slot instruction */
	for (i = 0; i < 2; i++)
	{
		inst_slot[i] = evg_isa_get_alu_inst(alu_group, alu / 2 * 2 + i);
		if (!inst_slot[i])
			fatal("%s: unexpected empty VLIW slots", __FUNCTION__);
		if (inst_slot[i]->info->inst != inst->info->inst)
			fatal("%s: invalid multi-slot instruction", __FUNCTION__);
	}
	
	/* Get low-order operand from slot 0.
	 * There is no high-order operand in this case. */
	src.as_uint[0] = evg_isa_read_op_src_int(work_item, inst_slot[0], 0);

	/* Operation */
	dst.as_double = src.as_float[0];

	/* Store low-order operand to slot 0, high-order to slot 1 */
	evg_isa_enqueue_write_dest(work_item, inst_slot[0], dst.as_uint[0]);
	evg_isa_enqueue_write_dest(work_item, inst_slot[1], dst.as_uint[1]);
}


void evg_isa_SAD_ACCUM_PREV_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_DOT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MUL_PREV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MUL_IEEE_PREV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_ADD_PREV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_PREV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_IEEE_PREV_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_XY_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_ZW_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_X_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_Z_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_STORE_FLAGS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LOAD_STORE_FLAGS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LDS_1A_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LDS_1A1D_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LDS_2A_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_LOAD_P0_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_LOAD_P10_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_INTERP_LOAD_P20_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_BFE_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	unsigned int src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);  /* Input data */
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);  /* Offset */
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);  /* Width */

	src1 = (src1 & 0x1F);
	src2 = (src2 & 0x1F);

	if (src2 == 0) 
	{
		dst = 0;
	}
	else if (src2 + src1 < 32) 
	{
		dst = (src0 << (32 - src1 - src2)) >> (32 - src2);
	}
	else 
	{
		dst = src0 >> src1;
	}

	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_BFE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{

        int src0, src1, src2, dst;

        src0 = evg_isa_read_op_src_int(work_item, inst, 0);
        src1 = evg_isa_read_op_src_int(work_item, inst, 1);
        src2 = evg_isa_read_op_src_int(work_item, inst, 2);

        src1 = (src1 & 0x1F);
        src2 = (src2 & 0x1F);

        if (src2 == 0)
        {
                dst = 0;
        }
        else if (src2 + src1 < 32)
        {
                dst = (src0 << (32-src1-src2)) >> (32-src2);
        }
        else
        {
                dst = src0 >> src1;
        }

        evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_BFI_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{

	uint32_t src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);

	/* Documentation says: dst = (src1 & src0) | (src2 & -src0) */
	dst = (src1 & src0) | (src2 & ~src0);
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_FMA_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_CNDNE_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_FMA_64_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LERP_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_BIT_ALIGN_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_BYTE_ALIGN_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAD_ACCUM_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAD_ACCUM_HI_UINT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_UINT24_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0 EVG_ALU_WORD0_LDS_IDX_OP
#define W1 EVG_ALU_WORD1_LDS_IDX_OP
void evg_isa_LDS_IDX_OP_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_work_group_t *work_group = work_item->work_group;

	struct mem_t *local_mem = work_group->local_mem;

	unsigned int idx_offset;

	unsigned int op0;
	unsigned int op1;
	unsigned int op2;

	/* Recompose 'idx_offset' field */
	idx_offset = (W0.idx_offset_5 << 5) | (W0.idx_offset_4 << 4) |
		(W1.idx_offset_3 << 3) | (W1.idx_offset_2 << 2) |
		(W1.idx_offset_1 << 1) | W1.idx_offset_0;
	
	/* Read operands */
	op0 = evg_isa_read_op_src_int(work_item, inst, 0);
	op1 = evg_isa_read_op_src_int(work_item, inst, 1);
	op2 = evg_isa_read_op_src_int(work_item, inst, 2);

	/* Process LDS instruction */
	switch (W1.lds_op)
	{

	/* DS_INST_WRITE: 1A1D WRITE(dst,src) : DS(dst) = src */
	case 13:
	{
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(op2, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(idx_offset, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.dst_chan, 0);
		/* FIXME: dst_chan? Does address need to be multiplied? */
		evg_isa_enqueue_write_lds(work_item, inst, op0, op1, 4);

		wavefront->local_mem_write = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 2;
		work_item->local_mem_access_addr[0] = op1;
		work_item->local_mem_access_size[0] = 4;
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

		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.dst_chan, 0);
		evg_isa_enqueue_write_lds(work_item, inst, dst, src0, 4);
		evg_isa_enqueue_write_lds(work_item, inst, tmp, src1, 4);  /* FIXME: correct? */

		wavefront->local_mem_write = 1;
		work_item->local_mem_access_count = 2;
		work_item->local_mem_access_type[0] = 2;
		work_item->local_mem_access_addr[0] = dst;
		work_item->local_mem_access_size[0] = 4;
		work_item->local_mem_access_type[1] = 2;
		work_item->local_mem_access_addr[1] = tmp;
		work_item->local_mem_access_size[1] = 4;
		break;
	}

	/* DS_INST_BYTE_WRITE: 1A1D BYTEWRITE (dst,src) : DS(dst) = src[7:0] */
	case 18:
	{
		uint32_t src, dst; 
		src = op1;
		dst = op0;

		evg_isa_enqueue_write_lds(work_item, inst, dst, src, 1);

		wavefront->local_mem_write = 1; 
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 2; /* write */
		work_item->local_mem_access_addr[0] = dst;
		work_item->local_mem_access_size[0] = 1;
		break;
	}	   

	/* DS_INST_SHORT_WRITE: 1A1D SHORTWRITE (dst,src) : DS(dst) = src[15:0] */
	case 19:
	{
		uint32_t src, dst;
		src = op1;
		dst = op0;

		evg_isa_enqueue_write_lds(work_item, inst, dst, src, 2);

		wavefront->local_mem_write = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 2; /* write */
		work_item->local_mem_access_addr[0] = dst;
		work_item->local_mem_access_size[0] = 2;
		break;

	}

	/* DS_INST_READ_RET: 1A READ(dst) : OQA = DS(dst) */
	case 50:
	{
		union evg_reg_t *value_ptr;

		value_ptr = xmalloc(4);

		mem_read(local_mem, op0, 4, value_ptr);
		list_enqueue(work_item->lds_oqa, value_ptr);
		evg_isa_debug("  t%d:LDS[0x%x]=(%u,%gf)=>OQA",
			work_item->id, op0, value_ptr->as_uint, value_ptr->as_float);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 1;
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 4;
		break;
	}

	
	/* DS_INST_READ2_RET: 2A READ2(dst0,dst1)
	 *   OQA=DS(dst0)
	 *   OQB=DS(dst1) */
	case 52:
	{
		union evg_reg_t *value_ptr;

		value_ptr = xmalloc(4);

		mem_read(local_mem, op0, 4, value_ptr);
		list_enqueue(work_item->lds_oqa, value_ptr);

		value_ptr = xmalloc(4);

		mem_read(local_mem, op1, 4, value_ptr);
		list_enqueue(work_item->lds_oqb, value_ptr);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 2;
		work_item->local_mem_access_type[0] = 1;
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 4;
		work_item->local_mem_access_type[1] = 1;
		work_item->local_mem_access_addr[1] = op1;
		work_item->local_mem_access_size[1] = 4;
		break;
	}


	/* DS_INST_BYTE_READ_RET: 1A BYTEREAD(dst) 
	 *    OQA=SignExtend(DS(dst)[7:0]) */
	case 54:
	{
		char value;
		int32_t *pvalue_se; 

		mem_read(local_mem, op0, 1, &value);

		pvalue_se = xmalloc(4);

		*pvalue_se = value;
		list_enqueue(work_item->lds_oqa, pvalue_se);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 1; /* read */
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 1;
		break;
	}


	/* DS_INST_UBYTE_READ_RET: 1A UBYTEREAD(dst) 
	 *    OQA={24'h0, DS(dst)[7:0]} */
	case 55: 
	{
		unsigned char value;
		uint32_t *pvalue_24h0;

		mem_read(local_mem, op0, 1, &value);

		pvalue_24h0 = xmalloc(4);

		*pvalue_24h0 = value;
		list_enqueue(work_item->lds_oqa, pvalue_24h0);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 1; /* read */
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 1;
		break;
	}

	/* DS_INST_SHORT_READ_RET: 1A SHORTREAD(dst) 
	 *    OQA=SignExtend({16'h0, DS(dst)[15:0]}) */
	case 56: 
	{
		short value;
		int32_t *pvalue_se;

		mem_read(local_mem, op0, 2, &value);

		pvalue_se = xmalloc(4);

		*pvalue_se = value;
		evg_isa_debug("  t%d: %d (pvalue = %d)", work_item->id, value, *pvalue_se);
		list_enqueue(work_item->lds_oqa, pvalue_se);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 1; /* read */
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 2;
		break;
	}

	/* DS_INST_USHORT_READ_RET: 1A USHORTREAD(dst) 
	 *    OQA={16'h0, DS(dst)[15:0]} */
	case 57: 
	{
		unsigned short value;
		uint32_t *pvalue_16h0;

		mem_read(local_mem, op0, 2, &value);

		pvalue_16h0 = xmalloc(4);

		*pvalue_16h0 = value;
		list_enqueue(work_item->lds_oqa, pvalue_16h0);

		wavefront->local_mem_read = 1;
		work_item->local_mem_access_count = 1;
		work_item->local_mem_access_type[0] = 1; /* read */
		work_item->local_mem_access_addr[0] = op0;
		work_item->local_mem_access_size[0] = 2;
		break;
	}

	default:
		EVG_ISA_ARG_NOT_SUPPORTED(W1.lds_op);
	}
}
#undef W0
#undef W1


void evg_isa_MULADD_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	src2 = evg_isa_read_op_src_float(work_item, inst, 2);
	dst = src0 * src1 + src2;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_MULADD_M2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_M4_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_D2_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_MULADD_IEEE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_float(work_item, inst, 0);
	src1 = evg_isa_read_op_src_float(work_item, inst, 1);
	src2 = evg_isa_read_op_src_float(work_item, inst, 2);
	dst = src0 * src1 + src2;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_CNDE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 == 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_CNDGT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 > 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_CNDGE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 >= 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(work_item, inst, dst);
}


void evg_isa_CNDE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	uint32_t src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 == 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_CNDGE_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int src0, src1, src2, dst;
	
	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 >= 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_CNDGT_INT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	int src0, src1, src2, dst;
	
	src0 = evg_isa_read_op_src_int(work_item, inst, 0);
	src1 = evg_isa_read_op_src_int(work_item, inst, 1);
	src2 = evg_isa_read_op_src_int(work_item, inst, 2);
	dst = src0 > 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(work_item, inst, dst);
}


void evg_isa_MUL_LIT_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0  EVG_VTX_WORD0
#define W1  EVG_VTX_WORD1_GPR
#define W2  EVG_VTX_WORD2
void evg_isa_FETCH_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	unsigned int addr;
	unsigned int base_addr;

	int data_format;
	int dst_sel[4];
	int dst_sel_elem;
	int i;

	unsigned int value[4];

	/* Related to data type */
	struct evg_opencl_mem_t *mem;
	size_t elem_size = 0;
	int num_elem;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.fetch_type, 2);  /* NO_INDEX_OFFSET */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.fetch_whole_quad, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W2.offset, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W2.endian_swap, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W2.const_buf_no_stride, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W2.mega_fetch, 1);

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

	/* Do not fetch for inactive work_items */
	if (!evg_work_item_get_active(work_item))
		return;
	
	/* Store 'dst_sel_{x,y,z,w}' in array */
	dst_sel[0] = W1.dst_sel_x;
	dst_sel[1] = W1.dst_sel_y;
	dst_sel[2] = W1.dst_sel_z;
	dst_sel[3] = W1.dst_sel_w;

	base_addr = 0;
	
	/* Data type information is either stored in instruction or constant fields */
	if (W1.use_const_fields)
	{
		/* Don't know what these buffers are */
		if (W0.buffer_id < 130)
		{
			fatal("Fetch instruction has unknown buffer ID (%d)\n", W0.buffer_id);
		}

		/* Information is in a constant buffer (filled by us) */
		else if (W0.buffer_id >= 130 && W0.buffer_id < 153)
		{ /* FIXME Verify that 153 is correct */
			mem = list_get(ndrange->kernel->constant_buffer_list, W0.buffer_id-128);
			if(!mem) 
				fatal("No table entry for constant UAV %d\n", W0.buffer_id);

			base_addr = mem->device_ptr;

			/* FIXME For constant pointers, it appears that 1 constant register is always
			 * filled, regardless of data size. */
			elem_size = 4;
			num_elem = 4; 
		}

		/* Data is a 32-bit type (cached) */
		else if (W0.buffer_id >= 144 && W0.buffer_id <= 153)
		{
			elem_size = 4;
			num_elem = 1;
		}

		/* Data is a 32-bit type (uncached -- though we cache it anyway) */
		else if ((W0.buffer_id >= 154 && W0.buffer_id <= 164) || W0.buffer_id == 173)
		{
			elem_size = 4;
			num_elem = 1;
		}

		/* Data is a 32-bit type (global return buffer) */
		else if (W0.buffer_id >= 165 && W0.buffer_id <= 172)
		{
			elem_size = 4;
			num_elem = 1;
		}

		/* Data is a 64-bit type (cached) */
		else if (W0.buffer_id == 174)
		{
			elem_size = 4;
			num_elem = 2;
		}

		/* Data is a 128-bit type (cached) */
		else if (W0.buffer_id == 175)
		{

			elem_size = 4;
			num_elem = 4;
		}
		else
		{

			fatal("Fetch instruction has unknown buffer ID (%d)\n", 
					W0.buffer_id);
		}
	}
	else
	{
		/* Format is defined within instruction */
		data_format = W1.data_format;
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.num_format_all, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.format_comp_all, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.srf_mode_all, 0);

		switch (data_format)
		{

		case 35:  /* DATA_FORMAT_32_32_32_32_FLOAT */
		case 34:  /* DATA_FORMAT_32_32_32_32 */

			elem_size = 4;
			num_elem = 4;
			break;

		case 48:  /* DATA_FORMAT_32_32_32_FLOAT */
		case 47:  /* DATA_FORMAT_32_32_32 */

			elem_size = 4;
			num_elem = 3;
			break;

		case 30:  /* DATA_FORMAT_32_32_FLOAT */
		case 29:  /* DATA_FORMAT_32_32 */

			elem_size = 4;
			num_elem = 2;
			break;

		case 14:  /* DATA_FORMAT_32_FLOAT */
		case 13:  /* DATA_FORMAT_32 */

			elem_size = 4;
			num_elem = 1;
			break;

		case 32:  /* DATA_FORMAT_16_16_16_16_FLOAT */
		case 31:  /* DATA_FORMAT_16_16_16_16 */

			elem_size = 2;
			num_elem = 4;
			break;

		case 46:  /* DATA_FORMAT_16_16_16_FLOAT */
		case 45:  /* DATA_FORMAT_16_16_16 */

			elem_size = 2;
			num_elem = 3;
			break;

		case 16:  /* DATA_FORMAT_16_16_FLOAT */
		case 15:  /* DATA_FORMAT_16_16 */

			elem_size = 2;
			num_elem = 2;
			break;

		case 6:  /* DATA_FORMAT_16_FLOAT */
		case 5:  /* DATA_FORMAT_16_*/

			elem_size = 2;
			num_elem = 1;
			break;

		case 26:  /* DATA_FORMAT_8_8_8_8 */

			elem_size = 1;
			num_elem = 4;
			break;

		case 44:  /* DATA_FORMAT_8_8_8 */

			elem_size = 1;
			num_elem = 3;
			break;

		case 7:  /* DATA_FORMAT_8_8 */

			elem_size = 1;
			num_elem = 2;
			break;

		case 1:  /* DATA_FORMAT_8 */

			elem_size = 1;
			num_elem = 1;
			break;

		default:
			EVG_ISA_ARG_NOT_SUPPORTED(W1.data_format);
		}
	}

	/* Address */
	addr = base_addr + evg_isa_read_gpr(work_item, W0.src_gpr, W0.src_rel,
		W0.src_sel_x, 0) * (num_elem * elem_size);
	evg_isa_debug("  t%d:read(%u)", work_item->id, addr);

	/* FIXME The number of bytes to read is defined by mega_fetch, but we currently
	 * cannot handle cases where num_elem*elem_size != mega_fetch */
	mem_read(evg_emu->global_mem, addr + W2.offset, num_elem * elem_size, value);

	/* Record global memory access */
	wavefront->global_mem_read = 1;
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = num_elem * elem_size;

	/* Write to each component of the GPR */
	for (i = 0; i < 4; i++)
	{
		/* Get index of read word to place in this GPR component */
		dst_sel_elem = dst_sel[i];
		switch (dst_sel_elem)
		{

		case 0:  /* SEL_X */
		case 1:  /* SEL_Y */
		case 2:  /* SEL_Z */
		case 3:  /* SEL_W */

			if (dst_sel_elem >= num_elem)
				EVG_ISA_ARG_NOT_SUPPORTED(dst_sel_elem);

			evg_isa_write_gpr(work_item, W1.dst_gpr, W1.dst_rel, i, value[dst_sel_elem]);
			if (debug_status(evg_isa_debug_category))
			{
				union evg_reg_t reg;
				reg.as_uint = value[dst_sel_elem];
				evg_isa_debug(" ");
				evg_inst_dump_gpr(W1.dst_gpr, W1.dst_rel, i, dst_sel_elem, debug_file(evg_isa_debug_category));
				evg_isa_debug("<=(%d,%gf)", reg.as_uint, reg.as_float);
			}
			break;

		case 7:
			/* SEL_MASK: mask this element */
			break;

		default:
			EVG_ISA_ARG_NOT_SUPPORTED(dst_sel[i]);
		}
	}
}
#undef W0
#undef W1
#undef W2


void evg_isa_GET_BUFFER_RESINFO_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SEMANTIC_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GATHER4_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GATHER4_C_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GATHER4_C_O_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GATHER4_O_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GET_GRADIENTS_H_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GET_GRADIENTS_V_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GET_LOD_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GET_NUMBER_OF_SAMPLES_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_GET_TEXTURE_RESINFO_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_KEEP_GRADIENTS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_LD_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


#define W0  EVG_TEX_WORD0
#define W1  EVG_TEX_WORD1
#define W2  EVG_TEX_WORD2
void evg_isa_SAMPLE_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	struct evg_wavefront_t *wavefront = work_item->wavefront;
	struct evg_ndrange_t *ndrange = work_item->ndrange;

	unsigned int base_addr;
	unsigned int pixel_size;

	union evg_reg_t addr;
	union evg_reg_t value;

	int dst_sel[4];
	int dst_sel_elem;
	int i;

	struct evg_opencl_mem_t *image;

	/* W0.tex_inst: <unknown> */
	/* W0.inst_mod: instruction modifier */
	/* W0.fwq (fetch_whole_quad): whether or not to fetch data for inactive work-items */
	/* W0.resource_id: surface (image?) id to read from */
	/* W0.src_gpr: source GPR to read from */
	/* W0.sr (src_rel): relative or absolute src_gpr */
	/* W0.ac (alt_const): whether the clause uses constants from alternative thread */
	/* W0.rim (resource_index_mode):  whether to add index0 or index1 to the resource ID */
	/* W0.sim (sampler_index_mode): whether to add index0 or index1 to the sampler ID */

	/* W1.dst_gpr: destination GPR to write result */
	/* W1.dr (dst_rel): relative or absolute dst_gpr */
	/* W1.{dsx,dsy,dsz,dsw}: specified which element of result to write into dst_gpr.{x,y,z,w} */
	/* W1.lod_bias: constant level-of-detail to add to the computed bias (twos-complement fixed-point value [-4,4))*/
	/* W1.{ctx,cty,ctz,ctw}: specifies type of source element ([un]normalized)*/

	/* W2.{offsetx,offsety,offsetz}: values added to address elements before sampling */
	/* W2.sampler_id: sampler ID */
	/* W2.{ssx,ssy,ssz,ssw}: specifies the element src */

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.tex_inst, 16); 
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.inst_mod, 0); /* Not used for SAMPLE */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.fwq, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.sr, 0); 
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.ac, 0); 
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.rim, 0); 
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.sim, 0); 
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W2.ssx, 0); 

	/* Look up the image object based on UAV */
	image = list_get(ndrange->kernel->uav_read_list, W0.resource_id);
	if(!image) 
		fatal("No table entry for read-only UAV %d\n", W0.resource_id);

	pixel_size = image->pixel_size;

	/* Calculate read address */
	base_addr = image->device_ptr;
	addr.as_uint = evg_isa_read_gpr(work_item, W0.src_gpr, 0, 0, 0);  /* FIXME Always reads from X */ 
	addr.as_uint = base_addr + (uint32_t) round(addr.as_float) * pixel_size;

	mem_read(evg_emu->global_mem, addr.as_uint, pixel_size, &value);

	evg_isa_debug("  t%d:read(%u)", work_item->id, addr.as_uint);
	evg_isa_debug("<=(%d,%gf) ", value.as_int, value.as_float);

	/* Do not fetch for inactive work_items */
	if (!evg_work_item_get_active(work_item))
		return;

	/* Store 'dst_sel_{x,y,z,w}' in array */
	dst_sel[0] = W1.dsx;
	dst_sel[1] = W1.dsy;
	dst_sel[2] = W1.dsz;
	dst_sel[3] = W1.dsw;
	
	/* Record global memory access */
	wavefront->global_mem_read = 1;
	work_item->global_mem_access_addr = addr.as_uint;
	work_item->global_mem_access_size = pixel_size;

	/* Write to each component of the GPR */
	for (i = 0; i < 4; i++) {

		/* Get index of read word to place in this GPR component */
		dst_sel_elem = dst_sel[i];
		switch (dst_sel_elem) {

		case 0:  /* SEL_X */
		case 1:  /* SEL_Y */
		case 2:  /* SEL_Z */
		case 3:  /* SEL_W */

			if (dst_sel_elem >= image->num_channels_per_pixel)
				EVG_ISA_ARG_NOT_SUPPORTED(dst_sel_elem);
			evg_isa_write_gpr(work_item, W1.dst_gpr, W1.dr, i, value.as_uint);
			if (debug_status(evg_isa_debug_category))
			{
				evg_isa_debug(" ");
				evg_inst_dump_gpr(W1.dst_gpr, W1.dr, i, dst_sel_elem, debug_file(evg_isa_debug_category));
				evg_isa_debug("<=(%d,%gf)", value.as_int, value.as_float);
			}
			break;

		case 7:
			/* SEL_MASK: mask this element */
			break;

		default:
			EVG_ISA_ARG_NOT_SUPPORTED(dst_sel[i]);
		}
	}
}
#undef W0
#undef W1
#undef W2


void evg_isa_SAMPLE_C_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_C_G_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_C_G_LB_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_C_L_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_C_LB_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_C_LZ_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_G_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_G_LB_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_L_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_LB_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SAMPLE_LZ_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_GRADIENTS_H_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_GRADIENTS_V_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}


void evg_isa_SET_TEXTURE_OFFSETS_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}

void evg_isa_MEM_impl(struct evg_work_item_t *work_item, struct evg_inst_t *inst)
{
	__EVG_NOT_IMPL__
}

