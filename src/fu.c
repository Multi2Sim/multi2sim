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

#define FU_RES_MAX	10


struct fu_res_t {
	int count;
	int oplat;
	int issuelat;
	char *name;
	uint64_t denied;
};


static struct fu_res_t fu_res_pool[fu_count] = {
	{ 0, 0, 0, "", 0 },

	{ 4, 2, 1, "IntAdd", 0 },
	{ 4, 2, 1, "IntSub", 0 },
	{ 1, 3, 1, "IntMult", 0 },
	{ 1, 20, 19, "IntDiv", 0 },
	{ 4, 2, 1, "EffAddr", 0 },
	{ 4, 1, 1, "Logical", 0 },

	{ 2, 2, 2, "FpSimple", 0 },
	{ 2, 5, 5, "FpAdd", 0 },
	{ 2, 5, 5, "FpComp", 0 },
	{ 1, 10, 10, "FpMult", 0 },
	{ 1, 20, 20, "FpDiv", 0 },
	{ 1, 40, 40, "FpSqrt", 0 },
	{ 2, 1, 1, "FpLoad", 0 },
	{ 2, 1, 1, "FpStore", 0 }
};


struct fu_t {
	uint64_t cycle_when_free[fu_count][FU_RES_MAX];
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
	int class, core;
	for (class = 1; class < fu_count; class++) {
		fprintf(stderr, "denied_fu[%s]  %lld  # Denied accesses\n",
			fu_res_pool[class].name,
			(long long) fu_res_pool[class].denied);
	}
	FOREACH_CORE
		free(CORE.fu);
}


/* reserve an fu; return fu latency or
 * 0 if it could not be reserved */
int fu_reserve(struct fu_t *fu, int class)
{
	int i;
	for (i = 0; i < fu_res_pool[class].count; i++) {
		if (fu->cycle_when_free[class][i] <= sim_cycle) {
			assert(fu_res_pool[class].issuelat > 0);
			assert(fu_res_pool[class].oplat > 0);
			fu->cycle_when_free[class][i] = sim_cycle
				+ fu_res_pool[class].issuelat;
			return fu_res_pool[class].oplat;
		}
	}
	fu_res_pool[class].denied++;
	return 0;
}


/* Release all functional units */
void fu_release(struct fu_t *fu)
{
	int i, j;
	for (i = 0; i < fu_count; i++)
		for (j = 0; j < fu_res_pool[i].count; j++)
			fu->cycle_when_free[i][j] = 0;
}

