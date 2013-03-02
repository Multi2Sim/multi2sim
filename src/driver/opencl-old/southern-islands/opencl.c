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

#include <arch/southern-islands/asm/bin-file.h>
#include <arch/southern-islands/emu/emu.h>
#include <arch/southern-islands/emu/ndrange.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <driver/opencl-old/evergreen/opencl.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
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
#define si_SYS_OPENCL_IMPL_VERSION_MAJOR	1
#define si_SYS_OPENCL_IMPL_VERSION_MINOR	0
#define si_SYS_OPENCL_IMPL_VERSION_BUILD	0
#define si_SYS_OPENCL_IMPL_VERSION		((si_SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(si_SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						si_SYS_OPENCL_IMPL_VERSION_BUILD)


/* Debug info */
int si_opencl_debug_category;

void si_opencl_debug_array(int nelem, int *array)
{
	char *comma = "";
	int i;

	si_opencl_debug("{");
	for (i = 0; i < nelem; i++)
	{
		si_opencl_debug("%s%d", comma, array[i]);
		comma = ", ";
	}
	si_opencl_debug("}");
}


/* List of OpenCL function names */
char *si_opencl_func_name[] = {
#define DEF_OPENCL_FUNC(_name, _argc) #_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	""
};


/* Number of arguments for each OpenCL function */
int si_opencl_func_argc[] = {
#define DEF_OPENCL_FUNC(_name, _argc) _argc,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	0
};


/* Forward declarations of OpenCL functions */
#define DEF_OPENCL_FUNC(_name, _argc) int si_opencl_##_name##_impl(struct x86_ctx_t *ctx, int *argv);
#include "opencl.dat"
#undef DEF_OPENCL_FUNC


/* Table of OpenCL function implementations */
typedef int (*si_opencl_func_impl_t)(struct x86_ctx_t *ctx, int *argv);
si_opencl_func_impl_t si_opencl_func_impl[] = {
#define DEF_OPENCL_FUNC(_name, _argc) si_opencl_##_name##_impl,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	NULL
};


/* Error messages */
char *si_err_opencl_note =
	"\tThe OpenCL interface is implemented in library 'm2s-opencl.so' as a set of\n"
	"\tsystem calls, intercepted by Multi2Sim and emulated in 'opencl.c'.\n"
	"\tHowever, only a subset of this interface is currently implemented in the simulator.\n"
	"\tTo request the implementation of a specific OpenCL call, please email\n"
	"\t'development@multi2sim.org'.\n";

char *si_err_opencl_param_note =
	"\tNote that a real OpenCL implementation would return an error code to the\n"
	"\tcalling program. However, the purpose of this OpenCL implementation is to\n"
	"\tsupport correctly written programs. Thus, a detailed implementation of OpenCL\n"
	"\terror handling is not provided, and any OpenCL error will cause the\n"
	"\tsimulation to stop.\n";

char *si_err_opencl_compiler =
	"\tThe Multi2Sim implementation of the OpenCL interface does not support runtime\n"
	"\tcompilation of kernel sources. To run OpenCL kernels, you should first compile\n"
	"\tthem off-line using a Southern Islands-compatible target device. Then, you have three\n"
	"\toptions to load them:\n"
	"\t  1) Replace 'clCreateProgramWithSource' calls by 'clCreateProgramWithBinary'\n"
	"\t     in your source files, referencing the pre-compiled kernel.\n"
	"\t  2) Tell Multi2Sim to provide the application with your pre-compiled kernel\n"
	"\t     using command-line option '--si-kernel-binary'.\n"
	"\t  3) If you are trying to run one of the OpenCL benchmarks provided in the\n"
	"\t     simulator website, option '--load' can be used as a program argument\n"
	"\t     (not a simulator argument). This option allows you to specify the path\n"
	"\t     for the pre-compiled kernel, which is provided in the downloaded package.\n";
	
char *si_err_opencl_binary_note =
	"\tYou have selected a pre-compiled OpenCL kernel binary to be passed to your\n"
	"\tOpenCL application when it requests a kernel compilation. It is your\n"
	"\tresponsibility to check that the chosen binary corresponds to the kernel\n"
	"\tthat your application is expecting to load.\n";

char *si_err_opencl_version_note =
	"\tThe version of the Multi2Sim OpenCL library ('libm2s-opencl') that your program\n"
	"\tis using is too old. You need to re-link your program with a version of the\n"
	"\tlibrary compatible for this Multi2Sim release. Please see the Multi2Sim Guide\n"
	"\tfor details (www.multi2sim.org).\n";


/* Error macros */

#define SI_OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", \
		__FUNCTION__, p, si_err_opencl_note);
#define SI_OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", \
		__FUNCTION__, (v), si_err_opencl_param_note); }
#define SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", \
		__FUNCTION__, (v), si_err_opencl_param_note); }
#define SI_OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", \
		__FUNCTION__, (v), si_err_opencl_param_note); }
#define SI_OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) \
		fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
		__FUNCTION__, (min), (max), si_err_opencl_param_note); }
#define SI_OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", \
		__FUNCTION__, si_err_opencl_param_note); }



/*
 * Common entry point for Southern Islands and Evergreen
 */

int opencl_old_abi_call(struct x86_ctx_t *ctx)
{
	switch (x86_emu->gpu_kind)
	{
	case x86_emu_gpu_evergreen:
		return evg_opencl_api_run(ctx);

	case x86_emu_gpu_southern_islands:
		return si_opencl_api_run(ctx);
	
	default:
		panic("%s: invalid GPU kind", __FUNCTION__);
		return 0;
	}
}






/*
 * Entry point for OpenCL API
 */

int si_opencl_api_run(struct x86_ctx_t *ctx)
{
	int argv[SI_OPENCL_MAX_ARGS];
	int code;
	int ret;

	/* Get function code and arguments */
	code = si_opencl_api_read_args(ctx, NULL, argv, sizeof argv);
	assert(IN_RANGE(code, SI_OPENCL_FUNC_FIRST, SI_OPENCL_FUNC_LAST));
	
	/* Call function */
	si_opencl_debug("%s\n", si_opencl_func_name[code-SI_OPENCL_FUNC_FIRST]);
	ret = si_opencl_func_impl[code - SI_OPENCL_FUNC_FIRST](ctx, argv);

	/* Return OpencL result */
	return ret;
}


/* Return OpenCL function arguments, as identified in the current state
 * of the x86 context stack and registers. The value returned by the function
 * is the OpenCL function code identified by register 'ebx'. */
