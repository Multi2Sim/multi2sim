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

#include "../include/cuda.h"
#include "api.h"
#include "debug.h"

#include <stdio.h>
#include <unistd.h>


/* Debug */
int frm_cuda_debug = 0;

/* Error messages */
#define __FRM_CUDA_NOT_IMPL__  warning("%s: not implemented.\n%s", \
	__FUNCTION__, err_frm_cuda_not_impl);

static char *err_frm_cuda_not_impl =
	"\tMulti2Sim provides partial support for CUDA driver library.\n"
	"\tTo request the implementation of a certain functionality,\n"
	"\tplease email development@multi2sim.org.\n";

static char *err_frm_cuda_version =
	"\tYour guest application is using a version of the CUDA driver library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tCUDA driver library ('libm2s-cuda').\n";

static char *err_frm_cuda_native =
	"\tYou are trying to run natively an application using the Multi2Sim CUDA driver\n"
	"\tlibrary implementation ('libm2s-cuda'). Please run this program on top of\n"
	"\tMulti2Sim.\n";




/*
 * Data Structures
 */

#define FRM_CUDA_VERSION_MAJOR	1
#define FRM_CUDA_VERSION_MINOR	700

struct frm_cuda_version_t
{
	int major;
	int minor;
};




/*
 * Internal Functions
 */


void versionCheck(void)
{
	char *env;
	struct frm_cuda_version_t version;
	int ret;

	env = getenv("LIBM2S_CUDA_DUMP");
	frm_cuda_debug = ((env[0] == '1') && (env[1] == '\0'));

	cuda_debug(stdout, "CUDA driver internal function '%s'\n", __FUNCTION__);

	/* Version negotiation */
	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_versionCheck, &version);

	cuda_debug(stdout, "\t(driver) out: version.major=%d version.minor=%d\n", version.major, version.minor);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	/* Check that exact major version matches */
	if (version.major != FRM_CUDA_VERSION_MAJOR
			|| version.minor < FRM_CUDA_VERSION_MINOR)
		fatal("incompatible CUDA versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", FRM_CUDA_VERSION_MAJOR, FRM_CUDA_VERSION_MINOR,
			version.major, version.minor, err_frm_cuda_version);
}




/*
 * Multi2Sim CUDA Driver Library
 */

