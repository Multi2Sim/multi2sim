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


#include <arch/evergreen/emu/emu.h>
#include <arch/evergreen/emu/ndrange.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/regs.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "command-queue.h"
#include "context.h"
#include "device.h"
#include "event.h"
#include "kernel.h"
#include "mem.h"
#include "opencl.h"
#include "platform.h"
#include "program.h"
#include "repo.h"
#include "sampler.h"


/* Required 'libm2s-opencl.so' version.
 * Increase this number when the user is required to download an updated release
 * of the Multi2Sim OpenCL implementation. */
#define EVG_SYS_OPENCL_IMPL_VERSION_MAJOR	1
#define EVG_SYS_OPENCL_IMPL_VERSION_MINOR	0
#define EVG_SYS_OPENCL_IMPL_VERSION_BUILD	0
#define EVG_SYS_OPENCL_IMPL_VERSION		((EVG_SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(EVG_SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						EVG_SYS_OPENCL_IMPL_VERSION_BUILD)


/* Debug info */
int evg_opencl_debug_category;

void evg_opencl_debug_array(int nelem, int *array)
{
	char *comma = "";
	int i;

	evg_opencl_debug("{");
	for (i = 0; i < nelem; i++)
	{
		evg_opencl_debug("%s%d", comma, array[i]);
		comma = ", ";
	}
	evg_opencl_debug("}");
}


/* List of OpenCL function names */
char *evg_opencl_func_name[] = {
#define DEF_OPENCL_FUNC(_name, _argc) #_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	""
};


/* Number of arguments for each OpenCL function */
int evg_opencl_func_argc[] = {
#define DEF_OPENCL_FUNC(_name, _argc) _argc,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	0
};


/* Forward declarations of OpenCL functions */
#define DEF_OPENCL_FUNC(_name, _argc) int evg_opencl_##_name##_impl(X86Context *ctx, int *argv);
#include "opencl.dat"
#undef DEF_OPENCL_FUNC


/* Table of OpenCL function implementations */
typedef int (*evg_opencl_func_impl_t)(X86Context *ctx, int *argv);
evg_opencl_func_impl_t evg_opencl_func_impl[] = {
#define DEF_OPENCL_FUNC(_name, _argc) evg_opencl_##_name##_impl,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	NULL
};


/* Error messages */
char *evg_err_opencl_note =
	"\tThe OpenCL interface is implemented in library 'm2s-opencl.so' as a set of\n"
	"\tsystem calls, intercepted by Multi2Sim and emulated in 'opencl.c'.\n"
	"\tHowever, only a subset of this interface is currently implemented in the simulator.\n"
	"\tTo request the implementation of a specific OpenCL call, please email\n"
	"\t'development@multi2sim.org'.\n";

char *evg_err_opencl_param_note =
	"\tNote that a real OpenCL implementation would return an error code to the\n"
	"\tcalling program. However, the purpose of this OpenCL implementation is to\n"
	"\tsupport correctly written programs. Thus, a detailed implementation of OpenCL\n"
	"\terror handling is not provided, and any OpenCL error will cause the\n"
	"\tsimulation to stop.\n";

char *evg_err_opencl_compiler =
	"\tThe Multi2Sim implementation of the OpenCL interface does not support runtime\n"
	"\tcompilation of kernel sources. To run OpenCL kernels, you should first compile\n"
	"\tthem off-line using an Evergreen-compatible target device. Then, you have three\n"
	"\toptions to load them:\n"
	"\t  1) Replace 'clCreateProgramWithSource' calls by 'clCreateProgramWithBinary'\n"
	"\t     in your source files, referencing the pre-compiled kernel.\n"
	"\t  2) Tell Multi2Sim to provide the application with your pre-compiled kernel\n"
	"\t     using command-line option '--evg-kernel-binary'.\n"
	"\t  3) If you are trying to run one of the OpenCL benchmarks provided in the\n"
	"\t     simulator website, option '--load' can be used as a program argument\n"
	"\t     (not a simulator argument). This option allows you to specify the path\n"
	"\t     for the pre-compiled kernel, which is provided in the downloaded package.\n";
	
char *evg_err_opencl_binary_note =
	"\tYou have selected a pre-compiled OpenCL kernel binary to be passed to your\n"
	"\tOpenCL application when it requests a kernel compilation. It is your\n"
	"\tresponsibility to check that the chosen binary corresponds to the kernel\n"
	"\tthat your application is expecting to load.\n";

char *evg_err_opencl_version_note =
	"\tThe version of the Multi2Sim OpenCL library ('libm2s-opencl') that your program\n"
	"\tis using is too old. You need to re-link your program with a version of the\n"
	"\tlibrary compatible for this Multi2Sim release. Please see the Multi2Sim Guide\n"
	"\tfor details (www.multi2sim.org).\n";


/* Error macros */

#define EVG_OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", \
		__FUNCTION__, p, evg_err_opencl_note);
#define EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", \
		__FUNCTION__, (v), evg_err_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", \
		__FUNCTION__, (v), evg_err_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", \
		__FUNCTION__, (v), evg_err_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) \
		fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
		__FUNCTION__, (min), (max), evg_err_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", \
		__FUNCTION__, evg_err_opencl_param_note); }



/*
 * Entry point for OpenCL API
 */

int evg_opencl_abi_call(X86Context *ctx)
{
	int argv[EVG_OPENCL_MAX_ARGS];
	int code;
	int ret;

	/* Get function code and arguments */
	code = evg_opencl_abi_read_args(ctx, NULL, argv, sizeof argv);
	assert(IN_RANGE(code, EVG_OPENCL_FUNC_FIRST, EVG_OPENCL_FUNC_LAST));
	
	/* Call function */
	evg_opencl_debug("%s\n", evg_opencl_func_name[code - EVG_OPENCL_FUNC_FIRST]);
	ret = evg_opencl_func_impl[code - EVG_OPENCL_FUNC_FIRST](ctx, argv);

	/* Return OpencL result */
	return ret;
}


/* Return OpenCL function arguments, as identified in the current state
 * of the x86 context stack and registers. The value returned by the function
 * is the OpenCL function code identified by register 'ebx'. */
int evg_opencl_abi_read_args(X86Context *ctx, int *argc_ptr,
		void *argv_ptr, int argv_size)
{
	struct mem_t *mem = ctx->mem;
	struct x86_regs_t *regs = ctx->regs;

	unsigned int argv_guest_ptr;

	int func_code;
	int argc;

	/* Read function code */
	func_code = regs->ebx;
	if (func_code < EVG_OPENCL_FUNC_FIRST || func_code > EVG_OPENCL_FUNC_LAST)
		fatal("%s: invalid OpenCL function code - %d\n",
			__FUNCTION__, func_code);

	/* Get number of arguments */
	argc = evg_opencl_func_argc[func_code - EVG_OPENCL_FUNC_FIRST];
	if (argc_ptr)
		*argc_ptr = argc;
	if (argv_size < argc * sizeof(int))
		fatal("%s: insufficient space for arguments", __FUNCTION__);

	/* Read arguments */
	argv_guest_ptr = regs->ecx;
	mem_read(mem, argv_guest_ptr, argc * 4, argv_ptr);

	/* Return the function code */
	return func_code;
}


/* Set return value of an OpenCL API call. This needs to be done explicitly when
 * a context gets suspended during the execution of the OpenCL call, and later the
 * wake-up call-back routine finishes the OpenCL call execution. */
void evg_opencl_abi_return(X86Context *ctx, int value)
{
	ctx->regs->eax = value;
}



/*
 * OpenCL call 'clGetPlatformIDs' (code 1000)
 */

