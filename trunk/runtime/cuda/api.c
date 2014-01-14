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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
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
#include "module.h"
#include "stream.h"


/*
 * Global Variables
 */

/* Version */
#define CUDA_VERSION_MAJOR 1

#define CUDA_VERSION_MINOR 1010

struct cuda_version_t
{
	int major;
	int minor;
};

/* Debug */
static int cuda_debug_enable;

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

/* If the environment variable 'M2S_CUDA_DEBUG' is set to 1, the Multi2Sim CUDA
 * Runtime/Driver library will dump debugging information about CUDA calls,
 * argument values, intermediate actions, and return values. */
void cuda_debug(char *fmt, ...)
{
	va_list va;
	char str[1024];
	static int cuda_debug_init = 0;

	/* Initialize debug */
	if (! cuda_debug_init)
	{
		cuda_debug_init = 1;
		cuda_debug_enable = atoi(getenv("M2S_CUDA_DEBUG"));
	}

	/* Exit if not enabled */
	if (! cuda_debug_enable)
		return;

	/* Reconstruct message in 'str' first. This is done to avoid multiple
	 * calls to 'printf', that can have race conditions among threads. */
	va_start(va, fmt);
	vsnprintf(str, sizeof str, fmt, va);
	fprintf(stderr, "[libm2s-cuda] %s\n", str);
}