int si_opencl_api_read_args(struct x86_ctx_t *ctx, int *argc_ptr,
	void *argv_ptr, int argv_size)
{
	struct mem_t *mem = ctx->mem;
	struct x86_regs_t *regs = ctx->regs;

	unsigned int argv_guest_ptr;

	int func_code;
	int argc;

	/* Read function code */
	func_code = regs->ebx;
	if (func_code < SI_OPENCL_FUNC_FIRST || 
		func_code > SI_OPENCL_FUNC_LAST)
	{
		fatal("%s: invalid OpenCL function code - %d\n",
			__FUNCTION__, func_code);
	}

	/* Get number of arguments */
	argc = si_opencl_func_argc[func_code - SI_OPENCL_FUNC_FIRST];
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


/* Set return value of an OpenCL API call. This needs to be done explicitly 
 * when a context gets suspended during the execution of the OpenCL call, 
 * and later the wake-up call-back routine finishes the OpenCL call 
 * execution. */
void si_opencl_api_return(struct x86_ctx_t *ctx, int value)
{
	ctx->regs->eax = value;
}



/*
 * OpenCL call 'clGetPlatformIDs' (code 1000)
 */

int si_opencl_clGetPlatformIDs_impl(struct x86_ctx_t *ctx, int *argv)
{
	int num_entries = argv[0];  /* cl_uint num_entries */
	unsigned int platforms = argv[1];  /* cl_platform_id *platforms */
	unsigned int num_platforms = argv[2];  /* cl_uint *num_platforms */
	unsigned int opencl_impl_version = argv[3];  /* Implementation-specific */

	unsigned int one = 1;
	int opencl_impl_version_major = opencl_impl_version >> 16;
	int opencl_impl_version_minor = (opencl_impl_version >> 8) & 0xff;
	int opencl_impl_version_build = opencl_impl_version & 0xff;

	/* Check 'libm2s-opencl' version */
	if (opencl_impl_version < si_SYS_OPENCL_IMPL_VERSION)
	{
		fatal("wrong Multi2Sim OpenCL library version "
			"(provided=%d.%d.%d, required=%d.%d.%d).\n%s",
			opencl_impl_version_major, opencl_impl_version_minor, 
			opencl_impl_version_build, 
			si_SYS_OPENCL_IMPL_VERSION_MAJOR, 
			si_SYS_OPENCL_IMPL_VERSION_MINOR,
			si_SYS_OPENCL_IMPL_VERSION_BUILD, 
			si_err_opencl_version_note);
	}
	si_opencl_debug("  'libm2s-opencl' version: %d.%d.%d\n",
		opencl_impl_version_major, opencl_impl_version_minor, 
		opencl_impl_version_build);

	/* Get platform id */
	si_opencl_debug("  num_entries=%d, platforms=0x%x, "
		"num_platforms=0x%x, version=0x%x\n", num_entries, platforms, 
		num_platforms, opencl_impl_version);
	if (num_platforms)
		mem_write(ctx->mem, num_platforms, 4, &one);
	if (platforms && num_entries > 0)
	{
		mem_write(ctx->mem, platforms, 4, 
			&si_emu->opencl_platform->id);
	}
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetPlatformInfo' (code 1001)
 */

int si_opencl_clGetPlatformInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int platform_id = argv[0];  /* cl_platform_id platform */
	unsigned int param_name = argv[1];  /* cl_platform_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct si_opencl_platform_t *platform;
	unsigned int size_ret;

	si_opencl_debug("  platform=0x%x, param_name=0x%x, "
		"param_value_size=0x%x,\n"
		"  param_value=0x%x, param_value_size_ret=0x%x\n",
		platform_id, param_name, param_value_size, param_value,
		param_value_size_ret);

	platform = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_platform, platform_id);
	size_ret = si_opencl_platform_get_info(platform, param_name,
		ctx->mem, param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(ctx->mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetDeviceIDs' (code 1002)
 */

int si_opencl_clGetDeviceIDs_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int platform = argv[0];  /* cl_platform_id platform */
	int device_type = argv[1];  /* cl_device_type device_type */
	int num_entries = argv[2];  /* cl_uint num_entries */
	unsigned int devices = argv[3];  /* cl_device_id *devices */
	unsigned int num_devices = argv[4];  /* cl_uint *num_devices */
	unsigned int one = 1;
	struct si_opencl_device_t *device;

	si_opencl_debug("  platform=0x%x, device_type=%d, num_entries=%d\n",
		platform, device_type, num_entries);
	si_opencl_debug("  devices=0x%x, num_devices=%x\n",
		devices, num_devices);
	if (platform != si_emu->opencl_platform->id)
		fatal("%s: invalid platform\n%s", __FUNCTION__,
			si_err_opencl_param_note);

	/* Return 1 in 'num_devices' */
	if (num_devices)
		mem_write(ctx->mem, num_devices, 4, &one);

	/* Return 'id' of the only existing device */
	if (devices && num_entries > 0)
	{
		device = si_opencl_repo_get_object_of_type(
			si_emu->opencl_repo, si_opencl_object_device);
		if (!device)
			panic("%s: no device", __FUNCTION__);
		mem_write(ctx->mem, devices, 4, &device->id);
	}

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetDeviceInfo' (code 1003)
 */

int si_opencl_clGetDeviceInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int device_id = argv[0];  /* cl_device_id device */
	unsigned int param_name = argv[1];  /* cl_device_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct si_opencl_device_t *device;
	unsigned int size_ret;

	si_opencl_debug("  device=0x%x, param_name=0x%x, "
			"param_value_size=%d\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			device_id, param_name, param_value_size, param_value,
			param_value_size_ret);

	device = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_device, device_id);
	size_ret = si_opencl_device_get_info(device, param_name, ctx->mem,
		param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(ctx->mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateContext' (code 1004)
 */

int si_opencl_clCreateContext_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int properties = argv[0];  /* const cl_context_properties *properties */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int devices = argv[2];  /* const cl_device_id *devices */
	unsigned int pfn_notify = argv[3];  /* void (CL_CALLBACK *pfn_notify)(const char *errinfo,
					       const void *private_info, size_t cb, void *user_data) */
	unsigned int user_data = argv[4];  /* void *user_data */
	unsigned int errcode_ret = argv[5];  /* cl_int *errcode_ret */

	struct si_opencl_device_t *device;
	struct si_opencl_context_t *context;
	
	unsigned int device_id;
	int zero = 0;

	si_opencl_debug("  properties=0x%x, num_devices=%d, devices=0x%x\n"
			"pfn_notify=0x%x, user_data=0x%x, errcode_ret=0x%x\n",
			properties, num_devices, devices, pfn_notify, 
			user_data, errcode_ret);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	SI_OPENCL_ARG_NOT_SUPPORTED_EQ(devices, 0);

	/* Read device id */
	mem_read(ctx->mem, devices, 4, &device_id);
	device = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_device, device_id);
	if (!device)
	{
		fatal("%s: invalid device\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

	/* Create context and return id */
	context = si_opencl_context_create();
	si_opencl_context_set_properties(context, ctx->mem, properties);
	context->device_id = device_id;

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);
	
	/* Return context */
	return context->id;
}




/*
 * OpenCL call 'clCreateContextFromType' (code 1005)
 */

int si_opencl_clCreateContextFromType_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int properties = argv[0];  /* const cl_context_properties *properties */
	unsigned int device_type = argv[1];  /* cl_device_type device_type */
	unsigned int pfn_notify = argv[2];  /* void (*pfn_notify)(const char *, const void *, size_t , void *) */
	unsigned int user_data = argv[3];  /* void *user_data */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct si_opencl_device_t *device;
	struct si_opencl_context_t *context;

	int zero = 0;

	si_opencl_debug("  properties=0x%x, device_type=0x%x, "
			"pfn_notify=0x%x,\n"
			"  user_data=0x%x, errcode_ret=0x%x\n",
			properties, device_type, pfn_notify, user_data, 
			errcode_ret);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);

	/* Get device */
	device = si_opencl_repo_get_object_of_type(si_emu->opencl_repo,
			si_opencl_object_device);
	assert(device);

	/* Create context */
	context = si_opencl_context_create();
	context->device_id = device->id;
	si_opencl_context_set_properties(context, ctx->mem, properties);

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);

	/* Return context */
	return context->id;
}




/*
 * OpenCL call 'clReleaseContext' (code 1007)
 */

int si_opencl_clReleaseContext_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */

	struct si_opencl_context_t *context;

	si_opencl_debug("  context=0x%x\n", context_id);
	context = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_context, context_id);

	/* Release context */
	assert(context->ref_count > 0);
	if (!--context->ref_count)
		si_opencl_context_free(context);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetContextInfo' (code 1008)
 */

int si_opencl_clGetContextInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int param_name = argv[1];  /* cl_context_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct si_opencl_context_t *context;
	unsigned int size_ret = 0;

	si_opencl_debug("  context=0x%x, param_name=0x%x, "
			"param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			context_id, param_name, param_value_size, param_value,
			param_value_size_ret);

	context = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_context, context_id);
	size_ret = si_opencl_context_get_info(context, param_name, ctx->mem,
		param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(ctx->mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateCommandQueue' (code 1009)
 */

int si_opencl_clCreateCommandQueue_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int device_id = argv[1];  /* cl_device_id device */
	unsigned int properties = argv[2];  /* cl_command_queue_properties properties */
	unsigned int errcode_ret = argv[3];  /* cl_int *errcode_ret */

	struct si_opencl_command_queue_t *command_queue;

	int zero = 0;

	si_opencl_debug("  context=0x%x, device=0x%x, properties=0x%x, "
		"errcode_ret=0x%x\n", context_id, device_id, properties, 
		errcode_ret);

	/* Check that context and device are valid */
	si_opencl_repo_get_object(si_emu->opencl_repo, 
		si_opencl_object_context, context_id);
	si_opencl_repo_get_object(si_emu->opencl_repo, 
		si_opencl_object_device, device_id);

	/* Create command queue and return id */
	command_queue = si_opencl_command_queue_create();
	command_queue->context_id = context_id;
	command_queue->device_id = device_id;
	command_queue->properties = properties;

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);
	
	/* Return command queue */
	return command_queue->id;
}




/*
 * OpenCL call 'clRetainCommandQueue' (code 1010)
 */

