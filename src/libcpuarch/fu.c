/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpuarch.h>


/*
 * Global variables
 */


struct fu_res_t fu_res_pool[fu_count];




/*
 * Private
 */


/* Table indexed by 'uop->uinst->opcode', providing the functional unit type
 * associated with a given type of micro-instruction. */
static enum fu_class_t fu_class_table[x86_uinst_opcode_count] =
{
	fu_none
};



/*
 * Public functions
 */


void fu_init()
{
	int core;
	FOREACH_CORE
		CORE.fu = calloc(1, sizeof(struct fu_t));
}


void fu_done()
{
	int core;
	FOREACH_CORE
		free(CORE.fu);
}


/* Reserve the functional unit required by the uop.
 * The return value is the f.u. latency, or 0 if it could not
 * be reserved. */
int fu_reserve(struct uop_t *uop)
{
	int i;
	int core = uop->core;
	struct fu_t *fu = CORE.fu;
	enum fu_class_t fu_class;

	/* Get the functional unit class required by the uop.
	 * If the uop does not require a functional unit, return
	 * 1 cycle latency. */
	fu_class = fu_class_table[uop->uinst->opcode];
	if (!fu_class)
		return 1;

	/* First time uop tries to reserve f.u. */
	if (!uop->issue_try_when)
		uop->issue_try_when = cpu->cycle;

	/* Find a free f.u. */
	assert(fu_class > fu_none && fu_class < fu_count);
	assert(fu_res_pool[fu_class].count <= FU_RES_MAX);
	for (i = 0; i < fu_res_pool[fu_class].count; i++) {
		if (fu->cycle_when_free[fu_class][i] <= cpu->cycle) {
			assert(fu_res_pool[fu_class].issuelat > 0);
			assert(fu_res_pool[fu_class].oplat > 0);
			fu->cycle_when_free[fu_class][i] = cpu->cycle + fu_res_pool[fu_class].issuelat;
			fu->accesses[fu_class]++;
			fu->waiting_time[fu_class] += cpu->cycle - uop->issue_try_when;
			return fu_res_pool[fu_class].oplat;
		}
	}

	/* No free f.u. was found */
	fu->denied[fu_class]++;
	return 0;
}


/* Release all functional units */
void fu_release(int core)
{
	int i, j;
	for (i = 0; i < fu_count; i++)
		for (j = 0; j < fu_res_pool[i].count; j++)
			CORE.fu->cycle_when_free[i][j] = 0;
}