void versionCheck(void)
{
	struct cuda_version_t version;
	int ret;

	cuda_debug("CUDA driver internal function '%s'", __func__);

	/* Version negotiation */
	ret = syscall(CUDA_SYS_CODE, cuda_call_versionCheck, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	/* Check that exact major version matches */
	if (version.major != CUDA_VERSION_MAJOR || version.minor <
			CUDA_VERSION_MINOR)
		fatal("%s:%d: incompatible CUDA versions. Guest library v. %d.%d / "
				"Host implementation v. %d.%d.\n"
				"\t%s", __FILE__, __LINE__,	CUDA_VERSION_MAJOR,
				CUDA_VERSION_MINOR, version.major, version.minor,
				cuda_err_version);
}


/*
 * Public CUDA Driver Library
 */

CUresult cuInit(unsigned int Flags)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: Flags = %u", Flags);

	/* Check version */
	versionCheck();

	/* Check input */
	if (Flags != 0U)
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Create CUDA object lists */
	context_list = list_create();
	device_list = list_create();
	module_list = list_create();
	function_list = list_create();
	stream_list = list_create();
	event_list = list_create();

	/* Create one Fermi device and one Kepler device */
	frm_device = cuda_device_create(CUDA_DEVICE_FERMI);
	kpl_device = cuda_device_create(CUDA_DEVICE_KEPLER);

	/* Fermi is selected by default. This could change if the user specifies
	 * which device should be used in cuDeviceGet function */
	active_devices = frm_device;

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuInit);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDriverGetVersion(int *driverVersion)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: driverVersion = [%p]", driverVersion);

	/* Check input */
	if (driverVersion == NULL)
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Version 5.5 */
	*driverVersion = 5050;

	cuda_debug("\t(driver) out: driverVersion = %d", *driverVersion);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGet(CUdevice *device, int ordinal)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: device = [%p]", device);
	cuda_debug("\t(driver) in: ordinal = %d", ordinal);

	/* Check input */
	if (ordinal >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get device */
	active_devices = list_get(device_list, ordinal);
	*device = active_devices->device;

	cuda_debug("\t(driver) out: device = %d", *device);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetCount(int *count)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: count = [%p]", count);

	*count = list_count(device_list);

	cuda_debug("\t(driver) out: count = %d", *count);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetName(char *name, int len, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: name = [%p]", name);
	cuda_debug("\t(driver) in: len = %d", len);
	cuda_debug("\t(driver) in: dev = %d", dev);

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Copy name */
	strncpy(name, ((struct cuda_device_t *)list_get(device_list, dev))->name,
			len);

	cuda_debug("\t(driver) out: name = %s", name);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceTotalMem(size_t *bytes, CUdevice dev)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: bytes = %p", bytes);
	cuda_debug("\t(driver) in: dev = %d", dev);

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuDeviceTotalMem, bytes);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: bytes = %d", *bytes);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: pi = [%p]", pi);
	cuda_debug("\t(driver) in: attrib = %d", attrib);
	cuda_debug("\t(driver) in: dev = %d", dev);

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get attribute */
	*pi = (((struct cuda_device_t *)list_get(device_list, dev))->attributes)
			[attrib];

	cuda_debug("\t(driver) out: pi = %d", *pi);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
	struct cuda_device_t *device;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: prop = %p", prop);
	cuda_debug("\t(driver) in: dev = %d", dev);

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get device */
	device = (struct cuda_device_t *)list_get(device_list, dev);

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

	cuda_debug("\t(driver) out: prop = %d", prop->maxThreadsPerBlock);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
	struct cuda_device_t *device;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: major = [%p]", major);
	cuda_debug("\t(driver) in: minor = [%p]", minor);
	cuda_debug("\t(driver) in: dev = %d", dev);

	/* Check input */
	if (dev >= list_count(device_list))
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Get version */
	device = (struct cuda_device_t *)list_get(device_list, dev);
	*major = device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR];
	*minor = device->attributes[CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR];

	cuda_debug("\t(driver) out: major = %d", *major);
	cuda_debug("\t(driver) out: minor = %d", *minor);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: pctx = [%p]", pctx);
	cuda_debug("\t(driver) in: flags = %u", flags);
	cuda_debug("\t(driver) in: dev = %d", dev);

	*pctx = cuda_context_create(dev);

	cuda_debug("\t(driver) out: pctx = %p", *pctx);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxDestroy(CUcontext ctx)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: ctx = %p", ctx);

	cuda_context_free(ctx);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: ctx = %p", ctx);
	cuda_debug("\t(driver) in: version = [%p]", version);

	*version= ((CUcontext)list_get(context_list, ctx->id))->version;

	cuda_debug("\t(driver) out: version = %u", *version);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: module = [%p]", module);
	cuda_debug("\t(driver) in: fname = [%p] %s", fname, fname);

	/* Create module */
	*module = cuda_module_create(fname);

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleLoad, fname);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: module = %p", *module);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: hmod = [%p]", hmod);

	/* Free module in driver */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleUnload, hmod->id);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	/* Free module in runtime */
	cuda_module_free(hmod);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: hfunc = [%p]", hfunc);
	cuda_debug("\t(driver) in: hmod = [%p]", hmod);
	cuda_debug("\t(driver) in: name = [%p] %s", name, name);

	/* Create function */
	*hfunc = cuda_function_create(hmod, name);

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuModuleGetFunction, hmod->id, name,
			(*hfunc)->inst_buf, (*hfunc)->inst_buf_size, (*hfunc)->numRegs);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: hfunc = %p", *hfunc);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: free = [%p]", free);
	cuda_debug("\t(driver) in: total = [%p]", total);

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuMemGetInfo, free, total);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: free = %d", *free);
	cuda_debug("\t(driver) out: total = %d", *total);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemAlloc(CUdeviceptr *dptr, size_t bytesize)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: device_ptr = [%p]", dptr);
	cuda_debug("\t(driver) in: bytesize = %d", bytesize);

	if (bytesize == 0)
	{
		cuda_debug("\t(driver) out: return = %d", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Syscall */
	if (active_devices->type == CUDA_DEVICE_FERMI)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuFrmMemAlloc, dptr, bytesize);
	else if (active_devices->type == CUDA_DEVICE_KEPLER)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemAlloc, dptr, bytesize);
	else
		fatal("device not supported.\n");


	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: device_ptr = 0x%08x", *dptr);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: device_ptr = 0x%08x", dptr);

	/* Syscall */
	ret = syscall(CUDA_SYS_CODE, cuda_call_cuMemFree, dptr);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	cuda_debug("\t(driver) in: pp = [%p]", pp);
	cuda_debug("\t(driver) in: bytesize = %d", bytesize);

	*pp = xcalloc(1, bytesize);

	cuda_debug("\t(driver) out: pp = %p", *pp);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemFreeHost(void *p)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p,
		unsigned int Flags)
{
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
	cuda_debug("\t(driver) in: dstDevice = 0x%08x",dstDevice);
	cuda_debug("\t(driver) in: srcHost = [%p]", srcHost);
	cuda_debug("\t(driver) in: ByteCount = %d", ByteCount);

	/* Syscall */
	if (active_devices->type == CUDA_DEVICE_FERMI)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuFrmMemcpyHtoD, 
				dstDevice, srcHost, ByteCount);
	else if (active_devices->type == CUDA_DEVICE_KEPLER)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyHtoD, 
				dstDevice, srcHost, ByteCount);
	else
		fatal("device not supported.\n");

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount)
{
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: dstHost = [%p]", dstHost);
	cuda_debug("\t(driver) in: srcDevice = 0x%08x", srcDevice);
	cuda_debug("\t(driver) in: ByteCount = %d", ByteCount);

	/* Syscall */
	if (active_devices->type == CUDA_DEVICE_FERMI)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuFrmMemcpyDtoH, dstHost,
				srcDevice, ByteCount);
	else if (active_devices->type == CUDA_DEVICE_KEPLER)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyDtoH, dstHost,
				srcDevice, ByteCount);
	else
		fatal("device not supported.\n");

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	__CUDA_NOT_IMPL__;
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
	cuda_debug("\t(driver) in: dstDevice = 0x%08x", dstDevice);
	cuda_debug("\t(driver) in: uc = 0x%x", uc);
	cuda_debug("\t(driver) in: N = %d", N);

	/* Syscall */
	src_host = xcalloc(N, sizeof(unsigned char));
	memset(src_host, uc, N);
	if (active_devices->type == CUDA_DEVICE_FERMI)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuFrmMemcpyHtoD, dstDevice,
			src_host, N);
	else if (active_devices->type == CUDA_DEVICE_KEPLER)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplMemcpyHtoD, dstDevice,
			src_host, N);
	else
		fatal("device not supported.\n");

	free(src_host);

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
	__CUDA_NOT_IMPL__;
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
	__CUDA_NOT_IMPL__;
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
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamAddCallback(CUstream hStream, CUstreamCallback callback,
		void *userData, unsigned int flags)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamQuery(CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamSynchronize(CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuStreamDestroy(CUstream hStream)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuEventCreate(CUevent *phEvent, unsigned int Flags)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: phEvent = [%p]", phEvent);
	cuda_debug("\t(driver) in: Flags = %u", Flags);

	*phEvent = cuda_event_create(Flags);

	cuda_debug("\t(driver) out: phEvent = %p", *phEvent);
	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventRecord(CUevent hEvent, CUstream hStream)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: hEvent = [%p]", hEvent);
	cuda_debug("\t(driver) in: hStream = [%p]", hStream);

	cuda_event_record(hEvent);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventQuery(CUevent hEvent)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuEventSynchronize(CUevent hEvent)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuEventDestroy(CUevent hEvent)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: hEvent = [%p]", hEvent);

	cuda_event_free(hEvent);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd)
{
	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: pMilliseconds = [%p]", pMilliseconds);
	cuda_debug("\t(driver) in: hStart = [%p]", hStart);
	cuda_debug("\t(driver) in: hEnd = [%p]", hEnd);

	*pMilliseconds = ((float)(hEnd->t - hStart->t)) / CLOCKS_PER_SEC * 1000;

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);
	return CUDA_SUCCESS;
}

