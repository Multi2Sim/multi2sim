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


/* Global variables */

uint32_t rf_size = 80;  /* per-thread register file size */
enum rf_kind_enum rf_kind = rf_kind_private;




/* Private variables and functions */

static uint32_t phregs_local_size;  /* maximum number of registers allowed per thread */


/* Return a free physical register, and remove it from the free list. */
static int phregs_reclaim(int core, int thread)
{
	int phreg;
	struct phregs_t *phregs = THREAD.phregs;

	/* Obtain a register from the free_phreg list */
	assert(phregs->free_phreg_count > 0);
	phreg = phregs->free_phreg[phregs->free_phreg_count - 1];
	phregs->free_phreg_count--;
	CORE.rf_count++;
	THREAD.rf_count++;
	assert(!phregs->phreg[phreg].busy);
	assert(!phregs->phreg[phreg].pending);
	return phreg;
}





/* Public functions */

void phregs_reg_options(void)
{
	static char *rf_kind_map[] = { "shared", "private" };
	opt_reg_enum("-rf_kind", "physical register file {shared|private}",
		(int *) &rf_kind, rf_kind_map, 2);
	opt_reg_uint32("-rf_size", "physical register file size per thread",
		&rf_size);
}


static void phregs_init_thread(int core, int thread)
{
	int dep, phreg, fphreg;
	struct phregs_t *phregs = THREAD.phregs;
	
	/* Initial mapping. Map each logical register to a new physical register,
	 * and map all flags to the first allocated physical register. */
	fphreg = -1;
	for (dep = 0; dep < DCOUNT; dep++) {
		if (DFLAG(dep + DFIRST)) {
			assert(fphreg >= 0);
			phreg = fphreg;
		} else {
			phreg = phregs_reclaim(core, thread);
			fphreg = phreg;
		}
		phregs->phreg[phreg].busy++;
		phregs->rat[dep] = phreg;
	}
}


#define MINREGS  (DCOUNT + ODEP_COUNT)
void phregs_init(void)
{
	int core, thread;
	
	if (rf_size < MINREGS)
		fatal("rf_size must be at least %d", MINREGS);
	phregs_local_size = rf_kind == rf_kind_private ? rf_size :
		rf_size * p_threads;
	FOREACH_CORE FOREACH_THREAD {
		THREAD.phregs = phregs_create(phregs_local_size);
		phregs_init_thread(core, thread);
	}
}


void phregs_done(void)
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		phregs_free(THREAD.phregs);
}


struct phregs_t *phregs_create(int size)
{
	struct phregs_t *phregs;
	int phreg;
	
	/* Create structure */
	phregs = calloc(1, sizeof(struct phregs_t));
	phregs->size = size;
	
	/* Register file and free list */
	phregs->phreg = calloc(size, sizeof(struct phreg_t));
	phregs->free_phreg = calloc(size, sizeof(int));
	phregs->free_phreg_count = size;
	for (phreg = 0; phreg < size; phreg++)
		phregs->free_phreg[phreg] = phreg;
	
	/* Return */
	return phregs;
}


void phregs_free(struct phregs_t *phregs)
{
	free(phregs->free_phreg);
	free(phregs->phreg);
	free(phregs);
}


void phregs_dump(int core, int thread, FILE *f)
{
	int i;

	fprintf(f, "Register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [busy, pending], * = free\n");
	for (i = 0; i < phregs_local_size; i++) {
		fprintf(f, "  %3d%c[%d-%d]", i, THREAD.phregs->phreg[i].busy ? ' ' : '*',
			THREAD.phregs->phreg[i].busy,
			THREAD.phregs->phreg[i].pending);
		if (i % 5 == 4 && i != phregs_local_size - 1)
			fprintf(f, "\n");
	}

	fprintf(f, "\nRegister Aliasing Table:\n");
	for (i = DFIRST; i < DLAST; i++) {
		fprintf(f, "  %2d->%-3d", i, THREAD.phregs->rat[i - DFIRST]);
		if ((i - DFIRST) % 8 == 7)
			fprintf(f, "\n");
	}

	fprintf(f, "\n");
	fprintf(f, "free_phreg_count  %d  # Number of free registers\n",
		THREAD.phregs->free_phreg_count);
	fprintf(f, "\n");
}


