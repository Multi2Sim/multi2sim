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

#include <gpukernel.h>
#include <gpuarch.h>
#include <options.h>
#include <config.h>
#include <debug.h>
#include <repos.h>



/*
 * Global variables
 */

char *gpu_config_help =
	"The GPU configuration file is a plain text file in the IniFile format, defining\n"
	"the parameters of the GPU model for a detailed (architectural) GPU configuration.\n"
	"This file is passed to Multi2Sim with the '--gpu-config <file>' option, and\n"
	"should always be used together with option '--gpu-sim detailed'.\n"
	"\n"
	"The following is a list of the sections allowed in the GPU configuration file,\n"
	"along with the list of variables for each section.\n"
	"\n"
	"Section '[ Device ]':\n"
	"\n"
	"  NumComputeUnits = <num> (Default = 20)\n"
	"      Number of compute units in the GPU. These are the hardware components\n"
	"      where software work-groups are executed.\n"
	"\n"
	"Section '[ ComputeUnit ]':\n"
	"\n"
	"  WavefrontSize = <size> (Default = 64)\n"
	"      Number of work-items within a wavefront which execute AMD Evergreen\n"
	"      instructions in a SIMD fashion.\n"
	"  MaxWorkGroupSize = <size> (default = 256)\n"
	"      Maximum number of work-items within a work-group. This is a device-specific\n"
	"      architectural parameter return by some OpenCL calls.\n"
	"  NumStreamCores = <num> (Default = 16)\n"
	"      Number of stream cores within a compute unit. Each work-item is mapped to a\n"
	"      stream core. Stream cores are time-multiplexed to cover all work-items in a\n"
	"      wavefront.\n"
	"\n";

enum gpu_sim_kind_enum gpu_sim_kind = gpu_sim_kind_functional;

char *gpu_config_file_name = "";

int gpu_pipeline_debug_category;

/* Default parameters based on the AMD Radeon HD 5870 */
int gpu_num_compute_units = 20;
int gpu_num_stream_cores = 16;

/* Number of time multiplexing slots for a stream core among different
 * portions of a wavefront. This parameter is computed as the ceiling
 * of the quotient between the wavefront size and number of stream cores. */
int gpu_compute_unit_time_slots;

struct gpu_t *gpu;

struct repos_t *gpu_uop_repos;






/*
 * GPU Uop
 */

static uint64_t gpu_uop_id_counter = 1000;

struct gpu_uop_t *gpu_uop_create()
{
	struct gpu_uop_t *uop;

	uop = repos_create_object(gpu_uop_repos);
	uop->id = gpu_uop_id_counter++;
	return uop;
}


void gpu_uop_free(struct gpu_uop_t *gpu_uop)
{
	repos_free_object(gpu_uop_repos, gpu_uop);
}




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
	free(compute_unit);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void gpu_compute_unit_next_cycle(struct gpu_compute_unit_t *compute_unit)
{
	gpu_compute_unit_write(compute_unit);
	gpu_compute_unit_execute(compute_unit);
	gpu_compute_unit_read(compute_unit);
	gpu_compute_unit_decode(compute_unit);
	gpu_compute_unit_fetch(compute_unit);
	gpu_compute_unit_schedule(compute_unit);
}




/*
 * GPU Device
 */

static void gpu_init_device()
{
	struct gpu_compute_unit_t *compute_unit;
	struct gpu_stream_core_t *stream_core;

	int compute_unit_id;
	int stream_core_id;

	/* Create device */
	gpu = calloc(1, sizeof(struct gpu_t));

	/* Create compute units */
	gpu->compute_units = calloc(gpu_num_compute_units, sizeof(void *));
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		gpu->compute_units[compute_unit_id] = gpu_compute_unit_create();
		compute_unit = gpu->compute_units[compute_unit_id];
		compute_unit->id = compute_unit_id;
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, idle, compute_unit);
		
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
}


