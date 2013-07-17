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

#include <arch/fermi/emu/emu.h>
#include <lib/util/list.h>
#include <lib/util/repos.h>

#include "uop.h"



/*
 * Public variables
 */

int frm_stack_debug_category;



/*
 * Private
 */

static long long gpu_uop_id_counter = 0;

static struct repos_t *gpu_uop_repos;


#if 0
static void frm_uop_add_src_idep(struct frm_uop_t *uop, struct frm_inst_t *inst, int src_idx)
{
	int sel, rel, chan, neg, abs;

	assert(uop->idep_count < FRM_UOP_MAX_IDEP);
	frm_inst_get_op_src(inst, src_idx, &sel, &rel, &chan, &neg, &abs);

	/* sel = 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
		uop->idep[uop->idep_count++] = FRM_UOP_DEP_REG(sel);

	/* sel = ALU_SRC_PV */
	else if (sel == 254)
		uop->idep[uop->idep_count++] = FRM_UOP_DEP_PV;

	/* sel = ALU_SRC_PS */
	else if (sel == 255)
		uop->idep[uop->idep_count++] = FRM_UOP_DEP_PS;

	/* sel = 219..222: QA, QA.pop, QB, QB.pop */
	else if (IN_RANGE(sel, 219, 222))
		uop->idep[uop->idep_count++] = FRM_UOP_DEP_LDS;
}
#endif




/*
 * Public Functions
 */

void frm_uop_init()
{
	/* GPU uop repository.
	 * The size assigned for each 'frm_uop_t' is equals to the 
	 * baseline structure size plus the size of a 'frm_thread_uop_t' 
	 * element for each thread in the warp. */
	gpu_uop_repos = repos_create(sizeof(struct frm_uop_t) + 
			sizeof(struct frm_thread_uop_t)	* frm_emu_warp_size,
			"gpu_uop_repos");
}


void frm_uop_done()
{
	repos_free(gpu_uop_repos);
}


struct frm_uop_t *frm_uop_create()
{
	struct frm_uop_t *uop;

	uop = repos_create_object(gpu_uop_repos);
	uop->id = gpu_uop_id_counter++;
	return uop;
}


void frm_uop_free(struct frm_uop_t *gpu_uop)
{
	if (!gpu_uop)
		return;
	repos_free_object(gpu_uop_repos, gpu_uop);
}


void frm_uop_list_free(struct list_t *uop_list)
{
	struct frm_uop_t *uop;
	while (list_count(uop_list))
	{
		uop = list_head(uop_list);
		list_remove(uop_list, uop);
		frm_uop_free(uop);
	}
}
