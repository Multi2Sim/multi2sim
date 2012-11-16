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

#include <assert.h>

#include <lib/mhandle/mhandle.h>

#include "cpu.h"
#include "fu.h"


/*
 * Global variables
 */


struct x86_fu_res_t x86_fu_res_pool[x86_fu_count];




/*
 * Private
 */


/* Table indexed by 'uop->uinst->opcode', providing the functional unit type
 * associated with a given type of micro-instruction. */
static enum x86_fu_class_t fu_class_table[x86_uinst_opcode_count] =
{
	x86_fu_none,  /* x86_uinst_nop */

	x86_fu_none,  /* x86_uinst_move */
	x86_fu_intadd,  /* x86_uinst_add */
	x86_fu_intadd,  /* x86_uinst_sub */
	x86_fu_intmult,  /* x86_uinst_mult */
	x86_fu_intdiv,  /* x86_uinst_div */
	x86_fu_effaddr,  /* x86_uinst_effaddr */

	x86_fu_logic,  /* x86_uinst_and */
	x86_fu_logic,  /* x86_uinst_or */
	x86_fu_logic,  /* x86_uinst_xor */
	x86_fu_logic,  /* x86_uinst_not */
	x86_fu_logic,  /* x86_uinst_shift */
	x86_fu_logic,  /* x86_uinst_sign */

	x86_fu_none,  /* x86_uinst_fp_move */
	x86_fu_fpsimple,  /* x86_uinst_fp_sign */
	x86_fu_fpsimple,  /* x86_uinst_fp_round */

	x86_fu_fpadd,  /* x86_uinst_fp_add */
	x86_fu_fpadd,  /* x86_uinst_fp_sub */
	x86_fu_fpadd,  /* x86_uinst_fp_comp */
	x86_fu_fpmult,  /* x86_uinst_fp_mult */
	x86_fu_fpdiv,  /* x86_uinst_fp_div */

	x86_fu_fpcomplex,  /* x86_uinst_fp_exp */
	x86_fu_fpcomplex,  /* x86_uinst_fp_log */
	x86_fu_fpcomplex,  /* x86_uinst_fp_sin */
	x86_fu_fpcomplex,  /* x86_uinst_fp_cos */
	x86_fu_fpcomplex,  /* x86_uinst_fp_sincos */
	x86_fu_fpcomplex,  /* x86_uinst_fp_tan */
	x86_fu_fpcomplex,  /* x86_uinst_fp_atan */
	x86_fu_fpcomplex,  /* x86_uinst_fp_sqrt */

	x86_fu_none,  /* x86_uinst_fp_push */
	x86_fu_none,  /* x86_uinst_fp_pop */

	x86_fu_xmm_logic,  /* x86_uinst_xmm_and */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_or */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_xor */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_not */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_shift */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_sign */

	x86_fu_xmm_int,  /* x86_uinst_xmm_add */
	x86_fu_xmm_int,  /* x86_uinst_xmm_sub */
	x86_fu_xmm_int,  /* x86_uinst_xmm_comp */
	x86_fu_xmm_int,  /* x86_uinst_xmm_mult */
	x86_fu_xmm_int,  /* x86_uinst_xmm_div */

	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_add */
	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_sub */
	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_comp */
	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_mult */
	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_div */

	x86_fu_xmm_float,  /* x86_uinst_xmm_fp_sqrt */

	x86_fu_xmm_logic,  /* x86_uinst_xmm_move */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_shuf */
	x86_fu_xmm_float,  /* x86_uinst_xmm_conv */

	x86_fu_none,  /* x86_uinst_load */
	x86_fu_none,  /* x86_uinst_store */
	x86_fu_none,  /* x86_uinst_prefetch */

	x86_fu_none,  /* x86_uinst_call */
	x86_fu_none,  /* x86_uinst_ret */
	x86_fu_none,  /* x86_uinst_jump */
	x86_fu_none,  /* x86_uinst_branch */
	x86_fu_none,  /* x86_uinst_ibranch */

	x86_fu_none  /* x86_uinst_syscall */
};



/*
 * Public functions
 */


void x86_fu_init()
{
	int core;

	X86_CORE_FOR_EACH
		X86_CORE.fu = xcalloc(1, sizeof(struct x86_fu_t));
}


void x86_fu_done()
{
	int core;

	X86_CORE_FOR_EACH
		free(X86_CORE.fu);
}


/* Reserve the functional unit required by the uop.
 * The return value is the f.u. latency, or 0 if it could not
 * be reserved. */
int x86_fu_reserve(struct x86_uop_t *uop)
{
	int i;
	int core = uop->core;
	struct x86_fu_t *fu = X86_CORE.fu;
	enum x86_fu_class_t fu_class;

	/* Get the functional unit class required by the uop.
	 * If the uop does not require a functional unit, return
	 * 1 cycle latency. */
	fu_class = fu_class_table[uop->uinst->opcode];
	if (!fu_class)
		return 1;

	/* First time uop tries to reserve f.u. */
	if (!uop->issue_try_when)
		uop->issue_try_when = x86_cpu->cycle;

	/* Find a free f.u. */
	assert(fu_class > x86_fu_none && fu_class < x86_fu_count);
	assert(x86_fu_res_pool[fu_class].count <= X86_FU_RES_MAX);
	for (i = 0; i < x86_fu_res_pool[fu_class].count; i++) {
		if (fu->cycle_when_free[fu_class][i] <= x86_cpu->cycle) {
			assert(x86_fu_res_pool[fu_class].issuelat > 0);
			assert(x86_fu_res_pool[fu_class].oplat > 0);
			fu->cycle_when_free[fu_class][i] = x86_cpu->cycle + x86_fu_res_pool[fu_class].issuelat;
			fu->accesses[fu_class]++;
			fu->waiting_time[fu_class] += x86_cpu->cycle - uop->issue_try_when;
			return x86_fu_res_pool[fu_class].oplat;
		}
	}

	/* No free f.u. was found */
	fu->denied[fu_class]++;
	return 0;
}


/* Release all functional units */
void x86_fu_release(int core)
{
	int i, j;
	for (i = 0; i < x86_fu_count; i++)
		for (j = 0; j < x86_fu_res_pool[i].count; j++)
			X86_CORE.fu->cycle_when_free[i][j] = 0;
}

