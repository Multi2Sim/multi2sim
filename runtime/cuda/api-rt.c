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
#include <elf.h>
#include <stdio.h>
#include <string.h>

#include "../include/cuda.h"
#include "../include/cuda_runtime_api.h"
#include "api.h"
#include "debug.h"
#include "device.h"
#include "elf-format.h"
#include "function-arg.h"
#include "function.h"
#include "list.h"
#include "mhandle.h"


/*
 * Global Variables
 */

CUfunction function;
cudaError_t cuda_rt_last_error;

/* Error messages */
char *cuda_rt_err_not_impl = 
"\tMulti2Sim provides partial support for CUDA runtime library.\n"
"\tTo request the implementation of a certain functionality,\n"
"\tplease email development@multi2sim.org.\n";

char *cuda_rt_err_native =
"\tYou are trying to run natively an application using the Multi2Sim\n"
"\tCUDA runtime/driver library implementation ('libm2s-cuda'). Please\n"
"\trun this program on top of Multi2Sim.\n";

char *cuda_rt_err_param_note =
"\tThis error message is generated by the Multi2Sim CUDA runtime\n"
"\tlibrary linked with your CUDA host program.\n"
"\tWhile a complete CUDA implementation would return an error code\n"
"\tto your application, the Multi2Sim CUDA runtime just makes your\n"
"\tprogram fail with an error message.\n";




/*
 * Private Functions
 */

static unsigned char get_uchar(const unsigned long long int *dev_func_bin_sec,
		int index)
{
	return (unsigned char)
		((dev_func_bin_sec[index / 8] >> (index % 8 * 8)) & 0xff);
}

static unsigned int get_uint(const unsigned long long int *dev_func_bin_sec, 
		int start_index)
{
	return (unsigned int)
		((unsigned int)get_uchar(dev_func_bin_sec, 
			start_index) |
		 ((unsigned int)get_uchar(dev_func_bin_sec, 
			 start_index + 1)) << 8 |
		 ((unsigned int)get_uchar(dev_func_bin_sec, 
			 start_index + 2)) << 16 |
		 ((unsigned int)get_uchar(dev_func_bin_sec, 
			 start_index + 3)) << 24);
}




/*
 * CUDA Runtime Internal Functions
 */

void **__cudaRegisterFatBinary(void *fatCubin)
{
	void **fatCubinHandle;

	cuInit(0);

	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n",
			__FUNCTION__);

	fatCubinHandle = xcalloc(1, sizeof(void *));
	*fatCubinHandle = fatCubin;

	cuda_debug_print(stdout, "\treturn\n");

	return fatCubinHandle;
}

void __cudaUnregisterFatBinary(void **fatCubinHandle)
{
	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n",
			__FUNCTION__);

	if (fatCubinHandle != NULL)
		free(fatCubinHandle);

	cuda_debug_print(stdout, "\treturn\n");
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
	__CUDART_NOT_IMPL__;
}

void __cudaRegisterTexture(void **fatCubinHandle,
		const struct textureReference *hostVar,
		const void **deviceAddress,
		const char *deviceName,
		int dim,
		int norm,
		int ext)
{
	__CUDART_NOT_IMPL__;
}

