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

#include <hash-table.h>

#include <evergreen-emu.h>


/*
 * GPU wavefront
 */


struct evg_wavefront_t *evg_wavefront_create()
{
	struct evg_wavefront_t *wavefront;

	/* Allocate */
	wavefront = calloc(1, sizeof(struct evg_wavefront_t));
	if (!wavefront)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	wavefront->active_stack = bit_map_create(EVG_MAX_STACK_SIZE * evg_emu_wavefront_size);
	wavefront->pred = bit_map_create(evg_emu_wavefront_size);
	/* FIXME: Remove once loop state is part of stack */
	wavefront->loop_depth = 0;

	/* Return */
	return wavefront;
}


void evg_wavefront_free(struct evg_wavefront_t *wavefront)
{
	/* Free wavefront */
	bit_map_free(wavefront->active_stack);
	bit_map_free(wavefront->pred);
	free(wavefront);
}


/* Comparison function to sort list */
static int gpu_wavefront_divergence_compare(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


void gpu_wavefront_divergence_dump(struct evg_wavefront_t *wavefront, FILE *f)
{
	struct evg_work_item_t *work_item;
	struct elem_t {
		int count;  /* 1st field hardcoded for comparison */
		int list_index;
		uint32_t branch_digest;
	};
	struct elem_t *elem;
	struct list_t *list;
	struct hash_table_t *ht;
	char str[10];
	int i, j;

	/* Create list and hash table */
	list = list_create();
	ht = hash_table_create(20, 1);

	/* Create one 'elem' for each work_item with a different branch digest, and
	 * store it into the hash table and list. */
	for (i = 0; i < wavefront->work_item_count; i++) {
		work_item = wavefront->work_items[i];
		sprintf(str, "%08x", work_item->branch_digest);
		elem = hash_table_get(ht, str);
		if (!elem) {
			elem = calloc(1, sizeof(struct elem_t));
			hash_table_insert(ht, str, elem);
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
	hash_table_free(ht);
}


void evg_wavefront_dump(struct evg_wavefront_t *wavefront, FILE *f)
{
	struct evg_ndrange_t *ndrange = wavefront->ndrange;
	struct evg_work_group_t *work_group = wavefront->work_group;
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

	fprintf(f, "Inst_Count = %lld\n", wavefront->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", wavefront->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", wavefront->local_mem_inst_count);
	fprintf(f, "\n");

	fprintf(f, "CF_Inst_Count = %lld\n", wavefront->cf_inst_count);
	fprintf(f, "CF_Inst_Global_Mem_Write_Count = %lld\n", wavefront->cf_inst_global_mem_write_count);
	fprintf(f, "\n");

	fprintf(f, "ALU_Clause_Count = %lld\n", wavefront->alu_clause_count);
	fprintf(f, "ALU_Group_Count = %lld\n", wavefront->alu_group_count);
	fprintf(f, "ALU_Group_Size =");
	for (i = 0; i < 5; i++)
		fprintf(f, " %lld", wavefront->alu_group_size[i]);
	fprintf(f, "\n");
	fprintf(f, "ALU_Inst_Count = %lld\n", wavefront->alu_inst_count);
	fprintf(f, "ALU_Inst_Local_Mem_Count = %lld\n", wavefront->alu_inst_local_mem_count);
	fprintf(f, "\n");

	fprintf(f, "TC_Clause_Count = %lld\n", wavefront->tc_clause_count);
	fprintf(f, "TC_Inst_Count = %lld\n", wavefront->tc_inst_count);
	fprintf(f, "TC_Inst_Global_Mem_Read_Count = %lld\n", wavefront->tc_inst_global_mem_read_count);
	fprintf(f, "\n");

	gpu_wavefront_divergence_dump(wavefront, f);

	fprintf(f, "\n");
}


void evg_wavefront_stack_push(struct evg_wavefront_t *wavefront)
{
	if (wavefront->stack_top == EVG_MAX_STACK_SIZE - 1)
		fatal("%s: stack overflow", wavefront->cf_inst.info->name);
	wavefront->stack_top++;
	wavefront->active_mask_push++;
	bit_map_copy(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
		wavefront->active_stack, (wavefront->stack_top - 1) * wavefront->work_item_count,
		wavefront->work_item_count);
	evg_isa_debug("  %s:push", wavefront->name);
}


void evg_wavefront_stack_pop(struct evg_wavefront_t *wavefront, int count)
{
	if (!count)
		return;
	if (wavefront->stack_top < count)
		fatal("%s: stack underflow", wavefront->cf_inst.info->name);
	wavefront->stack_top -= count;
	wavefront->active_mask_pop += count;
	wavefront->active_mask_update = 1;
	if (debug_status(evg_isa_debug_category)) {
		evg_isa_debug("  %s:pop(%d),act=", wavefront->name, count);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
			wavefront->work_item_count, debug_file(evg_isa_debug_category));
	}
}


/* Execute one instruction in the wavefront */
void evg_wavefront_execute(struct evg_wavefront_t *wavefront)
{
	extern struct evg_ndrange_t *gpu_isa_ndrange;
	extern struct evg_work_group_t *gpu_isa_work_group;
	extern struct evg_wavefront_t *gpu_isa_wavefront;
	extern struct evg_work_item_t *gpu_isa_work_item;
	extern struct evg_inst_t *gpu_isa_cf_inst;
	extern struct evg_inst_t *gpu_isa_inst;
	extern struct evg_alu_group_t *gpu_isa_alu_group;

	struct evg_ndrange_t *ndrange = wavefront->ndrange;

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

	case EVG_CLAUSE_CF:
	{
		int inst_num;

		/* Decode CF instruction */
		inst_num = (gpu_isa_wavefront->cf_buf - gpu_isa_wavefront->cf_buf_start) / 8;
		gpu_isa_wavefront->cf_buf = evg_inst_decode_cf(gpu_isa_wavefront->cf_buf, &gpu_isa_wavefront->cf_inst);

		/* Debug */
		if (debug_status(evg_isa_debug_category)) {
			evg_isa_debug("\n\n");
			evg_inst_dump(&gpu_isa_wavefront->cf_inst, inst_num, 0,
				debug_file(evg_isa_debug_category));
		}

		/* Execute once in wavefront */
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		gpu_isa_inst = &gpu_isa_wavefront->cf_inst;
		(*evg_isa_inst_func[gpu_isa_inst->info->inst])();

		/* If instruction updates the work_item's active mask, update digests */
		if (gpu_isa_inst->info->flags & EVG_INST_FLAG_ACT_MASK) {
			EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
				gpu_isa_work_item = ndrange->work_items[work_item_id];
				evg_work_item_update_branch_digest(gpu_isa_work_item, gpu_isa_wavefront->cf_inst_count, inst_num);
			}
		}

		/* Stats */
		evg_emu->inst_count++;
		gpu_isa_wavefront->emu_inst_count++;
		gpu_isa_wavefront->inst_count++;
		gpu_isa_wavefront->cf_inst_count++;
		if (gpu_isa_inst->info->flags & EVG_INST_FLAG_MEM) {
			gpu_isa_wavefront->global_mem_inst_count++;
			gpu_isa_wavefront->cf_inst_global_mem_write_count++;  /* CF inst accessing memory is a write */
		}

		break;
	}

	case EVG_CLAUSE_ALU:
	{
		int i;

		/* Decode ALU group */
		gpu_isa_wavefront->clause_buf = evg_inst_decode_alu_group(gpu_isa_wavefront->clause_buf,
			gpu_isa_wavefront->alu_group_count, &gpu_isa_wavefront->alu_group);

		/* Debug */
		if (debug_status(evg_isa_debug_category)) {
			evg_isa_debug("\n\n");
			evg_alu_group_dump(&gpu_isa_wavefront->alu_group, 0, debug_file(evg_isa_debug_category));
		}

		/* Execute group for each work_item in wavefront */
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		gpu_isa_alu_group = &gpu_isa_wavefront->alu_group;
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
			gpu_isa_work_item = ndrange->work_items[work_item_id];
			for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
				gpu_isa_inst = &gpu_isa_alu_group->inst[i];
				(*evg_isa_inst_func[gpu_isa_inst->info->inst])();
			}
			evg_isa_write_task_commit();
		}
		
		/* Stats */
		evg_emu->inst_count++;
		gpu_isa_wavefront->inst_count += gpu_isa_alu_group->inst_count;
		gpu_isa_wavefront->alu_inst_count += gpu_isa_alu_group->inst_count;
		gpu_isa_wavefront->alu_group_count++;
		gpu_isa_wavefront->emu_inst_count += gpu_isa_alu_group->inst_count * gpu_isa_wavefront->work_item_count;
		assert(gpu_isa_alu_group->inst_count > 0 && gpu_isa_alu_group->inst_count < 6);
		gpu_isa_wavefront->alu_group_size[gpu_isa_alu_group->inst_count - 1]++;
		for (i = 0; i < gpu_isa_alu_group->inst_count; i++) {
			gpu_isa_inst = &gpu_isa_alu_group->inst[i];
			if (gpu_isa_inst->info->flags & EVG_INST_FLAG_LDS) {
				gpu_isa_wavefront->local_mem_inst_count++;
				gpu_isa_wavefront->alu_inst_local_mem_count++;
			}
		}

		/* End of clause reached */
		assert(gpu_isa_wavefront->clause_buf <= gpu_isa_wavefront->clause_buf_end);
		if (gpu_isa_wavefront->clause_buf >= gpu_isa_wavefront->clause_buf_end) {
			evg_isa_alu_clause_end();
			gpu_isa_wavefront->clause_kind = EVG_CLAUSE_CF;
		}

		break;
	}

	case EVG_CLAUSE_TEX:
	{
		/* Decode TEX instruction */
		gpu_isa_wavefront->clause_buf = evg_inst_decode_tc(gpu_isa_wavefront->clause_buf,
			&gpu_isa_wavefront->tex_inst);

		/* Debug */
		if (debug_status(evg_isa_debug_category)) {
			evg_isa_debug("\n\n");
			evg_inst_dump(&gpu_isa_wavefront->tex_inst, 0, 0, debug_file(evg_isa_debug_category));
		}

		/* Execute in all work_items */
		gpu_isa_inst = &gpu_isa_wavefront->tex_inst;
		gpu_isa_cf_inst = &gpu_isa_wavefront->cf_inst;
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(gpu_isa_wavefront, work_item_id) {
			gpu_isa_work_item = ndrange->work_items[work_item_id];
			(*evg_isa_inst_func[gpu_isa_inst->info->inst])();
		}

		/* Stats */
		evg_emu->inst_count++;
		gpu_isa_wavefront->emu_inst_count += gpu_isa_wavefront->work_item_count;
		gpu_isa_wavefront->inst_count++;
		gpu_isa_wavefront->tc_inst_count++;
		if (gpu_isa_inst->info->flags & EVG_INST_FLAG_MEM) {
			gpu_isa_wavefront->global_mem_inst_count++;
			gpu_isa_wavefront->tc_inst_global_mem_read_count++;  /* Memory instructions in TC are reads */
		}

		/* End of clause reached */
		assert(gpu_isa_wavefront->clause_buf <= gpu_isa_wavefront->clause_buf_end);
		if (gpu_isa_wavefront->clause_buf == gpu_isa_wavefront->clause_buf_end) {
			evg_isa_tc_clause_end();
			gpu_isa_wavefront->clause_kind = EVG_CLAUSE_CF;
		}

		break;
	}

	default:
		abort();
	}

	/* Check if wavefront finished kernel execution */
	if (gpu_isa_wavefront->clause_kind == EVG_CLAUSE_CF && !gpu_isa_wavefront->cf_buf)
	{
		assert(DOUBLE_LINKED_LIST_MEMBER(gpu_isa_work_group, running, gpu_isa_wavefront));
		assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_isa_work_group, finished, gpu_isa_wavefront));
		DOUBLE_LINKED_LIST_REMOVE(gpu_isa_work_group, running, gpu_isa_wavefront);
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_isa_work_group, finished, gpu_isa_wavefront);

		/* Check if work-group finished kernel execution */
		if (gpu_isa_work_group->finished_list_count == gpu_isa_work_group->wavefront_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(ndrange, running, gpu_isa_work_group));
			assert(!DOUBLE_LINKED_LIST_MEMBER(ndrange, finished, gpu_isa_work_group));
			evg_work_group_clear_status(gpu_isa_work_group, evg_work_group_running);
			evg_work_group_set_status(gpu_isa_work_group, evg_work_group_finished);
		}
	}
}
