/*
F *  Multi2Sim
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
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "../include/cuda.h"
#include "api.h"
#include "context.h"
#include "debug.h"
#include "device.h"
#include "event.h"
#include "function.h"
#include "list.h"
#include "mhandle.h"
#include "misc.h"
#include "module.h"
#include "stream.h"


/*
 * Global Variables
 */

/* Runtime version numbers */
#define CUDA_VERSION_MAJOR 2
#define CUDA_VERSION_MINOR 0

/* Debug */
static FILE *cuda_debug_file;

/* Error */
char *cuda_err_not_impl =
		"\tMulti2Sim provides partial support for CUDA driver library. To\n"
		"\trequest the implementation of a certain functionality, please\n"
		"\temail development@multi2sim.org.\n";

char *cuda_err_version =
		"\tYour guest application is using a version of the CUDA driver\n"
		"\tlibrary that is incompatible with this version of Multi2Sim.\n"
		"\tPlease download the latest Multi2Sim version, and recompile your\n"
		"\tapplication with the latest CUDA driver library ('libm2s-cuda').\n";

char *cuda_err_native =
		"\tYou are trying to run natively an application using the Multi2Sim\n"
		"\tCUDA driver library implementation ('libm2s-cuda'). Please run\n"
		"\tthis program on top of Multi2Sim.\n";

#define __CUDA_NOT_IMPL__  warning("%s: not implemented.\n%s", \
		__func__, cuda_err_not_impl)




/*
 * M2S CUDA Internal Functions
 */

/* Environment variable M2S_CUDA_DEBUG specifies debug information for the
 * CUDA runtime. */
void cuda_debug(char *fmt, ...)
{
	va_list va;
	char str[1024];
	char *cuda_debug_file_name;
	static int cuda_debug_init = 0;

	/* Initialize debug */
	if (!cuda_debug_init)
	{
		cuda_debug_init = 1;
		cuda_debug_file_name = getenv("M2S_CUDA_DEBUG");
		if (cuda_debug_file_name)
		{
			cuda_debug_file = file_open_for_write(cuda_debug_file_name);
			if (!cuda_debug_file)
				fatal("%s: cannot open debug output file",
						cuda_debug_file_name);
		}
	}

	/* Exit if not enabled */
         if (!cuda_debug_file)
		return;

	/* Reconstruct message in 'str' first. This is done to avoid multiple
	 * calls to 'printf', that can have race conditions among threads. */
	va_start(va, fmt);
	vsnprintf(str, sizeof str, fmt, va);
	fprintf(cuda_debug_file, "[CUDA Runtime] %s\n", str);
}


void versionCheck(void)
{
	struct
	{
		int major;
		int minor;
	} version;
	int ret;

	/* Debug */
	cuda_debug("CUDA driver internal function '%s'", __func__);

	/* Version negotiation */
	unsigned args[1] = { (unsigned) &version };
	ret = ioctl(active_device->fd, cuda_call_Init, args);
	if (ret)
		fatal("[CUDA Runtime] %s: Unexpected error", __FUNCTION__);

	/* Debug */
	cuda_debug("\tReturned values:");
	cuda_debug("\tmajor = %d", version.major);
	cuda_debug("\tminor = %d", version.minor);

	/* Check that exact major version matches */
	if (version.major != CUDA_VERSION_MAJOR || version.minor <
			CUDA_VERSION_MINOR)
		fatal("%s:%d: incompatible CUDA versions. Guest library v. %d.%d / "
				"Host implementation v. %d.%d.\n\t%s", __FILE__, __LINE__,
				CUDA_VERSION_MAJOR, CUDA_VERSION_MINOR, version.major,
				version.minor, cuda_err_version);
}


/*
 * Public CUDA Driver Library
 */

CUresult cuInit(unsigned int Flags)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: Flags = %u", __func__, Flags);

	/* Check input */
	if (Flags != 0U)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Create CUDA object lists */
	context_list = list_create();
	device_list = list_create();
	memory_object_list = list_create();
	event_list = list_create();

	/* Create one Fermi device and one Kepler device */
	active_device = cuda_device_create();

	/* Check version */
	versionCheck();

	/* Create memory object lists */
	pinned_memory_object_list = list_create();
	device_memory_object_list = list_create();

	pinned_memory_object_tail_list = list_create();
	device_memory_object_tail_list = list_create();

	/* Initialize mutex */
	pthread_mutex_init(&cuda_mutex, NULL);

	/* Success */
	return CUDA_SUCCESS;
}

