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

#include <x86-timing.h>


/* Global variables */

char *x86_reg_file_kind_map[] = { "Shared", "Private" };
enum x86_reg_file_kind_t x86_reg_file_kind = x86_reg_file_kind_private;  /* Sharing policy for register file */
int x86_reg_file_int_size = 80;  /* Per-thread integer register file size */
int x86_reg_file_fp_size = 40;  /* Per-thread floating-point register file size */




/* Private variables and functions */

static uint32_t rf_int_local_size;  /* maximum number of registers allowed per thread */
static uint32_t rf_fp_local_size;


/* Reclaim an integer physical register, and return its identifier. */
static int rf_int_reclaim(int core, int thread)
{
	int phreg;
	struct x86_reg_file_t *rf = X86_THREAD.rf;

	/* Obtain a register from the free list */
	assert(rf->int_free_phreg_count > 0);
	phreg = rf->int_free_phreg[rf->int_free_phreg_count - 1];
	rf->int_free_phreg_count--;
	X86_CORE.reg_file_int_count++;
	X86_THREAD.reg_file_int_count++;
	assert(!rf->int_phreg[phreg].busy);
	assert(!rf->int_phreg[phreg].pending);
	return phreg;
}


/* Reclaim an FP physical register, and return its identifier. */
static int rf_fp_reclaim(int core, int thread)
{
	int phreg;
	struct x86_reg_file_t *rf = X86_THREAD.rf;

	/* Obtain a register from the free list */
	assert(rf->fp_free_phreg_count > 0);
	phreg = rf->fp_free_phreg[rf->fp_free_phreg_count - 1];
	rf->fp_free_phreg_count--;
	X86_CORE.reg_file_fp_count++;
	X86_THREAD.reg_file_fp_count++;
	assert(!rf->fp_phreg[phreg].busy);
	assert(!rf->fp_phreg[phreg].pending);
	return phreg;
}





/* Public functions */

static void rf_init_thread(int core, int thread)
{
	int dep, phreg, fphreg;
	struct x86_reg_file_t *rf = X86_THREAD.rf;
	
	/* Initial mapping for the integer register file.
	 * Map each logical register to a new physical register,
	 * and map all flags to the first allocated physical register. */
	fphreg = -1;
	for (dep = 0; dep < x86_dep_int_count; dep++) {
		if (X86_DEP_IS_FLAG(dep + x86_dep_int_first)) {
			assert(fphreg >= 0);
			phreg = fphreg;
		} else {
			phreg = rf_int_reclaim(core, thread);
			fphreg = phreg;
		}
		rf->int_phreg[phreg].busy++;
		rf->int_rat[dep] = phreg;
	}

	/* Initial mapping for floating-point registers. */
	for (dep = 0; dep < x86_dep_fp_count; dep++) {
		phreg = rf_fp_reclaim(core, thread);
		rf->fp_phreg[phreg].busy++;
		rf->fp_rat[dep] = phreg;
	}
}


void x86_reg_file_init(void)
{
	int core, thread;
	
	/* Register file size restrictions */
	if (x86_reg_file_int_size < X86_REG_FILE_MIN_INT_SIZE)
		fatal("rf_int_size must be at least %d", X86_REG_FILE_MIN_INT_SIZE);
	if (x86_reg_file_fp_size < X86_REG_FILE_MIN_FP_SIZE)
		fatal("rf_fp_size must be at least %d", X86_REG_FILE_MIN_FP_SIZE);
	
	/* Maximum size accessible to threads */
	if (x86_reg_file_kind == x86_reg_file_kind_private) {
		rf_int_local_size = x86_reg_file_int_size;
		rf_fp_local_size = x86_reg_file_fp_size;
	} else {
		rf_int_local_size = x86_reg_file_int_size * x86_cpu_num_threads;
		rf_fp_local_size = x86_reg_file_fp_size * x86_cpu_num_threads;
	}

	/* Create and initialize register files */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH {
		X86_THREAD.rf = x86_reg_file_create(rf_int_local_size, rf_fp_local_size);
		rf_init_thread(core, thread);
	}
}


void x86_reg_file_done(void)
{
	int core, thread;
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		x86_reg_file_free(X86_THREAD.rf);
}


struct x86_reg_file_t *x86_reg_file_create(int int_size, int fp_size)
{
	struct x86_reg_file_t *rf;
	int phreg;
	
