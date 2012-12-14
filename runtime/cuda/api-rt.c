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
#include "../include/cuda_runtime_api.h"
#include "api.h"
#include "debug.h"
#include "linked-list.h"

#include <stdio.h>
#include <unistd.h>


/* Error messages */
#define __CUDART_NOT_IMPL__  warning("%s: not implemented.\n%s", \
	__FUNCTION__, err_frm_cudart_not_impl);

static char *err_frm_cudart_not_impl =
	"\tMulti2Sim provides partial support for CUDA runtime library.\n"
	"\tTo request the implementation of a certain functionality,\n"
	"\tplease email development@multi2sim.org.\n";

static char *err_frm_cuda_native =
	"\tYou are trying to run natively an application using the Multi2Sim CUDA driver\n"
	"\tlibrary implementation ('libm2s-cuda'). Please run this program on top of\n"
	"\tMulti2Sim.\n";




/*
 * Data Structures and Macros
 */

#define __dv(v)

struct __fatDeviceText {
	int m;
	int v;
	const unsigned long long* d;
	char* f;
};

#define FRM_CUDART_VERSION_MAJOR	1
#define FRM_CUDART_VERSION_MINOR	700

struct frm_cudart_version_t
{
	int major;
	int minor;
};

CUfunction function;

dim3 grid_dim, threadblock_dim;

struct linked_list_t *args;

static int arg_index = 0;




/*
 * Internal Functions
 */

void** __cudaRegisterFatBinary(void *fatCubin)
{
	unsigned long long int **fatCubinHandle;

	cuInit(0);

	cuda_debug(stdout, "CUDA runtime internal function '%s'\n", __FUNCTION__);

	fatCubinHandle = (unsigned long long int **)malloc(sizeof(unsigned long long int *));
	*fatCubinHandle = (unsigned long long int *)(((struct __fatDeviceText *)fatCubin)->d);

	return (void **)fatCubinHandle;
}

void __cudaUnregisterFatBinary(void **fatCubinHandle)
{
	cuda_debug(stdout, "CUDA runtime internal function '%s'\n", __FUNCTION__);

	if (fatCubinHandle != NULL)
		free(fatCubinHandle);
}

void __cudaRegisterVar(void **fatCubinHandle,
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

void __cudaRegisterTexture(void **fatCubinHandle,
	const struct textureReference *hostVar,
	const void **deviceAddress,
	const char *deviceName,
	int dim,
	int norm,
	int ext)
{
	__CUDART_NOT_IMPL__
}

void __cudaRegisterSurface(void **fatCubinHandle,
	const struct surfaceReference *hostVar,
	const void **deviceAddress,
	const char *deviceName,
	int dim,
	int ext)
{
	__CUDART_NOT_IMPL__
}

void __cudaRegisterFunction(void **fatCubinHandle,
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
	char cubin_filename[500];
	CUmodule module;
	int ret;

	cuda_debug(stdout, "CUDA runtime internal function '%s'\n", __FUNCTION__);

	/* Get kernel binary name */
	/* FIXME - The system call should take another argument with the size of
	 * 'cubin_filename'. */
	ret = syscall(FRM_CUDA_SYS_CODE, frm_cuda_call_cudaRegisterFunction, cubin_filename);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call FRM_CUDA_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_frm_cuda_native);

	/* Load module */
	cuda_debug(stdout, "\t(runtime) out: cubin_filename=%s\n", cubin_filename);
	cuModuleLoad(&module, cubin_filename);

	/* Get function */
	cuModuleGetFunction(&function, module, deviceFun);
}




/*
 * CUDA Runtime API
 */

