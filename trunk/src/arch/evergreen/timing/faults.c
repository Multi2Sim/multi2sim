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

#include <heap.h>

#include <evergreen-timing.h>
#include <x86-emu.h>


char *gpu_faults_file_name = "";

enum gpu_fault_type_enum
{
	gpu_fault_ams,
	gpu_fault_reg,
	gpu_fault_mem
};

struct gpu_fault_t
{
	enum gpu_fault_type_enum type;
	long long cycle;
	int compute_unit_id;  /* 0, gpu_num_compute_units - 1 ] */
	int stack_id;  /* [ 0, gpu_max_wavefronts_per_compute_unit - 1 ] */
	int active_mask_id;  /* [ 0, GPU_MAX_STACK_SIZE - 1 ] */
	int bit;
	int reg_id;
	int byte;
};

/* Number of faults causing error */
int gpu_fault_errors;

/* List of faults read from fault injection file */
struct linked_list_t *gpu_fault_list;

int gpu_faults_debug_category;
char *gpu_faults_debug_file_name = "";


/*
 * Private Functions
 */

static int gpu_stack_faults_is_odep(struct gpu_uop_t *uop,
	struct gpu_wavefront_t *wavefront, int lo_reg)
{
	int i;

	if (wavefront != uop->wavefront)
		return 0;
	for (i = 0; i < GPU_UOP_MAX_ODEP; i++)
		if (uop->odep[i] == GPU_UOP_DEP_REG(lo_reg))
			return 1;
	return 0;
}


static int gpu_stack_faults_is_idep(struct gpu_uop_t *uop,
	struct gpu_wavefront_t *wavefront, int lo_reg)
{
	int i;

	if (wavefront != uop->wavefront)
		return 0;
	for (i = 0; i < GPU_UOP_MAX_IDEP; i++)
		if (uop->idep[i] == GPU_UOP_DEP_REG(lo_reg))
			return 1;
	return 0;
}




/*
 * Public Functions
 */

