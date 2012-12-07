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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_NDRANGE_H
#define ARCH_SOUTHERN_ISLANDS_EMU_NDRANGE_H

#include <stdio.h>

enum si_ndrange_status_t
{
	si_ndrange_pending		= 0x0001,
	si_ndrange_running		= 0x0002,
	si_ndrange_finished		= 0x0004
};

struct si_ndrange_t
{
	/* ID */
	char *name;
	int id;  /* Sequential ndrange ID (given by si_emu->ndrange_count counter) */

	/* Resources */
	int num_vgprs;
	int num_sgprs;

	/* Status */
	enum si_ndrange_status_t status;

	/* Event */
	struct si_opencl_event_t* event;

	/* OpenCL kernel associated */
	struct si_opencl_kernel_t *kernel;

	/* Command queue and command queue task associated */
	struct si_opencl_command_queue_t *command_queue;
	struct si_opencl_command_t *command;

	/* Pointers to work-groups, wavefronts, and work_items */
	struct si_work_group_t **work_groups;
	struct si_wavefront_t **wavefronts;
	struct si_work_item_t **work_items;
	struct si_work_item_t **scalar_work_items;

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
	int wavefronts_per_work_group;  /* = ceil(local_size / si_emu_wavefront_size) */

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_prev;
	struct si_ndrange_t *ndrange_list_next;
	struct si_ndrange_t *pending_ndrange_list_prev;
	struct si_ndrange_t *pending_ndrange_list_next;
	struct si_ndrange_t *running_ndrange_list_prev;
	struct si_ndrange_t *running_ndrange_list_next;
	struct si_ndrange_t *finished_ndrange_list_prev;
	struct si_ndrange_t *finished_ndrange_list_next;

	/* List of pending work-groups */
	struct si_work_group_t *pending_list_head;
	struct si_work_group_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	struct si_work_group_t *running_list_head;
	struct si_work_group_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	struct si_work_group_t *finished_list_head;
	struct si_work_group_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	unsigned int local_mem_top;

	/* UAV lists */
	struct list_t *uav_list;

	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel report
	 * option is active. */
	unsigned int *inst_histogram;
};

struct si_ndrange_t *si_ndrange_create(struct si_opencl_kernel_t *kernel);
void si_ndrange_free(struct si_ndrange_t *ndrange);
void si_ndrange_dump(struct si_ndrange_t *ndrange, FILE *f);

int si_ndrange_get_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status);
void si_ndrange_set_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);
void si_ndrange_clear_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);

void si_ndrange_setup_work_items(struct si_ndrange_t *ndrange);
void si_ndrange_setup_const_mem(struct si_ndrange_t *ndrange);
void si_ndrange_init_uav_table(struct si_ndrange_t *ndrange);
void si_ndrange_setup_args(struct si_ndrange_t *ndrange);

/* Access to constant memory */
void si_isa_const_mem_write(int buffer, int offset, void *pvalue);
void si_isa_const_mem_read(int buffer, int offset, void *pvalue);

#endif