	/* Create structure */
	rf = calloc(1, sizeof(struct x86_reg_file_t));
	
	/* Integer register file and free list */
	rf->int_phreg_count = int_size;
	rf->int_phreg = calloc(int_size, sizeof(struct x86_phreg_t));
	rf->int_free_phreg = calloc(int_size, sizeof(int));
	rf->int_free_phreg_count = int_size;
	for (phreg = 0; phreg < int_size; phreg++)
		rf->int_free_phreg[phreg] = phreg;

	/* Floating-point register file and free list */
	rf->fp_phreg_count = fp_size;
	rf->fp_phreg = calloc(fp_size, sizeof(struct x86_phreg_t));
	rf->fp_free_phreg = calloc(fp_size, sizeof(int));
	rf->fp_free_phreg_count = fp_size;
	for (phreg = 0; phreg < fp_size; phreg++)
		rf->fp_free_phreg[phreg] = phreg;
	
	/* Return */
	return rf;
}


void x86_reg_file_free(struct x86_reg_file_t *rf)
{
	free(rf->int_phreg);
	free(rf->int_free_phreg);
	free(rf->fp_phreg);
	free(rf->fp_free_phreg);
	free(rf);
}


void x86_reg_file_dump(int core, int thread, FILE *f)
{
	int i;

	/* Integer register file */
	fprintf(f, "Integer register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < rf_int_local_size; i++) {
		fprintf(f, "  %3d%c[%d-%d]", i, X86_THREAD.rf->int_phreg[i].busy ? ' ' : '*',
			X86_THREAD.rf->int_phreg[i].busy,
			X86_THREAD.rf->int_phreg[i].pending);
		if (i % 5 == 4 && i != rf_int_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nIteger Register Aliasing Table:\n");
	for (i = x86_dep_int_first; i <= x86_dep_int_last; i++) {
		fprintf(f, "  %2d->%-3d", i, X86_THREAD.rf->int_rat[i - x86_dep_int_first]);
		if ((i - x86_dep_int_first) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "int_free_phreg_count  %d  # Number of free integer registers\n",
		X86_THREAD.rf->int_free_phreg_count);
	fprintf(f, "\n");

	/* Floating point register file */
	fprintf(f, "Floating-point register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < rf_fp_local_size; i++) {
		fprintf(f, "  %3d%c[%d-%d]", i, X86_THREAD.rf->fp_phreg[i].busy ? ' ' : '*',
			X86_THREAD.rf->fp_phreg[i].busy,
			X86_THREAD.rf->fp_phreg[i].pending);
		if (i % 5 == 4 && i != rf_fp_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nIteger Register Aliasing Table:\n");
	for (i = x86_dep_fp_first; i <= x86_dep_fp_last; i++) {
		fprintf(f, "  %2d->%-3d", i, X86_THREAD.rf->fp_rat[i - x86_dep_fp_first]);
		if ((i - x86_dep_fp_first) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "fp_free_phreg_count  %d  # Number of free floating-point registers\n",
		X86_THREAD.rf->fp_free_phreg_count);
	fprintf(f, "\n");
}


/* Set the number of logical/physical registers needed by an instruction.
 * If there are only flags as destination dependences, only one register is
 * needed. Otherwise, one register per destination operand is needed, and the
 * output flags will be mapped to one of the destination physical registers
 * used for operands. */
void x86_reg_file_count_deps(struct x86_uop_t *uop)
{
	int dep, loreg;
	int int_count, fp_count, flag_count;

	/* Initialize */
	uop->idep_count = 0;
	uop->odep_count = 0;
	uop->ph_int_idep_count = 0;
	uop->ph_fp_idep_count = 0;
	uop->ph_int_odep_count = 0;
	uop->ph_fp_odep_count = 0;

	/* Output dependences */
	int_count = fp_count = flag_count = 0;
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++) {
		loreg = uop->uinst->odep[dep];
		if (X86_DEP_IS_FLAG(loreg))
			flag_count++;
		else if (X86_DEP_IS_INT_REG(loreg))
			int_count++;
		else if (X86_DEP_IS_FP_REG(loreg))
			fp_count++;
	}
	uop->odep_count = flag_count + int_count + fp_count;
	uop->ph_int_odep_count = flag_count && !int_count ? 1 : int_count;
	uop->ph_fp_odep_count = fp_count;

	/* Input dependences */
	int_count = fp_count = flag_count = 0;
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++) {
		loreg = uop->uinst->idep[dep];
		if (X86_DEP_IS_FLAG(loreg))
			flag_count++;
		else if (X86_DEP_IS_INT_REG(loreg))
			int_count++;
		else if (X86_DEP_IS_FP_REG(loreg))
			fp_count++;
	}
	uop->idep_count = flag_count + int_count + fp_count;
	uop->ph_int_idep_count = flag_count + int_count;
	uop->ph_fp_idep_count = fp_count;
}



int x86_reg_file_can_rename(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	/* Detect negative cases. */
	if (x86_reg_file_kind == x86_reg_file_kind_private) {
		if (X86_THREAD.reg_file_int_count + uop->ph_int_odep_count > rf_int_local_size)
			return 0;
		if (X86_THREAD.reg_file_fp_count + uop->ph_fp_odep_count > rf_fp_local_size)
			return 0;
	} else {
		if (X86_CORE.reg_file_int_count + uop->ph_int_odep_count > rf_int_local_size)
			return 0;
		if (X86_CORE.reg_file_fp_count + uop->ph_fp_odep_count > rf_fp_local_size)
			return 0;
	}

	/* Uop can be renamed. */
	return 1;
}


void x86_reg_file_rename(struct x86_uop_t *uop)
{
	int dep;
	int loreg, streg, phreg, ophreg;
	int flag_phreg, flag_count;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *rf = X86_THREAD.rf;

	/* Update floating-point top of stack */
	if (uop->uinst->opcode == x86_uinst_fp_pop)
	{
		/* Pop floating-point stack */
		rf->fp_top_of_stack = (rf->fp_top_of_stack + 1) % 8;
	}
	else if (uop->uinst->opcode == x86_uinst_fp_push)
	{
		/* Push floating-point stack */
		rf->fp_top_of_stack = (rf->fp_top_of_stack + 7) % 8;
	}

	/* Rename input int/FP registers */
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++)
	{
		loreg = uop->uinst->idep[dep];
		if (X86_DEP_IS_INT_REG(loreg))
		{
			phreg = rf->int_rat[loreg - x86_dep_int_first];
			uop->ph_idep[dep] = phreg;
			X86_THREAD.rat_int_reads++;
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + rf->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));

			/* Rename it. */
			phreg = rf->fp_rat[streg - x86_dep_fp_first];
			uop->ph_idep[dep] = phreg;
			X86_THREAD.rat_fp_reads++;
		}
		else
		{
			uop->ph_idep[dep] = -1;
		}
	}

	/* Rename output int/FP registers (not flags) */
	flag_phreg = -1;
	flag_count = 0;
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
	{
		loreg = uop->uinst->odep[dep];
		if (X86_DEP_IS_FLAG(loreg))
		{
			/* Record a new flag */
			flag_count++;
		}
		else if (X86_DEP_IS_INT_REG(loreg))
		{
			/* Reclaim a free integer register */
			phreg = rf_int_reclaim(core, thread);
			rf->int_phreg[phreg].busy++;
			rf->int_phreg[phreg].pending = 1;
			ophreg = rf->int_rat[loreg - x86_dep_int_first];
			if (flag_phreg < 0)
				flag_phreg = phreg;

			/* Allocate it */
			uop->ph_odep[dep] = phreg;
			uop->ph_oodep[dep] = ophreg;
			rf->int_rat[loreg - x86_dep_int_first] = phreg;
			X86_THREAD.rat_int_writes++;

		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + rf->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));
			
			/* Reclaim a free FP register */
			phreg = rf_fp_reclaim(core, thread);
			rf->fp_phreg[phreg].busy++;
			rf->fp_phreg[phreg].pending = 1;
			ophreg = rf->fp_rat[streg - x86_dep_fp_first];
			
			/* Allocate it */
			uop->ph_odep[dep] = phreg;
			uop->ph_oodep[dep] = ophreg;
			rf->fp_rat[streg - x86_dep_fp_first] = phreg;
			X86_THREAD.rat_fp_writes++;
		}
		else
		{
			/* Not a valid output dependence */
			uop->ph_odep[dep] = -1;
			uop->ph_oodep[dep] = -1;
		}
	}

	/* Rename flags */
	if (flag_count > 0) {
		if (flag_phreg < 0)
			flag_phreg = rf_int_reclaim(core, thread);
		for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++) {
			loreg = uop->uinst->odep[dep];
			if (!X86_DEP_IS_FLAG(loreg))
				continue;
			rf->int_phreg[flag_phreg].busy++;
			rf->int_phreg[flag_phreg].pending = 1;
			ophreg = rf->int_rat[loreg - x86_dep_int_first];
			uop->ph_oodep[dep] = ophreg;
			uop->ph_odep[dep] = flag_phreg;
			rf->int_rat[loreg - x86_dep_int_first] = flag_phreg;
		}
	}
}


