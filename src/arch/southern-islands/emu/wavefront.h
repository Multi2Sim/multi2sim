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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WAVEFRONT_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WAVEFRONT_H

#include <arch/southern-islands/asm/asm.h>


struct si_wavefront_t
{
	/* ID */
	int id;

	/* IDs of work-items it contains */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* Work-group it belongs to */
	struct si_work_group_t *work_group;

	/* Program counter. Offset in 'inst_buffer' where we can find the next
	 * instruction to be executed. */
	unsigned int pc;

	/* Current instruction */
	struct si_inst_t inst;
	int inst_size;

	/* Pointer to work_items */
	struct si_work_item_t *scalar_work_item;
	struct si_work_item_t **work_items;  

	/* Scalar registers */
	union si_reg_t sreg[256];

	/* Flags updated during instruction execution */
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int lds_read : 1;
	unsigned int lds_write : 1;
	unsigned int mem_wait : 1;
	unsigned int at_barrier : 1;  // Used for emu synchronization
	unsigned int finished : 1;
	unsigned int vector_mem_glc : 1;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;

	/* Fields introduced for timing simulation */
	int id_in_compute_unit;
	int uop_id_counter;
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;
	unsigned int barrier_inst : 1;

	/* Statistics */
	long long inst_count;  /* Total number of instructions */
	long long scalar_mem_inst_count;
	long long scalar_alu_inst_count;
	long long branch_inst_count;
	long long vector_mem_inst_count;
	long long vector_alu_inst_count;
	long long global_mem_inst_count;
	long long lds_inst_count;
	long long export_inst_count;
};

#define SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = 0; \
		(WAVEFRONT_ID) < (WORK_GROUP)->wavefront_count; \
		(WAVEFRONT_ID)++)

struct si_wavefront_t *si_wavefront_create(int wavefront_id,
	struct si_work_group_t *work_group);
void si_wavefront_sreg_init(struct si_wavefront_t *wavefront);
void si_wavefront_free(struct si_wavefront_t *wavefront);
void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f);

void si_wavefront_execute(struct si_wavefront_t *wavefront);

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, 
	int id_in_wavefront);

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, 
	int sreg, unsigned int value);
void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs, int cb);
void si_wavefront_init_sreg_with_cb_table(struct si_wavefront_t *wavefront,
        int first_reg, int num_regs);
void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs);
void si_wavefront_init_sreg_with_uav(struct si_wavefront_t *wavefront,
	int first_reg, int num_regs, int uav);
void si_wavefront_init_sreg_with_vertex_buffer_table(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs);
void si_wavefront_init_sreg_with_fetch_shader(struct si_wavefront_t *wavefront, 
	int first_reg, int num_regs);


#endif
