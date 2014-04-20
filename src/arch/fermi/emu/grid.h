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

#ifndef FERMI_EMU_GRID_H
#define FERMI_EMU_GRID_H

#include <lib/class/class.h>


/*
 * Class 'FrmGrid'
 */

CLASS_BEGIN(FrmGrid, Object)

	/* Emulator */
	FrmEmu *emu;

	/* ID */
	int id;

	/* CUDA function associated */
	struct cuda_function_t *function;

	/* Number of general purpose registers used by a thread */
	unsigned num_gpr;

	/* Lists of thread-blocks */
	struct list_t *pending_thread_blocks;
	struct list_t *running_thread_blocks;
	struct list_t *finished_thread_blocks;

	/* Sizes */
	int thread_count3[3];
	int thread_count;
	int thread_block_size3[3];
	int thread_block_size;
	int thread_block_count3[3];
	int thread_block_count;

	/* Call-back function run right before freeing grid, using the value in
	 * 'free_notify_data' as an argument. */
	void (*free_notify_func)(void *);
	void *free_notify_data;

	unsigned shared_mem_top;

	/* The following fields are used by architectural simulation only. */
	int max_thread_blocks_per_sm;

CLASS_END(FrmGrid)


void FrmGridCreate(FrmGrid *self, FrmEmu *emu,
		struct cuda_function_t *function);
void FrmGridDestroy(FrmGrid *self);
void FrmGridDump(FrmGrid *self, FILE *f);

void FrmGridSetupSize(FrmGrid *self, unsigned *global_size,
		unsigned *local_size);
void FrmGridSetupConstantMemory(FrmGrid *self);

#endif
