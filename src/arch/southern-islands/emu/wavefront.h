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
	char name[30];
	int id;
	int id_in_work_group;

	/* IDs of work-items it contains */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* NDRange and Work-group it belongs to */
	struct si_ndrange_t *ndrange;
	struct si_work_group_t *work_group;

	/* Instruction buffer (code, not architectural instruction buffer) */
	void *wavefront_pool;
	void *wavefront_pool_start;

	/* Current instruction */
	struct si_inst_t inst;
	int inst_size;

	/* Pointer to work_items */
	struct si_work_item_t *scalar_work_item;
	struct si_work_item_t **work_items;  /* Pointer to first work-items in 'kernel->work_items' */
	union si_reg_t sreg[256];  /* Scalar registers--used by scalar work items */

	/* Predicate mask */
	struct bit_map_t *pred;  /* work_item_count elements */

	/* Flags updated during instruction execution */
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;
	unsigned int mem_wait : 1;
	unsigned int barrier : 1;
	unsigned int finished : 1;

	/* Linked lists */
	struct si_wavefront_t *running_list_next;
	struct si_wavefront_t *running_list_prev;
	struct si_wavefront_t *barrier_list_next;
	struct si_wavefront_t *barrier_list_prev;
	struct si_wavefront_t *finished_list_next;
	struct si_wavefront_t *finished_list_prev;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;

	/* Fields introduced for timing simulation */
	int id_in_compute_unit;
	int uop_id_counter;
	long long sched_when;  /* GPU cycle when wavefront was last scheduled */
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;

	/* Statistics */
	long long inst_count;  /* Total number of instructions */
	long long scalar_mem_inst_count;
	long long scalar_alu_inst_count;
	long long branch_inst_count;
	long long vector_mem_inst_count;
	long long vector_alu_inst_count;
	long long global_mem_inst_count;
	long long local_mem_inst_count;
};

#define SI_FOREACH_WAVEFRONT_IN_NDRANGE(NDRANGE, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (NDRANGE)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (NDRANGE)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

#define SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (WORK_GROUP)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (WORK_GROUP)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

struct si_wavefront_t *si_wavefront_create();
void si_wavefront_sreg_init(struct si_wavefront_t *wavefront);
void si_wavefront_free(struct si_wavefront_t *wavefront);
void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f);

void si_wavefront_stack_push(struct si_wavefront_t *wavefront);
void si_wavefront_stack_pop(struct si_wavefront_t *wavefront, int count);
void si_wavefront_execute(struct si_wavefront_t *wavefront);

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, int id_in_wavefront);

#endif