CUresult cuInit(unsigned int Flags)
{
	versionCheck();

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: Flags=%u\n", Flags);

	if (Flags != 0U)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDriverGetVersion(int *driverVersion)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);

	if (driverVersion == NULL)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	*driverVersion = 4000;

	cuda_debug(stdout, "\t(driver) out: driverVersion=%d\n", *driverVersion);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGet(CUdevice *device, int ordinal)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: ordinal=%d\n", ordinal);

	if (ordinal >= 1)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	*device = 0;

	cuda_debug(stdout, "\t(driver) out: device=%d\n", *device);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetCount(int *count)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);

	*count = 1;

	cuda_debug(stdout, "\t(driver) out: count=%d\n", *count);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetName(char *name, int len, CUdevice dev)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: len=%d\n", len);
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	if (dev != 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	snprintf(name, len, "Multi2Sim Fermi Device");

	cuda_debug(stdout, "\t(driver) out: name=%s\n", name);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: major_ptr=%p\n", major);
	cuda_debug(stdout, "\t(driver) in: minor_ptr=%p\n", minor);
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	if (dev != 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	*major = 2;
	*minor = 0;

	cuda_debug(stdout, "\t(driver) out: major=%d\n", *major);
	cuda_debug(stdout, "\t(driver) out: minor=%d\n", *minor);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceTotalMem(size_t *bytes, CUdevice dev)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	if (dev != 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	*bytes = 1 << 31;

	cuda_debug(stdout, "\t(driver) out: bytes=%d\n", *bytes);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	if (dev != 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	prop->maxThreadsPerBlock = 1024;
	prop->maxThreadsDim[0] = 1024;
	prop->maxThreadsDim[1] = 1024;
	prop->maxThreadsDim[2] = 64;
	prop->maxGridSize[0] = 65535;
	prop->maxGridSize[1] = 65535;
	prop->maxGridSize[2] = 65535;
	prop->sharedMemPerBlock = 49152;
	prop->totalConstantMemory = 65536;
	prop->SIMDWidth = 32; /* warp size */
	prop->memPitch = 2147483647;
	prop->regsPerBlock = 32768;
	prop->clockRate = 1000000;
	/* FIXME: may not be 512 */
	prop->textureAlign = 512;

	cuda_debug(stdout, "\t(driver) out: prop->maxThreadsPerBlock=%d prop->maxThreadsDim[0]=%d \
		prop->maxThreadsDim[1]=%d prop->maxThreadsDim[2]=%d prop->maxGridSize[0]=%d \
		prop->maxGridSize[1]=%d prop->maxGridSize[2]=%d prop->sharedMemPerBlock=%d \
		prop->totalConstantMemory=%d prop->SIMDWidth=%d prop->memPitch=%d \
		prop->regsPerBlock=%d prop->clockRate=%d prop->textureAlign=%d\n", 
		prop->maxThreadsPerBlock, prop->maxThreadsDim[0], prop->maxThreadsDim[1], 
		prop->maxThreadsDim[2], prop->maxGridSize[0], prop->maxGridSize[1], 
		prop->maxGridSize[2], prop->sharedMemPerBlock, prop->totalConstantMemory, 
		prop->SIMDWidth, prop->memPitch, prop->regsPerBlock, prop->clockRate, prop->textureAlign);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: attrib=%d\n", attrib);
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	if (dev != 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	if (attrib == CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK)
		*pi = 512;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X)
		*pi = 1024;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y)
		*pi = 1024;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z)
		*pi = 64;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X)
		*pi = 65535;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y)
		*pi = 65535;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z)
		*pi = 65535;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK)
		*pi = 49152;
	else if (attrib == CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY)
		*pi = 65536;
	else if (attrib == CU_DEVICE_ATTRIBUTE_WARP_SIZE)
		*pi = 32;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_PITCH)
		*pi = 2147483647;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH)
		*pi = 65536;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH)
		*pi = 65536;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT)
		*pi = 65535;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH)
		*pi = 2048;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT)
		*pi = 2048;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH)
		*pi = 2048;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH)
		*pi = 16384;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS)
		*pi = 2048;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH)
		*pi = 16384;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT)
		*pi = 16384;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS)
		*pi = 2048;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK)
		*pi = 32768;
	else if (attrib == CU_DEVICE_ATTRIBUTE_CLOCK_RATE)
		*pi = 1000000;
	/* FIXME: may not be 512 */
	else if (attrib == CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT)
		*pi = 512;
	else if (attrib == CU_DEVICE_ATTRIBUTE_GPU_OVERLAP)
		*pi = 1;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT)
		*pi = 16;
	/* FIXME */
	else if (attrib == CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT)
		*pi = 1;
	/* FIXME */
	else if (attrib == CU_DEVICE_ATTRIBUTE_INTEGRATED)
		*pi = 0;
	/* FIXME */
	else if (attrib == CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY)
		*pi = 1;
	else if (attrib == CU_DEVICE_ATTRIBUTE_COMPUTE_MODE)
		*pi = CU_COMPUTEMODE_DEFAULT;
	else if (attrib == CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS)
		*pi = 1;
	else if (attrib == CU_DEVICE_ATTRIBUTE_ECC_ENABLED)
		*pi = 0;
	/* FIXME */
	else if (attrib == CU_DEVICE_ATTRIBUTE_PCI_BUS_ID)
		*pi = 8;
	/* FIXME */
	else if (attrib == CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID)
		*pi = 0;
	else if (attrib == CU_DEVICE_ATTRIBUTE_TCC_DRIVER)
		*pi = 0;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE)
		*pi = 1000000;
	else if (attrib == CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH)
		*pi = 384;
	else if (attrib == CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE)
		*pi = 768*1024;
	else if (attrib == CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR)
		*pi = 1536;
	else if (attrib == CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING)
		*pi = 0;

	cuda_debug(stdout, "\t(driver) out: pi=%d", *pi);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev)
{
	unsigned int sys_args[2];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: flags=%u\n", flags); /* FIXME: flags specifies scheduling policy */
	cuda_debug(stdout, "\t(driver) in: dev=%d\n", dev);

	/* Create context */
	*pctx = (CUcontext)malloc(sizeof(struct CUctx_st));
	if (*pctx == NULL)
		fatal("%s: cannot create context", __FUNCTION__);

	/* Syscall */
	sys_args[0] = (unsigned int)pctx;
	sys_args[1] = (unsigned int)dev;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuCtxCreate, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: context.id=0x%08x\n", (*pctx)->id);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxDestroy(CUcontext ctx)
{
	unsigned int sys_args[1];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: context.id=0x%08x\n", ctx->id);

	/* Syscall */
	sys_args[0] = (unsigned int)ctx;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuCtxDestroy, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	/* Free context */
	if (ctx != NULL)
		free(ctx);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxAttach(CUcontext *pctx, unsigned int flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxDetach(CUcontext ctx)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: context.id=0x%08x\n", ctx->id);

	cuCtxDestroy(ctx);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxPushCurrent(CUcontext ctx)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxPopCurrent(CUcontext *pctx)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxSetCurrent(CUcontext ctx)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetCurrent(CUcontext *pctx)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetDevice(CUdevice *device)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);

	*device = 0;

	cuda_debug(stdout, "\t(driver) out: device=%d\n", *device);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuCtxSynchronize(void)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxSetLimit(CUlimit limit, size_t value)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetLimit(size_t *pvalue, CUlimit limit)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetCacheConfig(CUfunc_cache *pconfig)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxSetCacheConfig(CUfunc_cache config)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetSharedMemConfig(CUsharedconfig *pConfig)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxSetSharedMemConfig(CUsharedconfig config)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxGetApiVersion(CUcontext ctx, unsigned int *version)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);

	*version= 4000U;

	cuda_debug(stdout, "\t(driver) out: version=%u\n", *version);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleLoad(CUmodule *module, const char *fname)
{
	unsigned int sys_args[2];
	int ret;
	FILE *fp;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: fname=%s\n", fname);

	/* Determine if the file exists */
	fp = fopen(fname, "r");
	if (fp == NULL)
		fatal("%s: cannot open file %s", __FUNCTION__, fname);
	fclose(fp);

	/* Create module */
	*module = (CUmodule)malloc(sizeof(struct CUmod_st));

	/* Syscall */
        sys_args[0] = (unsigned int)module;
        sys_args[1] = (unsigned int)fname;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuModuleLoad, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: module.id=0x%08x\n", (*module)->id);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleLoadData(CUmodule *module, const void *image)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

/* FIXME */
CUresult cuModuleUnload(CUmodule hmod)
{
	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: module.id=0x%08x\n", hmod->id);

	/* Free context */
	if (hmod != NULL)
		free(hmod);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name)
{
	unsigned int sys_args[3];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: module.id=0x%08x\n", hmod->id);
	cuda_debug(stdout, "\t(driver) in: name=%s\n", name);

	/* Create function */
	*hfunc = (CUfunction)malloc(sizeof(struct CUfunc_st));

	/* Syscall */
	sys_args[0] = (unsigned int)hfunc;
	sys_args[1] = (unsigned int)hmod;
	sys_args[2] = (unsigned int)name;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuModuleGetFunction, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: function.id=0x%08x\n", (*hfunc)->id);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuModuleGetGlobal(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod, const char *name)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuModuleGetTexRef(CUtexref *pTexRef, CUmodule hmod, const char *name)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuModuleGetSurfRef(CUsurfref *pSurfRef, CUmodule hmod, const char *name)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemGetInfo(size_t *free, size_t *total)
{
	unsigned int sys_args[2];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);

	/* Syscall */
	sys_args[0] = (unsigned int)free;
	sys_args[1] = (unsigned int)total;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuMemGetInfo, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: free=%d\n", *free);
	cuda_debug(stdout, "\t(driver) out: total=%d\n", *total);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemAlloc(CUdeviceptr *dptr, size_t bytesize)
{
	unsigned int sys_args[2];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: bytesize=%d\n", bytesize);

	if (bytesize == 0)
	{
		cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_ERROR_INVALID_VALUE);
		return CUDA_ERROR_INVALID_VALUE;
	}

	/* Syscall */
	sys_args[0] = (unsigned int)dptr;
	sys_args[1] = (unsigned int)bytesize;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuMemAlloc, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: dptr=0x%08x\n", *dptr);
	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemAllocPitch(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemFree(CUdeviceptr dptr)
{
	unsigned int sys_args[1];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: dptr=0x%08x\n", dptr);

	/* Syscall */
	sys_args[0] = (unsigned int)dptr;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuMemFree, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemGetAddressRange(CUdeviceptr *pbase, size_t *psize, CUdeviceptr dptr)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemAllocHost(void **pp, size_t bytesize)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemFreeHost(void *p)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemHostGetFlags(unsigned int *pFlags, void *p)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuDeviceGetByPCIBusId(CUdevice *dev, char *pciBusId)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuDeviceGetPCIBusId(char *pciBusId, int len, CUdevice dev)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuIpcGetEventHandle(CUipcEventHandle *pHandle, CUevent event)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuIpcOpenEventHandle(CUevent *phEvent, CUipcEventHandle handle)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuIpcGetMemHandle(CUipcMemHandle *pHandle, CUdeviceptr dptr)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuIpcOpenMemHandle(CUdeviceptr *pdptr, CUipcMemHandle handle, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuIpcCloseMemHandle(CUdeviceptr dptr)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemHostRegister(void *p, size_t bytesize, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemHostUnregister(void *p)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount)
{
	unsigned int sys_args[3];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: dstDevice=0x%08x\n", dstDevice);
	cuda_debug(stdout, "\t(driver) in: srcHost=%p\n", srcHost);
	cuda_debug(stdout, "\t(driver) in: ByteCount=%d\n", ByteCount);

	/* Syscall */
	sys_args[0] = (unsigned int)dstDevice;
	sys_args[1] = (unsigned int)srcHost;
	sys_args[2] = (unsigned int)ByteCount;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuMemcpyHtoD, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount)
{
	unsigned int sys_args[3];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: dstHost=%p\n", dstHost);
	cuda_debug(stdout, "\t(driver) in: srcDevice=0x%08x\n", srcDevice);
	cuda_debug(stdout, "\t(driver) in: ByteCount=%d\n", ByteCount);

	/* Syscall */
	sys_args[0] = (unsigned int)dstHost;
	sys_args[1] = (unsigned int)srcDevice;
	sys_args[2] = (unsigned int)ByteCount;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuMemcpyDtoH, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoD(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoA(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoD(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoA(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoH(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoA(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2D(const CUDA_MEMCPY2D *pCopy)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3D(const CUDA_MEMCPY3D *pCopy)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoDAsync(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoHAsync(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyDtoDAsync(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyHtoAAsync(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpyAtoHAsync(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD8(CUdeviceptr dstDevice, unsigned char uc, size_t N)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD16(CUdeviceptr dstDevice, unsigned short us, size_t N)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD32(CUdeviceptr dstDevice, unsigned int ui, size_t N)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D8(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D16(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D32(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuArrayCreate(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuArrayGetDescriptor(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuArrayDestroy(CUarray hArray)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuArray3DCreate(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuArray3DGetDescriptor(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuPointerGetAttribute(void *data, CUpointer_attribute attribute, CUdeviceptr ptr)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuStreamCreate(CUstream *phStream, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuStreamQuery(CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuStreamSynchronize(CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuStreamDestroy(CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventCreate(CUevent *phEvent, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventRecord(CUevent hEvent, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventQuery(CUevent hEvent)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventSynchronize(CUevent hEvent)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventDestroy(CUevent hEvent)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuFuncGetAttribute(int *pi, CUfunction_attribute attrib, CUfunction hfunc)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuFuncSetSharedMemConfig(CUfunction hfunc, CUsharedconfig config)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuLaunchKernel(CUfunction f,
	unsigned int gridDimX,
	unsigned int gridDimY,
	unsigned int gridDimZ,
	unsigned int blockDimX,
	unsigned int blockDimY,
	unsigned int blockDimZ,
	unsigned int sharedMemBytes,
	CUstream hStream,
	void **kernelParams,
	void **extra)
{
        unsigned int sys_args[11];
	int ret;

	cuda_debug(stdout, "CUDA driver API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(driver) in: function_id=%u\n", f->id);
	cuda_debug(stdout, "\t(driver) in: gridDimX=%u\n", gridDimX);
	cuda_debug(stdout, "\t(driver) in: gridDimY=%u\n", gridDimY);
	cuda_debug(stdout, "\t(driver) in: gridDimZ=%u\n", gridDimZ);
	cuda_debug(stdout, "\t(driver) in: blockDimX=%u\n", blockDimX);
	cuda_debug(stdout, "\t(driver) in: blockDimY=%u\n", blockDimY);
	cuda_debug(stdout, "\t(driver) in: blockDimZ=%u\n", blockDimZ);
	cuda_debug(stdout, "\t(driver) in: sharedMemBytes=%u\n", sharedMemBytes);
	cuda_debug(stdout, "\t(driver) in: hStream=%p\n", hStream);
	cuda_debug(stdout, "\t(driver) in: kernelParams=%p\n", kernelParams);
	cuda_debug(stdout, "\t(driver) in: extra=%p\n", extra);

	/* Syscall */
        sys_args[0] = (unsigned int)f;
        sys_args[1] = (unsigned int)gridDimX;
        sys_args[2] = (unsigned int)gridDimY;
        sys_args[3] = (unsigned int)gridDimZ;
        sys_args[4] = (unsigned int)blockDimX;
        sys_args[5] = (unsigned int)blockDimY;
        sys_args[6] = (unsigned int)blockDimZ;
        sys_args[7] = (unsigned int)sharedMemBytes;
        sys_args[8] = (unsigned int)hStream;
        sys_args[9] = (unsigned int)kernelParams;
        sys_args[10] = (unsigned int)extra;

	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cuLaunchKernel, sys_args);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	cuda_debug(stdout, "\t(driver) out: return=%d\n", CUDA_SUCCESS);

	return CUDA_SUCCESS;
}

CUresult cuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuParamSetSize(CUfunction hfunc, unsigned int numbytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuParamSeti(CUfunction hfunc, int offset, unsigned int value)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuParamSetf(CUfunction hfunc, int offset, float value)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuParamSetv(CUfunction hfunc, int offset, void *ptr, unsigned int numbytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuLaunch(CUfunction f)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuLaunchGrid(CUfunction f, int grid_width, int grid_height)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuLaunchGridAsync(CUfunction f, int grid_width, int grid_height, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddress(size_t *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddress2D(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetAddress(CUdeviceptr *pdptr, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetArray(CUarray *phArray, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetAddressMode(CUaddress_mode *pam, CUtexref hTexRef, int dim)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFilterMode(CUfilter_mode *pfm, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFormat(CUarray_format *pFormat, int *pNumChannels, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefGetFlags(unsigned int *pFlags, CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefCreate(CUtexref *pTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuTexRefDestroy(CUtexref hTexRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuSurfRefGetArray(CUarray *phArray, CUsurfref hSurfRef)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuDeviceCanAccessPeer(int *canAccessPeer, CUdevice dev, CUdevice peerDev)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuCtxDisablePeerAccess(CUcontext peerContext)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsUnregisterResource(CUgraphicsResource resource)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsSubResourceGetMappedArray(CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsResourceGetMappedPointer(CUdeviceptr *pDevPtr, size_t *pSize, CUgraphicsResource resource)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsResourceSetMapFlags(CUgraphicsResource resource, unsigned int flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsMapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGraphicsUnmapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

CUresult cuGetExportTable(const void **ppExportTable, const CUuuid *pExportTableId)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}

