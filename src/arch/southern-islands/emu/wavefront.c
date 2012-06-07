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

#include <southern-islands-asm.h>
#include <southern-islands-emu.h>


/*
 * Private Functions
 */

/* Comparison function to sort list */
static int si_wavefront_divergence_compare(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


static void si_wavefront_divergence_dump(struct si_wavefront_t *wavefront, FILE *f)
{
	struct si_work_item_t *work_item;
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
	for (i = 0; i < wavefront->work_item_count; i++)
	{
		work_item = wavefront->work_items[i];
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
	list_sort(list, si_wavefront_divergence_compare);
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

		for (j = 0; j < wavefront->work_item_count; j++)
		{
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




/*
 * Public Functions
 */


struct si_wavefront_t *si_wavefront_create()
{
	struct si_wavefront_t *wavefront;

	/* Allocate */
	wavefront = calloc(1, sizeof(struct si_wavefront_t));
	if (!wavefront)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	wavefront->active_stack = bit_map_create(SI_MAX_STACK_SIZE * si_emu_wavefront_size);
	wavefront->pred = bit_map_create(si_emu_wavefront_size);

	/* Return */
	return wavefront;
}


void si_wavefront_free(struct si_wavefront_t *wavefront)
{
	/* Free wavefront */
	bit_map_free(wavefront->active_stack);
	bit_map_free(wavefront->pred);
	free(wavefront);
}


void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f)
{
	struct si_ndrange_t *ndrange = wavefront->ndrange;
	struct si_work_group_t *work_group = wavefront->work_group;

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

	/* FIXME Count instruction statistics here */

	si_wavefront_divergence_dump(wavefront, f);

	fprintf(f, "\n");
}


void si_wavefront_stack_push(struct si_wavefront_t *wavefront)
{
	if (wavefront->stack_top == SI_MAX_STACK_SIZE - 1)
		fatal("stack overflow");
	wavefront->stack_top++;
	wavefront->active_mask_push++;
	bit_map_copy(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
		wavefront->active_stack, (wavefront->stack_top - 1) * wavefront->work_item_count,
		wavefront->work_item_count);
	si_isa_debug("  %s:push", wavefront->name);
}


void si_wavefront_stack_pop(struct si_wavefront_t *wavefront, int count)
{
	if (!count)
		return;
	if (wavefront->stack_top < count)
		fatal("stack underflow");
	wavefront->stack_top -= count;
	wavefront->active_mask_pop += count;
	wavefront->active_mask_update = 1;
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("  %s:pop(%d),act=", wavefront->name, count);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
			wavefront->work_item_count, debug_file(si_isa_debug_category));
	}
}


/* Execute one instruction in the wavefront */
void si_wavefront_execute(struct si_wavefront_t *wavefront)
{
	extern struct si_ndrange_t *si_isa_ndrange;
	extern struct si_work_group_t *si_isa_work_group;
	extern struct si_wavefront_t *si_isa_wavefront;
	extern struct si_work_item_t *si_isa_work_item;
	extern struct si_inst_t *si_isa_inst;

	struct si_ndrange_t *ndrange = wavefront->ndrange;

	int work_item_id;

	/* Get current work-group */
	si_isa_ndrange = wavefront->ndrange;
	si_isa_wavefront = wavefront;
	si_isa_work_group = wavefront->work_group;
	si_isa_work_item = NULL;
	si_isa_inst = NULL;
	assert(!DOUBLE_LINKED_LIST_MEMBER(si_isa_work_group, finished, si_isa_wavefront));

	/* Reset instruction flags */
	wavefront->global_mem_write = 0;
	wavefront->global_mem_read = 0;
	wavefront->local_mem_write = 0;
	wavefront->local_mem_read = 0;
	wavefront->pred_mask_update = 0;
	wavefront->active_mask_update = 0;
	wavefront->active_mask_push = 0;
	wavefront->active_mask_pop = 0;
	
	/* Grab the next instruction and update the pointer */
	si_isa_wavefront->inst_size = si_inst_decode(si_isa_wavefront->inst_buf, &si_isa_wavefront->inst);

	/* Increment the instruction pointer */
	si_isa_wavefront->inst_buf += si_isa_wavefront->inst_size;

	/* FIXME If instruction updates active mask, update digests */
	/* XXX What is a digest? */

	/* Stats */
	si_emu->inst_count++;
	si_isa_wavefront->emu_inst_count++;
	si_isa_wavefront->inst_count++;

	/* Set the current instruction */
	si_isa_inst = &si_isa_wavefront->inst;

	/* Execute the current instruction */
	switch (si_isa_inst->info->fmt)
	{

	/* Scalar Memory Instructions */
	case SI_FMT_SMRD:
	{
		/* Stats */
		si_isa_wavefront->scalar_inst_count++;

		/* Only one work item executes the instruction */
		si_isa_work_item = ndrange->scalar_work_item;
		(*si_isa_inst_func[si_isa_inst->info->inst])();

		break;
	}

	/* Vector ALU Instructions */
	case SI_FMT_VOP2:
	case SI_FMT_VOP1:
	case SI_FMT_VOPC:
	case SI_FMT_VOP3a:
	case SI_FMT_VOP3b:
	{
		/* Stats */
		si_isa_wavefront->vector_inst_count++;
	
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(si_isa_wavefront, work_item_id)
		{
			si_isa_work_item = ndrange->work_items[work_item_id];
			(*si_isa_inst_func[si_isa_inst->info->inst])();
		}

		break;
	}

	default:
	{
		
	}

	}

#if 0
	/* If done, set work group to done */
	if (1) 
	{
		si_work_group_clear_status(si_isa_work_group, si_work_group_running);
		si_work_group_set_status(si_isa_work_group, si_work_group_finished);
	}
#endif
}

