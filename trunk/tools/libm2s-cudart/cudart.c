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

#include <m2s-cudart.h>
#include <debug.h>
#include <list.h>
#include <misc.h>



/*
 * Error Messages
 */

#define __CUDART_NOT_IMPL__  warning("%s: not implemented.\n%s", \
	__FUNCTION__, err_cudart_not_impl);


static char *err_cudart_not_impl =
	"\tMulti2Sim provides partial support for CUDA runtime library.\n"
	"\tTo request the implementation of a certain functionality,\n"
	"\tplease email development@multi2sim.org.\n";

static char *err_cudart_version =
	"\tYour guest application is using a version of the CUDA runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tCUDA runtime library ('libm2s-cudart').\n";



/* Multi2Sim CUDA runtime required */
#define CUDART_VERSION_MAJOR	0
#define CUDART_VERSION_MINOR	0

struct cudart_version_t
{
	int major;
	int minor;
};



/* CUDA runtime data structures */
#define __dv(v)



/* CUDA runtime API */
void** CUDARTAPI __cudaRegisterFatBinary(void *fatCubin)
{
	__CUDART_NOT_IMPL__
	return NULL;
}


void CUDARTAPI __cudaUnregisterFatBinary(void **fatCubinHandle)
{
	__CUDART_NOT_IMPL__
}


void CUDARTAPI __cudaRegisterVar(void **fatCubinHandle,
	char *hostVar,
	char *deviceAddress,
	const char *deviceName,
	int ext,
	int size,
	int constant,
	int global)
{
	__CUDART_NOT_IMPL__
}


void CUDARTAPI __cudaRegisterTexture(void **fatCubinHandle,
	const struct textureReference *hostVar,
	const void **deviceAddress,
	const char *deviceName,
	int dim,
	int norm,
	int ext)
{
	__CUDART_NOT_IMPL__
}


void CUDARTAPI __cudaRegisterSurface(void **fatCubinHandle,
	const struct surfaceReference *hostVar,
	const void **deviceAddress,
	const char *deviceName,
	int dim,
	int ext)
{
	__CUDART_NOT_IMPL__
}


void CUDARTAPI __cudaRegisterFunction(void **fatCubinHandle,
	const char *hostFun,
	char *deviceFun,
	const char *deviceName,
	int thread_limit,
	uint3 *tid,
	uint3 *bid,
	dim3 *bDim,
	dim3 *gDim,
	int *wSize)
{
	__CUDART_NOT_IMPL__
}