int evg_opencl_clGetPlatformIDs_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	int num_entries = argv[0];  /* cl_uint num_entries */
	unsigned int platforms = argv[1];  /* cl_platform_id *platforms */
	unsigned int num_platforms = argv[2];  /* cl_uint *num_platforms */
	unsigned int opencl_impl_version = argv[3];  /* Implementation-specific */

	unsigned int one = 1;
	int opencl_impl_version_major = opencl_impl_version >> 16;
	int opencl_impl_version_minor = (opencl_impl_version >> 8) & 0xff;
	int opencl_impl_version_build = opencl_impl_version & 0xff;

	/* Check 'libm2s-opencl' version */
	if (opencl_impl_version < EVG_SYS_OPENCL_IMPL_VERSION)
		fatal("wrong Multi2Sim OpenCL library version (provided=%d.%d.%d, required=%d.%d.%d).\n%s",
			opencl_impl_version_major, opencl_impl_version_minor, opencl_impl_version_build,
			EVG_SYS_OPENCL_IMPL_VERSION_MAJOR, EVG_SYS_OPENCL_IMPL_VERSION_MINOR,
			EVG_SYS_OPENCL_IMPL_VERSION_BUILD, evg_err_opencl_version_note);
	evg_opencl_debug("  'libm2s-opencl' version: %d.%d.%d\n",
		opencl_impl_version_major, opencl_impl_version_minor, opencl_impl_version_build);

	/* Get platform id */
	evg_opencl_debug("  num_entries=%d, platforms=0x%x, num_platforms=0x%x, version=0x%x\n",
		num_entries, platforms, num_platforms, opencl_impl_version);
	if (num_platforms)
		mem_write(mem, num_platforms, 4, &one);
	if (platforms && num_entries > 0)
		mem_write(mem, platforms, 4, &evg_emu->opencl_platform->id);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetPlatformInfo' (code 1001)
 */

int evg_opencl_clGetPlatformInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int platform_id = argv[0];  /* cl_platform_id platform */
	unsigned int param_name = argv[1];  /* cl_platform_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct evg_opencl_platform_t *platform;
	unsigned int size_ret;

	evg_opencl_debug("  platform=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
		"  param_value=0x%x, param_value_size_ret=0x%x\n",
		platform_id, param_name, param_value_size, param_value,
		param_value_size_ret);

	platform = evg_opencl_repo_get_object(evg_emu->opencl_repo,
		evg_opencl_object_platform, platform_id);
	size_ret = evg_opencl_platform_get_info(platform, param_name,
		mem, param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetDeviceIDs' (code 1002)
 */

int evg_opencl_clGetDeviceIDs_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int platform = argv[0];  /* cl_platform_id platform */
	int device_type = argv[1];  /* cl_device_type device_type */
	int num_entries = argv[2];  /* cl_uint num_entries */
	unsigned int devices = argv[3];  /* cl_device_id *devices */
	unsigned int num_devices = argv[4];  /* cl_uint *num_devices */
	unsigned int one = 1;
	struct evg_opencl_device_t *device;

	evg_opencl_debug("  platform=0x%x, device_type=%d, num_entries=%d\n",
		platform, device_type, num_entries);
	evg_opencl_debug("  devices=0x%x, num_devices=%x\n",
		devices, num_devices);
	if (platform != evg_emu->opencl_platform->id)
		fatal("%s: invalid platform\n%s", __FUNCTION__,
			evg_err_opencl_param_note);

	/* Return 1 in 'num_devices' */
	if (num_devices)
		mem_write(mem, num_devices, 4, &one);

	/* Return 'id' of the only existing device */
	if (devices && num_entries > 0)
	{
		device = evg_opencl_repo_get_object_of_type(evg_emu->opencl_repo,
			evg_opencl_object_device);
		if (!device)
			panic("%s: no device", __FUNCTION__);
		mem_write(mem, devices, 4, &device->id);
	}

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetDeviceInfo' (code 1003)
 */

int evg_opencl_clGetDeviceInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int device_id = argv[0];  /* cl_device_id device */
	unsigned int param_name = argv[1];  /* cl_device_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct evg_opencl_device_t *device;
	unsigned int size_ret;

	evg_opencl_debug("  device=0x%x, param_name=0x%x, param_value_size=%d\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			device_id, param_name, param_value_size, param_value,
			param_value_size_ret);

	device = evg_opencl_repo_get_object(evg_emu->opencl_repo,
		evg_opencl_object_device, device_id);
	size_ret = evg_opencl_device_get_info(device, param_name, mem,
		param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateContext' (code 1004)
 */

int evg_opencl_clCreateContext_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int properties = argv[0];  /* const cl_context_properties *properties */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int devices = argv[2];  /* const cl_device_id *devices */
	unsigned int pfn_notify = argv[3];  /* void (CL_CALLBACK *pfn_notify)(const char *errinfo,
					       const void *private_info, size_t cb, void *user_data) */
	unsigned int user_data = argv[4];  /* void *user_data */
	unsigned int errcode_ret = argv[5];  /* cl_int *errcode_ret */

	struct evg_opencl_device_t *device;
	struct evg_opencl_context_t *context;
	
	unsigned int device_id;
	int zero = 0;

	evg_opencl_debug("  properties=0x%x, num_devices=%d, devices=0x%x\n"
			"pfn_notify=0x%x, user_data=0x%x, errcode_ret=0x%x\n",
			properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(devices, 0);

	/* Read device id */
	mem_read(mem, devices, 4, &device_id);
	device = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_device, device_id);
	if (!device)
		fatal("%s: invalid device\n%s", __FUNCTION__, evg_err_opencl_param_note);

	/* Create context and return id */
	context = evg_opencl_context_create();
	evg_opencl_context_set_properties(context, mem, properties);
	context->device_id = device_id;

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);
	
	/* Return context */
	return context->id;
}




/*
 * OpenCL call 'clCreateContextFromType' (code 1005)
 */

int evg_opencl_clCreateContextFromType_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int properties = argv[0];  /* const cl_context_properties *properties */
	unsigned int device_type = argv[1];  /* cl_device_type device_type */
	unsigned int pfn_notify = argv[2];  /* void (*pfn_notify)(const char *, const void *, size_t , void *) */
	unsigned int user_data = argv[3];  /* void *user_data */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct evg_opencl_device_t *device;
	struct evg_opencl_context_t *context;

	int zero = 0;

	evg_opencl_debug("  properties=0x%x, device_type=0x%x, pfn_notify=0x%x,\n"
			"  user_data=0x%x, errcode_ret=0x%x\n",
			properties, device_type, pfn_notify, user_data, errcode_ret);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);

	/* Get device */
	device = evg_opencl_repo_get_object_of_type(evg_emu->opencl_repo,
			evg_opencl_object_device);
	assert(device);

	/* Create context */
	context = evg_opencl_context_create();
	context->device_id = device->id;
	evg_opencl_context_set_properties(context, mem, properties);

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);

	/* Return context */
	return context->id;
}




/*
 * OpenCL call 'clReleaseContext' (code 1007)
 */

int evg_opencl_clReleaseContext_impl(X86Context *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	struct evg_opencl_context_t *context;

	evg_opencl_debug("  context=0x%x\n", context_id);
	context = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_context, context_id);

	/* Release context */
	assert(context->ref_count > 0);
	if (!--context->ref_count)
		evg_opencl_context_free(context);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetContextInfo' (code 1008)
 */

int evg_opencl_clGetContextInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int param_name = argv[1];  /* cl_context_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct evg_opencl_context_t *context;
	unsigned int size_ret = 0;

	evg_opencl_debug("  context=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			context_id, param_name, param_value_size, param_value, param_value_size_ret);

	context = evg_opencl_repo_get_object(evg_emu->opencl_repo,
		evg_opencl_object_context, context_id);
	size_ret = evg_opencl_context_get_info(context, param_name, mem,
		param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateCommandQueue' (code 1009)
 */

int evg_opencl_clCreateCommandQueue_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int device_id = argv[1];  /* cl_device_id device */
	unsigned int properties = argv[2];  /* cl_command_queue_properties properties */
	unsigned int errcode_ret = argv[3];  /* cl_int *errcode_ret */

	struct evg_opencl_command_queue_t *command_queue;

	int zero = 0;

	evg_opencl_debug("  context=0x%x, device=0x%x, properties=0x%x, errcode_ret=0x%x\n",
			context_id, device_id, properties, errcode_ret);

	/* Check that context and device are valid */
	evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_context, context_id);
	evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_device, device_id);

	/* Create command queue and return id */
	command_queue = evg_opencl_command_queue_create();
	command_queue->context_id = context_id;
	command_queue->device_id = device_id;
	command_queue->properties = properties;

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);
	
	/* Return command queue */
	return command_queue->id;
}




/*
 * OpenCL call 'clReleaseCommandQueue' (code 1009)
 */