int si_opencl_clRetainCommandQueue_impl(struct x86_ctx_t *ctx, int *argv)
{
   	unsigned int command_queue_id = argv[0];  /* cl_command_queue command_queue */

	struct si_opencl_command_queue_t *command_queue;

	si_opencl_debug("  command_queue=0x%x\n", command_queue_id);

	/* Check that the command queue argument is valid */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, command_queue_id);

	/* Increase the reference count */
	++command_queue->ref_count;

	/* Return success */
	return 0;
}



/*
 * OpenCL call 'clReleaseCommandQueue' (code 1009)
 */

int si_opencl_clReleaseCommandQueue_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int command_queue_id = argv[0];  /* cl_command_queue command_queue */

	struct si_opencl_command_queue_t *command_queue;

	si_opencl_debug("  command_queue=0x%x\n", command_queue_id);
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, command_queue_id);

	/* Release command queue */
	assert(command_queue->ref_count > 0);
	if (!--command_queue->ref_count)
		si_opencl_command_queue_free(command_queue);

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

int si_opencl_clCreateBuffer_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int flags = argv[1];  /* cl_mem_flags flags */
	unsigned int size = argv[2];  /* size_t size */
	unsigned int host_ptr = argv[3];  /* void *host_ptr */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct si_opencl_mem_t *mem;

	char flags_str[MAX_STRING_SIZE];
	int zero = 0;
	void *buf;

	str_map_flags(&create_buffer_flags_map, flags, flags_str, 
		sizeof(flags_str));
	si_opencl_debug("  context=0x%x, flags=%s, size=%d, host_ptr=0x%x, "
		"errcode_ret=0x%x\n", context_id, flags_str, size, host_ptr, 
		errcode_ret);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
	{
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with "
			"CL_MEM_USE_HOST_PTR\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
	{
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
	{
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

	/* Create memory object */
	mem = si_opencl_mem_create();
	mem->type = 0;  /* FIXME */
	mem->size = size;
	mem->flags = flags;
	mem->host_ptr = host_ptr;

	/* Assign position in device global memory */
	mem->device_ptr = si_emu->global_mem_top;
	si_emu->global_mem_top += size;

	/* If 'host_ptr' was specified, copy buffer into device memory */
	if (host_ptr)
	{
		buf = xmalloc(size);
		mem_read(ctx->mem, host_ptr, size, buf);
		mem_write(si_emu->global_mem, mem->device_ptr, size, buf);
		free(buf);
	}

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);
	
	/* Return memory object */
	return mem->id;
}




/*
 * OpenCL call 'clCreateImage2D' (code 1016)
 */
	
static struct str_map_t si_opencl_create_image_flags_map =
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

int si_opencl_clCreateImage2D_impl(struct x86_ctx_t *ctx, int *argv)
{
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
	struct si_opencl_image_format_t image_format;

	unsigned int num_channels_per_pixel;
	unsigned int pixel_size;
	unsigned int size;
	int zero = 0;

	struct si_opencl_mem_t *mem;

	mem_read(ctx->mem, image_format_ptr, 8, &image_format);
	channel_order = image_format.image_channel_order;
	channel_type = image_format.image_channel_data_type;

	str_map_flags(&si_opencl_create_image_flags_map, flags, flags_str, 
		sizeof(flags_str));
	si_opencl_debug("  context=0x%x, flags=%s, channel order =0x%x, "
		"channel_type=0x%x, image_width=%u, image_height=%u, "
		"image_row_pitch=%u, host_ptr=0x%x, errcode_ret=0x%x\n",
		context_id, flags_str, channel_order, channel_type, 
		image_width, image_height, image_row_pitch, host_ptr, 
		errcode_ret_ptr);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
	{
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with "
			"CL_MEM_USE_HOST_PTR\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
	{
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
	{
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

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
			__FUNCTION__, channel_order, 
			si_err_opencl_param_note);
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
			__FUNCTION__, channel_type, si_err_opencl_param_note);
	}


	/* Determine image geometry */
	if (image_row_pitch == 0)
	{
		image_row_pitch = image_width*pixel_size;
	}
	else if (image_row_pitch < image_width*pixel_size)
	{
		fatal("%s: image_row_pitch must be 0 or >= image_width "
			"* size of element in bytes\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

	/* Create memory object */
	size = image_row_pitch * image_height;
	mem = si_opencl_mem_create();
	mem->type = 1;  /* FIXME */
	mem->size = size;
	mem->flags = flags;
	mem->host_ptr = host_ptr;
	mem->pixel_size = pixel_size;
	mem->num_pixels = size/pixel_size;
	mem->num_channels_per_pixel = num_channels_per_pixel;
	mem->width = image_width;
	mem->height = image_height;
	mem->depth = 1;

	/* Assign position in device global memory */
	mem->device_ptr = si_emu->global_mem_top;
	si_emu->global_mem_top += size;
	si_opencl_debug("  creating device ptr at %u, for %u bytes\n", 
		mem->device_ptr, size);

	/* If 'host_ptr' was specified, copy image into device memory */
	if (host_ptr)
	{
		image = xmalloc(size);

		mem_read(ctx->mem, host_ptr, size, image);
		mem_write(si_emu->global_mem, mem->device_ptr, size, image);
		free(image);
	}
	fflush(NULL);

	/* Return success */
	if (errcode_ret_ptr)
		mem_write(ctx->mem, errcode_ret_ptr, 4, &zero);

	/* Return memory object */
	return mem->id;
}




/*
 * OpenCL call 'clCreateImage3D' (code 1017)
 */

int si_opencl_clCreateImage3D_impl(struct x86_ctx_t *ctx, int *argv)
{
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
	struct si_opencl_image_format_t image_format;

	unsigned int num_channels_per_pixel;
	unsigned int pixel_size;

	struct si_opencl_mem_t *mem;
	unsigned int size;

	int zero = 0;

	mem_read(ctx->mem, image_format_ptr, 8, &image_format);
	channel_order = image_format.image_channel_order;
	channel_type = image_format.image_channel_data_type;

	str_map_flags(&si_opencl_create_image_flags_map, flags, flags_str, 
		sizeof(flags_str));
	si_opencl_debug("  context=0x%x, flags=%s, channel order =0x%x, "
		"channel_type=0x%x\n"
		"  image_width=%u, image_height=%u, image_depth=%u\n"
		"  image_row_pitch=%u, image_slice_pitch=%u, host_ptr=0x%x\n"
		"  errcode_ret=0x%x\n",
		context_id, flags_str, channel_order, channel_type, 
		image_width, image_height, image_depth, 
		image_row_pitch, image_slice_pitch, host_ptr, 
		errcode_ret_ptr);

	/* Check flags */
	if ((flags & 0x10) && host_ptr)
	{
		fatal("%s: CL_MEM_ALLOC_HOST_PTR not compatible with "
			"CL_MEM_USE_HOST_PTR\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x8) && !host_ptr)  /* CL_MEM_USE_HOST_PTR */
	{
		fatal("%s: CL_MEM_USE_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}
	if ((flags & 0x20) && !host_ptr)  /* CL_MEM_COPY_HOST_PTR */
	{
		fatal("%s: CL_MEM_COPY_HOST_PTR only valid when 'host_ptr' "
			"!= NULL\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

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
			__FUNCTION__, channel_order, 
			si_err_opencl_param_note);
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
			__FUNCTION__, channel_type, si_err_opencl_param_note);
	}

	/* Determine image geometry */
	if (image_row_pitch == 0)
		image_row_pitch = image_width*pixel_size;
	else if (image_row_pitch < image_width*pixel_size)
		fatal("%s: image_row_pitch must be 0 or >= "
			"image_width * size of element in bytes\n%s", 
			__FUNCTION__, si_err_opencl_param_note);

	if (image_slice_pitch == 0)
		image_slice_pitch = image_row_pitch*image_height;
	else if (image_slice_pitch < image_row_pitch*image_height)
		fatal("%s: image_slice_pitch must be 0 or >= "
			"image_row_pitch * image_height\n%s", 
			__FUNCTION__, si_err_opencl_param_note);

	/* Create memory object */
	size = image_slice_pitch*image_depth;
	mem = si_opencl_mem_create();
	mem->type = 2; /* FIXME */
	mem->size = size;
	mem->flags = flags;
	mem->host_ptr = host_ptr;
	mem->num_pixels = size/pixel_size;
	mem->pixel_size = pixel_size;
	mem->num_channels_per_pixel = num_channels_per_pixel;
	mem->width = image_width;
	mem->height = image_height;
	mem->depth = image_depth;

	/* Assign position in device global memory */
	mem->device_ptr = si_emu->global_mem_top;
	si_emu->global_mem_top += size;

	/* If 'host_ptr' was specified, copy image into device memory */
	if (host_ptr)
	{
		image = xmalloc(size);

		mem_read(ctx->mem, host_ptr, size, image);
		mem_write(si_emu->global_mem, mem->device_ptr, size, image);
		free(image);
	}

	/* Return success */
	if (errcode_ret_ptr)
		mem_write(ctx->mem, errcode_ret_ptr, 4, &zero);

	/* Return memory object */
	return mem->id;
}




/*
 * OpenCL call 'clRetainMemObject' (code 1018)
 */

int si_opencl_clRetainMemObject_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int mem_id = argv[0];  /* cl_mem memobj */

	struct si_opencl_mem_t *mem;

	si_opencl_debug("  memobj=0x%x\n", mem_id);
	mem = si_opencl_repo_get_object(si_emu->opencl_repo, 
		si_opencl_object_mem, mem_id);

	/* Increase the reference count */
	++mem->ref_count;

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clReleaseMemObject' (code 1019)
 */

int si_opencl_clReleaseMemObject_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int mem_id = argv[0];  /* cl_mem memobj */

	struct si_opencl_mem_t *mem;

	si_opencl_debug("  memobj=0x%x\n", mem_id);
	mem = si_opencl_repo_get_object(si_emu->opencl_repo, 
		si_opencl_object_mem, mem_id);

	/* Release object */
	assert(mem->ref_count > 0);
	if (!--mem->ref_count)
		si_opencl_mem_free(mem);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateSampler' (code 1024)
 */

int si_opencl_clCreateSampler_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context = argv[0];  /* cl_context context */
	unsigned int normalized_coords = argv[1];  /* cl_bool normalized_coords */
	unsigned int addressing_mode = argv[2];  /* cl_addressing_mode addressing_mode */
	unsigned int filter_mode = argv[3];  /* cl_filter_mode filter_mode */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct si_opencl_sampler_t *sampler;

	int zero = 0;

	si_opencl_debug("  context=0x%x, normalized_coords=%d, addressing_mode=0x%x,"
			" lengths=0x%x, errcode_ret=0x%x\n",
			context, normalized_coords, addressing_mode, filter_mode, errcode_ret);

	if (normalized_coords != 0)
		fatal("%s: Normalized coordinates are not supported.\n", __FUNCTION__);

	if (filter_mode != 0x1140)  /* only CL_FILTER_NEAREST supported */
		fatal("%s: filter mode %u not supported.\n", __FUNCTION__, filter_mode);

	if (addressing_mode != 0x1130) /* only CL_ADDRESS_NONE supported */
		fatal("%s: addressing mode %u not supported.\n", __FUNCTION__, 
				addressing_mode);

	si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_context, context);

	/* Create command queue and return id */
	sampler = si_opencl_sampler_create();
	sampler->normalized_coords = normalized_coords;
	sampler->addressing_mode = addressing_mode;
	sampler->filter_mode = filter_mode;

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);
	
	/* Return sampler ID */
	return sampler->id;
}




/*
 * OpenCL call 'clCreateProgramWithSource' (code 1028)
 */

int si_opencl_clCreateProgramWithSource_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int count = argv[1];  /* cl_uint count */
	unsigned int strings = argv[2];  /* const char **strings */
	unsigned int lengths = argv[3];  /* const size_t *lengths */
	unsigned int errcode_ret = argv[4];  /* cl_int *errcode_ret */

	struct si_opencl_program_t *program;

	si_opencl_debug("  context=0x%x, count=%d, strings=0x%x, lengths=0x%x, errcode_ret=0x%x\n",
			context_id, count, strings, lengths, errcode_ret);

	/* Application tries to compile source, and no binary was passed to Multi2Sim */
	if (!*si_emu_opencl_binary_name)
		fatal("%s: kernel source compilation not supported.\n%s",
				__FUNCTION__, si_err_opencl_compiler);

	/* Create program */
	si_opencl_repo_get_object(si_emu->opencl_repo, si_opencl_object_context, context_id);
	program = si_opencl_program_create();
	warning("%s: binary '%s' used as pre-compiled kernel.\n%s",
			__FUNCTION__, si_emu_opencl_binary_name, si_err_opencl_binary_note);

	/* Load OpenCL binary passed to Multi2Sim and make a copy in temporary file */
	program->elf_file = elf_file_create_from_path(si_emu_opencl_binary_name);

	/* Return program */
	return program->id;
}




/*
 * OpenCL call 'clCreateProgramWithBinary' (code 1029)
 */

int si_opencl_clCreateProgramWithBinary_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int context_id = argv[0];  /* cl_context context */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int device_list = argv[2];  /* const cl_device_id *device_list */
	unsigned int lengths = argv[3];  /* const size_t *lengths */
	unsigned int binaries = argv[4];  /* const unsigned char **binaries */
	unsigned int binary_status = argv[5];  /* cl_int *binary_status */
	unsigned int errcode_ret = argv[6];  /* cl_int *errcode_ret */

	unsigned int length, binary;
	unsigned int device_id;

	struct si_opencl_program_t *program;
	void *buf;

	int zero = 0;

	char name[MAX_STRING_SIZE];

	si_opencl_debug("  context=0x%x, num_devices=%d, device_list=0x%x, lengths=0x%x\n"
			"  binaries=0x%x, binary_status=0x%x, errcode_ret=0x%x\n",
			context_id, num_devices, device_list, lengths, binaries,
			binary_status, errcode_ret);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	/* Get device and context */
	mem_read(ctx->mem, device_list, 4, &device_id);
	si_opencl_repo_get_object(si_emu->opencl_repo, si_opencl_object_device,
		device_id);
	si_opencl_repo_get_object(si_emu->opencl_repo, 
		si_opencl_object_context, context_id);

	/* Create program */
	program = si_opencl_program_create();

	/* Read binary length and pointer */
	mem_read(ctx->mem, lengths, 4, &length);
	mem_read(ctx->mem, binaries, 4, &binary);
	si_opencl_debug("    lengths[0] = %d\n", length);
	si_opencl_debug("    binaries[0] = 0x%x\n", binary);

	/* Read binary */
	buf = xmalloc(length);
	mem_read(ctx->mem, binary, length, buf);

	/* Load ELF binary from guest memory */
	snprintf(name, sizeof(name), "clProgram<%d>.externalELF", program->id);
	program->elf_file = elf_file_create_from_buffer(buf, length, name);

	/* Search ELF binary to see if there are any constant buffers 
	 * encoded inside */
	si_opencl_program_initialize_constant_buffers(program);

	free(buf);

	/* Return success */
	if (binary_status)
		mem_write(ctx->mem, binary_status, 4, &zero);
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);

	/* Return program */
	return program->id;
}