void __cudaRegisterSurface(void **fatCubinHandle,
		const struct surfaceReference *hostVar,
		const void **deviceAddress,
		const char *deviceName,
		int dim,
		int ext)
{
	__CUDART_NOT_IMPL__;
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
	char *cubin_path;
	char *binary_filename;
	char deviceFun_arr[1024];

	const unsigned long long int *dev_func_bin_sec;
	int dev_func_bin_sec_size;
	int elf_head;
	struct elf_file_t *dev_func_bin;
	FILE *dev_func_bin_f;
	char abi_version;

	CUmodule module;

	void *ptr;
	int size;
	int i;

	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n",
			__FUNCTION__);

	/* User can set an environment variable 'M2S_OPENCL_BINARY' to make the
	 * runtime load that specific pre-compiled binary. */
	cubin_path = getenv("M2S_CUDA_BINARY");
	if (cubin_path && !strchr(cubin_path, '/'))
	{
		fatal("\tPlease set M2S_CUDA_BINARY to the path of the cubin\n"
				"\tfile");
	}

	/* Get device function identifier. If M2S_CUDA_BINARY is set, we assume
	 * the binary filename (excluding .cubin) is the device function
	 * identifier */
	if (cubin_path)
	{
		binary_filename = strrchr(cubin_path, '/') + 1;
		assert(!strncmp(binary_filename + strlen(binary_filename) - 6,
					".cubin", 6));
		assert(strlen(binary_filename) < sizeof deviceFun_arr);
		strncpy(deviceFun_arr, binary_filename, 
				strlen(binary_filename) - 6);
		deviceFun_arr[strlen(binary_filename) - 6] = '\0';
		deviceFun = deviceFun_arr;
	}

	/* Get device function binary */
	if (!cubin_path)
	{
		/* Get the section containing device function binary */
		dev_func_bin_sec = 
			(*(struct {int m; int v; 
			   const unsigned long long int *d; char *f;} **)
			 fatCubinHandle)->d;
		dev_func_bin_sec_size = 
			*(((unsigned long long int *)dev_func_bin_sec) + 1) +
			16;

		/* Look for ELF head */
		elf_head = -1;
		for (i = 2; i < dev_func_bin_sec_size; ++i)
		{
			if (get_uint(dev_func_bin_sec, i) == 0x464c457f)
			{
				elf_head = i;
				break;
			}
		}
		assert(elf_head != -1);

		/* Get device function binary 
		 * Notice that this binary may be larger than the actual binary
		 * since we cannot determine the end of the binary. */
		ptr = (void *)(dev_func_bin_sec + 
				elf_head / sizeof(unsigned long long int)); 
		size = 16 + dev_func_bin_sec_size - elf_head;
		dev_func_bin = elf_file_create_from_buffer(ptr, size, NULL);

		/* Save device function binary in a tmp file for later use in
		 * CUDA driver */
		cubin_path = tmpnam(NULL);
		dev_func_bin_f = fopen(cubin_path, "wb");
		elf_buffer_dump(&(dev_func_bin->buffer), dev_func_bin_f);
		fclose(dev_func_bin_f);
	}
	else
		dev_func_bin = elf_file_create_from_path(cubin_path);

	/* Check for Fermi binary. Support for Fermi only for now. */
	elf_buffer_seek(&(dev_func_bin->buffer), 8);
	elf_buffer_read(&(dev_func_bin->buffer), &abi_version, 1);
	if (abi_version < 4 || abi_version > 6)
		fatal("\tThe cubin file has a unrecognized ABI version (0x%x).\n"
				"\tMulti2Sim CUDA library is currently\n"
				"\tcompatible with Fermi binary only.",
				abi_version);

	/* Load module */
	cuModuleLoad(&module, cubin_path);

	/* Get device function */
	cuModuleGetFunction(&function, module, deviceFun);

	/* Free */
	elf_file_free(dev_func_bin);

	cuda_debug_print(stdout, "\treturn\n");
}




/*
 * CUDA Runtime API
 */

cudaError_t cudaDeviceReset(void)
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);

	/* This function is not implemented for now since resources are freed in
	 * cuda_done() */

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaDeviceSynchronize(void)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadExit(void)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadSynchronize(void)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadSetLimit(enum cudaLimit limit, size_t value)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadGetLimit(size_t *pValue, enum cudaLimit limit)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadGetCacheConfig(enum cudaFuncCache *pCacheConfig)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaThreadSetCacheConfig(enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetLastError(void)
{
	cudaError_t cuda_rt_last_error_ret;

	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);

	cuda_rt_last_error_ret = cuda_rt_last_error;

	/* Reset */
	cuda_rt_last_error = cudaSuccess;

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n",
			cuda_rt_last_error_ret);

	return cuda_rt_last_error_ret;
}

cudaError_t cudaPeekAtLastError(void)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}


const char* cudaGetErrorString(cudaError_t error)
{
	__CUDART_NOT_IMPL__;
	return NULL;
}

cudaError_t cudaGetDeviceCount(int *count)
{
	/* 0 for kepler, 1 for fermi */
	*count = 2;	

	return cudaSuccess;
}