int evg_opencl_clReleaseCommandQueue_impl(X86Context *ctx, int *argv)
{
	unsigned int command_queue_id = argv[0];  /* cl_command_queue command_queue */
	struct evg_opencl_command_queue_t *command_queue;

	evg_opencl_debug("  command_queue=0x%x\n", command_queue_id);
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, command_queue_id);

	/* Release command queue */
	assert(command_queue->ref_count > 0);
	if (!--command_queue->ref_count)
		evg_opencl_command_queue_free(command_queue);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clRetainCommandQueue' (code 1010)
 */

int evg_opencl_clRetainCommandQueue_impl(X86Context *ctx, int *argv)
{
	unsigned int command_queue_id = argv[0];  /* cl_command_queue command_queue */

	struct evg_opencl_command_queue_t *command_queue;

	evg_opencl_debug("  command_queue=0x%x\n", command_queue_id);

	/* Check that the command queue argument is valid */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, command_queue_id);

	/* Increase the reference count */
	++command_queue->ref_count;

	/* Return success */
	return 0;
}



	
/*
 * OpenCL call 'clCreateBuffer' (code 1014)
 */

static struct str_map_t create_buffer_flags_map =
{
	4,
	{
		{ "CL_MEM_READ_WRITE", 0x1 },
		{ "CL_MEM_WRITE_ONLY", 0x2 },
		{ "CL_MEM_READ_ONLY", 0x4 },
		{ "CL_MEM_USE_HOST_PTR", 0x8 },
		{ "CL_MEM_ALLOC_HOST_PTR", 0x10 },
		{ "CL_MEM_COPY_HOST_PTR", 0x20 }
	}
};

int evg_opencl_clCreateBuffer_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int flags = argv[1];  /* cl_mem_flags flags */
	unsigned int size = argv[2];  /* size_t size */
	unsigned int host_ptr = argv[3];  /* void *host_ptr */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct evg_opencl_mem_t *opencl_mem;

	char flags_str[MAX_STRING_SIZE];
	int zero = 0;
	void *buf;

	str_map_flags(&create_buffer_flags_map, flags, flags_str, sizeof(flags_str));
	evg_opencl_debug("  context=0x%x, flags=%s, size=%d, host_ptr=0x%x, errcode_ret=0x%x\n",
			context_id, flags_str, size, host_ptr, errcode_ret);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with CL_MEM_USE_HOST_PTR\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Create memory object */
	opencl_mem = evg_opencl_mem_create();
	opencl_mem->type = 0;  /* FIXME */
	opencl_mem->size = size;
	opencl_mem->flags = flags;
	opencl_mem->host_ptr = host_ptr;

	/* Assign position in device global memory */
	opencl_mem->device_ptr = evg_emu->global_mem_top;
	evg_emu->global_mem_top += size;

	/* If 'host_ptr' was specified, copy buffer into device memory */
	if (host_ptr) {
		buf = xmalloc(size);
		mem_read(mem, host_ptr, size, buf);
		mem_write(evg_emu->global_mem, opencl_mem->device_ptr, size, buf);
		free(buf);
	}

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);
	
	/* Return memory object */
	return opencl_mem->id;
}




/*
 * OpenCL call 'clCreateImage2D' (code 1016)
 */
	
static struct str_map_t evg_opencl_create_image_flags_map =
{
	6, {
		{ "CL_MEM_READ_WRITE", 0x1 },
		{ "CL_MEM_WRITE_ONLY", 0x2 },
		{ "CL_MEM_READ_ONLY", 0x4 },
		{ "CL_MEM_USE_HOST_PTR", 0x8 },
		{ "CL_MEM_ALLOC_HOST_PTR", 0x10 },
		{ "CL_MEM_COPY_HOST_PTR", 0x20 }
	}
};

int evg_opencl_clCreateImage2D_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int flags = argv[1];  /* cl_mem_flags flags */
	unsigned int image_format_ptr = argv[2];  /* cl_image_format *image_format */
	unsigned int image_width = argv[3];  /* size_t image_width */
	unsigned int image_height = argv[4];  /* size_t image_height */
	unsigned int image_row_pitch = argv[5];  /* size_t image_row_pitch */
	unsigned int host_ptr = argv[6];  /* void *host_ptr */
	unsigned int errcode_ret_ptr = argv[7];  /* cl_int *errcode_ret */

	char flags_str[MAX_STRING_SIZE];

	void *image;
	unsigned int channel_order;
	unsigned int channel_type;
	struct evg_opencl_image_format_t image_format;

	unsigned int num_channels_per_pixel;
	unsigned int pixel_size;
	unsigned int size;
	int zero = 0;

	struct evg_opencl_mem_t *opencl_mem;

	mem_read(mem, image_format_ptr, 8, &image_format);
	channel_order = image_format.image_channel_order;
	channel_type = image_format.image_channel_data_type;

	str_map_flags(&evg_opencl_create_image_flags_map, flags, flags_str, sizeof(flags_str));
	evg_opencl_debug("  context=0x%x, flags=%s, channel order =0x%x, "
		"channel_type=0x%x, image_width=%u, image_height=%u, "
		"image_row_pitch=%u, host_ptr=0x%x, errcode_ret=0x%x\n",
		context_id, flags_str, channel_order, channel_type, image_width,
		image_height, image_row_pitch, host_ptr, errcode_ret_ptr);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with CL_MEM_USE_HOST_PTR\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Evaluate image channel order */
	switch(channel_order)
	{
	case 0x10B0:  /* CL_R */
	{
		num_channels_per_pixel = 1;
		break;
	}

	case 0x10B5: /* CL_RGBA */
	{
		num_channels_per_pixel = 4;
		break;
	}

	default:
		fatal("%s: image channel order %u not supported\n%s",
			__FUNCTION__, channel_order, evg_err_opencl_param_note);
	}

	/* Evaluate image channel type */
	switch(channel_type)
	{

	case 0x10DA: /* CL_UNSIGNED_INT8 */
	{
		pixel_size = 1 * num_channels_per_pixel;
		break;
	}

	case 0x10DE: /* CL_FLOAT */
	{
		pixel_size = 4 * num_channels_per_pixel;
		break;
	}

	default:
		fatal("%s: image channel type %u not supported\n%s",
			__FUNCTION__, channel_type, evg_err_opencl_param_note);
	}


	/* Determine image geometry */
	if (image_row_pitch == 0)
	{
		image_row_pitch = image_width*pixel_size;
	}
	else if (image_row_pitch < image_width*pixel_size)
	{
		fatal("%s: image_row_pitch must be 0 or >= image_width * size of element in bytes\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);
	}

	/* Create memory object */
	size = image_row_pitch * image_height;
	opencl_mem = evg_opencl_mem_create();
	opencl_mem->type = 1;  /* FIXME */
	opencl_mem->size = size;
	opencl_mem->flags = flags;
	opencl_mem->host_ptr = host_ptr;
	opencl_mem->pixel_size = pixel_size;
	opencl_mem->num_pixels = size/pixel_size;
	opencl_mem->num_channels_per_pixel = num_channels_per_pixel;
	opencl_mem->width = image_width;
	opencl_mem->height = image_height;
	opencl_mem->depth = 1;

	/* Assign position in device global memory */
	opencl_mem->device_ptr = evg_emu->global_mem_top;
	evg_emu->global_mem_top += size;
	evg_opencl_debug("  creating device ptr at %u, for %u bytes\n", opencl_mem->device_ptr, size);

	/* If 'host_ptr' was specified, copy image into device memory */
	if (host_ptr)
	{
		image = xmalloc(size);

		mem_read(mem, host_ptr, size, image);
		mem_write(evg_emu->global_mem, opencl_mem->device_ptr, size, image);
		free(image);
	}
	fflush(NULL);

	/* Return success */
	if (errcode_ret_ptr)
		mem_write(mem, errcode_ret_ptr, 4, &zero);

	/* Return memory object */
	return opencl_mem->id;
}




/*
 * OpenCL call 'clCreateImage3D' (code 1017)
 */

