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
	fu_none,  /* x86_uinst_nop */

	fu_none,  /* x86_uinst_move */
	fu_intadd,  /* x86_uinst_add */
	fu_intsub,  /* x86_uinst_sub */
	fu_intmult,  /* x86_uinst_mult */
	fu_intdiv,  /* x86_uinst_div */
	fu_effaddr,  /* x86_uinst_effaddr */

	fu_logic,  /* x86_uinst_and */
	fu_logic,  /* x86_uinst_or */
	fu_logic,  /* x86_uinst_xor */
	fu_logic,  /* x86_uinst_not */
	fu_logic,  /* x86_uinst_shift */
	fu_logic,  /* x86_uinst_sign */

	fu_none,  /* x86_uinst_fp_move */
	fu_fpsimple,  /* x86_uinst_fp_sign */
	fu_fpsimple,  /* x86_uinst_fp_round */

	fu_fpadd,  /* x86_uinst_fp_add */
	fu_fpadd,  /* x86_uinst_fp_sub */
	fu_fpcomp,  /* x86_uinst_fp_comp */
	fu_fpmult,  /* x86_uinst_fp_mult */
	fu_fpdiv,  /* x86_uinst_fp_div */

	fu_fpcomplex,  /* x86_uinst_fp_exp */
	fu_fpcomplex,  /* x86_uinst_fp_log */
	fu_fpcomplex,  /* x86_uinst_fp_sin */
	fu_fpcomplex,  /* x86_uinst_fp_cos */
	fu_fpcomplex,  /* x86_uinst_fp_sincos */
	fu_fpcomplex,  /* x86_uinst_fp_tan */
	fu_fpcomplex,  /* x86_uinst_fp_atan */
	fu_fpcomplex,  /* x86_uinst_fp_sqrt */

	fu_none,  /* x86_uinst_fp_push */
	fu_none,  /* x86_uinst_fp_pop */

	fu_none,  /* x86_uinst_xmm_move FIXME */
	fu_none,  /* x86_uinst_xmm_conv FIXME */

	fu_none,  /* x86_uinst_load */
	fu_none,  /* x86_uinst_store */

	fu_none,  /* x86_uinst_call */
	fu_none,  /* x86_uinst_ret */
	fu_none,  /* x86_uinst_jump */
	fu_none,  /* x86_uinst_branch */
	fu_none,  /* x86_uinst_ibranch */

	fu_none  /* x86_uinst_syscall */
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

