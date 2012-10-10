/*
 * Copyright 1993-2011 NVIDIA Corporation.  All rights reserved.
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

#if !defined(__SM_20_INTRINSICS_H__)
#define __SM_20_INTRINSICS_H__

#if defined(__cplusplus) && defined(__CUDACC__)

#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 200

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

#include "builtin_types.h"
#include "device_types.h"
#include "host_defines.h"

extern "C"
{

/*DEVICE_BUILTIN*/
extern __device__ void                   __threadfence_system(void);

/*DEVICE_BUILTIN*/
extern __device__ double                __ddiv_rn(double, double);
/*DEVICE_BUILTIN*/
extern __device__ double                __ddiv_rz(double, double);
/*DEVICE_BUILTIN*/
extern __device__ double                __ddiv_ru(double, double);
/*DEVICE_BUILTIN*/
extern __device__ double                __ddiv_rd(double, double);

/*DEVICE_BUILTIN*/
extern __device__ double                __drcp_rn(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __drcp_rz(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __drcp_ru(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __drcp_rd(double);

/*DEVICE_BUILTIN*/
extern __device__ double                __dsqrt_rn(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __dsqrt_rz(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __dsqrt_ru(double);
/*DEVICE_BUILTIN*/
extern __device__ double                __dsqrt_rd(double);

/*DEVICE_BUILTIN*/
extern __device__ unsigned int          __ballot(int);
/*DEVICE_BUILTIN*/
extern __device__ int                   __syncthreads_count(int);
/*DEVICE_BUILTIN*/
extern __device__ int                   __syncthreads_and(int);
/*DEVICE_BUILTIN*/
extern __device__ int                   __syncthreads_or(int);

/*DEVICE_BUILTIN*/
extern __device__ long long int         clock64(void);

/* The following intrinsics are special versions of __fmaf_r?() that always 
 * map to a single-precision FMA instruction in non-FTZ mode, regardless of
 * the compiler's -ftz flag setting. These are supported for NVIDIA internal 
 * library work only.
 */

/*DEVICE_BUILTIN*/
extern __device__ float                 __fmaf_ieee_rn(float, float, float);
/*DEVICE_BUILTIN*/
extern __device__ float                 __fmaf_ieee_rz(float, float, float);
/*DEVICE_BUILTIN*/
extern __device__ float                 __fmaf_ieee_ru(float, float, float);
/*DEVICE_BUILTIN*/
extern __device__ float                 __fmaf_ieee_rd(float, float, float);

/*DEVICE_BUILTIN*/
extern __device__ double                __rcp64h(double);

}

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

static __inline__ __device__ unsigned int ballot(bool pred)
{
  return __ballot((int)pred);
}

static __inline__ __device__ int syncthreads_count(bool pred)
{
  return __syncthreads_count((int)pred);
}

static __inline__ __device__ bool syncthreads_and(bool pred)
{
  return (bool)__syncthreads_and((int)pred);
}

static __inline__ __device__ bool syncthreads_or(bool pred)
{
  return (bool)__syncthreads_or((int)pred);
}

#endif /* !__CUDA_ARCH__ || __CUDA_ARCH__ >= 200 */

#endif /* __cplusplus && __CUDACC__ */

#endif /* !__SM_20_INTRINSICS_H__ */