void gpu_faults_init(void)
{
	FILE *f;
	char line[MAX_STRING_SIZE];
	char *line_ptr;
	struct gpu_fault_t *fault;
	int line_num;
	long long last_cycle;

	gpu_fault_list = linked_list_create();
	if (!*gpu_faults_file_name)
		return;

	f = fopen(gpu_faults_file_name, "rt");
	if (!f)
		fatal("%s: cannot open file", gpu_faults_file_name);
	
	line_num = 0;
	last_cycle = 0;
	while (!feof(f))
	{
		const char *delim = " ";
	
		/* Read a line */
		line_num++;
		line_ptr = fgets(line, MAX_STRING_SIZE, f);
		if (!line_ptr)
			break;

		/* Allocate new fault */
		fault = calloc(1, sizeof(struct gpu_fault_t));
		if (!fault)
			fatal("%s: out of memory", __FUNCTION__);

		/* Read <cycle> field */
		line_ptr = strtok(line_ptr, delim);
		if (!line_ptr)
			goto wrong_format;
		fault->cycle = atoll(line_ptr);
		if (fault->cycle < 1)
			fatal("%s: line %d: lowest possible cycle is 1",
				gpu_faults_file_name, line_num);
		if (fault->cycle < last_cycle)
			fatal("%s: line %d: cycles must be ordered",
				gpu_faults_file_name, line_num);

		/* <fault> - Type of fault */
		line_ptr = strtok(NULL, delim);
		if (!line_ptr)
			goto wrong_format;
		if (!strcmp(line_ptr, "ams"))
			fault->type = gpu_fault_ams;
		else if (!strcmp(line_ptr, "reg"))
			fault->type = gpu_fault_reg;
		else if (!strcmp(line_ptr, "mem"))
			fault->type = gpu_fault_mem;
		else
			fatal("%s: line %d: invalid value for <fault> ('%s')",
				gpu_faults_file_name, line_num, line_ptr);

		/* <cu_id> - Compute unit */
		line_ptr = strtok(NULL, delim);
		if (!line_ptr)
			goto wrong_format;
		fault->compute_unit_id = atoi(line_ptr);
		if (fault->compute_unit_id >= gpu_num_compute_units || fault->compute_unit_id < 0)
			fatal("%s: line %d: invalid compute unit ID",
				gpu_faults_file_name, line_num);

		/* Analyze rest of the line depending on fault type */
		switch (fault->type)
		{

		case gpu_fault_ams:

			/* <stack_id> - Stack ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->stack_id = atoi(line_ptr);
			if (fault->stack_id >= gpu_max_wavefronts_per_compute_unit)
				fatal("%s: line %d: invalid stack ID",
					gpu_faults_file_name, line_num);

			/* <am_id> - Active mask ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->active_mask_id = atoi(line_ptr);
			if (fault->active_mask_id >= GPU_MAX_STACK_SIZE)
				fatal("%s: line %d: invalid active mask ID",
					gpu_faults_file_name, line_num);

			/* <bit> */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit >= gpu_wavefront_size)
				fatal("%s: line %d: invalid bit index",
					gpu_faults_file_name, line_num);

			/* No more tokens */
			if (strtok(NULL, delim))
				fatal("%s: line %d: too many arguments",
					gpu_faults_file_name, line_num);

			break;

		case gpu_fault_reg:

			/* <reg_id> - Register ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->reg_id = atoi(line_ptr);
			if (fault->reg_id >= gpu_num_registers || fault->reg_id < 0)
				fatal("%s: line %d: invalid compute unit ID",
					gpu_faults_file_name, line_num);

			/* <bit> */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit < 0 || fault->bit >= 128)
				fatal("%s: line %d: invalid bit index",
					gpu_faults_file_name, line_num);

			break;

		case gpu_fault_mem:

			/* <byte> - Byte position in local memory */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->byte = atoi(line_ptr);
			if (fault->byte >= gpu_local_mem_size || fault->byte < 0)
				fatal("%s: line %d: invalid byte position",
					gpu_faults_file_name, line_num);

			/* <bit> - Bit position */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit > 7 || fault->bit < 0)
				fatal("%s: line %d: invalid bit position",
					gpu_faults_file_name, line_num);

			break;
		}

		/* Insert fault in fault list */
		linked_list_out(gpu_fault_list);
		linked_list_insert(gpu_fault_list, fault);
		last_cycle = fault->cycle;
		continue;

wrong_format:
		fatal("%s: line %d: not enough arguments",
			gpu_faults_file_name, line_num);
	}
	linked_list_head(gpu_fault_list);
}


void gpu_faults_done(void)
{
	while (linked_list_count(gpu_fault_list))
	{
		linked_list_head(gpu_fault_list);
		free(linked_list_get(gpu_fault_list));
		linked_list_remove(gpu_fault_list);
	}
	linked_list_free(gpu_fault_list);
}


