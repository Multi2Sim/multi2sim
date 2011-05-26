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

#include <gpuarch.h>


/*
 * GPU Stream Core
 */

struct gpu_stream_core_t *gpu_stream_core_create()
{
	struct gpu_stream_core_t *stream_core;

	stream_core = calloc(1, sizeof(struct gpu_stream_core_t));
	return stream_core;
}


void gpu_stream_core_free(struct gpu_stream_core_t *stream_core)
{
	free(stream_core);
}




/*
 * GPU Compute Unit
 */

struct gpu_compute_unit_t *gpu_compute_unit_create()
{
	struct gpu_compute_unit_t *compute_unit;

	compute_unit = calloc(1, sizeof(struct gpu_compute_unit_t));
	return compute_unit;
}


void gpu_compute_unit_free(struct gpu_compute_unit_t *compute_unit)
{
	free(compute_unit->stream_cores);
	free(compute_unit);
}




/*
 * GPU Device
 */

struct gpu_device_t *gpu_device_create()
{
	struct gpu_device_t *device;
	struct gpu_compute_unit_t *compute_unit;
	struct gpu_stream_core_t *stream_core;

	int compute_unit_id;
	int stream_core_id;

	/* Create device */
	device = calloc(1, sizeof(struct gpu_device_t));

	/* Create compute units */
	device->compute_units = calloc(gpu_num_compute_units, sizeof(void *));
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		device->compute_units[compute_unit_id] = gpu_compute_unit_create();
		compute_unit = device->compute_units[compute_unit_id];
		compute_unit->id = compute_unit_id;
		compute_unit->device = device;
		DOUBLE_LINKED_LIST_INSERT_TAIL(device, idle, compute_unit);
		
		/* Create stream cores */
		compute_unit->stream_cores = calloc(gpu_num_stream_cores, sizeof(void *));
		FOREACH_STREAM_CORE(stream_core_id)
		{
			compute_unit->stream_cores[stream_core_id] = gpu_stream_core_create();
			stream_core = compute_unit->stream_cores[stream_core_id];
			stream_core->id = stream_core_id;
			stream_core->compute_unit = compute_unit;
		}
	}
	return device;
}


void gpu_device_free(struct gpu_device_t *device)
{
	free(device->compute_units);
	free(device);
}


void gpu_device_run(struct gpu_device_t *device, struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	struct gpu_compute_unit_t *compute_unit;

	/* Debug */
	gpu_pipeline_debug("init global_size=%d, local_size=%d, group_count=%d, wavefront_size=%d, "
		"wavefronts_per_work_group=%d\n",
		kernel->global_size, kernel->local_size, kernel->group_count, gpu_wavefront_size,
		ndrange->wavefronts_per_work_group);

	device->ndrange = ndrange;
	compute_unit = device->compute_units[0];
	INIT_SCHEDULE.do_schedule = 1;
	INIT_SCHEDULE.work_group_id = 0;
	INIT_SCHEDULE.wavefront_id = 0;
	INIT_SCHEDULE.subwavefront_id = 0;

	while (INIT_SCHEDULE.do_schedule) {
		gpu_compute_unit_next_cycle(compute_unit);
	}
}

