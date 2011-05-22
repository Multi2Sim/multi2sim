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
	struct gpu_stream_core_t *sc;

	sc = calloc(1, sizeof(struct gpu_stream_core_t));
	return sc;
}


void gpu_stream_core_free(struct gpu_stream_core_t *sc)
{
	free(sc);
}




/*
 * GPU Compute Unit
 */

struct gpu_compute_unit_t *gpu_compute_unit_create()
{
	struct gpu_compute_unit_t *cu;

	cu = calloc(1, sizeof(struct gpu_compute_unit_t));
	cu->stream_cores = calloc(gpu_num_stream_cores, sizeof(void *));
	return cu;
}


void gpu_compute_unit_free(struct gpu_compute_unit_t *cu)
{
	free(cu->stream_cores);
	free(cu);
}




/*
 * GPU Device
 */

struct gpu_device_t *gpu_device_create()
{
	struct gpu_device_t *device;

	device = calloc(1, sizeof(struct gpu_device_t));
	device->compute_units = calloc(gpu_num_compute_units, sizeof(void *));
	return device;
}


void gpu_device_free(struct gpu_device_t *device)
{
	free(device->compute_units);
	free(device);
}

