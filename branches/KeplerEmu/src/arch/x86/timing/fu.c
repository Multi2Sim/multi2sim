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
#include <lib/util/config.h>
#include <lib/util/string.h>

#include "core.h"
#include "cpu.h"
#include "fu.h"




/*
 * Class 'X86Core'
 */


void X86CoreInitFunctionalUnits(X86Core *self)
{
	self->fu = xcalloc(1, sizeof(struct x86_fu_t));
}


void X86CoreFreeFunctionalUnits(X86Core *self)
{
	free(self->fu);
}


void X86CoreDumpFunctionalUnitsReport(X86Core *self, FILE *f)
{
	struct x86_fu_t *fu = self->fu;
	int i;

	fprintf(f, "; Functional unit pool\n");
	fprintf(f, ";    Accesses - Number of uops issued to a f.u.\n");
	fprintf(f, ";    Denied - Number of requests denied due to busy f.u.\n");
	fprintf(f, ";    WaitingTime - Average number of waiting cycles to reserve f.u.\n");

	for (i = 1; i < x86_fu_count; i++)
	{
		fprintf(f, "fu.%s.Accesses = %lld\n", x86_fu_name[i], fu->accesses[i]);
		fprintf(f, "fu.%s.Denied = %lld\n", x86_fu_name[i], fu->accesses[i]);
		fprintf(f, "fu.%s.WaitingTime = %.4g\n", x86_fu_name[i], fu->accesses[i] ?
			(double) fu->waiting_time[i] / fu->accesses[i] : 0.0);

	}

	fprintf(f, "\n");
}


/* Reserve the functional unit required by the uop.
 * The return value is the f.u. latency, or 0 if it could not
 * be reserved. */
int X86CoreReserveFunctionalUnit(X86Core *self, struct x86_uop_t *uop)
{
	X86Cpu *cpu = self->cpu;

	enum x86_fu_class_t fu_class;
	struct x86_fu_t *fu = self->fu;

	int i;

	/* Get the functional unit class required by the uop.
	 * If the uop does not require a functional unit, return
	 * 1 cycle latency. */
	fu_class = x86_fu_class_table[uop->uinst->opcode];
	if (!fu_class)
		return 1;

	/* First time uop tries to reserve f.u. */
	if (!uop->issue_try_when)
		uop->issue_try_when = asTiming(cpu)->cycle;

	/* Find a free f.u. */
	assert(fu_class > x86_fu_none && fu_class < x86_fu_count);
	assert(x86_fu_res_pool[fu_class].count <= X86_FU_RES_MAX);
	for (i = 0; i < x86_fu_res_pool[fu_class].count; i++)
	{
		if (fu->cycle_when_free[fu_class][i] <= asTiming(cpu)->cycle)
		{
			assert(x86_fu_res_pool[fu_class].issuelat > 0);
			assert(x86_fu_res_pool[fu_class].oplat > 0);
			fu->cycle_when_free[fu_class][i] = asTiming(cpu)->cycle
					+ x86_fu_res_pool[fu_class].issuelat;
			fu->accesses[fu_class]++;
			fu->waiting_time[fu_class] += asTiming(cpu)->cycle - uop->issue_try_when;
			return x86_fu_res_pool[fu_class].oplat;
		}
	}

	/* No free f.u. was found */
	fu->denied[fu_class]++;
	return 0;
}


/* Release all functional units */
void X86CoreReleaseAllFunctionalUnits(X86Core *self)
{
	int i;
	int j;

	for (i = 0; i < x86_fu_count; i++)
		for (j = 0; j < x86_fu_res_pool[i].count; j++)
			self->fu->cycle_when_free[i][j] = 0;
}




/*
 * Public
 */

/* Default configuration for functional units. Fields are:
 *	count - Number of units.
 *	oplat - Latency from issue to completion.
 *	issuelat - Latency from issue to next issue.
 */
struct x86_fu_res_t x86_fu_res_pool[x86_fu_count] =
{
	{ 0, 0, 0 },  /* Unused */

	{ 4, 1, 1 },  /* x86_fu_int_add */
	{ 2, 2, 2 },  /* x86_fu_int_mult */
	{ 1, 5, 5 },  /* x86_fu_int_div */

	{ 4, 2, 2 },  /* x86_fu_effaddr */
	{ 4, 1, 1 },  /* x86_fu_logic */

	{ 2, 2, 2 },  /* x86_fu_float_simple */
	{ 2, 5, 5 },  /* x86_fu_float_add */
	{ 2, 2, 2 },  /* x86_fu_float_comp */
	{ 1, 10, 10 },  /* x86_fu_float_mult */
	{ 1, 15, 15 },  /* x86_fu_float_div */
	{ 1, 20, 20 },  /* x86_fu_float_complex */

	{ 1, 1, 1 },  /* x86_fu_xmm_int_add */
	{ 1, 2, 2 },  /* x86_fu_xmm_int_mult */
	{ 1, 5, 5 },  /* x86_fu_xmm_int_div */

	{ 1, 1, 1 },  /* x86_fu_xmm_logic */

	{ 1, 5, 5 },  /* x86_fu_xmm_float_add */
	{ 1, 2, 2 },  /* x86_fu_xmm_float_comp */
	{ 1, 10, 10 },  /* x86_fu_xmm_float_mult */
	{ 1, 15, 15 },  /* x86_fu_xmm_float_div */
	{ 1, 5, 5 },  /* x86_fu_xmm_float_conv */
	{ 1, 20, 20 }  /* x86_fu_xmm_float_complex */
};

