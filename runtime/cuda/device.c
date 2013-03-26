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
struct cuda_device_t *cuda_device_create(void)
{
	struct cuda_device_t *device;

	/* Initialize */
	device = (struct cuda_device_t *)xcalloc(1, sizeof(struct cuda_device_t));
	device->device = list_count(device_list);
	device->name = xstrdup("Multi2Sim Fermi Device");
	device->cc.major = 2;
	device->cc.minor = 2;
	device->properties.maxThreadsPerBlock = 1024;
	device->properties.maxThreadsDim[0] = 1024;
	device->properties.maxThreadsDim[1] = 1024;
	device->properties.maxThreadsDim[2] = 64;
	device->properties.maxGridSize[0] = 65535;
	device->properties.maxGridSize[1] = 65535;
	device->properties.maxGridSize[2] = 65535;
	device->properties.sharedMemPerBlock = 49152;
	device->properties.totalConstantMemory = 65536;
	device->properties.SIMDWidth = 32;  /* warp size */
	device->properties.memPitch = 2147483647;
	device->properties.regsPerBlock = 32768;
	device->properties.clockRate = 1000000;
	device->properties.textureAlign = 512;  /* FIXME: check */
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK] = 512;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK] = 4;
	device->attributes[CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_WARP_SIZE] = 3;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_PITCH] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT] = 6;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_WIDTH] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_HEIGHT] = 2;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK] = 3;
	device->attributes[CU_DEVICE_ATTRIBUTE_CLOCK_RATE] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT] = 5;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_GPU_OVERLAP] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT] = 1;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_INTEGRATED] = 0;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY] = 1;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_MODE] = CU_COMPUTEMODE_DEFAULT;
	device->attributes[CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_ECC_ENABLED] = 0;
	device->attributes[CU_DEVICE_ATTRIBUTE_PCI_BUS_ID] = 8;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID] = 0;  /* FIXME */
	device->attributes[CU_DEVICE_ATTRIBUTE_TCC_DRIVER] = 0;
	device->attributes[CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH] = 3;
	device->attributes[CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE] = 7;
	device->attributes[CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR] = 1;
	device->attributes[CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING] = 0;

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

