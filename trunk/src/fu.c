/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>


/* Functional Units */

struct fu_res_t {
	int count;
	int oplat;
	int issuelat;
	char *name;
};


static struct fu_res_t fu_res_pool[fu_count] = {
	{ 0, 0, 0, "" },

	{ 4, 2, 1, "IntAdd" },
	{ 4, 2, 1, "IntSub" },
	{ 1, 3, 1, "IntMult" },
	{ 1, 20, 19, "IntDiv" },
	{ 4, 2, 1, "EffAddr" },
	{ 4, 1, 1, "Logical" },

	{ 2, 2, 2, "FpSimple" },
	{ 2, 5, 5, "FpAdd" },
	{ 2, 5, 5, "FpComp" },
	{ 1, 10, 10, "FpMult" },
	{ 1, 20, 20, "FpDiv" },
	{ 1, 40, 40, "FpComplex" }
};


void fu_reg_options()
{
	opt_reg_uint32_list("-fu:intadd", "Integer Adder (count, oplat, issuelat)",
		(uint32_t *) &fu_res_pool[fu_intadd], 3, NULL);
	opt_reg_uint32_list("-fu:intsub", "Integer Subtracter",
		(uint32_t *) &fu_res_pool[fu_intsub], 3, NULL);
	opt_reg_uint32_list("-fu:intmult", "Integer Multiplier",
		(uint32_t *) &fu_res_pool[fu_intmult], 3, NULL);
	opt_reg_uint32_list("-fu:intdiv", "Integer Divider",
		(uint32_t *) &fu_res_pool[fu_intdiv], 3, NULL);
	opt_reg_uint32_list("-fu:effaddr", "Effective Address",
		(uint32_t *) &fu_res_pool[fu_effaddr], 3, NULL);
	opt_reg_uint32_list("-fu:logical", "Logical Operations",
		(uint32_t *) &fu_res_pool[fu_logical], 3, NULL);
	
	opt_reg_uint32_list("-fu:fpsimple", "Floating-point simple operator",
		(uint32_t *) &fu_res_pool[fu_fpsimple], 3, NULL);
	opt_reg_uint32_list("-fu:fpadd", "Floating-pointer adder",
		(uint32_t *) &fu_res_pool[fu_fpadd], 3, NULL);
	opt_reg_uint32_list("-fu:fpcomp", "Floating-point comparator",
		(uint32_t *) &fu_res_pool[fu_fpcomp], 3, NULL);
	opt_reg_uint32_list("-fu:fpmult", "Floating-point multiplier",
		(uint32_t *) &fu_res_pool[fu_fpmult], 3, NULL);
	opt_reg_uint32_list("-fu:fpdiv", "Floating-point divider",
		(uint32_t *) &fu_res_pool[fu_fpdiv], 3, NULL);
	opt_reg_uint32_list("-fu:fpcomplex", "Floating-point complex operator",
		(uint32_t *) &fu_res_pool[fu_fpcomplex], 3, NULL);
}


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
	enum fu_class_enum fu_class = uop->fu_class;

	/* First time uop tries to reserve f.u. */
	if (!uop->issue_try_when)
		uop->issue_try_when = sim_cycle;

	/* Find a free f.u. */
	assert(fu_class > fu_none && fu_class < fu_count);
	assert(fu_res_pool[fu_class].count <= FU_RES_MAX);
	for (i = 0; i < fu_res_pool[fu_class].count; i++) {
		if (fu->cycle_when_free[fu_class][i] <= sim_cycle) {
			assert(fu_res_pool[fu_class].issuelat > 0);
			assert(fu_res_pool[fu_class].oplat > 0);
			fu->cycle_when_free[fu_class][i] = sim_cycle + fu_res_pool[fu_class].issuelat;
			fu->accesses[fu_class]++;
			fu->waiting_time[fu_class] += sim_cycle - uop->issue_try_when;
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