/* Return 1 if input dependencies are resolved */
int x86_reg_file_ready(struct x86_uop_t *uop)
{
	int loreg, phreg, dep;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *rf = X86_THREAD.rf;
	
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++) {
		loreg = uop->uinst->idep[dep];
		phreg = uop->ph_idep[dep];
		if (X86_DEP_IS_INT_REG(loreg) && rf->int_phreg[phreg].pending)
			return 0;
		if (X86_DEP_IS_FP_REG(loreg) && rf->fp_phreg[phreg].pending)
			return 0;
	}
	return 1;
}


void x86_reg_file_write(struct x86_uop_t *uop)
{
	int dep, loreg, phreg;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *rf = X86_THREAD.rf;
	
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++) {
		loreg = uop->uinst->odep[dep];
		phreg = uop->ph_odep[dep];
		if (X86_DEP_IS_INT_REG(loreg))
			rf->int_phreg[phreg].pending = 0;
		else if (X86_DEP_IS_FP_REG(loreg))
			rf->fp_phreg[phreg].pending = 0;
	}
}


void x86_reg_file_undo(struct x86_uop_t *uop)
{
	int dep, loreg, streg, phreg, ophreg;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *rf = X86_THREAD.rf;

	/* Undo mappings in reverse order, in case an instruction has a
	 * duplicated output dependence. */
	assert(uop->specmode);
	for (dep = X86_UINST_MAX_ODEPS - 1; dep >= 0; dep--)
	{
		loreg = uop->uinst->odep[dep];
		phreg = uop->ph_odep[dep];
		ophreg = uop->ph_oodep[dep];
		if (X86_DEP_IS_INT_REG(loreg))
		{
			/* Decrease busy counter and free if 0. */
			assert(rf->int_phreg[phreg].busy > 0);
			assert(!rf->int_phreg[phreg].pending);
			rf->int_phreg[phreg].busy--;
			if (!rf->int_phreg[phreg].busy)
			{
				assert(rf->int_free_phreg_count < rf_int_local_size);
				assert(X86_CORE.reg_file_int_count > 0 && X86_THREAD.reg_file_int_count > 0);
				rf->int_free_phreg[rf->int_free_phreg_count] = phreg;
				rf->int_free_phreg_count++;
				X86_CORE.reg_file_int_count--;
				X86_THREAD.reg_file_int_count--;
			}

			/* Return to previous mapping */
			rf->int_rat[loreg - x86_dep_int_first] = ophreg;
			assert(rf->int_phreg[ophreg].busy);
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + rf->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));
			
			/* Decrease busy counter and free if 0. */
			assert(rf->fp_phreg[phreg].busy > 0);
			assert(!rf->fp_phreg[phreg].pending);
			rf->fp_phreg[phreg].busy--;
			if (!rf->fp_phreg[phreg].busy)
			{
				assert(rf->fp_free_phreg_count < rf_fp_local_size);
				assert(X86_CORE.reg_file_fp_count > 0 && X86_THREAD.reg_file_fp_count > 0);
				rf->fp_free_phreg[rf->fp_free_phreg_count] = phreg;
				rf->fp_free_phreg_count++;
				X86_CORE.reg_file_fp_count--;
				X86_THREAD.reg_file_fp_count--;
			}

			/* Return to previous mapping */
			rf->fp_rat[streg - x86_dep_fp_first] = ophreg;
			assert(rf->fp_phreg[ophreg].busy);
		}
		else
		{
			/* Not a valid dependence. */
			assert(phreg == -1);
			assert(ophreg == -1);
		}
	}

	/* Undo modification in floating-point top of stack */
	if (uop->uinst->opcode == x86_uinst_fp_pop)
	{
		/* Inverse-pop floating-point stack */
		rf->fp_top_of_stack = (rf->fp_top_of_stack + 7) % 8;
	}
	else if (uop->uinst->opcode == x86_uinst_fp_push)
	{
		/* Inverse-push floating-point stack */
		rf->fp_top_of_stack = (rf->fp_top_of_stack + 1) % 8;
	}
}