/* Set the number of logical/physical registers needed by an instruction.
 * If there are only flags as destination dependences, only one register is
 * needed. Otherwise, one register per destination operand is needed, and the
 * output flags will be mapped to one of the destination physical registers
 * used for operands. */
void phregs_count_deps(struct uop_t *uop)
{
	int idep, odep, loreg;
	int lcount = 0, fcount = 0;

	uop->idep_count = 0;
	uop->odep_count = 0;
	uop->ph_idep_count = 0;
	uop->ph_odep_count = 0;

	/* Count output dependences */
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		loreg = uop->odep[odep];
		if (!DVALID(loreg))
			continue;
		uop->odep_count++;
		if (DFLAG(loreg))
			fcount++;
		else
			lcount++;
	}
	uop->ph_odep_count = fcount && !lcount ? 1 : lcount;

	/* Count input dependences */
	for (idep = 0; idep < IDEP_COUNT; idep++) {
		loreg = uop->idep[idep];
		if (!DVALID(loreg))
			continue;
		uop->idep_count++;
		uop->ph_idep_count++;
	}
}


int phregs_can_rename(struct uop_t *uop)
{
	int core = uop->core;
	int thread = uop->thread;
	
	return rf_kind == rf_kind_shared ?
		CORE.rf_count + uop->ph_odep_count <= phregs_local_size :
		THREAD.rf_count + uop->ph_odep_count <= phregs_local_size;
}


void phregs_rename(struct uop_t *uop)
{
	int idep, odep;
	int loreg, phreg, ophreg;
	int fcount, fphreg;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;

	/* Rename input registers */
	for (idep = 0; idep < IDEP_COUNT; idep++) {
		loreg = uop->idep[idep];
		if (!DVALID(loreg)) {
			uop->ph_idep[idep] = -1;
			continue;
		}
		
		/* Rename input register */
		phreg = phregs->rat[loreg - DFIRST];
		uop->ph_idep[idep] = phreg;
		THREAD.rat_reads++;
	}
	
	/* Rename output registers. Store in 'fphreg' an
	 * allocated output register, which will be used to
	 * rename flags below. */
	fphreg = -1;
	fcount = 0;
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		
		loreg = uop->odep[odep];
		if (DFLAG(loreg)) {
			fcount++;
			continue;
		}
		if (!DVALID(loreg)) {
			uop->ph_oodep[odep] = -1;
			uop->ph_odep[odep] = -1;
			continue;
		}

		/* Reclaim a free physical register */
		phreg = phregs_reclaim(core, thread);
		phregs->phreg[phreg].busy++;
		phregs->phreg[phreg].pending = 1;
		ophreg = phregs->rat[loreg - DFIRST];
		if (fphreg < 0)
			fphreg = phreg;
		
		/* Allocate output registers */
		uop->ph_oodep[odep] = ophreg;
		uop->ph_odep[odep] = phreg;
		phregs->rat[loreg - DFIRST] = phreg;
		THREAD.rat_writes++;
	}

	/* Rename flags. If there is no flag, done.
	 * If no destination register was renamed,
	 * allocate a new register for the flags. */
	if (!fcount)
		return;
	if (fphreg < 0)
		fphreg = phregs_reclaim(core, thread);
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		loreg = uop->odep[odep];
		if (!DFLAG(loreg))
			continue;
		phregs->phreg[fphreg].busy++;
		phregs->phreg[fphreg].pending = 1;
		ophreg = phregs->rat[loreg - DFIRST];
		uop->ph_oodep[odep] = ophreg;
		uop->ph_odep[odep] = fphreg;
		phregs->rat[loreg - DFIRST] = fphreg;
	}
	assert(phregs->phreg[fphreg].busy);
	assert(phregs->phreg[fphreg].pending);
}


