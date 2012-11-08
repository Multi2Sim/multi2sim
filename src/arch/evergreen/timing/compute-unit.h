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

#ifndef ARCH_EVERGREEN_COMPUTE_UNIT_H
#define ARCH_EVERGREEN_COMPUTE_UNIT_H

#include "uop.h"


/*
 * Register file in Compute Unit
 */

struct evg_reg_file_t;
struct evg_compute_unit_t;

void evg_reg_file_init(struct evg_compute_unit_t *compute_unit);
void evg_reg_file_done(struct evg_compute_unit_t *compute_unit);

void evg_reg_file_map_work_group(struct evg_compute_unit_t *compute_unit,
	struct evg_work_group_t *work_group);
void evg_reg_file_unmap_work_group(struct evg_compute_unit_t *compute_unit,
	struct evg_work_group_t *work_group);

int evg_reg_file_rename(struct evg_compute_unit_t *compute_unit,
	struct evg_work_item_t *work_item, int logical_register);
void evg_reg_file_inverse_rename(struct evg_compute_unit_t *compute_unit,
	int physical_register, struct evg_work_item_t **work_item, int *logical_register);





/*
 * GPU Compute Unit
 */

struct evg_compute_unit_t
{
	/* IDs */
	int id;
	long long gpu_uop_id_counter;  /* Counter to assign 'id_in_compute_unit' to uops */

	/* Double linked list of compute units */
	struct evg_compute_unit_t *ready_list_prev;
	struct evg_compute_unit_t *ready_list_next;
	struct evg_compute_unit_t *busy_list_prev;
	struct evg_compute_unit_t *busy_list_next;

	/* Entry points to memory hierarchy */
	struct mod_t *global_memory;
	struct mod_t *local_memory;

	/* List of currently mapped work-groups */
	int work_group_count;
	struct evg_work_group_t **work_groups;

	/* Statistics */
	long long mapped_work_groups;
	long long cycle;
	long long inst_count;

	/* Statistics for spatial report */
	long long inflight_mem_accesses ;
	long long interval_cycle;
	FILE * spatial_report_file;

	/* Ready wavefront pool.
	 * It includes suspended wavefronts, but excludes wavefronts in
	 * flight in the CF pipeline or running on the ALU/TEX Engines. */
	struct linked_list_t *wavefront_pool;

	/* Fields for CF Engine */
	struct
	{
		/* Buffers */
		struct evg_uop_t **fetch_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */
		struct evg_uop_t **inst_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */

		/* Wavefront selectors */
		int decode_index;  /* Next uop in 'fetch_buffer' to decode */
		int execute_index;  /* Next uop in 'inst_buffer' to execute */

		/* Complete queue */
		struct linked_list_t *complete_queue;  /* Queue of completed instructions */

		/* Statistics */
		long long inst_count;
		long long alu_clause_trigger_count;
		long long tex_clause_trigger_count;
		long long global_mem_write_count;

		/* Spatial Interval Reports */
		long long interval_inst_count;


	} cf_engine;

	/* Fields for ALU Engine */
	struct
	{
		/* CF uop queues.
		 * The CF engine enqueues a CF uop that triggers an ALU clause into 'pending_queue'.
		 * When the secondary clause finishes fetching, the CF uop is moved to
		 * 'finished_queue'. When secondary clause exists pipeline, it is extracted from
		 " 'finished_queue'. */
		struct linked_list_t *pending_queue;
		struct linked_list_t *finished_queue;

		/* Queues */
		struct linked_list_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct evg_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct evg_uop_t *exec_buffer;  /* Uop from read to execute stage */
		struct heap_t *event_queue;  /* Events for instruction execution */

		/* Table storing the in-flight uop that produced an output
		 * dependence. If the producer is not in flight, the value is NULL. */
		struct evg_uop_t *producers[EVG_UOP_DEP_COUNT];

		/* Statistics */
		long long wavefront_count;
		long long cycle;
		long long inst_count;
		long long inst_slot_count;
		long long local_mem_slot_count;
		long long vliw_slots[5];

		/* Spatial Interval Reports */
		long long interval_inst_count;


	} alu_engine;

	/* Fields for TEX Engine */
	struct
	{
		/* CF uop queues (see comment for ALU engine) */
		struct linked_list_t *pending_queue;
		struct linked_list_t *finished_queue;

		/* Queues */
		struct linked_list_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct evg_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct linked_list_t *load_queue;  /* Uops from read to write stage */

		/* Statistics */
		long long wavefront_count;
		long long cycle;
		long long inst_count;

		/* Spatial Interval Reports */
		long long interval_inst_count;

	} tex_engine;

};

struct evg_compute_unit_t *evg_compute_unit_create(void);
void evg_compute_unit_free(struct evg_compute_unit_t *gpu_compute_unit);
void evg_compute_unit_map_work_group(struct evg_compute_unit_t *compute_unit, struct evg_work_group_t *work_group);
void evg_compute_unit_unmap_work_group(struct evg_compute_unit_t *compute_unit, struct evg_work_group_t *work_group);
void evg_compute_unit_run(struct evg_compute_unit_t *compute_unit);

#endif