void gpu_faults_insert(void)
{
	struct gpu_fault_t *fault;
	struct gpu_compute_unit_t *compute_unit;

	for (;;)
	{
		linked_list_head(gpu_fault_list);
		fault = linked_list_get(gpu_fault_list);
		if (!fault || fault->cycle > gpu->cycle)
			break;

		/* Insert fault depending on fault type */
		switch (fault->type)
		{

		case gpu_fault_ams:
		{
			struct gpu_work_group_t *work_group;
			struct gpu_wavefront_t *wavefront;
			struct gpu_work_item_t *work_item;

			int work_group_id;  /* in compute unit */
			int wavefront_id;  /* in compute unit */
			int value;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"ams\" stack=%d am=%d bit=%d ",
				gpu->cycle,
				fault->compute_unit_id, fault->stack_id,
				fault->active_mask_id, fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count)
			{
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Get work-group and wavefront. If wavefront ID exceeds current number, dismiss */
			work_group_id = fault->stack_id / gpu->ndrange->wavefronts_per_work_group;
			wavefront_id = fault->stack_id % gpu->ndrange->wavefronts_per_work_group;
			if (work_group_id >= gpu_max_work_groups_per_compute_unit
				|| !compute_unit->work_groups[work_group_id])
			{
				gpu_faults_debug("effect=\"wf_idle\"");
				goto end_loop;
			}
			work_group = compute_unit->work_groups[work_group_id];
			wavefront = work_group->wavefronts[wavefront_id];

			/* If active_mask_id exceeds stack top, dismiss */
			if (fault->active_mask_id > wavefront->stack_top)
			{
				gpu_faults_debug("effect=\"am_idle\"");
				goto end_loop;
			}

			/* If 'bit' exceeds number of work-items in wavefront, dismiss */
			if (fault->bit >= wavefront->work_item_count)
			{
				gpu_faults_debug("effect=\"wi_idle\"");
				goto end_loop;
			}

			/* Fault caused an error, show affected software entities */
			work_item = wavefront->work_items[fault->bit];
			gpu_faults_debug("effect=\"error\" wg=%d wf=%d wi=%d",
				work_group->id,
				wavefront->id,
				work_item->id);

			/* Inject fault */
			value = bit_map_get(wavefront->active_stack,
				fault->active_mask_id * wavefront->work_item_count
				+ fault->bit, 1);
			bit_map_set(wavefront->active_stack,
				fault->active_mask_id * wavefront->work_item_count
				+ fault->bit, 1, !value);
			gpu_fault_errors++;

			break;
		}

		case gpu_fault_reg:
		{
			struct opencl_kernel_t *kernel = gpu->ndrange->kernel;

			int work_group_id_in_compute_unit;
			struct gpu_work_group_t *work_group;
			struct gpu_wavefront_t *wavefront;

			int num_registers_per_work_group;

			int work_item_id_in_compute_unit;
			int work_item_id_in_work_group;
			struct gpu_work_item_t *work_item;

			struct linked_list_t *fetch_queue;
			struct gpu_uop_t *inst_buffer;
			struct gpu_uop_t *exec_buffer;
			struct heap_t *event_queue;
			struct gpu_uop_t *uop;

			int lo_reg;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"reg\" reg=%d bit=%d ",
				gpu->cycle,
				fault->compute_unit_id,
				fault->reg_id,
				fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count)
			{
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Get work-group */
			num_registers_per_work_group = kernel->amd_bin->enc_dict_entry_evergreen->num_gpr_used
				* kernel->local_size;
			work_group_id_in_compute_unit = fault->reg_id / num_registers_per_work_group;
			if (work_group_id_in_compute_unit >= gpu_max_work_groups_per_compute_unit)
			{
				gpu_faults_debug("effect=\"reg_idle\"");
				goto end_loop;
			}

			/* Get work-group (again) */
			work_group = compute_unit->work_groups[work_group_id_in_compute_unit];
			if (!work_group)
			{
				gpu_faults_debug("effect=\"reg_idle\"");
				goto end_loop;
			}

			/* Get affected entities */
			work_item_id_in_compute_unit = fault->reg_id
				/ kernel->amd_bin->enc_dict_entry_evergreen->num_gpr_used;
			work_item_id_in_work_group = work_item_id_in_compute_unit % kernel->local_size;
			work_item = work_group->work_items[work_item_id_in_work_group];
			wavefront = work_item->wavefront;
			lo_reg = fault->reg_id % kernel->amd_bin->enc_dict_entry_evergreen->num_gpr_used;

			/* Fault falling between Fetch and Read stage of an instruction
			 * consuming register. This case cannot be modeled due to functional
			 * simulation skew. */
			fetch_queue = compute_unit->alu_engine.fetch_queue;
			inst_buffer = compute_unit->alu_engine.inst_buffer;
			for (linked_list_head(fetch_queue); !linked_list_is_end(fetch_queue);
				linked_list_next(fetch_queue))
			{
				uop = linked_list_get(fetch_queue);
				if (gpu_stack_faults_is_idep(uop, wavefront, lo_reg))
				{
					gpu_faults_debug("effect=\"reg_read\"");
					goto end_loop;
				}
			}
			uop = inst_buffer;
			if (uop && gpu_stack_faults_is_idep(uop, wavefront, lo_reg))
			{
				gpu_faults_debug("effect=\"reg_read\"");
				goto end_loop;
			}

			/* Fault falling between Fetch and Write stage of an instruction
			 * writing on the register. The instruction will overwrite the fault,
			 * so this shouldn't cause its injection. */
			exec_buffer = compute_unit->alu_engine.exec_buffer;
			for (linked_list_head(fetch_queue); !linked_list_is_end(fetch_queue);
				linked_list_next(fetch_queue))
			{
				uop = linked_list_get(fetch_queue);
				if (gpu_stack_faults_is_odep(uop, wavefront, lo_reg))
				{
					gpu_faults_debug("effect=\"reg_write\"");
					goto end_loop;
				}
			}
			uop = inst_buffer;
			if (uop && gpu_stack_faults_is_odep(uop, wavefront, lo_reg))
			{
				gpu_faults_debug("effect=\"reg_write\"");
				goto end_loop;
			}
			uop = exec_buffer;
			if (uop && gpu_stack_faults_is_odep(uop, wavefront, lo_reg))
			{
				gpu_faults_debug("effect=\"reg_write\"");
				goto end_loop;
			}
			event_queue = compute_unit->alu_engine.event_queue;
			for (heap_first(event_queue, (void **) &uop); uop;
				heap_next(event_queue, (void **) &uop))
			{
				if (gpu_stack_faults_is_odep(uop, wavefront, lo_reg))
				{
					gpu_faults_debug("effect=\"reg_write\"");
					goto end_loop;
				}
			}

			/* Fault caused error */
			gpu_faults_debug("effect=\"error\" ");
			gpu_faults_debug("wg=%d wf=%d wi=%d lo_reg=%d ",
				work_group->id, work_item->wavefront->id, work_item->id, lo_reg);

			/* Insert the fault */
			if (fault->bit < 32)
				work_item->gpr[lo_reg].elem[0] ^= 1 << fault->bit;
			else if (fault->bit < 64)
				work_item->gpr[lo_reg].elem[1] ^= 1 << (fault->bit - 32);
			else if (fault->bit < 96)
				work_item->gpr[lo_reg].elem[2] ^= 1 << (fault->bit - 64);
			else
				work_item->gpr[lo_reg].elem[3] ^= 1 << (fault->bit - 96);
			gpu_fault_errors++;

			break;

		}

		case gpu_fault_mem:
		{
			struct gpu_work_group_t *work_group;

			int work_group_id_in_compute_unit;
			unsigned char value;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"mem\" byte=%d bit=%d ",
				gpu->cycle,
				fault->compute_unit_id,
				fault->byte,
				fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count)
			{
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Check if there is any local memory used at all */
			if (!gpu->ndrange->local_mem_top)
			{
				gpu_faults_debug("effect=\"mem_idle\"");
				goto end_loop;
			}

			/* Get work-group */
			work_group_id_in_compute_unit = fault->byte / gpu->ndrange->local_mem_top;
			if (work_group_id_in_compute_unit >= gpu_max_work_groups_per_compute_unit)
			{
				gpu_faults_debug("effect=\"mem_idle\"");
				goto end_loop;
			}

			/* Get work-group (again) */
			work_group = compute_unit->work_groups[work_group_id_in_compute_unit];
			if (!work_group)
			{
				gpu_faults_debug("effect=\"mem_idle\"");
				goto end_loop;
			}

			/* Inject fault */
			gpu_faults_debug("effect=\"error\" wg=%d ",
				work_group->id);
			mem_read(work_group->local_mem, fault->byte, 1, &value);
			value ^= 1 << fault->bit;
			mem_write(work_group->local_mem, fault->byte, 1, &value);
			gpu_fault_errors++;

			break;

		}

		default:
			panic("invalid fault type");

		}

end_loop:
		/* Extract and free */
		free(fault);
		linked_list_remove(gpu_fault_list);
		gpu_faults_debug("\n");

		/* If all faults were inserted and no error was caused, end simulation */
		if (!linked_list_count(gpu_fault_list) && !gpu_fault_errors)
			x86_emu_finish = x86_emu_finish_gpu_no_faults;
	}
}