char *x86_fu_name[x86_fu_count] =
{
	"<invalid>",

	"IntAdd",
	"IntMult",
	"IntDiv",

	"EffAddr",
	"Logic",

	"FloatSimple",
	"FloatAdd",
	"FloatComp",
	"FloatMult",
	"FloatDiv",
	"FloatComplex",

	"XMMIntAdd",
	"XMMIntMult",
	"XMMIntDiv",

	"XMMLogic",
	
	"XMMFloatAdd",
	"XMMFloatComp",
	"XMMFloatMult",
	"XMMFloatDiv",
	"XMMFloatConv",
	"XMMFloatComplex",
};


/* Table indexed by 'uop->uinst->opcode', providing the functional unit type
 * associated with a given type of micro-instruction. */
enum x86_fu_class_t x86_fu_class_table[x86_uinst_opcode_count] =
{
	x86_fu_none,  /* x86_uinst_nop */

	x86_fu_none,  /* x86_uinst_move */
	x86_fu_int_add,  /* x86_uinst_add */
	x86_fu_int_add,  /* x86_uinst_sub */
	x86_fu_int_mult,  /* x86_uinst_mult */
	x86_fu_int_div,  /* x86_uinst_div */
	x86_fu_effaddr,  /* x86_uinst_effaddr */

	x86_fu_logic,  /* x86_uinst_and */
	x86_fu_logic,  /* x86_uinst_or */
	x86_fu_logic,  /* x86_uinst_xor */
	x86_fu_logic,  /* x86_uinst_not */
	x86_fu_logic,  /* x86_uinst_shift */
	x86_fu_logic,  /* x86_uinst_sign */

	x86_fu_none,  /* x86_uinst_fp_move */
	x86_fu_float_simple,  /* x86_uinst_fp_sign */
	x86_fu_float_simple,  /* x86_uinst_fp_round */

	x86_fu_float_add,  /* x86_uinst_fp_add */
	x86_fu_float_add,  /* x86_uinst_fp_sub */
	x86_fu_float_comp,  /* x86_uinst_fp_comp */
	x86_fu_float_mult,  /* x86_uinst_fp_mult */
	x86_fu_float_div,  /* x86_uinst_fp_div */

	x86_fu_float_complex,  /* x86_uinst_fp_exp */
	x86_fu_float_complex,  /* x86_uinst_fp_log */
	x86_fu_float_complex,  /* x86_uinst_fp_sin */
	x86_fu_float_complex,  /* x86_uinst_fp_cos */
	x86_fu_float_complex,  /* x86_uinst_fp_sincos */
	x86_fu_float_complex,  /* x86_uinst_fp_tan */
	x86_fu_float_complex,  /* x86_uinst_fp_atan */
	x86_fu_float_complex,  /* x86_uinst_fp_sqrt */

	x86_fu_none,  /* x86_uinst_fp_push */
	x86_fu_none,  /* x86_uinst_fp_pop */

	x86_fu_xmm_logic,  /* x86_uinst_xmm_and */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_or */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_xor */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_not */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_nand */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_shift */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_sign */

	x86_fu_xmm_int_add,  /* x86_uinst_xmm_add */
	x86_fu_xmm_int_add,  /* x86_uinst_xmm_sub */
	x86_fu_xmm_int_add,  /* x86_uinst_xmm_comp */
	x86_fu_xmm_int_mult,  /* x86_uinst_xmm_mult */
	x86_fu_xmm_int_div,  /* x86_uinst_xmm_div */

	x86_fu_xmm_float_add,  /* x86_uinst_xmm_fp_add */
	x86_fu_xmm_float_add,  /* x86_uinst_xmm_fp_sub */
	x86_fu_xmm_float_comp,  /* x86_uinst_xmm_fp_comp */
	x86_fu_xmm_float_mult,  /* x86_uinst_xmm_fp_mult */
	x86_fu_xmm_float_div,  /* x86_uinst_xmm_fp_div */

	x86_fu_xmm_float_complex,  /* x86_uinst_xmm_fp_sqrt */

	x86_fu_xmm_logic,  /* x86_uinst_xmm_move */
	x86_fu_xmm_logic,  /* x86_uinst_xmm_shuf */
	x86_fu_xmm_float_conv,  /* x86_uinst_xmm_conv */

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


void X86ReadFunctionalUnitsConfig(struct config_t *config)
{
	struct x86_fu_res_t *fu_res;

	char buf[MAX_STRING_SIZE];
	char *section;

	int i;

	section = "FunctionalUnits";
	for (i = 1; i < x86_fu_count; i++)
	{
		fu_res = &x86_fu_res_pool[i];

		snprintf(buf, sizeof buf, "%s.Count", x86_fu_name[i]);
		fu_res->count = config_read_int(config, section, buf, fu_res->count);

		snprintf(buf, sizeof buf, "%s.OpLat", x86_fu_name[i]);
		fu_res->oplat = config_read_int(config, section, buf, fu_res->oplat);

		snprintf(buf, sizeof buf, "%s.IssueLat", x86_fu_name[i]);
		fu_res->issuelat = config_read_int(config, section, buf, fu_res->issuelat);
	}
}


void X86DumpFunctionalUnitsConfig(FILE *f)
{
	struct x86_fu_res_t *fu_res;
	int i;

	fprintf(f, "[ Config.FunctionalUnits ]\n");
	for (i = 1; i < x86_fu_count; i++)
	{
		fu_res = &x86_fu_res_pool[i];
		fprintf(f, "%s.Count = %d\n", x86_fu_name[i], fu_res->count);
		fprintf(f, "%s.OpLat = %d\n", x86_fu_name[i], fu_res->oplat);
		fprintf(f, "%s.IssueLat = %d\n", x86_fu_name[i], fu_res->issuelat);

	}
	fprintf(f, "\n");
}