cudaError_t cudaDeviceReset(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceSynchronize(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadExit(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadSynchronize(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaThreadSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetLastError(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaPeekAtLastError(void)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}


const char* cudaGetErrorString(cudaError_t error)
{
	__CUDART_NOT_IMPL__
	return NULL;
}

cudaError_t cudaGetDeviceCount(int *count)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetDeviceProperties(struct cudaDeviceProp *prop, int device)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaChooseDevice(int *device, const struct cudaDeviceProp *prop)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaSetDevice(int device)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetDevice(int *device)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaSetValidDevices(int *device_arr, int len)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaSetDeviceFlags( unsigned int flags )
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaStreamCreate(cudaStream_t *pStream)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaStreamDestroy(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaStreamSynchronize(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaStreamQuery(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventCreate(cudaEvent_t *event)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventCreateWithFlags(cudaEvent_t *event, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventRecord(cudaEvent_t event, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventQuery(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventSynchronize(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventDestroy(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaEventElapsedTime(float *ms, cudaEvent_t start, cudaEvent_t end)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaConfigureCall(dim3 gridDim, dim3 blockDim, size_t sharedMem __dv(0), cudaStream_t stream __dv(0))
{
	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(runtime) in: gridDim=%u %u %u\n", 
		gridDim.x, gridDim.y, gridDim.z);
	cuda_debug(stdout, "\t(runtime) in: blockDim=%u %u %u\n", 
		blockDim.x, blockDim.y, blockDim.z);

	grid_dim.x = gridDim.x;
	grid_dim.y = gridDim.y;
	grid_dim.z = gridDim.z;
	threadblock_dim.x = blockDim.x;
	threadblock_dim.y = blockDim.y;
	threadblock_dim.z = blockDim.z;

	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaSetupArgument(const void *arg, size_t size, size_t offset)
{
	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(runtime) in: arg=%p\n", arg);
	cuda_debug(stdout, "\t(runtime) in: size=%d\n", size);
	cuda_debug(stdout, "\t(runtime) in: offset=%d\n", offset);

	if (arg_index == 0)
		args = linked_list_create();

	linked_list_add(args, (void *)arg);
	++arg_index;

	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaFuncSetCacheConfig(const void *func, enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaLaunch(const void *entry)
{
	CUdeviceptr **kernel_args;
	int i;

	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(runtime) in: entry=%p\n", entry);

	/* Copy kernel arguments */
	kernel_args = malloc(linked_list_count(args) * sizeof(CUdeviceptr *));
	linked_list_head(args);
	for (i = 0; i < linked_list_count(args); ++i)
	{
		kernel_args[i] = linked_list_get(args);
		linked_list_next(args);
	}

	/* Launch kernel */
	cuLaunchKernel(function, grid_dim.x, grid_dim.y, grid_dim.z, 
		threadblock_dim.x, threadblock_dim.y, threadblock_dim.z, 
		0, NULL, (void **)kernel_args, NULL);
	free(kernel_args);

	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaFuncGetAttributes(struct cudaFuncAttributes *attr, const void *func)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaSetDoubleForDevice(double *d)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaSetDoubleForHost(double *d)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMalloc(void **devPtr, size_t size)
{
	CUdeviceptr dptr;

	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(runtime) in: size=%d\n", size);

	cuMemAlloc(&dptr, size);
	*(CUdeviceptr *)devPtr = dptr;

	cuda_debug(stdout, "\t(runtime) out: devPtr=0x%08x\n", *(CUdeviceptr *)devPtr);
	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaMallocHost(void **ptr, size_t size)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMallocArray(struct cudaArray **array, const struct cudaChannelFormatDesc *desc, size_t width, size_t height __dv(0), unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaFree(void *devPtr)
{
	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug(stdout, "\t(runtime) in: devPtr=%p\n", devPtr);

	cuMemFree((CUdeviceptr)devPtr);

	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaFreeHost(void *ptr)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaFreeArray(struct cudaArray *array)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaHostAlloc(void **pHost, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaHostRegister(void *ptr, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaHostUnregister(void *ptr)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaHostGetFlags(unsigned int *pFlags, void *pHost)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMalloc3D(struct cudaPitchedPtr* pitchedDevPtr, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMalloc3DArray(struct cudaArray** array, const struct cudaChannelFormatDesc* desc, struct cudaExtent extent, unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy3D(const struct cudaMemcpy3DParms *p)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DPeer(const struct cudaMemcpy3DPeerParms *p)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DAsync(const struct cudaMemcpy3DParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DPeerAsync(const struct cudaMemcpy3DPeerParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemGetInfo(size_t *free, size_t *total)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	cuda_debug(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);

	/* FIXME: cudaMemcpyHostToHost, cudaMemcpyDeviceToDevice */
	if (kind == cudaMemcpyHostToDevice)
		cuMemcpyHtoD((CUdeviceptr)dst, src, count);
	else if (kind == cudaMemcpyDeviceToHost)
		cuMemcpyDtoH(dst, (CUdeviceptr)src, count);

	cuda_debug(stdout, "\t(runtime) out: return=%d\n", cudaSuccess);

	return cudaSuccess;
}

cudaError_t cudaMemcpyPeer(void *dst, int dstDevice, const void *src, int srcDevice, size_t count)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromArray(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t count, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DFromArray(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t width, size_t height, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyToSymbol(const void *symbol, const void *src, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyHostToDevice))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromSymbol(void *dst, const void *symbol, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToHost))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyAsync(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyPeerAsync(void *dst, int dstDevice, const void *src, int srcDevice, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromArrayAsync(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DFromArrayAsync(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyToSymbolAsync(const void *symbol, const void *src, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromSymbolAsync(void *dst, const void *symbol, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemset(void *devPtr, int value, size_t count)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemset2D(void *devPtr, size_t pitch, int value, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemset3D(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemsetAsync(void *devPtr, int value, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width, size_t height, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaMemset3DAsync(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetSymbolAddress(void **devPtr, const void *symbol)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetSymbolSize(size_t *size, const void *symbol)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaPointerGetAttributes(struct cudaPointerAttributes *attributes, const void *ptr)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceCanAccessPeer(int *canAccessPeer, int device, int peerDevice)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceEnablePeerAccess(int peerDevice, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDeviceDisablePeerAccess(int peerDevice)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsUnregisterResource(cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsResourceSetMapFlags(cudaGraphicsResource_t resource, unsigned int flags)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsMapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsUnmapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsResourceGetMappedPointer(void **devPtr, size_t *size, cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGraphicsSubResourceGetMappedArray(struct cudaArray **array, cudaGraphicsResource_t resource, unsigned int arrayIndex, unsigned int mipLevel)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetChannelDesc(struct cudaChannelFormatDesc *desc, const struct cudaArray *array)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}


struct cudaChannelFormatDesc cudaCreateChannelDesc(int x, int y, int z, int w, enum cudaChannelFormatKind f)
{
	__CUDART_NOT_IMPL__
	struct cudaChannelFormatDesc cfd;
	return cfd;
}

cudaError_t cudaBindTexture(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t size __dv(UINT_MAX))
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaBindTexture2D(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t width, size_t height, size_t pitch)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaBindTextureToArray(const struct textureReference *texref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaUnbindTexture(const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetTextureAlignmentOffset(size_t *offset, const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetTextureReference(const struct textureReference **texref, const void *symbol)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaBindSurfaceToArray(const struct surfaceReference *surfref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetSurfaceReference(const struct surfaceReference **surfref, const void *symbol)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaDriverGetVersion(int *driverVersion)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaRuntimeGetVersion(int *runtimeVersion)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

cudaError_t cudaGetExportTable(const void **ppExportTable, const cudaUUID_t *pExportTableId)
{
	__CUDART_NOT_IMPL__
	return cudaSuccess;
}

