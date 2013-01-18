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

#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>
#include <arch/southern-islands/timing/gpu.h>

#include "device.h"
#include "repo.h"


/* Create a device */
struct si_opencl_device_t *si_opencl_device_create()
{
	struct si_opencl_device_t *device;

	/* Initialize */
	device = xcalloc(1, sizeof(struct si_opencl_device_t));
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


unsigned int si_opencl_device_get_info(struct si_opencl_device_t *device, unsigned int name, struct mem_t *mem, unsigned int addr, unsigned int size)
{
	unsigned int size_ret = 0;
	void *info = NULL;

	switch (name)
	{

	case 0x1000:  /* CL_DEVICE_TYPE */
		size_ret = 8;
		info = &si_gpu_device_type;
		break;

	case 0x1001:  /* CL_DEVICE_VENDOR_ID */
		size_ret = 4;
		info = &si_gpu_device_vendor_id;
		break;

	case 0x1002:  /* CL_DEVICE_MAX_COMPUTE_UNITS */
		size_ret = 4;
		info = &si_gpu_num_compute_units;
		break;

	case 0x1003:  /* CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS */
		size_ret = 4;
		info = &si_gpu_work_item_dimensions;
		break;

	case 0x1004:  /* CL_DEVICE_MAX_WORK_GROUP_SIZE */
		size_ret = 4;
		info = &si_gpu_work_group_size;
		break;

	case 0x1005:  /* CL_DEVICE_MAX_WORK_ITEM_SIZES */
		size_ret = 12;
		info = si_gpu_work_item_sizes;
		break;

	case 0x1006:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR */
	case 0x1007:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT */
	case 0x1008:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT */
	case 0x1009:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG */
	case 0x100a:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT */
	case 0x100b:  /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE */
		size_ret = 4;
		info = &si_gpu_simd_num_simd_lanes;
		break;
	
	case 0x100c:  /* CL_DEVICE_MAX_CLOCK_FREQUENCY */
		size_ret = 4;
		info = &si_gpu_max_clock_frequency;
		break;

	case 0x100d:  /* CL_DEVICE_ADDRESS_BITS */
		size_ret = 4;
		info = &si_gpu_address_bits;
		break;

	case 0x100e:  /* CL_DEVICE_MAX_READ_IMAGE_ARGS */
		size_ret = 4;
		info = &si_gpu_max_read_image_args;
		break;

	case 0x100f:  /* CL_DEVICE_MAX_WRITE_IMAGE_ARGS */
		size_ret = 4;
		info = &si_gpu_max_write_image_args;
		break;

	case 0x1010:  /* CL_DEVICE_MAX_MEM_ALLOC_SIZE */
		size_ret = 8;
		info = &si_gpu_max_mem_alloc_size;
		break;

	case 0x1011:  /* CL_DEVICE_IMAGE2D_MAX_WIDTH */
		size_ret = 4;
		info = &si_gpu_image2d_max_width;
		break;

	case 0x1012:  /* CL_DEVICE_IMAGE2D_MAX_HEIGHT */
		size_ret = 4;
		info = &si_gpu_image2d_max_height;
		break;

	case 0x1013:  /* CL_DEVICE_IMAGE3D_MAX_WIDTH */
		size_ret = 4;
		info = &si_gpu_image3d_max_width;
		break;

	case 0x1014:  /* CL_DEVICE_IMAGE3D_MAX_HEIGHT */
		size_ret = 4;
		info = &si_gpu_image3d_max_height;
		break;

	case 0x1015:  /* CL_DEVICE_IMAGE3D_MAX_DEPTH */
		size_ret = 4;
		info = &si_gpu_image3d_max_depth;
		break;

	case 0x1016:  /* CL_DEVICE_IMAGE_SUPPORT */
		size_ret = 4;
		info = &si_gpu_image_support;
		break;

	case 0x1017:  /* CL_DEVICE_MAX_PARAMETER_SIZE */
		size_ret = 4;
		info = &si_gpu_max_parameter_size;
		break;

	case 0x1018:  /* CL_DEVICE_MAX_SAMPLERS */
		size_ret = 4;
		info = &si_gpu_max_samplers;
		break;

	case 0x1019:  /* CL_DEVICE_MEM_BASE_ADDR_ALIGN */
		size_ret = 4;
		info = &si_gpu_mem_base_addr_align;
		break;

	case 0x101a:  /* CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE */
		size_ret = 4;
		info = &si_gpu_min_data_type_align_size;
		break;

	case 0x101b:  /* CL_DEVICE_SINGLE_FP_CONFIG */
		size_ret = 4;
		info = &si_gpu_single_fp_config;
		break;

	case 0x101c:  /* CL_DEVICE_GLOBAL_MEM_CACHE_TYPE */
		size_ret = 4;
		info = &si_gpu_global_mem_cache_type;
		break;

	case 0x101d:  /* CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE */
		size_ret = 4;
		info = &si_gpu_global_mem_cacheline_size;
		break;

	case 0x101e:  /* CL_DEVICE_GLOBAL_MEM_CACHE_SIZE */
		size_ret = 8;
		info = &si_gpu_global_mem_cache_size;
		break;

	case 0x101f:  /* CL_DEVICE_GLOBAL_MEM_SIZE */
		size_ret = 8;
		info = &si_gpu_global_mem_size;
		break;

	case 0x1020:  /* CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE */
		size_ret = 8;
		info = &si_gpu_max_constant_buffer_size;
		break;

	case 0x1021:  /* CL_DEVICE_MAX_CONSTANT_ARGS */
		size_ret = 4;
		info = &si_gpu_max_constant_args;
		break;

	case 0x1022:  /* CL_DEVICE_LOCAL_MEM_TYPE */
		size_ret = 4;
		info = &si_gpu_local_mem_type;
		break;

	case 0x1023:  /* CL_DEVICE_LOCAL_MEM_SIZE */
		size_ret = 8;
		info = &si_gpu_lds_size;
		break;

	case 0x1024:  /* CL_DEVICE_ERROR_CORRECTION_SUPPORT */
		size_ret = 4;
		info = &si_gpu_error_correction_support;
		break;

	case 0x1025:  /* CL_DEVICE_PROFILING_TIMER_RESOLUTION */
		size_ret = 4;
		info = &si_gpu_profiling_timer_resolution;
		break;

	case 0x1026:  /* CL_DEVICE_ENDIAN_LITTLE */
		size_ret = 4;
		info = &si_gpu_endian_little;
		break;

	case 0x1027:  /* CL_DEVICE_AVAILABLE */
		size_ret = 4;
		info = &si_gpu_device_available;
		break;

	case 0x1028:  /* CL_DEVICE_COMPILER_AVAILABLE */
		size_ret = 4;
		info = &si_gpu_compiler_available;
		break;

	case 0x1029:  /* CL_DEVICE_EXECUTION_CAPABILITIES */
		size_ret = 4;
		info = &si_gpu_execution_capabilities;
		break;

	case 0x102a:  /* CL_DEVICE_QUEUE_PROPERTIES */
		size_ret = 4;
		info = &si_gpu_queue_properties;
		break;

	case 0x102b:  /* CL_DEVICE_NAME */
		size_ret = strlen(si_gpu_device_name) + 1;
		info = si_gpu_device_name;
		break;
	
	case 0x102c:  /* CL_DEVICE_VENDOR */
		size_ret = strlen(si_gpu_device_vendor) + 1;
		info = si_gpu_device_vendor;
		break;
	
	case 0x102d:  /* CL_DRIVER_VERSION */
		size_ret = strlen(si_gpu_driver_version) + 1;
		info = si_gpu_driver_version;
		break;

	case 0x102e:  /* CL_DEVICE_PROFILE */
		size_ret = strlen(si_gpu_device_profile) + 1;
		info = si_gpu_device_profile;
		break;
	
	case 0x102f:  /* CL_DEVICE_VERSION */
		size_ret = strlen(si_gpu_device_version) + 1;
		info = si_gpu_device_version;
		break;
	
	case 0x1030:  /* CL_DEVICE_EXTENSIONS */
		size_ret = strlen(si_gpu_device_extensions) + 1;
		info = si_gpu_device_extensions;
		break;

	case 0x1031:  /* CL_DEVICE_PLATFORM */
		size_ret = 4;
		info = &si_gpu_platform;
		break;

	case 0x1032:  /* CL_DEVICE_DOUBLE_FP_CONFIG */
		size_ret = 4;
		info = &si_gpu_double_fp_config;
		break;

	case 0x1034: /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF */
		size_ret = 4;
		info = &si_gpu_vector_width_half;
		break;

	case 0x1035:  /* CL_DEVICE_HOST_UNIFIED_MEMORY */
		size_ret = 4;
		info = &si_gpu_host_unified_memory;
		break;

	case 0x1036:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR */
	case 0x1037:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT */
	case 0x1038:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_INT */
	case 0x1039:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG */
	case 0x103a:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT */
	case 0x103b:  /* CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE */
		size_ret = 4;
		info = &si_gpu_simd_num_simd_lanes;
		break;

	case 0x103c: /* CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF */
		size_ret = 4;
		info = &si_gpu_vector_width_half;
		break;

	case 0x103d:  /* CL_DEVICE_OPENCL_C_VERSION */
		size_ret = strlen(si_gpu_opencl_version) + 1;
		info = si_gpu_opencl_version;
		break;

	case 0x103e:  /* CL_DEVICE_LINKER_AVAILABLE */
		size_ret = 4;
		info = &si_gpu_linker_available;
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

