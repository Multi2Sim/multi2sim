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
#include <lib/util/debug.h>

#include "cpu.h"
#include "reg-file.h"
#include "rob.h"


/* Global variables */

char *x86_reg_file_kind_map[] = { "Shared", "Private" };
enum x86_reg_file_kind_t x86_reg_file_kind = x86_reg_file_kind_private;  /* Sharing policy for register file */
int x86_reg_file_int_size = 80;  /* Per-thread integer register file size */
int x86_reg_file_fp_size = 40;  /* Per-thread floating-point register file size */
int x86_reg_file_xmm_size = 40;  /* Per-thread xmm register file size */




/* Private variables and functions */

/* maximum number of registers allowed per thread */
static int x86_reg_file_int_local_size;
static int x86_reg_file_fp_local_size;
static int x86_reg_file_xmm_local_size;


/* Reclaim an integer physical register, and return its identifier. */
static int x86_reg_file_int_reclaim(int core, int thread)
{
	int phreg;
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

	/* Obtain a register from the free list */
	assert(reg_file->int_free_phreg_count > 0);
	phreg = reg_file->int_free_phreg[reg_file->int_free_phreg_count - 1];
	reg_file->int_free_phreg_count--;
	X86_CORE.reg_file_int_count++;
	X86_THREAD.reg_file_int_count++;
	assert(!reg_file->int_phreg[phreg].busy);
	assert(!reg_file->int_phreg[phreg].pending);
	return phreg;
}


/* Reclaim an FP physical register, and return its identifier. */
static int x86_reg_file_fp_reclaim(int core, int thread)
{
	int phreg;
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

	/* Obtain a register from the free list */
	assert(reg_file->fp_free_phreg_count > 0);
	phreg = reg_file->fp_free_phreg[reg_file->fp_free_phreg_count - 1];
	reg_file->fp_free_phreg_count--;
	X86_CORE.reg_file_fp_count++;
	X86_THREAD.reg_file_fp_count++;
	assert(!reg_file->fp_phreg[phreg].busy);
	assert(!reg_file->fp_phreg[phreg].pending);
	return phreg;
}


/* Reclaim an xmm physical register, and return its identifier. */
static int x86_reg_file_xmm_reclaim(int core, int thread)
{
	int phreg;
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

	/* Obtain a register from the free list */
	assert(reg_file->xmm_free_phreg_count > 0);
	phreg = reg_file->xmm_free_phreg[reg_file->xmm_free_phreg_count - 1];
	reg_file->xmm_free_phreg_count--;
	X86_CORE.reg_file_xmm_count++;
	X86_THREAD.reg_file_xmm_count++;
	assert(!reg_file->xmm_phreg[phreg].busy);
	assert(!reg_file->xmm_phreg[phreg].pending);
	return phreg;
}



/*
 * Public functions
 */

static void x86_reg_file_init_thread(int core, int thread)
{
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;
	
	int dep;
	int phreg;
	int fphreg;

	/* Initial mapping for the integer register file.
	 * Map each logical register to a new physical register,
	 * and map all flags to the first allocated physical register. */
	fphreg = -1;
	for (dep = 0; dep < x86_dep_int_count; dep++)
	{
		if (X86_DEP_IS_FLAG(dep + x86_dep_int_first))
		{
			assert(fphreg >= 0);
			phreg = fphreg;
		}
		else
		{
			phreg = x86_reg_file_int_reclaim(core, thread);
			fphreg = phreg;
		}
		reg_file->int_phreg[phreg].busy++;
		reg_file->int_rat[dep] = phreg;
	}

	/* Initial mapping for floating-point registers. */
	for (dep = 0; dep < x86_dep_fp_count; dep++)
	{
		phreg = x86_reg_file_fp_reclaim(core, thread);
		reg_file->fp_phreg[phreg].busy++;
		reg_file->fp_rat[dep] = phreg;
	}
	
	/* Initial mapping for xmm registers. */
	for (dep = 0; dep < x86_dep_xmm_count; dep++)
	{
		phreg = x86_reg_file_xmm_reclaim(core, thread);
		reg_file->xmm_phreg[phreg].busy++;
		reg_file->xmm_rat[dep] = phreg;
	}
}