CUresult cuFuncGetAttribute(int *pi, CUfunction_attribute attrib,
		CUfunction hfunc)
{
	__CUDA_NOT_IMPL__;
	return CUDA_SUCCESS;
}

CUresult cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config)
{
	__CUDA_NOT_IMPL__;
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
	unsigned int sys_args[11];
	int ret;

	cuda_debug("CUDA driver API '%s'", __func__);
	cuda_debug("\t(driver) in: function = [%p]", f);
	cuda_debug("\t(driver) in: gridDimX = %u", gridDimX);
	cuda_debug("\t(driver) in: gridDimY = %u", gridDimY);
	cuda_debug("\t(driver) in: gridDimZ = %u", gridDimZ);
	cuda_debug("\t(driver) in: blockDimX = %u", blockDimX);
	cuda_debug("\t(driver) in: blockDimY = %u", blockDimY);
	cuda_debug("\t(driver) in: blockDimZ = %u", blockDimZ);
	cuda_debug("\t(driver) in: sharedMemBytes = %u", sharedMemBytes);
	cuda_debug("\t(driver) in: hStream = [%p]", hStream);
	cuda_debug("\t(driver) in: kernelParams = [%p]", kernelParams);
	cuda_debug("\t(driver) in: extra = [%p]", extra);

	/* Check input */
	assert(gridDimX != 0 && gridDimY != 0 && gridDimZ != 0);
	assert(blockDimX != 0 && blockDimY != 0 && blockDimZ != 0);

	/* Syscall arguments */
	sys_args[0] = f->id;
	sys_args[1] = gridDimX;
	sys_args[2] = gridDimY;
	sys_args[3] = gridDimZ;
	sys_args[4] = blockDimX;
	sys_args[5] = blockDimY;
	sys_args[6] = blockDimZ;
	sys_args[7] = sharedMemBytes;
	sys_args[8] = (hStream ? hStream->id : 0);
	sys_args[9] = (unsigned int)kernelParams;
	sys_args[10] = (unsigned int)extra;

	/* Syscall */
	if (active_devices->type == CUDA_DEVICE_FERMI)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuFrmLaunchKernel, sys_args);
	else if (active_devices->type == CUDA_DEVICE_KEPLER)
		ret = syscall(CUDA_SYS_CODE, cuda_call_cuKplLaunchKernel, sys_args);
	else
		fatal("device not supported.\n");

	/* Check that we are running on Multi2Sim. If a program linked with this
	 * library is running natively, system call CUDA_SYS_CODE is not
	 * supported. */
	if (ret)
		fatal("native execution not supported.\n%s", cuda_err_native);

	cuda_debug("\t(driver) out: return = %d", CUDA_SUCCESS);

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
