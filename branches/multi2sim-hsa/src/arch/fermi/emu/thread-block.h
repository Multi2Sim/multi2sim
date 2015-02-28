/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_THREADBLOCK_H
#define FERMI_EMU_THREADBLOCK_H

#include <lib/class/class.h>


/*
 * Class 'FrmThreadBlock'
 */

CLASS_BEGIN(FrmThreadBlock, Object)

	/* ID */
	int id;

	/* Grid it belongs to */
	FrmGrid *grid;

	/* Warps */
	int warp_count;
	FrmWarp **warps;
	struct list_t *running_warps;
	struct list_t *finished_warps;

	/* Threads */
	int thread_count;
	FrmThread **threads;

	/* Shared memory */
	struct mem_t *shared_mem;

	/* Flags */
	unsigned finished;
	unsigned at_barrier_warp_count;

	/* Fields for architectural simulation only */

	int id_in_sm;

	int finished_sm_count;

CLASS_END(FrmThreadBlock)


void FrmThreadBlockCreate(FrmThreadBlock *self, int id, FrmGrid *grid);
void FrmThreadBlockDestroy(FrmThreadBlock *self);

void FrmThreadBlockDump(FrmThreadBlock *self, FILE *f);


#endif