cudaError_t CUDARTAPI cudaDeviceReset(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceSynchronize(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadExit(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadSynchronize(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaThreadSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetLastError(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaPeekAtLastError(void)
{
	__CUDART_NOT_IMPL__
	return 0;
}


const char* CUDARTAPI cudaGetErrorString(cudaError_t error)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetDeviceCount(int *count)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetDeviceProperties(struct cudaDeviceProp *prop, int device)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaChooseDevice(int *device, const struct cudaDeviceProp *prop)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetDevice(int device)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetDevice(int *device)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetValidDevices(int *device_arr, int len)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetDeviceFlags( unsigned int flags )
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaStreamCreate(cudaStream_t *pStream)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaStreamDestroy(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaStreamSynchronize(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaStreamQuery(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventCreate(cudaEvent_t *event)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventCreateWithFlags(cudaEvent_t *event, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventRecord(cudaEvent_t event, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventQuery(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventSynchronize(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventDestroy(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaEventElapsedTime(float *ms, cudaEvent_t start, cudaEvent_t end)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaConfigureCall(dim3 gridDim, dim3 blockDim, size_t sharedMem __dv(0), cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetupArgument(const void *arg, size_t size, size_t offset)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaFuncSetCacheConfig(const char *func, enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaLaunch(const char *entry)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaFuncGetAttributes(struct cudaFuncAttributes *attr, const char *func)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetDoubleForDevice(double *d)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaSetDoubleForHost(double *d)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMalloc(void **devPtr, size_t size)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMallocHost(void **ptr, size_t size)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMallocArray(struct cudaArray **array, const struct cudaChannelFormatDesc *desc, size_t width, size_t height __dv(0), unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaFree(void *devPtr)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaFreeHost(void *ptr)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaFreeArray(struct cudaArray *array)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaHostAlloc(void **pHost, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaHostRegister(void *ptr, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaHostUnregister(void *ptr)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaHostGetFlags(unsigned int *pFlags, void *pHost)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMalloc3D(struct cudaPitchedPtr* pitchedDevPtr, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMalloc3DArray(struct cudaArray** array, const struct cudaChannelFormatDesc* desc, struct cudaExtent extent, unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy3D(const struct cudaMemcpy3DParms *p)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy3DPeer(const struct cudaMemcpy3DPeerParms *p)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy3DAsync(const struct cudaMemcpy3DParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy3DPeerAsync(const struct cudaMemcpy3DPeerParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemGetInfo(size_t *free, size_t *total)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyPeer(void *dst, int dstDevice, const void *src, int srcDevice, size_t count)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyFromArray(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t count, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DFromArray(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t width, size_t height, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyToSymbol(const char *symbol, const void *src, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyHostToDevice))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyFromSymbol(void *dst, const char *symbol, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToHost))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyAsync(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyPeerAsync(void *dst, int dstDevice, const void *src, int srcDevice, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyFromArrayAsync(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpy2DFromArrayAsync(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyToSymbolAsync(const char *symbol, const void *src, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemcpyFromSymbolAsync(void *dst, const char *symbol, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemset(void *devPtr, int value, size_t count)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemset2D(void *devPtr, size_t pitch, int value, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemset3D(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemsetAsync(void *devPtr, int value, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width, size_t height, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaMemset3DAsync(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetSymbolAddress(void **devPtr, const char *symbol)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetSymbolSize(size_t *size, const char *symbol)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaPointerGetAttributes(struct cudaPointerAttributes *attributes, void *ptr)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceCanAccessPeer(int *canAccessPeer, int device, int peerDevice)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceEnablePeerAccess(int peerDevice, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDeviceDisablePeerAccess(int peerDevice)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsUnregisterResource(cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsResourceSetMapFlags(cudaGraphicsResource_t resource, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsMapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsUnmapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsResourceGetMappedPointer(void **devPtr, size_t *size, cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGraphicsSubResourceGetMappedArray(struct cudaArray **array, cudaGraphicsResource_t resource, unsigned int arrayIndex, unsigned int mipLevel)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetChannelDesc(struct cudaChannelFormatDesc *desc, const struct cudaArray *array)
{
	__CUDART_NOT_IMPL__
	return 0;
}


struct cudaChannelFormatDesc CUDARTAPI cudaCreateChannelDesc(int x, int y, int z, int w, enum cudaChannelFormatKind f)
{
	__CUDART_NOT_IMPL__
	return;
}


cudaError_t CUDARTAPI cudaBindTexture(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t size __dv(UINT_MAX))
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaBindTexture2D(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t width, size_t height, size_t pitch)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaBindTextureToArray(const struct textureReference *texref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaUnbindTexture(const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetTextureAlignmentOffset(size_t *offset, const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetTextureReference(const struct textureReference **texref, const char *symbol)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaBindSurfaceToArray(const struct surfaceReference *surfref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetSurfaceReference(const struct surfaceReference **surfref, const char *symbol)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaDriverGetVersion(int *driverVersion)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaRuntimeGetVersion(int *runtimeVersion)
{
	__CUDART_NOT_IMPL__
	return 0;
}


cudaError_t CUDARTAPI cudaGetExportTable(const void **ppExportTable, const cudaUUID_t *pExportTableId)
{
	__CUDART_NOT_IMPL__
	return 0;
}