int evg_opencl_clCreateImage3D_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int flags = argv[1];  /* cl_mem_flags flags */
	unsigned int image_format_ptr = argv[2];  /* cl_image_format *image_format */
	unsigned int image_width = argv[3];  /* size_t image_width */
	unsigned int image_height = argv[4];  /* size_t image_height */
	unsigned int image_depth = argv[5];  /* size_t image_depth */
	unsigned int image_row_pitch = argv[6];  /* size_t image_row_pitch */
	unsigned int image_slice_pitch = argv[7];  /* size_t image_slice_pitch */
	unsigned int host_ptr = argv[8];  /* void *host_ptr */
	unsigned int errcode_ret_ptr = argv[9];  /* cl_int *errcode_ret */

	char flags_str[MAX_STRING_SIZE];

	void *image;

	unsigned int channel_order;
	unsigned int channel_type;
	struct evg_opencl_image_format_t image_format;

	unsigned int num_channels_per_pixel;
	unsigned int pixel_size;

	struct evg_opencl_mem_t *opencl_mem;
	unsigned int size;

	int zero = 0;

	mem_read(mem, image_format_ptr, 8, &image_format);
	channel_order = image_format.image_channel_order;
	channel_type = image_format.image_channel_data_type;

	str_map_flags(&evg_opencl_create_image_flags_map, flags, flags_str, sizeof(flags_str));
	evg_opencl_debug("  context=0x%x, flags=%s, channel order =0x%x, channel_type=0x%x\n" 
			"  image_width=%u, image_height=%u, image_depth=%u\n"
			"  image_row_pitch=%u, image_slice_pitch=%u, host_ptr=0x%x\n"
			"  errcode_ret=0x%x\n",
			context_id, flags_str, channel_order, channel_type, image_width, image_height, image_depth, 
			image_row_pitch, image_slice_pitch, host_ptr, errcode_ret_ptr);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with CL_MEM_USE_HOST_PTR\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' != NULL\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Evaluate image channel order */
	switch(channel_order)
	{

	case 0x10B0:  /* CL_R */
	{
		num_channels_per_pixel = 1;
		break;
	}

	case 0x10B5: /* CL_RGBA */
	{
		num_channels_per_pixel = 4;
		break;
	}

	default:
		fatal("%s: image channel order %u not supported\n%s",
				__FUNCTION__, channel_order, evg_err_opencl_param_note);
	}

	/* Evaluate image channel type */
	switch(channel_type)
	{

	case 0x10DA: /* CL_UNSIGNED_INT8 */
	{
		pixel_size = 1*num_channels_per_pixel;
		break;
	}

	case 0x10DE: /* CL_FLOAT */
	{
		pixel_size = 4*num_channels_per_pixel;
		break;
	}

	default:
		fatal("%s: image channel type %u not supported\n%s",
				__FUNCTION__, channel_type, evg_err_opencl_param_note);
	}

	/* Determine image geometry */
	if (image_row_pitch == 0)
		image_row_pitch = image_width*pixel_size;
	else if (image_row_pitch < image_width*pixel_size)
		fatal("%s: image_row_pitch must be 0 or >= image_width * size of element in bytes\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);

	if (image_slice_pitch == 0)
		image_slice_pitch = image_row_pitch*image_height;
	else if (image_slice_pitch < image_row_pitch*image_height)
		fatal("%s: image_slice_pitch must be 0 or >= image_row_pitch * image_height\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);

	/* Create memory object */
	size = image_slice_pitch*image_depth;
	opencl_mem = evg_opencl_mem_create();
	opencl_mem->type = 2; /* FIXME */
	opencl_mem->size = size;
	opencl_mem->flags = flags;
	opencl_mem->host_ptr = host_ptr;
	opencl_mem->num_pixels = size/pixel_size;
	opencl_mem->pixel_size = pixel_size;
	opencl_mem->num_channels_per_pixel = num_channels_per_pixel;
	opencl_mem->width = image_width;
	opencl_mem->height = image_height;
	opencl_mem->depth = image_depth;

	/* Assign position in device global memory */
	opencl_mem->device_ptr = evg_emu->global_mem_top;
	evg_emu->global_mem_top += size;

	/* If 'host_ptr' was specified, copy image into device memory */
	if (host_ptr)
	{
		image = xmalloc(size);

		mem_read(mem, host_ptr, size, image);
		mem_write(evg_emu->global_mem, opencl_mem->device_ptr, size, image);
		free(image);
	}

	/* Return success */
	if (errcode_ret_ptr)
		mem_write(mem, errcode_ret_ptr, 4, &zero);

	/* Return memory object */
	return opencl_mem->id;
}




/*
 * OpenCL call 'clRetainMemObject' (code 1018)
 */

int evg_opencl_clRetainMemObject_impl(X86Context *ctx, int *argv)
{
	unsigned int mem_id = argv[0];  /* cl_mem memobj */

	struct evg_opencl_mem_t *mem;

	evg_opencl_debug("  memobj=0x%x\n", mem_id);
	mem = evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_mem,
			mem_id);

	/* Increase the reference count */
	++mem->ref_count;

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clReleaseMemObject' (code 1019)
 */

int evg_opencl_clReleaseMemObject_impl(X86Context *ctx, int *argv)
{
	unsigned int mem_id = argv[0];  /* cl_mem memobj */
	struct evg_opencl_mem_t *opencl_mem;

	evg_opencl_debug("  memobj=0x%x\n", mem_id);
	opencl_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_mem, mem_id);

	/* Release object */
	assert(opencl_mem->ref_count > 0);
	if (!--opencl_mem->ref_count)
		evg_opencl_mem_free(opencl_mem);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateSampler' (code 1024)
 */

int evg_opencl_clCreateSampler_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context = argv[0];  /* cl_context context */
	unsigned int normalized_coords = argv[1];  /* cl_bool normalized_coords */
	unsigned int addressing_mode = argv[2];  /* cl_addressing_mode addressing_mode */
	unsigned int filter_mode = argv[3];  /* cl_filter_mode filter_mode */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct evg_opencl_sampler_t *sampler;
	int zero = 0;

	evg_opencl_debug("  context=0x%x, normalized_coords=%d, addressing_mode=0x%x," 
			" lengths=0x%x, errcode_ret=0x%x\n",
			context, normalized_coords, addressing_mode, filter_mode, errcode_ret);

	if (normalized_coords != 0)
		fatal("%s: Normalized coordinates are not supported.\n", __FUNCTION__);

	if (filter_mode != 0x1140)  /* only CL_FILTER_NEAREST supported */
		fatal("%s: filter mode %u not supported.\n", __FUNCTION__, filter_mode);

	if (addressing_mode != 0x1130) /* only CL_ADDRESS_NONE supported */
		fatal("%s: addressing mode %u not supported.\n", __FUNCTION__, 
				addressing_mode);

	evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_context, context);

	/* Create command queue and return id */
	sampler = evg_opencl_sampler_create();
	sampler->normalized_coords = normalized_coords;
	sampler->addressing_mode = addressing_mode;
	sampler->filter_mode = filter_mode;

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);
	
	/* Return sampler ID */
	return sampler->id;
}




/*
 * OpenCL call 'clCreateProgramWithSource' (code 1028)
 */

int evg_opencl_clCreateProgramWithSource_impl(X86Context *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int count = argv[1];  /* cl_uint count */
	unsigned int strings = argv[2];  /* const char **strings */
	unsigned int lengths = argv[3];  /* const size_t *lengths */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct evg_opencl_program_t *program;

	evg_opencl_debug("  context=0x%x, count=%d, strings=0x%x, lengths=0x%x, errcode_ret=0x%x\n",
			context_id, count, strings, lengths, errcode_ret);

	/* Application tries to compile source, and no binary was passed to Multi2Sim */
	if (!*evg_emu_opencl_binary_name)
		fatal("%s: kernel source compilation not supported.\n%s",
				__FUNCTION__, evg_err_opencl_compiler);

	/* Create program */
	evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_context, context_id);
	program = evg_opencl_program_create();
	warning("%s: binary '%s' used as pre-compiled kernel.\n%s",
			__FUNCTION__, evg_emu_opencl_binary_name, evg_err_opencl_binary_note);

	/* Load OpenCL binary passed to Multi2Sim and make a copy in temporary file */
	program->elf_file = elf_file_create_from_path(evg_emu_opencl_binary_name);

	/* Return program */
	return program->id;
}




/*
 * OpenCL call 'clCreateProgramWithBinary' (code 1029)
 */