cudaError_t cudaGetDeviceProperties(struct cudaDeviceProp *prop_ptr, int device)
{
	/* Check for valid properties pointer */
	if (!prop_ptr)
		fatal("%s: invalid value for 'prop_ptr'.\n%s",
				__FUNCTION__, cuda_rt_err_param_note);

	/* Check for valid device. For now, we just check that the device is
	 * 0 (default), but later we will check among an array of possible valid
	 * devices. */
	if (device != 1)  
		fatal("%s: invalid device (%d).\n%s", __FUNCTION__,
				device, cuda_rt_err_param_note);

	/* Initialize device properties */	
	strcpy (prop_ptr->name, "Device1-fermi");
	prop_ptr->totalGlobalMem = 1073741824; // 1G
	prop_ptr->sharedMemPerBlock = 49152; // 48M
	prop_ptr->regsPerBlock = 32768;
	prop_ptr->warpSize = 32;
	prop_ptr->memPitch = 2147483648u; // 2G
	prop_ptr->maxThreadsPerBlock = 512;
	prop_ptr->maxThreadsDim[1] = 512;
	prop_ptr->maxThreadsDim[2] = 512;
	prop_ptr->maxThreadsDim[3] = 64;
	prop_ptr->maxGridSize[1] = 65535; //64M
	prop_ptr->maxGridSize[2] = 65535; //64M
	prop_ptr->maxGridSize[3] = 65535; // 64M
	prop_ptr->clockRate = 500000;
	prop_ptr->totalConstMem = 65535; //64M
	prop_ptr->major = 1;
	prop_ptr->minor = 3;
	prop_ptr->textureAlignment = 512;
	prop_ptr->texturePitchAlignment = 32;
	prop_ptr->deviceOverlap = 1;
	prop_ptr->multiProcessorCount = 480;
	prop_ptr->kernelExecTimeoutEnabled = 0;
	prop_ptr->integrated = 1;
	prop_ptr->canMapHostMemory = 1;
	prop_ptr->computeMode = cudaComputeModeDefault;
	prop_ptr->maxTexture1D = 65536;
	//prop_ptr->maxTexture1DMipmap;
	//prop_ptr->maxTexture1DLinear;
	prop_ptr->maxTexture2D[1] = 65536; //64M
	prop_ptr->maxTexture2D[2] = 65536; //64M
	//prop_ptr->maxTexture2DMipmap[2];
	//prop_ptr->maxTexture2DLinear[3];
	//prop_ptr->maxTexture2DGather[2];
	prop_ptr->maxTexture3D[1] = 2048; //2M
	prop_ptr->maxTexture3D[2] = 2048; //2M
	prop_ptr->maxTexture3D[3] = 2048; //2M
	//prop_ptr->maxTextureCubemap;
	//prop_ptr->maxTexture1DLayered[2];
	//prop_ptr->maxTexture2DLayered[3];
	//prop_ptr->maxTextureCubemapLayered[2];
	//prop_ptr->maxSurface1D;
	//prop_ptr->maxSurface2D[2];
	//prop_ptr->maxSurface3D[3];
	//prop_ptr->maxSurface1DLayered[2];
	//prop_ptr->maxSurface2DLayered[3];
	//prop_ptr->maxSurfaceCubemap;
	//prop_ptr->maxSurfaceCubemapLayered[2];
	//prop_ptr->surfaceAlignment;
	prop_ptr->concurrentKernels = 1;
	prop_ptr->ECCEnabled = 1;
	//prop_ptr->pciBusID;
	//prop_ptr->pciDeviceID;
	//prop_ptr->pciDomainID;
	prop_ptr->tccDriver = 1;
	prop_ptr->asyncEngineCount = 2;
	prop_ptr->unifiedAddressing = 1;
	prop_ptr->memoryClockRate = 950000;
	prop_ptr->memoryBusWidth = 256;
	prop_ptr->l2CacheSize = 256;
	prop_ptr->maxThreadsPerMultiProcessor = 8;

	/* Populate fields */
	//snprintf(prop_ptr->name, sizeof prop_ptr->name, "Multi2Sim CUDA Device");

	/* Return success */
	return cudaSuccess;
}

cudaError_t cudaChooseDevice(int *device, const struct cudaDeviceProp *prop)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaSetDevice(int device)
{
	//Sets device as the current device for the calling host thread
	return cudaSuccess;
}

cudaError_t cudaGetDevice(int *device_ptr)
{
	/* Check valid device pointer */
	if (!device_ptr)
		fatal("%s: invalid device pointer.\n%s", __FUNCTION__,
				cuda_rt_err_param_note);

	/* We allow temporarily for only the Fermi device. We assigned a
	 * hardcoded device identifier equal to 1. This function will be
	 * extended later with support to Kepler. */
	*device_ptr = 1;

	/* Return success */
	return cudaSuccess;
}

