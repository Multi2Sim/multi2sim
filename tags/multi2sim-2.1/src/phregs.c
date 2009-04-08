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


uint32_t phregs_size = 64;
enum phregs_kind_enum phregs_kind = phregs_kind_private;

/* Number of register devoted for a thread. If register file is shared,
 * this is equals to phregs_size * threads */
static uint32_t phregs_local_size;



/* Private functions */


/* Return true if a physical register is free. */
static int phregs_is_free(struct phregs_t *phregs, int phreg)
{
	return !phregs->phreg[phreg].pending_readers &&
		phregs->phreg[phreg].valid_remapping &&
		phregs->phreg[phreg].completed;
}


/* This function must be called after modifying any field of the renaming
 * table in the entry 'phreg'. If suddently 'phreg' is free, it is added to
 * the free_phreg list. The physical register must not be free before. */
static void phregs_check_if_free(struct phregs_t *phregs, int phreg)
{
	if (phregs_is_free(phregs, phreg)) {
		assert(phregs->free_phreg_count < phregs->size);
		assert(phregs->phreg[phreg].busy);
		phregs->phreg[phreg].busy = 0;
		phregs->free_phreg[phregs->free_phreg_count] = phreg;
		phregs->free_phreg_count++;
	}
}


/* Reclaim a free physical register;
 * the assigned register's status is changed */
static int phregs_reclaim(int core, int thread)
{
	int phreg;
	struct phregs_t *phregs = THREAD.phregs;

	/* Obtain a register from the free_phreg list */
	if (!phregs->free_phreg_count)
		fatal("phregs_reclaim: no phyisical register free");
	phreg = phregs->free_phreg[phregs->free_phreg_count - 1];
	phregs->free_phreg_count--;
	assert(!phregs->phreg[phreg].pending_readers);
	assert(phregs->phreg[phreg].valid_remapping);
	assert(phregs->phreg[phreg].completed);
	

	/* Change renaming table entry to {0,0,0} */
	phregs->phreg[phreg].pending_readers = 0;
	phregs->phreg[phreg].valid_remapping = 0;
	phregs->phreg[phreg].completed = 0;
	phregs->phreg[phreg].thread = thread;
	phregs->phreg[phreg].busy = 1;
	
	/* Return register */
	return phreg;
}


static int phregs_needs(struct uop_t *uop)
{
	int odep, loreg, count = 0;
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		loreg = uop->odep[odep];
		if (DVALID(loreg))
			count++;
	}
	return count;
}




/* Public functions */

void phregs_reg_options(void)
{
	static char *phregs_kind_map[] = { "shared", "private" };
	opt_reg_enum("-phregs_kind", "physical register file {shared|private}",
		(int *) &phregs_kind, phregs_kind_map, 2);
	opt_reg_uint32("-phregs_size", "physical register file size per thread",
		&phregs_size);
}


static void phregs_init_thread(int core, int thread)
{
	int dep, phreg;
	struct phregs_t *phregs = THREAD.phregs;
	
	/* Initial mapping */
	for (dep = 0; dep < DCOUNT; dep++) {
		
		/* Get a free physical register and set T[phreg] to {0,0,1} */
		phreg = phregs_reclaim(core, thread);
		phregs->phreg[phreg].pending_readers = 0;
		phregs->phreg[phreg].valid_remapping = 0;
		phregs->phreg[phreg].completed = 1;
		
		/* Entries in rat */
		phregs->rat[dep] = phreg;
	}
}