CUresult cuDriverGetVersion(int *driverVersion)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: driverVersion = [%p]", __func__,
			driverVersion);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (driverVersion == NULL)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Version 5.5 */
	*driverVersion = 5050;

	cuda_debug("\t(driver) '%s' out: driverVersion = %d", __func__,
			*driverVersion);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGet(CUdevice *device, int ordinal)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: device = [%p]", __func__, device);
	cuda_debug("\t(driver) '%s' in: ordinal = %d", __func__, ordinal);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (ordinal >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get the currently active device */
	*device = active_device->device;

	cuda_debug("\t(driver) '%s' out: device = %d", __func__, *device);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetCount(int *count)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: count = [%p]", __func__, count);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* TODO: only the Fermi device can be detected for now. *count should be
	 * equal to list_count(device_list) when the Kepler device is ready. */
	*count = 1;

	cuda_debug("\t(driver) '%s' out: count = %d", __func__, *count);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetName(char *name, int len, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: name = [%p]", __func__, name);
	cuda_debug("\t(driver) '%s' in: len = %d", __func__, len);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Copy name */
	strncpy(name, ((struct cuda_device_t *)list_get(device_list, dev))->name,
			len);

	cuda_debug("\t(driver) '%s' out: name = %s", __func__, name);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceTotalMem(size_t *bytes, CUdevice dev)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: bytes = [%p]", __func__, bytes);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuMemGetInfo, &free, bytes);
	ret = 0;
	fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: bytes = %d", __func__, *bytes);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pi = [%p]", __func__, pi);
	cuda_debug("\t(driver) '%s' in: attrib = %d", __func__, attrib);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get attribute */
	*pi = (((struct cuda_device_t *)list_get(device_list, dev))->attributes)
			[attrib];

	cuda_debug("\t(driver) '%s' out: pi = %d", __func__, *pi);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
	struct cuda_device_t *device;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: prop = [%p]", __func__, prop);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get device */
	device = list_get(device_list, dev);

	/* Get properties */
	prop->maxThreadsPerBlock = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK];
	prop->maxThreadsDim[0] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X];
	prop->maxThreadsDim[1] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y];
	prop->maxThreadsDim[2] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z];
	prop->maxGridSize[0] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X];
	prop->maxGridSize[1] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y];
	prop->maxGridSize[2] = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z];
	prop->sharedMemPerBlock = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK];
	prop->totalConstantMemory = device->attributes
			[CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY];
	prop->SIMDWidth = device->attributes[CU_DEVICE_ATTRIBUTE_WARP_SIZE];
	prop->memPitch = device->attributes[CU_DEVICE_ATTRIBUTE_MAX_PITCH];
	prop->regsPerBlock = device->attributes
			[CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK];
	prop->clockRate = device->attributes[CU_DEVICE_ATTRIBUTE_CLOCK_RATE];
	prop->textureAlign = device->attributes
			[CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT];

	cuda_debug("\t(driver) '%s' out: prop = %d", __func__,
			prop->maxThreadsPerBlock);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
	struct cuda_device_t *device;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: major = [%p]", __func__, major);
	cuda_debug("\t(driver) '%s' in: minor = [%p]", __func__, minor);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get device */
	device = list_get(device_list, dev);

	/* Get version */
	*major = device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR];
	*minor = device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR];

	cuda_debug("\t(driver) '%s' out: major = %d", __func__, *major);
	cuda_debug("\t(driver) '%s' out: minor = %d", __func__, *minor);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pctx = [%p]", __func__, pctx);
	cuda_debug("\t(driver) '%s' in: flags = %u", __func__, flags);
	cuda_debug("\t(driver) '%s' in: dev = %d", __func__, dev);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	*pctx = cuda_context_create(dev);

	cuda_debug("\t(driver) '%s' out: pctx = [%p]", __func__, *pctx);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxDestroy(CUcontext ctx)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: ctx = [%p]", __func__, ctx);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	cuda_context_free(ctx);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxPushCurrent(CUcontext ctx)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxPopCurrent(CUcontext *pctx)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxSetCurrent(CUcontext ctx)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetCurrent(CUcontext *pctx)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetDevice(CUdevice *device)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxSynchronize(void)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxSetLimit(CUlimit limit, size_t value)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetLimit(size_t *pvalue, CUlimit limit)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetCacheConfig(CUfunc_cache *pconfig)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxSetCacheConfig(CUfunc_cache config)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetSharedMemConfig(CUsharedconfig *pConfig)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxSetSharedMemConfig(CUsharedconfig config)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxGetApiVersion(CUcontext ctx, unsigned int *version)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: ctx = [%p]", __func__, ctx);
	cuda_debug("\t(driver) '%s' in: version = [%p]", __func__, version);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	*version= ((CUcontext)list_get(context_list, ctx->id))->version;

	cuda_debug("\t(driver) '%s' out: version = %u", __func__, *version);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxGetStreamPriorityRange(int *leastPriority, int *greatestPriority)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxAttach(CUcontext *pctx, unsigned int flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult CUDAAPI cuCtxDetach(CUcontext ctx)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleLoad(CUmodule *module, const char *fname)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: module = [%p]", __func__, module);
	cuda_debug("\t(driver) '%s' in: fname = [%p] %s", __func__, fname, fname);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Create module */
	*module = cuda_module_create(fname);

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleLoad, fname);
	unsigned args[1] = {(unsigned) fname};
	ret = ioctl(active_device->fd, cuda_call_ModuleLoad, args);
	if(ret)
	fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: module = [%p]", __func__, *module);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleLoadData(CUmodule *module, const void *image)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleLoadDataEx(CUmodule *module, const void *image,
		unsigned int numOptions, CUjit_option *options, void **optionValues)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleUnload(CUmodule hmod)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hmod = [%p]", __func__, hmod);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Free module in driver */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleUnload, hmod->id);
	ret = 0;
	fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	/* Free module in runtime */
	cuda_module_free(hmod);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hfunc = [%p]", __func__, hfunc);
	cuda_debug("\t(driver) '%s' in: hmod = [%p]", __func__, hmod);
	cuda_debug("\t(driver) '%s' in: name = [%p] %s", __func__, name, name);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Create function */
	*hfunc = cuda_function_create(hmod, name);
	unsigned args[2] = {(unsigned) (hmod->id), (unsigned)name};

	ret = ioctl(active_device->fd, cuda_call_ModuleGetFunction, args);
	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleGetFunction, hmod->id, name,
	//		(*hfunc)->inst_buf, (*hfunc)->inst_buf_size, (*hfunc)->numRegs);

	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: hfunc = [%p]", __func__, *hfunc);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleGetGlobal(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod,
		const char *name)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleGetTexRef(CUtexref *pTexRef, CUmodule hmod, const char *name)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuModuleGetSurfRef(CUsurfref *pSurfRef, CUmodule hmod,
		const char *name)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}


