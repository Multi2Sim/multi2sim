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

#ifndef ARCH_EVERGREEN_TIMING_GPU_H
#define ARCH_EVERGREEN_TIMING_GPU_H

#include <arch/common/timing.h>


/* Trace */
#define evg_tracing() trace_status(evg_trace_category)
#define evg_trace(...) trace(evg_trace_category, __VA_ARGS__)
#define evg_trace_header(...) trace_header(evg_trace_category, __VA_ARGS__)
extern int evg_trace_category;

extern char *evg_gpu_config_help;
extern char *evg_gpu_config_file_name;
extern char *evg_gpu_report_file_name;

extern int evg_gpu_num_stream_cores;
extern int evg_gpu_num_compute_units;
extern int evg_gpu_num_registers;
extern int evg_gpu_register_alloc_size;

extern struct str_map_t evg_gpu_register_alloc_granularity_map;
extern enum evg_gpu_register_alloc_granularity_t
{
	evg_gpu_register_alloc_invalid = 0,  /* For invalid user input */
	evg_gpu_register_alloc_wavefront,
	evg_gpu_register_alloc_work_group
} evg_gpu_register_alloc_granularity;

extern int evg_gpu_max_work_groups_per_compute_unit;
extern int evg_gpu_max_wavefronts_per_compute_unit;

extern char *evg_gpu_calc_file_name;

extern int evg_gpu_local_mem_size;
extern int evg_gpu_local_mem_alloc_size;
extern int evg_gpu_local_mem_latency;
extern int evg_gpu_local_mem_block_size;
extern int evg_gpu_local_mem_num_ports;

extern int evg_gpu_cf_engine_inst_mem_latency;

extern int evg_gpu_alu_engine_inst_mem_latency;
extern int evg_gpu_alu_engine_fetch_queue_size;
extern int evg_gpu_alu_engine_pe_latency;

extern int evg_gpu_tex_engine_inst_mem_latency;
extern int evg_gpu_tex_engine_fetch_queue_size;
extern int evg_gpu_tex_engine_load_queue_size;


#define EVG_GPU_FOREACH_COMPUTE_UNIT(COMPUTE_UNIT_ID) \
	for ((COMPUTE_UNIT_ID) = 0; (COMPUTE_UNIT_ID) < evg_gpu_num_compute_units; (COMPUTE_UNIT_ID)++)

#define EVG_GPU_FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(WAVEFRONT, SUBWAVEFRONT_ID, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first + (SUBWAVEFRONT_ID) * evg_gpu_num_stream_cores; \
		(WORK_ITEM_ID) <= MIN((WAVEFRONT)->work_item_id_first + ((SUBWAVEFRONT_ID) + 1) \
			* evg_gpu_num_stream_cores - 1, (WAVEFRONT)->work_item_id_last); \
		(WORK_ITEM_ID)++)


/*
 * Class 'EvgGpu'
 */

CLASS_BEGIN(EvgGpu, Timing)

	/* ND-Range running on it */
	struct evg_ndrange_t *ndrange;
	int work_groups_per_compute_unit;
	int wavefronts_per_compute_unit;
	int work_items_per_compute_unit;

	/* Compute units */
	struct evg_compute_unit_t **compute_units;

	/* List of ready compute units accepting work-groups */
	struct evg_compute_unit_t *ready_list_head;
	struct evg_compute_unit_t *ready_list_tail;
	int ready_list_count;
	int ready_list_max;

	/* List of busy compute units */
	struct evg_compute_unit_t *busy_list_head;
	struct evg_compute_unit_t *busy_list_tail;
	int busy_list_count;
	int busy_list_max;

	/* List of deleted instructions */
	struct linked_list_t *trash_uop_list;

	/* Last cycle when an instructions completed in any engine. This is used
 	 * to decide when to stop simulation if there was a simulation stall. */
	long long last_complete_cycle;

CLASS_END(EvgGpu)

void EvgGpuCreate(EvgGpu *self);
void EvgGpuDestroy(EvgGpu *self);

void EvgGpuDump(Object *self, FILE *f);
void EvgGpuDumpSummary(Timing *self, FILE *f);

int EvgGpuRun(Timing *self);




/*
 * Public Functions
 */

extern EvgGpu *evg_gpu;

void evg_gpu_read_config(void);

void evg_gpu_init(void);
void evg_gpu_done(void);

void evg_gpu_dump_report(void);

struct evg_uop_t;
void evg_gpu_uop_trash_add(struct evg_uop_t *uop);
void evg_gpu_uop_trash_empty(void);




#endif

