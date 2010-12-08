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

#include <gpukernel-local.h>
#include <m2skernel.h>
#include <assert.h>
#include <debug.h>


/* Debug info */
int opencl_debug_category;


/* List of OpenCL function names */
char *opencl_func_names[] = {
#define DEF_OPENCL_FUNC(_name, _argc) #_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	""
};


/* Number of arguments for each OpenCL function */
int opencl_func_argc[] = {
#define DEF_OPENCL_FUNC(_name, _argc) _argc,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	0
};


/* Error messages */
char *err_opencl_note =
	"\tThe OpenCL interface is implemented in library 'm2s-opencl.so' as a set of\n"
	"\tsystem calls, intercepted by Multi2Sim and emulated in 'opencl.c'.\n"
	"\tHowever, only a subset of this interface is currently implemented in the simulator.\n"
	"\tTo request the implementation of a specific OpenCL call, please email\n"
	"\t'development@multi2sim.org'.\n";

char *err_opencl_param_note =
	"\tNote that a real OpenCL implementation would return an error code to the\n"
	"\tcalling program. However, the purpose of this OpenCL implementation is to\n"
	"\tsupport correctly written programs. Thus, a detailed implementation of OpenCL\n"
	"\terror handling is not provided, and any OpenCL error will cause the\n"
	"\tsimulation to stop.\n";

char *err_opencl_compiler =
	"\tThe current implementation of OpenCL does not support the compilation of\n"
	"\tkernels at run time. Instead, the OpenCL kernels should be pre-compiled for\n"
	"\tan Evergreen compatible target device, and loaded with the clCreateProgramWithBinary\n"
	"\tcall.\n";


/* OpenCL Error Codes,
 * as implemented in ATI SDK */
#define CL_SUCCESS                                   0
#define CL_DEVICE_NOT_FOUND                         -1
#define CL_DEVICE_NOT_AVAILABLE                     -2
#define CL_COMPILER_NOT_AVAILABLE                   -3
#define CL_MEM_OBJECT_ALLOCATION_FAILURE            -4
#define CL_OUT_OF_RESOURCES                         -5
#define CL_OUT_OF_HOST_MEMORY                       -6
#define CL_PROFILING_INFO_NOT_AVAILABLE             -7
#define CL_MEM_COPY_OVERLAP                         -8
#define CL_IMAGE_FORMAT_MISMATCH                    -9
#define CL_IMAGE_FORMAT_NOT_SUPPORTED               -10
#define CL_BUILD_PROGRAM_FAILURE                    -11
#define CL_MAP_FAILURE                              -12
#define CL_MISALIGNED_SUB_BUFFER_OFFSET             -13
#define CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST -14
#define CL_INVALID_VALUE                            -30
#define CL_INVALID_DEVICE_TYPE                      -31
#define CL_INVALID_PLATFORM                         -32
#define CL_INVALID_DEVICE                           -33
#define CL_INVALID_CONTEXT                          -34
#define CL_INVALID_QUEUE_PROPERTIES                 -35
#define CL_INVALID_COMMAND_QUEUE                    -36
#define CL_INVALID_HOST_PTR                         -37
#define CL_INVALID_MEM_OBJECT                       -38
#define CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          -39
#define CL_INVALID_IMAGE_SIZE                       -40
#define CL_INVALID_SAMPLER                          -41
#define CL_INVALID_BINARY                           -42
#define CL_INVALID_BUILD_OPTIONS                    -43
#define CL_INVALID_PROGRAM                          -44
#define CL_INVALID_PROGRAM_EXECUTABLE               -45
#define CL_INVALID_KERNEL_NAME                      -46
#define CL_INVALID_KERNEL_DEFINITION                -47
#define CL_INVALID_KERNEL                           -48
#define CL_INVALID_ARG_INDEX                        -49
#define CL_INVALID_ARG_VALUE                        -50
#define CL_INVALID_ARG_SIZE                         -51
#define CL_INVALID_KERNEL_ARGS                      -52
#define CL_INVALID_WORK_DIMENSION                   -53
#define CL_INVALID_WORK_GROUP_SIZE                  -54
#define CL_INVALID_WORK_ITEM_SIZE                   -55
#define CL_INVALID_GLOBAL_OFFSET                    -56
#define CL_INVALID_EVENT_WAIT_LIST                  -57
#define CL_INVALID_EVENT                            -58
#define CL_INVALID_OPERATION                        -59
#define CL_INVALID_GL_OBJECT                        -60
#define CL_INVALID_BUFFER_SIZE                      -61
#define CL_INVALID_MIP_LEVEL                        -62
#define CL_INVALID_GLOBAL_WORK_SIZE                 -63
#define CL_INVALID_PROPERTY                         -64