CUresult cuLinkCreate(unsigned int numOptions, CUjit_option *options,
		void **optionValues, CUlinkState *stateOut)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLinkAddData(CUlinkState state, CUjitInputType type, void *data,
		size_t size, const char *name, unsigned int numOptions,
		CUjit_option *options, void **optionValues)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLinkAddFile(CUlinkState state, CUjitInputType type, const char *path,
		unsigned int numOptions, CUjit_option *options, void **optionValues)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLinkComplete(CUlinkState state, void **cubinOut, size_t *sizeOut)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLinkDestroy(CUlinkState state)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemGetInfo(size_t *free, size_t *total)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: free = [%p]", __func__, free);
	cuda_debug("\t(driver) '%s' in: total = [%p]", __func__, total);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuMemGetInfo, free, total);
	unsigned args[2] = {(unsigned) &free, (unsigned) &total};
	ret = ioctl(active_device->fd, cuda_call_MemGetInfo, args);
	if (ret)
		fatal("[CUDA Runtime] %s: Unexpected error", __FUNCTION__);

	cuda_debug("\t(driver) '%s' out: free = %d", __func__, *free);
	cuda_debug("\t(driver) '%s' out: total = %d", __func__, *total);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemAlloc(CUdeviceptr *dptr, size_t bytesize)
{
	//int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: device ptr address = [%p]", __func__, dptr);
	cuda_debug("\t(driver) '%s' in: bytesize = %d", __func__, bytesize);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	if (bytesize == 0)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	//unsigned args[2] = {(unsigned)dptr, (unsigned) bytesize};
	unsigned args[1] = {(unsigned) bytesize};
	*dptr = (CUdeviceptr) ioctl(active_device->fd, cuda_call_MemAlloc, args);
	// cuda_debug("\t (driver) '%s' ret =  %d", __func__, ret);
	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemAlloc, dptr, bytesize);
	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	//if (ret)
	//	fatal("native execution not supported.\n%s", cuda_err_native);

	/* Update device memory list */
	list_enqueue(device_memory_object_list, (void *)*dptr);

	/*Update device memory tail list */
	CUdeviceptr dptr_tail;
	dptr_tail = *dptr + bytesize;
	list_enqueue(device_memory_object_tail_list, (void *) dptr_tail);

	cuda_debug("\t(driver) '%s' out: device_ptr_tail = 0x%d", __func__,
					dptr_tail);
	cuda_debug("\t(driver) '%s' out: device ptr address = [%p]", __func__, dptr);
	cuda_debug("\t(driver) '%s' out: device_ptr = 0x%d", __func__, *dptr);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemAllocPitch(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes,
		size_t Height, unsigned int ElementSizeBytes)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemFree(CUdeviceptr dptr)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: device_ptr = 0x%08x", __func__, dptr);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuMemFree, dptr);
	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);
	unsigned args[1] = {(unsigned) dptr};
	ret = ioctl(active_device->fd, cuda_call_MemFree, args);
	cuda_debug("\t (driver) '%s' ret =  %d", __func__, ret);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	/* Update device memory list */
	list_remove(device_memory_object_list, (void *)dptr);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemGetAddressRange(CUdeviceptr *pbase, size_t *psize,
		CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemAllocHost(void **pp, size_t bytesize)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pp = [%p]", __func__, pp);
	cuda_debug("\t(driver) '%s' in: bytesize = %d", __func__, bytesize);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Allocate */
	*pp = xcalloc(1, bytesize);

	/* Update pinned memory list */
	list_enqueue(pinned_memory_object_list, *pp);

	cuda_debug("\t(driver) '%s' out: pp = [%p]", __func__, *pp);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemFreeHost(void *p)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: p = [%p]", __func__, p);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Update pinned memory list */
	list_remove(pinned_memory_object_list, p);

	free(p);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pp = [%p]", __func__, pp);
	cuda_debug("\t(driver) '%s' in: bytesize = %d", __func__, bytesize);
	cuda_debug("\t(driver) '%s' in: Flags = %u", __func__, Flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Allocate */
	*pp = xcalloc(1, bytesize);

	/* Update pinned memory table */
	list_enqueue(pinned_memory_object_list, *pp);

	cuda_debug("\t(driver) '%s' out: pp = [%p]", __func__, *pp);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p,
		unsigned int Flags)
{
	/*int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pdptr = [%p]", __func__, pdptr);
	cuda_debug("\t(driver) '%s' in: p = [%p]", __func__, p);
	cuda_debug("\t(driver) '%s' in: Flags = %u", __func__, Flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	unsigned args[2] = {(unsigned) pdptr, (unsigned) p};
	ret = ioctl(active_device->fd, cuda_call_MemFree, args);

	return CUDA_SUCCESS;
	*/
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemHostGetFlags(unsigned int *pFlags, void *p)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuDeviceGetByPCIBusId(CUdevice *dev, char *pciBusId)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuDeviceGetPCIBusId(char *pciBusId, int len, CUdevice dev)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuIpcGetEventHandle(CUipcEventHandle *pHandle, CUevent event)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuIpcOpenEventHandle(CUevent *phEvent, CUipcEventHandle handle)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuIpcGetMemHandle(CUipcMemHandle *pHandle, CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuIpcOpenMemHandle(CUdeviceptr *pdptr, CUipcMemHandle handle,
		unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuIpcCloseMemHandle(CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemHostRegister(void *p, size_t bytesize, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemHostUnregister(void *p)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext,
		CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost,
		size_t ByteCount)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: dstDevice = 0x%08x", __func__, dstDevice);
	cuda_debug("\t(driver) '%s' in: srcHost = [%p]", __func__, srcHost);
	cuda_debug("\t(driver) '%s' in: ByteCount = %d", __func__, ByteCount);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	cuStreamSynchronize(0);

	unsigned args[3] = {(unsigned) dstDevice, (unsigned) srcHost,
				(unsigned) ByteCount};
	ret = ioctl(active_device->fd, cuda_call_MemWrite, args);
	/* Syscall */
	// ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyHtoD, dstDevice,
	//			srcHost, ByteCount);
	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: dstHost = [%p]", __func__, dstHost);
	cuda_debug("\t(driver) '%s' in: srcDevice = 0x%08x", __func__, srcDevice);
	cuda_debug("\t(driver) '%s' in: ByteCount = %d", __func__, ByteCount);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	cuStreamSynchronize(0);

	unsigned args[3] = {(unsigned) dstHost, (unsigned) srcDevice,
				(unsigned) ByteCount};
	ret = ioctl(active_device->fd, cuda_call_MemRead, args);

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyDtoH, dstHost,
	//		srcDevice, ByteCount);
	//ret = 0;
	//fatal("%s: not implemented", __FUNCTION__);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoD(CUdeviceptr dstDevice, CUdeviceptr srcDevice,
		size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoA(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice,
		size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoD(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset,
		size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoA(CUarray dstArray, size_t dstOffset, const void *srcHost,
		size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoH(void *dstHost, CUarray srcArray, size_t srcOffset,
		size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoA(CUarray dstArray, size_t dstOffset, CUarray srcArray,
		size_t srcOffset, size_t ByteCount)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2D(const CUDA_MEMCPY2D *pCopy)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3D(const CUDA_MEMCPY3D *pCopy)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount,
		CUstream hStream)
{
	struct cuda_stream_command_t *command;
	struct memory_args_t *args;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: dst = 0x%08x, stream id = %d",
					__func__, dst, hStream->id);
	cuda_debug("\t(driver) '%s' in: src = 0x%08x, stream id = %d",
					__func__, src, hStream->id);
	cuda_debug("\t(driver) '%s' in: ByteCount = %d, stream id = %d",
					__func__, ByteCount, hStream->id);
	cuda_debug("\t(driver) '%s' in: hStream = [%p], strean id = %d",
					__func__, hStream, hStream->id);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Create arguments */
	args = xcalloc(1, sizeof(struct memory_args_t));
	args->src_ptr = src;
	args->dst_ptr = dst;
	args->size = ByteCount;

	/* If stream == 0, it is the default stream. */
	if (hStream == 0)
		hStream = list_get(active_device->stream_list, 0);

	command = cuda_stream_command_create(hStream, cuMemcpyAsyncImpl, args,
			NULL, NULL, NULL);
	command->ready_to_run = 1;
	cuda_stream_enqueue(hStream, command);

	/* Free arguments */
	free(args);

	cuda_debug("\t(driver) '%s' out: return = %d stream id = %d",
					__func__, CUDA_SUCCESS, hStream->id);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext,
		CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount,
		CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoDAsync(CUdeviceptr dstDevice, const void *srcHost,
		size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoHAsync(void *dstHost, CUdeviceptr srcDevice,
		size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoDAsync(CUdeviceptr dstDevice, CUdeviceptr srcDevice,
		size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoAAsync(CUarray dstArray, size_t dstOffset,
		const void *srcHost, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoHAsync(void *dstHost, CUarray srcArray, size_t srcOffset,
		size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD8(CUdeviceptr dstDevice, unsigned char uc, size_t N)
{
	int ret;
	unsigned char *src_host;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: dstDevice = 0x%08x", __func__, dstDevice);
	cuda_debug("\t(driver) '%s' in: uc = 0x%x", __func__, uc);
	cuda_debug("\t(driver) '%s' in: N = %d", __func__, N);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Create host data */
	src_host = xcalloc(N, sizeof(unsigned char));
	memset(src_host, uc, N);

	/* Syscall */
	//ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyHtoD, dstDevice,
	//		src_host, N);
	ret = 0;
	fatal("%s: not implemented", __FUNCTION__);

	/* Free host data */
	free(src_host);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemsetD16(CUdeviceptr dstDevice, unsigned short us, size_t N)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD32(CUdeviceptr dstDevice, unsigned int ui, size_t N)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D8(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc,
		size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D16(CUdeviceptr dstDevice, size_t dstPitch,
		unsigned short us, size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D32(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui,
		size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N,
		CUstream hStream)
{
	struct cuda_stream_command_t *command;
	unsigned char *src;
	struct memory_args_t *args;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: dstDevice = 0x%08x", __func__, dstDevice);
	cuda_debug("\t(driver) '%s' in: uc = 0x%02x", __func__, uc);
	cuda_debug("\t(driver) '%s' in: N = %d", __func__, N);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Create host data */
	src = xcalloc(N, sizeof(unsigned char));
	memset(src, uc, N);

	/* Create arguments */
	args = xcalloc(1, sizeof(struct memory_args_t));
	args->src_ptr = (CUdeviceptr)src;
	args->dst_ptr = dstDevice;
	args->size = N;

	/* If stream == 0, it is the default stream. */
	if (hStream == 0)
		hStream = list_get(active_device->stream_list, 0);

	command = cuda_stream_command_create(hStream, cuMemcpyAsyncImpl, args,
			NULL, NULL, NULL);
	command->ready_to_run = 1;
	cuda_stream_enqueue(hStream, command);

	/* Free arguments */
	free(args);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N,
		CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N,
		CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch,
		unsigned char uc, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch,
		unsigned short us, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch,
		unsigned int ui, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuArrayCreate(CUarray *pHandle, 
		const CUDA_ARRAY_DESCRIPTOR *pAllocateArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuArrayGetDescriptor(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, 
		CUarray hArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuArrayDestroy(CUarray hArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuArray3DCreate(CUarray *pHandle, 
		const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuArray3DGetDescriptor(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, 
		CUarray hArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMipmappedArrayCreate(CUmipmappedArray *pHandle,
		const CUDA_ARRAY3D_DESCRIPTOR *pMipmappedArrayDesc,
		unsigned int numMipmapLevels)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMipmappedArrayGetLevel(CUarray *pLevelArray,
		CUmipmappedArray hMipmappedArray, unsigned int level)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMipmappedArrayDestroy(CUmipmappedArray hMipmappedArray)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuPointerGetAttribute(void *data, CUpointer_attribute attribute,
		CUdeviceptr ptr)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamCreate(CUstream *phStream, unsigned int Flags)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: phStream = [%p]", __func__, phStream);
	cuda_debug("\t(driver) '%s' in: Flags = %d", __func__, Flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	*phStream = cuda_stream_create();
	list_enqueue(active_device->stream_list, *phStream);

	cuda_debug("\t(driver) '%s' in: phStream = [%p]", __func__, *phStream);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuStreamCreateWithPriority(CUstream *phStream, unsigned int flags,
		int priority)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamGetPriority(CUstream hStream, int *priority)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamGetFlags(CUstream hStream, unsigned int *flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags)
{
	struct cuda_stream_command_t *command;
	struct event_args_t *args;
	CUstream stream;
	int i;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);
	cuda_debug("\t(driver) '%s' in: hEvent = [%p]", __func__, hEvent);
	cuda_debug("\t(driver) '%s' in: Flags = %d", __func__, Flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	if (hEvent->to_be_recorded != 0)
	{
		args = xcalloc(1, sizeof(struct event_args_t));
		args->event = hEvent;
		command = cuda_stream_command_create(hStream, cuWaitEventImpl, NULL,
				NULL, args, NULL);
		command->ready_to_run = 1;
		if (hStream == NULL)
		{
			for (i = 0; i < list_count(active_device->stream_list); ++i)
			{
				stream = list_get(active_device->stream_list, i);
				cuda_stream_enqueue(stream, command);
			}
		}
		else
			cuda_stream_enqueue(hStream, command);

		free(args);
	}

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuStreamAddCallback(CUstream hStream, CUstreamCallback callback,
		void *userData, unsigned int flags)
{
	struct cuda_stream_command_t *command;
	struct callback_t *cb;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);
	cuda_debug("\t(driver) '%s' in: callback = [%p]", __func__, callback);
	cuda_debug("\t(driver) '%s' in: userData = [%p]", __func__, userData);
	cuda_debug("\t(driver) '%s' in: flags = %u", __func__, flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	cb = xcalloc(1, sizeof(struct callback_t));
	cb->func = callback;
	cb->stream = hStream;
	cb->userData = userData;

	/* If stream == 0, it is the default stream. */
	if (hStream == 0)
		hStream = list_get(active_device->stream_list, 0);

	command = cuda_stream_command_create(hStream, cuStreamCallbackImpl, NULL,
			NULL, NULL, cb);
	command->ready_to_run = 1;
	cuda_stream_enqueue(hStream, command);

	free(cb);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuStreamQuery(CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamSynchronize(CUstream hStream)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* If stream == 0, it is the default stream. */
	if (hStream == 0)
		hStream = list_get(active_device->stream_list, 0);

	/* Wait until all GPU work is completed. */
	while (list_count(hStream->command_list))
		;

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuStreamDestroy(CUstream hStream)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	list_remove(active_device->stream_list, hStream);
	cuda_stream_free(hStream);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventCreate(CUevent *phEvent, unsigned int Flags)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: phEvent = [%p]", __func__, phEvent);
	cuda_debug("\t(driver) '%s' in: Flags = %u", __func__, Flags);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	*phEvent = cuda_event_create(Flags);

	cuda_debug("\t(driver) '%s' out: phEvent = [%p]", __func__, *phEvent);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventRecord(CUevent hEvent, CUstream hStream)
{
	struct cuda_stream_command_t *command;
	struct event_args_t *args;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hEvent = [%p]", __func__, hEvent);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	hEvent->recorded = 0;
	hEvent->to_be_recorded = 1;
	args = xcalloc(1, sizeof(struct event_args_t));
	args->event = hEvent;

	/* If stream == 0, it is the default stream. */
	if (hStream == 0)
		hStream = list_get(active_device->stream_list, 0);

	command = cuda_stream_command_create(hStream, cuEventRecordImpl, NULL, NULL,
			args, NULL);
	command->ready_to_run = 1;
	cuda_stream_enqueue(hStream, command);

	free(args);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventQuery(CUevent hEvent)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hEvent = [%p]", __func__, hEvent);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	if (!hEvent->recorded)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_READY);

		return CUDA_ERROR_NOT_READY;
	}

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventSynchronize(CUevent hEvent)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hEvent = [%p]", __func__, hEvent);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	while (!hEvent->to_be_recorded)
		;

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventDestroy(CUevent hEvent)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hEvent = [%p]", __func__, hEvent);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	cuda_event_free(hEvent);

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pMilliseconds = [%p]", __func__,
			pMilliseconds);
	cuda_debug("\t(driver) '%s' in: hStart = [%p]", __func__, hStart);
	cuda_debug("\t(driver) '%s' in: hEnd = [%p]", __func__, hEnd);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	if (!hStart->to_be_recorded || !hEnd->to_be_recorded)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_INVALID_HANDLE);
		return CUDA_ERROR_INVALID_HANDLE;
	}

	if (hStart->to_be_recorded && hEnd->to_be_recorded && (!hStart->recorded ||
			!hEnd->recorded))
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_READY);
		return CUDA_ERROR_NOT_READY;
	}

	*pMilliseconds = ((float)(hEnd->t - hStart->t)) / CLOCKS_PER_SEC * 1000;

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);
	return CUDA_SUCCESS;
}

CUresult cuFuncGetAttribute(int *pi, CUfunction_attribute attrib,
		CUfunction hfunc)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: pi = [%p]", __func__, pi);
	cuda_debug("\t(driver) '%s' in: attrib = %d", __func__, attrib);
	cuda_debug("\t(driver) '%s' in: hfunc = [%p]", __func__, hfunc);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	if (attrib == CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK)
		*pi = hfunc->maxThreadsPerBlock;
	else if (attrib == CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES)
		*pi = hfunc->sharedSizeBytes;
	else if (attrib == CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES)
		*pi = hfunc->constSizeBytes;
	else if (attrib == CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES)
		*pi = hfunc->localSizeBytes;
	else if (attrib == CU_FUNC_ATTRIBUTE_NUM_REGS)
		*pi = hfunc->numRegs;
	else if (attrib == CU_FUNC_ATTRIBUTE_PTX_VERSION)
		*pi = hfunc->ptxVersion;
	else if (attrib == CU_FUNC_ATTRIBUTE_BINARY_VERSION)
		*pi = hfunc->binaryVersion;

	cuda_debug("\t(driver) '%s' out: *pi = %d", __func__, *pi);
	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: hfunc = [%p]", __func__, hfunc);
	cuda_debug("\t(driver) '%s' in: config = %d", __func__, config);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	hfunc->cache_config = config;

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuFuncSetSharedMemConfig(CUfunction hfunc, CUsharedconfig config)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLaunchKernel(CUfunction f, unsigned int gridDimX,
		unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX,
		unsigned int blockDimY, unsigned int blockDimZ,
		unsigned int sharedMemBytes, CUstream hStream, void **kernelParams,
		void **extra)
{
	CUstream stream;
	struct cuda_stream_command_t *command;
	struct kernel_args_t *args;
	int i;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) '%s' in: function = [%p]", __func__, f);
	cuda_debug("\t(driver) '%s' in: gridDimX = %u", __func__, gridDimX);
	cuda_debug("\t(driver) '%s' in: gridDimY = %u", __func__, gridDimY);
	cuda_debug("\t(driver) '%s' in: gridDimZ = %u", __func__, gridDimZ);
	cuda_debug("\t(driver) '%s' in: blockDimX = %u", __func__, blockDimX);
	cuda_debug("\t(driver) '%s' in: blockDimY = %u", __func__, blockDimY);
	cuda_debug("\t(driver) '%s' in: blockDimZ = %u", __func__, blockDimZ);
	cuda_debug("\t(driver) '%s' in: sharedMemBytes = %u", __func__,
			sharedMemBytes);
	cuda_debug("\t(driver) '%s' in: hStream = [%p]", __func__, hStream);


	cuda_debug("\t(driver) '%s' in: hStream id = [%d]", __func__,
			hStream->id);


	cuda_debug("\t(driver) '%s' in: kernelParams = [%p]", __func__,
			kernelParams);
	cuda_debug("\t(driver) '%s' in: extra = [%p]", __func__, extra);
	cuda_debug("\t(driver) '%s' function is is %d", __func__, f->id);

	if (!active_device)
	{
		cuda_debug("\t(driver) '%s' out: return = %d", __func__,
				CUDA_ERROR_NOT_INITIALIZED);
		return CUDA_ERROR_NOT_INITIALIZED;
	}

	/* Find stream */
	for (i = 0; i < list_count(active_device->stream_list); ++i)
	{
		stream = list_get(active_device->stream_list, i);
		if (pthread_equal(pthread_self(), stream->user_thread) &&
				stream->configuring)
			break;
	}
	assert(i < list_count(active_device->stream_list));

	/* Update command */
	for (i = 0; i < list_count(stream->command_list); ++i)
	{
		command = list_get(stream->command_list, i);
		if (command->func == cuLaunchKernelImpl)
			break;
	}
	assert(i < list_count(stream->command_list));
	if (!command)  /* Directly called, i.e., CUDA runtime APIs are used. */
	{
		/* If stream == 0, it is the default stream. */
		if (hStream == 0)
			hStream = list_get(active_device->stream_list, 0);

		args = xcalloc(1, sizeof(struct kernel_args_t));
		args->grid_dim_x = gridDimX;
		args->grid_dim_y = gridDimY;
		args->grid_dim_z = gridDimZ;
		args->block_dim_x = blockDimX;
		args->block_dim_y = blockDimY;
		args->block_dim_z = blockDimZ;
		args->shared_mem_size = sharedMemBytes;
		args->kernel = f;
		args->stream = hStream;
		args->kernel_params = kernelParams;
		args->extra = extra;

		command = cuda_stream_command_create(hStream, cuLaunchKernelImpl, NULL,
				args, NULL, NULL);
		command->ready_to_run = 1;
		cuda_stream_enqueue(hStream, command);

		free(args);
	}
	else  /* Indirectly called, i.e., CUDA driver APIs are used. */
	{
		command->k_args.kernel = f;
		command->k_args.stream = hStream;
		command->k_args.kernel_params = kernelParams;
		command->k_args.extra = extra;

		command->ready_to_run = 1;
	}

	cuda_debug("\t(driver) '%s' out: return = %d", __func__, CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuParamSetSize(CUfunction hfunc, unsigned int numbytes)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuParamSeti(CUfunction hfunc, int offset, unsigned int value)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuParamSetf(CUfunction hfunc, int offset, float value)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuParamSetv(CUfunction hfunc, int offset, void *ptr,
		unsigned int numbytes)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLaunch(CUfunction f)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLaunchGrid(CUfunction f, int grid_width, int grid_height)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuLaunchGridAsync(CUfunction f, int grid_width, int grid_height,
		CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetMipmappedArray(CUtexref hTexRef,
		CUmipmappedArray hMipmappedArray, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddress(size_t *ByteOffset, CUtexref hTexRef,
		CUdeviceptr dptr, size_t bytes)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddress2D(CUtexref hTexRef, 
		const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt,
		int NumPackedComponents)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetMipmapFilterMode(CUtexref hTexRef, CUfilter_mode fm)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetMipmapLevelBias(CUtexref hTexRef, float bias)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetMipmapLevelClamp(CUtexref hTexRef,
		float minMipmapLevelClamp, float maxMipmapLevelClamp)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetMaxAnisotropy(CUtexref hTexRef, unsigned int maxAniso)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetAddress(CUdeviceptr *pdptr, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetArray(CUarray *phArray, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetMipmappedArray(CUmipmappedArray *phMipmappedArray,
		CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetAddressMode(CUaddress_mode *pam, CUtexref hTexRef, int dim)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFilterMode(CUfilter_mode *pfm, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFormat(CUarray_format *pFormat, int *pNumChannels,
		CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetMipmapFilterMode(CUfilter_mode *pfm, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetMipmapLevelBias(float *pbias, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetMipmapLevelClamp(float *pminMipmapLevelClamp,
		float *pmaxMipmapLevelClamp, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetMaxAnisotropy(int *pmaxAniso, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFlags(unsigned int *pFlags, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefCreate(CUtexref *pTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexRefDestroy(CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray,
		unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuSurfRefGetArray(CUarray *phArray, CUsurfref hSurfRef)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexObjectCreate(CUtexObject *pTexObject,
		const CUDA_RESOURCE_DESC *pResDesc,
		const CUDA_TEXTURE_DESC *pTexDesc,
		const CUDA_RESOURCE_VIEW_DESC *pResViewDesc)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexObjectDestroy(CUtexObject texObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexObjectGetResourceDesc(CUDA_RESOURCE_DESC *pResDesc,
		CUtexObject texObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexObjectGetTextureDesc(CUDA_TEXTURE_DESC *pTexDesc,
		CUtexObject texObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuTexObjectGetResourceViewDesc(CUDA_RESOURCE_VIEW_DESC *pResViewDesc,
		CUtexObject texObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuSurfObjectCreate(CUsurfObject *pSurfObject,
		const CUDA_RESOURCE_DESC *pResDesc)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuSurfObjectDestroy(CUsurfObject surfObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuSurfObjectGetResourceDesc(CUDA_RESOURCE_DESC *pResDesc,
		CUsurfObject surfObject)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuDeviceCanAccessPeer(int *canAccessPeer, CUdevice dev,
		CUdevice peerDev)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuCtxDisablePeerAccess(CUcontext peerContext)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsUnregisterResource(CUgraphicsResource resource)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsSubResourceGetMappedArray(CUarray *pArray, 
		CUgraphicsResource resource, unsigned int arrayIndex,
		unsigned int mipLevel)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsResourceGetMappedMipmappedArray(
		CUmipmappedArray *pMipmappedArray, CUgraphicsResource resource)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsResourceGetMappedPointer(CUdeviceptr *pDevPtr, 
		size_t *pSize, CUgraphicsResource resource)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsResourceSetMapFlags(CUgraphicsResource resource, 
		unsigned int flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsMapResources(unsigned int count, 
		CUgraphicsResource *resources, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGraphicsUnmapResources(unsigned int count, 
		CUgraphicsResource *resources, CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuGetExportTable(const void **ppExportTable, 
		const CUuuid *pExportTableId)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