int evg_opencl_clCreateProgramWithBinary_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int device_list = argv[2];  /* const cl_device_id *device_list */
	unsigned int lengths = argv[3];  /* const size_t *lengths */
	unsigned int binaries = argv[4];  /* const unsigned char **binaries */
	unsigned int binary_status = argv[5];  /* cl_int *binary_status */
	unsigned int errcode_ret = argv[6];  /* cl_int *errcode_ret */

	unsigned int length, binary;
	unsigned int device_id;

	struct evg_opencl_program_t *program;
	void *buf;

	int zero = 0;

	char name[MAX_STRING_SIZE];

	evg_opencl_debug("  context=0x%x, num_devices=%d, device_list=0x%x, lengths=0x%x\n"
			"  binaries=0x%x, binary_status=0x%x, errcode_ret=0x%x\n",
			context_id, num_devices, device_list, lengths, binaries,
			binary_status, errcode_ret);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	/* Get device and context */
	mem_read(mem, device_list, 4, &device_id);
	evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_device, device_id);
	evg_opencl_repo_get_object(evg_emu->opencl_repo, evg_opencl_object_context, context_id);

	/* Create program */
	program = evg_opencl_program_create();

	/* Read binary length and pointer */
	mem_read(mem, lengths, 4, &length);
	mem_read(mem, binaries, 4, &binary);
	evg_opencl_debug("    lengths[0] = %d\n", length);
	evg_opencl_debug("    binaries[0] = 0x%x\n", binary);

	/* Read binary */
	buf = xmalloc(length);
	mem_read(mem, binary, length, buf);

	/* Load ELF binary from guest memory */
	snprintf(name, sizeof(name), "clProgram<%d>.externalELF", program->id);
	program->elf_file = elf_file_create_from_buffer(buf, length, name);

	/* Search ELF binary to see if there are any constant buffers encoded inside */
	evg_opencl_program_initialize_constant_buffers(program);

	free(buf);

	/* Return success */
	if (binary_status)
		mem_write(mem, binary_status, 4, &zero);
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);

	/* Return program */
	return program->id;
}




/*
 * OpenCL call 'clRetainProgram' (code 1030)
 */

int evg_opencl_clRetainProgram_impl(X86Context *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */

	struct evg_opencl_program_t *program;

	evg_opencl_debug("  program=0x%x\n", program_id);
	program = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_program, program_id);

	/* Increase the reference count */
	++program->ref_count;
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clReleaseProgram' (code 1031)
 */

int evg_opencl_clReleaseProgram_impl(X86Context *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */
	struct evg_opencl_program_t *program;

	evg_opencl_debug("  program=0x%x\n", program_id);
	program = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_program, program_id);

	/* Release program */
	assert(program->ref_count > 0);
	if (!--program->ref_count)
		evg_opencl_program_free(program);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clBuildProgram' (code 1032)
 */

int evg_opencl_clBuildProgram_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int program_id = argv[0];  /* cl_program program */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int device_list = argv[2];  /* const cl_device_id *device_list */
	unsigned int options = argv[3];  /* const char *options */
	unsigned int pfn_notify = argv[4];  /* void (CL_CALLBACK *pfn_notify)(cl_program program,
					       void *user_data) */
	unsigned int user_data = argv[5];  /* void *user_data */

	struct evg_opencl_program_t *program;
	char options_str[MAX_STRING_SIZE];

	options_str[0] = 0;
	if (options)
		mem_read_string(mem, options, MAX_STRING_SIZE, options_str);

	evg_opencl_debug("  program=0x%x, num_devices=%d, device_list=0x%x, options=0x%x\n"
			"  pfn_notify=0x%x, user_data=0x%x, options='%s'\n",
			program_id, num_devices, device_list, options, pfn_notify,
			user_data, options_str);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(user_data, 0);
	if (options_str[0])
		warning("%s: clBuildProgram: option string '%s' ignored\n",
				__FUNCTION__, options_str);

	/* Get program */
	program = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_program, program_id);
	if (!program->elf_file)
		fatal("%s: program binary must be loaded first.\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Build program */
	evg_opencl_program_build(program);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateKernel' (code 1036)
 */

int evg_opencl_clCreateKernel_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int program_id = argv[0];  /* cl_program program */
	unsigned int kernel_name = argv[1];  /* const char *kernel_name */
	unsigned int errcode_ret = argv[2];  /* cl_int *errcode_ret */

	char kernel_name_str[MAX_STRING_SIZE];

	struct evg_opencl_kernel_t *kernel;
	struct evg_opencl_program_t *program;

	void *constant_tmp;

	int zero = 0;
	int i;

	evg_opencl_debug("  program=0x%x, kernel_name=0x%x, errcode_ret=0x%x\n",
			program_id, kernel_name, errcode_ret);
	if (mem_read_string(mem, kernel_name, MAX_STRING_SIZE, kernel_name_str) == MAX_STRING_SIZE)
		fatal("%s: 'kernel_name' string is too long", __FUNCTION__);
	evg_opencl_debug("    kernel_name='%s'\n", kernel_name_str);

	/* Get program */
	program = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_program, program_id);

	/* Create the kernel */
	kernel = evg_opencl_kernel_create();
	kernel->program_id = program_id;

	/* Program must be built */
	if (!program->elf_file)
		fatal("%s: program should be first built with clBuildProgram.\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Load kernel */
	evg_opencl_kernel_load(kernel, kernel_name_str);

	/* Add program-wide constant buffers to the kernel-specific list */
	for (i = 0; i < 25; i++) 
	{
		constant_tmp = list_get(program->constant_buffer_list, i);
		if (constant_tmp) 
			list_set(kernel->constant_buffer_list, i, constant_tmp);
	}

	/* Return success */
	if (errcode_ret)
		mem_write(mem, errcode_ret, 4, &zero);
	
	/* Return kernel */
	return kernel->id;
}




/*
 * OpenCL call 'clReleaseKernel' (code 1039)
 */

int evg_opencl_clReleaseKernel_impl(X86Context *ctx, int *argv)
{
	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */
	struct evg_opencl_kernel_t *kernel;

	evg_opencl_debug("  kernel=0x%x\n", kernel_id);
	kernel = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_kernel, kernel_id);

	/* Release kernel */
	assert(kernel->ref_count > 0);
	if (!--kernel->ref_count)
		evg_opencl_kernel_free(kernel);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clSetKernelArg' (code 1040)
 */

int evg_opencl_clSetKernelArg_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */
	unsigned int arg_index = argv[1];  /* cl_uint arg_index */
	unsigned int arg_size = argv[2];  /* size_t arg_size */
	unsigned int arg_value = argv[3];  /* const void *arg_value */

	struct evg_opencl_kernel_t *kernel;
	struct evg_opencl_kernel_arg_t *arg;

	evg_opencl_debug("  kernel_id=0x%x, arg_index=%d, arg_size=%d, arg_value=0x%x\n",
			kernel_id, arg_index, arg_size, arg_value);

	/* Check */
	kernel = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_kernel, kernel_id);
	if (arg_value)
		EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(arg_size, 4);
	if (arg_index >= list_count(kernel->arg_list))
		fatal("%s: argument index out of bounds.\n%s", __FUNCTION__,
				evg_err_opencl_param_note);

	/* Copy to kernel object */
	arg = list_get(kernel->arg_list, arg_index);
	assert(arg);
	arg->set = 1;
	arg->size = arg_size;
	if (arg_value)
		mem_read(mem, arg_value, 4, &arg->value);

	/* If OpenCL argument scope is __local, argument value must be NULL */
	if (arg->mem_scope == EVG_OPENCL_MEM_SCOPE_LOCAL && arg_value)
		fatal("%s: value for local arguments must be NULL.\n%s", __FUNCTION__,
				evg_err_opencl_param_note);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetKernelWorkGroupInfo' (code 1042)
 */

int evg_opencl_clGetKernelWorkGroupInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */
	unsigned int device_id = argv[1];  /* cl_device_id device */
	unsigned int param_name = argv[2];  /* cl_kernel_work_group_info param_name */
	unsigned int param_value_size = argv[3];  /* size_t param_value_size */
	unsigned int param_value = argv[4];  /* void *param_value */
	unsigned int param_value_size_ret = argv[5];  /* size_t *param_value_size_ret */

	struct evg_opencl_kernel_t *kernel;
	unsigned int size_ret;

	evg_opencl_debug("  kernel=0x%x, device=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			kernel_id, device_id, param_name, param_value_size, param_value,
			param_value_size_ret);

	kernel = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_kernel, kernel_id);
	evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_device, device_id);
	size_ret = evg_opencl_kernel_get_work_group_info(kernel, param_name, mem,
			param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clWaitForEvents' (code 1043)
 */

int evg_opencl_clWaitForEvents_impl(X86Context *ctx, int *argv)
{
	unsigned int num_events = argv[0];  /* cl_uint num_events */
	unsigned int event_list = argv[1];  /* const cl_event *event_list */

	evg_opencl_debug("  num_events=0x%x, event_list=0x%x\n",
			num_events, event_list);
	
	/* FIXME: block until events in list are completed */

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetEventInfo' (code 1044)
 */

int evg_opencl_clGetEventInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int param_name = argv[1]; /* cl_event_info param_name */
	unsigned int param_value = argv[3]; /* void *param_value */

	switch (param_name)
	{

	case 0x11d0: /* CL_EVENT_COMMAND_QUEUE */
		fatal("param_name CL_EVENT_COMMAND_QUEUE not supported for clGetEventInfo"); 
		break;

	case 0x11d1: /* CL_EVENT_COMMAND_TYPE */
		fatal("param_name CL_EVENT_COMMAND_TYPE not supported for clGetEventInfo"); 
		break;

	case 0x11d2: /* CL_EVENT_REFERENCE_COUNT */
		fatal("param_name CL_EVENT_REFERNCE_COUNT not supported for clGetEventInfo"); 
		break;

	case 0x11d3: /* CL_EVENT_COMMAND_EXECUTION_STATUS */
	{
		int status = 0;  /* CL_COMPLETE */;

		warning("clGetEventInfo always returns CL_COMPLETE");
		mem_write(mem, param_value, 4, &status);

		break;
	}

	case 0x11d4: /* CL_EVENT_CONTEXT */
		fatal("param_name CL_EVENT_CONTEXT not supported for clGetEventInfo"); 
		break;

	default:
		fatal("%s: invalid value for 'param_name'", __FUNCTION__); 
	}

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clReleaseEvent' (code 1047)
 */

int evg_opencl_clReleaseEvent_impl(X86Context *ctx, int *argv)
{
	unsigned int event_id = argv[0];  /* cl_event event */
	struct evg_opencl_event_t *event;

	evg_opencl_debug("  event=0x%x\n", event_id);
	event = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_event, event_id);

	/* Release event */
	assert(event->ref_count > 0);
	if (!--event->ref_count)
		evg_opencl_event_free(event);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetEventProfilingInfo' (code 1050)
 */

int evg_opencl_clGetEventProfilingInfo_impl(X86Context *ctx, int *argv)
{
	struct mem_t *mem = ctx->mem;

	unsigned int event_id = argv[0];  /* cl_event event */
	unsigned int param_name = argv[1];  /* cl_profiling_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct evg_opencl_event_t *event;
	int size_ret;

	evg_opencl_debug("  event=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			event_id, param_name, param_value_size, param_value,
			param_value_size_ret);
	event = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_event, event_id);
	size_ret = evg_opencl_event_get_profiling_info(event, param_name, mem,
			param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clFlush' (code 1051)
 */

int evg_opencl_clFlush_impl(X86Context *ctx, int *argv)
{
	unsigned int command_queue = argv[0];  /* cl_command_queue command_queue */

	evg_opencl_debug("  command_queue=0x%x\n", command_queue);
	return 0;
}




/*
 * OpenCL call 'clFinish' (code 1052)
 */

struct evg_opencl_clFinish_args_t
{
	unsigned int command_queue_id;  /* cl_command_queue command_queue */
};

void evg_opencl_clFinish_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clFinish_args_t argv;
	int code;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1052);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clFinish_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clFinish_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clFinish_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x\n", argv->command_queue_id);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue_id);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clFinish_wakeup, NULL);

	/* Return value ignored by caller, since context is getting suspended.
	 * It will be explicitly set by the wake-up call-back routine. */
	return 0;

}




/*
 * OpenCL call 'clEnqueueReadBuffer' (code 1053)
 */

struct evg_opencl_clEnqueueReadBuffer_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int buffer;  /* cl_mem buffer */
	unsigned int blocking_read;  /* cl_bool blocking_read */
	unsigned int offset;  /* size_t offset */
	unsigned int cb;  /* size_t cb */
	unsigned int ptr;  /* void *ptr */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void evg_opencl_clEnqueueReadBuffer_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueReadBuffer_args_t argv;
	struct evg_opencl_mem_t *opencl_mem;
	struct evg_opencl_event_t *event;

	void *buf;

	int code;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1053);

	/* Get memory object */
	opencl_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.offset + argv.cb > opencl_mem->size)
		fatal("%s: buffer storage exceeded\n%s", __FUNCTION__,
				evg_err_opencl_param_note);

	/* Copy buffer from device memory to host memory */
	buf = xmalloc(argv.cb);
	mem_read(evg_emu->global_mem, opencl_mem->device_ptr + argv.offset, argv.cb, buf);
	mem_write(ctx->mem, argv.ptr, argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = EVG_OPENCL_EVENT_STATUS_SUBMITTED;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();
		event->time_end = evg_opencl_event_timer();
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	evg_opencl_debug("\t%d bytes copied from device (0x%x) to host(0x%x)\n",
			argv.cb, opencl_mem->device_ptr + argv.offset, argv.ptr);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueReadBuffer_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueReadBuffer_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueReadBuffer_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_read=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_read, argv->offset,
			argv->cb, argv->ptr, argv->num_events_in_wait_list, argv->event_wait_list,
			argv->event_ptr);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue);
	
	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueReadBuffer_wakeup, NULL);

	/* Return value ignored by caller, since context is getting suspended.
	 * It will be explicitly set by the wake-up call-back routine. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueWriteBuffer' (code 1055)
 */

struct evg_opencl_clEnqueueWriteBuffer_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int buffer;  /* cl_mem buffer */
	unsigned int blocking_write;  /* cl_bool blocking_write */
	unsigned int offset;  /* size_t offset */
	unsigned int cb;  /* size_t cb */
	unsigned int ptr;  /* const void *ptr */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void evg_opencl_clEnqueueWriteBuffer_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueWriteBuffer_args_t argv;
	struct evg_opencl_mem_t *opencl_mem;
	struct evg_opencl_event_t *event;

	void *buf;

	int code;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1055);

	/* Get memory object */
	opencl_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.offset + argv.cb > opencl_mem->size)
		fatal("%s: buffer storage exceeded.\n%s", __FUNCTION__,
				evg_err_opencl_param_note);

	/* Copy buffer from host memory to device memory */
	buf = xmalloc(argv.cb);
	mem_read(ctx->mem, argv.ptr, argv.cb, buf);
	mem_write(evg_emu->global_mem, opencl_mem->device_ptr + argv.offset, argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_MAP_BUFFER);
		event->status = EVG_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();
		event->time_end = evg_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	evg_opencl_debug("\t%d bytes copied from host (0x%x) to device (0x%x)\n",
			argv.cb, argv.ptr, opencl_mem->device_ptr + argv.offset);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueWriteBuffer_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueWriteBuffer_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueWriteBuffer_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_write=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_write,
			argv->offset, argv->cb, argv->ptr, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueWriteBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueCopyBuffer' (code 1057)
 */

struct evg_opencl_clEnqueueCopyBuffer_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int src_buffer;  /* cl_mem src_buffer */
	unsigned int dst_buffer;  /* cl_mem dst_buffer */
	unsigned int src_offset;  /* size_t src_offset */
	unsigned int dst_offset;  /* size_t dst_offset */
	unsigned int cb;  /* size_t cb */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void evg_opencl_clEnqueueCopyBuffer_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueCopyBuffer_args_t argv;
	struct evg_opencl_mem_t *src_mem;
	struct evg_opencl_mem_t *dst_mem;
	struct evg_opencl_event_t *event;

	void *buf;
	int code;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1057);

	/* Get memory objects */
	src_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.src_buffer);
	dst_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.dst_buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.src_offset + argv.cb > src_mem->size || argv.dst_offset + argv.cb > dst_mem->size)
		fatal("%s: buffer storage exceeded\n%s", __FUNCTION__, evg_err_opencl_param_note);

	/* Copy buffers */
	buf = xmalloc(argv.cb);
	mem_read(evg_emu->global_mem, src_mem->device_ptr + argv.src_offset, argv.cb, buf);
	mem_write(evg_emu->global_mem, dst_mem->device_ptr + argv.dst_offset, argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_MAP_BUFFER);
		event->status = EVG_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();
		event->time_end = evg_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	evg_opencl_debug("\t%d bytes copied in device memory (0x%x -> 0x%x)\n",
			argv.cb, src_mem->device_ptr + argv.src_offset,
			dst_mem->device_ptr + argv.dst_offset);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueCopyBuffer_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueCopyBuffer_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueCopyBuffer_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, src_buffer=0x%x, dst_buffer=0x%x,\n"
			"  src_offset=0x%x, dst_offset=0x%x, cb=%u, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->src_buffer, argv->dst_buffer, argv->src_offset,
			argv->dst_offset, argv->cb, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueCopyBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueReadImage' (code 1059)
 */

