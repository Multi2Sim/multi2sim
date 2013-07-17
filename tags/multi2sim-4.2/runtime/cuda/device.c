/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../include/cuda.h"
#include "device.h"
#include "list.h"
#include "mhandle.h"


struct list_t *device_list;

/* Create a device */
struct cuda_device_t *cuda_device_create(int device_type)
{
	struct cuda_device_t *device;

	/* Create device */
	device = (struct cuda_device_t *)xcalloc(1, 
			sizeof(struct cuda_device_t));

	/* Initialize */
	device->device = list_count(device_list);
	if (device_type == 0)
	{
		/* GeForce GTX 480 */
		device->name = xstrdup("Multi2Sim GeForce GTX 480");
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK]
			= 1024;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X] = 1024;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y] = 1024;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z] = 64;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X] = 65535;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y] = 65535;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z] = 65535;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK]
			= 49152;
		device->attributes[CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_WARP_SIZE] = 32;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_PITCH] = 2147483647;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_CLOCK_RATE] = 1401000;
		device->attributes[CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT] = 512;
		device->attributes[CU_DEVICE_ATTRIBUTE_GPU_OVERLAP] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT]
			= 15;
		device->attributes[CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_INTEGRATED] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_MODE] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT]
			= 65535;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_SURFACE_ALIGNMENT] = 512;
		device->attributes[CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_ECC_ENABLED] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_PCI_BUS_ID] = 2;
		device->attributes[CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_TCC_DRIVER] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE]
			= 1848000;
		device->attributes[CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH]
			= 384;
		device->attributes[CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE] = 786432;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR]
			= 1536;
		device->attributes[CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_CAN_TEX2D_GATHER] = 1;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_HEIGHT]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH_ALTERNATE]
			= 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT_ALTERNATE]
			= 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH_ALTERNATE]
			= 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID] = 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_TEXTURE_PITCH_ALIGNMENT]
			= 32;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_LAYERS]
			= 2046;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_HEIGHT]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_HEIGHT]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_DEPTH]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_LAYERS]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_WIDTH]
			= 65536;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_HEIGHT]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_LAYERS]
			= 2048;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_WIDTH]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_WIDTH]
			= 32768;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_LAYERS]
			= 2046;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LINEAR_WIDTH]
			= 134217728;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_WIDTH]
			= 65000;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_HEIGHT]
			= 65000;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_PITCH]
			= 1048544;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_WIDTH]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_HEIGHT]
			= 16384;
		device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR]
			= 2;
		device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR]
			= 0;
		device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_MIPMAPPED_WIDTH]
			= 16384;
	}

	list_add(device_list, device);

	return device;
}

/* Free device */
void cuda_device_free(struct cuda_device_t *device)
{
	list_remove(device_list, device);

	free(device->name);
	free(device);
}

