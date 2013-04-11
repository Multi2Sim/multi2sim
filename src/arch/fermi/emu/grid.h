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
	int id;  /* Sequential grid ID (given by frm_emu->grid_count counter) */
	char name[MAX_STRING_SIZE];

        /* Status */
        enum frm_grid_status_t status;

	/* Call-back function run right before freeing ND-Range, using the value in
	 * 'free_notify_data' as an argument. */
	void (*free_notify_func)(void *);
	void *free_notify_data;

	/* CUDA function associated */
	struct cuda_function_t *function;

	/* Number of work dimensions */
	int work_dim;

	/* 3D work size counters */
	int grid_size3[3];  /* Total number of threads */
	int block_size3[3];  /* Number of threads in a thread block */
	int block_count3[3];  /* Number of thread blocks */

	/* 1D work size counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int grid_size;
	int block_size;
	int block_count;

	/* Pointers to thread_blocks, warps, and threads */
	struct frm_thread_block_t **thread_blocks;
	struct frm_warp_t **warps;
	struct frm_thread_t **threads;

	/* IDs of thread_blocks contained */
	int thread_block_id_first;
	int thread_block_id_last;
	int thread_block_count;
	
	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* IDs of threads contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* Size of thread_blocks */
	int warps_per_thread_block;  /* = ceil(local_size / frm_gpu_warp_size) */

        /* List of Grid */
        struct frm_grid_t *grid_list_prev;
        struct frm_grid_t *grid_list_next;
        struct frm_grid_t *pending_grid_list_prev;
        struct frm_grid_t *pending_grid_list_next;
        struct frm_grid_t *running_grid_list_prev;
        struct frm_grid_t *running_grid_list_next;
        struct frm_grid_t *finished_grid_list_prev;
        struct frm_grid_t *finished_grid_list_next;

	/* List of pending thread blocks */
	struct frm_thread_block_t *pending_list_head;
	struct frm_thread_block_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running thread blocks */
	struct frm_thread_block_t *running_list_head;
	struct frm_thread_block_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished thread blocks */
	struct frm_thread_block_t *finished_list_head;
	struct frm_thread_block_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	void *inst_buffer;
	unsigned int inst_buffer_size;

	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in 
	 * kernel function. */
	unsigned int local_mem_top;

	/* Number of register used by each thread. This fields determines
	 * how many thread blocks can be allocated per SM, among
	 * others. */
	unsigned int num_gpr_used;
};

struct frm_grid_t *frm_grid_create(struct cuda_function_t *function);
void frm_grid_free(struct frm_grid_t *grid);
int frm_grid_get_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_set_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_clear_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_dump(struct frm_grid_t *grid, FILE *f);
void frm_grid_setup_threads(struct frm_grid_t *grid);
void frm_grid_setup_const_mem(struct frm_grid_t *grid);
void frm_grid_setup_args(struct frm_grid_t *grid);
void frm_grid_run(struct frm_grid_t *grid);

void frm_grid_setup_size(struct frm_grid_t *grid,
		unsigned int *global_size,
		unsigned int *local_size,
		int work_dim);

#endif