cudaError_t cudaSetValidDevices(int *device_arr, int len)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaSetDeviceFlags( unsigned int flags )
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaStreamCreate(cudaStream_t *pStream)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaStreamDestroy(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaStreamSynchronize(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaStreamQuery(cudaStream_t stream)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventCreate(cudaEvent_t *event)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventCreateWithFlags(cudaEvent_t *event, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventRecord(cudaEvent_t event, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventQuery(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventSynchronize(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventDestroy(cudaEvent_t event)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaEventElapsedTime(float *ms, cudaEvent_t start, cudaEvent_t end)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaConfigureCall(dim3 gridDim, dim3 blockDim, size_t sharedMem __dv(0), cudaStream_t stream __dv(0))
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: gridDim = %u %u %u\n", 
			gridDim.x, gridDim.y, gridDim.z);
	cuda_debug_print(stdout, "\t(runtime) in: blockDim = %u %u %u\n", 
			blockDim.x, blockDim.y, blockDim.z);

	function->global_sizes[0] = gridDim.x;
	function->global_sizes[1] = gridDim.y;
	function->global_sizes[2] = gridDim.z;
	function->local_sizes[0] = blockDim.x;
	function->local_sizes[1] = blockDim.y;
	function->local_sizes[2] = blockDim.z;

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaSetupArgument(const void *arg, size_t size, size_t offset)
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: arg = %p\n", arg);
	cuda_debug_print(stdout, "\t(runtime) in: size = %d\n", size);
	cuda_debug_print(stdout, "\t(runtime) in: offset = %d\n", offset);

	cuda_function_arg_create(function, *(CUdeviceptr *)arg, size);

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaFuncSetCacheConfig(const void *func, enum cudaFuncCache cacheConfig)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaLaunch(const void *entry)
{
	int i;

	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: entry = %p\n", entry);

	/* Setup arguments */
	function->arg_array = (CUdeviceptr **)xcalloc(list_count(function->arg_list), sizeof(CUdeviceptr *));
	for (i = 0; i < list_count(function->arg_list); ++i)
		(function->arg_array)[i] = 
			&(((struct cuda_function_arg_t *)list_get(function->arg_list, i))->value);

	/* Launch kernel */
	cuLaunchKernel(function, function->global_sizes[0],
			function->global_sizes[1], function->global_sizes[2], 
			function->local_sizes[0], function->local_sizes[1], 
			function->local_sizes[2], 
			0, NULL, (void **)function->arg_array, NULL);

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaFuncGetAttributes(struct cudaFuncAttributes *attr, const void *func)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaSetDoubleForDevice(double *d)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaSetDoubleForHost(double *d)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMalloc(void **devPtr, size_t size)
{
	CUdeviceptr dptr;

	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: size = %d\n", size);

	cuMemAlloc(&dptr, size);
	*(CUdeviceptr *)devPtr = dptr;

	cuda_debug_print(stdout, "\t(runtime) out: devPtr = 0x%08x\n", *(CUdeviceptr *)devPtr);
	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaMallocHost(void **ptr, size_t size)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMallocPitch(void **devPtr, size_t *pitch, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMallocArray(struct cudaArray **array, const struct cudaChannelFormatDesc *desc, size_t width, size_t height __dv(0), unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaFree(void *devPtr)
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: devPtr = %p\n", devPtr);

	cuMemFree((CUdeviceptr)devPtr);

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaFreeHost(void *ptr)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaFreeArray(struct cudaArray *array)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaHostAlloc(void **pHost, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaHostRegister(void *ptr, size_t size, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaHostUnregister(void *ptr)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaHostGetDevicePointer(void **pDevice, void *pHost, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaHostGetFlags(unsigned int *pFlags, void *pHost)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMalloc3D(struct cudaPitchedPtr* pitchedDevPtr, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMalloc3DArray(struct cudaArray** array, const struct cudaChannelFormatDesc* desc, struct cudaExtent extent, unsigned int flags __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy3D(const struct cudaMemcpy3DParms *p)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DPeer(const struct cudaMemcpy3DPeerParms *p)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DAsync(const struct cudaMemcpy3DParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy3DPeerAsync(const struct cudaMemcpy3DPeerParms *p, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemGetInfo(size_t *free, size_t *total)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: dst = %p\n", dst);
	cuda_debug_print(stdout, "\t(runtime) in: src = %p\n", src);
	cuda_debug_print(stdout, "\t(runtime) in: count = %d\n", count);
	cuda_debug_print(stdout, "\t(runtime) in: kind = %d\n", kind);

	if (kind == cudaMemcpyHostToDevice)
		cuMemcpyHtoD((CUdeviceptr)dst, src, count);
	else if (kind == cudaMemcpyDeviceToHost)
		cuMemcpyDtoH(dst, (CUdeviceptr)src, count);
	/* FIXME: implement cudaMemcpyHostToHost, cudaMemcpyDeviceToDevice */

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

	return cudaSuccess;
}

cudaError_t cudaMemcpyPeer(void *dst, int dstDevice, const void *src, int srcDevice, size_t count)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromArray(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t count, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DToArray(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DFromArray(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DArrayToArray(struct cudaArray *dst, size_t wOffsetDst, size_t hOffsetDst, const struct cudaArray *src, size_t wOffsetSrc, size_t hOffsetSrc, size_t width, size_t height, enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToDevice))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyToSymbol(const void *symbol, const void *src, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyHostToDevice))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromSymbol(void *dst, const void *symbol, size_t count, size_t offset __dv(0), enum cudaMemcpyKind kind __dv(cudaMemcpyDeviceToHost))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyAsync(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyPeerAsync(void *dst, int dstDevice, const void *src, int srcDevice, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromArrayAsync(void *dst, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DToArrayAsync(struct cudaArray *dst, size_t wOffset, size_t hOffset, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpy2DFromArrayAsync(void *dst, size_t dpitch, const struct cudaArray *src, size_t wOffset, size_t hOffset, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyToSymbolAsync(const void *symbol, const void *src, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemcpyFromSymbolAsync(void *dst, const void *symbol, size_t count, size_t offset, enum cudaMemcpyKind kind, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemset(void *devPtr, int value, size_t count)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemset2D(void *devPtr, size_t pitch, int value, size_t width, size_t height)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemset3D(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemsetAsync(void *devPtr, int value, size_t count, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width, size_t height, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaMemset3DAsync(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetSymbolAddress(void **devPtr, const void *symbol)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetSymbolSize(size_t *size, const void *symbol)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaPointerGetAttributes(struct cudaPointerAttributes *attributes, const void *ptr)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceCanAccessPeer(int *canAccessPeer, int device, int peerDevice)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceEnablePeerAccess(int peerDevice, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDeviceDisablePeerAccess(int peerDevice)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsUnregisterResource(cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsResourceSetMapFlags(cudaGraphicsResource_t resource, unsigned int flags)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsMapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsUnmapResources(int count, cudaGraphicsResource_t *resources, cudaStream_t stream __dv(0))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsResourceGetMappedPointer(void **devPtr, size_t *size, cudaGraphicsResource_t resource)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGraphicsSubResourceGetMappedArray(struct cudaArray **array, cudaGraphicsResource_t resource, unsigned int arrayIndex, unsigned int mipLevel)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetChannelDesc(struct cudaChannelFormatDesc *desc, const struct cudaArray *array)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}


struct cudaChannelFormatDesc cudaCreateChannelDesc(int x, int y, int z, int w, enum cudaChannelFormatKind f)
{
	__CUDART_NOT_IMPL__;
	struct cudaChannelFormatDesc cfd;
	return cfd;
}

cudaError_t cudaBindTexture(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t size __dv(UINT_MAX))
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaBindTexture2D(size_t *offset, const struct textureReference *texref, const void *devPtr, const struct cudaChannelFormatDesc *desc, size_t width, size_t height, size_t pitch)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaBindTextureToArray(const struct textureReference *texref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaUnbindTexture(const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetTextureAlignmentOffset(size_t *offset, const struct textureReference *texref)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetTextureReference(const struct textureReference **texref, const void *symbol)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaBindSurfaceToArray(const struct surfaceReference *surfref, const struct cudaArray *array, const struct cudaChannelFormatDesc *desc)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetSurfaceReference(const struct surfaceReference **surfref, const void *symbol)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaDriverGetVersion(int *driverVersion)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaRuntimeGetVersion(int *runtimeVersion)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

cudaError_t cudaGetExportTable(const void **ppExportTable, const cudaUUID_t *pExportTableId)
{
	__CUDART_NOT_IMPL__;
	return cudaSuccess;
}

