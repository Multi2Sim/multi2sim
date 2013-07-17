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

#include <string.h>

#include <arch/evergreen/emu/emu.h>
#include <arch/evergreen/emu/ndrange.h>
#include <arch/evergreen/emu/wavefront.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/repos.h>
#include <lib/util/string.h>

#include "compute-unit.h"
#include "gpu.h"
#include "uop.h"


/*
 * Public variables
 */

int evg_stack_debug_category;



/*
 * Private
 */

static long long gpu_uop_id_counter = 0;

static struct repos_t *gpu_uop_repos;


static void evg_uop_add_src_idep(struct evg_uop_t *uop, struct evg_inst_t *inst, int src_idx)
{
	int sel, rel, chan, neg, abs;

	assert(uop->idep_count < EVG_UOP_MAX_IDEP);
	evg_inst_get_op_src(inst, src_idx, &sel, &rel, &chan, &neg, &abs);

	/* sel = 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
		uop->idep[uop->idep_count++] = EVG_UOP_DEP_REG(sel);

	/* sel = ALU_SRC_PV */
	else if (sel == 254)
		uop->idep[uop->idep_count++] = EVG_UOP_DEP_PV;

	/* sel = ALU_SRC_PS */
	else if (sel == 255)
		uop->idep[uop->idep_count++] = EVG_UOP_DEP_PS;

	/* sel = 219..222: QA, QA.pop, QB, QB.pop */
	else if (IN_RANGE(sel, 219, 222))
		uop->idep[uop->idep_count++] = EVG_UOP_DEP_LDS;
}




/*
 * Public Functions
 */

void evg_uop_init()
{
	/* GPU uop repository.
	 * The size assigned for each 'evg_uop_t' is equals to the baseline structure size plus the
	 * size of a 'evg_work_item_uop_t' element for each work-item in the wavefront. */
	gpu_uop_repos = repos_create(sizeof(struct evg_uop_t) + sizeof(struct evg_work_item_uop_t)
		* evg_emu_wavefront_size, "gpu_uop_repos");
	
}


void evg_uop_done()
{
	repos_free(gpu_uop_repos);
}


struct evg_uop_t *evg_uop_create()
{
	struct evg_uop_t *uop;

	uop = repos_create_object(gpu_uop_repos);
	uop->id = gpu_uop_id_counter++;
	return uop;
}


struct evg_uop_t *evg_uop_create_from_alu_group(struct evg_alu_group_t *alu_group)
{
	struct evg_uop_t *uop;
	struct evg_inst_t *inst;
	int i;

	/* Create uop */
	uop = evg_uop_create();

	/* Update dependences */
	for (i = 0; i < alu_group->inst_count; i++)
	{
		inst = &alu_group->inst[i];

		/* Local memory access instruction */
		if (inst->info->fmt[0] == EVG_FMT_ALU_WORD0_LDS_IDX_OP)
		{
			/* Assume read and write to local memory */
			assert(uop->idep_count < EVG_UOP_MAX_IDEP);
			uop->idep[uop->idep_count++] = EVG_UOP_DEP_LDS;
			assert(uop->odep_count < EVG_UOP_MAX_ODEP);
			uop->odep[uop->odep_count++] = EVG_UOP_DEP_LDS;
		}

		/* Arithmetic instruction */
		else if (inst->info->fmt[0] == EVG_FMT_ALU_WORD0)
		{
			/* Add input dependences */
			evg_uop_add_src_idep(uop, inst, 0);
			evg_uop_add_src_idep(uop, inst, 1);
			if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3)
				evg_uop_add_src_idep(uop, inst, 2);

			/* Add register output dependence if not masked */
			if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3 ||
				inst->words[1].alu_word1_op2.write_mask)
			{
				assert(uop->odep_count < EVG_UOP_MAX_ODEP);
				uop->odep[uop->odep_count++] =
					EVG_UOP_DEP_REG(inst->words[1].alu_word1_op2.dst_gpr);
			}

			/* Add PV/PS output dependence */
			assert(uop->odep_count < EVG_UOP_MAX_ODEP);
			uop->odep[uop->odep_count++] =
				inst->alu == EVG_ALU_TRANS ? EVG_UOP_DEP_PS : EVG_UOP_DEP_PV;
		}
	}

	/* Return */
	return uop;
}