/* Constants for cl_device_type */
#define CL_DEVICE_TYPE_DEFAULT                      (1 << 0)
#define CL_DEVICE_TYPE_CPU                          (1 << 1)
#define CL_DEVICE_TYPE_GPU                          (1 << 2)
#define CL_DEVICE_TYPE_ACCELERATOR                  (1 << 3)
#define CL_DEVICE_TYPE_ALL                          0xFFFFFFFF




/* OpenCL API Implementation */

int opencl_func_run(int code, unsigned int *args)
{
	char err_prefix[MAX_STRING_SIZE];
	char *func_name;
	int func_code, func_argc;
	int retval = 0;
	
	/* Decode OpenCL function */
	assert(code >= OPENCL_FUNC_FIRST && code <= OPENCL_FUNC_LAST);
	func_code = code - OPENCL_FUNC_FIRST;
	func_name = opencl_func_names[func_code];
	func_argc = opencl_func_argc[func_code];
	assert(func_argc <= OPENCL_MAX_ARGS);
	snprintf(err_prefix, MAX_STRING_SIZE, "OpenCL call '%s'", func_name);
	
	/* Execute */
	opencl_debug("%s\n", func_name);
	switch (func_code) {

	/* 1000 */
	case OPENCL_FUNC_clGetPlatformIDs:
	{
		int num_entries = args[0];  /* cl_uint num_entries */
		uint32_t platforms = args[1];  /* cl_platform_id *platforms */
		uint32_t num_platforms = args[2];  /* cl_uint *num_platforms */
		uint32_t one = 1;
		
		opencl_debug("  num_entries=%d, platforms=0x%x, num_platforms=0x%x\n",
			num_entries, platforms, num_platforms);
		if (num_platforms)
			mem_write(isa_mem, num_platforms, 4, &one);
		if (platforms && num_entries > 0)
			mem_write(isa_mem, platforms, 4, &opencl_platform->id);
		break;
	}


	/* 1002 */
	case OPENCL_FUNC_clGetDeviceIDs:
	{
		uint32_t platform = args[0];  /* cl_platform_id platform */
		int device_type = args[1];  /* cl_device_type device_type */
		int num_entries = args[2];  /* cl_uint num_entries */
		uint32_t devices = args[3];  /* cl_device_id *devices */
		uint32_t num_devices = args[4];  /* cl_uint *num_devices */
		uint32_t one = 1;
		struct opencl_device_t *device;

		opencl_debug("  platform=0x%x, device_type=%d, num_entries=%d\n",
			platform, device_type, num_entries);
		opencl_debug("  devices=0x%x, num_devices=%x\n",
			devices, num_devices);
		if (platform != opencl_platform->id)
			fatal("%s: invalid platform\n%s", err_prefix, err_opencl_param_note);
		if (device_type != CL_DEVICE_TYPE_GPU)
			fatal("%s: device_type only supported for CL_DEVICE_TYPE_GPU\n%s",
				err_prefix, err_opencl_note);
		
		/* Return 1 in 'num_devices' */
		if (num_devices)
			mem_write(isa_mem, num_devices, 4, &one);

		/* Return 'id' of the only existing device */
		if (devices && num_entries > 0) {
			if (!(device = (struct opencl_device_t *) opencl_object_get_type(OPENCL_OBJ_DEVICE)))
				panic("%s: no device", err_prefix);
			mem_write(isa_mem, devices, 4, &device->id);
		}
		break;
	}


	/* 1004 */
	case OPENCL_FUNC_clCreateContext:
	{
		uint32_t properties = args[0];  /* const cl_context_properties *properties */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t devices = args[2];  /* const cl_device_id *devices */
		uint32_t pfn_notify = args[3];  /* void (CL_CALLBACK *pfn_notify)(const char *errinfo,
						   const void *private_info, size_t cb, void *user_data) */
		uint32_t user_data = args[4];  /* void *user_data */
		uint32_t errcode_ret = args[5];  /* cl_int *errcode_ret */

		struct opencl_device_t *device;
		uint32_t device_id;
		struct opencl_context_t *context;
		uint32_t zero = 0;

		opencl_debug("  properties=0x%x, num_devices=%d, devices=0x%x\n"
			"pfn_notify=0x%x, user_data=0x%x, errcode_ret=0x%x\n",
			properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
		if (properties)
			fatal("%s: properties != NULL not supported\n%s", err_prefix, err_opencl_note);
		if (pfn_notify)
			fatal("%s: pfn_notify != NULL not supported\n%s", err_prefix, err_opencl_note);
		if (num_devices != 1)
			fatal("%s: num_devices != 1 not supportes\n%s", err_prefix, err_opencl_note);

		/* Read device id */
		if (!devices)
			fatal("%s: decives must be other than NULL\n%s", err_prefix, err_opencl_param_note);
		mem_read(isa_mem, devices, 4, &device_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		if (!device)
			fatal("%s: invalid device\n%s", err_prefix, err_opencl_param_note);

		/* Create context and return id */
		context = opencl_context_create();
		context->device_id = device_id;
		retval = context->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &zero);  /* CL_SUCCESS */
		break;
	}


	/* 1009 */
	case OPENCL_FUNC_clCreateCommandQueue:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t device_id = args[1];  /* cl_device_id device */
		uint32_t properties = args[2];  /* cl_command_queue_properties properties */
		uint32_t errcode_ret = args[3];  /* cl_int *errcode_ret */

		struct opencl_context_t *context;
		struct opencl_device_t *device;
		struct opencl_command_queue_t *command_queue;
		uint32_t zero = 0;

		opencl_debug("  context=0x%x, device=0x%x, properties=0x%x, errcode_ret=0x%x\n",
			context_id, device_id, properties, errcode_ret);

		/* Unsupported parameter combinations */
		if (properties)
			fatal("%s: properties != NULL not supported\n%s", err_prefix, err_opencl_note);

		/* Check that context and device are valid */
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		if (!context || !device)
			fatal("%s: context or device not valid.\n%s", err_prefix, err_opencl_param_note);

		/* Create command queue and return id */
		command_queue = opencl_command_queue_create();
		command_queue->context_id = context_id;
		command_queue->device_id = device_id;
		retval = command_queue->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &zero);
		break;
	}


	/* 1028 */
	case OPENCL_FUNC_clCreateProgramWithSource:
	{
		uint32_t context = args[0];  /* cl_context context */
		uint32_t count = args[1];  /* cl_uint count */
		uint32_t strings = args[2];  /* const char **strings */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		opencl_debug("  context=0x%x, count=%d, strings=0x%x, lengths=0x%x, errcode_ret=0x%x\n",
			context, count, strings, lengths, errcode_ret);
		fatal("%s: OpenCL source compilation not supported.\n%s",
			err_prefix, err_opencl_compiler);
		break;
	}


	/* 1029 */
	case OPENCL_FUNC_clCreateProgramWithBinary:
	{
		uint32_t context = args[0];  /* cl_context context */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t device_list = args[2];  /* const cl_device_id *device_list */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t binaries = args[4];  /* const unsigned char **binaries */
		uint32_t binary_status = args[5];  /* cl_int *binary_status */
		uint32_t errcode_ret = args[6];  /* cl_int *errcode_ret */

		uint32_t zero = 0;
		uint32_t length;

		opencl_debug("  context=0x%x, num_devices=%d, device_list=0x%x, lengths=0x%x\n"
			"  binaries=0x%x, binary_status=0x%x, errcode_ret=0x%x\n",
			context, num_devices, device_list, lengths, binaries,
			binary_status, errcode_ret);

		/* Check number of devices */
		if (num_devices != 1)
			fatal("%s: only supported for 'num_devices' = 1\n%s",
				err_prefix, err_opencl_note);

		/* Read binary length and code */
		mem_read(isa_mem, lengths, 4, &length);
		opencl_debug("    lengths[0] = %d\n", length);

		/* Return success */
		if (binary_status)
			mem_write(isa_mem, binary_status, 4, &zero);
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &zero);
		fatal("not implemented");
		break;
	}


	default:
		
		fatal("opencl_func_run: function '%s' (code=%d) not implemented.\n%s",
			func_name, code, err_opencl_note);
	}

	return retval;
}