void phregs_init(void)
{
	int core, thread;
	int minregs = DCOUNT + ODEP_COUNT;
	
	if (phregs_size < minregs)
		fatal("phregs_size must be at least %d", minregs);
	phregs_local_size = phregs_kind == phregs_kind_private ? phregs_size :
		phregs_size * p_threads;
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
	
	/* Create renaming table, and set all registers as free {0,1,1} */
	phregs->phreg = calloc(size, sizeof(struct phreg_t));
	for (phreg = 0; phreg < size; phreg++) {
		phregs->phreg[phreg].pending_readers = 0;
		phregs->phreg[phreg].valid_remapping = 1;
		phregs->phreg[phreg].completed = 1;
	}

	/* Create list of free registers; all are free */
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
	int is_free;

	fprintf(f, "Register file at core %d, thread %d\n", core, thread);
	fprintf(f, "Format is [pending_readers, completed, valid_remapping], * = free\n");
	for (i = 0; i < phregs_local_size; i++) {
		is_free = THREAD.phregs->phreg[i].pending_readers == 0 &&
			THREAD.phregs->phreg[i].completed &&
			THREAD.phregs->phreg[i].valid_remapping;
		fprintf(f, "  %3d%c[%02d-%d-%d]", i, is_free ? '*' : ' ',
			THREAD.phregs->phreg[i].pending_readers,
			THREAD.phregs->phreg[i].completed,
			THREAD.phregs->phreg[i].valid_remapping);
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


int phregs_can_rename(struct uop_t *uop)
{
	int needs, busy = 0;
	int core = uop->core;
	int thread = uop->thread;
	
	/* dcra */
	if (p_fetch_policy == p_fetch_policy_dcra && phregs_kind == phregs_kind_shared) {
		THREAD.dcra_active[dcra_resource_phregs] = DCRA_ACTIVE_MAX;
		busy = THREAD.phregs->size - THREAD.phregs->free_phreg_count;
		needs = phregs_needs(uop);
		return busy + needs <= THREAD.dcra_limit[dcra_resource_phregs];
	}

	/* Calculate busy registers */
	if (phregs_kind == phregs_kind_shared) {
		FOREACH_THREAD
			busy += THREAD.phregs->size - THREAD.phregs->free_phreg_count;
	} else
		busy = THREAD.phregs->size - THREAD.phregs->free_phreg_count;
	assert(busy <= phregs_local_size);
	needs = phregs_needs(uop);

	/* Return TRUE if there are enough registers */
	return busy + needs <= phregs_local_size;
}


void phregs_rename(struct uop_t *uop)
{
	int idep, odep;
	int loreg, phreg, ophreg;
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
		phregs->phreg[phreg].pending_readers++;
	}
	
	/* Rename output registers */
	for (odep = 0; odep < ODEP_COUNT; odep++) {
		
		loreg = uop->odep[odep];
		if (!DVALID(loreg)) {
			uop->ph_oodep[odep] = -1;
			uop->ph_odep[odep] = -1;
			continue;
		}
		
		/* Reclaim a free physical register */
		phreg = phregs_reclaim(core, thread);
		assert(phreg >= 0 && phreg < phregs->size);
		ophreg = phregs->rat[loreg - DFIRST];
		
		/* Allocate output registers */
		uop->ph_oodep[odep] = ophreg;
		uop->ph_odep[odep] = phreg;
		phregs->rat[loreg - DFIRST] = phreg;
	}
}


void phregs_read(struct uop_t *uop)
{
	int idep, phreg;
	int core = uop->core;
	int thread = uop->thread;
	struct phregs_t *phregs = THREAD.phregs;
	
	/* Rename input registers */
	for (idep = 0; idep < IDEP_COUNT; idep++) {
		phreg = uop->ph_idep[idep];
		if (phreg < 0)
			continue;
		
		/* Decrease number of pending readers */
		assert(phregs->phreg[phreg].pending_readers > 0);
		phregs->phreg[phreg].pending_readers--;
		phregs_check_if_free(phregs, phreg);
	}
}


/* Return true if input dependencies are resolved */
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
		if (!phregs->phreg[phreg].completed)
			return FALSE;
	}
	return TRUE;
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
		assert(!phregs->phreg[phreg].completed);
		phregs->phreg[phreg].completed = 1;
		phregs_check_if_free(phregs, phreg);
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

		/* Undo renaming. Current triplet must be {0,0,1} and
		 * the new physical register must not have been freed. */
		assert(!phregs->phreg[phreg].pending_readers);
		assert(!phregs->phreg[phreg].valid_remapping);
		assert(phregs->phreg[phreg].completed);
		assert(phregs->phreg[phreg].thread == uop->thread);
		phregs->phreg[phreg].valid_remapping = 1;
		phregs_check_if_free(phregs, phreg);
		phregs->rat[loreg - DFIRST] = ophreg;
		assert(!phregs_is_free(phregs, ophreg));
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
		
		/* Update 'valid_remapping' of previous mapping. */
		assert(!phregs->phreg[ophreg].valid_remapping);
		phregs->phreg[ophreg].valid_remapping = 1;
		phregs_check_if_free(phregs, ophreg);
	}
}


void phregs_check(int core, int thread)
{
	struct phregs_t *phregs = THREAD.phregs;
	struct uop_t *uop;
	int loreg, phreg, odep, i;

	/* Check that all mapped registers are busy */
	for (loreg = DFIRST; loreg <= DLAST; loreg++) {
		phreg = phregs->rat[loreg - DFIRST];
		assert(!phregs_is_free(phregs, phreg));
	}

	/* Check that all destination and previous destination
	 * registers of instructions in the rob are busy */
	for (i = 0; i < THREAD.rob_count; i++) {
		uop = rob_get(core, thread, i);
		assert(uop);
		for (odep = 0; odep < ODEP_COUNT; odep++) {
			if (!DVALID(uop->odep[odep]))
				continue;
			assert(!phregs_is_free(phregs, uop->ph_odep[odep]));
			assert(!phregs_is_free(phregs, uop->ph_oodep[odep]));
		}
	}
}

