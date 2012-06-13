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

#include <fermi-emu.h>


/*
 * Private Functions
 */

/* Comparison function to sort list */
static int frm_warp_divergence_compare(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


static void frm_warp_divergence_dump(struct frm_warp_t *warp, FILE *f)
{
	struct frm_work_item_t *work_item;
	struct elem_t
	{
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
	for (i = 0; i < warp->work_item_count; i++)
	{
		work_item = warp->work_items[i];
		sprintf(str, "%08x", work_item->branch_digest);
		elem = hash_table_get(ht, str);
		if (!elem)
		{
			elem = calloc(1, sizeof(struct elem_t));
			if (!elem)
				fatal("%s: out of memory", __FUNCTION__);

			hash_table_insert(ht, str, elem);
			elem->list_index = list_count(list);
			elem->branch_digest = work_item->branch_digest;
			list_add(list, elem);
		}
		elem->count++;
	}

	/* Sort divergence groups as per size */
	list_sort(list, frm_warp_divergence_compare);
	fprintf(f, "DivergenceGroups = %d\n", list_count(list));
	
	/* Dump size of groups with */
	fprintf(f, "DivergenceGroupsSize =");
	for (i = 0; i < list_count(list); i++)
	{
		elem = list_get(list, i);
		fprintf(f, " %d", elem->count);
	}
	fprintf(f, "\n\n");

	/* Dump work_item ids contained in each work_item divergence group */
	for (i = 0; i < list_count(list); i++)
	{
		elem = list_get(list, i);
		fprintf(f, "DivergenceGroup[%d] =", i);

		for (j = 0; j < warp->work_item_count; j++)
		{
			int first, last;
			first = warp->work_items[j]->branch_digest == elem->branch_digest &&
				(j == 0 || warp->work_items[j - 1]->branch_digest != elem->branch_digest);
			last = warp->work_items[j]->branch_digest == elem->branch_digest &&
				(j == warp->work_item_count - 1 || warp->work_items[j + 1]->branch_digest != elem->branch_digest);
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




/*
 * Public Functions
 */


struct frm_warp_t *frm_warp_create()
{
	struct frm_warp_t *warp;

	/* Allocate */
	warp = calloc(1, sizeof(struct frm_warp_t));
	if (!warp)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	warp->active_stack = bit_map_create(FRM_MAX_STACK_SIZE * frm_emu_warp_size);
	warp->pred = bit_map_create(frm_emu_warp_size);
	/* FIXME: Remove once loop state is part of stack */
	warp->loop_depth = 0;

	/* Return */
	return warp;
}


void frm_warp_free(struct frm_warp_t *warp)
{
	/* Free warp */
	bit_map_free(warp->active_stack);
	bit_map_free(warp->pred);
	free(warp);
}


void frm_warp_dump(struct frm_warp_t *warp, FILE *f)
{
	struct frm_grid_t *grid = warp->grid;
	struct frm_threadblock_t *threadblock = warp->threadblock;
	int i;

	if (!f)
		return;
	
	/* Dump warp statistics in GPU report */
	fprintf(f, "[ NDRange[%d].Wavefront[%d] ]\n\n", grid->id, warp->id);

	fprintf(f, "Name = %s\n", warp->name);
	fprintf(f, "WorkGroup = %d\n", threadblock->id);
	fprintf(f, "WorkItemFirst = %d\n", warp->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", warp->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", warp->work_item_count);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", warp->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", warp->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", warp->local_mem_inst_count);
	fprintf(f, "\n");

	fprintf(f, "CF_Inst_Count = %lld\n", warp->cf_inst_count);
	fprintf(f, "CF_Inst_Global_Mem_Write_Count = %lld\n", warp->cf_inst_global_mem_write_count);
	fprintf(f, "\n");

	fprintf(f, "ALU_Clause_Count = %lld\n", warp->alu_clause_count);
	fprintf(f, "ALU_Group_Count = %lld\n", warp->alu_group_count);
	fprintf(f, "ALU_Group_Size =");
	for (i = 0; i < 5; i++)
		fprintf(f, " %lld", warp->alu_group_size[i]);
	fprintf(f, "\n");
	fprintf(f, "ALU_Inst_Count = %lld\n", warp->alu_inst_count);
	fprintf(f, "ALU_Inst_Local_Mem_Count = %lld\n", warp->alu_inst_local_mem_count);
	fprintf(f, "\n");

	fprintf(f, "TC_Clause_Count = %lld\n", warp->tc_clause_count);
	fprintf(f, "TC_Inst_Count = %lld\n", warp->tc_inst_count);
	fprintf(f, "TC_Inst_Global_Mem_Read_Count = %lld\n", warp->tc_inst_global_mem_read_count);
	fprintf(f, "\n");

	frm_warp_divergence_dump(warp, f);

	fprintf(f, "\n");
}


void frm_warp_stack_push(struct frm_warp_t *warp)
{
	if (warp->stack_top == FRM_MAX_STACK_SIZE - 1)
		fatal("%s: stack overflow", warp->cf_inst.info->name);
	warp->stack_top++;
	warp->active_mask_push++;
	bit_map_copy(warp->active_stack, warp->stack_top * warp->work_item_count,
		warp->active_stack, (warp->stack_top - 1) * warp->work_item_count,
		warp->work_item_count);
	frm_isa_debug("  %s:push", warp->name);
}


void frm_warp_stack_pop(struct frm_warp_t *warp, int count)
{
	if (!count)
		return;
	if (warp->stack_top < count)
		fatal("%s: stack underflow", warp->cf_inst.info->name);
	warp->stack_top -= count;
	warp->active_mask_pop += count;
	warp->active_mask_update = 1;
	if (debug_status(frm_isa_debug_category))
	{
		frm_isa_debug("  %s:pop(%d),act=", warp->name, count);
		bit_map_dump(warp->active_stack, warp->stack_top * warp->work_item_count,
			warp->work_item_count, debug_file(frm_isa_debug_category));
	}
}


/* Execute one instruction in the warp */
void frm_warp_execute(struct frm_warp_t *warp)
{
	extern struct frm_grid_t *frm_isa_grid;
	extern struct frm_threadblock_t *frm_isa_threadblock;
	extern struct frm_warp_t *frm_isa_warp;
	extern struct frm_work_item_t *frm_isa_work_item;
	extern struct frm_inst_t *frm_isa_cf_inst;
	extern struct frm_inst_t *frm_isa_inst;
	extern struct frm_alu_group_t *frm_isa_alu_group;

	struct frm_grid_t *grid = warp->grid;

	int work_item_id;

	/* Get current work-group */
	frm_isa_grid = warp->grid;
	frm_isa_warp = warp;
	frm_isa_threadblock = warp->threadblock;
	frm_isa_work_item = NULL;
	frm_isa_cf_inst = NULL;
	frm_isa_inst = NULL;
	frm_isa_alu_group = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(frm_isa_threadblock, finished, frm_isa_warp));

	/* Reset instruction flags */
	warp->global_mem_write = 0;
	warp->global_mem_read = 0;
	warp->local_mem_write = 0;
	warp->local_mem_read = 0;
	warp->pred_mask_update = 0;
	warp->active_mask_update = 0;
	warp->active_mask_push = 0;
	warp->active_mask_pop = 0;

	switch (warp->clause_kind)
	{

	case FRM_CLAUSE_CF:
	{
		int inst_num;

		/* Decode CF instruction */
		inst_num = (frm_isa_warp->cf_buf - frm_isa_warp->cf_buf_start) / 8;
		frm_isa_warp->cf_buf = frm_inst_decode_cf(frm_isa_warp->cf_buf, &frm_isa_warp->cf_inst);

		/* Debug */
		if (debug_status(frm_isa_debug_category))
		{
			frm_isa_debug("\n\n");
			frm_inst_dump(&frm_isa_warp->cf_inst, inst_num, 0,
				debug_file(frm_isa_debug_category));
		}

		/* Execute once in warp */
		frm_isa_cf_inst = &frm_isa_warp->cf_inst;
		frm_isa_inst = &frm_isa_warp->cf_inst;
		(*frm_isa_inst_func[frm_isa_inst->info->inst])();

		/* If instruction updates the work_item's active mask, update digests */
		if (frm_isa_inst->info->flags & FRM_INST_FLAG_ACT_MASK)
		{
			FRM_FOREACH_WORK_ITEM_IN_WAVEFRONT(frm_isa_warp, work_item_id)
			{
				frm_isa_work_item = grid->work_items[work_item_id];
				frm_work_item_update_branch_digest(frm_isa_work_item, frm_isa_warp->cf_inst_count, inst_num);
			}
		}

		/* Stats */
		frm_emu->inst_count++;
		frm_isa_warp->emu_inst_count++;
		frm_isa_warp->inst_count++;
		frm_isa_warp->cf_inst_count++;
		if (frm_isa_inst->info->flags & FRM_INST_FLAG_MEM)
		{
			frm_isa_warp->global_mem_inst_count++;
			frm_isa_warp->cf_inst_global_mem_write_count++;  /* CF inst accessing memory is a write */
		}

		break;
	}

	case FRM_CLAUSE_ALU:
	{
		int i;

		/* Decode ALU group */
		frm_isa_warp->clause_buf = frm_inst_decode_alu_group(frm_isa_warp->clause_buf,
			frm_isa_warp->alu_group_count, &frm_isa_warp->alu_group);

		/* Debug */
		if (debug_status(frm_isa_debug_category))
		{
			frm_isa_debug("\n\n");
			frm_alu_group_dump(&frm_isa_warp->alu_group, 0, debug_file(frm_isa_debug_category));
		}

		/* Execute group for each work_item in warp */
		frm_isa_cf_inst = &frm_isa_warp->cf_inst;
		frm_isa_alu_group = &frm_isa_warp->alu_group;
		FRM_FOREACH_WORK_ITEM_IN_WAVEFRONT(frm_isa_warp, work_item_id)
		{
			frm_isa_work_item = grid->work_items[work_item_id];
			for (i = 0; i < frm_isa_alu_group->inst_count; i++)
			{
				frm_isa_inst = &frm_isa_alu_group->inst[i];
				(*frm_isa_inst_func[frm_isa_inst->info->inst])();
			}
			frm_isa_write_task_commit();
		}
		
		/* Stats */
		frm_emu->inst_count++;
		frm_isa_warp->inst_count += frm_isa_alu_group->inst_count;
		frm_isa_warp->alu_inst_count += frm_isa_alu_group->inst_count;
		frm_isa_warp->alu_group_count++;
		frm_isa_warp->emu_inst_count += frm_isa_alu_group->inst_count * frm_isa_warp->work_item_count;
		assert(frm_isa_alu_group->inst_count > 0 && frm_isa_alu_group->inst_count < 6);
		frm_isa_warp->alu_group_size[frm_isa_alu_group->inst_count - 1]++;
		for (i = 0; i < frm_isa_alu_group->inst_count; i++)
		{
			frm_isa_inst = &frm_isa_alu_group->inst[i];
			if (frm_isa_inst->info->flags & FRM_INST_FLAG_LDS)
			{
				frm_isa_warp->local_mem_inst_count++;
				frm_isa_warp->alu_inst_local_mem_count++;
			}
		}

		/* End of clause reached */
		assert(frm_isa_warp->clause_buf <= frm_isa_warp->clause_buf_end);
		if (frm_isa_warp->clause_buf >= frm_isa_warp->clause_buf_end)
		{
			frm_isa_alu_clause_end();
			frm_isa_warp->clause_kind = FRM_CLAUSE_CF;
		}

		break;
	}

	case FRM_CLAUSE_TEX:
	{
		/* Decode TEX instruction */
		frm_isa_warp->clause_buf = frm_inst_decode_tc(frm_isa_warp->clause_buf,
			&frm_isa_warp->tex_inst);

		/* Debug */
		if (debug_status(frm_isa_debug_category))
		{
			frm_isa_debug("\n\n");
			frm_inst_dump(&frm_isa_warp->tex_inst, 0, 0, debug_file(frm_isa_debug_category));
		}

		/* Execute in all work_items */
		frm_isa_inst = &frm_isa_warp->tex_inst;
		frm_isa_cf_inst = &frm_isa_warp->cf_inst;
		FRM_FOREACH_WORK_ITEM_IN_WAVEFRONT(frm_isa_warp, work_item_id)
		{
			frm_isa_work_item = grid->work_items[work_item_id];
			(*frm_isa_inst_func[frm_isa_inst->info->inst])();
		}

		/* Stats */
		frm_emu->inst_count++;
		frm_isa_warp->emu_inst_count += frm_isa_warp->work_item_count;
		frm_isa_warp->inst_count++;
		frm_isa_warp->tc_inst_count++;
		if (frm_isa_inst->info->flags & FRM_INST_FLAG_MEM)
		{
			frm_isa_warp->global_mem_inst_count++;
			frm_isa_warp->tc_inst_global_mem_read_count++;  /* Memory instructions in TC are reads */
		}

		break;
	}

	default:
		abort();
	}

	/* Check if warp finished kernel execution */
	if (frm_isa_warp->clause_kind == FRM_CLAUSE_CF && !frm_isa_warp->cf_buf)
	{
		assert(DOUBLE_LINKED_LIST_MEMBER(frm_isa_threadblock, running, frm_isa_warp));
		assert(!DOUBLE_LINKED_LIST_MEMBER(frm_isa_threadblock, finished, frm_isa_warp));
		DOUBLE_LINKED_LIST_REMOVE(frm_isa_threadblock, running, frm_isa_warp);
		DOUBLE_LINKED_LIST_INSERT_TAIL(frm_isa_threadblock, finished, frm_isa_warp);

		/* Check if work-group finished kernel execution */
		if (frm_isa_threadblock->finished_list_count == frm_isa_threadblock->warp_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(grid, running, frm_isa_threadblock));
			assert(!DOUBLE_LINKED_LIST_MEMBER(grid, finished, frm_isa_threadblock));
			frm_threadblock_clear_status(frm_isa_threadblock, frm_threadblock_running);
			frm_threadblock_set_status(frm_isa_threadblock, frm_threadblock_finished);
		}
	}
}