/*
 * OpenCL call 'clRetainProgram' (code 1030)
 */

int si_opencl_clRetainProgram_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */

	struct si_opencl_program_t *program;

	si_opencl_debug("  program=0x%x\n", program_id);
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_program, program_id);

	/* Increase the reference count */
	++program->ref_count;
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clReleaseProgram' (code 1031)
 */

int si_opencl_clReleaseProgram_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */

	struct si_opencl_program_t *program;

	si_opencl_debug("  program=0x%x\n", program_id);
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_program, program_id);

	/* Release program */
	assert(program->ref_count > 0);
	if (!--program->ref_count)
		si_opencl_program_free(program);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clBuildProgram' (code 1032)
 */

int si_opencl_clBuildProgram_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */
	unsigned int num_devices = argv[1];  /* cl_uint num_devices */
	unsigned int device_list = argv[2];  /* const cl_device_id *device_list */
	unsigned int options = argv[3];  /* const char *options */
	unsigned int pfn_notify = argv[4];  /* void (CL_CALLBACK *pfn_notify)(cl_program program,
					       void *user_data) */
	unsigned int user_data = argv[5];  /* void *user_data */

	struct si_opencl_program_t *program;
	char options_str[MAX_STRING_SIZE];

	options_str[0] = 0;
	if (options)
	{
		mem_read_string(ctx->mem, options, MAX_STRING_SIZE, 
			options_str);
	}

	si_opencl_debug("  program=0x%x, num_devices=%d, device_list=0x%x, " 
			"options=0x%x\n"
			"  pfn_notify=0x%x, user_data=0x%x, options='%s'\n",
			program_id, num_devices, device_list, options, 
			pfn_notify, user_data, options_str);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(pfn_notify, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(user_data, 0);
	if (options_str[0])
	{
		warning("%s: clBuildProgram: option string '%s' ignored\n",
				__FUNCTION__, options_str);
	}

	/* Get program */
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_program, program_id);
	if (!program->elf_file)
		fatal("%s: program binary must be loaded first.\n%s",
				__FUNCTION__, si_err_opencl_param_note);

	/* Build program */
	si_opencl_program_build(program);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clCreateKernel' (code 1036)
 */

