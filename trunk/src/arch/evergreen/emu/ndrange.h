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

#ifndef ARCH_EVERGREEN_EMU_NDRANGE_H
#define ARCH_EVERGREEN_EMU_NDRANGE_H

#include <stdio.h>


enum evg_ndrange_status_t
{
	evg_ndrange_pending		= 0x0001,
	evg_ndrange_running		= 0x0002,
	evg_ndrange_finished		= 0x0004
};

struct evg_ndrange_t
{
	/* ID */
	char *name;
	int id;  /* Sequential ndrange ID (given by evg_emu->ndrange_count counter) */

	/* Status */
	enum evg_ndrange_status_t status;

	/* OpenCL kernel associated */
	struct evg_opencl_kernel_t *kernel;

	/* Command queue and command queue task associated */
	struct evg_opencl_command_queue_t *command_queue;
	struct evg_opencl_command_t *command;

	/* Pointers to work-groups, wavefronts, and work_items */
	struct evg_work_group_t **work_groups;
	struct evg_wavefront_t **wavefronts;
	struct evg_work_item_t **work_items;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* IDs of work-groups contained */
	int work_group_id_first;
	int work_group_id_last;
	int work_group_count;
	
	/* Size of work-groups */
	int wavefronts_per_work_group;  /* = ceil(local_size / evg_emu_wavefront_size) */

	/* List of ND-Ranges */
	struct evg_ndrange_t *ndrange_list_prev;
	struct evg_ndrange_t *ndrange_list_next;
	struct evg_ndrange_t *pending_ndrange_list_prev;
	struct evg_ndrange_t *pending_ndrange_list_next;
	struct evg_ndrange_t *running_ndrange_list_prev;
	struct evg_ndrange_t *running_ndrange_list_next;
	struct evg_ndrange_t *finished_ndrange_list_prev;
	struct evg_ndrange_t *finished_ndrange_list_next;

	/* List of pending work-groups */
	struct evg_work_group_t *pending_list_head;
	struct evg_work_group_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	struct evg_work_group_t *running_list_head;
	struct evg_work_group_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	struct evg_work_group_t *finished_list_head;
	struct evg_work_group_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	unsigned int local_mem_top;


	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel report
	 * option is active. */
	unsigned int *inst_histogram;
};

struct evg_ndrange_t *evg_ndrange_create(struct evg_opencl_kernel_t *kernel);
void evg_ndrange_free(struct evg_ndrange_t *ndrange);
void evg_ndrange_dump(struct evg_ndrange_t *ndrange, FILE *f);

int evg_ndrange_get_status(struct evg_ndrange_t *ndrange, enum evg_ndrange_status_t status);
void evg_ndrange_set_status(struct evg_ndrange_t *work_group, enum evg_ndrange_status_t status);
void evg_ndrange_clear_status(struct evg_ndrange_t *work_group, enum evg_ndrange_status_t status);

void evg_ndrange_setup_work_items(struct evg_ndrange_t *ndrange);
void evg_ndrange_setup_const_mem(struct evg_ndrange_t *ndrange);
void evg_ndrange_setup_args(struct evg_ndrange_t *ndrange);

void evg_ndrange_finish(struct evg_ndrange_t *ndrange);


#endif

