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


/* Required 'libm2s-opencl.so' version.
 * Increase this number when the user is required to download an updated release
 * of the Multi2Sim OpenCL implementation. */
#define SYS_OPENCL_IMPL_VERSION_MAJOR		1
#define SYS_OPENCL_IMPL_VERSION_MINOR		0
#define SYS_OPENCL_IMPL_VERSION_BUILD		0
#define SYS_OPENCL_IMPL_VERSION			((SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						SYS_OPENCL_IMPL_VERSION_BUILD)


/* Debug info */
int opencl_debug_category;

void opencl_debug_array(int nelem, int *array)
{
	char *comma = "";
	int i;

	opencl_debug("{");
	for (i = 0; i < nelem; i++) {
		opencl_debug("%s%d", comma, array[i]);
		comma = ", ";
	}
	opencl_debug("}");
}


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
	"\tThe Multi2Sim implementation of the OpenCL interface does not support runtime\n"
	"\tcompilation of kernel sources. To run OpenCL kernels, you should first compile\n"
	"\tthem off-line using an Evergreen-compatible target device. Then, you have three\n"
	"\toptions to load them:\n"
	"\t  1) Replace 'clCreateProgramWithSource' calls by 'clCreateProgramWithBinary'\n"
	"\t     in your source files, referencing the pre-compiled kernel.\n"
	"\t  2) Tell Multi2Sim to provide the application with your pre-compiled kernel\n"
	"\t     using command-line option '-opencl:binary'.\n"
	"\t  3) If you are trying to run one of the OpenCL benchmarks provided in the\n"
	"\t     simulator website, option '--load' can be used as a program argument\n"
	"\t     (not a simulator argument). This option allows you to specify the path\n"
	"\t     for the pre-compiled kernel, which is provided in the downloaded package.\n";
	
char *err_opencl_binary_note =
	"\tYou have selected a pre-compiled OpenCL kernel binary to be passed to your\n"
	"\tOpenCL application when it requests a kernel compilation. It is your\n"
	"\tresponsibility to check that the chosen binary corresponds to the kernel\n"
	"\tthat your application is expecting to load.\n";

char *err_opencl_version_note =
	"\tThe version of the Multi2Sim OpenCL library ('libm2s-opencl') that your program\n"
	"\tis using is too old. You need to re-link your program with a version of the\n"
	"\tlibrary compatible for this Multi2Sim release. Please see the Multi2Sim Guide\n"
	"\tfor details (www.multi2sim.org).\n";


/* Error macros */

#define OPENCL_PARAM_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", err_prefix, p, err_opencl_note);
#define OPENCL_PARAM_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_OOR(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	err_prefix, (min), (max), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", err_prefix, err_opencl_param_note); }





/* OpenCL API Implementation */

