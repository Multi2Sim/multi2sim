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

#include <arch/southern-islands/asm/bin-file.h>

#include "emu.h"

enum si_ndrange_table_entry_kind_t
{
        SI_TABLE_ENTRY_KIND_BUFFER_DESC = 1,
        SI_TABLE_ENTRY_KIND_IMAGE_DESC,
        SI_TABLE_ENTRY_KIND_SAMPLER_DESC
};

struct si_ndrange_table_entry_t
{
        unsigned int valid : 1;
        enum si_ndrange_table_entry_kind_t kind;
        unsigned int size;
};

struct si_ndrange_t
{
	/* ID */
	int id;  /* Sequential ND-Range ID (given by si_emu->ndrange_count) */

	/* Number of work dimensions */
	unsigned int work_dim;

	/* 3D work size counters */
	unsigned int global_size3[3];  /* Total number of work_items */
	unsigned int local_size3[3];  /* Number of work_items in a group */
	unsigned int group_count3[3];  /* Number of work_item groups */

	/* 1D work size counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	unsigned int global_size;
	unsigned int local_size;
	unsigned int group_count;

	/* ABI data copied from the kernel */
	unsigned int userElementCount;
	struct si_bin_enc_user_element_t userElements[SI_ABI_MAX_USER_ELEMENTS];

	/* Instruction memory containing Southern Islands ISA */
	void *inst_buffer;
	unsigned int inst_buffer_size;

	/* Fetch shader memory containing Fetch shader instructions */
	int fs_buffer_initialized;
	unsigned int fs_buffer_ptr;
	unsigned int fs_buffer_size;

	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in 
	 * kernel function. */
	unsigned int local_mem_top;

	/* Number of register used by each work-item. This fields determines
	 * how many work-groups can be allocated per compute unit, among
	 * others. */
	unsigned int num_vgpr_used;
	unsigned int num_sgpr_used;
	unsigned int wg_id_sgpr;

	/* Addresses and entries of tables that reside in global memory */
	unsigned int const_buf_table;
	struct si_ndrange_table_entry_t
		const_buf_table_entries[SI_EMU_MAX_NUM_CONST_BUFS];
	unsigned int resource_table;
	struct si_ndrange_table_entry_t
		resource_table_entries[SI_EMU_MAX_NUM_RESOURCES];
	unsigned int uav_table;
	struct si_ndrange_table_entry_t
		uav_table_entries[SI_EMU_MAX_NUM_UAVS];
	unsigned int vertex_buffer_table;
	struct si_ndrange_table_entry_t
		vertex_buffer_table_entries[SI_EMU_MAX_NUM_VERTEX_BUFFERS];

	/* Addresses of the constant buffers */
	unsigned int cb0;
	unsigned int cb1;

	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel 
	 * report option is active. */
	unsigned int *inst_histogram;
};


struct si_ndrange_t *si_ndrange_create();
void si_ndrange_free(struct si_ndrange_t *ndrange);
void si_ndrange_dump(struct si_ndrange_t *ndrange, FILE *f);

/* Functions to set up ND-Range after initialization */
void si_ndrange_setup_size(struct si_ndrange_t *ndrange, 
	unsigned int *global_size, unsigned int *local_size, int work_dim);
void si_ndrange_setup_fs_mem(struct si_ndrange_t *ndrange, void *buf, 
	int size, unsigned int pc);
void si_ndrange_setup_inst_mem(struct si_ndrange_t *ndrange, void *buf, 
	int size, unsigned int pc);

/* Access constant buffers */
void si_ndrange_const_buf_write(struct si_ndrange_t *ndrange, 
	int const_buf_num, int offset, void *pvalue, unsigned int size);
void si_ndrange_const_buf_read(struct si_ndrange_t *ndrange, int const_buf_num, 	int offset, void *pvalue, unsigned int size);

/* Access internal tables */
void si_ndrange_insert_buffer_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int uav);
void si_ndrange_insert_buffer_into_vertex_buffer_table(struct si_ndrange_t *ndrange,
	struct si_buffer_desc_t *buf_desc, unsigned int vertex_buffer);
void si_ndrange_insert_buffer_into_const_buf_table(struct si_ndrange_t *ndrange,
        struct si_buffer_desc_t *buf_desc, unsigned int const_buf_num);
void si_ndrange_insert_image_into_uav_table(struct si_ndrange_t *ndrange,
        struct si_image_desc_t *image_desc, unsigned int uav);

#endif