void gpu_init()
{
	struct config_t *gpu_config;
	char *section;

	/* Debug */
	gpu_pipeline_debug_category = debug_new_category();

	/* Load GPU configuration file */
	gpu_config = config_create(gpu_config_file_name);
	if (*gpu_config_file_name && !config_load(gpu_config))
		fatal("%s: cannot load GPU configuration file", gpu_config_file_name);
	
	/*
	 * Read configuration file
	 */
	
	/* Device */
	section = "Device";
	gpu_num_compute_units = config_read_int(gpu_config, section, "NumComputeUnits", gpu_num_compute_units);
	
	/* Compute Unit */
	section = "ComputeUnit";
	gpu_wavefront_size = config_read_int(gpu_config, section, "WavefrontSize", gpu_wavefront_size);
	gpu_max_work_group_size = config_read_int(gpu_config, section, "MaxWorkGroupSize", gpu_max_work_group_size);
	if (gpu_max_work_group_size & (gpu_max_work_group_size - 1))
		fatal("'MaxWorkGroupSize' must be a power of 2");
	gpu_num_stream_cores = config_read_int(gpu_config, section, "NumStreamCores", gpu_num_stream_cores);
	gpu_compute_unit_time_slots = (gpu_wavefront_size + gpu_num_stream_cores - 1) / gpu_num_stream_cores;
	
	/* Stream Core */
	section = "StreamCore";
	
	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);

	/* Initialize GPU */
	gpu_init_device();

	/* GPU uop repository.
	 * The size assigned for each 'gpu_uop_t' is equals to the baseline structure size plus the
	 * size of a 'gpu_work_item_uop_t' element for each work-item in the wavefront. */
	gpu_uop_repos = repos_create(sizeof(struct gpu_uop_t) + sizeof(struct gpu_work_item_uop_t)
		* gpu_wavefront_size, "gpu_uop_repos");
	
	/* GPU memory access repository */
	gpu_mem_access_repos = repos_create(sizeof(struct gpu_mem_access_t), "gpu_mem_access_repos");
}


void gpu_done()
{
	struct gpu_compute_unit_t *compute_unit;
	struct gpu_stream_core_t *stream_core;

	int compute_unit_id;
	int stream_core_id;

	/* Free stream cores, compute units, and device */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		compute_unit = gpu->compute_units[compute_unit_id];
		FOREACH_STREAM_CORE(stream_core_id) {
			stream_core = compute_unit->stream_cores[stream_core_id];
			gpu_stream_core_free(stream_core);
		}
		free(compute_unit->stream_cores);
		gpu_compute_unit_free(compute_unit);
	}
	free(gpu->compute_units);
	free(gpu);
	
	/* GPU uop repository */
	repos_free(gpu_uop_repos);

	/* GPU memory access repository */
	repos_free(gpu_mem_access_repos);
}



static void gpu_schedule_work_groups(void)
{
	struct gpu_ndrange_t *ndrange = gpu->ndrange;
	struct gpu_work_group_t *work_group;
	struct gpu_compute_unit_t *compute_unit;

	while (gpu->idle_list_head && ndrange->pending_list_head) {
		work_group = ndrange->pending_list_head;
		compute_unit = gpu->idle_list_head;

		/* Change work-group status to running, which implicitly removes it from
		 * the 'pending' list, and inserts it to the 'running' list */
		gpu_work_group_clear_status(work_group, gpu_work_group_pending);
		gpu_work_group_set_status(work_group, gpu_work_group_running);
		
		/* Delete compute unit from 'idle' list and insert it to 'busy' list. */
		DOUBLE_LINKED_LIST_REMOVE(gpu, idle, compute_unit);
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, busy, compute_unit);

		/* Assign work-group to compute unit */
		INIT_SCHEDULE.do_schedule = 1;
		INIT_SCHEDULE.work_group_id = work_group->id;
		INIT_SCHEDULE.wavefront_id = work_group->wavefront_id_first;
		INIT_SCHEDULE.subwavefront_id = 0;

		/* Debug */
		gpu_pipeline_debug("cu "
			"a=\"run\" "
			"id=\"%d\" "
			"wg=\"%d\""
			"\n",
			compute_unit->id,
			work_group->id);
	}
}


void gpu_run(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	struct gpu_compute_unit_t *compute_unit, *compute_unit_next;

	/* Debug */
	gpu_pipeline_debug("init "
		"global_size=%d "
		"local_size=%d "
		"group_count=%d "
		"wavefront_size=%d "
		"wavefronts_per_work_group=%d"
		"\n",
		kernel->global_size,
		kernel->local_size,
		kernel->group_count,
		gpu_wavefront_size,
		ndrange->wavefronts_per_work_group);

	gpu->ndrange = ndrange;

	for (;;) {
		
		/* Assign pending work-items to idle compute units. */
		if (gpu->idle_list_head && ndrange->pending_list_head)
			gpu_schedule_work_groups();
		
		/* If no compute unit got any work, done. */
		if (!gpu->busy_list_head)
			break;

		/* Next cycle */
		gpu->cycle++;
		gpu_pipeline_debug("clk c=%lld\n", (long long) gpu->cycle);
		
		/* Advance one cycle on each busy compute unit */
		for (compute_unit = gpu->busy_list_head; compute_unit; compute_unit = compute_unit_next) {
			
			/* Save next non-idle compute unit */
			compute_unit_next = compute_unit->busy_next;

			/* Simulate cycle in compute unit */
			gpu_compute_unit_next_cycle(compute_unit);
		}
	}
}