/* Return 1 if input dependencies are resolved */
int phregs_ready(struct uop_t *uop)
{
	int phreg, idep;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;
	
	for (idep = 0; idep < IDEP_COUNT; idep++) {
		phreg = uop->ph_idep[idep];
		if (phreg < 0)
			continue;
		if (phregs->phreg[phreg].pending)
			return 0;
	}
	return 1;
}


void phregs_write(struct uop_t *uop)
{
	int odep, phreg;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;
	
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		phreg = uop->ph_odep[odep];
		if (phreg < 0)
			continue;
		assert(phreg < phregs->size);
		phregs->phreg[phreg].pending = 0;
	}
}


void phregs_undo(struct uop_t *uop)
{
	int odep, loreg, phreg, ophreg;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;

	/* Undo mappings in reverse order, in case an instruction has a
	 * duplicated output dependence. */
	assert(uop->specmode);
	for (odep = ODEP_COUNT - 1; odep >= 0; odep--) {
	
		loreg = uop->odep[odep];
		phreg = uop->ph_odep[odep];
		ophreg = uop->ph_oodep[odep];
		if (!DVALID(loreg)) {
			assert(phreg == -1);
			assert(ophreg == -1);
			continue;
		}

		/* Decrease busy counter in current mapping, and free if 0. */
		assert(phregs->phreg[phreg].busy > 0);
		assert(!phregs->phreg[phreg].pending);
		phregs->phreg[phreg].busy--;
		if (!phregs->phreg[phreg].busy) {
			assert(phregs->free_phreg_count < phregs->size);
			assert(CORE.rf_count > 0 && THREAD.rf_count > 0);
			phregs->free_phreg[phregs->free_phreg_count] = phreg;
			phregs->free_phreg_count++;
			CORE.rf_count--;
			THREAD.rf_count--;
		}

		/* Return to previous mapping. */
		phregs->rat[loreg - DFIRST] = ophreg;
		assert(phregs->phreg[ophreg].busy);
	}
}


void phregs_commit(struct uop_t *uop)
{
	int odep, loreg, phreg, ophreg;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;

	assert(!uop->specmode);
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		loreg = uop->odep[odep];
		phreg = uop->ph_odep[odep];
		ophreg = uop->ph_oodep[odep];
		if (!DVALID(loreg)) {
			assert(phreg == -1 && ophreg == -1);
			continue;
		}
		
		/* Decrease counter of previous mapping and free if 0. */
		assert(phregs->phreg[ophreg].busy > 0);
		phregs->phreg[ophreg].busy--;
		if (!phregs->phreg[ophreg].busy) {
			assert(!phregs->phreg[ophreg].pending);
			assert(phregs->free_phreg_count < phregs->size);
			assert(CORE.rf_count > 0 && THREAD.rf_count > 0);
			phregs->free_phreg[phregs->free_phreg_count] = ophreg;
			phregs->free_phreg_count++;
			CORE.rf_count--;
			THREAD.rf_count--;
		}
	}
}


void phregs_check(int core, int thread)
{
	struct phregs_t *phregs = THREAD.phregs;
	struct uop_t *uop;
	int loreg, phreg, odep, i;

	/* Check that all registers in the free queue are actually free. */
	for (i = 0; i < phregs->free_phreg_count; i++) {
		phreg = phregs->free_phreg[i];
		assert(!phregs->phreg[phreg].busy);
		assert(!phregs->phreg[phreg].pending);
	}

	/* Check that all mapped registers are busy */
	for (loreg = DFIRST; loreg <= DLAST; loreg++) {
		phreg = phregs->rat[loreg - DFIRST];
		assert(phregs->phreg[phreg].busy);
	}

	/* Check that all destination and previous destination
	 * registers of instructions in the rob are busy */
	for (i = 0; i < THREAD.rob_count; i++) {
		uop = rob_get(core, thread, i);
		assert(uop);
		for (odep = 0; odep < ODEP_COUNT; odep++) {
			if (!DVALID(uop->odep[odep]))
				continue;
			assert(phregs->phreg[uop->ph_odep[odep]].busy);
			assert(phregs->phreg[uop->ph_oodep[odep]].busy);
		}
	}
}

