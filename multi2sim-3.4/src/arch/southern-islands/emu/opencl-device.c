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

#include <assert.h>
#include <debug.h>
#include <stdlib.h>

#include <southern-islands-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


/* Create a device */
struct si_opencl_device_t *si_opencl_device_create()
{
	struct si_opencl_device_t *device;

	/* Allocate */
	device = calloc(1, sizeof(struct si_opencl_device_t));
	if (!device)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	device->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_device);

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, device);
	return device;
}


/* Free device */
void si_opencl_device_free(struct si_opencl_device_t *device)
{
	si_opencl_repo_remove_object(si_emu->opencl_repo, device);
	free(device);
}


uint32_t si_opencl_device_get_info(struct si_opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t max_compute_units = 1;  /* FIXME */
	uint32_t max_work_group_size = 256 * 256;  /* FIXME */
	uint32_t max_work_item_dimensions = 3;  /* FIXME */
	uint32_t max_work_item_sizes[3];  /* FIXME */
	uint32_t local_mem_type = 1;  /* CL_LOCAL FIXME */
	uint32_t local_mem_size = 32 * 1024;  /* FIXME */
	uint32_t max_clock_frequency = 850;
	uint64_t global_mem_size = 1ull << 31;  /* 2GB of global memory reported */
	uint32_t image_support = 1;

	char *device_name = "Multi2Sim Virtual GPU Device";
	char *device_vendor = "www.multi2sim.org";
	char *device_extensions = "cl_amd_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics "
		"cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "
		"cl_khr_gl_sharing cl_ext_device_fission cl_amd_device_attribute_query cl_amd_media_ops cl_amd_popcnt "
		"cl_amd_printf ";
	char *device_version = "OpenCL 1.1 ATI-Stream-v2.3 (451)";
	char *driver_version = VERSION;

	uint32_t size_ret = 0;
	void *info = NULL;

	switch (name)
	{

	case 0x1002:  /* CL_DEVICE_MAX_COMPUTE_UNITS */
		size_ret = 4;
		info = &max_compute_units;
		break;

	case 0x1003:  /* CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS */
		size_ret = 4;
		info = &max_work_item_dimensions;
		break;

	case 0x1004:  /* CL_DEVICE_MAX_WORK_GROUP_SIZE */
		size_ret = 4;
		info = &max_work_group_size;
		break;

	case 0x1005:  /* CL_DEVICE_MAX_WORK_ITEM_SIZES */
		max_work_item_sizes[0] = 256;
		max_work_item_sizes[1] = 256;
		max_work_item_sizes[2] = 256;
		size_ret = 12;
		info = max_work_item_sizes;
		break;
	
	case 0x100c:  /* CL_DEVICE_MAX_CLOCK_FREQUENCY */
		size_ret = 4;
		info = &max_clock_frequency;
		break;

	case 0x1016:  /* CL_DEVICE_IMAGE_SUPPORT */
		size_ret = 4;
		info = &image_support;
		break;

	case 0x101f:  /* CL_GLOBAL_MEM_SIZE */
		size_ret = 8;
		info = &global_mem_size;
		break;

	case 0x1022:  /* CL_DEVICE_LOCAL_MEM_TYPE */
		size_ret = 4;
		info = &local_mem_type;
		break;

	case 0x1023:  /* CL_DEVICE_LOCAL_MEM_SIZE */
		size_ret = 4;
		info = &local_mem_size;
		break;

	case 0x102b:  /* CL_DEVICE_NAME */
		size_ret = strlen(device_name) + 1;
		info = device_name;
		break;
	
	case 0x102c:  /* CL_DEVICE_VENDOR */
		size_ret = strlen(device_vendor) + 1;
		info = device_vendor;
		break;
	
	case 0x102d:  /* CL_DRIVER_VERSION */
		size_ret = strlen(driver_version) + 1;
		info = driver_version;
		break;
	
	case 0x102f:  /* CL_DEVICE_VERSION */
		size_ret = strlen(device_version) + 1;
		info = device_version;
		break;
	
	case 0x1030:  /* CL_DEVICE_EXTENSIONS */
		size_ret = strlen(device_extensions) + 1;
		info = device_extensions;
		break;

	default:
		fatal("opencl_device_get_info: invalid or not implemented value for 'name' (0x%x)\n%s",
			name, si_err_opencl_note);

	}

	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}

