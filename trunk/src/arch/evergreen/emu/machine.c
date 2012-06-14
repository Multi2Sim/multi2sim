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

#include <math.h>

#include <evergreen-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


char *err_evg_isa_note =
	"\tThe AMD Evergreen instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	evg_isa_inst->info->name, err_evg_isa_note)


/*
 * CF Instructions
 */

#define W0  EVG_CF_ALU_WORD0
#define W1  EVG_CF_ALU_WORD1
void evg_isa_ALU_impl()
{
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.alt_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	/* FIXME: block constant cache sets */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */

	/* Start ALU clause */
	evg_isa_wavefront->clause_buf_start = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
	evg_isa_wavefront->clause_buf_end = evg_isa_wavefront->clause_buf_start + (W1.count + 1) * 8;
	evg_isa_wavefront->clause_buf = evg_isa_wavefront->clause_buf_start;
	evg_isa_wavefront->clause_kind = EVG_CLAUSE_ALU;
	evg_isa_alu_clause_start();
}
#undef W0
#undef W1


void evg_isa_ALU_BREAK_impl()
{
	/* Same behavior as ALU */
	/* FIXME: what's the difference? */
	evg_isa_ALU_impl();
}


void evg_isa_ALU_POP_AFTER_impl()
{
	/* Same behavior as ALU */
	evg_isa_ALU_impl();
}

void evg_isa_ALU_POP2_AFTER_impl()
{
	/* Initiates an ALU clause, and pops the stack twice after the clause completes execution. */
	NOT_IMPL();
}

void evg_isa_ALU_PUSH_BEFORE_impl()
{
	/* Same behavior as ALU inst */
	evg_isa_ALU_impl();
}

void evg_isa_CALL_impl()
{
	/* Execute a subroutine call (push call variables onto stack) */
	NOT_IMPL();
}

void evg_isa_CALL_FS_impl()
{
	/* Call Fetch Subroutine */
	NOT_IMPL();
}

