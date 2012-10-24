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

	/* CUDA function associated */
	struct frm_cuda_function_t *function;

	/* Pointers to threadblocks, warps, and threads */
	struct frm_threadblock_t **threadblocks;
	struct frm_warp_t **warps;
	struct frm_thread_t **threads;

	/* IDs of threadblocks contained */
	int threadblock_id_first;
	int threadblock_id_last;
	int threadblock_count;
	
	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* IDs of threads contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* Size of threadblocks */
	int warps_per_threadblock;  /* = ceil(local_size / frm_emu_warp_size) */

        /* List of Grid */
        struct frm_grid_t *grid_list_prev;
        struct frm_grid_t *grid_list_next;
        struct frm_grid_t *pending_grid_list_prev;
        struct frm_grid_t *pending_grid_list_next;
        struct frm_grid_t *running_grid_list_prev;
        struct frm_grid_t *running_grid_list_next;
        struct frm_grid_t *finished_grid_list_prev;
        struct frm_grid_t *finished_grid_list_next;

	/* List of pending threadblocks */
	struct frm_threadblock_t *pending_list_head;
	struct frm_threadblock_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running threadblocks */
	struct frm_threadblock_t *running_list_head;
	struct frm_threadblock_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished threadblocks */
	struct frm_threadblock_t *finished_list_head;
	struct frm_threadblock_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
};

struct frm_grid_t *frm_grid_create(struct frm_cuda_function_t *function);
void frm_grid_free(struct frm_grid_t *grid);
int frm_grid_get_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_set_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_clear_status(struct frm_grid_t *grid, enum frm_grid_status_t status);
void frm_grid_dump(struct frm_grid_t *grid, FILE *f);
void frm_grid_setup_threads(struct frm_grid_t *grid);
void frm_grid_setup_const_mem(struct frm_grid_t *grid);
void frm_grid_setup_args(struct frm_grid_t *grid);
void frm_grid_run(struct frm_grid_t *grid);


#endif

