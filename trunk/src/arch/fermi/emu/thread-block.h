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


enum frm_thread_block_status_t
{
	frm_thread_block_pending		= 0x0001,
	frm_thread_block_running		= 0x0002,
	frm_thread_block_finished	= 0x0004
};

struct frm_thread_block_t
{
	/* ID */
	int id;
	char name[1024];
	int id_3d[3];

	/* Status */
	enum frm_thread_block_status_t status;

	/* Grid it belongs to */
	struct frm_grid_t *grid;
	struct frm_sm_t *sm;

	/* IDs of threads contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* Pointers to warps and threads */
	struct frm_thread_t **threads;  /* Pointer to first thread in 'function->threads' */
	struct frm_warp_t **warps;  /* Pointer to first warp in 'function->warps' */
	struct frm_warp_pool_t *warp_pool;

	/* Double linked lists of thread_blocks */
	struct frm_thread_block_t *pending_list_prev;
	struct frm_thread_block_t *pending_list_next;
	struct frm_thread_block_t *running_list_prev;
	struct frm_thread_block_t *running_list_next;
	struct frm_thread_block_t *finished_list_prev;
	struct frm_thread_block_t *finished_list_next;

	/* List of running warps */
	struct frm_warp_t *running_list_head;
	struct frm_warp_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of warps in barrier */
	struct frm_warp_t *barrier_list_head;
	struct frm_warp_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished warps */
	struct frm_warp_t *finished_list_head;
	struct frm_warp_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_sm;
	int sm_finished_count;  /* like 'finished_list_count', but when warp reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define FRM_FOR_EACH_THREADBLOCK_IN_GRID(GRID, THREADBLOCK_ID) \
	for ((THREADBLOCK_ID) = (GRID)->thread_block_id_first; \
		(THREADBLOCK_ID) <= (GRID)->thread_block_id_last; \
		(THREADBLOCK_ID)++)

struct frm_thread_block_t *frm_thread_block_create(void);
void frm_thread_block_free(struct frm_thread_block_t *thread_block);
void frm_thread_block_dump(struct frm_thread_block_t *thread_block, FILE *f);
int frm_thread_block_get_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status);
void frm_thread_block_set_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status);
void frm_thread_block_clear_status(struct frm_thread_block_t *thread_block, enum frm_thread_block_status_t status);


#endif