void x86_reg_file_commit(struct x86_uop_t *uop)
{
	int dep, loreg, phreg, ophreg;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *rf = X86_THREAD.rf;

	assert(!uop->specmode);
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
	{
		loreg = uop->uinst->odep[dep];
		phreg = uop->ph_odep[dep];
		ophreg = uop->ph_oodep[dep];

		if (X86_DEP_IS_INT_REG(loreg))
		{
			/* Decrease counter of previous mapping and free if 0. */
			assert(rf->int_phreg[ophreg].busy > 0);
			rf->int_phreg[ophreg].busy--;
			if (!rf->int_phreg[ophreg].busy)
			{
				assert(!rf->int_phreg[ophreg].pending);
				assert(rf->int_free_phreg_count < rf_int_local_size);
				assert(X86_CORE.reg_file_int_count > 0 && X86_THREAD.reg_file_int_count > 0);
				rf->int_free_phreg[rf->int_free_phreg_count] = ophreg;
				rf->int_free_phreg_count++;
				X86_CORE.reg_file_int_count--;
				X86_THREAD.reg_file_int_count--;
			}
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Decrease counter of previous mapping and free if 0. */
			assert(rf->fp_phreg[ophreg].busy > 0);
			rf->fp_phreg[ophreg].busy--;
			if (!rf->fp_phreg[ophreg].busy)
			{
				assert(!rf->fp_phreg[ophreg].pending);
				assert(rf->fp_free_phreg_count < rf_fp_local_size);
				assert(X86_CORE.reg_file_fp_count > 0 && X86_THREAD.reg_file_fp_count > 0);
				rf->fp_free_phreg[rf->fp_free_phreg_count] = ophreg;
				rf->fp_free_phreg_count++;
				X86_CORE.reg_file_fp_count--;
				X86_THREAD.reg_file_fp_count--;
			}
		}
		else
		{
			/* Not a valid dependence. */
			assert(phreg == -1);
			assert(ophreg == -1);
		}
	}
}


