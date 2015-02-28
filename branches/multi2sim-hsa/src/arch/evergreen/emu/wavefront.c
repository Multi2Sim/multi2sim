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


#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "emu.h"
#include "isa.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"


/*
 * Class 'EvgWavefront'
 */

/* Comparison function to sort list */
static int EvgWavefrontCompareDivergence(const void *elem1, const void *elem2)
{
	const int count1 = * (const int *) elem1;
	const int count2 = * (const int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


static void EvgWavefrontDumpDivergence(EvgWavefront *self, FILE *f)
{
	EvgWorkItem *work_item;
	struct elem_t
	{
		int count;  /* 1st field hardcoded for comparison */
		int list_index;
		unsigned int branch_digest;
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
	for (i = 0; i < self->work_item_count; i++)
	{
		work_item = self->work_items[i];
		sprintf(str, "%08x", work_item->branch_digest);
		elem = hash_table_get(ht, str);
		if (!elem)
		{
			elem = xcalloc(1, sizeof(struct elem_t));

			hash_table_insert(ht, str, elem);
			elem->list_index = list_count(list);
			elem->branch_digest = work_item->branch_digest;
			list_add(list, elem);
		}
		elem->count++;
	}

	/* Sort divergence groups as per size */
	list_sort(list, EvgWavefrontCompareDivergence);
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

		for (j = 0; j < self->work_item_count; j++)
		{
			int first, last;
			first = self->work_items[j]->branch_digest == elem->branch_digest &&
				(j == 0 || self->work_items[j - 1]->branch_digest != elem->branch_digest);
			last = self->work_items[j]->branch_digest == elem->branch_digest &&
				(j == self->work_item_count - 1 || self->work_items[j + 1]->branch_digest != elem->branch_digest);
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


void EvgWavefrontCreate(EvgWavefront *self, EvgWorkGroup *work_group)
{
	EvgNDRange *ndrange = work_group->ndrange;
	EvgEmu *emu = ndrange->emu;
	EvgAsm *as = emu->as;

	/* Initialize */
	self->work_group = work_group;
	self->ndrange = work_group->ndrange;
	self->active_stack = bit_map_create(EVG_WAVEFRONT_STACK_SIZE * evg_emu_wavefront_size);
	self->pred = bit_map_create(evg_emu_wavefront_size);
	/* FIXME: Remove once loop state is part of stack */
	self->loop_depth = 0;
	self->cf_inst = new(EvgInst, as);
	self->alu_group = new(EvgALUGroup, as);
	self->tex_inst = new(EvgInst, as);
}


void EvgWavefrontDestroy(EvgWavefront *self)
{
	/* Free wavefront */
	bit_map_free(self->active_stack);
	bit_map_free(self->pred);
	str_free(self->name);
	delete(self->cf_inst);
	delete(self->alu_group);
	delete(self->tex_inst);
}


void EvgWavefrontDump(EvgWavefront *self, FILE *f)
{
	EvgNDRange *ndrange = self->ndrange;
	EvgWorkGroup *work_group = self->work_group;
	int i;

	if (!f)
		return;
	
	/* Dump wavefront statistics in GPU report */
	fprintf(f, "[ NDRange[%d].Wavefront[%d] ]\n\n", ndrange->id, self->id);

	fprintf(f, "Name = %s\n", self->name);
	fprintf(f, "WorkGroup = %d\n", work_group->id);
	fprintf(f, "WorkItemFirst = %d\n", self->work_item_id_first);
	fprintf(f, "WorkItemLast = %d\n", self->work_item_id_last);
	fprintf(f, "WorkItemCount = %d\n", self->work_item_count);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", self->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", self->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", self->local_mem_inst_count);
	fprintf(f, "\n");

	fprintf(f, "CF_Inst_Count = %lld\n", self->cf_inst_count);
	fprintf(f, "CF_Inst_Global_Mem_Write_Count = %lld\n", self->cf_inst_global_mem_write_count);
	fprintf(f, "\n");

	fprintf(f, "ALU_Clause_Count = %lld\n", self->alu_clause_count);
	fprintf(f, "ALU_Group_Count = %lld\n", self->alu_group_count);
	fprintf(f, "ALU_Group_Size =");
	for (i = 0; i < 5; i++)
		fprintf(f, " %lld", self->alu_group_size[i]);
	fprintf(f, "\n");
	fprintf(f, "ALU_Inst_Count = %lld\n", self->alu_inst_count);
	fprintf(f, "ALU_Inst_Local_Mem_Count = %lld\n", self->alu_inst_local_mem_count);
	fprintf(f, "\n");

	fprintf(f, "TC_Clause_Count = %lld\n", self->tc_clause_count);
	fprintf(f, "TC_Inst_Count = %lld\n", self->tc_inst_count);
	fprintf(f, "TC_Inst_Global_Mem_Read_Count = %lld\n", self->tc_inst_global_mem_read_count);
	fprintf(f, "\n");

	EvgWavefrontDumpDivergence(self, f);

	fprintf(f, "\n");
}


void EvgWavefrontSetName(EvgWavefront *self, char *name)
{
	self->name = str_set(self->name, name);
}


void EvgWavefrontPush(EvgWavefront *self)
{
	if (self->stack_top == EVG_WAVEFRONT_STACK_SIZE - 1)
		fatal("%s: stack overflow", self->cf_inst->info->name);
	self->stack_top++;
	self->active_mask_push++;
	bit_map_copy(self->active_stack, self->stack_top * self->work_item_count,
		self->active_stack, (self->stack_top - 1) * self->work_item_count,
		self->work_item_count);
	evg_isa_debug("  %s:push", self->name);
}


void EvgWavefrontPop(EvgWavefront *self, int count)
{
	if (!count)
		return;
	if (self->stack_top < count)
		fatal("%s: stack underflow", self->cf_inst->info->name);
	self->stack_top -= count;
	self->active_mask_pop += count;
	self->active_mask_update = 1;
	if (debug_status(evg_isa_debug_category))
	{
		evg_isa_debug("  %s:pop(%d),act=", self->name, count);
		bit_map_dump(self->active_stack, self->stack_top * self->work_item_count,
			self->work_item_count, debug_file(evg_isa_debug_category));
	}
}


void EvgWavefrontExecute(EvgWavefront *self)
{
	EvgNDRange *ndrange = self->ndrange;
	EvgWorkGroup *work_group = self->work_group;
	EvgWorkItem *work_item;
	EvgEmu *emu = ndrange->emu;

	EvgALUGroup *alu_group;
	EvgInst *inst;

	EvgInstOpcode inst_opcode;

	int work_item_id;

	/* Reset instruction flags */
	assert(!DOUBLE_LINKED_LIST_MEMBER(work_group, finished, self));
	self->global_mem_write = 0;
	self->global_mem_read = 0;
	self->local_mem_write = 0;
	self->local_mem_read = 0;
	self->pred_mask_update = 0;
	self->active_mask_update = 0;
	self->active_mask_push = 0;
	self->active_mask_pop = 0;

	switch (self->clause_kind)
	{

	case EvgInstClauseCF:
	{
		EvgWorkItem *work_item;

		int inst_num;

		/* Decode CF instruction */
		inst = self->cf_inst;
		inst_num = (self->cf_buf - self->cf_buf_start) / 8;
		self->cf_buf = EvgInstDecodeCF(inst, self->cf_buf);

		/* Debug */
		if (debug_status(evg_isa_debug_category))
		{
			evg_isa_debug("\n\n");
			EvgInstDump(inst, inst_num, 0,
				debug_file(evg_isa_debug_category));
		}

		/* Get first work-item in wavefront */
		work_item = self->work_items[0];
		assert(work_item);

		/* Execute once in wavefront */
		emu->inst_func[inst->info->opcode](work_item, inst);

		/* If instruction updates the work_item's active mask, update digests */
		if (inst->info->flags & EvgInstFlagActMask)
		{
			EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
			{
				work_item = ndrange->work_items[work_item_id];
				EvgWorkItemUpdateBranchDigest(work_item,
					self->cf_inst_count, inst_num);
			}
		}

		/* Stats */
		asEmu(emu)->instructions++;
		self->inst_count++;
		self->cf_inst_count++;
		if (inst->info->flags & EvgInstFlagMem)
		{
			self->global_mem_inst_count++;
			self->cf_inst_global_mem_write_count++;
		}
		if (ndrange->inst_histogram)
		{
			inst_opcode = self->cf_inst->info->opcode;
			assert(inst_opcode < EvgInstOpcodeCount);
			ndrange->inst_histogram[inst_opcode]++;
		}

		break;
	}

	case EvgInstClauseALU:
	{
		int i;

		/* Decode ALU group */
		alu_group = self->alu_group;
		self->clause_buf = EvgALUGroupDecode(alu_group,
				self->clause_buf, self->alu_group_count);

		/* Debug */
		if (debug_status(evg_isa_debug_category))
		{
			evg_isa_debug("\n\n");
			EvgALUGroupDump(alu_group, 0, debug_file(evg_isa_debug_category));
		}

		/* Execute group for each work_item in wavefront */
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			for (i = 0; i < alu_group->inst_count; i++)
			{
				inst = &alu_group->inst[i];
				emu->inst_func[inst->info->opcode](work_item, inst);
			}
			evg_isa_write_task_commit(work_item);
		}
		
		/* Statistics */
		asEmu(emu)->instructions++;
		self->inst_count += alu_group->inst_count;
		self->alu_inst_count += alu_group->inst_count;
		self->alu_group_count++;
		assert(alu_group->inst_count > 0 && alu_group->inst_count < 6);
		self->alu_group_size[alu_group->inst_count - 1]++;
		for (i = 0; i < alu_group->inst_count; i++)
		{
			inst = &alu_group->inst[i];
			if (inst->info->flags & EvgInstFlagLDS)
			{
				self->local_mem_inst_count++;
				self->alu_inst_local_mem_count++;
			}
			if (ndrange->inst_histogram)
			{
				inst_opcode = inst->info->opcode;
				assert(inst_opcode < EvgInstOpcodeCount);
				ndrange->inst_histogram[inst_opcode]++;
			}
		}

		/* End of clause reached */
		assert(self->clause_buf <= self->clause_buf_end);
		if (self->clause_buf >= self->clause_buf_end)
		{
			evg_isa_alu_clause_end(self);
			self->clause_kind = EvgInstClauseCF;
		}

		break;
	}

	case EvgInstClauseTEX:
	{
		/* Decode TEX instruction */
		inst = self->tex_inst;
		self->clause_buf = EvgInstDecodeTC(inst, self->clause_buf);

		/* Debug */
		if (debug_status(evg_isa_debug_category))
		{
			evg_isa_debug("\n\n");
			EvgInstDump(inst, 0, 0, debug_file(evg_isa_debug_category));
		}

		/* Execute in all work_items */
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(self, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			emu->inst_func[inst->info->opcode](work_item, inst);
		}

		/* Statistics */
		asEmu(emu)->instructions++;
		self->inst_count++;
		self->tc_inst_count++;
		if (inst->info->flags & EvgInstFlagMem)
		{
			self->global_mem_inst_count++;
			self->tc_inst_global_mem_read_count++;
		}
		if (ndrange->inst_histogram)
		{
			inst_opcode = inst->info->opcode;
			assert(inst_opcode < EvgInstOpcodeCount);
			ndrange->inst_histogram[inst_opcode]++;
		}

		/* End of clause reached */
		assert(self->clause_buf <= self->clause_buf_end);
		if (self->clause_buf == self->clause_buf_end)
		{
			evg_isa_tc_clause_end(self);
			self->clause_kind = EvgInstClauseCF;
		}

		break;
	}

	default:
		abort();
	}

	/* Check if wavefront finished kernel execution */
	if (self->clause_kind == EvgInstClauseCF && !self->cf_buf)
	{
		assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, self));
		assert(!DOUBLE_LINKED_LIST_MEMBER(work_group, finished, self));
		DOUBLE_LINKED_LIST_REMOVE(work_group, running, self);
		DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, finished, self);

		/* Check if work-group finished kernel execution */
		if (work_group->finished_list_count == work_group->wavefront_count)
		{
			assert(DOUBLE_LINKED_LIST_MEMBER(ndrange, running, work_group));
			assert(!DOUBLE_LINKED_LIST_MEMBER(ndrange, finished, work_group));
			EvgWorkGroupClearState(work_group, EvgWorkGroupRunning);
			EvgWorkGroupSetState(work_group, EvgWorkGroupFinished);

			/* Check if ND-Range finished kernel execution */
			if (ndrange->finished_list_count == ndrange->work_group_count)
			{
				assert(DOUBLE_LINKED_LIST_MEMBER(emu, running_ndrange, ndrange));
				assert(!DOUBLE_LINKED_LIST_MEMBER(emu, finished_ndrange, ndrange));
				EvgNDRangeClearState(ndrange, EvgNDRangeRunning);
				EvgNDRangeSetState(ndrange, EvgNDRangeFinished);
			}
		}
	}
}
