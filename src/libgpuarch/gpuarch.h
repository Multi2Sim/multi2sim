/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#ifndef GPUARCH_H
#define GPUARCH_H

#include <gpukernel.h>



/* Public variables */

extern int gpu_num_stream_cores;
extern int gpu_num_compute_units;
extern int gpu_compute_unit_time_slots;

extern struct gpu_device_t *gpu_device;




/* GPU Stream Core */

struct gpu_stream_core_t {
	
	/* ID */
	int id;

	/* Compute unit it belongs to */
	struct gpu_compute_unit_t *compute_unit;

};

struct gpu_stream_core_t *gpu_stream_core_create();
void gpu_stream_core_free(struct gpu_stream_core_t *gpu_stream_core);




/* GPU Compute Unit */

struct gpu_compute_unit_t
{
	/* ID */
	int id;

	/* Device that it belongs to */
	struct gpu_device_t *device;

	/* Stream cores */
	struct gpu_stream_core_t **stream_cores;

	/* Double linked list for idle compute units */
	struct gpu_compute_unit_t *idle_prev, *idle_next;

	/* Initial pipe register (for Schedule stage state) */
	struct {
		int do_schedule;
		int work_group_id;
		int wavefront_id;
		int subwavefront_id;
		struct gpu_wavefront_t *wavefront_running_next;
	} init_schedule;

	/* Schedule/Fetch pipe register */
	struct {
		int do_fetch;  /* Set to 1 by 'schedule' stage if output valid */
	} schedule_fetch;

	/* Fetch/Decode pipe register */
	struct {
		int do_decode;
	} fetch_decode;

	/* Decode/Read pipe register */
	struct {
		int do_read;
	} decode_read;

	/* Read/Execute pipe register */
	struct {
		int do_execute;
	} read_execute;

	/* Execute/Write pipe register */
	struct {
		int do_write;
	} execute_write;
};

#define FOREACH_STREAM_CORE(STREAM_CORE_ID) \
	for ((STREAM_CORE_ID) = 0; (STREAM_CORE_ID) < gpu_num_stream_cores; (STREAM_CORE_ID)++)

/* Macros for quick access to pipe registers */
#define INIT_SCHEDULE  (compute_unit->init_schedule)
#define SCHEDULE_FETCH  (compute_unit->schedule_fetch)
#define FETCH_DECODE  (compute_unit->fetch_decode)
#define DECODE_READ  (compute_unit->decode_read)
#define READ_EXECUTE  (compute_unit->read_execute)
#define EXECUTE_WRITE  (compute_unit->execute_write)


struct gpu_compute_unit_t *gpu_compute_unit_create();
void gpu_compute_unit_free(struct gpu_compute_unit_t *gpu_compute_unit);

void gpu_compute_unit_next_cycle(struct gpu_compute_unit_t *compute_unit);




/* GPU Device */

struct gpu_device_t
{
	/* ND-Range running on it */
	struct gpu_ndrange_t *ndrange;

	/* Compute units */
	struct gpu_compute_unit_t **compute_units;

	/* Double linked list of idle compute units */
	struct gpu_compute_unit_t *idle_list_head, *idle_list_tail;
	int idle_count, idle_max;
};

#define FOREACH_COMPUTE_UNIT(COMPUTE_UNIT_ID) \
	for ((COMPUTE_UNIT_ID) = 0; (COMPUTE_UNIT_ID) < gpu_num_compute_units; (COMPUTE_UNIT_ID)++)

struct gpu_device_t *gpu_device_create();
void gpu_device_free(struct gpu_device_t *device);

void gpu_device_run(struct gpu_device_t *device, struct gpu_ndrange_t *ndrange);




/* Generic public functions */

/* Debugging */
#define gpu_pipeline_debug(...) debug(gpu_pipeline_debug_category, __VA_ARGS__)
extern int gpu_pipeline_debug_category;


void gpu_reg_options(void);
void gpu_init(void);
void gpu_done(void);



#endif