void evg_isa_CUT_VERTEX_impl()
{
	/* End Primitive Strip, Start New Primitive Strip */
	NOT_IMPL();
}

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_ELSE_impl()
{
	int active, active_last, active_new;
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
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:act=", evg_isa_wavefront->name);
		bit_map_dump(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Invert active mask */
	if (!evg_isa_wavefront->stack_top)
		fatal("ELSE: cannot execute for stack_top=0");
	for (i = 0; i < evg_isa_wavefront->work_item_count; i++) {
		active = bit_map_get(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count + i, 1);
		active_last = bit_map_get(evg_isa_wavefront->active_stack, (evg_isa_wavefront->stack_top - 1) *
			evg_isa_wavefront->work_item_count + i, 1);
		active_new = !active && active_last;
		active_count += active_new;
		bit_map_set(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count + i, 1, active_new);
	}
	
	/* Debug */
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:invert(act)=", evg_isa_wavefront->name);
		bit_map_dump(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* If all pixels are inactive, pop stack and jump */
	if (!active_count) {
		evg_wavefront_stack_pop(evg_isa_wavefront, W1.pop_count);
		evg_isa_wavefront->cf_buf = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1

void evg_isa_EMIT_CUT_VERTEX_impl()
{
	/* Emit Vertex, End Primitive Strip */
	NOT_IMPL();
}

void evg_isa_EMIT_VERTEX_impl()
{
	/* Vertex Exported to Memory */
	NOT_IMPL();
}

void evg_isa_EXPORT_impl()
{
	/* Export from VS or PS */
	NOT_IMPL();
}

void evg_isa_EXPORT_DONE_impl()
{
	/* Export Last Data */
	NOT_IMPL();
}

void evg_isa_GDS_impl()
{
	/* Global Data Share */
	NOT_IMPL();
}

void evg_isa_GWS_BARRIER_impl()
{
	/* Global Wavefront Barrier */
	NOT_IMPL();
}

void evg_isa_GWS_INIT_impl()
{
	/* Global Wavefront Resource Initialization */
	NOT_IMPL();
}

void evg_isa_GWS_SEMA_P_impl()
{
	/* Global Wavefront Sync Semaphore P */
	NOT_IMPL();
}

void evg_isa_GWS_SEMA_V_impl()
{
	/* Global Wavefront Sync Semaphore V */
	NOT_IMPL();
}

void evg_isa_HALT_impl()
{
	/* Halt Wavefront Execution */
	NOT_IMPL();
}

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_JUMP_impl()
{
	int active_count;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* If all pixels are inactive, pop stack and jump */
	active_count = bit_map_count_ones(evg_isa_wavefront->active_stack,
		evg_isa_wavefront->stack_top * evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count);
	if (!active_count) {
		evg_wavefront_stack_pop(evg_isa_wavefront, W1.pop_count);
		evg_isa_wavefront->cf_buf = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
	}
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_JUMPTABLE_impl()
{
	/* Executes a jump through a jump table. */
	NOT_IMPL();
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_KILL_impl()
{
	/* Kill (prevent rendering of) pixels that pass a condition test. */
	NOT_IMPL();
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_END_impl()
{
	int active_count;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if any pixel is active */

	/* Increment the trip count */
	++evg_isa_wavefront->loop_trip_count;

	/* Dump current loop state */
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:act=", evg_isa_wavefront->name);
		bit_map_dump(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Decrement loop index */
	evg_isa_wavefront->loop_index -= evg_isa_wavefront->loop_step;

	/* If any pixel is active and loop index is not zero, jump back */
	active_count = bit_map_count_ones(evg_isa_wavefront->active_stack,
		evg_isa_wavefront->stack_top * evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count);
	if (active_count && (evg_isa_wavefront->loop_index == 0) && 
		(evg_isa_wavefront->loop_trip_count != evg_isa_wavefront->loop_max_trip_count)) 
	{
		evg_isa_wavefront->cf_buf = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
		return;
	}

	/* Pop stack once */
	evg_wavefront_stack_pop(evg_isa_wavefront, 1);

	/* FIXME: Get rid of this once loop state is pushed on the stack */
	--evg_isa_wavefront->loop_depth;
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_START_DX10_impl()
{
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);  /* FIXME: what does it refer to? */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);

	/* W0.addr: jump if all pixels fail. */
	/* W0.pop_count: pop if all pixels fail. */

	/* Initialize the loop state for the wavefront */
	evg_isa_wavefront->loop_max_trip_count = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][0];
	evg_isa_wavefront->loop_start = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][1];
	evg_isa_wavefront->loop_step = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][2];

	evg_isa_wavefront->loop_index = evg_isa_wavefront->loop_start;
	evg_isa_wavefront->loop_trip_count = 0;


	/* FIXME: if initial condition fails, jump to 'addr' */

	/* FIXME: Push loop state */

	/* FIXME: Set up new loop state */

	/* FIXME: Remove this once loop state is part of stack */
	++evg_isa_wavefront->loop_depth;

	/* FIXME: Push active mask? */
	evg_wavefront_stack_push(evg_isa_wavefront);
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_START_NO_AL_impl()
{
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);

	/* FIXME: Remove this once loop state is part of stack */
	++evg_isa_wavefront->loop_depth;
	if(evg_isa_wavefront->loop_depth > 1) {		
		fatal("Nested loops not supported");
	}

	/* Initialize the loop state for the wavefront */
	evg_isa_wavefront->loop_max_trip_count = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][0];
	evg_isa_wavefront->loop_start = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][1];
	evg_isa_wavefront->loop_step = 
	   evg_isa_ndrange->kernel->bin_file->enc_dict_entry_evergreen->consts->int_consts[W1.cf_const][2];

	evg_isa_wavefront->loop_index = evg_isa_wavefront->loop_start;
	evg_isa_wavefront->loop_trip_count = 0;

	evg_wavefront_stack_push(evg_isa_wavefront);
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_CONTINUE_impl()
{
	NOT_IMPL();
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_LOOP_BREAK_impl()
{
	int active_count;
	int active;
	int i;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_RANGE(W1.cond, 0, 1);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
 
	/* W0.addr: jump if all pixels are disabled  */

	/* Dump current loop state */
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:act=", evg_isa_wavefront->name);
		bit_map_dump(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count,
			debug_file(evg_isa_debug_category));
	}

	/* Mark active work items as inactive */
	for (i = 0; i < evg_isa_wavefront->work_item_count; i++) {

		active = bit_map_get(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
			evg_isa_wavefront->work_item_count + i, 1);

		if (active) 
		{
			/* Pixel is active, so mark it as inactive */
			bit_map_set(evg_isa_wavefront->active_stack, evg_isa_wavefront->stack_top *
				evg_isa_wavefront->work_item_count + i, 1, 0);
		}
	}

	/* If no pixels are active, jump to addr */
	active_count = bit_map_count_ones(evg_isa_wavefront->active_stack,
		evg_isa_wavefront->stack_top * evg_isa_wavefront->work_item_count, evg_isa_wavefront->work_item_count);
	if (active_count == 0) {
		evg_isa_wavefront->cf_buf = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
		return;
	}

	/* Pop stack */
	evg_wavefront_stack_pop(evg_isa_wavefront, W1.pop_count);

	/* FIXME: Get rid of this once loop state is pushed on the stack */
	evg_isa_wavefront->loop_depth -= W1.pop_count;
}
#undef W0
#undef W1

void evg_isa_MEM_EXPORT_impl()
{
	/* Performs a memory read or write on the scatter buffer. */
	NOT_IMPL();
}

void evg_isa_MEM_EXPORT_COMBINED_impl()
{
	/* Performs a memory read or write on the scatter buffer. */
	NOT_IMPL();
}

#define W0  EVG_CF_ALLOC_EXPORT_WORD0_RAT
#define W1  EVG_CF_ALLOC_EXPORT_WORD1_BUF
void evg_isa_MEM_RAT_impl()
{
	switch (W0.rat_inst) {

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
		evg_isa_wavefront->global_mem_write = 1;

		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(evg_isa_wavefront, work_item_id)
		{
			evg_isa_work_item = evg_isa_ndrange->work_items[work_item_id];

			/* If VPM is set, do not export for inactive pixels. */
			if (W1.valid_pixel_mode && !evg_work_item_get_active(evg_isa_work_item))
				continue;

			/* W0.rw_gpr: GPR register from which to read data */
			/* W0.rr: relative/absolute rw_gpr */
			/* W0.index_gpr: GPR containing buffer coordinates. It is multiplied by (elem_size+1) */
			/* W0.elem_size: number of doublewords per array element, minus one */
			/* W1.array_size: array size (elem-size units) */

			/* This is a write, so we need to get the correct address based on the UAV number */
			uav = W0.rat_id;

			/* Otherwise, we have an image, and we need to provide a base address */
			mem = list_get(evg_isa_ndrange->kernel->uav_write_list, uav);
			base_addr = mem->device_ptr;
			addr = base_addr + evg_isa_read_gpr(W0.index_gpr, 0, 0, 0) * 4;  /* FIXME: only 1D - X coordinate, FIXME: x4? */
			evg_isa_debug("  t%d:write(0x%x)", evg_isa_work_item->id, addr);

			/* Record access */
			evg_isa_work_item->global_mem_access_addr = addr;
			evg_isa_work_item->global_mem_access_size = 0;

			for (i = 0; i < 4; i++) {
				
				/* If component is masked, skip */
				if (!(W1.comp_mask & (1 << i)))
					continue;

				/* FIXME We only support single-channel images, and don't know
				 * how the ISA knows which components to write */
				if(i != 0) 
					continue;

				/* Record size for memory access (warning: this is done inaccurately by assuming a
				 * baseline access and a contiguous set of accessed elements */
				evg_isa_work_item->global_mem_access_size += 4;

				/* Access */
				value.as_uint = evg_isa_read_gpr(W0.rw_gpr, W0.rr, i, 0);

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
void evg_isa_MEM_RAT_CACHELESS_impl()
{
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

		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(evg_isa_wavefront, work_item_id)
		{
			evg_isa_work_item = evg_isa_ndrange->work_items[work_item_id];

			/* If VPM is set, do not export for inactive pixels. */
			if (W1.valid_pixel_mode && !evg_work_item_get_active(evg_isa_work_item))
				continue;

			/* W0.rw_gpr: GPR register from which to read data */
			/* W0.rr: relative/absolute rw_gpr */
			/* W0.index_gpr: GPR containing buffer coordinates. It is multiplied by (elem_size+1) */
			/* W0.elem_size: number of doublewords per array element, minus one */
			/* W1.array_size: array size (elem-size units) */

			/* This is a write, so we need to get the correct address based on the UAV number */
			addr = evg_isa_read_gpr(W0.index_gpr, 0, 0, 0) * 4;  /* FIXME: only 1D - X coordinate, FIXME: x4? */
			evg_isa_debug("  t%d:write(0x%x)", evg_isa_work_item->id, addr);

			/* Record access */
			evg_isa_wavefront->global_mem_write = 1;
			evg_isa_work_item->global_mem_access_addr = addr;
			evg_isa_work_item->global_mem_access_size = 0;

			for (i = 0; i < 4; i++)
			{
				/* If component is masked, skip */
				if (!(W1.comp_mask & (1 << i)))
					continue;

				/* Record size for memory access (warning: this is done inaccurately by assuming a
				 * baseline access and a contiguous set of accessed elements */
				evg_isa_work_item->global_mem_access_size += 4;

				/* Access */
				value.as_uint = evg_isa_read_gpr(W0.rw_gpr, W0.rr, i, 0);

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

void evg_isa_MEM_RAT_COMBINED_CACHELESS_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_RING_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_RING1_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_RING2_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_RING3_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM0_BUF0_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM0_BUF1_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM0_BUF2_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM0_BUF3_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM1_BUF0_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM1_BUF1_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM1_BUF2_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM1_BUF3_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM2_BUF0_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM2_BUF1_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM2_BUF2_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM2_BUF3_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM3_BUF0_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM3_BUF1_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM3_BUF2_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_STREAM3_BUF3_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_WR_SCRATCH_impl()
{
	NOT_IMPL();
}

#define W0 evg_isa_inst->words[0].cf_word0
#define W1 evg_isa_inst->words[1].cf_word1
void evg_isa_NOP_CF_impl()
{
}
#undef W0
#undef W1


#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_POP_impl()
{
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cf_const, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.count, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.valid_pixel_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.whole_quad_mode, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.barrier, 1);

	/* Pop 'pop_count' from stack and jump to 'addr' */
	evg_wavefront_stack_pop(evg_isa_wavefront, W1.pop_count);
	evg_isa_wavefront->cf_buf = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_PUSH_impl()
{
	/* Push State To Stack */
	NOT_IMPL();
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_RETURN_impl()
{
	/* Return From Subroutine */
	NOT_IMPL();
}
#undef W0
#undef W1

#define W0  EVG_CF_WORD0
#define W1  EVG_CF_WORD1
void evg_isa_TC_impl()
{
	int active;
	int i;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.jump_table_sel, 0);  /* ignored for this instruction */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.pop_count, 0);  /* number of entries to pop from stack */
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.cond, 0);  /* how to evaluate condition test for pixels (ACTIVE) */
	/* FIXME: valid_pixel_mode */
	/* FIXME: whole_quad_mode */
	/* FIXME: barrier */
	
	/* Start TC clause */
	evg_isa_wavefront->clause_buf_start = evg_isa_wavefront->cf_buf_start + W0.addr * 8;
	evg_isa_wavefront->clause_buf_end = evg_isa_wavefront->clause_buf_start + (W1.count + 1) * 16;
	evg_isa_wavefront->clause_buf = evg_isa_wavefront->clause_buf_start;
	evg_isa_wavefront->clause_kind = EVG_CLAUSE_TEX;
	evg_isa_tc_clause_start();

	/* If VPM is set, copy 'active' mask at the top of the stack to 'pred' mask.
	 * This will make all fetches within the clause happen only for active pixels.
	 * If VPM is clear, copy a mask set to ones. */
	for (i = 0; i < evg_isa_wavefront->work_item_count; i++) {
		active = W1.valid_pixel_mode ? bit_map_get(evg_isa_wavefront->active_stack,
			evg_isa_wavefront->stack_top * evg_isa_wavefront->work_item_count + i, 1) : 1;
		bit_map_set(evg_isa_wavefront->pred, i, 1, active);
	}
}
#undef W0
#undef W1

void evg_isa_TC_ACK_impl()
{
	/* Fetch Clause Through Texture Cache With ACK */
	NOT_IMPL();
}

void evg_isa_VC_impl()
{
	/* Initiate Clause of Vertex or Constant Fetches Through Vertex Cache */
	NOT_IMPL();
}

void evg_isa_VC_ACK_impl()
{
	/* Fetch Clause Through Vertex Cache With ACK */
	NOT_IMPL();
}

void evg_isa_WAIT_ACK_impl()
{
	/* FIXME: wait for Write ACKs */
}


void evg_isa_ADD_impl()
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	dst = src0 + src1;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MUL_impl()
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MUL_IEEE_impl()
{
	float src0, src1, dst;

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MAX_impl()
{
	NOT_IMPL();
}


void evg_isa_MIN_impl()
{
	NOT_IMPL();
}


void evg_isa_MAX_DX10_impl()
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MIN_DX10_impl()
{
	float src0, src1, dst;

	/* FIXME: This instruction uses the DirectX 10 method of handling of NaNs
	 * How? */
	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest_float(dst);
}


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETE_impl()
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 == src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGT_impl()
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 > src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGE_impl()
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 >= src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETNE_impl()
{
	float src0, src1;
	float dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 != src1;
	dst = cond ? 1.0f : 0.0f;
	evg_isa_enqueue_write_dest_float(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETE_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 == src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGT_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGE_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETNE_DX10_impl()
{
	float src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


void evg_isa_FRACT_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = fmodf(src, 1.0f);
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_TRUNC_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = truncf(src);
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_CEIL_impl()
{
	NOT_IMPL();
}


void evg_isa_RNDNE_impl()
{
	NOT_IMPL();
}


void evg_isa_FLOOR_impl() 
{
	float src0, dst;

	src0 = evg_isa_read_op_src_int(0);
	
	dst = trunc(src0);
	if((src0 < 0.0f) && (src0 != dst))
		dst += -1.0f;

	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_ASHR_INT_impl()
{
	int32_t src0, dst;
	uint32_t src1;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	if (src1 > 31)
		dst = src0 < 0 ? -1 : 0;
	else
		dst = src0 >> src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_LSHR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 >> src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_LSHL_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 << src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MOV_impl()
{
	uint32_t value;
	value = evg_isa_read_op_src_int(0);
	evg_isa_enqueue_write_dest(value);
}


void evg_isa_NOP_ALU_impl()
{
}


void evg_isa_MUL_64_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT64_TO_FLT32_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT32_TO_FLT64_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGT_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGE_UINT_impl()
{
	NOT_IMPL();
}


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETE_impl()
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 == src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETGT_impl()
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 > src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETGE_impl()
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 >= src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETNE_impl()
{
	float src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	cond = src0 != src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


void evg_isa_PRED_SET_INV_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SET_POP_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SET_CLR_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SET_RESTORE_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETE_PUSH_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGT_PUSH_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGE_PUSH_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETNE_PUSH_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLE_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLGT_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLGE_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLNE_impl()
{
	NOT_IMPL();
}


void evg_isa_AND_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 & src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_OR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 | src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_XOR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 ^ src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_NOT_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_ADD_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 + src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_SUB_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 - src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MAX_INT_impl()
{
	int32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MIN_INT_impl()
{
	int32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MAX_UINT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 > src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MIN_UINT_impl()
{
	uint32_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 < src1 ? src0 : src1;
	evg_isa_enqueue_write_dest(dst);
}


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETE_INT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 == src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGT_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGE_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETNE_INT_impl()
{
	int32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 != src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1




#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGT_UINT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 > src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_SETGE_UINT_impl()
{
	uint32_t src0, src1;
	int32_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 >= src1;
	dst = cond ? -1 : 0;
	evg_isa_enqueue_write_dest(dst);
}
#undef W0
#undef W1


void evg_isa_KILLGT_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLGE_UINT_impl()
{
	NOT_IMPL();
}


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PREDE_INT_impl()
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 == src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETGE_INT_impl()
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 >= src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETGT_INT_impl()
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 > src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


#define W0 evg_isa_inst->words[0].alu_word0
#define W1 evg_isa_inst->words[1].alu_word1_op2
void evg_isa_PRED_SETNE_INT_impl()
{
	int32_t src0, src1;
	union evg_reg_t dst;
	int cond;

	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.omod, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.bank_swizzle, 0);
	EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.clamp, 0);

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	cond = src0 != src1;
	dst.as_float = cond ? 0.0f : 1.0f;
	evg_isa_enqueue_write_dest(dst.as_uint);

	/* Active masks */
	evg_isa_enqueue_push_before();
	evg_isa_enqueue_pred_set(cond);
}
#undef W0
#undef W1


void evg_isa_KILLE_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLGT_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLGE_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_KILLNE_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETE_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGT_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGE_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETNE_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETLT_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETLE_PUSH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT_TO_INT_impl()
{
	float src;
	int32_t dst;
	
	src = evg_isa_read_op_src_float(0);
	if (isinf(src) == 1)
		dst = INT32_MAX;
	else if (isinf(src) == -1)
		dst = INT32_MIN;
	else if (isnan(src))
		dst = 0;
	else
		dst = src;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_BFREV_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_ADDC_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_SUBB_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_GROUP_BARRIER_impl()
{
	/* Only the first work-item in a wavefront handles barriers */
	if (evg_isa_work_item->id_in_wavefront)
		return;
	
	/* Suspend current wavefront at the barrier */
	assert(DOUBLE_LINKED_LIST_MEMBER(evg_isa_work_group, running, evg_isa_wavefront));
	DOUBLE_LINKED_LIST_REMOVE(evg_isa_work_group, running, evg_isa_wavefront);
	DOUBLE_LINKED_LIST_INSERT_TAIL(evg_isa_work_group, barrier, evg_isa_wavefront);
	evg_isa_debug("%s (gid=%d) reached barrier (%d reached, %d left)\n",
		evg_isa_wavefront->name, evg_isa_work_group->id, evg_isa_work_group->barrier_list_count,
		evg_isa_work_group->wavefront_count - evg_isa_work_group->barrier_list_count);
	
	/* If all wavefronts in work-group reached the barrier, wake them up */
	if (evg_isa_work_group->barrier_list_count == evg_isa_work_group->wavefront_count)
	{
		struct evg_wavefront_t *wavefront;
		while (evg_isa_work_group->barrier_list_head)
		{
			wavefront = evg_isa_work_group->barrier_list_head;
			DOUBLE_LINKED_LIST_REMOVE(evg_isa_work_group, barrier, wavefront);
			DOUBLE_LINKED_LIST_INSERT_TAIL(evg_isa_work_group, running, wavefront);
		}
		assert(evg_isa_work_group->running_list_count == evg_isa_work_group->wavefront_count);
		assert(evg_isa_work_group->barrier_list_count == 0);
		evg_isa_debug("%s completed barrier, waking up wavefronts\n",
			evg_isa_work_group->name);
	}
}


void evg_isa_GROUP_SEQ_BEGIN_impl()
{
	NOT_IMPL();
}


void evg_isa_GROUP_SEQ_END_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_MODE_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_CF_IDX0_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_CF_IDX1_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_LDS_SIZE_impl()
{
	NOT_IMPL();
}


void evg_isa_EXP_IEEE_impl()
{
	NOT_IMPL();
}


void evg_isa_LOG_CLAMPED_impl()
{
	NOT_IMPL();
}


void evg_isa_LOG_IEEE_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIP_CLAMPED_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIP_FF_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIP_IEEE_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = 1.0f / src;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_RECIPSQRT_CLAMPED_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIPSQRT_FF_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIPSQRT_IEEE_impl()
{
	NOT_IMPL();
}


void evg_isa_SQRT_IEEE_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = sqrtf(src);
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_SIN_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = sinf(src);
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_COS_impl()
{
	float src, dst;

	src = evg_isa_read_op_src_float(0);
	dst = cosf(src);
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MULLO_INT_impl()
{
	int64_t src0, src1, dst;

	src0 = (int32_t) evg_isa_read_op_src_int(0);
	src1 = (int32_t) evg_isa_read_op_src_int(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MULHI_INT_impl()
{
	int64_t src0, src1, dst;

	src0 = (int32_t) evg_isa_read_op_src_int(0);
	src1 = (int32_t) evg_isa_read_op_src_int(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(dst >> 32);
}


void evg_isa_MULLO_UINT_impl()
{
	uint64_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MULHI_UINT_impl()
{
	uint64_t src0, src1, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	dst = src0 * src1;
	evg_isa_enqueue_write_dest(dst >> 32);
}


void evg_isa_RECIP_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIP_UINT_impl()
{
	uint32_t src;
	uint32_t dst;

	src = evg_isa_read_op_src_int(0);
	dst = 0xffffffff / src;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_RECIP_64_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIP_CLAMPED_64_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIPSQRT_64_impl()
{
	NOT_IMPL();
}


void evg_isa_RECIPSQRT_CLAMPED_64_impl()
{
	NOT_IMPL();
}


void evg_isa_SQRT_64_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT_TO_UINT_impl()
{
	float src;
	uint32_t dst;
	
	src = evg_isa_read_op_src_float(0);
	if (isinf(src) == 1)
		dst = UINT32_MAX;
	else if (isinf(src) == -1 || isnan(src))
		dst = 0;
	else
		dst = src;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_INT_TO_FLT_impl()
{
	int32_t src0;
	float dst;

	src0 = evg_isa_read_op_src_int(0);
	dst = (float) src0;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_UINT_TO_FLT_impl()
{
	uint32_t src0;
	float dst;

	src0 = evg_isa_read_op_src_int(0);
	dst = (float) src0;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_BFM_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT32_TO_FLT16_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT16_TO_FLT32_impl()
{
	NOT_IMPL();
}


void evg_isa_UBYTE0_FLT_impl()
{
	NOT_IMPL();
}


void evg_isa_UBYTE1_FLT_impl()
{
	NOT_IMPL();
}


void evg_isa_UBYTE2_FLT_impl()
{
	NOT_IMPL();
}


void evg_isa_UBYTE3_FLT_impl()
{
	NOT_IMPL();
}


void evg_isa_BCNT_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FFBH_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_FFBL_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FFBH_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT_TO_UINT4_impl()
{
	NOT_IMPL();
}


void evg_isa_DOT_IEEE_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT_TO_INT_RPI_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT_TO_INT_FLOOR_impl()
{
	NOT_IMPL();
}


void evg_isa_MULHI_UINT24_impl()
{
	NOT_IMPL();
}


void evg_isa_MBCNT_32HI_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_OFFSET_TO_FLT_impl()
{
	NOT_IMPL();
}


void evg_isa_MUL_UINT24_impl()
{
	NOT_IMPL();
}


void evg_isa_BCNT_ACCUM_PREV_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_MBCNT_32LO_ACCUM_PREV_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_SETE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_SETNE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_SETGT_64_impl()
{
	NOT_IMPL();
}


void evg_isa_SETGE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_MIN_64_impl()
{
	NOT_IMPL();
}


void evg_isa_MAX_64_impl()
{
	NOT_IMPL();
}


void evg_isa_DOT4_impl()
{
	NOT_IMPL();
}


void evg_isa_DOT4_IEEE_impl()
{
	NOT_IMPL();
}


void evg_isa_CUBE_impl()
{
	NOT_IMPL();
}


void evg_isa_MAX4_impl()
{
	NOT_IMPL();
}


void evg_isa_FREXP_64_impl()
{
	NOT_IMPL();
}


void evg_isa_LDEXP_64_impl()
{
	NOT_IMPL();
}


void evg_isa_FRACT_64_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGT_64_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_PRED_SETGE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_MUL_64_VEC_impl()
{
	NOT_IMPL();
}


void evg_isa_ADD_64_impl()
{
	NOT_IMPL();
}


void evg_isa_MOVA_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT64_TO_FLT32_VEC_impl()
{
	NOT_IMPL();
}


void evg_isa_FLT32_TO_FLT64_VEC_impl()
{
	NOT_IMPL();
}


void evg_isa_SAD_ACCUM_PREV_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_DOT_impl()
{
	NOT_IMPL();
}


void evg_isa_MUL_PREV_impl()
{
	NOT_IMPL();
}


void evg_isa_MUL_IEEE_PREV_impl()
{
	NOT_IMPL();
}


void evg_isa_ADD_PREV_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_PREV_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_IEEE_PREV_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_XY_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_ZW_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_X_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_Z_impl()
{
	NOT_IMPL();
}


void evg_isa_STORE_FLAGS_impl()
{
	NOT_IMPL();
}


void evg_isa_LOAD_STORE_FLAGS_impl()
{
	NOT_IMPL();
}


void evg_isa_LDS_1A_impl()
{
	NOT_IMPL();
}


void evg_isa_LDS_1A1D_impl()
{
	NOT_IMPL();
}


void evg_isa_LDS_2A_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_LOAD_P0_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_LOAD_P10_impl()
{
	NOT_IMPL();
}


void evg_isa_INTERP_LOAD_P20_impl()
{
	NOT_IMPL();
}


void evg_isa_BFE_UINT_impl() 
{
	uint32_t src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);

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

	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_BFE_INT_impl()
{

        uint32_t src0, src1, src2, dst;

        src0 = evg_isa_read_op_src_int(0);
        src1 = evg_isa_read_op_src_int(1);
        src2 = evg_isa_read_op_src_int(2);

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

        evg_isa_enqueue_write_dest(dst);
}


void evg_isa_BFI_INT_impl()
{

	uint32_t src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);

	/* Documentation says: dst = (src1 & src0) | (src2 & -src0)
	 * Though probably it means '~src0'. */
	dst = (src1 & src0) | (src2 & ~src0);
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_FMA_impl()
{
	NOT_IMPL();
}


void evg_isa_CNDNE_64_impl()
{
	NOT_IMPL();
}


void evg_isa_FMA_64_impl()
{
	NOT_IMPL();
}


void evg_isa_LERP_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_BIT_ALIGN_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_BYTE_ALIGN_INT_impl()
{
	NOT_IMPL();
}


void evg_isa_SAD_ACCUM_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_SAD_ACCUM_HI_UINT_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_UINT24_impl()
{
	NOT_IMPL();
}


#define W0 evg_isa_inst->words[0].alu_word0_lds_idx_op
#define W1 evg_isa_inst->words[1].alu_word1_lds_idx_op
void evg_isa_LDS_IDX_OP_impl()
{
	struct evg_wavefront_t *wavefront = evg_isa_wavefront;
	struct evg_work_item_t *work_item = evg_isa_work_item;
	struct mem_t *local_mem = evg_isa_work_group->local_mem;
	unsigned int idx_offset;
	uint32_t op0, op1, op2;

	/* Recompose 'idx_offset' field */
	idx_offset = (W0.idx_offset_5 << 5) | (W0.idx_offset_4 << 4) |
		(W1.idx_offset_3 << 3) | (W1.idx_offset_2 << 2) |
		(W1.idx_offset_1 << 1) | W1.idx_offset_0;
	
	/* Read operands */
	op0 = evg_isa_read_op_src_int(0);
	op1 = evg_isa_read_op_src_int(1);
	op2 = evg_isa_read_op_src_int(2);

	/* Process LDS instruction */
	switch (W1.lds_op) {

	/* DS_INST_WRITE: 1A1D WRITE(dst,src) : DS(dst) = src */
	case 13:
	{
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W0.pred_sel, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(op2, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(idx_offset, 0);
		EVG_ISA_ARG_NOT_SUPPORTED_NEQ(W1.dst_chan, 0);
		/* FIXME: dst_chan? Does address need to be multiplied? */
		evg_isa_enqueue_write_lds(op0, op1, 4);

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
		evg_isa_enqueue_write_lds(dst, src0, 4);
		evg_isa_enqueue_write_lds(tmp, src1, 4);  /* FIXME: correct? */

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

		evg_isa_enqueue_write_lds(dst, src, 1);

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

		evg_isa_enqueue_write_lds(dst, src, 2);

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

		value_ptr = malloc(4);
		if (!value_ptr)
			fatal("%s: out of memory", __FUNCTION__);

		mem_read(local_mem, op0, 4, value_ptr);
		list_enqueue(evg_isa_work_item->lds_oqa, value_ptr);
		evg_isa_debug("  t%d:LDS[0x%x]=(%u,%gf)=>OQA",
			evg_isa_work_item->id, op0, value_ptr->as_uint, value_ptr->as_float);

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

		value_ptr = malloc(4);
		if (!value_ptr)
			fatal("%s: out of memory", __FUNCTION__);

		mem_read(local_mem, op0, 4, value_ptr);
		list_enqueue(evg_isa_work_item->lds_oqa, value_ptr);

		value_ptr = malloc(4);
		if (!value_ptr)
			fatal("%s: out of memory", __FUNCTION__);

		mem_read(local_mem, op1, 4, value_ptr);
		list_enqueue(evg_isa_work_item->lds_oqb, value_ptr);

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

		pvalue_se = malloc(4);
		if (!pvalue_se)
			fatal("%s: out of memory", __FUNCTION__);

		*pvalue_se = value;
		list_enqueue(evg_isa_work_item->lds_oqa, pvalue_se);

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

		pvalue_24h0 = malloc(4);
		if (!pvalue_24h0)
			fatal("%s: out of memory", __FUNCTION__);

		*pvalue_24h0 = value;
		list_enqueue(evg_isa_work_item->lds_oqa, pvalue_24h0);

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

		pvalue_se = malloc(4);
		if (!pvalue_se)
			fatal("%s: out of memory", __FUNCTION__);

		*pvalue_se = value;
		evg_isa_debug("  t%d: %d (pvalue = %d)", evg_isa_work_item->id, value, *pvalue_se);
		list_enqueue(evg_isa_work_item->lds_oqa, pvalue_se);

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

		pvalue_16h0 = malloc(4);
		if (!pvalue_16h0)
			fatal("%s: out of memory", __FUNCTION__);

		*pvalue_16h0 = value;
		list_enqueue(evg_isa_work_item->lds_oqa, pvalue_16h0);

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


void evg_isa_MULADD_impl()
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	src2 = evg_isa_read_op_src_float(2);
	dst = src0 * src1 + src2;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_MULADD_M2_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_M4_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_D2_impl()
{
	NOT_IMPL();
}


void evg_isa_MULADD_IEEE_impl()
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_float(0);
	src1 = evg_isa_read_op_src_float(1);
	src2 = evg_isa_read_op_src_float(2);
	dst = src0 * src1 + src2;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_CNDE_impl()
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 == 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_CNDGT_impl()
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 > 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_CNDGE_impl()
{
	float src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 >= 0.0f ? src1 : src2;
	evg_isa_enqueue_write_dest_float(dst);
}


void evg_isa_CNDE_INT_impl()
{
	uint32_t src0, src1, src2, dst;

	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 == 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_CNDGE_INT_impl()
{
	int src0, src1, src2, dst;
	
	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 >= 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_CNDGT_INT_impl()
{
	int src0, src1, src2, dst;
	
	src0 = evg_isa_read_op_src_int(0);
	src1 = evg_isa_read_op_src_int(1);
	src2 = evg_isa_read_op_src_int(2);
	dst = src0 > 0 ? src1 : src2;
	evg_isa_enqueue_write_dest(dst);
}


void evg_isa_MUL_LIT_impl()
{
	NOT_IMPL();
}


#define W0  EVG_VTX_WORD0
#define W1  EVG_VTX_WORD1_GPR
#define W2  EVG_VTX_WORD2
void evg_isa_FETCH_impl()
{
	uint32_t addr;
	uint32_t base_addr;
	int data_format;
	int dst_sel[4], dst_sel_elem;
	int i;
	uint32_t value[4];

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
	if (!evg_work_item_get_active(evg_isa_work_item))
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
			mem = list_get(evg_isa_ndrange->kernel->constant_buffer_list, W0.buffer_id-128);
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
	addr = base_addr + evg_isa_read_gpr(W0.src_gpr, W0.src_rel, W0.src_sel_x, 0) * (num_elem*elem_size);
	evg_isa_debug("  t%d:read(%u)", evg_isa_work_item->id, addr);

	/* FIXME The number of bytes to read is defined by mega_fetch, but we currently
	 * cannot handle cases where num_elem*elem_size != mega_fetch */
	mem_read(evg_emu->global_mem, addr + W2.offset, num_elem * elem_size, value);

	/* Record global memory access */
	evg_isa_wavefront->global_mem_read = 1;
	evg_isa_work_item->global_mem_access_addr = addr;
	evg_isa_work_item->global_mem_access_size = num_elem * elem_size;

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

			evg_isa_write_gpr(W1.dst_gpr, W1.dst_rel, i, value[dst_sel_elem]);
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


void evg_isa_GET_BUFFER_RESINFO_impl()
{
	NOT_IMPL();
}


void evg_isa_SEMANTIC_impl()
{
	NOT_IMPL();
}


void evg_isa_GATHER4_impl()
{
	NOT_IMPL();
}


void evg_isa_GATHER4_C_impl()
{
	NOT_IMPL();
}


void evg_isa_GATHER4_C_O_impl()
{
	NOT_IMPL();
}


void evg_isa_GATHER4_O_impl()
{
	NOT_IMPL();
}


void evg_isa_GET_GRADIENTS_H_impl()
{
	NOT_IMPL();
}


void evg_isa_GET_GRADIENTS_V_impl()
{
	NOT_IMPL();
}


void evg_isa_GET_LOD_impl()
{
	NOT_IMPL();
}


void evg_isa_GET_NUMBER_OF_SAMPLES_impl()
{
	NOT_IMPL();
}


void evg_isa_GET_TEXTURE_RESINFO_impl()
{
	NOT_IMPL();
}


void evg_isa_KEEP_GRADIENTS_impl()
{
	NOT_IMPL();
}


void evg_isa_LD_impl()
{
	NOT_IMPL();
}


#define W0  EVG_TEX_WORD0
#define W1  EVG_TEX_WORD1
#define W2  EVG_TEX_WORD2
void evg_isa_SAMPLE_impl()
{
	int i;
	uint32_t base_addr;
	union evg_reg_t addr;
	union evg_reg_t value;
	int dst_sel[4], dst_sel_elem;
	struct evg_opencl_mem_t *image;

	uint32_t pixel_size;

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
	image = list_get(evg_isa_ndrange->kernel->uav_read_list, W0.resource_id);
	if(!image) 
		fatal("No table entry for read-only UAV %d\n", W0.resource_id);

	pixel_size = image->pixel_size;

	/* Calculate read address */
	base_addr = image->device_ptr;
	addr.as_uint = evg_isa_read_gpr(W0.src_gpr, 0, 0, 0);  /* FIXME Always reads from X */ 
	addr.as_uint = base_addr + (uint32_t) round(addr.as_float) * pixel_size;

	mem_read(evg_emu->global_mem, addr.as_uint, pixel_size, &value);

	evg_isa_debug("  t%d:read(%u)", evg_isa_work_item->id, addr.as_uint);
	evg_isa_debug("<=(%d,%gf) ", value.as_int, value.as_float);

	/* Do not fetch for inactive work_items */
	if (!evg_work_item_get_active(evg_isa_work_item))
		return;

	/* Store 'dst_sel_{x,y,z,w}' in array */
	dst_sel[0] = W1.dsx;
	dst_sel[1] = W1.dsy;
	dst_sel[2] = W1.dsz;
	dst_sel[3] = W1.dsw;
	
	/* Record global memory access */
	evg_isa_wavefront->global_mem_read = 1;
	evg_isa_work_item->global_mem_access_addr = addr.as_uint;
	evg_isa_work_item->global_mem_access_size = pixel_size;

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
			evg_isa_write_gpr(W1.dst_gpr, W1.dr, i, value.as_uint);
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


void evg_isa_SAMPLE_C_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_C_G_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_C_G_LB_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_C_L_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_C_LB_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_C_LZ_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_G_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_G_LB_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_L_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_LB_impl()
{
	NOT_IMPL();
}


void evg_isa_SAMPLE_LZ_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_GRADIENTS_H_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_GRADIENTS_V_impl()
{
	NOT_IMPL();
}


void evg_isa_SET_TEXTURE_OFFSETS_impl()
{
	NOT_IMPL();
}

void evg_isa_MEM_impl()
{
	NOT_IMPL();
}