struct evg_opencl_clEnqueueReadImage_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int image;  /* cl_mem image*/
	unsigned int blocking_read;  /* cl_bool blocking_read */
	unsigned int origin;  /* size_t origin[3] */
	unsigned int region;  /* size_t region[3] */
	unsigned int row_pitch;  /* size_t row_pitch */
	unsigned int slice_pitch;  /* size_t slice_pitch */
	unsigned int ptr;  /* void *ptr */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void evg_opencl_clEnqueueReadImage_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueReadImage_args_t argv;

	unsigned int read_region[3];
	unsigned int read_origin[3];

	struct evg_opencl_mem_t *opencl_mem;
	struct evg_opencl_event_t *event;

	void *img;
	int code;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1059);

	/* Get memory object */
	opencl_mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.image);

	/* Determine image geometry */
	/* NOTE size_t is 32-bits on 32-bit systems, but 64-bits on 64-bit systems.  Since
	 * the simulator may be on a 64-bit system, we should not use size_t here, but unsigned int 
	 * instead. */
	mem_read(ctx->mem, argv.region, 12, read_region);
	mem_read(ctx->mem, argv.origin, 12, read_origin);

	if (!argv.row_pitch)
		argv.row_pitch = opencl_mem->width * opencl_mem->pixel_size;
	else if (argv.row_pitch < opencl_mem->width * opencl_mem->pixel_size)
		fatal("%s: row_pitch must be 0 or >= image_width * size of element in bytes\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);

	if (!argv.slice_pitch)
		argv.slice_pitch = argv.row_pitch * opencl_mem->height;
	else if (argv.slice_pitch < argv.row_pitch * opencl_mem->height)
		fatal("%s: slice_pitch must be 0 or >= row_pitch * image_height\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);

	/* FIXME Start with origin = {0,0,0}, region = {width, height, depth},
	 * then add support for subregions */
	if (read_origin[0] != 0 || read_origin[1] != 0 || read_origin[2] != 0 ||
			read_region[0] != opencl_mem->width || read_region[1] != opencl_mem->height ||
			read_region[2] != opencl_mem->depth)
		fatal("%s: Origin/region must match dimensions of image\n%s", 
				__FUNCTION__, evg_err_opencl_param_note);

	/* Read the entire image */
	img = xmalloc(opencl_mem->size);
	mem_read(evg_emu->global_mem, opencl_mem->device_ptr, opencl_mem->size, img);
	mem_write(ctx->mem, argv.ptr, opencl_mem->size, img);
	free(img);

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = EVG_OPENCL_EVENT_STATUS_SUBMITTED;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();
		event->time_end = evg_opencl_event_timer();
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	/* FIXME Return size of region, not entire image */
	evg_opencl_debug("\t%d bytes copied from device (0x%x) to host (0x%x)\n",
			opencl_mem->size, opencl_mem->device_ptr, argv.ptr);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueReadImage_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueReadImage_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueReadImage_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, image=0x%x, blocking_read=0x%x,\n"
			"  origin=0x%x, region=0x%x, row_pitch=0x%x, slice_pitch=0x%x, ptr=0x%x\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->image, argv->blocking_read, argv->origin,
			argv->region, argv->row_pitch, argv->slice_pitch, argv->ptr,
			argv->num_events_in_wait_list, argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueReadImage_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueMapBuffer' (code 1064)
 */

struct evg_opencl_clEnqueueMapBuffer_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int buffer;  /* cl_mem buffer */
	unsigned int blocking_map;  /* cl_bool blocking_map */
	unsigned int map_flags;  /* cl_map_flags str_map_flags */
	unsigned int offset;  /* size_t offset */
	unsigned int cb;  /* size_t cb */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
	unsigned int errcode_ret;  /* cl_int *errcode_ret */
};

void evg_opencl_clEnqueueMapBuffer_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueMapBuffer_args_t argv;
	struct evg_opencl_event_t *event;

	int code;
	int zero = 0;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1064);

	/* Get memory object */
	evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_mem, argv.buffer);

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_MAP_BUFFER);
		event->status = EVG_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();
		event->time_end = evg_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Return success */
	if (argv.errcode_ret)
		mem_write(ctx->mem, argv.errcode_ret, 4, &zero);
	
	/* Not ready yet */
	fatal("%s: not implemented", __FUNCTION__);

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueMapBuffer_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueMapBuffer_args_t *argv;
	struct evg_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueMapBuffer_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_map=0x%x, map_flags=0x%x,\n"
			"  offset=0x%x, cb=0x%x, num_events_in_wait_list=0x%x, event_wait_list=0x%x,\n"
			"  event=0x%x, errcode_ret=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_map, argv->map_flags,
			argv->offset, argv->cb, argv->num_events_in_wait_list, argv->event_wait_list,
			argv->event_ptr, argv->errcode_ret);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(argv->blocking_map, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueMapBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueNDRangeKernel' (code 1067)
 */

struct evg_opencl_clEnqueueNDRangeKernel_args_t
{
	unsigned int command_queue_id;  /* cl_command_queue command_queue */
	unsigned int kernel_id;  /* cl_kernel kernel */
	unsigned int work_dim;  /* cl_uint work_dim */
	unsigned int global_work_offset_ptr;  /* const size_t *global_work_offset */
	unsigned int global_work_size_ptr;  /* const size_t *global_work_size */
	unsigned int local_work_size_ptr;  /* const size_t *local_work_size */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void evg_opencl_clEnqueueNDRangeKernel_wakeup(X86Context *ctx, void *data)
{
	struct evg_opencl_clEnqueueNDRangeKernel_args_t argv;

	struct evg_opencl_kernel_t *kernel;
	struct evg_opencl_event_t *event = NULL;
	struct evg_opencl_kernel_arg_t *arg;
	struct evg_opencl_mem_t *mem;

	struct evg_opencl_command_queue_t *command_queue;
	struct evg_opencl_command_t *task;

	struct evg_ndrange_t *ndrange;

	int code;
	int i;

	/* Read function arguments again */
	code = evg_opencl_abi_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1067);

	/* Get kernel */
	kernel = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_kernel, argv.kernel_id);
	kernel->work_dim = argv.work_dim;

	/* Build UAV lists */
	for (i = 0; i < list_count(kernel->arg_list); i++) 
	{
		arg = list_get(kernel->arg_list, i);

		/* If argument is an image, add it to the appropriate UAV list */
		if (arg->kind == EVG_OPENCL_KERNEL_ARG_KIND_IMAGE) 
		{
			mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
					evg_opencl_object_mem, arg->value);

			if (arg->access_type == EVG_OPENCL_KERNEL_ARG_READ_ONLY) 
				list_set(kernel->uav_read_list, arg->uav, mem);
			else if (arg->access_type == EVG_OPENCL_KERNEL_ARG_WRITE_ONLY) 
				list_set(kernel->uav_write_list, arg->uav, mem);
			else 
				fatal("%s: unsupported image access type (%d)\n", __FUNCTION__, 
						arg->access_type);
		}

		/* If argument is a pointer and not in UAV 11, then it is
		   presumably a constant pointer */
		/* TODO Check if __read_only or __write_only affects uav number */
		if (arg->mem_scope == EVG_OPENCL_MEM_SCOPE_CONSTANT)
		{	
			mem = evg_opencl_repo_get_object(evg_emu->opencl_repo,
					evg_opencl_object_mem, arg->value);
			list_set(kernel->constant_buffer_list, arg->uav, mem);
		}
	}


	/* Global work sizes */
	kernel->global_size3[1] = 1;
	kernel->global_size3[2] = 1;
	for (i = 0; i < argv.work_dim; i++)
		mem_read(ctx->mem, argv.global_work_size_ptr + i * 4, 4, &kernel->global_size3[i]);
	kernel->global_size = kernel->global_size3[0] * kernel->global_size3[1] * kernel->global_size3[2];
	evg_opencl_debug("    global_work_size=");
	evg_opencl_debug_array(argv.work_dim, kernel->global_size3);
	evg_opencl_debug("\n");

	/* Local work sizes.
	 * If no pointer provided, assign the same as global size - FIXME: can be done better. */
	memcpy(kernel->local_size3, kernel->global_size3, 12);
	if (argv.local_work_size_ptr)
	{
		for (i = 0; i < argv.work_dim; i++)
		{
			mem_read(ctx->mem, argv.local_work_size_ptr + i * 4, 4, &kernel->local_size3[i]);
			if (kernel->local_size3[i] < 1)
				fatal("%s: local work size must be greater than 0.\n%s",
						__FUNCTION__, evg_err_opencl_param_note);
		}
	}
	kernel->local_size = kernel->local_size3[0] * kernel->local_size3[1] * kernel->local_size3[2];
	evg_opencl_debug("    local_work_size=");
	evg_opencl_debug_array(argv.work_dim, kernel->local_size3);
	evg_opencl_debug("\n");

	/* Check valid global/local sizes */
	if (kernel->global_size3[0] < 1 || kernel->global_size3[1] < 1
			|| kernel->global_size3[2] < 1)
		fatal("%s: invalid global size.\n%s", __FUNCTION__, evg_err_opencl_param_note);
	if (kernel->local_size3[0] < 1 || kernel->local_size3[1] < 1
			|| kernel->local_size3[2] < 1)
		fatal("%s: invalid local size.\n%s", __FUNCTION__, evg_err_opencl_param_note);

	/* Check divisibility of global by local sizes */
	if ((kernel->global_size3[0] % kernel->local_size3[0])
			|| (kernel->global_size3[1] % kernel->local_size3[1])
			|| (kernel->global_size3[2] % kernel->local_size3[2]))
		fatal("%s: global work sizes must be multiples of local sizes.\n%s",
				__FUNCTION__, evg_err_opencl_param_note);

	/* Calculate number of groups */
	for (i = 0; i < 3; i++)
		kernel->group_count3[i] = kernel->global_size3[i] / kernel->local_size3[i];
	kernel->group_count = kernel->group_count3[0] * kernel->group_count3[1] * kernel->group_count3[2];
	evg_opencl_debug("    group_count=");
	evg_opencl_debug_array(argv.work_dim, kernel->group_count3);
	evg_opencl_debug("\n");

	/* Event */
	if (argv.event_ptr)
	{
		event = evg_opencl_event_create(EVG_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = EVG_OPENCL_EVENT_STATUS_SUBMITTED;
		event->time_queued = evg_opencl_event_timer();
		event->time_submit = evg_opencl_event_timer();
		event->time_start = evg_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		evg_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Setup ND-Range */
	ndrange = evg_ndrange_create(kernel);
	evg_ndrange_setup_work_items(ndrange);
	evg_ndrange_setup_const_mem(ndrange);
	evg_ndrange_setup_args(ndrange);

	/* Save in kernel */
	kernel->ndrange = ndrange;

	/* Set ND-Range status to 'pending'. This makes it immediately a candidate for
	 * execution, whether we have functional or detailed simulation. */
	evg_ndrange_set_status(ndrange, evg_ndrange_pending);

	/* Create command queue task */
	task = evg_opencl_command_create(evg_opencl_command_queue_task_ndrange_kernel);
	task->u.ndrange_kernel.ndrange = ndrange;

	/* Enqueue task */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv.command_queue_id);
	evg_opencl_command_queue_submit(command_queue, task);
	ndrange->command_queue = command_queue;
	ndrange->command = task;

	/* Return success */
	evg_opencl_abi_return(ctx, 0);
}