int si_opencl_clCreateKernel_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int program_id = argv[0];  /* cl_program program */
	unsigned int kernel_name = argv[1];  /* const char *kernel_name */
	unsigned int errcode_ret = argv[2];  /* cl_int *errcode_ret */

	char kernel_name_str[MAX_STRING_SIZE];

	struct si_opencl_kernel_t *kernel;
	struct si_opencl_program_t *program;

	void *constant_tmp;

	int zero = 0;
	int i;

	si_opencl_debug("  program=0x%x, kernel_name=0x%x, errcode_ret=0x%x\n",
			program_id, kernel_name, errcode_ret);
	if (mem_read_string(ctx->mem, kernel_name, MAX_STRING_SIZE, 
		kernel_name_str) == MAX_STRING_SIZE)
	{
		fatal("%s: 'kernel_name' string is too long", __FUNCTION__);
	}
	si_opencl_debug("    kernel_name='%s'\n", kernel_name_str);

	/* Get program */
	program = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_program, program_id);

	/* Create the kernel */
	kernel = si_opencl_kernel_create(kernel_name_str);
	kernel->program_id = program_id;

	/* Program must be built */
	if (!program->elf_file)
	{
		fatal("%s: program should be first built with "
			"clBuildProgram.\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

	/* Load kernel */
	si_opencl_kernel_load(kernel, kernel_name_str);

	/* Add program-wide constant buffers to the kernel-specific list */
	for (i = 0; i < 25; i++) 
	{
		constant_tmp = list_get(program->constant_buffer_list, i);
		if (constant_tmp) 
			list_set(kernel->constant_buffer_list, i, constant_tmp);
	}

	/* Return success */
	if (errcode_ret)
		mem_write(ctx->mem, errcode_ret, 4, &zero);
	
	/* Return kernel */
	return kernel->id;
}




/*
 * OpenCL call 'clReleaseKernel' (code 1039)
 */

int si_opencl_clReleaseKernel_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */

	struct si_opencl_kernel_t *kernel;

	si_opencl_debug("  kernel=0x%x\n", kernel_id);
	kernel = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_kernel, kernel_id);

	/* Release kernel */
	assert(kernel->ref_count > 0);
	if (!--kernel->ref_count)
		si_opencl_kernel_free(kernel);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clSetKernelArg' (code 1040)
 */

int si_opencl_clSetKernelArg_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */
	unsigned int arg_index = argv[1];  /* cl_uint arg_index */
	unsigned int arg_size = argv[2];  /* size_t arg_size */
	unsigned int arg_value = argv[3];  /* const void *arg_value */

	struct si_opencl_kernel_t *kernel;
	struct si_opencl_kernel_arg_t *arg;

	si_opencl_debug("  kernel_id=0x%x, arg_index=%d, arg_size=%d, "
		"arg_value=0x%x\n", kernel_id, arg_index, arg_size, arg_value);

	/* Check */
	kernel = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_kernel, kernel_id);
	if (arg_index >= list_count(kernel->arg_list))
		fatal("%s: argument index out of bounds.\n%s", __FUNCTION__,
			si_err_opencl_param_note);

	if (arg_size >= SI_OPENCL_KERNEL_ARG_MAX_SIZE)
		fatal("%s: kernel argument is too large.\n%s", __FUNCTION__,
			si_err_opencl_param_note);

	/* XXX Need to implement as described in Metadata.pdf */
	/* 1) Scalar arguments are stored in CB1 at offset specified by
	 *    metadata.  If the argument is over 4-bytes, it gets stored in
	 *    successive elements of the vector register.  16-byte aligned.
	 * 2) 32-bit pointer arguments store the offset into the memory space
	 *    in the first 4-bytes (VERIFY with descriptors). 16-byte aligned.
	 * 3) 64-bit pointers are the same as 32-bit, except that the upper
	 *    32-bits are stored in the second 32-bits of the CB.
	 * 4) Aggregate arguments (structs and unions) are similar to scalar
	 *    arguments.
	 */

	/* Copy to kernel argument to OpenCL structure */
	arg = list_get(kernel->arg_list, arg_index);
	assert(arg);
	arg->set = 1;

	if (arg->kind == SI_OPENCL_KERNEL_ARG_KIND_VALUE)
	{
		assert(arg_value);
		mem_read(ctx->mem, arg_value, arg_size, 
			&arg->value.value[0]);
	}
	else if (arg->kind == SI_OPENCL_KERNEL_ARG_KIND_POINTER)
	{
		/* Global memory that we know how to handle */
		if ((arg->pointer.mem_type == 
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_GLOBAL) ||
			(arg->pointer.mem_type == 
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_UAV) || 
			(arg->pointer.mem_type == 
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_CONSTANT))

		{
			assert(arg_size == 4);
			mem_read(ctx->mem, arg_value, arg_size, 
				&arg->pointer.mem_obj_id);
		}
		/* Local memory */
		else if (arg->pointer.mem_type == 
			SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL)
		{
			if (arg_value)
			{
				/* If OpenCL argument scope is __local, 
				 * argument value must be NULL */
				fatal("%s: value for local arguments must "
					"be NULL.\n%s", __FUNCTION__, 
					si_err_opencl_param_note);
			}

			/* Do nothing */
		}
		/* Types that we don't know how to handle yet */
		else
		{
			fatal("%s: Unexpected mem type.\n%s", __FUNCTION__,
			si_err_opencl_param_note);
		}
	}


	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetKernelWorkGroupInfo' (code 1042)
 */

int si_opencl_clGetKernelWorkGroupInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int kernel_id = argv[0];  /* cl_kernel kernel */
	unsigned int device_id = argv[1];  /* cl_device_id device */
	unsigned int param_name = argv[2];  /* cl_kernel_work_group_info param_name */
	unsigned int param_value_size = argv[3];  /* size_t param_value_size */
	unsigned int param_value = argv[4];  /* void *param_value */
	unsigned int param_value_size_ret = argv[5];  /* size_t *param_value_size_ret */

	struct si_opencl_kernel_t *kernel;
	unsigned int size_ret;

	si_opencl_debug("  kernel=0x%x, device=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			kernel_id, device_id, param_name, param_value_size, param_value,
			param_value_size_ret);

	kernel = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_kernel, kernel_id);
	si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_device, device_id);
	size_ret = si_opencl_kernel_get_work_group_info(kernel, param_name, ctx->mem,
			param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(ctx->mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clWaitForEvents' (code 1043)
 */

struct si_opencl_clWaitForEvents_args_t
{
	unsigned int num_events;  /* cl_uint num_events */
	unsigned int event_list;  /* const cl_event *event_list */
};

void si_opencl_clWaitForEvents_wakeup(struct x86_ctx_t *ctx, void *data)
{
	/* Pop the next event to wait for. */
	struct list_t* events = (struct list_t *)data;
	struct si_opencl_event_t* event = list_pop(events);

	/* If there are no more events to wait for, finish */
	if (!event)
	{
		/* Free the list */
		list_free(events);

		/* Return success */
		si_opencl_api_return(ctx, 0);
		si_opencl_debug("  All events completed.\n");
		return;
	}

	/* Suspend context until event is complete, and wake up to wait for the next event. */
	si_opencl_debug("  waiting for event:0x%x\n", event->id);
	x86_ctx_suspend(ctx, si_opencl_event_can_wakeup, event,
		si_opencl_clWaitForEvents_wakeup, events);
}

int si_opencl_clWaitForEvents_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clWaitForEvents_args_t *argv = (struct si_opencl_clWaitForEvents_args_t *) argv_ptr;

	si_opencl_debug("  waiting for event list:0x%x, length:0x%x\n", argv->event_list, argv->num_events);

	/* Create list of events in guest memory. */
	struct list_t* events = list_create();
	struct si_opencl_event_t* event;
	for(int i = 0; i < argv->num_events; i++)
	{
		/* Read the event from memory */
		int event_ptr = argv->event_list + i * sizeof(struct si_opencl_event_t);
		int event_id;
		mem_read(ctx->mem, event_ptr, 4, &event_id);
		event = si_opencl_repo_get_object(si_emu->opencl_repo,
					si_opencl_object_event, event_id);

		/* Add the event to the list */
		list_add(events, event);
	}

	/* Wait for all events. */
	si_opencl_clWaitForEvents_wakeup(ctx, events);

	/* Return value ignored by caller, since context is getting suspended.
	 * It will be explicitly set by the wake-up call-back routine. */
	return 0;
}




/*
 * OpenCL call 'clGetEventInfo' (code 1044)
 */

int si_opencl_clGetEventInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
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
		mem_write(ctx->mem, param_value, 4, &status);

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

int si_opencl_clReleaseEvent_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int event_id = argv[0];  /* cl_event event */

	struct si_opencl_event_t *event;

	si_opencl_debug("  event=0x%x\n", event_id);

	event = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_event, event_id);

	/* Release event */
	assert(event->ref_count > 0);
	if (!event->ref_count)
		si_opencl_event_free(event);

	/* Return success */
	si_opencl_api_return(ctx, 0);

	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clGetEventProfilingInfo' (code 1050)
 */