void evg_uop_free(struct evg_uop_t *gpu_uop)
{
	if (!gpu_uop)
		return;
	repos_free_object(gpu_uop_repos, gpu_uop);
}


void evg_uop_list_free(struct linked_list_t *gpu_uop_list)
{
	struct evg_uop_t *uop;
	while (linked_list_count(gpu_uop_list))
	{
		linked_list_head(gpu_uop_list);
		uop = linked_list_get(gpu_uop_list);
		evg_uop_free(uop);
		linked_list_remove(gpu_uop_list);
	}
}


void evg_uop_dump_dep_list(char *buf, int size, int *dep_list, int dep_count)
{
	static struct str_map_t gpu_uop_dep_map = {
		4, {
			{ "none", EVG_UOP_DEP_NONE },
			{ "LDS", EVG_UOP_DEP_LDS },
			{ "PS", EVG_UOP_DEP_PS },
			{ "PV", EVG_UOP_DEP_PV }
		}
	};
	char *comma = "";
	char str[MAX_STRING_SIZE];
	int i;

	str_printf(&buf, &size, "{");
	for (i = 0; i < dep_count; i++)
	{
		if (IN_RANGE(dep_list[i], EVG_UOP_DEP_REG_FIRST, EVG_UOP_DEP_REG_LAST))
			sprintf(str, "R%d", dep_list[i] - EVG_UOP_DEP_REG_FIRST);
		else
			strcpy(str, str_map_value(&gpu_uop_dep_map, dep_list[i]));
		str_printf(&buf, &size, "%s%s", comma, str);
		comma = ",";
	}
	str_printf(&buf, &size, "}");
}


/* Stack debug - store current active mask in each work_item_uop. */
void evg_uop_save_active_mask(struct evg_uop_t *uop)
{
	struct evg_wavefront_t *wavefront = uop->wavefront;
	struct evg_ndrange_t *ndrange = wavefront->ndrange;
	struct evg_work_item_t *work_item;
	struct evg_work_item_uop_t *work_item_uop;
	int work_item_id;

	if (debug_status(evg_stack_debug_category) && wavefront->active_mask_update)
	{
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			work_item_uop->active = evg_work_item_get_active(work_item);
		}
	}
}


/* Stack debug - dump active mask */
void gpu_uop_dump_active_mask(struct evg_uop_t *uop, FILE *f)
{
	struct evg_wavefront_t *wavefront = uop->wavefront;
	struct evg_ndrange_t *ndrange = wavefront->ndrange;
	struct evg_work_item_t *work_item;
	struct evg_work_item_uop_t *work_item_uop;
	int work_item_id;

	assert(f);
	EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
	{
		work_item = ndrange->work_items[work_item_id];
		work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
		fprintf(f, "%d", work_item_uop->active);
	}
}


/* Stack debug - dump debugging information */
void evg_uop_debug_active_mask(struct evg_uop_t *uop)
{
	struct evg_wavefront_t *wavefront = uop->wavefront;

	FILE *f;

	/* Get debug file */
	f = debug_file(evg_stack_debug_category);
	assert(f);

	/* Pop */
	if (uop->active_mask_pop)
	{
		evg_stack_debug("stack clk=%lld cu=%d stack=%d wf=%d a=\"pop\" cnt=%d top=%d mask=\"",
			asTiming(evg_gpu)->cycle,
			uop->compute_unit->id,
			wavefront->id_in_compute_unit,
			wavefront->id,
			uop->active_mask_pop,
			uop->active_mask_stack_top);
		gpu_uop_dump_active_mask(uop, f);
		evg_stack_debug("\"\n");
	}

	/* Push */
	if (uop->active_mask_push)
	{
		evg_stack_debug("stack clk=%lld cu=%d stack=%d wf=%d a=\"push\" cnt=%d top=%d mask=\"",
			asTiming(evg_gpu)->cycle,
			uop->compute_unit->id,
			wavefront->id_in_compute_unit,
			wavefront->id,
			uop->active_mask_push,
			uop->active_mask_stack_top);
		gpu_uop_dump_active_mask(uop, f);
		evg_stack_debug("\"\n");
	}
}