void x86_reg_file_init(void)
{
	int core;
	int thread;
	
	/* Register file size restrictions */
	if (x86_reg_file_int_size < X86_REG_FILE_MIN_INT_SIZE)
		fatal("rf_int_size must be at least %d", X86_REG_FILE_MIN_INT_SIZE);
	if (x86_reg_file_fp_size < X86_REG_FILE_MIN_FP_SIZE)
		fatal("rf_fp_size must be at least %d", X86_REG_FILE_MIN_FP_SIZE);
	if (x86_reg_file_xmm_size < X86_REG_FILE_MIN_XMM_SIZE)
		fatal("rf_xmm_size must be at least %d", X86_REG_FILE_MIN_XMM_SIZE);
	
	/* Maximum size accessible to threads */
	if (x86_reg_file_kind == x86_reg_file_kind_private)
	{
		x86_reg_file_int_local_size = x86_reg_file_int_size;
		x86_reg_file_fp_local_size = x86_reg_file_fp_size;
		x86_reg_file_xmm_local_size = x86_reg_file_xmm_size;
	}
	else
	{
		x86_reg_file_int_local_size = x86_reg_file_int_size * x86_cpu_num_threads;
		x86_reg_file_fp_local_size = x86_reg_file_fp_size * x86_cpu_num_threads;
		x86_reg_file_xmm_local_size = x86_reg_file_xmm_size * x86_cpu_num_threads;
	}

	/* Create and initialize register files */
	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
	{
		X86_THREAD.reg_file = x86_reg_file_create(x86_reg_file_int_local_size,
				x86_reg_file_fp_local_size, x86_reg_file_xmm_local_size);
		x86_reg_file_init_thread(core, thread);
	}
}


void x86_reg_file_done(void)
{
	int core;
	int thread;

	X86_CORE_FOR_EACH X86_THREAD_FOR_EACH
		x86_reg_file_free(X86_THREAD.reg_file);
}


struct x86_reg_file_t *x86_reg_file_create(int int_size, int fp_size, int xmm_size)
{
	struct x86_reg_file_t *reg_file;
	int phreg;
	
	/* Integer register file */
	reg_file = xcalloc(1, sizeof(struct x86_reg_file_t));
	reg_file->int_phreg_count = int_size;
	reg_file->int_phreg = xcalloc(int_size, sizeof(struct x86_phreg_t));

	/* Free list */
	reg_file->int_free_phreg_count = int_size;
	reg_file->int_free_phreg = xcalloc(int_size, sizeof(int));
	for (phreg = 0; phreg < int_size; phreg++)
		reg_file->int_free_phreg[phreg] = phreg;

	/* Floating-point register file */
	reg_file->fp_phreg_count = fp_size;
	reg_file->fp_phreg = xcalloc(fp_size, sizeof(struct x86_phreg_t));

	/* Free list */
	reg_file->fp_free_phreg_count = fp_size;
	reg_file->fp_free_phreg = xcalloc(fp_size, sizeof(int));

	/* Initialize free list */
	for (phreg = 0; phreg < fp_size; phreg++)
		reg_file->fp_free_phreg[phreg] = phreg;
	
	/* XMM register file */
	reg_file->xmm_phreg_count = xmm_size;
	reg_file->xmm_phreg = xcalloc(xmm_size, sizeof(struct x86_phreg_t));

	/* Free list */
	reg_file->xmm_free_phreg_count = xmm_size;
	reg_file->xmm_free_phreg = xcalloc(xmm_size, sizeof(int));

	/* Initialize free list */
	for (phreg = 0; phreg < xmm_size; phreg++)
		reg_file->xmm_free_phreg[phreg] = phreg;

	/* Return */
	return reg_file;
}


void x86_reg_file_free(struct x86_reg_file_t *reg_file)
{
	free(reg_file->int_phreg);
	free(reg_file->int_free_phreg);
	free(reg_file->fp_phreg);
	free(reg_file->fp_free_phreg);
	free(reg_file->xmm_phreg);
	free(reg_file->xmm_free_phreg);
	free(reg_file);
}


