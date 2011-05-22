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
};

#define STREAM_CORE  (gpu_device->compute_units[compute_unit].stream_cores[stream_core])
#define FOREACH_STREAM_CORE  for (stream_core = 0; stream_core < gpu_num_stream_cores; stream_core++)

struct gpu_stream_core_t *gpu_stream_core_create();
void gpu_stream_core_free(struct gpu_stream_core_t *gpu_stream_core);




/* GPU Compute Unit */

struct gpu_compute_unit_t {
	
	/* Stream cores */
	struct gpu_stream_core_t *stream_cores;

	/* State for the 'schedule' stage */
	int current_wavefront_id;
	int current_subwavefront_id;

	/* Initial pipe register (for Schedule stage state) */
	struct {
		int do_schedule;
		int wavefront_id;
		int subwavefront_id;
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

#define COMPUTE_UNIT  (gpu_device->compute_units[compute_unit])
#define FOREACH_COMPUTE_UNIT  for (compute_unit = 0; compute_unit < gpu_num_compute_units; compute_unit++)

struct gpu_compute_unit_t *gpu_compute_unit_create();
void gpu_compute_unit_free(struct gpu_compute_unit_t *gpu_compute_unit);

void gpu_compute_unit_next_cycle(int compute_unit);




/* GPU Device */

struct gpu_device_t {
	
	/* Compute units */
	struct gpu_compute_unit_t *compute_units;
};

struct gpu_device_t *gpu_device_create();
void gpu_device_free(struct gpu_device_t *gpu_device);



/* Generic public functions */

void gpu_reg_options();
void gpu_init();
void gpu_done();

void gpu_run(struct opencl_kernel_t *kernel); ///// FIXME


#endif

