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



/*
 * Error Messages
 */

#define __CUDA_NOT_IMPL__  warning("%s: not implemented.\n%s", \
	__FUNCTION__, err_cuda_not_impl);


static char *err_cuda_not_impl =
	"\tMulti2Sim provides partial support for CUDA driver library.\n"
	"\tTo request the implementation of a certain functionality,\n"
	"\tplease email development@multi2sim.org.\n";

static char *err_cuda_version =
	"\tYour guest application is using a version of the CUDA driver library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tCUDA driver library ('libm2s-cuda').\n";



/* Multi2Sim CUDA driver required */
#define CUDA_VERSION_MAJOR	0
#define CUDA_VERSION_MINOR	0

struct cuda_version_t
{
	int major;
	int minor;
};



/* CUDA driver data structures */
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



/* CUDA driver API */
CUresult cuInit(unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDriverGetVersion(int *driverVersion)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGet(CUdevice *device, int ordinal)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetCount(int *count)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetName(char *name, int len, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceTotalMem(size_t *bytes, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxDestroy(CUcontext ctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxAttach(CUcontext *pctx, unsigned int flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxDetach(CUcontext ctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxPushCurrent(CUcontext ctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxPopCurrent(CUcontext *pctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxSetCurrent(CUcontext ctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetCurrent(CUcontext *pctx)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetDevice(CUdevice *device)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxSynchronize(void)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxSetLimit(CUlimit limit, size_t value)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetLimit(size_t *pvalue, CUlimit limit)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetCacheConfig(CUfunc_cache *pconfig)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxSetCacheConfig(CUfunc_cache config)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetSharedMemConfig(CUsharedconfig *pConfig)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxSetSharedMemConfig(CUsharedconfig config)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxGetApiVersion(CUcontext ctx, unsigned int *version)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleLoad(CUmodule *module, const char *fname)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleLoadData(CUmodule *module, const void *image)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleUnload(CUmodule hmod)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleGetGlobal(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod, const char *name)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleGetTexRef(CUtexref *pTexRef, CUmodule hmod, const char *name)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuModuleGetSurfRef(CUsurfref *pSurfRef, CUmodule hmod, const char *name)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemGetInfo(size_t *free, size_t *total)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemAlloc(CUdeviceptr *dptr, size_t bytesize)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemAllocPitch(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemFree(CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemGetAddressRange(CUdeviceptr *pbase, size_t *psize, CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemAllocHost(void **pp, size_t bytesize)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemFreeHost(void *p)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemHostGetFlags(unsigned int *pFlags, void *p)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetByPCIBusId(CUdevice *dev, char *pciBusId)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceGetPCIBusId(char *pciBusId, int len, CUdevice dev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuIpcGetEventHandle(CUipcEventHandle *pHandle, CUevent event)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuIpcOpenEventHandle(CUevent *phEvent, CUipcEventHandle handle)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuIpcGetMemHandle(CUipcMemHandle *pHandle, CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuIpcOpenMemHandle(CUdeviceptr *pdptr, CUipcMemHandle handle, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuIpcCloseMemHandle(CUdeviceptr dptr)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemHostRegister(void *p, size_t bytesize, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemHostUnregister(void *p)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyDtoD(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyDtoA(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyAtoD(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyHtoA(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyAtoH(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyAtoA(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy2D(const CUDA_MEMCPY2D *pCopy)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy3D(const CUDA_MEMCPY3D *pCopy)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyHtoDAsync(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyDtoHAsync(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyDtoDAsync(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyHtoAAsync(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpyAtoHAsync(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD8(CUdeviceptr dstDevice, unsigned char uc, size_t N)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD16(CUdeviceptr dstDevice, unsigned short us, size_t N)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD32(CUdeviceptr dstDevice, unsigned int ui, size_t N)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D8(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D16(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D32(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuArrayCreate(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuArrayGetDescriptor(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuArrayDestroy(CUarray hArray)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuArray3DCreate(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuArray3DGetDescriptor(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuPointerGetAttribute(void *data, CUpointer_attribute attribute, CUdeviceptr ptr)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuStreamCreate(CUstream *phStream, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuStreamQuery(CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuStreamSynchronize(CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuStreamDestroy(CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventCreate(CUevent *phEvent, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventRecord(CUevent hEvent, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventQuery(CUevent hEvent)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventSynchronize(CUevent hEvent)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventDestroy(CUevent hEvent)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuFuncGetAttribute(int *pi, CUfunction_attribute attrib, CUfunction hfunc)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuFuncSetSharedMemConfig(CUfunction hfunc, CUsharedconfig config)
{
	__CUDA_NOT_IMPL__
	return 0;
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
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuParamSetSize(CUfunction hfunc, unsigned int numbytes)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuParamSeti(CUfunction hfunc, int offset, unsigned int value)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuParamSetf(CUfunction hfunc, int offset, float value)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuParamSetv(CUfunction hfunc, int offset, void *ptr, unsigned int numbytes)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuLaunch(CUfunction f)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuLaunchGrid(CUfunction f, int grid_width, int grid_height)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuLaunchGridAsync(CUfunction f, int grid_width, int grid_height, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetAddress(size_t *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetAddress2D(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetAddress(CUdeviceptr *pdptr, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetArray(CUarray *phArray, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetAddressMode(CUaddress_mode *pam, CUtexref hTexRef, int dim)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetFilterMode(CUfilter_mode *pfm, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetFormat(CUarray_format *pFormat, int *pNumChannels, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefGetFlags(unsigned int *pFlags, CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefCreate(CUtexref *pTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuTexRefDestroy(CUtexref hTexRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuSurfRefGetArray(CUarray *phArray, CUsurfref hSurfRef)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuDeviceCanAccessPeer(int *canAccessPeer, CUdevice dev, CUdevice peerDev)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuCtxDisablePeerAccess(CUcontext peerContext)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsUnregisterResource(CUgraphicsResource resource)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsSubResourceGetMappedArray(CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsResourceGetMappedPointer(CUdeviceptr *pDevPtr, size_t *pSize, CUgraphicsResource resource)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsResourceSetMapFlags(CUgraphicsResource resource, unsigned int flags)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsMapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGraphicsUnmapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream)
{
	__CUDA_NOT_IMPL__
	return 0;
}


CUresult cuGetExportTable(const void **ppExportTable, const CUuuid *pExportTableId)
{
	__CUDA_NOT_IMPL__
	return 0;
}

