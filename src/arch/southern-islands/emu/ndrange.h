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

#include "emu.h"

enum si_ndrange_status_t
{
	si_ndrange_pending		= 0x0001,
	si_ndrange_running		= 0x0002,
	si_ndrange_finished		= 0x0004
};

enum si_emu_table_entry_kind_t
{
        SI_TABLE_ENTRY_KIND_BUFFER_DESC = 1,
        SI_TABLE_ENTRY_KIND_IMAGE_DESC,
        SI_TABLE_ENTRY_KIND_SAMPLER_DESC
};

struct si_emu_table_entry_t
{
        unsigned int valid : 1;
        enum si_emu_table_entry_kind_t kind;
        unsigned int size;
};

struct si_ndrange_t
{
	/* ID */
	char *name;
	int id;  /* Sequential ND-Range ID (given by si_emu->ndrange_count counter) */

	/* Status */
	enum si_ndrange_status_t status;

	/* Event */
	struct si_opencl_event_t *event;

	/* Call-back function run right before freeing ND-Range, using the value in
	 * 'free_notify_data' as an argument. */
	void (*free_notify_func)(void *);
	void *free_notify_data;

	/* Command queue and command queue task associated */
	struct si_opencl_command_queue_t *command_queue;
	struct si_opencl_command_t *command;

	/* Number of work dimensions */
	int work_dim;

	/* 3D work size counters */
	int global_size3[3];  /* Total number of work_items */
	int local_size3[3];  /* Number of work_items in a group */
	int group_count3[3];  /* Number of work_item groups */

	/* 1D work size counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int global_size;
	int local_size;
	int group_count;

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
	int wavefronts_per_work_group; /* ceil(local_size/si_emu_wavefront_size) */

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

	/* Instruction memory containing Southern Islands ISA */
	void *inst_buffer;
	unsigned int inst_buffer_size;

	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in 
	 * kernel function. */
	unsigned int local_mem_top;

	/* Number of register used by each work-item. This fields determines
	 * how many work-groups can be allocated per compute unit, among
	 * others. */
	unsigned int num_vgpr_used;
	unsigned int num_sgpr_used;

        /* Internal tables that reside in global memory */
        unsigned int const_buf_table;
        struct si_emu_table_entry_t
                const_buf_table_entries[SI_EMU_MAX_NUM_CONST_BUFS];
        unsigned int resource_table;
        struct si_emu_table_entry_t
                resource_table_entries[SI_EMU_MAX_NUM_RESOURCES];
        unsigned int uav_table;
        struct si_emu_table_entry_t
                uav_table_entries[SI_EMU_MAX_NUM_UAVS];

	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel 
	 * report option is active. */
	unsigned int *inst_histogram;
};


struct si_ndrange_t *si_ndrange_create(char *name);
void si_ndrange_free(struct si_ndrange_t *ndrange);
void si_ndrange_dump(struct si_ndrange_t *ndrange, FILE *f);

/* Set up call-back function to be run right before the ND-Range is freed.
 * Useful for host/device synchronization. */
void si_ndrange_set_free_notify_func(struct si_ndrange_t *ndrange,
		void (*func)(void *), void *user_data);

/* Functions to set up ND-Range after initialization */
void si_ndrange_setup_size(struct si_ndrange_t *ndrange,
		unsigned int *global_size,
		unsigned int *local_size,
		int work_dim);
void si_ndrange_setup_inst_mem(struct si_ndrange_t *ndrange,
		void *buf, int size, unsigned int pc);

int si_ndrange_get_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status);
void si_ndrange_set_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);
void si_ndrange_clear_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);

/* Access constant buffers */
void si_ndrange_const_buf_write(struct si_ndrange_t *ndrange, 
	int const_buf_num, int offset, void *pvalue, unsigned int size);
void si_ndrange_const_buf_read(struct si_ndrange_t *ndrange, int const_buf_num, 	int offset, void *pvalue, unsigned int size);

/* Access internal tables */
void si_ndrange_insert_buffer_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int uav);
void si_ndrange_insert_buffer_into_const_buf_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int const_buf_num);
void si_ndrange_insert_image_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_image_desc_t *image_desc, unsigned int uav);

#endif
