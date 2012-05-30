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

#include <m2s-cuda.h>
#include <debug.h>
#include <list.h>
#include <misc.h>
#include <string.h>



/*
 * Error Messages
 */

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



/* CUDA Driver Data Structures */
#define CU_IPC_HANDLE_SIZE 64

typedef struct CUipcEventHandle_st {
    char reserved[CU_IPC_HANDLE_SIZE];
} CUipcEventHandle;

typedef struct CUipcMemHandle_st {
    char reserved[CU_IPC_HANDLE_SIZE];
} CUipcMemHandle;

typedef enum CUsharedconfig_enum {
    CU_SHARED_MEM_CONFIG_DEFAULT_BANK_SIZE    = 0x00, /**< set default shared memory bank size */
    CU_SHARED_MEM_CONFIG_FOUR_BYTE_BANK_SIZE  = 0x01, /**< set shared memory bank width to four bytes */
    CU_SHARED_MEM_CONFIG_EIGHT_BYTE_BANK_SIZE = 0x02  /**< set shared memory bank width to eight bytes */
} CUsharedconfig;



/* Multi2Sim CUDA Driver Version */
#define FRM_CUDA_VERSION_MAJOR	1
#define FRM_CUDA_VERSION_MINOR	700

struct frm_cuda_version_t
{
	int major;
	int minor;
};

CUresult cuInit(unsigned int Flags)
{
	struct frm_cuda_version_t version;

	int ret;

	/* Version negotiation */
	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_version, &version);

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

	if (Flags != 0U)
		return CUDA_ERROR_INVALID_VALUE;

	return CUDA_SUCCESS;
}


CUresult cuDriverGetVersion(int *driverVersion)
{
	if (driverVersion == NULL)
		return CUDA_ERROR_INVALID_VALUE;

	*driverVersion = 123456789;

	return CUDA_SUCCESS;
}


CUresult cuDeviceGet(CUdevice *device, int ordinal)
{
	if (ordinal >= 1)
		return CUDA_ERROR_INVALID_VALUE;

	*device = 0;

	return CUDA_SUCCESS;
}


CUresult cuDeviceGetCount(int *count)
{
	*count = 1;

	return CUDA_SUCCESS;
}


CUresult cuDeviceGetName(char *name, int len, CUdevice dev)
{
	if (dev != 0)
		return CUDA_ERROR_INVALID_VALUE;

	sprintf(name, "Multi2Sim Fermi Device");
	name[len] = '\0';

	return CUDA_SUCCESS;
}


CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
	if (dev != 0)
		return CUDA_ERROR_INVALID_VALUE;

	*major = 2;
	*minor = 0;

	return CUDA_SUCCESS;
}


CUresult cuDeviceTotalMem(size_t *bytes, CUdevice dev)
{
	if (dev != 0)
		return CUDA_ERROR_INVALID_VALUE;

	*bytes = 1024*1024;

	return CUDA_SUCCESS;
}


CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
	if (dev != 0)
		return CUDA_ERROR_INVALID_VALUE;

	prop->maxThreadsPerBlock = 1;
	prop->maxThreadsDim[0] = 1;
	prop->maxThreadsDim[1] = 1;
	prop->maxThreadsDim[2] = 1;
	prop->maxGridSize[0] = 1;
	prop->maxGridSize[1] = 1;
	prop->maxGridSize[2] = 1;
	prop->sharedMemPerBlock = 1;
	prop->totalConstantMemory = 1;
	prop->SIMDWidth = 1;
	prop->memPitch = 1;
	prop->regsPerBlock = 1;
	prop->clockRate = 1;
	prop->textureAlign = 1;

	return CUDA_SUCCESS;
}


CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev)
{
	*pctx = (CUcontext)malloc(sizeof(struct CUctx_st));
	if (*pctx == NULL)
		fatal("cuCtxCreate: cannot create context");

	return CUDA_SUCCESS;
}


CUresult cuCtxDestroy(CUcontext ctx)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuCtxAttach(CUcontext *pctx, unsigned int flags)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuCtxDetach(CUcontext ctx)
{
	warning("cuCtxDetach: this function is deprecated");
	free(ctx);

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
	*device = 0;

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
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuModuleLoad(CUmodule *module, const char *fname)
{
	FILE *fp;

	fp = fopen(fname, "r");
	if (fp == NULL)
		fatal("cuModuleLoad: cannot load module %s", fname);

	fclose(fp);
	*module = (CUmodule)malloc(sizeof(struct CUmod_st));

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


CUresult cuModuleUnload(CUmodule hmod)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name)
{
	int i;

	for (i = 0; i < 1024; ++i)
	{
		if (!strcmp(name, hmod->func[i].name))
			break;
	}
	if (i == 1024)
		return CUDA_ERROR_NOT_FOUND;
	*hfunc = hmod->func + i;

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
	*free = 1024*1024;
	*total = 1024*1024;

	return CUDA_SUCCESS;
}


CUresult cuMemAlloc(CUdeviceptr *dptr, size_t bytesize)
{
	if (bytesize == 0)
		return CUDA_ERROR_INVALID_VALUE;

	dptr = (CUdeviceptr*)malloc(bytesize);
	if (dptr == NULL)
		fatal("cuMemAlloc: cannot allocate %u bytes", bytesize);

	return CUDA_SUCCESS;
}


CUresult cuMemAllocPitch(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes)
{
	__FRM_CUDA_NOT_IMPL__
	return CUDA_SUCCESS;
}


CUresult cuMemFree(CUdeviceptr dptr)
{
	free(&dptr);

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
	return CUDA_SUCCESS;
}


CUresult cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount)
{
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
	__FRM_CUDA_NOT_IMPL__
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