int si_opencl_clGetEventProfilingInfo_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int event_id = argv[0];  /* cl_event event */
	unsigned int param_name = argv[1];  /* cl_profiling_info param_name */
	unsigned int param_value_size = argv[2];  /* size_t param_value_size */
	unsigned int param_value = argv[3];  /* void *param_value */
	unsigned int param_value_size_ret = argv[4];  /* size_t *param_value_size_ret */

	struct si_opencl_event_t *event;
	int size_ret;

	si_opencl_debug("  event=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			event_id, param_name, param_value_size, param_value,
			param_value_size_ret);
	event = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_event, event_id);
	size_ret = si_opencl_event_get_profiling_info(event, param_name, ctx->mem,
			param_value, param_value_size);
	if (param_value_size_ret)
		mem_write(ctx->mem, param_value_size_ret, 4, &size_ret);
	
	/* Return success */
	return 0;
}




/*
 * OpenCL call 'clFlush' (code 1051)
 */

int si_opencl_clFlush_impl(struct x86_ctx_t *ctx, int *argv)
{
	unsigned int command_queue = argv[0];  /* cl_command_queue command_queue */

	si_opencl_debug("  command_queue=0x%x\n", command_queue);
	return 0;
}




/*
 * OpenCL call 'clFinish' (code 1052)
 */

struct si_opencl_clFinish_args_t
{
	unsigned int command_queue_id;  /* cl_command_queue command_queue */
};

void si_opencl_clFinish_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clFinish_args_t argv;

	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1052);

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clFinish_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clFinish_args_t *argv;
	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clFinish_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x\n", argv->command_queue_id);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue_id);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clFinish_wakeup, NULL);

	/* Return value ignored by caller, since context is getting suspended.
	 * It will be explicitly set by the wake-up call-back routine. */
	return 0;

}




/*
 * OpenCL call 'clEnqueueReadBuffer' (code 1053)
 */

struct si_opencl_clEnqueueReadBuffer_args_t
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

void si_opencl_clEnqueueReadBuffer_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueReadBuffer_args_t argv;

	struct si_opencl_mem_t *mem;
	struct si_opencl_event_t *event;

	void *buf;

	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1053);

	/* Get memory object */
	mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.offset + argv.cb > mem->size)
		fatal("%s: buffer storage exceeded\n%s", __FUNCTION__,
				si_err_opencl_param_note);

	/* Copy buffer from device memory to host memory */
	buf = xmalloc(argv.cb);
	mem_read(si_emu->global_mem, mem->device_ptr + argv.offset, argv.cb, buf);
	mem_write(ctx->mem, argv.ptr, argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	si_opencl_debug("\t%d bytes copied from device (0x%x) to host(0x%x)\n",
			argv.cb, mem->device_ptr + argv.offset, argv.ptr);

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueReadBuffer_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clEnqueueReadBuffer_args_t *argv;

	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueReadBuffer_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_read=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_read, argv->offset,
			argv->cb, argv->ptr, argv->num_events_in_wait_list, argv->event_wait_list,
			argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue);
	
	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueReadBuffer_wakeup, NULL);

	/* Return value ignored by caller, since context is getting suspended.
	 * It will be explicitly set by the wake-up call-back routine. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueWriteBuffer' (code 1055)
 */

struct si_opencl_clEnqueueWriteBuffer_args_t
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

void si_opencl_clEnqueueWriteBuffer_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueWriteBuffer_args_t argv;

	struct si_opencl_mem_t *mem;
	struct si_opencl_event_t *event;

	void *buf;

	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1055);

	/* Get memory object */
	mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.offset + argv.cb > mem->size)
		fatal("%s: buffer storage exceeded.\n%s", __FUNCTION__,
				si_err_opencl_param_note);

	/* Copy buffer from host memory to device memory */
	buf = xmalloc(argv.cb);
	mem_read(ctx->mem, argv.ptr, argv.cb, buf);
	mem_write(si_emu->global_mem, mem->device_ptr + argv.offset, 
		argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_MAP_BUFFER);
		event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	si_opencl_debug("\t%d bytes copied from host (0x%x) to device (0x%x)\n",
			argv.cb, argv.ptr, mem->device_ptr + argv.offset);

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueWriteBuffer_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clEnqueueWriteBuffer_args_t *argv;
	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueWriteBuffer_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_write=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_write,
			argv->offset, argv->cb, argv->ptr, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
		si_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueWriteBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueCopyBuffer' (code 1057)
 */

struct si_opencl_clEnqueueCopyBuffer_args_t
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

void si_opencl_clEnqueueCopyBuffer_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueCopyBuffer_args_t argv;

	struct si_opencl_mem_t *src_mem, *dst_mem;
	struct si_opencl_event_t *event;

	void *buf;

	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1057);

	/* Get memory objects */
	src_mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.src_buffer);
	dst_mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.dst_buffer);

	/* Check that device buffer storage is not exceeded */
	if (argv.src_offset + argv.cb > src_mem->size || 
		argv.dst_offset + argv.cb > dst_mem->size)
	{
		fatal("%s: buffer storage exceeded\n%s", __FUNCTION__, 
			si_err_opencl_param_note);
	}

	/* Copy buffers */
	buf = xmalloc(argv.cb);
	mem_read(si_emu->global_mem, src_mem->device_ptr + argv.src_offset, 
		argv.cb, buf);
	mem_write(si_emu->global_mem, dst_mem->device_ptr + argv.dst_offset, 
		argv.cb, buf);
	free(buf);

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_MAP_BUFFER);
		event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	si_opencl_debug("\t%d bytes copied in device memory (0x%x -> 0x%x)\n",
			argv.cb, src_mem->device_ptr + argv.src_offset,
			dst_mem->device_ptr + argv.dst_offset);

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueCopyBuffer_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clEnqueueCopyBuffer_args_t *argv;
	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueCopyBuffer_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, src_buffer=0x%x, dst_buffer=0x%x,\n"
			"  src_offset=0x%x, dst_offset=0x%x, cb=%u, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->src_buffer, argv->dst_buffer, argv->src_offset,
			argv->dst_offset, argv->cb, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueCopyBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueReadImage' (code 1059)
 */

struct si_opencl_clEnqueueReadImage_args_t
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

