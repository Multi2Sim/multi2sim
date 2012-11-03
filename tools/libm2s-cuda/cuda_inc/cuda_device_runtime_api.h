/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

#if !defined(__CNP_RUNTIME_API_H__)
#define __CNP_RUNTIME_API_H__

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

#if defined(__CUDABE__)

#if (__CUDA_ARCH__ >= 350) && !defined(__BUILDING_CNPRT__)
struct cudaFuncAttributes;

__device__ __attribute__((nv_weak)) cudaError_t cudaMalloc(void **p, size_t s)
{
  return cudaErrorUnknown;
}

__device__ __attribute__((nv_weak)) cudaError_t cudaFuncGetAttributes(struct cudaFuncAttributes *p, const void *c)
{
  return cudaErrorUnknown;
}

#endif /* (__CUDA_ARCH__ >= 350) && !defined(__BUILDING_CNPRT__) */

#else /* defined(__CUDABE__) */

#if defined(__cplusplus) && defined(__CUDACC__)         // Visible to nvcc front-end only
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 350)   // Visible to SM>=3.5 and "__host__ __device__" only

#include "driver_types.h"
#include "host_defines.h"

// Compatibility macros
#define CNPRTFUNC extern __device__ __cudart_builtin__

extern "C"
{
CNPRTFUNC cudaError_t CUDARTAPI cudaDeviceGetAttribute(int *value, enum cudaDeviceAttr attr, int device);
CNPRTFUNC cudaError_t CUDARTAPI cudaDeviceGetLimit(size_t *pValue, enum cudaLimit limit);
CNPRTFUNC cudaError_t CUDARTAPI cudaDeviceGetCacheConfig(enum cudaFuncCache *pCacheConfig);
CNPRTFUNC cudaError_t CUDARTAPI cudaDeviceGetSharedMemConfig(enum cudaSharedMemConfig *pConfig);
CNPRTFUNC cudaError_t CUDARTAPI cudaDeviceSynchronize(void);
CNPRTFUNC cudaError_t CUDARTAPI cudaGetLastError(void);
CNPRTFUNC cudaError_t CUDARTAPI cudaPeekAtLastError(void);
CNPRTFUNC const char* CUDARTAPI cudaGetErrorString(cudaError_t error);
CNPRTFUNC cudaError_t CUDARTAPI cudaGetDeviceCount(int *count);
CNPRTFUNC cudaError_t CUDARTAPI cudaGetDevice(int *device);
CNPRTFUNC cudaError_t CUDARTAPI cudaStreamCreateWithFlags(cudaStream_t *pStream, unsigned int flags);
CNPRTFUNC cudaError_t CUDARTAPI cudaStreamDestroy(cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event, unsigned int flags);
CNPRTFUNC cudaError_t CUDARTAPI cudaEventCreateWithFlags(cudaEvent_t *event, unsigned int flags);
CNPRTFUNC cudaError_t CUDARTAPI cudaEventRecord(cudaEvent_t event, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaEventDestroy(cudaEvent_t event);
CNPRTFUNC cudaError_t CUDARTAPI cudaFuncGetAttributes(struct cudaFuncAttributes *attr, const void *func);
CNPRTFUNC cudaError_t CUDARTAPI cudaFree(void *devPtr);
CNPRTFUNC cudaError_t CUDARTAPI cudaMalloc(void **devPtr, size_t size);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemcpyAsync(void *dst, const void *src, size_t count, enum cudaMemcpyKind kind, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemcpy2DAsync(void *dst, size_t dpitch, const void *src, size_t spitch, size_t width, size_t height, enum cudaMemcpyKind kind, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemcpy3DAsync(const struct cudaMemcpy3DParms *p, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemsetAsync(void *devPtr, int value, size_t count, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemset2DAsync(void *devPtr, size_t pitch, int value, size_t width, size_t height, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaMemset3DAsync(struct cudaPitchedPtr pitchedDevPtr, int value, struct cudaExtent extent, cudaStream_t stream);
CNPRTFUNC cudaError_t CUDARTAPI cudaRuntimeGetVersion(int *runtimeVersion);

CNPRTFUNC void * CUDARTAPI cudaGetParameterBuffer(size_t alignment, size_t size);
CNPRTFUNC cudaError_t CUDARTAPI cudaLaunchDevice(void *func, void *parameterBuffer, dim3 gridDimension, dim3 blockDimension, unsigned int sharedMemSize, cudaStream_t stream);
}
template <typename T> __inline__ __device__ __cudart_builtin__ cudaError_t cudaMalloc(T **devPtr, size_t size);
template <typename T> __inline__ __device__ __cudart_builtin__ cudaError_t cudaFuncGetAttributes(struct cudaFuncAttributes *attr, T *entry);

#endif // !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 350)
#endif // defined(__cplusplus) && defined(__CUDACC__)

#endif /* defined(__CUDABE__) */

#endif /* !__CNP_RUNTIME_API_H__ */
