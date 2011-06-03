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

struct gpu_device_t *gpu_device;

struct repos_t *gpu_uop_repos;






/*
 * Public Functions
 */

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
	gpu_device = gpu_device_create();

	/* GPU uop repository */
	gpu_uop_repos = repos_create(sizeof(struct gpu_uop_t), "gpu_uop_repos");
}


void gpu_done()
{
	repos_free(gpu_uop_repos);
	gpu_device_free(gpu_device);
}



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