void si_opencl_clEnqueueReadImage_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueReadImage_args_t argv;

	unsigned int read_region[3];
	unsigned int read_origin[3];

	struct si_opencl_mem_t *mem;
	struct si_opencl_event_t *event;

	void *img;
	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1059);

	/* Get memory object */
	mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.image);

	/* Determine image geometry */
	/* NOTE size_t is 32-bits on 32-bit systems, but 64-bits on 64-bit systems.  Since
	 * the simulator may be on a 64-bit system, we should not use size_t here, but unsigned int 
	 * instead. */
	mem_read(ctx->mem, argv.region, 12, read_region);
	mem_read(ctx->mem, argv.origin, 12, read_origin);

	if (!argv.row_pitch)
		argv.row_pitch = mem->width * mem->pixel_size;
	else if (argv.row_pitch < mem->width * mem->pixel_size)
		fatal("%s: row_pitch must be 0 or >= image_width * size of element in bytes\n%s", 
				__FUNCTION__, si_err_opencl_param_note);

	if (!argv.slice_pitch)
		argv.slice_pitch = argv.row_pitch * mem->height;
	else if (argv.slice_pitch < argv.row_pitch * mem->height)
		fatal("%s: slice_pitch must be 0 or >= row_pitch * image_height\n%s", 
				__FUNCTION__, si_err_opencl_param_note);

	/* FIXME Start with origin = {0,0,0}, region = {width, height, depth},
	 * then add support for subregions */
	if (read_origin[0] != 0 || read_origin[1] != 0 || read_origin[2] != 0 ||
			read_region[0] != mem->width || read_region[1] != mem->height ||
			read_region[2] != mem->depth)
		fatal("%s: Origin/region must match dimensions of image\n%s", 
				__FUNCTION__, si_err_opencl_param_note);

	/* Read the entire image */
	img = xmalloc(mem->size);
	mem_read(si_emu->global_mem, mem->device_ptr, mem->size, img);
	mem_write(ctx->mem, argv.ptr, mem->size, img);
	free(img);

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = SI_OPENCL_EVENT_STATUS_SUBMITTED;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Debug */
	/* FIXME Return size of region, not entire image */
	si_opencl_debug("\t%d bytes copied from device (0x%x) to host (0x%x)\n",
			mem->size, mem->device_ptr, argv.ptr);

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueReadImage_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clEnqueueReadImage_args_t *argv;
	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueReadImage_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, image=0x%x, blocking_read=0x%x,\n"
			"  origin=0x%x, region=0x%x, row_pitch=0x%x, slice_pitch=0x%x, ptr=0x%x\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue, argv->image, argv->blocking_read, argv->origin,
			argv->region, argv->row_pitch, argv->slice_pitch, argv->ptr,
			argv->num_events_in_wait_list, argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueReadImage_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * OpenCL call 'clEnqueueMapBuffer' (code 1064)
 */

struct si_opencl_clEnqueueMapBuffer_args_t
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

void si_opencl_clEnqueueMapBuffer_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueMapBuffer_args_t argv;
	struct si_opencl_mem_t *mem;
	struct si_opencl_event_t *event;

	void* buf;

	int code;
	int zero = 0;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1064);

	/* Get memory object */
	mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.buffer);

	/* Set map info for use in clEnqueueUnmapMemObj */
	mem->map_flags = argv.map_flags;
	mem->map_offset = argv.offset;
	mem->map_cb = argv.cb;

	/* Check that device buffer storage is not exceeded */
	if (argv.offset + argv.cb > mem->size)
		fatal("%s: buffer storage exceeded.\n%s", __FUNCTION__,
				si_err_opencl_param_note);

	/* CL_MAP_READ, Copy buffer from device memory to host memory */
	if ((argv.map_flags & 1) == 1)
	{
		buf = xmalloc(argv.cb);
		mem_read(si_emu->global_mem, mem->device_ptr + argv.offset, argv.cb, buf);
		if (mem->flags & 0x8) /* CL_MEM_USE_HOST_PTR */
		{
			mem_write(ctx->mem, mem->host_ptr, argv.cb, buf);
		}
		else
		{
			//mem_write(ctx->mem, argv.new_buffer, argv.cb, buf);
		}
		free(buf);

		/* Debug */
		si_opencl_debug("\t%d bytes copied from device (0x%x) to host(0x%x)\n",
				argv.cb, mem->device_ptr + argv.offset, mem->host_ptr);
	}

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_MAP_BUFFER);
		event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Return success */
	if (argv.errcode_ret)
		mem_write(ctx->mem, argv.errcode_ret, 4, &zero);

	/* Return success */
	//si_opencl_api_return(ctx, (mem->flags & 0x8) ? mem->host_ptr: argv.new_buffer);
}

int si_opencl_clEnqueueMapBuffer_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	fatal("clEnqueueMapBuffer not implemented.");

	struct si_opencl_clEnqueueMapBuffer_args_t *argv;
	struct si_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueMapBuffer_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_map=0x%x, map_flags=0x%x,\n"
			"  offset=0x%x, cb=0x%x, num_events_in_wait_list=0x%x, event_wait_list=0x%x,\n"
			"  event=0x%x, errcode_ret=0x%x\n",
			argv->command_queue, argv->buffer, argv->blocking_map, argv->map_flags,
			argv->offset, argv->cb, argv->num_events_in_wait_list, argv->event_wait_list,
			argv->event_ptr, argv->errcode_ret);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_EQ(argv->map_flags & 4, 4);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueMapBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}


/*
 * OpenCL call 'clEnqueueUnmapMemObject' (code 1066)
 */

struct si_opencl_clEnqueueUnmapMemObject_args_t
{
	unsigned int command_queue;  /* cl_command_queue command_queue */
	unsigned int memobj;  /* cl_mem memobj */
	unsigned int mapped_ptr; /* void *mapped_ptr */
	unsigned int num_events_in_wait_list;  /* cl_uint num_events_in_wait_list */
	unsigned int event_wait_list;  /* const cl_event *event_wait_list */
	unsigned int event_ptr;  /* cl_event *event */
};

void si_opencl_clEnqueueUnmapMemObject_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueUnmapMemObject_args_t argv;
	struct si_opencl_mem_t *mem;
	struct si_opencl_event_t *event;

	void* buf;

	int code;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1066);

	/* Get memory object */
	mem = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_mem, argv.memobj);

	/* CL_MAP_WRITE, Copy buffer from host memory to device memory */
	if ((mem->map_flags & 2) == 2)
	{
		buf = xmalloc(mem->map_cb);
		mem_read(ctx->mem, argv.mapped_ptr, mem->map_cb, buf);
		mem_write(si_emu->global_mem, mem->device_ptr + mem->map_offset, mem->map_cb, buf);
		free(buf);

		/* Debug */
		si_opencl_debug("\t%d bytes copied from host (0x%x) to device (0x%x)\n",
				mem->map_cb, argv.mapped_ptr, mem->device_ptr + mem->map_offset);
	}

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_MAP_BUFFER);
		event->status = SI_OPENCL_EVENT_STATUS_COMPLETE;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();
		event->time_end = si_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueUnmapMemObject_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	fatal("clEnqueueUnmapMemObject not implemented.");

	struct si_opencl_clEnqueueUnmapMemObject_args_t *argv;
	struct si_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueUnmapMemObject_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, memobj=0x%x, mapped_ptr=0x%x\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x,\n"
			"  event=0x%x\n",
			argv->command_queue, argv->memobj, argv->mapped_ptr,
			argv->num_events_in_wait_list, argv->event_wait_list,
			argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueMapBuffer_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}


/*
 * OpenCL call 'clEnqueueNDRangeKernel' (code 1067)
 */

struct si_opencl_clEnqueueNDRangeKernel_args_t
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