int evg_opencl_clEnqueueNDRangeKernel_impl(X86Context *ctx, int *argv_ptr)
{
	struct evg_opencl_clEnqueueNDRangeKernel_args_t *argv;
	struct evg_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct evg_opencl_clEnqueueNDRangeKernel_args_t *) argv_ptr;
	evg_opencl_debug("  command_queue=0x%x, kernel=0x%x, work_dim=%d,\n"
			"  global_work_offset=0x%x, global_work_size_ptr=0x%x, local_work_size_ptr=0x%x,\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue_id, argv->kernel_id, argv->work_dim,
			argv->global_work_offset_ptr, argv->global_work_size_ptr,
			argv->local_work_size_ptr, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->global_work_offset_ptr, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_RANGE(argv->work_dim, 1, 3);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = evg_opencl_repo_get_object(evg_emu->opencl_repo,
			evg_opencl_object_command_queue, argv->command_queue_id);

	/* Suspend context until command queue is empty */
	X86ContextSuspend(ctx, evg_opencl_command_queue_can_wakeup, command_queue,
			evg_opencl_clEnqueueNDRangeKernel_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * Rest of not implemented OpenCL API
 */

#define __EVG_OPENCL_NOT_IMPL__(_name) \
	int evg_opencl_##_name##_impl(X86Context *ctx, int *argv) \
	{ \
		fatal("%s: OpenCL function not implemented.\n%s", __FUNCTION__, \
			evg_err_opencl_note); \
		return 0; \
	}

__EVG_OPENCL_NOT_IMPL__(clRetainContext)
__EVG_OPENCL_NOT_IMPL__(clGetCommandQueueInfo)
__EVG_OPENCL_NOT_IMPL__(clSetCommandQueueProperty)
__EVG_OPENCL_NOT_IMPL__(clCreateSubBuffer)
__EVG_OPENCL_NOT_IMPL__(clGetSupportedImageFormats)
__EVG_OPENCL_NOT_IMPL__(clGetMemObjectInfo)
__EVG_OPENCL_NOT_IMPL__(clGetImageInfo)
__EVG_OPENCL_NOT_IMPL__(clSetMemObjectDestructorCallback)
__EVG_OPENCL_NOT_IMPL__(clRetainSampler)
__EVG_OPENCL_NOT_IMPL__(clReleaseSampler)
__EVG_OPENCL_NOT_IMPL__(clGetSamplerInfo)
__EVG_OPENCL_NOT_IMPL__(clUnloadCompiler)
__EVG_OPENCL_NOT_IMPL__(clGetProgramInfo)
__EVG_OPENCL_NOT_IMPL__(clGetProgramBuildInfo)
__EVG_OPENCL_NOT_IMPL__(clCreateKernelsInProgram)
__EVG_OPENCL_NOT_IMPL__(clRetainKernel)
__EVG_OPENCL_NOT_IMPL__(clGetKernelInfo)
__EVG_OPENCL_NOT_IMPL__(clCreateUserEvent)
__EVG_OPENCL_NOT_IMPL__(clRetainEvent)
__EVG_OPENCL_NOT_IMPL__(clSetUserEventStatus)
__EVG_OPENCL_NOT_IMPL__(clSetEventCallback)
__EVG_OPENCL_NOT_IMPL__(clEnqueueReadBufferRect)
__EVG_OPENCL_NOT_IMPL__(clEnqueueWriteBufferRect)
__EVG_OPENCL_NOT_IMPL__(clEnqueueCopyBufferRect)
__EVG_OPENCL_NOT_IMPL__(clEnqueueWriteImage)
__EVG_OPENCL_NOT_IMPL__(clEnqueueCopyImage)
__EVG_OPENCL_NOT_IMPL__(clEnqueueCopyImageToBuffer)
__EVG_OPENCL_NOT_IMPL__(clEnqueueCopyBufferToImage)
__EVG_OPENCL_NOT_IMPL__(clEnqueueMapImage)
__EVG_OPENCL_NOT_IMPL__(clEnqueueUnmapMemObject)
__EVG_OPENCL_NOT_IMPL__(clEnqueueTask)
__EVG_OPENCL_NOT_IMPL__(clEnqueueNativeKernel)
__EVG_OPENCL_NOT_IMPL__(clEnqueueMarker)
__EVG_OPENCL_NOT_IMPL__(clEnqueueWaitForEvents)
__EVG_OPENCL_NOT_IMPL__(clEnqueueBarrier)
__EVG_OPENCL_NOT_IMPL__(clGetExtensionFunctionAddress)