void x86_reg_file_dump(int core, int thread, FILE *f)
{
	int i;

	/* Integer register file */
	fprintf(f, "Integer register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < x86_reg_file_int_local_size; i++)
	{
		fprintf(f, "  %3d%c[%d-%d]", i, X86_THREAD.reg_file->int_phreg[i].busy ? ' ' : '*',
			X86_THREAD.reg_file->int_phreg[i].busy,
			X86_THREAD.reg_file->int_phreg[i].pending);
		if (i % 5 == 4 && i != x86_reg_file_int_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nInteger Register Aliasing Table:\n");
	for (i = x86_dep_int_first; i <= x86_dep_int_last; i++)
	{
		fprintf(f, "  %2d->%-3d", i, X86_THREAD.reg_file->int_rat[i - x86_dep_int_first]);
		if ((i - x86_dep_int_first) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "int_free_phreg_count  %d  # Number of free integer registers\n",
		X86_THREAD.reg_file->int_free_phreg_count);
	fprintf(f, "\n");

	/* Floating point register file */
	fprintf(f, "Floating-point register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < x86_reg_file_fp_local_size; i++)
	{
		fprintf(f, "  %3d%c[%d-%d]", i, X86_THREAD.reg_file->fp_phreg[i].busy ? ' ' : '*',
			X86_THREAD.reg_file->fp_phreg[i].busy,
			X86_THREAD.reg_file->fp_phreg[i].pending);
		if (i % 5 == 4 && i != x86_reg_file_fp_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nIteger Register Aliasing Table:\n");
	for (i = x86_dep_fp_first; i <= x86_dep_fp_last; i++)
	{
		fprintf(f, "  %2d->%-3d", i, X86_THREAD.reg_file->fp_rat[i - x86_dep_fp_first]);
		if ((i - x86_dep_fp_first) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "fp_free_phreg_count  %d  # Number of free floating-point registers\n",
		X86_THREAD.reg_file->fp_free_phreg_count);
	fprintf(f, "\n");
	
	/* XMM register file */
	fprintf(f, "XMM register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < x86_reg_file_xmm_local_size; i++)
	{
		fprintf(f, "  %3d%c[%d-%d]", i, X86_THREAD.reg_file->xmm_phreg[i].busy ? ' ' : '*',
			X86_THREAD.reg_file->xmm_phreg[i].busy,
			X86_THREAD.reg_file->xmm_phreg[i].pending);
		if (i % 5 == 4 && i != x86_reg_file_xmm_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nXMM Register Aliasing Table:\n");
	for (i = x86_dep_xmm_first; i <= x86_dep_xmm_last; i++)
	{
		fprintf(f, "  %2d->%-3d", i, X86_THREAD.reg_file->xmm_rat[i - x86_dep_xmm_first]);
		if ((i - x86_dep_xmm_first) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "xmm_free_phreg_count  %d  # Number of free integer registers\n",
		X86_THREAD.reg_file->xmm_free_phreg_count);
	fprintf(f, "\n");
}


/* Set the number of logical/physical registers needed by an instruction.
 * If there are only flags as destination dependences, only one register is
 * needed. Otherwise, one register per destination operand is needed, and the
 * output flags will be mapped to one of the destination physical registers
 * used for operands. */
void x86_reg_file_count_deps(struct x86_uop_t *uop)
{
	int dep;
	int loreg;

	int int_count;
	int fp_count;
	int flag_count;
	int xmm_count;

	/* Initialize */
	uop->idep_count = 0;
	uop->odep_count = 0;
	uop->ph_int_idep_count = 0;
	uop->ph_fp_idep_count = 0;
	uop->ph_xmm_idep_count = 0;
	uop->ph_int_odep_count = 0;
	uop->ph_fp_odep_count = 0;
	uop->ph_xmm_odep_count = 0;

	/* Output dependences */
	int_count = fp_count = flag_count = xmm_count = 0;
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
	{
		loreg = uop->uinst->odep[dep];
		if (X86_DEP_IS_FLAG(loreg))
			flag_count++;
		else if (X86_DEP_IS_INT_REG(loreg))
			int_count++;
		else if (X86_DEP_IS_FP_REG(loreg))
			fp_count++;
		else if (X86_DEP_IS_XMM_REG(loreg))
			xmm_count++;
	}
	uop->odep_count = flag_count + int_count + fp_count + xmm_count;
	uop->ph_int_odep_count = flag_count && !int_count ? 1 : int_count;
	uop->ph_fp_odep_count = fp_count;
	uop->ph_xmm_odep_count = xmm_count;

	/* Input dependences */
	int_count = fp_count = flag_count = xmm_count = 0;
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++)
	{
		loreg = uop->uinst->idep[dep];
		if (X86_DEP_IS_FLAG(loreg))
			flag_count++;
		else if (X86_DEP_IS_INT_REG(loreg))
			int_count++;
		else if (X86_DEP_IS_FP_REG(loreg))
			fp_count++;
		else if (X86_DEP_IS_XMM_REG(loreg))
			xmm_count++;
	}
	uop->idep_count = flag_count + int_count + fp_count + xmm_count;
	uop->ph_int_idep_count = flag_count + int_count;
	uop->ph_fp_idep_count = fp_count;
	uop->ph_xmm_idep_count = xmm_count;
}



int x86_reg_file_can_rename(struct x86_uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;

	/* Detect negative cases. */
	if (x86_reg_file_kind == x86_reg_file_kind_private)
	{
		if (X86_THREAD.reg_file_int_count + uop->ph_int_odep_count > x86_reg_file_int_local_size)
			return 0;
		if (X86_THREAD.reg_file_fp_count + uop->ph_fp_odep_count > x86_reg_file_fp_local_size)
			return 0;
		if (X86_THREAD.reg_file_xmm_count + uop->ph_xmm_odep_count > x86_reg_file_xmm_local_size)
			return 0;
	}
	else
	{
		if (X86_CORE.reg_file_int_count + uop->ph_int_odep_count > x86_reg_file_int_local_size)
			return 0;
		if (X86_CORE.reg_file_fp_count + uop->ph_fp_odep_count > x86_reg_file_fp_local_size)
			return 0;
		if (X86_CORE.reg_file_xmm_count + uop->ph_xmm_odep_count > x86_reg_file_xmm_local_size)
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
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

	/* Update floating-point top of stack */
	if (uop->uinst->opcode == x86_uinst_fp_pop)
	{
		/* Pop floating-point stack */
		reg_file->fp_top_of_stack = (reg_file->fp_top_of_stack + 1) % 8;
	}
	else if (uop->uinst->opcode == x86_uinst_fp_push)
	{
		/* Push floating-point stack */
		reg_file->fp_top_of_stack = (reg_file->fp_top_of_stack + 7) % 8;
	}

	/* Rename input int/FP/XMM registers */
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++)
	{
		loreg = uop->uinst->idep[dep];
		if (X86_DEP_IS_INT_REG(loreg))
		{
			phreg = reg_file->int_rat[loreg - x86_dep_int_first];
			uop->ph_idep[dep] = phreg;
			X86_THREAD.rat_int_reads++;
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + reg_file->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));

			/* Rename it. */
			phreg = reg_file->fp_rat[streg - x86_dep_fp_first];
			uop->ph_idep[dep] = phreg;
			X86_THREAD.rat_fp_reads++;
		}
		else if (X86_DEP_IS_XMM_REG(loreg))
		{
			phreg = reg_file->xmm_rat[loreg - x86_dep_xmm_first];
			uop->ph_idep[dep] = phreg;
			X86_THREAD.rat_xmm_reads++;
		}
		else
		{
			uop->ph_idep[dep] = -1;
		}
	}

	/* Rename output int/FP/XMM registers (not flags) */
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
			phreg = x86_reg_file_int_reclaim(core, thread);
			reg_file->int_phreg[phreg].busy++;
			reg_file->int_phreg[phreg].pending = 1;
			ophreg = reg_file->int_rat[loreg - x86_dep_int_first];
			if (flag_phreg < 0)
				flag_phreg = phreg;

			/* Allocate it */
			uop->ph_odep[dep] = phreg;
			uop->ph_oodep[dep] = ophreg;
			reg_file->int_rat[loreg - x86_dep_int_first] = phreg;
			X86_THREAD.rat_int_writes++;
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + reg_file->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));
			
			/* Reclaim a free FP register */
			phreg = x86_reg_file_fp_reclaim(core, thread);
			reg_file->fp_phreg[phreg].busy++;
			reg_file->fp_phreg[phreg].pending = 1;
			ophreg = reg_file->fp_rat[streg - x86_dep_fp_first];
			
			/* Allocate it */
			uop->ph_odep[dep] = phreg;
			uop->ph_oodep[dep] = ophreg;
			reg_file->fp_rat[streg - x86_dep_fp_first] = phreg;
			X86_THREAD.rat_fp_writes++;
		}
		else if (X86_DEP_IS_XMM_REG(loreg))
		{
			/* Reclaim a free xmm register */
			phreg = x86_reg_file_xmm_reclaim(core, thread);
			reg_file->xmm_phreg[phreg].busy++;
			reg_file->xmm_phreg[phreg].pending = 1;
			ophreg = reg_file->xmm_rat[loreg - x86_dep_xmm_first];

			/* Allocate it */
			uop->ph_odep[dep] = phreg;
			uop->ph_oodep[dep] = ophreg;
			reg_file->xmm_rat[loreg - x86_dep_xmm_first] = phreg;
			X86_THREAD.rat_xmm_writes++;
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
			flag_phreg = x86_reg_file_int_reclaim(core, thread);
		for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
		{
			loreg = uop->uinst->odep[dep];
			if (!X86_DEP_IS_FLAG(loreg))
				continue;
			reg_file->int_phreg[flag_phreg].busy++;
			reg_file->int_phreg[flag_phreg].pending = 1;
			ophreg = reg_file->int_rat[loreg - x86_dep_int_first];
			uop->ph_oodep[dep] = ophreg;
			uop->ph_odep[dep] = flag_phreg;
			reg_file->int_rat[loreg - x86_dep_int_first] = flag_phreg;
		}
	}
}


/* Return 1 if input dependencies are resolved */
int x86_reg_file_ready(struct x86_uop_t *uop)
{
	int loreg;
	int phreg;
	int dep;

	int core = uop->core;
	int thread = uop->thread;

	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;
	
	for (dep = 0; dep < X86_UINST_MAX_IDEPS; dep++)
	{
		loreg = uop->uinst->idep[dep];
		phreg = uop->ph_idep[dep];
		if (X86_DEP_IS_INT_REG(loreg) && reg_file->int_phreg[phreg].pending)
			return 0;
		if (X86_DEP_IS_FP_REG(loreg) && reg_file->fp_phreg[phreg].pending)
			return 0;
		if (X86_DEP_IS_XMM_REG(loreg) && reg_file->xmm_phreg[phreg].pending)
			return 0;
	}
	return 1;
}


void x86_reg_file_write(struct x86_uop_t *uop)
{
	int dep;
	int loreg;
	int phreg;

	int core = uop->core;
	int thread = uop->thread;

	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;
	
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
	{
		loreg = uop->uinst->odep[dep];
		phreg = uop->ph_odep[dep];
		if (X86_DEP_IS_INT_REG(loreg))
			reg_file->int_phreg[phreg].pending = 0;
		else if (X86_DEP_IS_FP_REG(loreg))
			reg_file->fp_phreg[phreg].pending = 0;
		else if (X86_DEP_IS_XMM_REG(loreg))
			reg_file->xmm_phreg[phreg].pending = 0;
	}
}


void x86_reg_file_undo(struct x86_uop_t *uop)
{
	int dep;
	int loreg;
	int streg;
	int phreg;
	int ophreg;

	int core = uop->core;
	int thread = uop->thread;

	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

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
			assert(reg_file->int_phreg[phreg].busy > 0);
			assert(!reg_file->int_phreg[phreg].pending);
			reg_file->int_phreg[phreg].busy--;
			if (!reg_file->int_phreg[phreg].busy)
			{
				assert(reg_file->int_free_phreg_count < x86_reg_file_int_local_size);
				assert(X86_CORE.reg_file_int_count > 0 && X86_THREAD.reg_file_int_count > 0);
				reg_file->int_free_phreg[reg_file->int_free_phreg_count] = phreg;
				reg_file->int_free_phreg_count++;
				X86_CORE.reg_file_int_count--;
				X86_THREAD.reg_file_int_count--;
			}

			/* Return to previous mapping */
			reg_file->int_rat[loreg - x86_dep_int_first] = ophreg;
			assert(reg_file->int_phreg[ophreg].busy);
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Convert to top-of-stack relative */
			streg = (loreg - x86_dep_fp_first + reg_file->fp_top_of_stack) % 8 + x86_dep_fp_first;
			assert(X86_DEP_IS_FP_REG(streg));
			
			/* Decrease busy counter and free if 0. */
			assert(reg_file->fp_phreg[phreg].busy > 0);
			assert(!reg_file->fp_phreg[phreg].pending);
			reg_file->fp_phreg[phreg].busy--;
			if (!reg_file->fp_phreg[phreg].busy)
			{
				assert(reg_file->fp_free_phreg_count < x86_reg_file_fp_local_size);
				assert(X86_CORE.reg_file_fp_count > 0 && X86_THREAD.reg_file_fp_count > 0);
				reg_file->fp_free_phreg[reg_file->fp_free_phreg_count] = phreg;
				reg_file->fp_free_phreg_count++;
				X86_CORE.reg_file_fp_count--;
				X86_THREAD.reg_file_fp_count--;
			}

			/* Return to previous mapping */
			reg_file->fp_rat[streg - x86_dep_fp_first] = ophreg;
			assert(reg_file->fp_phreg[ophreg].busy);
		}
		else if (X86_DEP_IS_XMM_REG(loreg))
		{
			/* Decrease busy counter and free if 0. */
			assert(reg_file->xmm_phreg[phreg].busy > 0);
			assert(!reg_file->xmm_phreg[phreg].pending);
			reg_file->xmm_phreg[phreg].busy--;
			if (!reg_file->xmm_phreg[phreg].busy)
			{
				assert(reg_file->xmm_free_phreg_count < x86_reg_file_xmm_local_size);
				assert(X86_CORE.reg_file_xmm_count > 0 && X86_THREAD.reg_file_xmm_count > 0);
				reg_file->xmm_free_phreg[reg_file->xmm_free_phreg_count] = phreg;
				reg_file->xmm_free_phreg_count++;
				X86_CORE.reg_file_xmm_count--;
				X86_THREAD.reg_file_xmm_count--;
			}

			/* Return to previous mapping */
			reg_file->xmm_rat[loreg - x86_dep_xmm_first] = ophreg;
			assert(reg_file->xmm_phreg[ophreg].busy);
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
		reg_file->fp_top_of_stack = (reg_file->fp_top_of_stack + 7) % 8;
	}
	else if (uop->uinst->opcode == x86_uinst_fp_push)
	{
		/* Inverse-push floating-point stack */
		reg_file->fp_top_of_stack = (reg_file->fp_top_of_stack + 1) % 8;
	}
}


void x86_reg_file_commit(struct x86_uop_t *uop)
{
	int dep, loreg, phreg, ophreg;
	int core = uop->core;
	int thread = uop->thread;
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;

	assert(!uop->specmode);
	for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
	{
		loreg = uop->uinst->odep[dep];
		phreg = uop->ph_odep[dep];
		ophreg = uop->ph_oodep[dep];

		if (X86_DEP_IS_INT_REG(loreg))
		{
			/* Decrease counter of previous mapping and free if 0. */
			assert(reg_file->int_phreg[ophreg].busy > 0);
			reg_file->int_phreg[ophreg].busy--;
			if (!reg_file->int_phreg[ophreg].busy)
			{
				assert(!reg_file->int_phreg[ophreg].pending);
				assert(reg_file->int_free_phreg_count < x86_reg_file_int_local_size);
				assert(X86_CORE.reg_file_int_count > 0 && X86_THREAD.reg_file_int_count > 0);
				reg_file->int_free_phreg[reg_file->int_free_phreg_count] = ophreg;
				reg_file->int_free_phreg_count++;
				X86_CORE.reg_file_int_count--;
				X86_THREAD.reg_file_int_count--;
			}
		}
		else if (X86_DEP_IS_FP_REG(loreg))
		{
			/* Decrease counter of previous mapping and free if 0. */
			assert(reg_file->fp_phreg[ophreg].busy > 0);
			reg_file->fp_phreg[ophreg].busy--;
			if (!reg_file->fp_phreg[ophreg].busy)
			{
				assert(!reg_file->fp_phreg[ophreg].pending);
				assert(reg_file->fp_free_phreg_count < x86_reg_file_fp_local_size);
				assert(X86_CORE.reg_file_fp_count > 0 && X86_THREAD.reg_file_fp_count > 0);
				reg_file->fp_free_phreg[reg_file->fp_free_phreg_count] = ophreg;
				reg_file->fp_free_phreg_count++;
				X86_CORE.reg_file_fp_count--;
				X86_THREAD.reg_file_fp_count--;
			}
		}
		else if (X86_DEP_IS_XMM_REG(loreg))
		{
			/* Decrease counter of previous mapping and free if 0. */
			assert(reg_file->xmm_phreg[ophreg].busy > 0);
			reg_file->xmm_phreg[ophreg].busy--;
			if (!reg_file->xmm_phreg[ophreg].busy)
			{
				assert(!reg_file->xmm_phreg[ophreg].pending);
				assert(reg_file->xmm_free_phreg_count < x86_reg_file_xmm_local_size);
				assert(X86_CORE.reg_file_xmm_count > 0 && X86_THREAD.reg_file_xmm_count > 0);
				reg_file->xmm_free_phreg[reg_file->xmm_free_phreg_count] = ophreg;
				reg_file->xmm_free_phreg_count++;
				X86_CORE.reg_file_xmm_count--;
				X86_THREAD.reg_file_xmm_count--;
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
	struct x86_reg_file_t *reg_file = X86_THREAD.reg_file;
	struct x86_uop_t *uop;

	int loreg;
	int phreg;
	int ophreg;
	int dep;
	int i;

	/* Check that all registers in the free list are actually free. */
	for (i = 0; i < reg_file->int_free_phreg_count; i++)
	{
		phreg = reg_file->int_free_phreg[i];
		assert(!reg_file->int_phreg[phreg].busy);
		assert(!reg_file->int_phreg[phreg].pending);
	}
	for (i = 0; i < reg_file->fp_free_phreg_count; i++)
	{
		phreg = reg_file->fp_free_phreg[i];
		assert(!reg_file->fp_phreg[phreg].busy);
		assert(!reg_file->fp_phreg[phreg].pending);
	}
	for (i = 0; i < reg_file->xmm_free_phreg_count; i++)
	{
		phreg = reg_file->xmm_free_phreg[i];
		assert(!reg_file->xmm_phreg[phreg].busy);
		assert(!reg_file->xmm_phreg[phreg].pending);
	}

	/* Check that all mapped registers are busy */
	for (loreg = x86_dep_int_first; loreg <= x86_dep_int_last; loreg++)
	{
		phreg = reg_file->int_rat[loreg - x86_dep_int_first];
		assert(reg_file->int_phreg[phreg].busy);
	}
	for (loreg = x86_dep_fp_first; loreg <= x86_dep_fp_last; loreg++)
	{
		phreg = reg_file->fp_rat[loreg - x86_dep_fp_first];
		assert(reg_file->fp_phreg[phreg].busy);
	}
	for (loreg = x86_dep_xmm_first; loreg <= x86_dep_xmm_last; loreg++)
	{
		phreg = reg_file->xmm_rat[loreg - x86_dep_xmm_first];
		assert(reg_file->xmm_phreg[phreg].busy);
	}

	/* Check that all destination and previous destination
	 * registers of instructions in the rob are busy */
	for (i = 0; i < X86_THREAD.rob_count; i++)
	{
		uop = x86_rob_get(core, thread, i);
		assert(uop);
		for (dep = 0; dep < X86_UINST_MAX_ODEPS; dep++)
		{
			loreg = uop->uinst->odep[dep];
			phreg = uop->ph_odep[dep];
			ophreg = uop->ph_oodep[dep];
			if (X86_DEP_IS_INT_REG(loreg))
			{
				assert(reg_file->int_phreg[phreg].busy);
				assert(reg_file->int_phreg[ophreg].busy);
			}
			else if (X86_DEP_IS_FP_REG(loreg))
			{
				assert(reg_file->fp_phreg[phreg].busy);
				assert(reg_file->fp_phreg[ophreg].busy);
			}
			else if (X86_DEP_IS_XMM_REG(loreg))
			{
				assert(reg_file->xmm_phreg[phreg].busy);
				assert(reg_file->xmm_phreg[ophreg].busy);
			}
			else
			{
				assert(phreg == -1);
				assert(ophreg == -1);
			}
		}
	}
}