int opencl_func_run(int code, unsigned int *args)
{
	char err_prefix[MAX_STRING_SIZE];
	char *func_name;
	uint32_t opencl_success = 0;
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
		uint32_t opencl_impl_version = args[3];  /* Implementation-specific */

		uint32_t one = 1;
		int opencl_impl_version_major = opencl_impl_version >> 16;
		int opencl_impl_version_minor = (opencl_impl_version >> 8) & 0xff;
		int opencl_impl_version_build = opencl_impl_version & 0xff;

		/* Check 'libm2s-opencl' version */
		if (opencl_impl_version < SYS_OPENCL_IMPL_VERSION)
			fatal("wrong Multi2Sim OpenCL library version (provided=%d.%d.%d, required=%d.%d.%d).\n%s",
				opencl_impl_version_major, opencl_impl_version_minor, opencl_impl_version_build,
				SYS_OPENCL_IMPL_VERSION_MAJOR, SYS_OPENCL_IMPL_VERSION_MINOR, SYS_OPENCL_IMPL_VERSION_BUILD,
				err_opencl_version_note);
		opencl_debug("  'libm2s-opencl' version: %d.%d.%d\n",
				opencl_impl_version_major, opencl_impl_version_minor, opencl_impl_version_build);
		
		/* Get platform id */
		opencl_debug("  num_entries=%d, platforms=0x%x, num_platforms=0x%x, version=0x%x\n",
			num_entries, platforms, num_platforms, opencl_impl_version);
		if (num_platforms)
			mem_write(isa_mem, num_platforms, 4, &one);
		if (platforms && num_entries > 0)
			mem_write(isa_mem, platforms, 4, &opencl_platform->id);
		break;
	}


	/* 1001 */
	case OPENCL_FUNC_clGetPlatformInfo:
	{
		uint32_t platform_id = args[0];  /* cl_platform_id platform */
		uint32_t param_name = args[1];  /* cl_platform_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_platform_t *platform;
		uint32_t size_ret;

		opencl_debug("  platform=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			platform_id, param_name, param_value_size, param_value, param_value_size_ret);

		platform = opencl_object_get(OPENCL_OBJ_PLATFORM, platform_id);
		size_ret = opencl_platform_get_info(platform, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
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


	/* 1003 */
	case OPENCL_FUNC_clGetDeviceInfo:
	{
		uint32_t device_id = args[0];  /* cl_device_id device */
		uint32_t param_name = args[1];  /* cl_device_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_device_t *device;
		uint32_t size_ret;

		opencl_debug("  device=0x%x, param_name=0x%x, param_value_size=%d\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			device_id, param_name, param_value_size, param_value, param_value_size_ret);

		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		size_ret = opencl_device_get_info(device, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
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

		opencl_debug("  properties=0x%x, num_devices=%d, devices=0x%x\n"
			"pfn_notify=0x%x, user_data=0x%x, errcode_ret=0x%x\n",
			properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);
		OPENCL_PARAM_NOT_SUPPORTED_EQ(devices, 0);

		/* Read device id */
		mem_read(isa_mem, devices, 4, &device_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		if (!device)
			fatal("%s: invalid device\n%s", err_prefix, err_opencl_param_note);

		/* Create context and return id */
		context = opencl_context_create();
		opencl_context_set_properties(context, isa_mem, properties);
		context->device_id = device_id;
		retval = context->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1005 */
	case OPENCL_FUNC_clCreateContextFromType:
	{
		uint32_t properties = args[0];  /* const cl_context_properties *properties */
		uint32_t device_type = args[1];  /* cl_device_type device_type */
		uint32_t pfn_notify = args[2];  /* void (*pfn_notify)(const char *, const void *, size_t , void *) */
		uint32_t user_data = args[3];  /* void *user_data */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		struct opencl_device_t *device;
		struct opencl_context_t *context;

		opencl_debug("  properties=0x%x, device_type=0x%x, pfn_notify=0x%x,\n"
			"  user_data=0x%x, errcode_ret=0x%x\n",
			properties, device_type, pfn_notify, user_data, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);

		/* Get device */
		device = (struct opencl_device_t *) opencl_object_get_type(OPENCL_OBJ_DEVICE);
		assert(device);

		/* Create context */
		context = opencl_context_create();
		context->device_id = device->id;
		opencl_context_set_properties(context, isa_mem, properties);
		retval = context->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);

		break;
	}


	/* 1007 */
	case OPENCL_FUNC_clReleaseContext:
	{
		uint32_t context_id = args[0];  /* cl_context context */

		struct opencl_context_t *context;

		opencl_debug("  context=0x%x\n", context_id);
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		assert(context->ref_count > 0);
		if (!--context->ref_count)
			opencl_context_free(context);
		break;
	}


	/* 1008 */
	case OPENCL_FUNC_clGetContextInfo:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t param_name = args[1];  /* cl_context_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_context_t *context;
		uint32_t size_ret = 0;

		opencl_debug("  context=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			context_id, param_name, param_value_size, param_value, param_value_size_ret);

		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		size_ret = opencl_context_get_info(context, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
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

		opencl_debug("  context=0x%x, device=0x%x, properties=0x%x, errcode_ret=0x%x\n",
			context_id, device_id, properties, errcode_ret);

		/* Check that context and device are valid */
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);

		/* Create command queue and return id */
		command_queue = opencl_command_queue_create();
		command_queue->context_id = context_id;
		command_queue->device_id = device_id;
		command_queue->properties = properties;
		retval = command_queue->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1011 */
	case OPENCL_FUNC_clReleaseCommandQueue:
	{
		uint32_t command_queue_id = args[0];  /* cl_command_queue command_queue */

		struct opencl_command_queue_t *command_queue;

		opencl_debug("  command_queue=0x%x\n", command_queue_id);
		command_queue = opencl_object_get(OPENCL_OBJ_COMMAND_QUEUE, command_queue_id);
		assert(command_queue->ref_count > 0);
		if (!--command_queue->ref_count)
			opencl_command_queue_free(command_queue);
		break;
	}


	/* 1014 */
	case OPENCL_FUNC_clCreateBuffer:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t flags = args[1];  /* cl_mem_flags flags */
		uint32_t size = args[2];  /* size_t size */
		uint32_t host_ptr = args[3];  /* void *host_ptr */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */
		void *buf;

		char sflags[MAX_STRING_SIZE];
		static struct string_map_t create_buffer_flags_map = { 4, {
			{ "CL_MEM_READ_WRITE", 0x1 },
			{ "CL_MEM_WRITE_ONLY", 0x2 },
			{ "CL_MEM_READ_ONLY", 0x4 },
			{ "CL_MEM_USE_HOST_PTR", 0x8 },
			{ "CL_MEM_ALLOC_HOST_PTR", 0x10 },
			{ "CL_MEM_COPY_HOST_PTR", 0x20 }
		}};

		struct opencl_mem_t *mem;

		map_flags(&create_buffer_flags_map, flags, sflags, sizeof(sflags));
		opencl_debug("  context=0x%x, flags=%s, size=%d, host_ptr=0x%x, errcode_ret=0x%x\n",
			context_id, sflags, size, host_ptr, errcode_ret);

		/* Check flags */
		OPENCL_PARAM_NOT_SUPPORTED_FLAG(flags, 0x10, "CL_MEM_ALLOC_HOST_PTR");
		if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
			fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				err_prefix, err_opencl_param_note);
		if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
			fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				err_prefix, err_opencl_param_note);

		/* Create memory object */
		mem = opencl_mem_create();
		mem->size = size;
		mem->flags = flags;
		mem->host_ptr = host_ptr;

		/* Assign position in device global memory */
		mem->device_ptr = gk->global_mem_top;
		gk->global_mem_top += size;

		/* If 'host_ptr' was specified, copy buffer into device memory */
		if (host_ptr) {
			buf = malloc(size);
			if (!buf)
				fatal("%s: out of memory", err_prefix);
			mem_read(isa_mem, host_ptr, size, buf);
			mem_write(gk->global_mem, mem->device_ptr, size, buf);
			free(buf);
		}

		/* Return memory object */
		retval = mem->id;
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1019 */
	case OPENCL_FUNC_clReleaseMemObject:
	{
		uint32_t mem_id = args[0];  /* cl_mem memobj */

		struct opencl_mem_t *mem;

		opencl_debug("  memobj=0x%x\n", mem_id);
		mem = opencl_object_get(OPENCL_OBJ_MEM, mem_id);
		assert(mem->ref_count > 0);
		if (!--mem->ref_count)
			opencl_mem_free(mem);
		break;
	}


	/* 1028 */
	case OPENCL_FUNC_clCreateProgramWithSource:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t count = args[1];  /* cl_uint count */
		uint32_t strings = args[2];  /* const char **strings */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		struct opencl_context_t *context;
		struct opencl_program_t *program;
		void *buf;
		int buf_size;

		opencl_debug("  context=0x%x, count=%d, strings=0x%x, lengths=0x%x, errcode_ret=0x%x\n",
			context_id, count, strings, lengths, errcode_ret);

		/* Application tries to compile source, and no binary was passed to Multi2Sim */
		if (!*gk_opencl_binary_name)
			fatal("%s: kernel source compilation not supported.\n%s",
				err_prefix, err_opencl_compiler);

		/* Create program */
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		program = opencl_program_create();
		retval = program->id;
		warning("%s: binary '%s' used as pre-compiled kernel.\n%s",
			err_prefix, gk_opencl_binary_name, err_opencl_binary_note);

		/* Load OpenCL binary passed to Multi2Sim and make a copy in temporary file */
		program->binary_file = create_temp_file(program->binary_file_name, MAX_PATH_SIZE);
		buf = read_buffer(gk_opencl_binary_name, &buf_size);
		if (!buf)
			fatal("%s: cannot read from file '%s'", err_prefix, gk_opencl_binary_name);
		write_buffer(program->binary_file_name, buf, buf_size);
		free_buffer(buf);
		break;
	}


	/* 1029 */
	case OPENCL_FUNC_clCreateProgramWithBinary:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t device_list = args[2];  /* const cl_device_id *device_list */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t binaries = args[4];  /* const unsigned char **binaries */
		uint32_t binary_status = args[5];  /* cl_int *binary_status */
		uint32_t errcode_ret = args[6];  /* cl_int *errcode_ret */

		uint32_t length, binary;
		uint32_t device_id;
		struct opencl_context_t *context;
		struct opencl_device_t *device;
		struct opencl_program_t *program;
		void *buf;

		opencl_debug("  context=0x%x, num_devices=%d, device_list=0x%x, lengths=0x%x\n"
			"  binaries=0x%x, binary_status=0x%x, errcode_ret=0x%x\n",
			context_id, num_devices, device_list, lengths, binaries,
			binary_status, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);

		/* Get device and context */
		mem_read(isa_mem, device_list, 4, &device_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);

		/* Create program */
		program = opencl_program_create();
		retval = program->id;

		/* Read binary length and pointer */
		mem_read(isa_mem, lengths, 4, &length);
		mem_read(isa_mem, binaries, 4, &binary);
		opencl_debug("    lengths[0] = %d\n", length);
		opencl_debug("    binaries[0] = 0x%x\n", binary);

		/* Read binary */
		buf = malloc(length);
		assert(buf);
		mem_read(isa_mem, binary, length, buf);

		/* Create program temporary file and copy binary */
		program->binary_file = create_temp_file(program->binary_file_name, MAX_PATH_SIZE);
		write_buffer(program->binary_file_name, buf, length);
		free(buf);

		/* Return success */
		if (binary_status)
			mem_write(isa_mem, binary_status, 4, &opencl_success);
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1031 */
	case OPENCL_FUNC_clReleaseProgram:
	{
		uint32_t program_id = args[0];  /* cl_program program */
		
		struct opencl_program_t *program;

		opencl_debug("  program=0x%x\n", program_id);
		program = opencl_object_get(OPENCL_OBJ_PROGRAM, program_id);
		assert(program->ref_count > 0);
		if (!--program->ref_count)
			opencl_program_free(program);
		break;
	}


	/* 1032 */
	case OPENCL_FUNC_clBuildProgram:
	{
		uint32_t program_id = args[0];  /* cl_program program */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t device_list = args[2];  /* const cl_device_id *device_list */
		uint32_t options = args[3];  /* const char *options */
		uint32_t pfn_notify = args[4];  /* void (CL_CALLBACK *pfn_notify)(cl_program program,
							void *user_data) */
		uint32_t user_data = args[5];  /* void *user_data */

		struct opencl_program_t *program;

		opencl_debug("  program=0x%x, num_devices=%d, device_list=0x%x, options=0x%x\n"
			"  pfn_notify=0x%x, user_data=0x%x\n",
			program_id, num_devices, device_list, options, pfn_notify, user_data);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(user_data, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(options, 0);

		/* Get program */
		program = opencl_object_get(OPENCL_OBJ_PROGRAM, program_id);
		if (!program->binary_file)
			fatal("%s: program binary must be loaded first.\n%s",
				err_prefix, err_opencl_param_note);

		/* Build program */
		opencl_program_build(program);
		break;
	}


	/* 1036 */
	case OPENCL_FUNC_clCreateKernel:
	{
		uint32_t program_id = args[0];  /* cl_program program */
		uint32_t kernel_name = args[1];  /* const char *kernel_name */
		uint32_t errcode_ret = args[2];  /* cl_int *errcode_ret */

		char kernel_name_str[MAX_STRING_SIZE];
		struct opencl_kernel_t *kernel;
		struct opencl_program_t *program;

		opencl_debug("  program=0x%x, kernel_name=0x%x, errcode_ret=0x%x\n",
			program_id, kernel_name, errcode_ret);
		if (mem_read_string(isa_mem, kernel_name, MAX_STRING_SIZE, kernel_name_str) == MAX_STRING_SIZE)
			fatal("%s: 'kernel_name' string is too long", err_prefix);
		opencl_debug("    kernel_name='%s'\n", kernel_name_str);

		/* Get program */
		program = opencl_object_get(OPENCL_OBJ_PROGRAM, program_id);

		/* Create the kernel */
		kernel = opencl_kernel_create();
		kernel->program_id = program_id;

		/* Program must be built */
		if (!program->binary_file_elf)
			fatal("%s: program should be first built with clBuildProgram.\n%s",
				err_prefix, err_opencl_param_note);

		/* Load kernel */
		opencl_kernel_load(kernel, kernel_name_str);

		/* Return kernel id */
		retval = kernel->id;
		break;
	}


	/* 1039 */
	case OPENCL_FUNC_clReleaseKernel:
	{
		uint32_t kernel_id = args[0];  /* cl_kernel kernel */

		struct opencl_kernel_t *kernel;

		opencl_debug("  kernel=0x%x\n", kernel_id);
		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		assert(kernel->ref_count > 0);
		if (!--kernel->ref_count)
			opencl_kernel_free(kernel);
		break;
	}


	/* 1040 */
	case OPENCL_FUNC_clSetKernelArg:
	{
		uint32_t kernel_id = args[0];  /* cl_kernel kernel */
		uint32_t arg_index = args[1];  /* cl_uint arg_index */
		uint32_t arg_size = args[2];  /* size_t arg_size */
		uint32_t arg_value = args[3];  /* const void *arg_value */

		struct opencl_kernel_t *kernel;
		struct opencl_kernel_arg_t *arg;

		opencl_debug("  kernel_id=0x%x, arg_index=%d, arg_size=%d, arg_value=0x%x\n",
			kernel_id, arg_index, arg_size, arg_value);

		/* Check */
		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		if (arg_value)
			OPENCL_PARAM_NOT_SUPPORTED_NEQ(arg_size, 4);
		if (arg_index >= list_count(kernel->arg_list))
			fatal("%s: argument index out of bounds.\n%s", err_prefix,
				err_opencl_param_note);

		/* Copy to kernel object */
		arg = list_get(kernel->arg_list, arg_index);
		assert(arg);
		arg->set = 1;
		arg->size = arg_size;
		if (arg_value)
			mem_read(isa_mem, arg_value, 4, &arg->value);

		/* If OpenCL argument scope is __local, argument value must be NULL */
		if (arg->mem_scope == OPENCL_MEM_SCOPE_LOCAL && arg_value)
			fatal("%s: value for local arguments must be NULL.\n%s", err_prefix,
				err_opencl_param_note);

		/* Return success */
		break;
	}


	/* 1042 */
	case OPENCL_FUNC_clGetKernelWorkGroupInfo:
	{
		uint32_t kernel_id = args[0];  /* cl_kernel kernel */
		uint32_t device_id = args[1];  /* cl_device_id device */
		uint32_t param_name = args[2];  /* cl_kernel_work_group_info param_name */
		uint32_t param_value_size = args[3];  /* size_t param_value_size */
		uint32_t param_value = args[4];  /* void *param_value */
		uint32_t param_value_size_ret = args[5];  /* size_t *param_value_size_ret */
		
		struct opencl_kernel_t *kernel;
		struct opencl_device_t *device;
		uint32_t size_ret;

		opencl_debug("  kernel=0x%x, device=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			kernel_id, device_id, param_name, param_value_size, param_value,
			param_value_size_ret);

		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		size_ret = opencl_kernel_get_work_group_info(kernel, param_name, isa_mem,
			param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
		break;
	}


	/* 1043 */
	case OPENCL_FUNC_clWaitForEvents:
	{
		uint32_t num_events = args[0];  /* cl_uint num_events */
		uint32_t event_list = args[1];  /* const cl_event *event_list */

		opencl_debug("  num_events=0x%x, event_list=0x%x\n",
			num_events, event_list);
		/* FIXME: block until events in list are completed */
		break;
	}


	/* 1047 */
	case OPENCL_FUNC_clReleaseEvent:
	{
		uint32_t event_id = args[0];  /* cl_event event */

		struct opencl_event_t *event;

		opencl_debug("  event=0x%x\n", event_id);
		event = opencl_object_get(OPENCL_OBJ_EVENT, event_id);
		assert(event->ref_count > 0);
		if (!--event->ref_count)
			opencl_event_free(event);
		break;
	}


	/* 1050 */
	case OPENCL_FUNC_clGetEventProfilingInfo:
	{
		uint32_t event_id = args[0];  /* cl_event event */
		uint32_t param_name = args[1];  /* cl_profiling_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_event_t *event;
		int size_ret;

		opencl_debug("  event=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			event_id, param_name, param_value_size, param_value,
			param_value_size_ret);
		event = opencl_object_get(OPENCL_OBJ_EVENT, event_id);
		size_ret = opencl_event_get_profiling_info(event, param_name, isa_mem,
			param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
		break;
	}


	/* 1052 */
	case OPENCL_FUNC_clFinish:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */

		opencl_debug("  command_queue=0x%x\n", command_queue);
		/* FIXME: block until command queue empty */
		break;
	}


	/* 1053 */
	case OPENCL_FUNC_clEnqueueReadBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_read = args[2];  /* cl_bool blocking_read */
		uint32_t offset = args[3];  /* size_t offset */
		uint32_t cb = args[4];  /* size_t cb */
		uint32_t ptr = args[5];  /* void *ptr */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event_ptr = args[8];  /* cl_event *event */
		
		struct opencl_mem_t *mem;
		struct opencl_event_t *event;
		void *buf;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_read=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			command_queue, buffer, blocking_read, offset, cb, ptr,
			num_events_in_wait_list, event_wait_list, event_ptr);

		/* FIXME: 'blocking_read' ignored */
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Check that device buffer storage is not exceeded */
		if (offset + cb > mem->size)
			fatal("%s: buffer storage exceeded\n%s", err_prefix, err_opencl_param_note);

		/* Copy buffer from device memory to host memory */
		buf = malloc(cb);
		assert(buf);
		mem_read(gk->global_mem, mem->device_ptr + offset, cb, buf);
		mem_write(isa_mem, ptr, cb, buf);
		free(buf);

		/* Event */
		if (event_ptr) {
			event = opencl_event_create(OPENCL_EVENT_NDRANGE_KERNEL);
			event->status = OPENCL_EVENT_STATUS_SUBMITTED;
			event->time_queued = opencl_event_timer();
			event->time_submit = opencl_event_timer();
			event->time_start = opencl_event_timer();
			event->time_end = opencl_event_timer();
			mem_write(isa_mem, event_ptr, 4, &event->id);
			opencl_debug("    event: 0x%x\n", event->id);
		}

		/* Return success */
		opencl_debug("    %d bytes copied from device memory (0x%x) to host memory (0x%x)\n",
			cb, mem->device_ptr + offset, ptr);
		break;
	}


	/* 1055 */
	case OPENCL_FUNC_clEnqueueWriteBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_write = args[2];  /* cl_bool blocking_write */
		uint32_t offset = args[3];  /* size_t offset */
		uint32_t cb = args[4];  /* size_t cb */
		uint32_t ptr = args[5];  /* const void *ptr */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event_ptr = args[8];  /* cl_event *event */

		struct opencl_mem_t *mem;
		struct opencl_event_t *event;
		void *buf;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_write=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			command_queue, buffer, blocking_write, offset, cb,
			ptr, num_events_in_wait_list, event_wait_list, event_ptr);

		/* FIXME: 'blocking_write' ignored */
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Check that device buffer storage is not exceeded */
		if (offset + cb > mem->size)
			fatal("%s: buffer storage exceeded\n%s", err_prefix, err_opencl_param_note);

		/* Copy buffer from host memory to device memory */
		buf = malloc(cb);
		assert(buf);
		mem_read(isa_mem, ptr, cb, buf);
		mem_write(gk->global_mem, mem->device_ptr + offset, cb, buf);
		free(buf);

		/* Event */
		if (event_ptr) {
			event = opencl_event_create(OPENCL_EVENT_MAP_BUFFER);
			event->status = OPENCL_EVENT_STATUS_COMPLETE;
			event->time_queued = opencl_event_timer();
			event->time_submit = opencl_event_timer();
			event->time_start = opencl_event_timer();
			event->time_end = opencl_event_timer();  /* FIXME: change for asynchronous exec */
			mem_write(isa_mem, event_ptr, 4, &event->id);
			opencl_debug("    event: 0x%x\n", event->id);
		}

		/* Return success */
		opencl_debug("    %d bytes copied from host memory (0x%x) to device memory (0x%x)\n",
			cb, ptr, mem->device_ptr + offset);
		break;
	}


	/* 1064 */
	case OPENCL_FUNC_clEnqueueMapBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_map = args[2];  /* cl_bool blocking_map */
		uint32_t map_flags = args[3];  /* cl_map_flags map_flags */
		uint32_t offset = args[4];  /* size_t offset */
		uint32_t cb = args[5];  /* size_t cb */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event_ptr = args[8];  /* cl_event *event */
		uint32_t errcode_ret = args[9];  /* cl_int *errcode_ret */

		struct opencl_mem_t *mem;
		struct opencl_event_t *event;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_map=0x%x, map_flags=0x%x,\n"
			"  offset=0x%x, cb=0x%x, num_events_in_wait_list=0x%x, event_wait_list=0x%x,\n"
			"  event=0x%x, errcode_ret=0x%x\n",
			command_queue, buffer, blocking_map, map_flags, offset, cb,
			num_events_in_wait_list, event_wait_list, event_ptr, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_EQ(blocking_map, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Event */
		if (event_ptr) {
			event = opencl_event_create(OPENCL_EVENT_MAP_BUFFER);
			event->status = OPENCL_EVENT_STATUS_COMPLETE;
			event->time_queued = opencl_event_timer();
			event->time_submit = opencl_event_timer();
			event->time_start = opencl_event_timer();
			event->time_end = opencl_event_timer();  /* FIXME: change for asynchronous exec */
			mem_write(isa_mem, event_ptr, 4, &event->id);
			opencl_debug("    event: 0x%x\n", event->id);
		}

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		fatal("clEnqueueMapBuffer: not implemented");
		break;
	}


	/* 1067 */
	case OPENCL_FUNC_clEnqueueNDRangeKernel:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t kernel_id = args[1];  /* cl_kernel kernel */
		uint32_t work_dim = args[2];  /* cl_uint work_dim */
		uint32_t global_work_offset_ptr = args[3];  /* const size_t *global_work_offset */
		uint32_t global_work_size_ptr = args[4];  /* const size_t *global_work_size */
		uint32_t local_work_size_ptr = args[5];  /* const size_t *local_work_size */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event_ptr = args[8];  /* cl_event *event */

		struct opencl_kernel_t *kernel;
		struct opencl_event_t *event;
		int i;

		opencl_debug("  command_queue=0x%x, kernel=0x%x, work_dim=%d,\n"
			"  global_work_offset=0x%x, global_work_size_ptr=0x%x, local_work_size_ptr=0x%x,\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			command_queue, kernel_id, work_dim, global_work_offset_ptr, global_work_size_ptr,
			local_work_size_ptr, num_events_in_wait_list, event_wait_list, event_ptr);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(global_work_offset_ptr, 0);
		OPENCL_PARAM_NOT_SUPPORTED_OOR(work_dim, 1, 3);
		if (num_events_in_wait_list || event_wait_list)
			warning("%s: event list arguments ignored", err_prefix);

		/* Get kernel */
		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		kernel->work_dim = work_dim;

		/* Global work sizes */
		kernel->global_size3[1] = 1;
		kernel->global_size3[2] = 1;
		for (i = 0; i < work_dim; i++)
			mem_read(isa_mem, global_work_size_ptr + i * 4, 4, &kernel->global_size3[i]);
		kernel->global_size = kernel->global_size3[0] * kernel->global_size3[1] * kernel->global_size3[2];
		opencl_debug("    global_work_size=");
		opencl_debug_array(work_dim, kernel->global_size3);
		opencl_debug("\n");

		/* Local work sizes.
		 * If no pointer provided, assign the same as global size - FIXME: can be done better. */
		memcpy(kernel->local_size3, kernel->global_size3, 12);
		if (local_work_size_ptr)
			for (i = 0; i < work_dim; i++)
				mem_read(isa_mem, local_work_size_ptr + i * 4, 4, &kernel->local_size3[i]);
		kernel->local_size = kernel->local_size3[0] * kernel->local_size3[1] * kernel->local_size3[2];
		opencl_debug("    local_work_size=");
		opencl_debug_array(work_dim, kernel->local_size3);
		opencl_debug("\n");

		/* Check divisibility of global by local sizes */
		if ((kernel->global_size3[0] % kernel->local_size3[0])
			|| (kernel->global_size3[1] % kernel->local_size3[1])
			|| (kernel->global_size3[2] % kernel->local_size3[2]))
			fatal("%s: global work sizes must be multiples of local sizes.\n%s",
				err_prefix, err_opencl_param_note);

		/* Calculate number of groups */
		for (i = 0; i < 3; i++)
			kernel->group_count3[i] = kernel->global_size3[i] / kernel->local_size3[i];
		kernel->group_count = kernel->group_count3[0] * kernel->group_count3[1] * kernel->group_count3[2];
		opencl_debug("    group_count=");
		opencl_debug_array(work_dim, kernel->group_count3);
		opencl_debug("\n");

		/* Event */
		if (event_ptr) {
			event = opencl_event_create(OPENCL_EVENT_NDRANGE_KERNEL);
			event->status = OPENCL_EVENT_STATUS_SUBMITTED;
			event->time_queued = opencl_event_timer();
			event->time_submit = opencl_event_timer();
			event->time_start = opencl_event_timer();  /* FIXME: change for asynchronous exec */
			mem_write(isa_mem, event_ptr, 4, &event->id);
			opencl_debug("    event: 0x%x\n", event->id);
		}

		/* FIXME: asynchronous execution */
		gpu_isa_run(kernel);

		/* Event */
		if (event_ptr) {
			event->status = OPENCL_EVENT_STATUS_COMPLETE;
			event->time_end = opencl_event_timer();  /* FIXME: change for asynchronous exec */
		}
		break;
	}



	default:
		
		fatal("opencl_func_run: function '%s' (code=%d) not implemented.\n%s",
			func_name, code, err_opencl_note);
	}

	return retval;
}