void si_opencl_clEnqueueNDRangeKernel_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_clEnqueueNDRangeKernel_args_t argv;
	struct si_opencl_kernel_t *kernel;
	struct si_opencl_event_t *event = NULL;
	struct si_opencl_kernel_arg_t *arg;
	struct si_opencl_command_queue_t *command_queue;
	struct si_opencl_command_t *task;
	struct si_ndrange_t *ndrange;
	int global_size3[3];
	int local_size3[3];

	struct elf_buffer_t *elf_buffer;

	struct si_buffer_desc_t buf_desc;


	int code;
	int i;

	/* Read function arguments again */
	code = si_opencl_api_read_args(ctx, NULL, &argv, sizeof argv);
	assert(code == 1067);

	/* Get kernel */
	kernel = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_kernel, argv.kernel_id);

	/* Global work sizes */
	global_size3[1] = 1;
	global_size3[2] = 1;
	for (i = 0; i < argv.work_dim; i++)
	{
		mem_read(ctx->mem, argv.global_work_size_ptr + i * 4, 4, 
			&global_size3[i]);
	}

	/* Local work sizes.
	 * If no pointer provided, assign the same as global size 
	 * FIXME: can be done better. */
	memcpy(local_size3, global_size3, 12);
	if (argv.local_work_size_ptr)
	{
		for (i = 0; i < argv.work_dim; i++)
		{
			mem_read(ctx->mem, argv.local_work_size_ptr + i * 4, 
				4, &local_size3[i]);
		}
	}

	/* Event */
	if (argv.event_ptr)
	{
		event = si_opencl_event_create(SI_OPENCL_EVENT_NDRANGE_KERNEL);
		event->status = SI_OPENCL_EVENT_STATUS_SUBMITTED;
		event->time_queued = si_opencl_event_timer();
		event->time_submit = si_opencl_event_timer();
		event->time_start = si_opencl_event_timer();  /* FIXME: change for asynchronous exec */
		mem_write(ctx->mem, argv.event_ptr, 4, &event->id);
		si_opencl_debug("    event: 0x%x\n", event->id);
	}

	/* Setup ND-Range */
	ndrange = si_ndrange_create(global_size3, local_size3, argv.work_dim);
	si_ndrange_setup_kernel(ndrange, kernel);
	si_ndrange_setup_work_items(ndrange);
	si_ndrange_setup_const_mem(ndrange);
	si_ndrange_setup_args(ndrange);

	/* Set up instruction memory */
	/* Initialize wavefront instruction buffer and PC */
	elf_buffer = &kernel->bin_file->enc_dict_entry_southern_islands->
		sec_text_buffer;
	if (!elf_buffer->size)
		fatal("%s: cannot load kernel code", __FUNCTION__);
	si_ndrange_setup_inst_mem(ndrange, elf_buffer->ptr, elf_buffer->size, 0);

	/* Build UAV lists */
	for (i = 0; i < list_count(kernel->arg_list); i++)
	{
		arg = list_get(kernel->arg_list, i);

		if (arg->kind == SI_OPENCL_KERNEL_ARG_KIND_IMAGE)
		{
			/* Add the image to the resource table */
			/*FIXME*/
			assert(0);
			/*
			mem = si_opencl_repo_get_object(si_emu->opencl_repo,
					si_opencl_object_mem, arg->value);

			if (arg->access_type == 
				SI_OPENCL_KERNEL_ARG_READ_ONLY)
			{
				list_set(kernel->uav_read_list, arg->uav, 
					mem);
			}
			else if (arg->access_type == 
				SI_OPENCL_KERNEL_ARG_WRITE_ONLY)
			{
				list_set(kernel->uav_write_list, arg->uav,
			       		mem);
			}
			else
			{
				fatal("%s: unsupported image access type "
					"(%d)\n", __FUNCTION__,
					arg->access_type);
			}
			*/
		}

		if(arg->kind == SI_OPENCL_KERNEL_ARG_KIND_POINTER)
		{
			/* Add the uav to the UAV table */
			if (arg->pointer.mem_type == 
				SI_OPENCL_KERNEL_ARG_MEM_TYPE_UAV)
			{
				buf_desc = si_emu_create_buffer_desc(arg);
				si_emu_insert_into_uav_table(buf_desc, arg);
			}
		}
	}

	/* Debugging */
	si_ndrange_dump_initialized_state(ndrange);

	/* Save in kernel */
	kernel->ndrange = ndrange;

	/* Set ND-Range status to 'pending'. This makes it immediately a 
	 * candidate for execution, whether we have functional or 
	 * detailed simulation. */
	si_ndrange_set_status(ndrange, si_ndrange_pending);

	/* Associate NDRange event */
	ndrange->event = event;

	/* Create command queue task */
	task = si_opencl_command_create(
		si_opencl_command_queue_task_ndrange_kernel);
	task->u.ndrange_kernel.ndrange = ndrange;

	/* Enqueue task */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv.command_queue_id);
	si_opencl_command_queue_submit(command_queue, task);
	ndrange->command_queue = command_queue;
	ndrange->command = task;

	/* Return success */
	si_opencl_api_return(ctx, 0);
}

int si_opencl_clEnqueueNDRangeKernel_impl(struct x86_ctx_t *ctx, int *argv_ptr)
{
	struct si_opencl_clEnqueueNDRangeKernel_args_t *argv;
	struct si_opencl_command_queue_t *command_queue;

	/* Debug arguments */
	argv = (struct si_opencl_clEnqueueNDRangeKernel_args_t *) argv_ptr;
	si_opencl_debug("  command_queue=0x%x, kernel=0x%x, work_dim=%d,\n"
			"  global_work_offset=0x%x, global_work_size_ptr=0x%x, local_work_size_ptr=0x%x,\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			argv->command_queue_id, argv->kernel_id, argv->work_dim,
			argv->global_work_offset_ptr, argv->global_work_size_ptr,
			argv->local_work_size_ptr, argv->num_events_in_wait_list,
			argv->event_wait_list, argv->event_ptr);

	/* Not supported arguments */
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->global_work_offset_ptr, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_RANGE(argv->work_dim, 1, 3);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->num_events_in_wait_list, 0);
	SI_OPENCL_ARG_NOT_SUPPORTED_NEQ(argv->event_wait_list, 0);

	/* Get command queue */
	command_queue = si_opencl_repo_get_object(si_emu->opencl_repo,
			si_opencl_object_command_queue, argv->command_queue_id);

	/* Suspend context until command queue is empty */
	x86_ctx_suspend(ctx, si_opencl_command_queue_can_wakeup, command_queue,
			si_opencl_clEnqueueNDRangeKernel_wakeup, NULL);

	/* Return success, ignored for suspended context. */
	return 0;
}




/*
 * Rest of not implemented OpenCL API
 */

#define __SI_OPENCL_NOT_IMPL__(_name) \
	int si_opencl_##_name##_impl(struct x86_ctx_t *ctx, int *argv) \
	{ \
		fatal("%s: OpenCL function not implemented.\n%s", __FUNCTION__, \
			si_err_opencl_note); \
		return 0; \
	}

__SI_OPENCL_NOT_IMPL__(clRetainContext)
__SI_OPENCL_NOT_IMPL__(clGetCommandQueueInfo)
__SI_OPENCL_NOT_IMPL__(clSetCommandQueueProperty)
__SI_OPENCL_NOT_IMPL__(clCreateSubBuffer)
__SI_OPENCL_NOT_IMPL__(clGetSupportedImageFormats)
__SI_OPENCL_NOT_IMPL__(clGetMemObjectInfo)
__SI_OPENCL_NOT_IMPL__(clGetImageInfo)
__SI_OPENCL_NOT_IMPL__(clSetMemObjectDestructorCallback)
__SI_OPENCL_NOT_IMPL__(clRetainSampler)
__SI_OPENCL_NOT_IMPL__(clReleaseSampler)
__SI_OPENCL_NOT_IMPL__(clGetSamplerInfo)
__SI_OPENCL_NOT_IMPL__(clUnloadCompiler)
__SI_OPENCL_NOT_IMPL__(clGetProgramInfo)
__SI_OPENCL_NOT_IMPL__(clGetProgramBuildInfo)
__SI_OPENCL_NOT_IMPL__(clCreateKernelsInProgram)
__SI_OPENCL_NOT_IMPL__(clRetainKernel)
__SI_OPENCL_NOT_IMPL__(clGetKernelInfo)
__SI_OPENCL_NOT_IMPL__(clCreateUserEvent)
__SI_OPENCL_NOT_IMPL__(clRetainEvent)
__SI_OPENCL_NOT_IMPL__(clSetUserEventStatus)
__SI_OPENCL_NOT_IMPL__(clSetEventCallback)
__SI_OPENCL_NOT_IMPL__(clEnqueueReadBufferRect)
__SI_OPENCL_NOT_IMPL__(clEnqueueWriteBufferRect)
__SI_OPENCL_NOT_IMPL__(clEnqueueCopyBufferRect)
__SI_OPENCL_NOT_IMPL__(clEnqueueWriteImage)
__SI_OPENCL_NOT_IMPL__(clEnqueueCopyImage)
__SI_OPENCL_NOT_IMPL__(clEnqueueCopyImageToBuffer)
__SI_OPENCL_NOT_IMPL__(clEnqueueCopyBufferToImage)
__SI_OPENCL_NOT_IMPL__(clEnqueueMapImage)
__SI_OPENCL_NOT_IMPL__(clEnqueueTask)
__SI_OPENCL_NOT_IMPL__(clEnqueueNativeKernel)
__SI_OPENCL_NOT_IMPL__(clEnqueueMarker)
__SI_OPENCL_NOT_IMPL__(clEnqueueWaitForEvents)
__SI_OPENCL_NOT_IMPL__(clEnqueueBarrier)
__SI_OPENCL_NOT_IMPL__(clGetExtensionFunctionAddress)