void x86_reg_file_check_integrity(int core, int thread)
{
	struct x86_reg_file_t *rf = X86_THREAD.rf;
	struct x86_uop_t *uop;
	int loreg, phreg, ophreg, dep, i;

	/* Check that all registers in the free list are actually free. */
	for (i = 0; i < rf->int_free_phreg_count; i++) {
		phreg = rf->int_free_phreg[i];
		assert(!rf->int_phreg[phreg].busy);
		assert(!rf->int_phreg[phreg].pending);
	}
	for (i = 0; i < rf->fp_free_phreg_count; i++) {
		phreg = rf->fp_free_phreg[i];
		assert(!rf->fp_phreg[phreg].busy);
		assert(!rf->fp_phreg[phreg].pending);
	}

	/* Check that all mapped registers are busy */
	for (loreg = x86_dep_int_first; loreg <= x86_dep_int_last; loreg++) {
		phreg = rf->int_rat[loreg - x86_dep_int_first];
		assert(rf->int_phreg[phreg].busy);
	}
	for (loreg = x86_dep_fp_first; loreg <= x86_dep_fp_last; loreg++) {
		phreg = rf->fp_rat[loreg - x86_dep_fp_first];
		assert(rf->fp_phreg[phreg].busy);
	}

	/* Check that all destination and previous destination
	 * registers of instructions in the rob are busy */
	for (i = 0; i < X86_THREAD.rob_count; i++) {
		uop = x86_rob_get(core, thread, i);
		assert(uop);
		for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++) {
			loreg = uop->uinst->odep[dep];
			phreg = uop->ph_odep[dep];
			ophreg = uop->ph_oodep[dep];
			if (X86_DEP_IS_INT_REG(loreg)) {
				assert(rf->int_phreg[phreg].busy);
				assert(rf->int_phreg[ophreg].busy);
			} else if (X86_DEP_IS_FP_REG(loreg)) {
				assert(rf->fp_phreg[phreg].busy);
				assert(rf->fp_phreg[ophreg].busy);
			} else {
				assert(phreg == -1);
				assert(ophreg == -1);
			}
		}
	}
}

