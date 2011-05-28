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

/* Global variables */

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




/* Private variables */

static struct config_t *gpu_config;




/*
 * Private functions
 */

static void gpu_config_default()
{
}




/*
 * Public Functions
 */

void gpu_reg_options()
{
	opt_reg_string("-gpuconfig", "Configuration file for the GPU model",
		&gpu_config_file_name);
}


void gpu_init()
{
	char *section;

	/* Debug */
	gpu_pipeline_debug_category = debug_new_category();

	/* Load GPU configuration file */
	gpu_config = config_create(gpu_config_file_name);
	if (!*gpu_config_file_name)
		gpu_config_default();
	else if (!config_load(gpu_config))
		fatal("%s: cannot load GPU configuration file", gpu_config_file_name);
	
	/* Specify configuration file format */
	section = "Device";
	config_section_allow(gpu_config, section);
	config_key_allow(gpu_config, section, "NumComputeUnits");

	section = "ComputeUnit";
	config_section_allow(gpu_config, section);
	config_key_allow(gpu_config, section, "WavefrontSize");
	config_key_allow(gpu_config, section, "MaxWorkGroupSize");
	config_key_allow(gpu_config, section, "NumStreamCores");

	section = "StreamCore";
	config_section_allow(gpu_config, section);

	/* Check configuration file */
	config_check(gpu_config);


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
	config_free(gpu_config);


	/*
	 * Initialize GPU
	 */

	gpu_device = gpu_device_create();
}


void gpu_done()
{
	gpu_device_free(gpu_device);
}

