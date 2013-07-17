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

#include <lib/util/string.h>


/* Forward declarations */
struct cuda_function_t;


enum frm_grid_status_t
{
        frm_grid_pending             = 0x0001,
        frm_grid_running             = 0x0002,
        frm_grid_finished            = 0x0004
};

struct frm_grid_t
{
	/* ID */
	int id;
	char name[MAX_STRING_SIZE];

        /* Status */
        enum frm_grid_status_t status;

	/* CUDA function associated */
	struct cuda_function_t *function;

	/* Number of register used by each thread. */
	unsigned int num_gpr_used;

	/* Call-back function run right before freeing ND-Range, using the value in
	 * 'free_notify_data' as an argument. */
	void (*free_notify_func)(void *);
	void *free_notify_data;

	/* 3D work size counters */
	int grid_size3[3];  /* Total number of threads */
	int block_size3[3];  /* Number of threads in a thread block */
	int block_count3[3];  /* Number of thread blocks */

	/* 1D work size counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int grid_size;
	int block_size;
	int block_count;

	/* Array of thread blocks */
	int thread_block_count;
	struct frm_thread_block_t **thread_blocks;

	/* Lists of thread blocks */
	struct list_t *pending_thread_blocks;
	struct list_t *running_thread_blocks;
	struct list_t *finished_thread_blocks;

        /* List of Grid */
        struct frm_grid_t *grid_list_prev;
        struct frm_grid_t *grid_list_next;
        struct frm_grid_t *pending_grid_list_prev;
        struct frm_grid_t *pending_grid_list_next;
        struct frm_grid_t *running_grid_list_prev;
        struct frm_grid_t *running_grid_list_next;
        struct frm_grid_t *finished_grid_list_prev;
        struct frm_grid_t *finished_grid_list_next;

	void *inst_buffer;
	unsigned int inst_buffer_size;

	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in 
	 * kernel function. */
	unsigned int local_mem_top;

};

struct frm_grid_t *frm_grid_create(struct cuda_function_t *function);
void frm_grid_free(struct frm_grid_t *grid);
void frm_grid_dump(struct frm_grid_t *grid, FILE *f);
void frm_grid_setup_threads(struct frm_grid_t *grid);
void frm_grid_setup_const_mem(struct frm_grid_t *grid);
void frm_grid_setup_args(struct frm_grid_t *grid);
void frm_grid_run(struct frm_grid_t *grid);

void frm_grid_setup_size(struct frm_grid_t *grid,
		unsigned int *global_size,
		unsigned int *local_size);

#endif

