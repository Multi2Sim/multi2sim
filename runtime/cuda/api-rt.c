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

#include <elf.h>
#include <stdio.h>
#include <string.h>

#include "../include/cuda.h"
#include "../include/cuda_runtime_api.h"
#include "api.h"
#include "debug.h"
#include "device.h"
#include "function-arg.h"
#include "function.h"
#include "list.h"
#include "mhandle.h"


/*
 * Global Variables
 */

CUmodule module;
CUfunction function;
unsigned long long int *inst_buffer;
unsigned int inst_buffer_size;
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




/*
 * Private Functions
 */

unsigned char get_uchar(const unsigned long long int *rodata, int index)
{
	return (unsigned char)((rodata[index / 8] >> (index % 8 * 8)) & 0xff);
}

unsigned short get_ushort(const unsigned long long int *rodata, int start_index)
{
	return (unsigned short)((unsigned short)get_uchar(rodata, start_index) |
			((unsigned short)get_uchar(rodata, start_index + 1)) << 8);
}

unsigned int get_uint(const unsigned long long int *rodata, int start_index)
{
	return (unsigned int)((unsigned int)get_uchar(rodata, start_index) |
			((unsigned int)get_uchar(rodata, start_index + 1)) << 8 |
			((unsigned int)get_uchar(rodata, start_index + 2)) << 16 |
			((unsigned int)get_uchar(rodata, start_index + 3)) << 24);
}

unsigned long long int get_ulonglong(const unsigned long long int *rodata, int start_index)
{
	return (unsigned long long int)
		(((unsigned long long int)get_uint(rodata, start_index)) << 32 |
		((unsigned long long int)get_uint(rodata, start_index + 4)));
}

int get_str_len(const unsigned long long int *rodata, int start_index)
{
	int i;

	for (i = start_index; get_uchar(rodata, i) != '\0'; ++i)
		;

	return i;
}

void get_str(unsigned char *s, const unsigned long long int *rodata, int start_index, int end_index)
{
	int i;

	for (i = start_index; i < end_index; ++i)
		s[i - start_index] = get_uchar(rodata, i);
	s[i - start_index] = '\0';
}




/*
 * CUDA Runtime Internal Functions
 */

void **__cudaRegisterFatBinary(void *fatCubin)
{
	struct __fatDeviceText **fatCubinHandle;

	cuInit(0);

	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n", __FUNCTION__);

	fatCubinHandle = (struct __fatDeviceText **)xcalloc(1, sizeof(struct __fatDeviceText *));
	*fatCubinHandle = fatCubin;

	cuda_debug_print(stdout, "\treturn\n");

	return (void **)fatCubinHandle;
}

void __cudaUnregisterFatBinary(void **fatCubinHandle)
{
	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n", __FUNCTION__);

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
	const unsigned long long int *rodata;

	int elf_head;
	int str_tab_head;
	Elf32_Ehdr elf_header;
	Elf32_Shdr *sections;
	unsigned char **section_names;
	int section_name_len;
	char text_section_name[1024];
	unsigned short int text_section_index = 0;

	int i;

	cuda_debug_print(stdout, "CUDA runtime internal function '%s'\n",
			__FUNCTION__);

	/* Get .rodata section */
	rodata = (*(struct __fatDeviceText **)fatCubinHandle)->d;

	/* Look for ELF head */
	/* FIXME: boundary check */
	for (i = 0; get_uint(rodata, i) != 0x464c457f; ++i)
		;
	elf_head = i;

	/* Get section header info */
	elf_header.e_shoff = get_uint(rodata, elf_head + 32);
	elf_header.e_shentsize = get_ushort(rodata, elf_head + 46);
	elf_header.e_shnum = get_ushort(rodata, elf_head + 48);

	/* Get section info */
	sections = (Elf32_Shdr *)xcalloc(elf_header.e_shnum,
			sizeof(Elf32_Shdr));
	for (i = 0; i < elf_header.e_shnum; ++i)
	{
		sections[i].sh_name = get_uint(rodata, 
				elf_head + 52 + i * elf_header.e_shentsize);
		sections[i].sh_offset = get_uint(rodata, 
				elf_head + 52 + i * elf_header.e_shentsize +
				16);
		sections[i].sh_size = get_uint(rodata, 
				elf_head + 52 + i * elf_header.e_shentsize +
				20);
	}

	/* Get string table head */
	elf_header.e_shstrndx = get_ushort(rodata, elf_head + 50);
	str_tab_head = elf_head + sections[elf_header.e_shstrndx].sh_offset;

	/* Get section names */
	section_names = (unsigned char **)xcalloc(elf_header.e_shnum, sizeof(unsigned char *));
	for (i = 0; i < elf_header.e_shnum - 1; ++i)
	{
		section_name_len = get_str_len(rodata, str_tab_head +
				sections[i].sh_name);
		section_names[i] = (unsigned char *)xcalloc(1, section_name_len
				+ 1);
		get_str(section_names[i], rodata, str_tab_head +
				sections[i].sh_name, str_tab_head + 
				sections[i].sh_name + section_name_len);
	}

	/* Look for .text.kernel_name section */
	snprintf(text_section_name, sizeof text_section_name, ".text.%s", deviceFun);
	for (i = 0; i < elf_header.e_shnum; ++i)
	{
		if (!strcmp(text_section_name, (char *)section_names[i]))
			break;
	}
	if (i == elf_header.e_shnum)
		fatal("%s section not found", text_section_name);
	text_section_index = i;

	/* Get instruction binary */
	inst_buffer = (unsigned long long int *)xcalloc(1,
			sections[text_section_index].sh_size);
	for (i = 0; i < sections[text_section_index].sh_size / 8; ++i)
		inst_buffer[i] = get_ulonglong(rodata, elf_head +
				sections[text_section_index].sh_offset + i * 8);
	inst_buffer_size = sections[text_section_index].sh_size;

	/* Load module */
	cuModuleLoad(&module, "ignored_cubin_filename");

	/* Get function */
	cuModuleGetFunction(&function, module, deviceFun);

	/* Free */
	free(inst_buffer);

	cuda_debug_print(stdout, "\treturn\n");
}




/*
 * CUDA Runtime API
 */

cudaError_t cudaDeviceReset(void)
{
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);

	/* Free default device */
	cuda_device_free(device);

	/* Free lists */
	list_free(context_list);
	list_free(device_list);
	list_free(module_list);
	list_free(function_list);

	/* FIXME: free resources in simulator */

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

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
	__CUDART_NOT_IMPL__
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
	cuda_debug_print(stdout, "CUDA runtime API '%s'\n", __FUNCTION__);
	cuda_debug_print(stdout, "\t(runtime) in: devPtr = %p\n", devPtr);

	cuMemFree((CUdeviceptr)devPtr);

	cuda_debug_print(stdout, "\t(runtime) out: return = %d\n", cudaSuccess);

	cuda_rt_last_error = cudaSuccess;

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

