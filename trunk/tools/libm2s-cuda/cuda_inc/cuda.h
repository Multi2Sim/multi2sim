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

#ifndef __cuda_cuda_h__
#define __cuda_cuda_h__

#include <stdlib.h>

/**
 * CUDA API versioning support
 */
#if defined(CUDA_FORCE_API_VERSION)
    #if (CUDA_FORCE_API_VERSION == 3010)
        #define __CUDA_API_VERSION 3010
    #else
        #error "Unsupported value of CUDA_FORCE_API_VERSION"
    #endif
#else
    #define __CUDA_API_VERSION 4000
#endif /* CUDA_FORCE_API_VERSION */

#if defined(__CUDA_API_VERSION_INTERNAL) || __CUDA_API_VERSION >= 3020
    #define cuDeviceTotalMem                    cuDeviceTotalMem_v2
    #define cuCtxCreate                         cuCtxCreate_v2
    #define cuModuleGetGlobal                   cuModuleGetGlobal_v2
    #define cuMemGetInfo                        cuMemGetInfo_v2
    #define cuMemAlloc                          cuMemAlloc_v2
    #define cuMemAllocPitch                     cuMemAllocPitch_v2
    #define cuMemFree                           cuMemFree_v2
    #define cuMemGetAddressRange                cuMemGetAddressRange_v2
    #define cuMemAllocHost                      cuMemAllocHost_v2
    #define cuMemHostGetDevicePointer           cuMemHostGetDevicePointer_v2
    #define cuMemcpyHtoD                        cuMemcpyHtoD_v2
    #define cuMemcpyDtoH                        cuMemcpyDtoH_v2
    #define cuMemcpyDtoD                        cuMemcpyDtoD_v2
    #define cuMemcpyDtoA                        cuMemcpyDtoA_v2
    #define cuMemcpyAtoD                        cuMemcpyAtoD_v2
    #define cuMemcpyHtoA                        cuMemcpyHtoA_v2
    #define cuMemcpyAtoH                        cuMemcpyAtoH_v2
    #define cuMemcpyAtoA                        cuMemcpyAtoA_v2
    #define cuMemcpyHtoAAsync                   cuMemcpyHtoAAsync_v2
    #define cuMemcpyAtoHAsync                   cuMemcpyAtoHAsync_v2
    #define cuMemcpy2D                          cuMemcpy2D_v2
    #define cuMemcpy2DUnaligned                 cuMemcpy2DUnaligned_v2
    #define cuMemcpy3D                          cuMemcpy3D_v2
    #define cuMemcpyHtoDAsync                   cuMemcpyHtoDAsync_v2
    #define cuMemcpyDtoHAsync                   cuMemcpyDtoHAsync_v2
    #define cuMemcpyDtoDAsync                   cuMemcpyDtoDAsync_v2
    #define cuMemcpy2DAsync                     cuMemcpy2DAsync_v2
    #define cuMemcpy3DAsync                     cuMemcpy3DAsync_v2
    #define cuMemsetD8                          cuMemsetD8_v2
    #define cuMemsetD16                         cuMemsetD16_v2
    #define cuMemsetD32                         cuMemsetD32_v2
    #define cuMemsetD2D8                        cuMemsetD2D8_v2
    #define cuMemsetD2D16                       cuMemsetD2D16_v2
    #define cuMemsetD2D32                       cuMemsetD2D32_v2
    #define cuArrayCreate                       cuArrayCreate_v2
    #define cuArrayGetDescriptor                cuArrayGetDescriptor_v2
    #define cuArray3DCreate                     cuArray3DCreate_v2
    #define cuArray3DGetDescriptor              cuArray3DGetDescriptor_v2
    #define cuTexRefSetAddress                  cuTexRefSetAddress_v2
    #define cuTexRefSetAddress2D                cuTexRefSetAddress2D_v2
    #define cuTexRefGetAddress                  cuTexRefGetAddress_v2
    #define cuGraphicsResourceGetMappedPointer  cuGraphicsResourceGetMappedPointer_v2
#endif /* __CUDA_API_VERSION_INTERNAL || __CUDA_API_VERSION >= 3020 */
#if defined(__CUDA_API_VERSION_INTERNAL) || __CUDA_API_VERSION >= 4000
    #define cuCtxDestroy                        cuCtxDestroy_v2
    #define cuCtxPopCurrent                     cuCtxPopCurrent_v2
    #define cuCtxPushCurrent                    cuCtxPushCurrent_v2
    #define cuStreamDestroy                     cuStreamDestroy_v2
    #define cuEventDestroy                      cuEventDestroy_v2
#endif /* __CUDA_API_VERSION_INTERNAL || __CUDA_API_VERSION >= 4000 */

/**
 * \defgroup CUDA_DRIVER CUDA Driver API
 *
 * This section describes the low-level CUDA driver application programming
 * interface.
 *
 * @{
 */

/**
 * \defgroup CUDA_TYPES Data types used by CUDA driver
 * @{
 */

/**
 * CUDA API version number
 */
#define CUDA_VERSION 4000 /* 4.0 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CUDA device pointer
 */ 
#if __CUDA_API_VERSION >= 3020

#if defined(__x86_64) || defined(AMD64) || defined(_M_AMD64)
typedef unsigned long long CUdeviceptr;
#else
typedef unsigned int CUdeviceptr;
#endif

#endif /* __CUDA_API_VERSION >= 3020 */

typedef int CUdevice;                                     /**< CUDA device */
typedef struct CUctx_st *CUcontext;                       /**< CUDA context */
typedef struct CUmod_st *CUmodule;                        /**< CUDA module */
typedef struct CUfunc_st *CUfunction;                     /**< CUDA function */
typedef struct CUarray_st *CUarray;                       /**< CUDA array */
typedef struct CUtexref_st *CUtexref;                     /**< CUDA texture reference */
typedef struct CUsurfref_st *CUsurfref;                   /**< CUDA surface reference */
typedef struct CUevent_st *CUevent;                       /**< CUDA event */
typedef struct CUstream_st *CUstream;                     /**< CUDA stream */
typedef struct CUgraphicsResource_st *CUgraphicsResource; /**< CUDA graphics interop resource */

typedef struct CUuuid_st {                                /**< CUDA definition of UUID */
    char bytes[16];
} CUuuid;

/**
 * Context creation flags
 */
typedef enum CUctx_flags_enum {
    CU_CTX_SCHED_AUTO          = 0x00, /**< Automatic scheduling */
    CU_CTX_SCHED_SPIN          = 0x01, /**< Set spin as default scheduling */
    CU_CTX_SCHED_YIELD         = 0x02, /**< Set yield as default scheduling */
    CU_CTX_SCHED_BLOCKING_SYNC = 0x04, /**< Set blocking synchronization as default scheduling */
    CU_CTX_BLOCKING_SYNC       = 0x04, /**< Set blocking synchronization as default scheduling \deprecated */
    CU_CTX_SCHED_MASK          = 0x07, 
    CU_CTX_MAP_HOST            = 0x08, /**< Support mapped pinned allocations */
    CU_CTX_LMEM_RESIZE_TO_MAX  = 0x10, /**< Keep local memory allocation after launch */
    CU_CTX_FLAGS_MASK          = 0x1f
} CUctx_flags;

/**
 * Event creation flags
 */
typedef enum CUevent_flags_enum {
    CU_EVENT_DEFAULT        = 0, /**< Default event flag */
    CU_EVENT_BLOCKING_SYNC  = 1, /**< Event uses blocking synchronization */
    CU_EVENT_DISABLE_TIMING = 2  /**< Event will not record timing data */
} CUevent_flags;

/**
 * Array formats
 */
typedef enum CUarray_format_enum {
    CU_AD_FORMAT_UNSIGNED_INT8  = 0x01, /**< Unsigned 8-bit integers */
    CU_AD_FORMAT_UNSIGNED_INT16 = 0x02, /**< Unsigned 16-bit integers */
    CU_AD_FORMAT_UNSIGNED_INT32 = 0x03, /**< Unsigned 32-bit integers */
    CU_AD_FORMAT_SIGNED_INT8    = 0x08, /**< Signed 8-bit integers */
    CU_AD_FORMAT_SIGNED_INT16   = 0x09, /**< Signed 16-bit integers */
    CU_AD_FORMAT_SIGNED_INT32   = 0x0a, /**< Signed 32-bit integers */
    CU_AD_FORMAT_HALF           = 0x10, /**< 16-bit floating point */
    CU_AD_FORMAT_FLOAT          = 0x20  /**< 32-bit floating point */
} CUarray_format;

/**
 * Texture reference addressing modes
 */
typedef enum CUaddress_mode_enum {
    CU_TR_ADDRESS_MODE_WRAP   = 0, /**< Wrapping address mode */
    CU_TR_ADDRESS_MODE_CLAMP  = 1, /**< Clamp to edge address mode */
    CU_TR_ADDRESS_MODE_MIRROR = 2, /**< Mirror address mode */
    CU_TR_ADDRESS_MODE_BORDER = 3  /**< Border address mode */
} CUaddress_mode;

/**
 * Texture reference filtering modes
 */
typedef enum CUfilter_mode_enum {
    CU_TR_FILTER_MODE_POINT  = 0, /**< Point filter mode */
    CU_TR_FILTER_MODE_LINEAR = 1  /**< Linear filter mode */
} CUfilter_mode;

/**
 * Device properties
 */
typedef enum CUdevice_attribute_enum {
    CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 1,              /**< Maximum number of threads per block */
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X = 2,                    /**< Maximum block dimension X */
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y = 3,                    /**< Maximum block dimension Y */
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z = 4,                    /**< Maximum block dimension Z */
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X = 5,                     /**< Maximum grid dimension X */
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y = 6,                     /**< Maximum grid dimension Y */
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z = 7,                     /**< Maximum grid dimension Z */
    CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK = 8,        /**< Maximum shared memory available per block in bytes */
    CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK = 8,            /**< Deprecated, use CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK */
    CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY = 9,              /**< Memory available on device for __constant__ variables in a CUDA C kernel in bytes */
    CU_DEVICE_ATTRIBUTE_WARP_SIZE = 10,                         /**< Warp size in threads */
    CU_DEVICE_ATTRIBUTE_MAX_PITCH = 11,                         /**< Maximum pitch in bytes allowed by memory copies */
    CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK = 12,           /**< Maximum number of 32-bit registers available per block */
    CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK = 12,               /**< Deprecated, use CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK */
    CU_DEVICE_ATTRIBUTE_CLOCK_RATE = 13,                        /**< Peak clock frequency in kilohertz */
    CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT = 14,                 /**< Alignment requirement for textures */
    CU_DEVICE_ATTRIBUTE_GPU_OVERLAP = 15,                       /**< Device can possibly copy memory and execute a kernel concurrently. Deprecated. Use instead CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT. */
    CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT = 16,              /**< Number of multiprocessors on device */
    CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT = 17,               /**< Specifies whether there is a run time limit on kernels */
    CU_DEVICE_ATTRIBUTE_INTEGRATED = 18,                        /**< Device is integrated with host memory */
    CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY = 19,               /**< Device can map host memory into CUDA address space */
    CU_DEVICE_ATTRIBUTE_COMPUTE_MODE = 20,                      /**< Compute mode (See ::CUcomputemode for details) */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH = 21,           /**< Maximum 1D texture width */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH = 22,           /**< Maximum 2D texture width */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT = 23,          /**< Maximum 2D texture height */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH = 24,           /**< Maximum 3D texture width */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT = 25,          /**< Maximum 3D texture height */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH = 26,           /**< Maximum 3D texture depth */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH = 27,   /**< Maximum 2D layered texture width */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT = 28,  /**< Maximum 2D layered texture height */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS = 29,  /**< Maximum layers in a 2D layered texture */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_WIDTH = 27,     /**< Deprecated, use CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_HEIGHT = 28,    /**< Deprecated, use CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_NUMSLICES = 29, /**< Deprecated, use CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS */
    CU_DEVICE_ATTRIBUTE_SURFACE_ALIGNMENT = 30,                 /**< Alignment requirement for surfaces */
    CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS = 31,                /**< Device can possibly execute multiple kernels concurrently */
    CU_DEVICE_ATTRIBUTE_ECC_ENABLED = 32,                       /**< Device has ECC support enabled */
    CU_DEVICE_ATTRIBUTE_PCI_BUS_ID = 33,                        /**< PCI bus ID of the device */
    CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID = 34,                     /**< PCI device ID of the device */
    CU_DEVICE_ATTRIBUTE_TCC_DRIVER = 35,                        /**< Device is using TCC driver model */
    CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE = 36,                 /**< Peak memory clock frequency in kilohertz */
    CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH = 37,           /**< Global memory bus width in bits */
    CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE = 38,                     /**< Size of L2 cache in bytes */
    CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR = 39,    /**< Maximum resident threads per multiprocessor */
    CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT = 40,                /**< Number of asynchronous engines */
    CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING = 41,                /**< Device shares a unified address space with the host */    
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH = 42,   /**< Maximum 1D layered texture width */
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS = 43,  /**< Maximum layers in a 1D layered texture */
    CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID = 50                      /**< PCI domain ID of the device */
} CUdevice_attribute;

/**
 * Legacy device properties
 */
typedef struct CUdevprop_st {
    int maxThreadsPerBlock;     /**< Maximum number of threads per block */
    int maxThreadsDim[3];       /**< Maximum size of each dimension of a block */
    int maxGridSize[3];         /**< Maximum size of each dimension of a grid */
    int sharedMemPerBlock;      /**< Shared memory available per block in bytes */
    int totalConstantMemory;    /**< Constant memory available on device in bytes */
    int SIMDWidth;              /**< Warp size in threads */
    int memPitch;               /**< Maximum pitch in bytes allowed by memory copies */
    int regsPerBlock;           /**< 32-bit registers available per block */
    int clockRate;              /**< Clock frequency in kilohertz */
    int textureAlign;           /**< Alignment requirement for textures */
} CUdevprop;

/**
 * Pointer information
 */
typedef enum CUpointer_attribute_enum {
    CU_POINTER_ATTRIBUTE_CONTEXT = 1,        /**< The ::CUcontext on which a pointer was allocated or registered */
    CU_POINTER_ATTRIBUTE_MEMORY_TYPE = 2,    /**< The ::CUmemorytype describing the physical location of a pointer */
    CU_POINTER_ATTRIBUTE_DEVICE_POINTER = 3, /**< The address at which a pointer's memory may be accessed on the device */
    CU_POINTER_ATTRIBUTE_HOST_POINTER = 4,   /**< The address at which a pointer's memory may be accessed on the host */
} CUpointer_attribute;

/**
 * Function properties
 */
typedef enum CUfunction_attribute_enum {
    /**
     * The maximum number of threads per block, beyond which a launch of the
     * function would fail. This number depends on both the function and the
     * device on which the function is currently loaded.
     */
    CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 0,

    /**
     * The size in bytes of statically-allocated shared memory required by
     * this function. This does not include dynamically-allocated shared
     * memory requested by the user at runtime.
     */
    CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES = 1,

    /**
     * The size in bytes of user-allocated constant memory required by this
     * function.
     */
    CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES = 2,

    /**
     * The size in bytes of local memory used by each thread of this function.
     */
    CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES = 3,

    /**
     * The number of registers used by each thread of this function.
     */
    CU_FUNC_ATTRIBUTE_NUM_REGS = 4,

    /**
     * The PTX virtual architecture version for which the function was
     * compiled. This value is the major PTX version * 10 + the minor PTX
     * version, so a PTX version 1.3 function would return the value 13.
     * Note that this may return the undefined value of 0 for cubins
     * compiled prior to CUDA 3.0.
     */
    CU_FUNC_ATTRIBUTE_PTX_VERSION = 5,

    /**
     * The binary architecture version for which the function was compiled.
     * This value is the major binary version * 10 + the minor binary version,
     * so a binary version 1.3 function would return the value 13. Note that
     * this will return a value of 10 for legacy cubins that do not have a
     * properly-encoded binary architecture version.
     */
    CU_FUNC_ATTRIBUTE_BINARY_VERSION = 6,

    CU_FUNC_ATTRIBUTE_MAX
} CUfunction_attribute;

/**
 * Function cache configurations
 */
typedef enum CUfunc_cache_enum {
    CU_FUNC_CACHE_PREFER_NONE    = 0x00, /**< no preference for shared memory or L1 (default) */
    CU_FUNC_CACHE_PREFER_SHARED  = 0x01, /**< prefer larger shared memory and smaller L1 cache */
    CU_FUNC_CACHE_PREFER_L1      = 0x02  /**< prefer larger L1 cache and smaller shared memory */
} CUfunc_cache;

/**
 * Memory types
 */
typedef enum CUmemorytype_enum {
    CU_MEMORYTYPE_HOST    = 0x01,    /**< Host memory */
    CU_MEMORYTYPE_DEVICE  = 0x02,    /**< Device memory */
    CU_MEMORYTYPE_ARRAY   = 0x03,    /**< Array memory */
    CU_MEMORYTYPE_UNIFIED = 0x04     /**< Unified device or host memory */
} CUmemorytype;

/**
 * Compute Modes
 */
typedef enum CUcomputemode_enum {
    CU_COMPUTEMODE_DEFAULT    = 0,  /**< Default compute mode (Multiple contexts allowed per device) */
    CU_COMPUTEMODE_EXCLUSIVE         = 1, /**< Compute-exclusive-thread mode (Only one context used by a single thread can be present on this device at a time) */
    CU_COMPUTEMODE_PROHIBITED        = 2, /**< Compute-prohibited mode (No contexts can be created on this device at this time) */
    CU_COMPUTEMODE_EXCLUSIVE_PROCESS = 3  /**< Compute-exclusive-process mode (Only one context used by a single process can be present on this device at a time) */
} CUcomputemode;

/**
 * Online compiler options
 */
typedef enum CUjit_option_enum
{
    /**
     * Max number of registers that a thread may use.\n
     * Option type: unsigned int
     */
    CU_JIT_MAX_REGISTERS = 0,

    /**
     * IN: Specifies minimum number of threads per block to target compilation
     * for\n
     * OUT: Returns the number of threads the compiler actually targeted.
     * This restricts the resource utilization fo the compiler (e.g. max
     * registers) such that a block with the given number of threads should be
     * able to launch based on register limitations. Note, this option does not
     * currently take into account any other resource limitations, such as
     * shared memory utilization.\n
     * Option type: unsigned int
     */
    CU_JIT_THREADS_PER_BLOCK,

    /**
     * Returns a float value in the option of the wall clock time, in
     * milliseconds, spent creating the cubin\n
     * Option type: float
     */
    CU_JIT_WALL_TIME,

    /**
     * Pointer to a buffer in which to print any log messsages from PTXAS
     * that are informational in nature (the buffer size is specified via
     * option ::CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES) \n
     * Option type: char*
     */
    CU_JIT_INFO_LOG_BUFFER,

    /**
     * IN: Log buffer size in bytes.  Log messages will be capped at this size
     * (including null terminator)\n
     * OUT: Amount of log buffer filled with messages\n
     * Option type: unsigned int
     */
    CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES,

    /**
     * Pointer to a buffer in which to print any log messages from PTXAS that
     * reflect errors (the buffer size is specified via option
     * ::CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES)\n
     * Option type: char*
     */
    CU_JIT_ERROR_LOG_BUFFER,

    /**
     * IN: Log buffer size in bytes.  Log messages will be capped at this size
     * (including null terminator)\n
     * OUT: Amount of log buffer filled with messages\n
     * Option type: unsigned int
     */
    CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES,

    /**
     * Level of optimizations to apply to generated code (0 - 4), with 4
     * being the default and highest level of optimizations.\n
     * Option type: unsigned int
     */
    CU_JIT_OPTIMIZATION_LEVEL,

    /**
     * No option value required. Determines the target based on the current
     * attached context (default)\n
     * Option type: No option value needed
     */
    CU_JIT_TARGET_FROM_CUCONTEXT,

    /**
     * Target is chosen based on supplied ::CUjit_target_enum.\n
     * Option type: unsigned int for enumerated type ::CUjit_target_enum
     */
    CU_JIT_TARGET,

    /**
     * Specifies choice of fallback strategy if matching cubin is not found.
     * Choice is based on supplied ::CUjit_fallback_enum.\n
     * Option type: unsigned int for enumerated type ::CUjit_fallback_enum
     */
    CU_JIT_FALLBACK_STRATEGY

} CUjit_option;

/**
 * Online compilation targets
 */
typedef enum CUjit_target_enum
{
    CU_TARGET_COMPUTE_10 = 0,   /**< Compute device class 1.0 */
    CU_TARGET_COMPUTE_11,       /**< Compute device class 1.1 */
    CU_TARGET_COMPUTE_12,       /**< Compute device class 1.2 */
    CU_TARGET_COMPUTE_13,       /**< Compute device class 1.3 */
    CU_TARGET_COMPUTE_20,       /**< Compute device class 2.0 */
    CU_TARGET_COMPUTE_21        /**< Compute device class 2.1 */
} CUjit_target;

/**
 * Cubin matching fallback strategies
 */
typedef enum CUjit_fallback_enum
{
    CU_PREFER_PTX = 0,  /**< Prefer to compile ptx */

    CU_PREFER_BINARY    /**< Prefer to fall back to compatible binary code */

} CUjit_fallback;

/**
 * Flags to register a graphics resource
 */
typedef enum CUgraphicsRegisterFlags_enum {
    CU_GRAPHICS_REGISTER_FLAGS_NONE          = 0x00,
    CU_GRAPHICS_REGISTER_FLAGS_READ_ONLY     = 0x01,
    CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD = 0x02,
    CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST  = 0x04
} CUgraphicsRegisterFlags;

/**
 * Flags for mapping and unmapping interop resources
 */
typedef enum CUgraphicsMapResourceFlags_enum {
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE          = 0x00,
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_READ_ONLY     = 0x01,
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITE_DISCARD = 0x02
} CUgraphicsMapResourceFlags;

/**
 * Array indices for cube faces
 */
typedef enum CUarray_cubemap_face_enum {
    CU_CUBEMAP_FACE_POSITIVE_X  = 0x00, /**< Positive X face of cubemap */
    CU_CUBEMAP_FACE_NEGATIVE_X  = 0x01, /**< Negative X face of cubemap */
    CU_CUBEMAP_FACE_POSITIVE_Y  = 0x02, /**< Positive Y face of cubemap */
    CU_CUBEMAP_FACE_NEGATIVE_Y  = 0x03, /**< Negative Y face of cubemap */
    CU_CUBEMAP_FACE_POSITIVE_Z  = 0x04, /**< Positive Z face of cubemap */
    CU_CUBEMAP_FACE_NEGATIVE_Z  = 0x05  /**< Negative Z face of cubemap */
} CUarray_cubemap_face;

/**
 * Limits
 */
typedef enum CUlimit_enum {
    CU_LIMIT_STACK_SIZE        = 0x00, /**< GPU thread stack size */
    CU_LIMIT_PRINTF_FIFO_SIZE  = 0x01, /**< GPU printf FIFO size */
    CU_LIMIT_MALLOC_HEAP_SIZE  = 0x02  /**< GPU malloc heap size */
} CUlimit;

/**
 * Error codes
 */
typedef enum cudaError_enum {
    /**
     * The API call returned with no errors. In the case of query calls, this
     * can also mean that the operation being queried is complete (see
     * ::cuEventQuery() and ::cuStreamQuery()).
     */
    CUDA_SUCCESS                              = 0,

    /**
     * This indicates that one or more of the parameters passed to the API call
     * is not within an acceptable range of values.
     */
    CUDA_ERROR_INVALID_VALUE                  = 1,

    /**
     * The API call failed because it was unable to allocate enough memory to
     * perform the requested operation.
     */
    CUDA_ERROR_OUT_OF_MEMORY                  = 2,

    /**
     * This indicates that the CUDA driver has not been initialized with
     * ::cuInit() or that initialization has failed.
     */
    CUDA_ERROR_NOT_INITIALIZED                = 3,

    /**
     * This indicates that the CUDA driver is in the process of shutting down.
     */
    CUDA_ERROR_DEINITIALIZED                  = 4,

    /**
     * This indicates profiling APIs are called while application is running
     * in visual profiler mode. 
    */
    CUDA_ERROR_PROFILER_DISABLED           = 5,
    /**
     * This indicates profiling has not been initialized for this context. 
     * Call cuProfilerInitialize() to resolve this. 
    */
    CUDA_ERROR_PROFILER_NOT_INITIALIZED       = 6,
    /**
     * This indicates profiler has already been started and probably
     * cuProfilerStart() is incorrectly called.
    */
    CUDA_ERROR_PROFILER_ALREADY_STARTED       = 7,
    /**
     * This indicates profiler has already been stopped and probably
     * cuProfilerStop() is incorrectly called.
    */
    CUDA_ERROR_PROFILER_ALREADY_STOPPED       = 8,  
    /**
     * This indicates that no CUDA-capable devices were detected by the installed
     * CUDA driver.
     */
    CUDA_ERROR_NO_DEVICE                      = 100,

    /**
     * This indicates that the device ordinal supplied by the user does not
     * correspond to a valid CUDA device.
     */
    CUDA_ERROR_INVALID_DEVICE                 = 101,


    /**
     * This indicates that the device kernel image is invalid. This can also
     * indicate an invalid CUDA module.
     */
    CUDA_ERROR_INVALID_IMAGE                  = 200,

    /**
     * This most frequently indicates that there is no context bound to the
     * current thread. This can also be returned if the context passed to an
     * API call is not a valid handle (such as a context that has had
     * ::cuCtxDestroy() invoked on it). This can also be returned if a user
     * mixes different API versions (i.e. 3010 context with 3020 API calls).
     * See ::cuCtxGetApiVersion() for more details.
     */
    CUDA_ERROR_INVALID_CONTEXT                = 201,

    /**
     * This indicated that the context being supplied as a parameter to the
     * API call was already the active context.
     * \deprecated
     * This error return is deprecated as of CUDA 3.2. It is no longer an
     * error to attempt to push the active context via ::cuCtxPushCurrent().
     */
    CUDA_ERROR_CONTEXT_ALREADY_CURRENT        = 202,

    /**
     * This indicates that a map or register operation has failed.
     */
    CUDA_ERROR_MAP_FAILED                     = 205,

    /**
     * This indicates that an unmap or unregister operation has failed.
     */
    CUDA_ERROR_UNMAP_FAILED                   = 206,

    /**
     * This indicates that the specified array is currently mapped and thus
     * cannot be destroyed.
     */
    CUDA_ERROR_ARRAY_IS_MAPPED                = 207,

    /**
     * This indicates that the resource is already mapped.
     */
    CUDA_ERROR_ALREADY_MAPPED                 = 208,

    /**
     * This indicates that there is no kernel image available that is suitable
     * for the device. This can occur when a user specifies code generation
     * options for a particular CUDA source file that do not include the
     * corresponding device configuration.
     */
    CUDA_ERROR_NO_BINARY_FOR_GPU              = 209,

    /**
     * This indicates that a resource has already been acquired.
     */
    CUDA_ERROR_ALREADY_ACQUIRED               = 210,

    /**
     * This indicates that a resource is not mapped.
     */
    CUDA_ERROR_NOT_MAPPED                     = 211,

    /**
     * This indicates that a mapped resource is not available for access as an
     * array.
     */
    CUDA_ERROR_NOT_MAPPED_AS_ARRAY            = 212,

    /**
     * This indicates that a mapped resource is not available for access as a
     * pointer.
     */
    CUDA_ERROR_NOT_MAPPED_AS_POINTER          = 213,

    /**
     * This indicates that an uncorrectable ECC error was detected during
     * execution.
     */
    CUDA_ERROR_ECC_UNCORRECTABLE              = 214,

    /**
     * This indicates that the ::CUlimit passed to the API call is not
     * supported by the active device.
     */
    CUDA_ERROR_UNSUPPORTED_LIMIT              = 215,

    /**
     * This indicates that the ::CUcontext passed to the API call can
     * only be bound to a single CPU thread at a time but is already 
     * bound to a CPU thread.
     */
    CUDA_ERROR_CONTEXT_ALREADY_IN_USE         = 216,

    /**
     * This indicates that the device kernel source is invalid.
     */
    CUDA_ERROR_INVALID_SOURCE                 = 300,

    /**
     * This indicates that the file specified was not found.
     */
    CUDA_ERROR_FILE_NOT_FOUND                 = 301,

    /**
     * This indicates that a link to a shared object failed to resolve.
     */
    CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND = 302,

    /**
     * This indicates that initialization of a shared object failed.
     */
    CUDA_ERROR_SHARED_OBJECT_INIT_FAILED      = 303,

    /**
     * This indicates that an OS call failed.
     */
    CUDA_ERROR_OPERATING_SYSTEM               = 304,


    /**
     * This indicates that a resource handle passed to the API call was not
     * valid. Resource handles are opaque types like ::CUstream and ::CUevent.
     */
    CUDA_ERROR_INVALID_HANDLE                 = 400,


    /**
     * This indicates that a named symbol was not found. Examples of symbols
     * are global/constant variable names, texture names, and surface names.
     */
    CUDA_ERROR_NOT_FOUND                      = 500,


    /**
     * This indicates that asynchronous operations issued previously have not
     * completed yet. This result is not actually an error, but must be indicated
     * differently than ::CUDA_SUCCESS (which indicates completion). Calls that
     * may return this value include ::cuEventQuery() and ::cuStreamQuery().
     */
    CUDA_ERROR_NOT_READY                      = 600,


    /**
     * An exception occurred on the device while executing a kernel. Common
     * causes include dereferencing an invalid device pointer and accessing
     * out of bounds shared memory. The context cannot be used, so it must
     * be destroyed (and a new one should be created). All existing device
     * memory allocations from this context are invalid and must be
     * reconstructed if the program is to continue using CUDA.
     */
    CUDA_ERROR_LAUNCH_FAILED                  = 700,

    /**
     * This indicates that a launch did not occur because it did not have
     * appropriate resources. This error usually indicates that the user has
     * attempted to pass too many arguments to the device kernel, or the
     * kernel launch specifies too many threads for the kernel's register
     * count. Passing arguments of the wrong size (i.e. a 64-bit pointer
     * when a 32-bit int is expected) is equivalent to passing too many
     * arguments and can also result in this error.
     */
    CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES        = 701,

    /**
     * This indicates that the device kernel took too long to execute. This can
     * only occur if timeouts are enabled - see the device attribute
     * ::CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT for more information. The
     * context cannot be used (and must be destroyed similar to
     * ::CUDA_ERROR_LAUNCH_FAILED). All existing device memory allocations from
     * this context are invalid and must be reconstructed if the program is to
     * continue using CUDA.
     */
    CUDA_ERROR_LAUNCH_TIMEOUT                 = 702,

    /**
     * This error indicates a kernel launch that uses an incompatible texturing
     * mode.
     */
    CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING  = 703,
    
    /**
     * This error indicates that a call to ::cuCtxEnablePeerAccess() is
     * trying to re-enable peer access to a context which has already
     * had peer access to it enabled.
     */
    CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED = 704,

    /**
     * This error indicates that ::cuCtxDisablePeerAccess() is 
     * trying to disable peer access which has not been enabled yet 
     * via ::cuCtxEnablePeerAccess(). 
     */
    CUDA_ERROR_PEER_ACCESS_NOT_ENABLED    = 705,

    /**
     * This error indicates that the primary context for the specified device
     * has already been initialized.
     */
    CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE         = 708,

    /**
     * This error indicates that the context current to the calling thread
     * has been destroyed using ::cuCtxDestroy, or is a primary context which
     * has not yet been initialized.
     */
    CUDA_ERROR_CONTEXT_IS_DESTROYED           = 709,

    /**
     * This indicates that an unknown internal error has occurred.
     */
    CUDA_ERROR_UNKNOWN                        = 999
} CUresult;

/**
 * If set, host memory is portable between CUDA contexts.
 * Flag for ::cuMemHostAlloc()
 */
#define CU_MEMHOSTALLOC_PORTABLE        0x01

/**
 * If set, host memory is mapped into CUDA address space and
 * ::cuMemHostGetDevicePointer() may be called on the host pointer.
 * Flag for ::cuMemHostAlloc()
 */
#define CU_MEMHOSTALLOC_DEVICEMAP       0x02

/**
 * If set, host memory is allocated as write-combined - fast to write,
 * faster to DMA, slow to read except via SSE4 streaming load instruction
 * (MOVNTDQA).
 * Flag for ::cuMemHostAlloc()
 */
#define CU_MEMHOSTALLOC_WRITECOMBINED   0x04

/**
 * If set, host memory is portable between CUDA contexts.
 * Flag for ::cuMemHostRegister()
 */
#define CU_MEMHOSTREGISTER_PORTABLE     0x01

/**
 * If set, host memory is mapped into CUDA address space and
 * ::cuMemHostGetDevicePointer() may be called on the host pointer.
 * Flag for ::cuMemHostRegister()
 */
#define CU_MEMHOSTREGISTER_DEVICEMAP    0x02

#if __CUDA_API_VERSION >= 3020

/**
 * 2D memory copy parameters
 */
typedef struct CUDA_MEMCPY2D_st {
    size_t srcXInBytes;         /**< Source X in bytes */
    size_t srcY;                /**< Source Y */

    CUmemorytype srcMemoryType; /**< Source memory type (host, device, array) */
    const void *srcHost;        /**< Source host pointer */
    CUdeviceptr srcDevice;      /**< Source device pointer */
    CUarray srcArray;           /**< Source array reference */
    size_t srcPitch;            /**< Source pitch (ignored when src is array) */

    size_t dstXInBytes;         /**< Destination X in bytes */
    size_t dstY;                /**< Destination Y */

    CUmemorytype dstMemoryType; /**< Destination memory type (host, device, array) */
    void *dstHost;              /**< Destination host pointer */
    CUdeviceptr dstDevice;      /**< Destination device pointer */
    CUarray dstArray;           /**< Destination array reference */
    size_t dstPitch;            /**< Destination pitch (ignored when dst is array) */

    size_t WidthInBytes;        /**< Width of 2D memory copy in bytes */
    size_t Height;              /**< Height of 2D memory copy */
} CUDA_MEMCPY2D;

/**
 * 3D memory copy parameters
 */
typedef struct CUDA_MEMCPY3D_st {
    size_t srcXInBytes;         /**< Source X in bytes */
    size_t srcY;                /**< Source Y */
    size_t srcZ;                /**< Source Z */
    size_t srcLOD;              /**< Source LOD */
    CUmemorytype srcMemoryType; /**< Source memory type (host, device, array) */
    const void *srcHost;        /**< Source host pointer */
    CUdeviceptr srcDevice;      /**< Source device pointer */
    CUarray srcArray;           /**< Source array reference */
    void *reserved0;            /**< Must be NULL */
    size_t srcPitch;            /**< Source pitch (ignored when src is array) */
    size_t srcHeight;           /**< Source height (ignored when src is array; may be 0 if Depth==1) */

    size_t dstXInBytes;         /**< Destination X in bytes */
    size_t dstY;                /**< Destination Y */
    size_t dstZ;                /**< Destination Z */
    size_t dstLOD;              /**< Destination LOD */
    CUmemorytype dstMemoryType; /**< Destination memory type (host, device, array) */
    void *dstHost;              /**< Destination host pointer */
    CUdeviceptr dstDevice;      /**< Destination device pointer */
    CUarray dstArray;           /**< Destination array reference */
    void *reserved1;            /**< Must be NULL */
    size_t dstPitch;            /**< Destination pitch (ignored when dst is array) */
    size_t dstHeight;           /**< Destination height (ignored when dst is array; may be 0 if Depth==1) */

    size_t WidthInBytes;        /**< Width of 3D memory copy in bytes */
    size_t Height;              /**< Height of 3D memory copy */
    size_t Depth;               /**< Depth of 3D memory copy */
} CUDA_MEMCPY3D;

/**
 * 3D memory cross-context copy parameters
 */
typedef struct CUDA_MEMCPY3D_PEER_st {
    size_t srcXInBytes;         /**< Source X in bytes */
    size_t srcY;                /**< Source Y */
    size_t srcZ;                /**< Source Z */
    size_t srcLOD;              /**< Source LOD */
    CUmemorytype srcMemoryType; /**< Source memory type (host, device, array) */
    const void *srcHost;        /**< Source host pointer */
    CUdeviceptr srcDevice;      /**< Source device pointer */
    CUarray srcArray;           /**< Source array reference */
    CUcontext srcContext;       /**< Source context (ignored with srcMemoryType is ::CU_MEMORYTYPE_ARRAY) */
    size_t srcPitch;            /**< Source pitch (ignored when src is array) */
    size_t srcHeight;           /**< Source height (ignored when src is array; may be 0 if Depth==1) */

    size_t dstXInBytes;         /**< Destination X in bytes */
    size_t dstY;                /**< Destination Y */
    size_t dstZ;                /**< Destination Z */
    size_t dstLOD;              /**< Destination LOD */
    CUmemorytype dstMemoryType; /**< Destination memory type (host, device, array) */
    void *dstHost;              /**< Destination host pointer */
    CUdeviceptr dstDevice;      /**< Destination device pointer */
    CUarray dstArray;           /**< Destination array reference */
    CUcontext dstContext;       /**< Destination context (ignored with dstMemoryType is ::CU_MEMORYTYPE_ARRAY) */
    size_t dstPitch;            /**< Destination pitch (ignored when dst is array) */
    size_t dstHeight;           /**< Destination height (ignored when dst is array; may be 0 if Depth==1) */

    size_t WidthInBytes;        /**< Width of 3D memory copy in bytes */
    size_t Height;              /**< Height of 3D memory copy */
    size_t Depth;               /**< Depth of 3D memory copy */
} CUDA_MEMCPY3D_PEER;

/**
 * Array descriptor
 */
typedef struct CUDA_ARRAY_DESCRIPTOR_st
{
    size_t Width;             /**< Width of array */
    size_t Height;            /**< Height of array */

    CUarray_format Format;    /**< Array format */
    unsigned int NumChannels; /**< Channels per array element */
} CUDA_ARRAY_DESCRIPTOR;

/**
 * 3D array descriptor
 */
typedef struct CUDA_ARRAY3D_DESCRIPTOR_st
{
    size_t Width;             /**< Width of 3D array */
    size_t Height;            /**< Height of 3D array */
    size_t Depth;             /**< Depth of 3D array */

    CUarray_format Format;    /**< Array format */
    unsigned int NumChannels; /**< Channels per array element */
    unsigned int Flags;       /**< Flags */
} CUDA_ARRAY3D_DESCRIPTOR;

#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * If set, the CUDA array is a collection of layers, where each layer is either a 1D
 * or a 2D array and the Depth member of CUDA_ARRAY3D_DESCRIPTOR specifies the number 
 * of layers, not the depth of a 3D array.
 */
#define CUDA_ARRAY3D_LAYERED        0x01

/**
 * Deprecated, use CUDA_ARRAY3D_LAYERED
 */
#define CUDA_ARRAY3D_2DARRAY        0x01

/**
 * This flag must be set in order to bind a surface reference
 * to the CUDA array
 */
#define CUDA_ARRAY3D_SURFACE_LDST   0x02

/**
 * Override the texref format with a format inferred from the array.
 * Flag for ::cuTexRefSetArray()
 */
#define CU_TRSA_OVERRIDE_FORMAT 0x01

/**
 * Read the texture as integers rather than promoting the values to floats
 * in the range [0,1].
 * Flag for ::cuTexRefSetFlags()
 */
#define CU_TRSF_READ_AS_INTEGER         0x01

/**
 * Use normalized texture coordinates in the range [0,1) instead of [0,dim).
 * Flag for ::cuTexRefSetFlags()
 */
#define CU_TRSF_NORMALIZED_COORDINATES  0x02

/**
 * Perform sRGB->linear conversion during texture read.
 * Flag for ::cuTexRefSetFlags()
 */
#define CU_TRSF_SRGB  0x10

/**
 * End of array terminator for the \p extra parameter to
 * ::cuLaunchKernel
 */
#define CU_LAUNCH_PARAM_END            ((void*)0x00)

/**
 * Indicator that the next value in the \p extra parameter to
 * ::cuLaunchKernel will be a pointer to a buffer containing all kernel
 * parameters used for launching kernel \p f.  This buffer needs to
 * honor all alignment/padding requirements of the individual parameters.
 * If ::CU_LAUNCH_PARAM_BUFFER_SIZE is not also specified in the
 * \p extra array, then ::CU_LAUNCH_PARAM_BUFFER_POINTER will have no
 * effect.
 */
#define CU_LAUNCH_PARAM_BUFFER_POINTER ((void*)0x01)

/**
 * Indicator that the next value in the \p extra parameter to
 * ::cuLaunchKernel will be a pointer to a size_t which contains the
 * size of the buffer specified with ::CU_LAUNCH_PARAM_BUFFER_POINTER.
 * It is required that ::CU_LAUNCH_PARAM_BUFFER_POINTER also be specified
 * in the \p extra array if the value associated with
 * ::CU_LAUNCH_PARAM_BUFFER_SIZE is not zero.
 */
#define CU_LAUNCH_PARAM_BUFFER_SIZE    ((void*)0x02)

/**
 * For texture references loaded into the module, use default texunit from
 * texture reference.
 */
#define CU_PARAM_TR_DEFAULT -1

/** @} */ /* END CUDA_TYPES */

#ifdef _WIN32
#define CUDAAPI __stdcall
#else
#define CUDAAPI
#endif

/**
 * \defgroup CUDA_INITIALIZE Initialization
 *
 * This section describes the initialization functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

/**
 * \brief Initialize the CUDA driver API
 *
 * Initializes the driver API and must be called before any other function from
 * the driver API. Currently, the \p Flags parameter must be 0. If ::cuInit()
 * has not been called, any function from the driver API will return
 * ::CUDA_ERROR_NOT_INITIALIZED.
 *
 * \param Flags - Initialization flag for CUDA.
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 */
CUresult CUDAAPI cuInit(unsigned int Flags);

/** @} */ /* END CUDA_INITIALIZE */

/**
 * \defgroup CUDA_VERSION Version Management
 *
 * This section describes the version management functions of the low-level
 * CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Returns the CUDA driver version
 *
 * Returns in \p *driverVersion the version number of the installed CUDA
 * driver. This function automatically returns ::CUDA_ERROR_INVALID_VALUE if
 * the \p driverVersion argument is NULL.
 *
 * \param driverVersion - Returns the CUDA driver version
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 */
CUresult CUDAAPI cuDriverGetVersion(int *driverVersion);

/** @} */ /* END CUDA_VERSION */

/**
 * \defgroup CUDA_DEVICE Device Management
 *
 * This section describes the device management functions of the low-level
 * CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Returns a handle to a compute device
 *
 * Returns in \p *device a device handle given an ordinal in the range <b>[0,
 * ::cuDeviceGetCount()-1]</b>.
 *
 * \param device  - Returned device handle
 * \param ordinal - Device number to get handle for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetCount,
 * ::cuDeviceGetName,
 * ::cuDeviceGetProperties,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceGet(CUdevice *device, int ordinal);

/**
 * \brief Returns the number of compute-capable devices
 *
 * Returns in \p *count the number of devices with compute capability greater
 * than or equal to 1.0 that are available for execution. If there is no such
 * device, ::cuDeviceGetCount() returns 0.
 *
 * \param count - Returned number of compute-capable devices
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetName,
 * ::cuDeviceGet,
 * ::cuDeviceGetProperties,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceGetCount(int *count);

/**
 * \brief Returns an identifer string for the device
 *
 * Returns an ASCII string identifying the device \p dev in the NULL-terminated
 * string pointed to by \p name. \p len specifies the maximum length of the
 * string that may be returned.
 *
 * \param name - Returned identifier string for the device
 * \param len  - Maximum length of string to store in \p name
 * \param dev  - Device to get identifier string for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetCount,
 * ::cuDeviceGet,
 * ::cuDeviceGetProperties,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceGetName(char *name, int len, CUdevice dev);

/**
 * \brief Returns the compute capability of the device
 *
 * Returns in \p *major and \p *minor the major and minor revision numbers that
 * define the compute capability of the device \p dev.
 *
 * \param major - Major revision number
 * \param minor - Minor revision number
 * \param dev   - Device handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetCount,
 * ::cuDeviceGetName,
 * ::cuDeviceGet,
 * ::cuDeviceGetProperties,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceComputeCapability(int *major, int *minor, CUdevice dev);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Returns the total amount of memory on the device
 *
 * Returns in \p *bytes the total amount of memory available on the device
 * \p dev in bytes.
 *
 * \param bytes - Returned memory available on device in bytes
 * \param dev   - Device handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetCount,
 * ::cuDeviceGetName,
 * ::cuDeviceGet,
 * ::cuDeviceGetProperties,
 */
CUresult CUDAAPI cuDeviceTotalMem(size_t *bytes, CUdevice dev);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Returns properties for a selected device
 *
 * Returns in \p *prop the properties of device \p dev. The ::CUdevprop
 * structure is defined as:
 *
 * \code
     typedef struct CUdevprop_st {
     int maxThreadsPerBlock;
     int maxThreadsDim[3];
     int maxGridSize[3];
     int sharedMemPerBlock;
     int totalConstantMemory;
     int SIMDWidth;
     int memPitch;
     int regsPerBlock;
     int clockRate;
     int textureAlign
  } CUdevprop;
 * \endcode
 * where:
 *
 * - ::maxThreadsPerBlock is the maximum number of threads per block;
 * - ::maxThreadsDim[3] is the maximum sizes of each dimension of a block;
 * - ::maxGridSize[3] is the maximum sizes of each dimension of a grid;
 * - ::sharedMemPerBlock is the total amount of shared memory available per
 *   block in bytes;
 * - ::totalConstantMemory is the total amount of constant memory available on
 *   the device in bytes;
 * - ::SIMDWidth is the warp size;
 * - ::memPitch is the maximum pitch allowed by the memory copy functions that
 *   involve memory regions allocated through ::cuMemAllocPitch();
 * - ::regsPerBlock is the total number of registers available per block;
 * - ::clockRate is the clock frequency in kilohertz;
 * - ::textureAlign is the alignment requirement; texture base addresses that
 *   are aligned to ::textureAlign bytes do not need an offset applied to
 *   texture fetches.
 *
 * \param prop - Returned properties of device
 * \param dev  - Device to get properties for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetAttribute,
 * ::cuDeviceGetCount,
 * ::cuDeviceGetName,
 * ::cuDeviceGet,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceGetProperties(CUdevprop *prop, CUdevice dev);

/**
 * \brief Returns information about the device
 *
 * Returns in \p *pi the integer value of the attribute \p attrib on device
 * \p dev. The supported attributes are:
 * - ::CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK: Maximum number of threads per
 *   block;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X: Maximum x-dimension of a block;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y: Maximum y-dimension of a block;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z: Maximum z-dimension of a block;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X: Maximum x-dimension of a grid;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y: Maximum y-dimension of a grid;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z: Maximum z-dimension of a grid;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK: Maximum amount of
 *   shared memory available to a thread block in bytes; this amount is shared
 *   by all thread blocks simultaneously resident on a multiprocessor;
 * - ::CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY: Memory available on device for
 *   __constant__ variables in a CUDA C kernel in bytes;
 * - ::CU_DEVICE_ATTRIBUTE_WARP_SIZE: Warp size in threads;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_PITCH: Maximum pitch in bytes allowed by the
 *   memory copy functions that involve memory regions allocated through
 *   ::cuMemAllocPitch();
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH: Maximum 1D 
 *  texture width;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH: Maximum 2D 
 *  texture width;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT: Maximum 2D 
 *  texture height;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH: Maximum 3D 
 *  texture width;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT: Maximum 3D 
 *  texture height;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH: Maximum 3D 
 *  texture depth;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH: 
 *  Maximum 1D layered texture width;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS: 
 *   Maximum layers in a 1D layered texture;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH: 
 *  Maximum 2D layered texture width;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT: 
 *   Maximum 2D layered texture height;
 * - ::CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS: 
 *   Maximum layers in a 2D layered texture;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK: Maximum number of 32-bit
 *   registers available to a thread block; this number is shared by all thread
 *   blocks simultaneously resident on a multiprocessor;
 * - ::CU_DEVICE_ATTRIBUTE_CLOCK_RATE: Peak clock frequency in kilohertz;
 * - ::CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT: Alignment requirement; texture
 *   base addresses aligned to ::textureAlign bytes do not need an offset
 *   applied to texture fetches;
 * - ::CU_DEVICE_ATTRIBUTE_GPU_OVERLAP: 1 if the device can concurrently copy
 *   memory between host and device while executing a kernel, or 0 if not;
 * - ::CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT: Number of multiprocessors on
 *   the device;
 * - ::CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT: 1 if there is a run time limit
 *   for kernels executed on the device, or 0 if not;
 * - ::CU_DEVICE_ATTRIBUTE_INTEGRATED: 1 if the device is integrated with the
 *   memory subsystem, or 0 if not;
 * - ::CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY: 1 if the device can map host
 *   memory into the CUDA address space, or 0 if not;
 * - ::CU_DEVICE_ATTRIBUTE_COMPUTE_MODE: Compute mode that device is currently
 *   in. Available modes are as follows:
 *   - ::CU_COMPUTEMODE_DEFAULT: Default mode - Device is not restricted and
 *     can have multiple CUDA contexts present at a single time.
 *   - ::CU_COMPUTEMODE_EXCLUSIVE: Compute-exclusive mode - Device can have
 *     only one CUDA context present on it at a time.
 *   - ::CU_COMPUTEMODE_PROHIBITED: Compute-prohibited mode - Device is
 *     prohibited from creating new CUDA contexts.
 *   - ::CU_COMPUTEMODE_EXCLUSIVE_PROCESS:  Compute-exclusive-process mode - Device
 *     can have only one context used by a single process at a time.
 * - ::CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS: 1 if the device supports
 *   executing multiple kernels within the same context simultaneously, or 0 if
 *   not. It is not guaranteed that multiple kernels will be resident
 *   on the device concurrently so this feature should not be relied upon for
 *   correctness;
 * - ::CU_DEVICE_ATTRIBUTE_ECC_ENABLED: 1 if error correction is enabled on the
 *    device, 0 if error correction is disabled or not supported by the device;
 * - ::CU_DEVICE_ATTRIBUTE_PCI_BUS_ID: PCI bus identifier of the device;
 * - ::CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID: PCI device (also known as slot) identifier
 *   of the device;
 * - ::CU_DEVICE_ATTRIBUTE_TCC_DRIVER: 1 if the device is using a TCC driver. TCC
 *    is only available on Tesla hardware running Windows Vista or later;
 * - ::CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE: Peak memory clock frequency in kilohertz;
 * - ::CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH: Global memory bus width in bits;
 * - ::CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE: Size of L2 cache in bytes. 0 if the device doesn't have L2 cache;
 * - ::CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR: Maximum resident threads per multiprocessor;
 * - ::CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING: 1 if the device shares a unified address space with 
 *   the host, or 0 if not;
 *
 * \param pi     - Returned device attribute value
 * \param attrib - Device attribute to query
 * \param dev    - Device handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuDeviceComputeCapability,
 * ::cuDeviceGetCount,
 * ::cuDeviceGetName,
 * ::cuDeviceGet,
 * ::cuDeviceGetProperties,
 * ::cuDeviceTotalMem
 */
CUresult CUDAAPI cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev);

/** @} */ /* END CUDA_DEVICE */


/**
 * \defgroup CUDA_CTX Context Management
 *
 * This section describes the context management functions of the low-level
 * CUDA driver application programming interface.
 *
 * @{
 */

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Create a CUDA context
 *
 * Creates a new CUDA context and associates it with the calling thread. The
 * \p flags parameter is described below. The context is created with a usage
 * count of 1 and the caller of ::cuCtxCreate() must call ::cuCtxDestroy() or
 * when done using the context. If a context is already current to the thread, 
 * it is supplanted by the newly created context and may be restored by a subsequent 
 * call to ::cuCtxPopCurrent().
 *
 * The three LSBs of the \p flags parameter can be used to control how the OS
 * thread, which owns the CUDA context at the time of an API call, interacts
 * with the OS scheduler when waiting for results from the GPU. Only one of
 * the scheduling flags can be set when creating a context.
 *
 * - ::CU_CTX_SCHED_AUTO: The default value if the \p flags parameter is zero,
 * uses a heuristic based on the number of active CUDA contexts in the
 * process \e C and the number of logical processors in the system \e P. If
 * \e C > \e P, then CUDA will yield to other OS threads when waiting for
 * the GPU, otherwise CUDA will not yield while waiting for results and
 * actively spin on the processor.
 *
 * - ::CU_CTX_SCHED_SPIN: Instruct CUDA to actively spin when waiting for
 * results from the GPU. This can decrease latency when waiting for the GPU,
 * but may lower the performance of CPU threads if they are performing work in
 * parallel with the CUDA thread.
 *
 * - ::CU_CTX_SCHED_YIELD: Instruct CUDA to yield its thread when waiting for
 * results from the GPU. This can increase latency when waiting for the GPU,
 * but can increase the performance of CPU threads performing work in parallel
 * with the GPU.
 * 
 * - ::CU_CTX_SCHED_BLOCKING_SYNC: Instruct CUDA to block the CPU thread on a
 * synchronization primitive when waiting for the GPU to finish work.
 *
 * - ::CU_CTX_BLOCKING_SYNC: Instruct CUDA to block the CPU thread on a
 * synchronization primitive when waiting for the GPU to finish work.
 * \deprecated This flag was deprecated as of CUDA 4.0 and was replaced 
 * with ::CU_CTX_SCHED_BLOCKING_SYNC. 
 *
 * - ::CU_CTX_MAP_HOST: Instruct CUDA to support mapped pinned allocations.
 * This flag must be set in order to allocate pinned host memory that is
 * accessible to the GPU.
 *
 * - ::CU_CTX_LMEM_RESIZE_TO_MAX: Instruct CUDA to not reduce local memory
 * after resizing local memory for a kernel. This can prevent thrashing by
 * local memory allocations when launching many kernels with high local
 * memory usage at the cost of potentially increased memory usage.
 *
 * Context creation will fail with ::CUDA_ERROR_UNKNOWN if the compute mode of
 * the device is ::CU_COMPUTEMODE_PROHIBITED. Similarly, context creation will
 * also fail with ::CUDA_ERROR_UNKNOWN if the compute mode for the device is
 * set to ::CU_COMPUTEMODE_EXCLUSIVE and there is already an active context on
 * the device. The function ::cuDeviceGetAttribute() can be used with
 * ::CU_DEVICE_ATTRIBUTE_COMPUTE_MODE to determine the compute mode of the
 * device. The <i>nvidia-smi</i> tool can be used to set the compute mode for
 * devices. Documentation for <i>nvidia-smi</i> can be obtained by passing a
 * -h option to it.
 *
 * \param pctx  - Returned context handle of the new context
 * \param flags - Context creation flags
 * \param dev   - Device to create context on
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_DEVICE,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev);
#endif /* __CUDA_API_VERSION >= 3020 */

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Destroy a CUDA context
 *
 * Destroys the CUDA context specified by \p ctx.  The context \p ctx will be
 * destroyed regardless of how many threads it is current to.  It is the 
 * caller's responsibility to ensure that no API call is issued to \p ctx
 * while ::cuCtxDestroy() is executing.
 *
 * If \p ctx is current to the calling thread then \p ctx will also be 
 * popped from the current thread's context stack (as though ::cuCtxPopCurrent()
 * were called).  If \p ctx is current to other threads, then \p ctx will
 * remain current to those threads, and attempting to access \p ctx from
 * those threads will result in the error ::CUDA_ERROR_CONTEXT_IS_DESTROYED.
 *
 * \param ctx - Context to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxDestroy(CUcontext ctx);
#endif /* __CUDA_API_VERSION >= 4000 */

/**
 * \defgroup CUDA_CTX_DEPRECATED Context Management [DEPRECATED]
 *
 * This section describes the deprecated context management functions of the low-level
 * CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Increment a context's usage-count
 *
 * \deprecated
 *
 * Note that this function is deprecated and should not be used.
 *
 * Increments the usage count of the context and passes back a context handle
 * in \p *pctx that must be passed to ::cuCtxDetach() when the application is
 * done with the context. ::cuCtxAttach() fails if there is no context current
 * to the thread.
 *
 * Currently, the \p flags parameter must be 0.
 *
 * \param pctx  - Returned context handle of the current context
 * \param flags - Context attach flags (must be 0)
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxDetach,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxAttach(CUcontext *pctx, unsigned int flags);

/**
 * \brief Decrement a context's usage-count
 *
 * \deprecated
 *
 * Note that this function is deprecated and should not be used.
 *
 * Decrements the usage count of the context \p ctx, and destroys the context
 * if the usage count goes to 0. The context must be a handle that was passed
 * back by ::cuCtxCreate() or ::cuCtxAttach(), and must be current to the
 * calling thread.
 *
 * \param ctx - Context to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxDetach(CUcontext ctx);

/** @} */ /* END CUDA_CTX_DEPRECATED */

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Pushes a context on the current CPU thread
 *
 * Pushes the given context \p ctx onto the CPU thread's stack of current
 * contexts. The specified context becomes the CPU thread's current context, so
 * all CUDA functions that operate on the current context are affected.
 *
 * The previous current context may be made current again by calling
 * ::cuCtxDestroy() or ::cuCtxPopCurrent().
 *
 * \param ctx - Context to push
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxPushCurrent(CUcontext ctx);

/**
 * \brief Pops the current CUDA context from the current CPU thread.
 *
 * Pops the current CUDA context from the CPU thread and passes back the 
 * old context handle in \p *pctx. That context may then be made current 
 * to a different CPU thread by calling ::cuCtxPushCurrent().
 *
 * If a context was current to the CPU thread before ::cuCtxCreate() or
 * ::cuCtxPushCurrent() was called, this function makes that context current to
 * the CPU thread again.
 *
 * \param pctx - Returned new context handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxPopCurrent(CUcontext *pctx);

/**
 * \brief Binds the specified CUDA context to the calling CPU thread
 *
 * Binds the specified CUDA context to the calling CPU thread.
 * If \p ctx is NULL then the CUDA context previously bound to the
 * calling CPU thread is unbound and ::CUDA_SUCCESS is returned.
 *
 * If there exists a CUDA context stack on the calling CPU thread, this
 * will replace the top of that stack with \p ctx.  
 * If \p ctx is NULL then this will be equivalent to popping the top
 * of the calling CPU thread's CUDA context stack (or a no-op if the
 * calling CPU thread's CUDA context stack is empty).
 *
 * \param ctx - Context to bind to the calling CPU thread
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT
 * \notefnerr
 *
 * \sa ::cuCtxGetCurrent, ::cuCtxCreate, ::cuCtxDestroy
 */
CUresult CUDAAPI cuCtxSetCurrent(CUcontext ctx);

/**
 * \brief Returns the CUDA context bound to the calling CPU thread.
 *
 * Returns in \p *pctx the CUDA context bound to the calling CPU thread.
 * If no context is bound to the calling CPU thread then \p *pctx is
 * set to NULL and ::CUDA_SUCCESS is returned.
 *
 * \param pctx - Returned context handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * \notefnerr
 *
 * \sa ::cuCtxSetCurrent, ::cuCtxCreate, ::cuCtxDestroy
 */
CUresult CUDAAPI cuCtxGetCurrent(CUcontext *pctx);
#endif /* __CUDA_API_VERSION >= 4000 */

/**
 * \brief Returns the device ID for the current context
 *
 * Returns in \p *device the ordinal of the current context's device.
 *
 * \param device - Returned device ID for the current context
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxGetDevice(CUdevice *device);

/**
 * \brief Block for a context's tasks to complete
 *
 * Blocks until the device has completed all preceding requested tasks.
 * ::cuCtxSynchronize() returns an error if one of the preceding tasks failed.
 * If the context was created with the ::CU_CTX_SCHED_BLOCKING_SYNC flag, the 
 * CPU thread will block until the GPU context has finished its work.
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit
 */
CUresult CUDAAPI cuCtxSynchronize(void);

/**
 * \brief Set resource limits
 *
 * Setting \p limit to \p value is a request by the application to update
 * the current limit maintained by the context.  The driver is free to
 * modify the requested value to meet h/w requirements (this could be
 * clamping to minimum or maximum values, rounding up to nearest element
 * size, etc).  The application can use ::cuCtxGetLimit() to find out exactly
 * what the limit has been set to.
 *
 * Setting each ::CUlimit has its own specific restrictions, so each is
 * discussed here.
 *
 * - ::CU_LIMIT_STACK_SIZE controls the stack size of each GPU thread.
 *   This limit is only applicable to devices of compute capability
 *   2.0 and higher.  Attempting to set this limit on devices of
 *   compute capability less than 2.0 will result in the error
 *   ::CUDA_ERROR_UNSUPPORTED_LIMIT being returned.
 *
 * - ::CU_LIMIT_PRINTF_FIFO_SIZE controls the size of the FIFO used
 *   by the ::printf() device system call.  Setting
 *   ::CU_LIMIT_PRINTF_FIFO_SIZE must be performed before launching any
 *   kernel that uses the ::printf() device system call, otherwise
 *   ::CUDA_ERROR_INVALID_VALUE will be returned.
 *   This limit is only applicable to devices of compute capability
 *   2.0 and higher.  Attempting to set this limit on devices of
 *   compute capability less than 2.0 will result in the error
 *   ::CUDA_ERROR_UNSUPPORTED_LIMIT being returned.
 *
 * - ::CU_LIMIT_MALLOC_HEAP_SIZE controls the size of the heap used
 *   by the ::malloc() and ::free() device system calls.  Setting
 *   ::CU_LIMIT_MALLOC_HEAP_SIZE must be performed before launching
 *   any kernel that uses the ::malloc() or ::free() device system calls,
 *   otherwise ::CUDA_ERROR_INVALID_VALUE will be returned.
 *   This limit is only applicable to devices of compute capability
 *   2.0 and higher.  Attempting to set this limit on devices of
 *   compute capability less than 2.0 will result in the error
 *   ::CUDA_ERROR_UNSUPPORTED_LIMIT being returned.
 *
 * \param limit - Limit to set
 * \param value - Size in bytes of limit
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_UNSUPPORTED_LIMIT
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxSetLimit(CUlimit limit, size_t value);

/**
 * \brief Returns resource limits
 *
 * Returns in \p *pvalue the current size of \p limit.  The supported
 * ::CUlimit values are:
 * - ::CU_LIMIT_STACK_SIZE: stack size of each GPU thread;
 * - ::CU_LIMIT_PRINTF_FIFO_SIZE: size of the FIFO used by the
 *   ::printf() device system call.
 * - ::CU_LIMIT_MALLOC_HEAP_SIZE: size of the heap used by the
 *   ::malloc() and ::free() device system calls;
 *
 * \param limit  - Limit to query
 * \param pvalue - Returned size in bytes of limit
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_UNSUPPORTED_LIMIT
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxGetLimit(size_t *pvalue, CUlimit limit);

/**
 * \brief Returns the preferred cache configuration for the current context.
 *
 * On devices where the L1 cache and shared memory use the same hardware
 * resources, this returns through \p pconfig the preferred cache configuration
 * for the current context. This is only a preference. The driver will use
 * the requested configuration if possible, but it is free to choose a different
 * configuration if required to execute functions.
 *
 * This will return a \p pconfig of ::CU_FUNC_CACHE_PREFER_NONE on devices
 * where the size of the L1 cache and shared memory are fixed.
 *
 * The supported cache configurations are:
 * - ::CU_FUNC_CACHE_PREFER_NONE: no preference for shared memory or L1 (default)
 * - ::CU_FUNC_CACHE_PREFER_SHARED: prefer larger shared memory and smaller L1 cache
 * - ::CU_FUNC_CACHE_PREFER_L1: prefer larger L1 cache and smaller shared memory
 *
 * \param pconfig - Returned cache configuration
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize,
 * ::cuFuncSetCacheConfig
 */
CUresult CUDAAPI cuCtxGetCacheConfig(CUfunc_cache *pconfig);

/**
 * \brief Sets the preferred cache configuration for the current context.
 *
 * On devices where the L1 cache and shared memory use the same hardware
 * resources, this sets through \p config the preferred cache configuration for
 * the current context. This is only a preference. The driver will use
 * the requested configuration if possible, but it is free to choose a different
 * configuration if required to execute the function. Any function preference
 * set via ::cuFuncSetCacheConfig() will be preferred over this context-wide
 * setting. Setting the context-wide cache configuration to
 * ::CU_FUNC_CACHE_PREFER_NONE will cause subsequent kernel launches to prefer
 * to not change the cache configuration unless required to launch the kernel.
 *
 * This setting does nothing on devices where the size of the L1 cache and
 * shared memory are fixed.
 *
 * Launching a kernel with a different preference than the most recent
 * preference setting may insert a device-side synchronization point.
 *
 * The supported cache configurations are:
 * - ::CU_FUNC_CACHE_PREFER_NONE: no preference for shared memory or L1 (default)
 * - ::CU_FUNC_CACHE_PREFER_SHARED: prefer larger shared memory and smaller L1 cache
 * - ::CU_FUNC_CACHE_PREFER_L1: prefer larger L1 cache and smaller shared memory
 *
 * \param config - Requested cache configuration
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetApiVersion,
 * ::cuCtxGetCacheConfig,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize,
 * ::cuFuncSetCacheConfig
 */
CUresult CUDAAPI cuCtxSetCacheConfig(CUfunc_cache config);

/**
 * \brief Gets the context's API version.
 *
 * Returns the API version used to create \p ctx in \p version. If \p ctx
 * is NULL, returns the API version used to create the currently bound
 * context.
 *
 * This wil return the API version used to create a context (for example,
 * 3010 or 3020), which library developers can use to direct callers to a
 * specific API version. Note that this API version may not be the same as
 * returned by cuDriverGetVersion.
 *
 * \param ctx     - Context to check
 * \param version - Pointer to version
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa ::cuCtxCreate,
 * ::cuCtxDestroy,
 * ::cuCtxGetDevice,
 * ::cuCtxGetLimit,
 * ::cuCtxPopCurrent,
 * ::cuCtxPushCurrent,
 * ::cuCtxSetCacheConfig,
 * ::cuCtxSetLimit,
 * ::cuCtxSynchronize
 */
CUresult CUDAAPI cuCtxGetApiVersion(CUcontext ctx, unsigned int *version);

/** @} */ /* END CUDA_CTX */


/**
 * \defgroup CUDA_MODULE Module Management
 *
 * This section describes the module management functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

/**
 * \brief Loads a compute module
 *
 * Takes a filename \p fname and loads the corresponding module \p module into
 * the current context. The CUDA driver API does not attempt to lazily
 * allocate the resources needed by a module; if the memory for functions and
 * data (constant and global) needed by the module cannot be allocated,
 * ::cuModuleLoad() fails. The file should be a \e cubin file as output by
 * \b nvcc, or a \e PTX file either as output by \b nvcc or handwritten, or
 * a \e fatbin file as output by \b nvcc from toolchain 4.0 or later.
 *
 * \param module - Returned module
 * \param fname  - Filename of module to load
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_FILE_NOT_FOUND,
 * ::CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleLoad(CUmodule *module, const char *fname);

/**
 * \brief Load a module's data
 *
 * Takes a pointer \p image and loads the corresponding module \p module into
 * the current context. The pointer may be obtained by mapping a \e cubin or
 * \e PTX or \e fatbin file, passing a \e cubin or \e PTX or \e fatbin file
 * as a NULL-terminated text string, or incorporating a \e cubin or \e fatbin
 * object into the executable resources and using operating system calls such
 * as Windows \c FindResource() to obtain the pointer.
 *
 * \param module - Returned module
 * \param image  - Module data to load
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleLoadData(CUmodule *module, const void *image);

/**
 * \brief Load a module's data with options
 *
 * Takes a pointer \p image and loads the corresponding module \p module into
 * the current context. The pointer may be obtained by mapping a \e cubin or
 * \e PTX or \e fatbin file, passing a \e cubin or \e PTX or \e fatbin file
 * as a NULL-terminated text string, or incorporating a \e cubin or \e fatbin
 * object into the executable resources and using operating system calls such
 * as Windows \c FindResource() to obtain the pointer. Options are passed as
 * an array via \p options and any corresponding parameters are passed in
 * \p optionValues. The number of total options is supplied via \p numOptions.
 * Any outputs will be returned via \p optionValues. Supported options are
 * (types for the option values are specified in parentheses after the option
 * name):
 *
 * - ::CU_JIT_MAX_REGISTERS: (unsigned int) input specifies the maximum number
 * of registers per thread;
 * - ::CU_JIT_THREADS_PER_BLOCK: (unsigned int) input specifies number of
 * threads per block to target compilation for; output returns the number of
 * threads the compiler actually targeted;
 * - ::CU_JIT_WALL_TIME: (float) output returns the float value of wall clock
 * time, in milliseconds, spent compiling the \e PTX code;
 * - ::CU_JIT_INFO_LOG_BUFFER: (char*) input is a pointer to a buffer in
 * which to print any informational log messages from \e PTX assembly (the
 * buffer size is specified via option ::CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES);
 * - ::CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES: (unsigned int) input is the size in
 * bytes of the buffer; output is the number of bytes filled with messages;
 * - ::CU_JIT_ERROR_LOG_BUFFER: (char*) input is a pointer to a buffer in
 * which to print any error log messages from \e PTX assembly (the buffer size
 * is specified via option ::CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES);
 * - ::CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES: (unsigned int) input is the size in
 * bytes of the buffer; output is the number of bytes filled with messages;
 * - ::CU_JIT_OPTIMIZATION_LEVEL: (unsigned int) input is the level of
 * optimization to apply to generated code (0 - 4), with 4 being the default
 * and highest level;
 * - ::CU_JIT_TARGET_FROM_CUCONTEXT: (No option value) causes compilation
 * target to be determined based on current attached context (default);
 * - ::CU_JIT_TARGET: (unsigned int for enumerated type ::CUjit_target_enum)
 * input is the compilation target based on supplied ::CUjit_target_enum;
 * possible values are:
 *   - ::CU_TARGET_COMPUTE_10
 *   - ::CU_TARGET_COMPUTE_11
 *   - ::CU_TARGET_COMPUTE_12
 *   - ::CU_TARGET_COMPUTE_13
 *   - ::CU_TARGET_COMPUTE_20
 * - ::CU_JIT_FALLBACK_STRATEGY: (unsigned int for enumerated type
 * ::CUjit_fallback_enum) chooses fallback strategy if matching cubin is not
 * found; possible values are:
 *   - ::CU_PREFER_PTX
 *   - ::CU_PREFER_BINARY
 *
 * \param module       - Returned module
 * \param image        - Module data to load
 * \param numOptions   - Number of options
 * \param options      - Options for JIT
 * \param optionValues - Option values for JIT
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_NO_BINARY_FOR_GPU,
 * ::CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues);

/**
 * \brief Load a module's data
 *
 * Takes a pointer \p fatCubin and loads the corresponding module \p module
 * into the current context. The pointer represents a <i>fat binary</i> object,
 * which is a collection of different \e cubin and/or \e PTX files, all
 * representing the same device code, but compiled and optimized for different
 * architectures.
 *
 * Prior to CUDA 4.0, there was no documented API for constructing and using
 * fat binary objects by programmers.  Starting with CUDA 4.0, fat binary
 * objects can be constructed by providing the <i>-fatbin option</i> to \b nvcc.
 * More information can be found in the \b nvcc document.
 *
 * \param module   - Returned module
 * \param fatCubin - Fat binary to load
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_NO_BINARY_FOR_GPU,
 * ::CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin);

/**
 * \brief Unloads a module
 *
 * Unloads a module \p hmod from the current context.
 *
 * \param hmod - Module to unload
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary
 */
CUresult CUDAAPI cuModuleUnload(CUmodule hmod);

/**
 * \brief Returns a function handle
 *
 * Returns in \p *hfunc the handle of the function of name \p name located in
 * module \p hmod. If no function of that name exists, ::cuModuleGetFunction()
 * returns ::CUDA_ERROR_NOT_FOUND.
 *
 * \param hfunc - Returned function handle
 * \param hmod  - Module to retrieve function from
 * \param name  - Name of function to retrieve
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND
 * \notefnerr
 *
 * \sa ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Returns a global pointer from a module
 *
 * Returns in \p *dptr and \p *bytes the base pointer and size of the
 * global of name \p name located in module \p hmod. If no variable of that name
 * exists, ::cuModuleGetGlobal() returns ::CUDA_ERROR_NOT_FOUND. Both
 * parameters \p dptr and \p bytes are optional. If one of them is
 * NULL, it is ignored.
 *
 * \param dptr  - Returned global device pointer
 * \param bytes - Returned global size in bytes
 * \param hmod  - Module to retrieve global from
 * \param name  - Name of global to retrieve
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleGetGlobal(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod, const char *name);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Returns a handle to a texture reference
 *
 * Returns in \p *pTexRef the handle of the texture reference of name \p name
 * in the module \p hmod. If no texture reference of that name exists,
 * ::cuModuleGetTexRef() returns ::CUDA_ERROR_NOT_FOUND. This texture reference
 * handle should not be destroyed, since it will be destroyed when the module
 * is unloaded.
 *
 * \param pTexRef  - Returned texture reference
 * \param hmod     - Module to retrieve texture reference from
 * \param name     - Name of texture reference to retrieve
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetSurfRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleGetTexRef(CUtexref *pTexRef, CUmodule hmod, const char *name);

/**
 * \brief Returns a handle to a surface reference
 *
 * Returns in \p *pSurfRef the handle of the surface reference of name \p name
 * in the module \p hmod. If no surface reference of that name exists,
 * ::cuModuleGetSurfRef() returns ::CUDA_ERROR_NOT_FOUND.
 *
 * \param pSurfRef  - Returned surface reference
 * \param hmod     - Module to retrieve surface reference from
 * \param name     - Name of surface reference to retrieve
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_FOUND
 * \notefnerr
 *
 * \sa ::cuModuleGetFunction,
 * ::cuModuleGetGlobal,
 * ::cuModuleGetTexRef,
 * ::cuModuleLoad,
 * ::cuModuleLoadData,
 * ::cuModuleLoadDataEx,
 * ::cuModuleLoadFatBinary,
 * ::cuModuleUnload
 */
CUresult CUDAAPI cuModuleGetSurfRef(CUsurfref *pSurfRef, CUmodule hmod, const char *name);

/** @} */ /* END CUDA_MODULE */


/**
 * \defgroup CUDA_MEM Memory Management
 *
 * This section describes the memory management functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Gets free and total memory
 *
 * Returns in \p *free and \p *total respectively, the free and total amount of
 * memory available for allocation by the CUDA context, in bytes.
 *
 * \param free  - Returned free memory in bytes
 * \param total - Returned total memory in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemGetInfo(size_t *free, size_t *total);

/**
 * \brief Allocates device memory
 *
 * Allocates \p bytesize bytes of linear memory on the device and returns in
 * \p *dptr a pointer to the allocated memory. The allocated memory is suitably
 * aligned for any kind of variable. The memory is not cleared. If \p bytesize
 * is 0, ::cuMemAlloc() returns ::CUDA_ERROR_INVALID_VALUE.
 *
 * \param dptr     - Returned device pointer
 * \param bytesize - Requested allocation size in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemAlloc(CUdeviceptr *dptr, size_t bytesize);

/**
 * \brief Allocates pitched device memory
 *
 * Allocates at least \p WidthInBytes * \p Height bytes of linear memory on
 * the device and returns in \p *dptr a pointer to the allocated memory. The
 * function may pad the allocation to ensure that corresponding pointers in
 * any given row will continue to meet the alignment requirements for
 * coalescing as the address is updated from row to row. \p ElementSizeBytes
 * specifies the size of the largest reads and writes that will be performed
 * on the memory range. \p ElementSizeBytes may be 4, 8 or 16 (since coalesced
 * memory transactions are not possible on other data sizes). If
 * \p ElementSizeBytes is smaller than the actual read/write size of a kernel,
 * the kernel will run correctly, but possibly at reduced speed. The pitch
 * returned in \p *pPitch by ::cuMemAllocPitch() is the width in bytes of the
 * allocation. The intended usage of pitch is as a separate parameter of the
 * allocation, used to compute addresses within the 2D array. Given the row
 * and column of an array element of type \b T, the address is computed as:
 * \code
   T* pElement = (T*)((char*)BaseAddress + Row * Pitch) + Column;
 * \endcode
 *
 * The pitch returned by ::cuMemAllocPitch() is guaranteed to work with
 * ::cuMemcpy2D() under all circumstances. For allocations of 2D arrays, it is
 * recommended that programmers consider performing pitch allocations using
 * ::cuMemAllocPitch(). Due to alignment restrictions in the hardware, this is
 * especially true if the application will be performing 2D memory copies
 * between different regions of device memory (whether linear memory or CUDA
 * arrays).
 *
 * The byte alignment of the pitch returned by ::cuMemAllocPitch() is guaranteed
 * to match or exceed the alignment requirement for texture binding with
 * ::cuTexRefSetAddress2D().
 *
 * \param dptr             - Returned device pointer
 * \param pPitch           - Returned pitch of allocation in bytes
 * \param WidthInBytes     - Requested allocation width in bytes
 * \param Height           - Requested allocation height in rows
 * \param ElementSizeBytes - Size of largest reads/writes for range
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemAllocPitch(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes);

/**
 * \brief Frees device memory
 *
 * Frees the memory space pointed to by \p dptr, which must have been returned
 * by a previous call to ::cuMemAlloc() or ::cuMemAllocPitch().
 *
 * \param dptr - Pointer to memory to free
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemFree(CUdeviceptr dptr);

/**
 * \brief Get information on memory allocations
 *
 * Returns the base address in \p *pbase and size in \p *psize of the
 * allocation by ::cuMemAlloc() or ::cuMemAllocPitch() that contains the input
 * pointer \p dptr. Both parameters \p pbase and \p psize are optional. If one
 * of them is NULL, it is ignored.
 *
 * \param pbase - Returned base address
 * \param psize - Returned size of device memory allocation
 * \param dptr  - Device pointer to query
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemGetAddressRange(CUdeviceptr *pbase, size_t *psize, CUdeviceptr dptr);

/**
 * \brief Allocates page-locked host memory
 *
 * Allocates \p bytesize bytes of host memory that is page-locked and
 * accessible to the device. The driver tracks the virtual memory ranges
 * allocated with this function and automatically accelerates calls to
 * functions such as ::cuMemcpy(). Since the memory can be accessed directly by
 * the device, it can be read or written with much higher bandwidth than
 * pageable memory obtained with functions such as ::malloc(). Allocating
 * excessive amounts of memory with ::cuMemAllocHost() may degrade system
 * performance, since it reduces the amount of memory available to the system
 * for paging. As a result, this function is best used sparingly to allocate
 * staging areas for data exchange between host and device.
 *
 * Note all host memory allocated using ::cuMemHostAlloc() will automatically
 * be immediately accessible to all contexts on all devices which support unified
 * addressing (as may be queried using ::CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING).
 * The device pointer that may be used to access this host memory from those 
 * contexts is always equal to the returned host pointer \p *pp.
 * See \ref CUDA_UNIFIED for additional details.
 *
 * \param pp       - Returned host pointer to page-locked memory
 * \param bytesize - Requested allocation size in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemAllocHost(void **pp, size_t bytesize);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Frees page-locked host memory
 *
 * Frees the memory space pointed to by \p p, which must have been returned by
 * a previous call to ::cuMemAllocHost().
 *
 * \param p - Pointer to memory to free
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemFreeHost(void *p);

/**
 * \brief Allocates page-locked host memory
 *
 * Allocates \p bytesize bytes of host memory that is page-locked and accessible
 * to the device. The driver tracks the virtual memory ranges allocated with
 * this function and automatically accelerates calls to functions such as
 * ::cuMemcpyHtoD(). Since the memory can be accessed directly by the device,
 * it can be read or written with much higher bandwidth than pageable memory
 * obtained with functions such as ::malloc(). Allocating excessive amounts of
 * pinned memory may degrade system performance, since it reduces the amount
 * of memory available to the system for paging. As a result, this function is
 * best used sparingly to allocate staging areas for data exchange between
 * host and device.
 *
 * The \p Flags parameter enables different options to be specified that
 * affect the allocation, as follows.
 *
 * - ::CU_MEMHOSTALLOC_PORTABLE: The memory returned by this call will be
 *   considered as pinned memory by all CUDA contexts, not just the one that
 *   performed the allocation.
 *
 * - ::CU_MEMHOSTALLOC_DEVICEMAP: Maps the allocation into the CUDA address
 *   space. The device pointer to the memory may be obtained by calling
 *   ::cuMemHostGetDevicePointer(). This feature is available only on GPUs
 *   with compute capability greater than or equal to 1.1.
 *
 * - ::CU_MEMHOSTALLOC_WRITECOMBINED: Allocates the memory as write-combined
 *   (WC). WC memory can be transferred across the PCI Express bus more
 *   quickly on some system configurations, but cannot be read efficiently by
 *   most CPUs. WC memory is a good option for buffers that will be written by
 *   the CPU and read by the GPU via mapped pinned memory or host->device
 *   transfers.
 *
 * All of these flags are orthogonal to one another: a developer may allocate
 * memory that is portable, mapped and/or write-combined with no restrictions.
 *
 * The CUDA context must have been created with the ::CU_CTX_MAP_HOST flag in
 * order for the ::CU_MEMHOSTALLOC_MAPPED flag to have any effect.
 *
 * The ::CU_MEMHOSTALLOC_MAPPED flag may be specified on CUDA contexts for
 * devices that do not support mapped pinned memory. The failure is deferred
 * to ::cuMemHostGetDevicePointer() because the memory may be mapped into
 * other CUDA contexts via the ::CU_MEMHOSTALLOC_PORTABLE flag.
 *
 * The memory allocated by this function must be freed with ::cuMemFreeHost().
 *
 * Note all host memory allocated using ::cuMemHostAlloc() will automatically
 * be immediately accessible to all contexts on all devices which support unified
 * addressing (as may be queried using ::CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING).
 * Unless the flag ::CU_MEMHOSTALLOC_WRITECOMBINED is specified, the device pointer 
 * that may be used to access this host memory from those contexts is always equal 
 * to the returned host pointer \p *pp.  If the flag ::CU_MEMHOSTALLOC_WRITECOMBINED
 * is specified, then the function ::cuMemHostGetDevicePointer() must be used
 * to query the device pointer, even if the context supports unified addressing.
 * See \ref CUDA_UNIFIED for additional details.
 *
 * \param pp       - Returned host pointer to page-locked memory
 * \param bytesize - Requested allocation size in bytes
 * \param Flags    - Flags for allocation request
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Passes back device pointer of mapped pinned memory
 *
 * Passes back the device pointer \p pdptr corresponding to the mapped, pinned
 * host buffer \p p allocated by ::cuMemHostAlloc.
 *
 * ::cuMemHostGetDevicePointer() will fail if the ::CU_MEMALLOCHOST_DEVICEMAP
 * flag was not specified at the time the memory was allocated, or if the
 * function is called on a GPU that does not support mapped pinned memory.
 *
 * \p Flags provides for future releases. For now, it must be set to 0.
 *
 * \param pdptr - Returned device pointer
 * \param p     - Host pointer
 * \param Flags - Options (must be 0)
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p, unsigned int Flags);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Passes back flags that were used for a pinned allocation
 *
 * Passes back the flags \p pFlags that were specified when allocating
 * the pinned host buffer \p p allocated by ::cuMemHostAlloc.
 *
 * ::cuMemHostGetFlags() will fail if the pointer does not reside in
 * an allocation performed by ::cuMemAllocHost() or ::cuMemHostAlloc().
 *
 * \param pFlags - Returned flags word
 * \param p     - Host pointer
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuMemAllocHost, ::cuMemHostAlloc
 */
CUresult CUDAAPI cuMemHostGetFlags(unsigned int *pFlags, void *p);

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Registers an existing host memory range for use by CUDA
 *
 * Page-locks the memory range specified by \p p and \p bytesize and maps it
 * for the device(s) as specified by \p Flags. This memory range also is added
 * to the same tracking mechanism as ::cuMemHostAlloc to automatically accelerate
 * calls to functions such as ::cuMemcpyHtoD(). Since the memory can be accessed 
 * directly by the device, it can be read or written with much higher bandwidth 
 * than pageable memory that has not been registered.  Page-locking excessive
 * amounts of memory may degrade system performance, since it reduces the amount
 * of memory available to the system for paging. As a result, this function is
 * best used sparingly to register staging areas for data exchange between
 * host and device.
 *
 * This function is not yet supported on Mac OS X.
 *
 * The \p Flags parameter enables different options to be specified that
 * affect the allocation, as follows.
 *
 * - ::CU_MEMHOSTREGISTER_PORTABLE: The memory returned by this call will be
 *   considered as pinned memory by all CUDA contexts, not just the one that
 *   performed the allocation.
 *
 * - ::CU_MEMHOSTREGISTER_DEVICEMAP: Maps the allocation into the CUDA address
 *   space. The device pointer to the memory may be obtained by calling
 *   ::cuMemHostGetDevicePointer(). This feature is available only on GPUs
 *   with compute capability greater than or equal to 1.1.
 *
 * All of these flags are orthogonal to one another: a developer may page-lock
 * memory that is portable or mapped with no restrictions.
 *
 * The CUDA context must have been created with the ::CU_CTX_MAP_HOST flag in
 * order for the ::CU_MEMHOSTREGISTER_DEVICEMAP flag to have any effect.
 *
 * The ::CU_MEMHOSTREGISTER_DEVICEMAP flag may be specified on CUDA contexts for
 * devices that do not support mapped pinned memory. The failure is deferred
 * to ::cuMemHostGetDevicePointer() because the memory may be mapped into
 * other CUDA contexts via the ::CU_MEMHOSTREGISTER_PORTABLE flag.
 *
 * The pointer \p p and size \p bytesize must be aligned to the host page 
 * size (4 KB).
 *
 * The memory page-locked by this function must be unregistered with 
 * ::cuMemHostUnregister().
 *
 * \param p        - Host pointer to memory to page-lock
 * \param bytesize - Size in bytes of the address range to page-lock
 * \param Flags    - Flags for allocation request
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuMemHostUnregister, ::cuMemHostGetFlags, ::cuMemHostGetDevicePointer
 */
CUresult CUDAAPI cuMemHostRegister(void *p, size_t bytesize, unsigned int Flags);

/**
 * \brief Unregisters a memory range that was registered with ::cuMemHostRegister().
 *
 * Unmaps the memory range whose base address is specified by \p p, and makes
 * it pageable again.
 *
 * The base address must be the same one specified to ::cuMemHostRegister().
 *
 * \param p - Host pointer to memory to unregister
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuMemHostRegister
 */
CUresult CUDAAPI cuMemHostUnregister(void *p);

/**
 * \brief Copies memory
 *
 * Copies data between two pointers. 
 * \p dst and \p src are base pointers of the destination and source, respectively.  
 * \p ByteCount specifies the number of bytes to copy.
 * Note that this function infers the type of the transfer (host to host, host to 
 *   device, device to device, or device to host) from the pointer values.  This
 *   function is only allowed in contexts which support unified addressing.
 * Note that this function is synchronous.
 *
 * \param dst - Destination unified virtual address space pointer
 * \param src - Source unified virtual address space pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount);

/**
 * \brief Copies device memory between two contexts
 *
 * Copies from device memory in one context to device memory in another
 * context. \p dstDevice is the base device pointer of the destination memory 
 * and \p dstContext is the destination context.  \p srcDevice is the base 
 * device pointer of the source memory and \p srcContext is the source pointer.  
 * \p ByteCount specifies the number of bytes to copy.

 * Note that this function is asynchronous with respect to the host, but 
 * serialized with respect all pending and future asynchronous work in to the 
 * current context, \p srcContext, and \p dstContext (use ::cuMemcpyPeerAsync 
 * to avoid this synchronization).
 *
 * \param dstDevice  - Destination device pointer
 * \param dstContext - Destination context
 * \param srcDevice  - Source device pointer
 * \param srcContext - Source context
 * \param ByteCount  - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuMemcpyDtoD, ::cuMemcpy3DPeer, ::cuMemcpyDtoDAsync, ::cuMemcpyPeerAsync,
 * ::cuMemcpy3DPeerAsync
 */
CUresult CUDAAPI cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount);



#endif /* __CUDA_API_VERSION >= 4000 */

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Copies memory from Host to Device
 *
 * Copies from host memory to device memory. \p dstDevice and \p srcHost are
 * the base addresses of the destination and source, respectively. \p ByteCount
 * specifies the number of bytes to copy. Note that this function is
 * synchronous.
 *
 * \param dstDevice - Destination device pointer
 * \param srcHost   - Source host pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount);

/**
 * \brief Copies memory from Device to Host
 *
 * Copies from device to host memory. \p dstHost and \p srcDevice specify the
 * base pointers of the destination and source, respectively. \p ByteCount
 * specifies the number of bytes to copy. Note that this function is
 * synchronous.
 *
 * \param dstHost   - Destination host pointer
 * \param srcDevice - Source device pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount);

/**
 * \brief Copies memory from Device to Device
 *
 * Copies from device memory to device memory. \p dstDevice and \p srcDevice
 * are the base pointers of the destination and source, respectively.
 * \p ByteCount specifies the number of bytes to copy. Note that this function
 * is asynchronous.
 *
 * \param dstDevice - Destination device pointer
 * \param srcDevice - Source device pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyDtoD(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount);

/**
 * \brief Copies memory from Device to Array
 *
 * Copies from device memory to a 1D CUDA array. \p dstArray and \p dstOffset
 * specify the CUDA array handle and starting index of the destination data.
 * \p srcDevice specifies the base pointer of the source. \p ByteCount
 * specifies the number of bytes to copy.
 *
 * \param dstArray  - Destination array
 * \param dstOffset - Offset in bytes of destination array
 * \param srcDevice - Source device pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyDtoA(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount);

/**
 * \brief Copies memory from Array to Device
 *
 * Copies from one 1D CUDA array to device memory. \p dstDevice specifies the
 * base pointer of the destination and must be naturally aligned with the CUDA
 * array elements. \p srcArray and \p srcOffset specify the CUDA array handle
 * and the offset in bytes into the array where the copy is to begin.
 * \p ByteCount specifies the number of bytes to copy and must be evenly
 * divisible by the array element size.
 *
 * \param dstDevice - Destination device pointer
 * \param srcArray  - Source array
 * \param srcOffset - Offset in bytes of source array
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyAtoD(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount);

/**
 * \brief Copies memory from Host to Array
 *
 * Copies from host memory to a 1D CUDA array. \p dstArray and \p dstOffset
 * specify the CUDA array handle and starting offset in bytes of the destination
 * data.  \p pSrc specifies the base address of the source. \p ByteCount specifies
 * the number of bytes to copy.
 *
 * \param dstArray  - Destination array
 * \param dstOffset - Offset in bytes of destination array
 * \param srcHost   - Source host pointer
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyHtoA(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount);

/**
 * \brief Copies memory from Array to Host
 *
 * Copies from one 1D CUDA array to host memory. \p dstHost specifies the base
 * pointer of the destination. \p srcArray and \p srcOffset specify the CUDA
 * array handle and starting offset in bytes of the source data.
 * \p ByteCount specifies the number of bytes to copy.
 *
 * \param dstHost   - Destination device pointer
 * \param srcArray  - Source array
 * \param srcOffset - Offset in bytes of source array
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyAtoH(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount);

/**
 * \brief Copies memory from Array to Array
 *
 * Copies from one 1D CUDA array to another. \p dstArray and \p srcArray
 * specify the handles of the destination and source CUDA arrays for the copy,
 * respectively. \p dstOffset and \p srcOffset specify the destination and
 * source offsets in bytes into the CUDA arrays. \p ByteCount is the number of
 * bytes to be copied. The size of the elements in the CUDA arrays need not be
 * the same format, but the elements must be the same size; and count must be
 * evenly divisible by that size.
 *
 * \param dstArray  - Destination array
 * \param dstOffset - Offset in bytes of destination array
 * \param srcArray  - Source array
 * \param srcOffset - Offset in bytes of source array
 * \param ByteCount - Size of memory copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpyAtoA(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount);

/**
 * \brief Copies memory for 2D arrays
 *
 * Perform a 2D memory copy according to the parameters specified in \p pCopy.
 * The ::CUDA_MEMCPY2D structure is defined as:
 *
 * \code
   typedef struct CUDA_MEMCPY2D_st {
      unsigned int srcXInBytes, srcY;
      CUmemorytype srcMemoryType;
          const void *srcHost;
          CUdeviceptr srcDevice;
          CUarray srcArray;
          unsigned int srcPitch;

      unsigned int dstXInBytes, dstY;
      CUmemorytype dstMemoryType;
          void *dstHost;
          CUdeviceptr dstDevice;
          CUarray dstArray;
          unsigned int dstPitch;

      unsigned int WidthInBytes;
      unsigned int Height;
   } CUDA_MEMCPY2D;
 * \endcode
 * where:
 * - ::srcMemoryType and ::dstMemoryType specify the type of memory of the
 *   source and destination, respectively; ::CUmemorytype_enum is defined as:
 *
 * \code
   typedef enum CUmemorytype_enum {
      CU_MEMORYTYPE_HOST = 0x01,
      CU_MEMORYTYPE_DEVICE = 0x02,
      CU_MEMORYTYPE_ARRAY = 0x03,
      CU_MEMORYTYPE_UNIFIED = 0x04
   } CUmemorytype;
 * \endcode
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::srcDevice and ::srcPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::srcArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_HOST, ::srcHost and ::srcPitch
 * specify the (host) base address of the source data and the bytes per row to
 * apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_DEVICE, ::srcDevice and ::srcPitch
 * specify the (device) base address of the source data and the bytes per row
 * to apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_ARRAY, ::srcArray specifies the
 * handle of the source data. ::srcHost, ::srcDevice and ::srcPitch are
 * ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_HOST, ::dstHost and ::dstPitch
 * specify the (host) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::dstDevice and ::dstPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::dstArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_DEVICE, ::dstDevice and ::dstPitch
 * specify the (device) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_ARRAY, ::dstArray specifies the
 * handle of the destination data. ::dstHost, ::dstDevice and ::dstPitch are
 * ignored.
 *
 * - ::srcXInBytes and ::srcY specify the base address of the source data for
 *   the copy.
 *
 * \par
 * For host pointers, the starting address is
 * \code
  void* Start = (void*)((char*)srcHost+srcY*srcPitch + srcXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr Start = srcDevice+srcY*srcPitch+srcXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::srcXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::dstXInBytes and ::dstY specify the base address of the destination data
 *   for the copy.
 *
 * \par
 * For host pointers, the base address is
 * \code
  void* dstStart = (void*)((char*)dstHost+dstY*dstPitch + dstXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr dstStart = dstDevice+dstY*dstPitch+dstXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::dstXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::WidthInBytes and ::Height specify the width (in bytes) and height of
 *   the 2D copy being performed.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 *
 * \par
 * ::cuMemcpy2D() returns an error if any pitch is greater than the maximum
 * allowed (::CU_DEVICE_ATTRIBUTE_MAX_PITCH). ::cuMemAllocPitch() passes back
 * pitches that always work with ::cuMemcpy2D(). On intra-device memory copies
 * (device to device, CUDA array to device, CUDA array to CUDA array),
 * ::cuMemcpy2D() may fail for pitches not computed by ::cuMemAllocPitch().
 * ::cuMemcpy2DUnaligned() does not have this restriction, but may run
 * significantly slower in the cases where ::cuMemcpy2D() would have returned
 * an error code.
 *
 * \param pCopy - Parameters for the memory copy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpy2D(const CUDA_MEMCPY2D *pCopy);

/**
 * \brief Copies memory for 2D arrays
 *
 * Perform a 2D memory copy according to the parameters specified in \p pCopy.
 * The ::CUDA_MEMCPY2D structure is defined as:
 *
 * \code
   typedef struct CUDA_MEMCPY2D_st {
      unsigned int srcXInBytes, srcY;
      CUmemorytype srcMemoryType;
      const void *srcHost;
      CUdeviceptr srcDevice;
      CUarray srcArray;
      unsigned int srcPitch;
      unsigned int dstXInBytes, dstY;
      CUmemorytype dstMemoryType;
      void *dstHost;
      CUdeviceptr dstDevice;
      CUarray dstArray;
      unsigned int dstPitch;
      unsigned int WidthInBytes;
      unsigned int Height;
   } CUDA_MEMCPY2D;
 * \endcode
 * where:
 * - ::srcMemoryType and ::dstMemoryType specify the type of memory of the
 *   source and destination, respectively; ::CUmemorytype_enum is defined as:
 *
 * \code
   typedef enum CUmemorytype_enum {
      CU_MEMORYTYPE_HOST = 0x01,
      CU_MEMORYTYPE_DEVICE = 0x02,
      CU_MEMORYTYPE_ARRAY = 0x03,
      CU_MEMORYTYPE_UNIFIED = 0x04
   } CUmemorytype;
 * \endcode
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::srcDevice and ::srcPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::srcArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_HOST, ::srcHost and ::srcPitch
 * specify the (host) base address of the source data and the bytes per row to
 * apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_DEVICE, ::srcDevice and ::srcPitch
 * specify the (device) base address of the source data and the bytes per row
 * to apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_ARRAY, ::srcArray specifies the
 * handle of the source data. ::srcHost, ::srcDevice and ::srcPitch are
 * ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::dstDevice and ::dstPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::dstArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_HOST, ::dstHost and ::dstPitch
 * specify the (host) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_DEVICE, ::dstDevice and ::dstPitch
 * specify the (device) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_ARRAY, ::dstArray specifies the
 * handle of the destination data. ::dstHost, ::dstDevice and ::dstPitch are
 * ignored.
 *
 * - ::srcXInBytes and ::srcY specify the base address of the source data for
 *   the copy.
 *
 * \par
 * For host pointers, the starting address is
 * \code
  void* Start = (void*)((char*)srcHost+srcY*srcPitch + srcXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr Start = srcDevice+srcY*srcPitch+srcXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::srcXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::dstXInBytes and ::dstY specify the base address of the destination data
 *   for the copy.
 *
 * \par
 * For host pointers, the base address is
 * \code
  void* dstStart = (void*)((char*)dstHost+dstY*dstPitch + dstXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr dstStart = dstDevice+dstY*dstPitch+dstXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::dstXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::WidthInBytes and ::Height specify the width (in bytes) and height of
 *   the 2D copy being performed.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 *
 * \par
 * ::cuMemcpy2D() returns an error if any pitch is greater than the maximum
 * allowed (::CU_DEVICE_ATTRIBUTE_MAX_PITCH). ::cuMemAllocPitch() passes back
 * pitches that always work with ::cuMemcpy2D(). On intra-device memory copies
 * (device to device, CUDA array to device, CUDA array to CUDA array),
 * ::cuMemcpy2D() may fail for pitches not computed by ::cuMemAllocPitch().
 * ::cuMemcpy2DUnaligned() does not have this restriction, but may run
 * significantly slower in the cases where ::cuMemcpy2D() would have returned
 * an error code.
 *
 * \param pCopy - Parameters for the memory copy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy);

/**
 * \brief Copies memory for 3D arrays
 *
 * Perform a 3D memory copy according to the parameters specified in
 * \p pCopy. The ::CUDA_MEMCPY3D structure is defined as:
 *
 * \code
        typedef struct CUDA_MEMCPY3D_st {

            unsigned int srcXInBytes, srcY, srcZ;
            unsigned int srcLOD;
            CUmemorytype srcMemoryType;
                const void *srcHost;
                CUdeviceptr srcDevice;
                CUarray srcArray;
                unsigned int srcPitch;  // ignored when src is array
                unsigned int srcHeight; // ignored when src is array; may be 0 if Depth==1

            unsigned int dstXInBytes, dstY, dstZ;
            unsigned int dstLOD;
            CUmemorytype dstMemoryType;
                void *dstHost;
                CUdeviceptr dstDevice;
                CUarray dstArray;
                unsigned int dstPitch;  // ignored when dst is array
                unsigned int dstHeight; // ignored when dst is array; may be 0 if Depth==1

            unsigned int WidthInBytes;
            unsigned int Height;
            unsigned int Depth;
        } CUDA_MEMCPY3D;
 * \endcode
 * where:
 * - ::srcMemoryType and ::dstMemoryType specify the type of memory of the
 *   source and destination, respectively; ::CUmemorytype_enum is defined as:
 *
 * \code
   typedef enum CUmemorytype_enum {
      CU_MEMORYTYPE_HOST = 0x01,
      CU_MEMORYTYPE_DEVICE = 0x02,
      CU_MEMORYTYPE_ARRAY = 0x03,
      CU_MEMORYTYPE_UNIFIED = 0x04
   } CUmemorytype;
 * \endcode
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::srcDevice and ::srcPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::srcArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_HOST, ::srcHost, ::srcPitch and
 * ::srcHeight specify the (host) base address of the source data, the bytes
 * per row, and the height of each 2D slice of the 3D array. ::srcArray is
 * ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_DEVICE, ::srcDevice, ::srcPitch and
 * ::srcHeight specify the (device) base address of the source data, the bytes
 * per row, and the height of each 2D slice of the 3D array. ::srcArray is
 * ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_ARRAY, ::srcArray specifies the
 * handle of the source data. ::srcHost, ::srcDevice, ::srcPitch and
 * ::srcHeight are ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::dstDevice and ::dstPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::dstArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_HOST, ::dstHost and ::dstPitch
 * specify the (host) base address of the destination data, the bytes per row,
 * and the height of each 2D slice of the 3D array. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_DEVICE, ::dstDevice and ::dstPitch
 * specify the (device) base address of the destination data, the bytes per
 * row, and the height of each 2D slice of the 3D array. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_ARRAY, ::dstArray specifies the
 * handle of the destination data. ::dstHost, ::dstDevice, ::dstPitch and
 * ::dstHeight are ignored.
 *
 * - ::srcXInBytes, ::srcY and ::srcZ specify the base address of the source
 *   data for the copy.
 *
 * \par
 * For host pointers, the starting address is
 * \code
  void* Start = (void*)((char*)srcHost+(srcZ*srcHeight+srcY)*srcPitch + srcXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr Start = srcDevice+(srcZ*srcHeight+srcY)*srcPitch+srcXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::srcXInBytes must be evenly divisible by the array
 * element size.
 *
 * - dstXInBytes, ::dstY and ::dstZ specify the base address of the
 *   destination data for the copy.
 *
 * \par
 * For host pointers, the base address is
 * \code
  void* dstStart = (void*)((char*)dstHost+(dstZ*dstHeight+dstY)*dstPitch + dstXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr dstStart = dstDevice+(dstZ*dstHeight+dstY)*dstPitch+dstXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::dstXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::WidthInBytes, ::Height and ::Depth specify the width (in bytes), height
 *   and depth of the 3D copy being performed.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 * - If specified, ::srcHeight must be greater than or equal to ::Height +
 *   ::srcY, and ::dstHeight must be greater than or equal to ::Height + ::dstY.
 *
 * \par
 * ::cuMemcpy3D() returns an error if any pitch is greater than the maximum
 * allowed (::CU_DEVICE_ATTRIBUTE_MAX_PITCH).
 *
 * The ::srcLOD and ::dstLOD members of the ::CUDA_MEMCPY3D structure must be
 * set to 0.
 *
 * \param pCopy - Parameters for the memory copy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemcpy3D(const CUDA_MEMCPY3D *pCopy);
#endif /* __CUDA_API_VERSION >= 3020 */

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Copies memory between contexts
 *
 * Perform a 3D memory copy according to the parameters specified in
 * \p pCopy.  See the definition of the ::CUDA_MEMCPY3D_PEER structure
 * for documentation of its parameters.
 *
 * Note that this function is synchronous with respect to the host only if
 * the source or destination memory is of type ::CU_MEMORYTYPE_HOST.
 * Note also that this copy is serialized with respect all pending and future 
 * asynchronous work in to the current context, the copy's source context,
 * and the copy's destination context (use ::cuMemcpy3DPeerAsync to avoid 
 * this synchronization).
 *
 * \param pCopy - Parameters for the memory copy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuMemcpyDtoD, ::cuMemcpyPeer, ::cuMemcpyDtoDAsync, ::cuMemcpyPeerAsync,
 * ::cuMemcpy3DPeerAsync
 */
CUresult CUDAAPI cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy);

/**
 * \brief Copies memory asynchronously
 *
 * Copies data between two pointers. 
 * \p dst and \p src are base pointers of the destination and source, respectively.  
 * \p ByteCount specifies the number of bytes to copy.
 * Note that this function infers the type of the transfer (host to host, host to 
 *   device, device to device, or device to host) from the pointer values.  This
 *   function is only allowed in contexts which support unified addressing.
 * Note that this function is asynchronous and can optionally be associated to 
 *   a stream by passing a non-zero \p hStream argument
 *
 * \param dst       - Destination unified virtual address space pointer
 * \param src       - Source unified virtual address space pointer
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies device memory between two contexts asynchronously.
 *
 * Copies from device memory in one context to device memory in another
 * context. \p dstDevice is the base device pointer of the destination memory 
 * and \p dstContext is the destination context.  \p srcDevice is the base 
 * device pointer of the source memory and \p srcContext is the source pointer.  
 * \p ByteCount specifies the number of bytes to copy.  Note that this function
 * is asynchronous with respect to the host and all work in other streams in
 * other devices.
 *
 * \param dstDevice  - Destination device pointer
 * \param dstContext - Destination context
 * \param srcDevice  - Source device pointer
 * \param srcContext - Source context
 * \param ByteCount  - Size of memory copy in bytes
 * \param hStream    - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuMemcpyDtoD, ::cuMemcpyPeer, ::cuMemcpy3DPeer, ::cuMemcpyDtoDAsync, 
 * ::cuMemcpy3DPeerAsync
 */
CUresult CUDAAPI cuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream);
#endif /* __CUDA_API_VERSION >= 4000 */

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Copies memory from Host to Device
 *
 * Copies from host memory to device memory. \p dstDevice and \p srcHost are
 * the base addresses of the destination and source, respectively. \p ByteCount
 * specifies the number of bytes to copy.
 *
 * ::cuMemcpyHtoDAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p hStream argument. It only works on
 * page-locked memory and returns an error if a pointer to pageable memory is
 * passed as input.
 *
 * \param dstDevice - Destination device pointer
 * \param srcHost   - Source host pointer
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyHtoDAsync(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies memory from Device to Host
 *
 * Copies from device to host memory. \p dstHost and \p srcDevice specify the
 * base pointers of the destination and source, respectively. \p ByteCount
 * specifies the number of bytes to copy.
 *
 * ::cuMemcpyDtoHAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p hStream argument. It only works on
 * page-locked memory and returns an error if a pointer to pageable memory is
 * passed as input.
 *
 * \param dstHost   - Destination host pointer
 * \param srcDevice - Source device pointer
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyDtoHAsync(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies memory from Device to Device
 *
 * Copies from device memory to device memory. \p dstDevice and \p srcDevice
 * are the base pointers of the destination and source, respectively.
 * \p ByteCount specifies the number of bytes to copy. Note that this function
 * is asynchronous and can optionally be associated to a stream by passing a
 * non-zero \p hStream argument
 *
 * \param dstDevice - Destination device pointer
 * \param srcDevice - Source device pointer
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyDtoDAsync(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies memory from Host to Array
 *
 * Copies from host memory to a 1D CUDA array. \p dstArray and \p dstOffset
 * specify the CUDA array handle and starting offset in bytes of the
 * destination data. \p srcHost specifies the base address of the source.
 * \p ByteCount specifies the number of bytes to copy.
 *
 * ::cuMemcpyHtoAAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p hStream argument. It only works on
 * page-locked memory and returns an error if a pointer to pageable memory is
 * passed as input.
 *
 * \param dstArray  - Destination array
 * \param dstOffset - Offset in bytes of destination array
 * \param srcHost   - Source host pointer
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyHtoAAsync(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies memory from Array to Host
 *
 * Copies from one 1D CUDA array to host memory. \p dstHost specifies the base
 * pointer of the destination. \p srcArray and \p srcOffset specify the CUDA
 * array handle and starting offset in bytes of the source data.
 * \p ByteCount specifies the number of bytes to copy.
 *
 * ::cuMemcpyAtoHAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument. It only works on
 * page-locked host memory and returns an error if a pointer to pageable
 * memory is passed as input.
 *
 * \param dstHost   - Destination pointer
 * \param srcArray  - Source array
 * \param srcOffset - Offset in bytes of source array
 * \param ByteCount - Size of memory copy in bytes
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpyAtoHAsync(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream);

/**
 * \brief Copies memory for 2D arrays
 *
 * Perform a 2D memory copy according to the parameters specified in \p pCopy.
 * The ::CUDA_MEMCPY2D structure is defined as:
 *
 * \code
   typedef struct CUDA_MEMCPY2D_st {
      unsigned int srcXInBytes, srcY;
      CUmemorytype srcMemoryType;
      const void *srcHost;
      CUdeviceptr srcDevice;
      CUarray srcArray;
      unsigned int srcPitch;
      unsigned int dstXInBytes, dstY;
      CUmemorytype dstMemoryType;
      void *dstHost;
      CUdeviceptr dstDevice;
      CUarray dstArray;
      unsigned int dstPitch;
      unsigned int WidthInBytes;
      unsigned int Height;
   } CUDA_MEMCPY2D;
 * \endcode
 * where:
 * - ::srcMemoryType and ::dstMemoryType specify the type of memory of the
 *   source and destination, respectively; ::CUmemorytype_enum is defined as:
 *
 * \code
   typedef enum CUmemorytype_enum {
      CU_MEMORYTYPE_HOST = 0x01,
      CU_MEMORYTYPE_DEVICE = 0x02,
      CU_MEMORYTYPE_ARRAY = 0x03,
      CU_MEMORYTYPE_UNIFIED = 0x04
   } CUmemorytype;
 * \endcode
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_HOST, ::srcHost and ::srcPitch
 * specify the (host) base address of the source data and the bytes per row to
 * apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::srcDevice and ::srcPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::srcArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_DEVICE, ::srcDevice and ::srcPitch
 * specify the (device) base address of the source data and the bytes per row
 * to apply. ::srcArray is ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_ARRAY, ::srcArray specifies the
 * handle of the source data. ::srcHost, ::srcDevice and ::srcPitch are
 * ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::dstDevice and ::dstPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::dstArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_HOST, ::dstHost and ::dstPitch
 * specify the (host) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_DEVICE, ::dstDevice and ::dstPitch
 * specify the (device) base address of the destination data and the bytes per
 * row to apply. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_ARRAY, ::dstArray specifies the
 * handle of the destination data. ::dstHost, ::dstDevice and ::dstPitch are
 * ignored.
 *
 * - ::srcXInBytes and ::srcY specify the base address of the source data for
 *   the copy.
 *
 * \par
 * For host pointers, the starting address is
 * \code
  void* Start = (void*)((char*)srcHost+srcY*srcPitch + srcXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr Start = srcDevice+srcY*srcPitch+srcXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::srcXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::dstXInBytes and ::dstY specify the base address of the destination data
 *   for the copy.
 *
 * \par
 * For host pointers, the base address is
 * \code
  void* dstStart = (void*)((char*)dstHost+dstY*dstPitch + dstXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr dstStart = dstDevice+dstY*dstPitch+dstXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::dstXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::WidthInBytes and ::Height specify the width (in bytes) and height of
 *   the 2D copy being performed.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 * - If specified, ::srcHeight must be greater than or equal to ::Height +
 *   ::srcY, and ::dstHeight must be greater than or equal to ::Height + ::dstY.
 *
 * \par
 * ::cuMemcpy2D() returns an error if any pitch is greater than the maximum
 * allowed (::CU_DEVICE_ATTRIBUTE_MAX_PITCH). ::cuMemAllocPitch() passes back
 * pitches that always work with ::cuMemcpy2D(). On intra-device memory copies
 * (device to device, CUDA array to device, CUDA array to CUDA array),
 * ::cuMemcpy2D() may fail for pitches not computed by ::cuMemAllocPitch().
 * ::cuMemcpy2DUnaligned() does not have this restriction, but may run
 * significantly slower in the cases where ::cuMemcpy2D() would have returned
 * an error code.
 *
 * ::cuMemcpy2DAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p hStream argument. It only works on
 * page-locked host memory and returns an error if a pointer to pageable
 * memory is passed as input.
 *
 * \param pCopy   - Parameters for the memory copy
 * \param hStream - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream);

/**
 * \brief Copies memory for 3D arrays
 *
 * Perform a 3D memory copy according to the parameters specified in
 * \p pCopy. The ::CUDA_MEMCPY3D structure is defined as:
 *
 * \code
        typedef struct CUDA_MEMCPY3D_st {

            unsigned int srcXInBytes, srcY, srcZ;
            unsigned int srcLOD;
            CUmemorytype srcMemoryType;
                const void *srcHost;
                CUdeviceptr srcDevice;
                CUarray srcArray;
                unsigned int srcPitch;  // ignored when src is array
                unsigned int srcHeight; // ignored when src is array; may be 0 if Depth==1

            unsigned int dstXInBytes, dstY, dstZ;
            unsigned int dstLOD;
            CUmemorytype dstMemoryType;
                void *dstHost;
                CUdeviceptr dstDevice;
                CUarray dstArray;
                unsigned int dstPitch;  // ignored when dst is array
                unsigned int dstHeight; // ignored when dst is array; may be 0 if Depth==1

            unsigned int WidthInBytes;
            unsigned int Height;
            unsigned int Depth;
        } CUDA_MEMCPY3D;
 * \endcode
 * where:
 * - ::srcMemoryType and ::dstMemoryType specify the type of memory of the
 *   source and destination, respectively; ::CUmemorytype_enum is defined as:
 *
 * \code
   typedef enum CUmemorytype_enum {
      CU_MEMORYTYPE_HOST = 0x01,
      CU_MEMORYTYPE_DEVICE = 0x02,
      CU_MEMORYTYPE_ARRAY = 0x03,
      CU_MEMORYTYPE_UNIFIED = 0x04
   } CUmemorytype;
 * \endcode
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::srcDevice and ::srcPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::srcArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_HOST, ::srcHost, ::srcPitch and
 * ::srcHeight specify the (host) base address of the source data, the bytes
 * per row, and the height of each 2D slice of the 3D array. ::srcArray is
 * ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_DEVICE, ::srcDevice, ::srcPitch and
 * ::srcHeight specify the (device) base address of the source data, the bytes
 * per row, and the height of each 2D slice of the 3D array. ::srcArray is
 * ignored.
 *
 * \par
 * If ::srcMemoryType is ::CU_MEMORYTYPE_ARRAY, ::srcArray specifies the
 * handle of the source data. ::srcHost, ::srcDevice, ::srcPitch and
 * ::srcHeight are ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_UNIFIED, ::dstDevice and ::dstPitch
 *   specify the (unified virtual address space) base address of the source data 
 *   and the bytes per row to apply.  ::dstArray is ignored.  
 * This value may be used only if unified addressing is supported in the calling 
 *   context.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_HOST, ::dstHost and ::dstPitch
 * specify the (host) base address of the destination data, the bytes per row,
 * and the height of each 2D slice of the 3D array. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_DEVICE, ::dstDevice and ::dstPitch
 * specify the (device) base address of the destination data, the bytes per
 * row, and the height of each 2D slice of the 3D array. ::dstArray is ignored.
 *
 * \par
 * If ::dstMemoryType is ::CU_MEMORYTYPE_ARRAY, ::dstArray specifies the
 * handle of the destination data. ::dstHost, ::dstDevice, ::dstPitch and
 * ::dstHeight are ignored.
 *
 * - ::srcXInBytes, ::srcY and ::srcZ specify the base address of the source
 *   data for the copy.
 *
 * \par
 * For host pointers, the starting address is
 * \code
  void* Start = (void*)((char*)srcHost+(srcZ*srcHeight+srcY)*srcPitch + srcXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr Start = srcDevice+(srcZ*srcHeight+srcY)*srcPitch+srcXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::srcXInBytes must be evenly divisible by the array
 * element size.
 *
 * - dstXInBytes, ::dstY and ::dstZ specify the base address of the
 *   destination data for the copy.
 *
 * \par
 * For host pointers, the base address is
 * \code
  void* dstStart = (void*)((char*)dstHost+(dstZ*dstHeight+dstY)*dstPitch + dstXInBytes);
 * \endcode
 *
 * \par
 * For device pointers, the starting address is
 * \code
  CUdeviceptr dstStart = dstDevice+(dstZ*dstHeight+dstY)*dstPitch+dstXInBytes;
 * \endcode
 *
 * \par
 * For CUDA arrays, ::dstXInBytes must be evenly divisible by the array
 * element size.
 *
 * - ::WidthInBytes, ::Height and ::Depth specify the width (in bytes), height
 *   and depth of the 3D copy being performed.
 * - If specified, ::srcPitch must be greater than or equal to ::WidthInBytes +
 *   ::srcXInBytes, and ::dstPitch must be greater than or equal to
 *   ::WidthInBytes + dstXInBytes.
 * - If specified, ::srcHeight must be greater than or equal to ::Height +
 *   ::srcY, and ::dstHeight must be greater than or equal to ::Height + ::dstY.
 *
 * \par
 * ::cuMemcpy3D() returns an error if any pitch is greater than the maximum
 * allowed (::CU_DEVICE_ATTRIBUTE_MAX_PITCH).
 *
 * ::cuMemcpy3DAsync() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p hStream argument. It only works on
 * page-locked host memory and returns an error if a pointer to pageable
 * memory is passed as input.
 *
 * The ::srcLOD and ::dstLOD members of the ::CUDA_MEMCPY3D structure must be
 * set to 0.
 *
 * \param pCopy - Parameters for the memory copy
 * \param hStream - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream);
#endif /* __CUDA_API_VERSION >= 3020 */

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Copies memory between contexts asynchronously.
 *
 * Perform a 3D memory copy according to the parameters specified in
 * \p pCopy.  See the definition of the ::CUDA_MEMCPY3D_PEER structure
 * for documentation of its parameters.
 *
 * \param pCopy - Parameters for the memory copy
 * \param hStream - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuMemcpyDtoD, ::cuMemcpyPeer, ::cuMemcpyDtoDAsync, ::cuMemcpyPeerAsync,
 * ::cuMemcpy3DPeerAsync
 */
CUresult CUDAAPI cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream);
#endif /* __CUDA_API_VERSION >= 4000 */

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Initializes device memory
 *
 * Sets the memory range of \p N 8-bit values to the specified value
 * \p uc.
 *
 * \param dstDevice - Destination device pointer
 * \param uc        - Value to set
 * \param N         - Number of elements
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD8(CUdeviceptr dstDevice, unsigned char uc, size_t N);

/**
 * \brief Initializes device memory
 *
 * Sets the memory range of \p N 16-bit values to the specified value
 * \p us.
 *
 * \param dstDevice - Destination device pointer
 * \param us        - Value to set
 * \param N         - Number of elements
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD16(CUdeviceptr dstDevice, unsigned short us, size_t N);

/**
 * \brief Initializes device memory
 *
 * Sets the memory range of \p N 32-bit values to the specified value
 * \p ui.
 *
 * \param dstDevice - Destination device pointer
 * \param ui        - Value to set
 * \param N         - Number of elements
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD32(CUdeviceptr dstDevice, unsigned int ui, size_t N);

/**
 * \brief Initializes device memory
 *
 * Sets the 2D memory range of \p Width 8-bit values to the specified value
 * \p uc. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param uc        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D8(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height);

/**
 * \brief Initializes device memory
 *
 * Sets the 2D memory range of \p Width 16-bit values to the specified value
 * \p us. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param us        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D16(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height);

/**
 * \brief Initializes device memory
 *
 * Sets the 2D memory range of \p Width 32-bit values to the specified value
 * \p ui. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param ui        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D32(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height);

/**
 * \brief Sets device memory
 *
 * Sets the memory range of \p N 8-bit values to the specified value
 * \p uc.
 *
 * ::cuMemsetD8Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param uc        - Value to set
 * \param N         - Number of elements
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream);

/**
 * \brief Sets device memory
 *
 * Sets the memory range of \p N 16-bit values to the specified value
 * \p us.
 *
 * ::cuMemsetD16Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param us        - Value to set
 * \param N         - Number of elements
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream);

/**
 * \brief Sets device memory
 *
 * Sets the memory range of \p N 32-bit values to the specified value
 * \p ui.
 *
 * ::cuMemsetD32Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param ui        - Value to set
 * \param N         - Number of elements
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async, ::cuMemsetD32
 */
CUresult CUDAAPI cuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream);

/**
 * \brief Sets device memory
 *
 * Sets the 2D memory range of \p Width 8-bit values to the specified value
 * \p uc. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * ::cuMemsetD2D8Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param uc        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream);

/**
 * \brief Sets device memory
 *
 * Sets the 2D memory range of \p Width 16-bit values to the specified value
 * \p us. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * ::cuMemsetD2D16Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param us        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D32, ::cuMemsetD2D32Async,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream);

/**
 * \brief Sets device memory
 *
 * Sets the 2D memory range of \p Width 32-bit values to the specified value
 * \p ui. \p Height specifies the number of rows to set, and \p dstPitch
 * specifies the number of bytes between each row. This function performs
 * fastest when the pitch is one that has been passed back by
 * ::cuMemAllocPitch().
 *
 * ::cuMemsetD2D32Async() is asynchronous and can optionally be associated to a
 * stream by passing a non-zero \p stream argument.
 *
 * \param dstDevice - Destination device pointer
 * \param dstPitch  - Pitch of destination device pointer
 * \param ui        - Value to set
 * \param Width     - Width of row
 * \param Height    - Number of rows
 * \param hStream   - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D8Async,
 * ::cuMemsetD2D16, ::cuMemsetD2D16Async, ::cuMemsetD2D32,
 * ::cuMemsetD8, ::cuMemsetD8Async, ::cuMemsetD16, ::cuMemsetD16Async,
 * ::cuMemsetD32, ::cuMemsetD32Async
 */
CUresult CUDAAPI cuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream);

/**
 * \brief Creates a 1D or 2D CUDA array
 *
 * Creates a CUDA array according to the ::CUDA_ARRAY_DESCRIPTOR structure
 * \p pAllocateArray and returns a handle to the new CUDA array in \p *pHandle.
 * The ::CUDA_ARRAY_DESCRIPTOR is defined as:
 *
 * \code
    typedef struct {
        unsigned int Width;
        unsigned int Height;
        CUarray_format Format;
        unsigned int NumChannels;
    } CUDA_ARRAY_DESCRIPTOR;
 * \endcode
 * where:
 *
 * - \p Width, and \p Height are the width, and height of the CUDA array (in
 * elements); the CUDA array is one-dimensional if height is 0, two-dimensional
 * otherwise;
 * - ::Format specifies the format of the elements; ::CUarray_format is
 * defined as:
 * \code
    typedef enum CUarray_format_enum {
        CU_AD_FORMAT_UNSIGNED_INT8 = 0x01,
        CU_AD_FORMAT_UNSIGNED_INT16 = 0x02,
        CU_AD_FORMAT_UNSIGNED_INT32 = 0x03,
        CU_AD_FORMAT_SIGNED_INT8 = 0x08,
        CU_AD_FORMAT_SIGNED_INT16 = 0x09,
        CU_AD_FORMAT_SIGNED_INT32 = 0x0a,
        CU_AD_FORMAT_HALF = 0x10,
        CU_AD_FORMAT_FLOAT = 0x20
    } CUarray_format;
 *  \endcode
 * - \p NumChannels specifies the number of packed components per CUDA array
 * element; it may be 1, 2, or 4;
 *
 * Here are examples of CUDA array descriptions:
 *
 * Description for a CUDA array of 2048 floats:
 * \code
    CUDA_ARRAY_DESCRIPTOR desc;
    desc.Format = CU_AD_FORMAT_FLOAT;
    desc.NumChannels = 1;
    desc.Width = 2048;
    desc.Height = 1;
 * \endcode
 *
 * Description for a 64 x 64 CUDA array of floats:
 * \code
    CUDA_ARRAY_DESCRIPTOR desc;
    desc.Format = CU_AD_FORMAT_FLOAT;
    desc.NumChannels = 1;
    desc.Width = 64;
    desc.Height = 64;
 * \endcode
 *
 * Description for a \p width x \p height CUDA array of 64-bit, 4x16-bit
 * float16's:
 * \code
    CUDA_ARRAY_DESCRIPTOR desc;
    desc.FormatFlags = CU_AD_FORMAT_HALF;
    desc.NumChannels = 4;
    desc.Width = width;
    desc.Height = height;
 * \endcode
 *
 * Description for a \p width x \p height CUDA array of 16-bit elements, each
 * of which is two 8-bit unsigned chars:
 * \code
    CUDA_ARRAY_DESCRIPTOR arrayDesc;
    desc.FormatFlags = CU_AD_FORMAT_UNSIGNED_INT8;
    desc.NumChannels = 2;
    desc.Width = width;
    desc.Height = height;
 * \endcode
 *
 * \param pHandle        - Returned array
 * \param pAllocateArray - Array descriptor
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuArrayCreate(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray);

/**
 * \brief Get a 1D or 2D CUDA array descriptor
 *
 * Returns in \p *pArrayDescriptor a descriptor containing information on the
 * format and dimensions of the CUDA array \p hArray. It is useful for
 * subroutines that have been passed a CUDA array, but need to know the CUDA
 * array parameters for validation or other purposes.
 *
 * \param pArrayDescriptor - Returned array descriptor
 * \param hArray           - Array to get descriptor of
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuArrayGetDescriptor(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
#endif /* __CUDA_API_VERSION >= 3020 */


/**
 * \brief Destroys a CUDA array
 *
 * Destroys the CUDA array \p hArray.
 *
 * \param hArray - Array to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_ARRAY_IS_MAPPED
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuArrayDestroy(CUarray hArray);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Creates a 3D CUDA array
 *
 * Creates a CUDA array according to the ::CUDA_ARRAY3D_DESCRIPTOR structure
 * \p pAllocateArray and returns a handle to the new CUDA array in \p *pHandle.
 * The ::CUDA_ARRAY3D_DESCRIPTOR is defined as:
 *
 * \code
    typedef struct {
        unsigned int Width;
        unsigned int Height;
        unsigned int Depth;
        CUarray_format Format;
        unsigned int NumChannels;
        unsigned int Flags;
    } CUDA_ARRAY3D_DESCRIPTOR;
 * \endcode
 * where:
 *
 * - \p Width, \p Height, and \p Depth are the width, height, and depth of the
 * CUDA array (in elements); the CUDA array is one-dimensional if height and
 * depth are 0, two-dimensional if depth is 0, and three-dimensional otherwise;
 * If the ::CUDA_ARRAY3D_LAYERED flag is set, then the CUDA array is a collection of
 * layers, where \p Depth indicates the number of layers. Each layer is a 1D array 
 * if \p Height is 0, and a 2D array otherwise.
 * - ::Format specifies the format of the elements; ::CUarray_format is
 * defined as:
 * \code
    typedef enum CUarray_format_enum {
        CU_AD_FORMAT_UNSIGNED_INT8 = 0x01,
        CU_AD_FORMAT_UNSIGNED_INT16 = 0x02,
        CU_AD_FORMAT_UNSIGNED_INT32 = 0x03,
        CU_AD_FORMAT_SIGNED_INT8 = 0x08,
        CU_AD_FORMAT_SIGNED_INT16 = 0x09,
        CU_AD_FORMAT_SIGNED_INT32 = 0x0a,
        CU_AD_FORMAT_HALF = 0x10,
        CU_AD_FORMAT_FLOAT = 0x20
    } CUarray_format;
 *  \endcode
 * - \p NumChannels specifies the number of packed components per CUDA array
 * element; it may be 1, 2, or 4;
 * - ::Flags may be set to 
 *   - ::CUDA_ARRAY3D_LAYERED to enable creation of layered CUDA arrays. If this flag is set, 
 *     \p Depth specifies the number of layers, not the depth of a 3D array.
 *   - ::CUDA_ARRAY3D_SURFACE_LDST to enable surface references to be bound to the CUDA array.  
 *     If this flag is not set, ::cuSurfRefSetArray will fail when attempting to bind the CUDA array 
 *     to a surface reference.
 *
 * Here are examples of CUDA array descriptions:
 *
 * Description for a CUDA array of 2048 floats:
 * \code
    CUDA_ARRAY3D_DESCRIPTOR desc;
    desc.Format = CU_AD_FORMAT_FLOAT;
    desc.NumChannels = 1;
    desc.Width = 2048;
    desc.Height = 0;
    desc.Depth = 0;
 * \endcode
 *
 * Description for a 64 x 64 CUDA array of floats:
 * \code
    CUDA_ARRAY3D_DESCRIPTOR desc;
    desc.Format = CU_AD_FORMAT_FLOAT;
    desc.NumChannels = 1;
    desc.Width = 64;
    desc.Height = 64;
    desc.Depth = 0;
 * \endcode
 *
 * Description for a \p width x \p height x \p depth CUDA array of 64-bit,
 * 4x16-bit float16's:
 * \code
    CUDA_ARRAY3D_DESCRIPTOR desc;
    desc.FormatFlags = CU_AD_FORMAT_HALF;
    desc.NumChannels = 4;
    desc.Width = width;
    desc.Height = height;
    desc.Depth = depth;
 * \endcode
 *
 * \param pHandle        - Returned array
 * \param pAllocateArray - 3D array descriptor
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa ::cuArray3DGetDescriptor, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuArray3DCreate(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray);

/**
 * \brief Get a 3D CUDA array descriptor
 *
 * Returns in \p *pArrayDescriptor a descriptor containing information on the
 * format and dimensions of the CUDA array \p hArray. It is useful for
 * subroutines that have been passed a CUDA array, but need to know the CUDA
 * array parameters for validation or other purposes.
 *
 * This function may be called on 1D and 2D arrays, in which case the \p Height
 * and/or \p Depth members of the descriptor struct will be set to 0.
 *
 * \param pArrayDescriptor - Returned 3D array descriptor
 * \param hArray           - 3D array to get descriptor of
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE
 * \notefnerr
 *
 * \sa ::cuArray3DCreate, ::cuArrayCreate,
 * ::cuArrayDestroy, ::cuArrayGetDescriptor, ::cuMemAlloc, ::cuMemAllocHost,
 * ::cuMemAllocPitch, ::cuMemcpy2D, ::cuMemcpy2DAsync, ::cuMemcpy2DUnaligned,
 * ::cuMemcpy3D, ::cuMemcpy3DAsync, ::cuMemcpyAtoA, ::cuMemcpyAtoD,
 * ::cuMemcpyAtoH, ::cuMemcpyAtoHAsync, ::cuMemcpyDtoA, ::cuMemcpyDtoD, ::cuMemcpyDtoDAsync,
 * ::cuMemcpyDtoH, ::cuMemcpyDtoHAsync, ::cuMemcpyHtoA, ::cuMemcpyHtoAAsync,
 * ::cuMemcpyHtoD, ::cuMemcpyHtoDAsync, ::cuMemFree, ::cuMemFreeHost,
 * ::cuMemGetAddressRange, ::cuMemGetInfo, ::cuMemHostAlloc,
 * ::cuMemHostGetDevicePointer, ::cuMemsetD2D8, ::cuMemsetD2D16,
 * ::cuMemsetD2D32, ::cuMemsetD8, ::cuMemsetD16, ::cuMemsetD32
 */
CUresult CUDAAPI cuArray3DGetDescriptor(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
#endif /* __CUDA_API_VERSION >= 3020 */

/** @} */ /* END CUDA_MEM */

/**
 * \defgroup CUDA_UNIFIED Unified Addressing
 *
 * This section describes the unified addressing functions of the 
 * low-level CUDA driver application programming interface.
 *
 * @{
 *
 * \section CUDA_UNIFIED_overview Overview
 *
 * CUDA devices can share a unified address space with the host.  
 * For these devices there is no distinction between a device
 * pointer and a host pointer -- the same pointer value may be 
 * used to access memory from the host program and from a kernel 
 * running on the device (with exceptions enumerated below).
 *
 * \section CUDA_UNIFIED_support Supported Platforms
 * 
 * Whether or not a device supports unified addressing may be 
 * queried by calling ::cuDeviceGetAttribute() with the device 
 * attribute ::CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING.
 *
 * Unified addressing is automatically enabled in 64-bit processes 
 * on devices with compute capability greater than or equal to 2.0.
 *
 * Unified addressing is not yet supported on Windows Vista or
 * Windows 7 for devices that do not use the TCC driver model.
 *
 * \section CUDA_UNIFIED_lookup Looking Up Information from Pointer Values
 *
 * It is possible to look up information about the memory which backs a 
 * pointer value.  For instance, one may want to know if a pointer points
 * to host or device memory.  As another example, in the case of device 
 * memory, one may want to know on which CUDA device the memory 
 * resides.  These properties may be queried using the function 
 * ::cuPointerGetAttribute()
 *
 * Because pointers are unique, it is not necessary to specify information
 * about the pointers specified to the various copy functions in the 
 * CUDA API.  The function ::cuMemcpy() may be used to perform a copy
 * between two pointers, ignoring whether they point to host or device
 * memory (making ::cuMemcpyHtoD(), ::cuMemcpyDtoD(), and ::cuMemcpyDtoH()
 * unnecessary for devices supporting unified addressing).  For 
 * multidimensional copies, the memory type ::CU_MEMORYTYPE_UNIFIED may be
 * used to specify that the CUDA driver should infer the location of the
 * pointer from its value.
 *
 * \section CUDA_UNIFIED_automaphost Automatic Mapping of Host Allocated Host Memory
 *
 * All host memory allocated in all contexts using ::cuMemAllocHost() and
 * ::cuMemHostAlloc() is always directly accessible from all contexts on
 * all devices that support unified addressing.  This is the case regardless 
 * of whether or not the flags ::CU_MEMHOSTALLOC_PORTABLE and
 * ::CU_MEMHOSTALLOC_DEVICEMAP are specified.
 *
 * The pointer value through which allocated host memory may be accessed 
 * in kernels on all devices that support unified addressing is the same 
 * as the pointer value through which that memory is accessed on the host,
 * so it is not necessary to call ::cuMemHostGetDevicePointer() to get the device 
 * pointer for these allocations.
 * 
 * Note that this is not the case for memory allocated using the flag
 * ::CU_MEMHOSTALLOC_WRITECOMBINED, as discussed below.
 *
 * \section CUDA_UNIFIED_autopeerregister Automatic Registration of Peer Memory
 *
 * Upon enabling direct access from a context that supports unified addressing 
 * to another peer context that supports unified addressing using 
 * ::cuCtxEnablePeerAccess() all memory allocated in the peer context using 
 * ::cuMemAlloc() and ::cuMemAllocPitch() will immediately be accessible 
 * by the current context.  The device pointer value through
 * which any peer memory may be accessed in the current context
 * is the same pointer value through which that memory may be
 * accessed in the peer context.
 *
 * \section CUDA_UNIFIED_exceptions Exceptions, Disjoint Addressing
 * 
 * Not all memory may be accessed on devices through the same pointer
 * value through which they are accessed on the host.  These exceptions
 * are host memory registered using ::cuMemHostRegister() and host memory
 * allocated using the flag ::CU_MEMHOSTALLOC_WRITECOMBINED.  For these 
 * exceptions, there exists a distinct host and device address for the
 * memory.  The device address is guaranteed to not overlap any valid host
 * pointer range and is guaranteed to have the same value across all 
 * contexts that support unified addressing.  
 * 
 * This device address may be queried using ::cuMemHostGetDevicePointer() 
 * when a context using unified addressing is current.  Either the host 
 * or the unified device pointer value may be used to refer to this memory 
 * through ::cuMemcpy() and similar functions using the 
 * ::CU_MEMORYTYPE_UNIFIED memory type.
 *
 */

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Returns information about a pointer
 * 
 * The supported attributes are:
 * 
 * - ::CU_POINTER_ATTRIBUTE_CONTEXT: 
 * 
 *      Returns in \p *data the ::CUcontext in which \p ptr was allocated or 
 *      registered.   
 *      The type of \p data must be ::CUcontext *.  
 *      
 *      If \p ptr was not allocated by, mapped by, or registered with
 *      a ::CUcontext which uses unified virtual addressing then 
 *      ::CUDA_ERROR_INVALID_VALUE is returned.
 * 
 * - ::CU_POINTER_ATTRIBUTE_MEMORY_TYPE: 
 *    
 *      Returns in \p *data the physical memory type of the memory that 
 *      \p ptr addresses as a ::CUmemorytype enumerated value.
 *      The type of \p data must be unsigned int.
 *      
 *      If \p ptr addresses device memory then \p *data is set to 
 *      ::CU_MEMORYTYPE_DEVICE.  The particular ::CUdevice on which the 
 *      memory resides is the ::CUdevice of the ::CUcontext returned by the 
 *      ::CU_POINTER_ATTRIBUTE_CONTEXT attribute of \p ptr.
 *      
 *      If \p ptr addresses host memory then \p *data is set to 
 *      ::CU_MEMORYTYPE_HOST.
 *      
 *      If \p ptr was not allocated by, mapped by, or registered with
 *      a ::CUcontext which uses unified virtual addressing then 
 *      ::CUDA_ERROR_INVALID_VALUE is returned.
 *
 *      If the current ::CUcontext does not support unified virtual 
 *      addressing then ::CUDA_ERROR_INVALID_CONTEXT is returned.
 *    
 * - ::CU_POINTER_ATTRIBUTE_DEVICE_POINTER:
 * 
 *      Returns in \p *data the device pointer value through which
 *      \p ptr may be accessed by kernels running in the current 
 *      ::CUcontext.
 *      The type of \p data must be CUdeviceptr *.
 * 
 *      If there exists no device pointer value through which
 *      kernels running in the current ::CUcontext may access
 *      \p ptr then ::CUDA_ERROR_INVALID_VALUE is returned.
 * 
 *      If there is no current ::CUcontext then 
 *      ::CUDA_ERROR_INVALID_CONTEXT is returned.
 *      
 *      Except in the exceptional disjoint addressing cases discussed 
 *      below, the value returned in \p *data will equal the input 
 *      value \p ptr.
 * 
 * - ::CU_POINTER_ATTRIBUTE_HOST_POINTER:
 * 
 *      Returns in \p *data the host pointer value through which 
 *      \p ptr may be accessed by by the host program.
 *      The type of \p data must be void **.
 *      If there exists no host pointer value through which
 *      the host program may directly access \p ptr then 
 *      ::CUDA_ERROR_INVALID_VALUE is returned.
 * 
 *      Except in the exceptional disjoint addressing cases discussed 
 *      below, the value returned in \p *data will equal the input 
 *      value \p ptr.
 * 
 *
 * \par
 *
 * Note that for most allocations in the unified virtual address space
 * the host and device pointer for accessing the allocation will be the 
 * same.  The exceptions to this are
 *  - user memory registered using ::cuMemHostRegister 
 *  - host memory allocated using ::cuMemHostAlloc with the 
 *    ::CU_MEMHOSTALLOC_WRITECOMBINED flag
 * For these types of allocation there will exist separate, disjoint host 
 * and device addresses for accessing the allocation.  In particular 
 *  - The host address will correspond to an invalid unmapped device address 
 *    (which will result in an exception if accessed from the device) 
 *  - The device address will correspond to an invalid unmapped host address 
 *    (which will result in an exception if accessed from the host).
 * For these types of allocations, querying ::CU_POINTER_ATTRIBUTE_HOST_POINTER 
 * and ::CU_POINTER_ATTRIBUTE_DEVICE_POINTER may be used to retrieve the host 
 * and device addresses from either address.
 *
 * \param data      - Returned pointer attribute value
 * \param attribute - Pointer attribute to query
 * \param ptr       - Pointer
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuMemAlloc,
 * ::cuMemFree,
 * ::cuMemAllocHost,
 * ::cuMemFreeHost,
 * ::cuMemHostAlloc,
 * ::cuMemHostRegister,
 * ::cuMemHostUnregister
 */
CUresult CUDAAPI cuPointerGetAttribute(void *data, CUpointer_attribute attribute, CUdeviceptr ptr);
#endif /* __CUDA_API_VERSION >= 4000 */

/** @} */ /* END CUDA_UNIFIED */

/**
 * \defgroup CUDA_STREAM Stream Management
 *
 * This section describes the stream management functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

/**
 * \brief Create a stream
 *
 * Creates a stream and returns a handle in \p phStream. \p Flags is required
 * to be 0.
 *
 * \param phStream - Returned newly created stream
 * \param Flags    - Parameters for stream creation (must be 0)
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa ::cuStreamDestroy,
 * ::cuStreamWaitEvent,
 * ::cuStreamQuery,
 * ::cuStreamSynchronize
 */
CUresult CUDAAPI cuStreamCreate(CUstream *phStream, unsigned int Flags);

/**
 * \brief Make a compute stream wait on an event
 *
 * Makes all future work submitted to \p hStream wait until \p hEvent
 * reports completion before beginning execution.  This synchronization
 * will be performed efficiently on the device.  The event \p hEvent may
 * be from a different context than \p hStream, in which case this function
 * will perform cross-device synchronization.
 *
 * The stream \p hStream will wait only for the completion of the most recent
 * host call to ::cuEventRecord() on \p hEvent.  Once this call has returned,
 * any functions (including ::cuEventRecord() and ::cuEventDestroy()) may be
 * called on \p hEvent again, and the subsequent calls will not have any
 * effect on \p hStream.
 *
 * If \p hStream is 0 (the NULL stream) any future work submitted in any stream
 * will wait for \p hEvent to complete before beginning execution. This
 * effectively creates a barrier for all future work submitted to the context.
 *
 * If ::cuEventRecord() has not been called on \p hEvent, this call acts as if
 * the record has already completed, and so is a functional no-op.
 *
 * \param hStream - Stream to wait
 * \param hEvent  - Event to wait on (may not be NULL)
 * \param Flags   - Parameters for the operation (must be 0)
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * \notefnerr
 *
 * \sa ::cuStreamCreate,
 * ::cuEventRecord,
 * ::cuStreamQuery,
 * ::cuStreamSynchronize,
 * ::cuStreamDestroy
 */
CUresult CUDAAPI cuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags);

/**
 * \brief Determine status of a compute stream
 *
 * Returns ::CUDA_SUCCESS if all operations in the stream specified by
 * \p hStream have completed, or ::CUDA_ERROR_NOT_READY if not.
 *
 * \param hStream - Stream to query status of
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_READY
 * \notefnerr
 *
 * \sa ::cuStreamCreate,
 * ::cuStreamWaitEvent,
 * ::cuStreamDestroy,
 * ::cuStreamSynchronize
 */
CUresult CUDAAPI cuStreamQuery(CUstream hStream);

/**
 * \brief Wait until a stream's tasks are completed
 *
 * Waits until the device has completed all operations in the stream specified
 * by \p hStream. If the context was created with the 
 * ::CU_CTX_SCHED_BLOCKING_SYNC flag, the CPU thread will block until the
 * stream is finished with all of its tasks.
 *
 * \param hStream - Stream to wait for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE
 * \notefnerr
 *
 * \sa ::cuStreamCreate,
 * ::cuStreamDestroy,
 * ::cuStreamWaitEvent,
 * ::cuStreamQuery
 */
CUresult CUDAAPI cuStreamSynchronize(CUstream hStream);

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Destroys a stream
 *
 * Destroys the stream specified by \p hStream.  
 *
 * In the case that the device is still doing work in the stream \p hStream
 * when ::cuStreamDestroy() is called, the function will return immediately 
 * and the resources associated with \p hStream will be released automatically 
 * once the device has completed all work in \p hStream.
 *
 * \param hStream - Stream to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuStreamCreate,
 * ::cuStreamWaitEvent,
 * ::cuStreamQuery,
 * ::cuStreamSynchronize
 */
CUresult CUDAAPI cuStreamDestroy(CUstream hStream);
#endif /* __CUDA_API_VERSION >= 4000 */

/** @} */ /* END CUDA_STREAM */


/**
 * \defgroup CUDA_EVENT Event Management
 *
 * This section describes the event management functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

/**
 * \brief Creates an event
 *
 * Creates an event *phEvent with the flags specified via \p Flags. Valid flags
 * include:
 * - ::CU_EVENT_DEFAULT: Default event creation flag.
 * - ::CU_EVENT_BLOCKING_SYNC: Specifies that the created event should use blocking
 *   synchronization.  A CPU thread that uses ::cuEventSynchronize() to wait on
 *   an event created with this flag will block until the event has actually
 *   been recorded.
 * - ::CU_EVENT_DISABLE_TIMING: Specifies that the created event does not need
 *   to record timing data.  Events created with this flag specified and
 *   the ::CU_EVENT_BLOCKING_SYNC flag not specified will provide the best
 *   performance when used with ::cuStreamWaitEvent() and ::cuEventQuery().
 *
 * \param phEvent - Returns newly created event
 * \param Flags   - Event creation flags
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_OUT_OF_MEMORY
 * \notefnerr
 *
 * \sa
 * ::cuEventRecord,
 * ::cuEventQuery,
 * ::cuEventSynchronize,
 * ::cuEventDestroy,
 * ::cuEventElapsedTime
 */
CUresult CUDAAPI cuEventCreate(CUevent *phEvent, unsigned int Flags);

/**
 * \brief Records an event
 *
 * Records an event. If \p hStream is non-zero, the event is recorded after all
 * preceding operations in \p hStream have been completed; otherwise, it is
 * recorded after all preceding operations in the CUDA context have been
 * completed. Since operation is asynchronous, ::cuEventQuery and/or
 * ::cuEventSynchronize() must be used to determine when the event has actually
 * been recorded.
 *
 * If ::cuEventRecord() has previously been called on \p hEvent, then this
 * call will overwrite any existing state in \p hEvent.  Any subsequent calls
 * which examine the status of \p hEvent will only examine the completion of
 * this most recent call to ::cuEventRecord().
 *
 * It is necessary that \p hEvent and \p hStream be created on the same context.
 *
 * \param hEvent  - Event to record
 * \param hStream - Stream to record event for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuEventCreate,
 * ::cuEventQuery,
 * ::cuEventSynchronize,
 * ::cuStreamWaitEvent,
 * ::cuEventDestroy,
 * ::cuEventElapsedTime
 */
CUresult CUDAAPI cuEventRecord(CUevent hEvent, CUstream hStream);

/**
 * \brief Queries an event's status
 *
 * Query the status of all device work preceding the most recent
 * call to ::cuEventRecord() (in the appropriate compute streams,
 * as specified by the arguments to ::cuEventRecord()).
 *
 * If this work has successfully been completed by the device, or if
 * ::cuEventRecord() has not been called on \p hEvent, then ::CUDA_SUCCESS is
 * returned. If this work has not yet been completed by the device then
 * ::CUDA_ERROR_NOT_READY is returned.
 *
 * \param hEvent - Event to query
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_NOT_READY
 * \notefnerr
 *
 * \sa ::cuEventCreate,
 * ::cuEventRecord,
 * ::cuEventSynchronize,
 * ::cuEventDestroy,
 * ::cuEventElapsedTime
 */
CUresult CUDAAPI cuEventQuery(CUevent hEvent);

/**
 * \brief Waits for an event to complete
 *
 * Wait until the completion of all device work preceding the most recent
 * call to ::cuEventRecord() (in the appropriate compute streams, as specified
 * by the arguments to ::cuEventRecord()).
 *
 * If ::cuEventRecord() has not been called on \p hEvent, ::CUDA_SUCCESS is
 * returned immediately.
 *
 * Waiting for an event that was created with the ::CU_EVENT_BLOCKING_SYNC
 * flag will cause the calling CPU thread to block until the event has
 * been completed by the device.  If the ::CU_EVENT_BLOCKING_SYNC flag has
 * not been set, then the CPU thread will busy-wait until the event has
 * been completed by the device.
 *
 * \param hEvent - Event to wait for
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE
 * \notefnerr
 *
 * \sa ::cuEventCreate,
 * ::cuEventRecord,
 * ::cuEventQuery,
 * ::cuEventDestroy,
 * ::cuEventElapsedTime
 */
CUresult CUDAAPI cuEventSynchronize(CUevent hEvent);

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Destroys an event
 *
 * Destroys the event specified by \p hEvent.
 *
 * In the case that \p hEvent has been recorded but has not yet been completed
 * when ::cuEventDestroy() is called, the function will return immediately and 
 * the resources associated with \p hEvent will be released automatically once
 * the device has completed \p hEvent.
 *
 * \param hEvent - Event to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE
 * \notefnerr
 *
 * \sa ::cuEventCreate,
 * ::cuEventRecord,
 * ::cuEventQuery,
 * ::cuEventSynchronize,
 * ::cuEventElapsedTime
 */
CUresult CUDAAPI cuEventDestroy(CUevent hEvent);
#endif /* __CUDA_API_VERSION >= 4000 */

/**
 * \brief Computes the elapsed time between two events
 *
 * Computes the elapsed time between two events (in milliseconds with a
 * resolution of around 0.5 microseconds).
 *
 * If either event was last recorded in a non-NULL stream, the resulting time
 * may be greater than expected (even if both used the same stream handle). This
 * happens because the ::cuEventRecord() operation takes place asynchronously
 * and there is no guarantee that the measured latency is actually just between
 * the two events. Any number of other different stream operations could execute
 * in between the two measured events, thus altering the timing in a significant
 * way.
 *
 * If ::cuEventRecord() has not been called on either event then
 * ::CUDA_ERROR_INVALID_HANDLE is returned. If ::cuEventRecord() has been called
 * on both events but one or both of them has not yet been completed (that is,
 * ::cuEventQuery() would return ::CUDA_ERROR_NOT_READY on at least one of the
 * events), ::CUDA_ERROR_NOT_READY is returned. If either event was created with
 * the ::CU_EVENT_DISABLE_TIMING flag, then this function will return
 * ::CUDA_ERROR_INVALID_HANDLE.
 *
 * \param pMilliseconds - Time between \p hStart and \p hEnd in ms
 * \param hStart        - Starting event
 * \param hEnd          - Ending event
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_READY
 * \notefnerr
 *
 * \sa ::cuEventCreate,
 * ::cuEventRecord,
 * ::cuEventQuery,
 * ::cuEventSynchronize,
 * ::cuEventDestroy
 */
CUresult CUDAAPI cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd);

/** @} */ /* END CUDA_EVENT */


/**
 * \defgroup CUDA_EXEC Execution Control
 *
 * This section describes the execution control functions of the low-level CUDA
 * driver application programming interface.
 *
 * @{
 */

/**
 * \brief Returns information about a function
 *
 * Returns in \p *pi the integer value of the attribute \p attrib on the kernel
 * given by \p hfunc. The supported attributes are:
 * - ::CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK: The maximum number of threads
 *   per block, beyond which a launch of the function would fail. This number
 *   depends on both the function and the device on which the function is
 *   currently loaded.
 * - ::CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES: The size in bytes of
 *   statically-allocated shared memory per block required by this function.
 *   This does not include dynamically-allocated shared memory requested by
 *   the user at runtime.
 * - ::CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES: The size in bytes of user-allocated
 *   constant memory required by this function.
 * - ::CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES: The size in bytes of local memory
 *   used by each thread of this function.
 * - ::CU_FUNC_ATTRIBUTE_NUM_REGS: The number of registers used by each thread
 *   of this function.
 * - ::CU_FUNC_ATTRIBUTE_PTX_VERSION: The PTX virtual architecture version for
 *   which the function was compiled. This value is the major PTX version * 10
 *   + the minor PTX version, so a PTX version 1.3 function would return the
 *   value 13. Note that this may return the undefined value of 0 for cubins
 *   compiled prior to CUDA 3.0.
 * - ::CU_FUNC_ATTRIBUTE_BINARY_VERSION: The binary architecture version for
 *   which the function was compiled. This value is the major binary
 *   version * 10 + the minor binary version, so a binary version 1.3 function
 *   would return the value 13. Note that this will return a value of 10 for
 *   legacy cubins that do not have a properly-encoded binary architecture
 *   version.
 *
 * \param pi     - Returned attribute value
 * \param attrib - Attribute requested
 * \param hfunc  - Function to query attribute of
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuCtxGetCacheConfig,
 * ::cuCtxSetCacheConfig,
 * ::cuFuncSetCacheConfig,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuFuncGetAttribute(int *pi, CUfunction_attribute attrib, CUfunction hfunc);

/**
 * \brief Sets the preferred cache configuration for a device function
 *
 * On devices where the L1 cache and shared memory use the same hardware
 * resources, this sets through \p config the preferred cache configuration for
 * the device function \p hfunc. This is only a preference. The driver will use
 * the requested configuration if possible, but it is free to choose a different
 * configuration if required to execute \p hfunc.  Any context-wide preference
 * set via ::cuCtxSetCacheConfig() will be overridden by this per-function
 * setting unless the per-function setting is ::CU_FUNC_CACHE_PREFER_NONE. In
 * that case, the current context-wide setting will be used.
 *
 * This setting does nothing on devices where the size of the L1 cache and
 * shared memory are fixed.
 *
 * Launching a kernel with a different preference than the most recent
 * preference setting may insert a device-side synchronization point.
 *
 *
 * The supported cache configurations are:
 * - ::CU_FUNC_CACHE_PREFER_NONE: no preference for shared memory or L1 (default)
 * - ::CU_FUNC_CACHE_PREFER_SHARED: prefer larger shared memory and smaller L1 cache
 * - ::CU_FUNC_CACHE_PREFER_L1: prefer larger L1 cache and smaller shared memory
 *
 * \param hfunc  - Kernel to configure cache for
 * \param config - Requested cache configuration
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT
 * \notefnerr
 *
 * \sa ::cuCtxGetCacheConfig,
 * ::cuCtxSetCacheConfig,
 * ::cuFuncGetAttribute,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config);

#if __CUDA_API_VERSION >= 4000
/**
 * \brief Launches a CUDA function
 *
 * Invokes the kernel \p f on a \p gridDimX x \p gridDimY x \p gridDimZ
 * grid of blocks. Each block contains \p blockDimX x \p blockDimY x
 * \p blockDimZ threads.
 *
 * \p sharedMemBytes sets the amount of dynamic shared memory that will be
 * available to each thread block.
 *
 * ::cuLaunchKernel() can optionally be associated to a stream by passing a
 * non-zero \p hStream argument.
 *
 * Kernel parameters to \p f can be specified in one of two ways:
 *
 * 1) Kernel parameters can be specified via \p kernelParams.  If \p f
 * has N parameters, then \p kernelParams needs to be an array of N
 * pointers.  Each of \p kernelParams[0] through \p kernelParams[N-1]
 * must point to a region of memory from which the actual kernel
 * parameter will be copied.  The number of kernel parameters and their
 * offsets and sizes do not need to be specified as that information is
 * retrieved directly from the kernel's image.
 *
 * 2) Kernel parameters can also be packaged by the application into
 * a single buffer that is passed in via the \p extra parameter.
 * This places the burden on the application of knowing each kernel
 * parameter's size and alignment/padding within the buffer.  Here is
 * an example of using the \p extra parameter in this manner:
 * \code
    size_t argBufferSize;
    char argBuffer[256];

    // populate argBuffer and argBufferSize

    void *config[] = {
        CU_LAUNCH_PARAM_BUFFER_POINTER, argBuffer,
        CU_LAUNCH_PARAM_BUFFER_SIZE,    &argBufferSize,
        CU_LAUNCH_PARAM_END
    };
    status = cuLaunchKernel(f, gx, gy, gz, bx, by, bz, sh, s, NULL, config);
 * \endcode
 *
 * The \p extra parameter exists to allow ::cuLaunchKernel to take
 * additional less commonly used arguments.  \p extra specifies a list of
 * names of extra settings and their corresponding values.  Each extra
 * setting name is immediately followed by the corresponding value.  The
 * list must be terminated with either NULL or ::CU_LAUNCH_PARAM_END.
 *
 * - ::CU_LAUNCH_PARAM_END, which indicates the end of the \p extra
 *   array;
 * - ::CU_LAUNCH_PARAM_BUFFER_POINTER, which specifies that the next
 *   value in \p extra will be a pointer to a buffer containing all
 *   the kernel parameters for launching kernel \p f;
 * - ::CU_LAUNCH_PARAM_BUFFER_SIZE, which specifies that the next
 *   value in \p extra will be a pointer to a size_t containing the
 *   size of the buffer specified with ::CU_LAUNCH_PARAM_BUFFER_POINTER;
 *
 * The error ::CUDA_ERROR_INVALID_VALUE will be returned if kernel
 * parameters are specified with both \p kernelParams and \p extra
 * (i.e. both \p kernelParams and \p extra are non-NULL).
 *
 * Calling ::cuLaunchKernel() sets persistent function state that is
 * the same as function state set through the following deprecated APIs:
 *
 *  ::cuFuncSetBlockShape()
 *  ::cuFuncSetSharedSize()
 *  ::cuParamSetSize()
 *  ::cuParamSeti()
 *  ::cuParamSetf()
 *  ::cuParamSetv()
 *
 * When the kernel \p f is launched via ::cuLaunchKernel(), the previous
 * block shape, shared size and parameter info associated with \p f
 * is overwritten.
 *
 * Note that to use ::cuLaunchKernel(), the kernel \p f must either have
 * been compiled with toolchain version 3.2 or later so that it will
 * contain kernel parameter information, or have no kernel parameters.
 * If either of these conditions is not met, then ::cuLaunchKernel() will
 * return ::CUDA_ERROR_INVALID_IMAGE.
 *
 * \param f              - Kernel to launch
 * \param gridDimX       - Width of grid in blocks
 * \param gridDimY       - Height of grid in blocks
 * \param gridDimZ       - Depth of grid in blocks
 * \param blockDimX      - X dimension of each thread block
 * \param blockDimY      - Y dimension of each thread block
 * \param blockDimZ      - Z dimension of each thread block
 * \param sharedMemBytes - Dynamic shared-memory size per thread block in bytes
 * \param hStream        - Stream identifier
 * \param kernelParams   - Array of pointers to kernel parameters
 * \param extra          - Extra options
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_IMAGE,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_LAUNCH_FAILED,
 * ::CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES,
 * ::CUDA_ERROR_LAUNCH_TIMEOUT,
 * ::CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuCtxGetCacheConfig,
 * ::cuCtxSetCacheConfig,
 * ::cuFuncSetCacheConfig,
 * ::cuFuncGetAttribute,
 */
CUresult CUDAAPI cuLaunchKernel(CUfunction f,
                                unsigned int gridDimX,
                                unsigned int gridDimY,
                                unsigned int gridDimZ,
                                unsigned int blockDimX,
                                unsigned int blockDimY,
                                unsigned int blockDimZ,
                                unsigned int sharedMemBytes,
                                CUstream hStream,
                                void **kernelParams,
                                void **extra);
#endif /* __CUDA_API_VERSION >= 4000 */

/**
 * \defgroup CUDA_EXEC_DEPRECATED Execution Control [DEPRECATED]
 *
 * This section describes the deprecated execution control functions of the
 * low-level CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Sets the block-dimensions for the function
 *
 * \deprecated
 *
 * Specifies the \p x, \p y, and \p z dimensions of the thread blocks that are
 * created when the kernel given by \p hfunc is launched.
 *
 * \param hfunc - Kernel to specify dimensions of
 * \param x     - X dimension
 * \param y     - Y dimension
 * \param z     - Z dimension
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetSharedSize,
 * ::cuFuncSetCacheConfig,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSeti,
 * ::cuParamSetf,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z);

/**
 * \brief Sets the dynamic shared-memory size for the function
 *
 * \deprecated
 *
 * Sets through \p bytes the amount of dynamic shared memory that will be
 * available to each thread block when the kernel given by \p hfunc is launched.
 *
 * \param hfunc - Kernel to specify dynamic shared-memory size for
 * \param bytes - Dynamic shared-memory size per thread in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetCacheConfig,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSeti,
 * ::cuParamSetf,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes);

/**
 * \brief Sets the parameter size for the function
 *
 * \deprecated
 *
 * Sets through \p numbytes the total size in bytes needed by the function
 * parameters of the kernel corresponding to \p hfunc.
 *
 * \param hfunc    - Kernel to set parameter size for
 * \param numbytes - Size of parameter list in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetf,
 * ::cuParamSeti,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuParamSetSize(CUfunction hfunc, unsigned int numbytes);

/**
 * \brief Adds an integer parameter to the function's argument list
 *
 * \deprecated
 *
 * Sets an integer parameter that will be specified the next time the
 * kernel corresponding to \p hfunc will be invoked. \p offset is a byte offset.
 *
 * \param hfunc  - Kernel to add parameter to
 * \param offset - Offset to add parameter to argument list
 * \param value  - Value of parameter
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSetf,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuParamSeti(CUfunction hfunc, int offset, unsigned int value);

/**
 * \brief Adds a floating-point parameter to the function's argument list
 *
 * \deprecated
 *
 * Sets a floating-point parameter that will be specified the next time the
 * kernel corresponding to \p hfunc will be invoked. \p offset is a byte offset.
 *
 * \param hfunc  - Kernel to add parameter to
 * \param offset - Offset to add parameter to argument list
 * \param value  - Value of parameter
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSeti,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuParamSetf(CUfunction hfunc, int offset, float value);

/**
 * \brief Adds arbitrary data to the function's argument list
 *
 * \deprecated
 *
 * Copies an arbitrary amount of data (specified in \p numbytes) from \p ptr
 * into the parameter space of the kernel corresponding to \p hfunc. \p offset
 * is a byte offset.
 *
 * \param hfunc    - Kernel to add data to
 * \param offset   - Offset to add data to argument list
 * \param ptr      - Pointer to arbitrary data
 * \param numbytes - Size of data to copy in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSetf,
 * ::cuParamSeti,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuParamSetv(CUfunction hfunc, int offset, void *ptr, unsigned int numbytes);

/**
 * \brief Launches a CUDA function
 *
 * \deprecated
 *
 * Invokes the kernel \p f on a 1 x 1 x 1 grid of blocks. The block
 * contains the number of threads specified by a previous call to
 * ::cuFuncSetBlockShape().
 *
 * \param f - Kernel to launch
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_LAUNCH_FAILED,
 * ::CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES,
 * ::CUDA_ERROR_LAUNCH_TIMEOUT,
 * ::CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSetf,
 * ::cuParamSeti,
 * ::cuParamSetv,
 * ::cuLaunchGrid,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuLaunch(CUfunction f);

/**
 * \brief Launches a CUDA function
 *
 * \deprecated
 *
 * Invokes the kernel \p f on a \p grid_width x \p grid_height grid of
 * blocks. Each block contains the number of threads specified by a previous
 * call to ::cuFuncSetBlockShape().
 *
 * \param f           - Kernel to launch
 * \param grid_width  - Width of grid in blocks
 * \param grid_height - Height of grid in blocks
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_LAUNCH_FAILED,
 * ::CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES,
 * ::CUDA_ERROR_LAUNCH_TIMEOUT,
 * ::CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSetf,
 * ::cuParamSeti,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGridAsync,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuLaunchGrid(CUfunction f, int grid_width, int grid_height);

/**
 * \brief Launches a CUDA function
 *
 * \deprecated
 *
 * Invokes the kernel \p f on a \p grid_width x \p grid_height grid of
 * blocks. Each block contains the number of threads specified by a previous
 * call to ::cuFuncSetBlockShape().
 *
 * ::cuLaunchGridAsync() can optionally be associated to a stream by passing a
 * non-zero \p hStream argument.
 *
 * \param f           - Kernel to launch
 * \param grid_width  - Width of grid in blocks
 * \param grid_height - Height of grid in blocks
 * \param hStream     - Stream identifier
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_LAUNCH_FAILED,
 * ::CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES,
 * ::CUDA_ERROR_LAUNCH_TIMEOUT,
 * ::CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING,
 * ::CUDA_ERROR_SHARED_OBJECT_INIT_FAILED
 * \notefnerr
 *
 * \sa ::cuFuncSetBlockShape,
 * ::cuFuncSetSharedSize,
 * ::cuFuncGetAttribute,
 * ::cuParamSetSize,
 * ::cuParamSetf,
 * ::cuParamSeti,
 * ::cuParamSetv,
 * ::cuLaunch,
 * ::cuLaunchGrid,
 * ::cuLaunchKernel
 */
CUresult CUDAAPI cuLaunchGridAsync(CUfunction f, int grid_width, int grid_height, CUstream hStream);


/**
 * \brief Adds a texture-reference to the function's argument list
 *
 * \deprecated
 *
 * Makes the CUDA array or linear memory bound to the texture reference
 * \p hTexRef available to a device program as a texture. In this version of
 * CUDA, the texture-reference must be obtained via ::cuModuleGetTexRef() and
 * the \p texunit parameter must be set to ::CU_PARAM_TR_DEFAULT.
 *
 * \param hfunc   - Kernel to add texture-reference to
 * \param texunit - Texture unit (must be ::CU_PARAM_TR_DEFAULT)
 * \param hTexRef - Texture-reference to add to argument list
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 */
CUresult CUDAAPI cuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef);
/** @} */ /* END CUDA_EXEC_DEPRECATED */

/** @} */ /* END CUDA_EXEC */


/**
 * \defgroup CUDA_TEXREF Texture Reference Management
 *
 * This section describes the texture reference management functions of the
 * low-level CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Binds an array as a texture reference
 *
 * Binds the CUDA array \p hArray to the texture reference \p hTexRef. Any
 * previous address or CUDA array state associated with the texture reference
 * is superseded by this function. \p Flags must be set to
 * ::CU_TRSA_OVERRIDE_FORMAT. Any CUDA array previously bound to \p hTexRef is
 * unbound.
 *
 * \param hTexRef - Texture reference to bind
 * \param hArray  - Array to bind
 * \param Flags   - Options (must be ::CU_TRSA_OVERRIDE_FORMAT)
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Binds an address as a texture reference
 *
 * Binds a linear address range to the texture reference \p hTexRef. Any
 * previous address or CUDA array state associated with the texture reference
 * is superseded by this function. Any memory previously bound to \p hTexRef
 * is unbound.
 *
 * Since the hardware enforces an alignment requirement on texture base
 * addresses, ::cuTexRefSetAddress() passes back a byte offset in
 * \p *ByteOffset that must be applied to texture fetches in order to read from
 * the desired memory. This offset must be divided by the texel size and
 * passed to kernels that read from the texture so they can be applied to the
 * ::tex1Dfetch() function.
 *
 * If the device memory pointer was returned from ::cuMemAlloc(), the offset
 * is guaranteed to be 0 and NULL may be passed as the \p ByteOffset parameter.
 *
 * \param ByteOffset - Returned byte offset
 * \param hTexRef    - Texture reference to bind
 * \param dptr       - Device pointer to bind
 * \param bytes      - Size of memory to bind in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetAddress(size_t *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes);

/**
 * \brief Binds an address as a 2D texture reference
 *
 * Binds a linear address range to the texture reference \p hTexRef. Any
 * previous address or CUDA array state associated with the texture reference
 * is superseded by this function. Any memory previously bound to \p hTexRef
 * is unbound.
 *
 * Using a ::tex2D() function inside a kernel requires a call to either
 * ::cuTexRefSetArray() to bind the corresponding texture reference to an
 * array, or ::cuTexRefSetAddress2D() to bind the texture reference to linear
 * memory.
 *
 * Function calls to ::cuTexRefSetFormat() cannot follow calls to
 * ::cuTexRefSetAddress2D() for the same texture reference.
 *
 * It is required that \p dptr be aligned to the appropriate hardware-specific
 * texture alignment. You can query this value using the device attribute
 * ::CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT. If an unaligned \p dptr is
 * supplied, ::CUDA_ERROR_INVALID_VALUE is returned.
 *
 * \param hTexRef - Texture reference to bind
 * \param desc    - Descriptor of CUDA array
 * \param dptr    - Device pointer to bind
 * \param Pitch   - Line pitch in bytes
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetAddress2D(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Sets the format for a texture reference
 *
 * Specifies the format of the data to be read by the texture reference
 * \p hTexRef. \p fmt and \p NumPackedComponents are exactly analogous to the
 * ::Format and ::NumChannels members of the ::CUDA_ARRAY_DESCRIPTOR structure:
 * They specify the format of each component and the number of components per
 * array element.
 *
 * \param hTexRef             - Texture reference
 * \param fmt                 - Format to set
 * \param NumPackedComponents - Number of components per array element
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents);

/**
 * \brief Sets the addressing mode for a texture reference
 *
 * Specifies the addressing mode \p am for the given dimension \p dim of the
 * texture reference \p hTexRef. If \p dim is zero, the addressing mode is
 * applied to the first parameter of the functions used to fetch from the
 * texture; if \p dim is 1, the second, and so on. ::CUaddress_mode is defined
 * as:
 * \code
   typedef enum CUaddress_mode_enum {
      CU_TR_ADDRESS_MODE_WRAP = 0,
      CU_TR_ADDRESS_MODE_CLAMP = 1,
      CU_TR_ADDRESS_MODE_MIRROR = 2,
      CU_TR_ADDRESS_MODE_BORDER = 3
   } CUaddress_mode;
 * \endcode
 *
 * Note that this call has no effect if \p hTexRef is bound to linear memory.
 * Also, if the flag, ::CU_TRSF_NORMALIZED_COORDINATES, is not set, the only 
 * supported address mode is ::CU_TR_ADDRESS_MODE_CLAMP.
 *
 * \param hTexRef - Texture reference
 * \param dim     - Dimension
 * \param am      - Addressing mode to set
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am);

/**
 * \brief Sets the filtering mode for a texture reference
 *
 * Specifies the filtering mode \p fm to be used when reading memory through
 * the texture reference \p hTexRef. ::CUfilter_mode_enum is defined as:
 *
 * \code
   typedef enum CUfilter_mode_enum {
      CU_TR_FILTER_MODE_POINT = 0,
      CU_TR_FILTER_MODE_LINEAR = 1
   } CUfilter_mode;
 * \endcode
 *
 * Note that this call has no effect if \p hTexRef is bound to linear memory.
 *
 * \param hTexRef - Texture reference
 * \param fm      - Filtering mode to set
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm);

/**
 * \brief Sets the flags for a texture reference
 *
 * Specifies optional flags via \p Flags to specify the behavior of data
 * returned through the texture reference \p hTexRef. The valid flags are:
 *
 * - ::CU_TRSF_READ_AS_INTEGER, which suppresses the default behavior of
 *   having the texture promote integer data to floating point data in the
 *   range [0, 1];
 * - ::CU_TRSF_NORMALIZED_COORDINATES, which suppresses the default behavior
 *   of having the texture coordinates range from [0, Dim) where Dim is the
 *   width or height of the CUDA array. Instead, the texture coordinates
 *   [0, 1.0) reference the entire breadth of the array dimension;
 *
 * \param hTexRef - Texture reference
 * \param Flags   - Optional flags to set
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Gets the address associated with a texture reference
 *
 * Returns in \p *pdptr the base address bound to the texture reference
 * \p hTexRef, or returns ::CUDA_ERROR_INVALID_VALUE if the texture reference
 * is not bound to any device memory range.
 *
 * \param pdptr   - Returned device address
 * \param hTexRef - Texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefGetAddress(CUdeviceptr *pdptr, CUtexref hTexRef);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Gets the array bound to a texture reference
 *
 * Returns in \p *phArray the CUDA array bound to the texture reference
 * \p hTexRef, or returns ::CUDA_ERROR_INVALID_VALUE if the texture reference
 * is not bound to any CUDA array.
 *
 * \param phArray - Returned array
 * \param hTexRef - Texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefGetArray(CUarray *phArray, CUtexref hTexRef);

/**
 * \brief Gets the addressing mode used by a texture reference
 *
 * Returns in \p *pam the addressing mode corresponding to the
 * dimension \p dim of the texture reference \p hTexRef. Currently, the only
 * valid value for \p dim are 0 and 1.
 *
 * \param pam     - Returned addressing mode
 * \param hTexRef - Texture reference
 * \param dim     - Dimension
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefGetAddressMode(CUaddress_mode *pam, CUtexref hTexRef, int dim);

/**
 * \brief Gets the filter-mode used by a texture reference
 *
 * Returns in \p *pfm the filtering mode of the texture reference
 * \p hTexRef.
 *
 * \param pfm     - Returned filtering mode
 * \param hTexRef - Texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFlags, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefGetFilterMode(CUfilter_mode *pfm, CUtexref hTexRef);

/**
 * \brief Gets the format used by a texture reference
 *
 * Returns in \p *pFormat and \p *pNumChannels the format and number
 * of components of the CUDA array bound to the texture reference \p hTexRef.
 * If \p pFormat or \p pNumChannels is NULL, it will be ignored.
 *
 * \param pFormat      - Returned format
 * \param pNumChannels - Returned number of components
 * \param hTexRef      - Texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFlags
 */
CUresult CUDAAPI cuTexRefGetFormat(CUarray_format *pFormat, int *pNumChannels, CUtexref hTexRef);

/**
 * \brief Gets the flags used by a texture reference
 *
 * Returns in \p *pFlags the flags of the texture reference \p hTexRef.
 *
 * \param pFlags  - Returned flags
 * \param hTexRef - Texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefSetAddress,
 * ::cuTexRefSetAddress2D, ::cuTexRefSetAddressMode, ::cuTexRefSetArray,
 * ::cuTexRefSetFilterMode, ::cuTexRefSetFlags, ::cuTexRefSetFormat,
 * ::cuTexRefGetAddress, ::cuTexRefGetAddressMode, ::cuTexRefGetArray,
 * ::cuTexRefGetFilterMode, ::cuTexRefGetFormat
 */
CUresult CUDAAPI cuTexRefGetFlags(unsigned int *pFlags, CUtexref hTexRef);

/**
 * \defgroup CUDA_TEXREF_DEPRECATED Texture Reference Management [DEPRECATED]
 *
 * This section describes the deprecated texture reference management
 * functions of the low-level CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Creates a texture reference
 *
 * \deprecated
 *
 * Creates a texture reference and returns its handle in \p *pTexRef. Once
 * created, the application must call ::cuTexRefSetArray() or
 * ::cuTexRefSetAddress() to associate the reference with allocated memory.
 * Other texture reference functions are used to specify the format and
 * interpretation (addressing, filtering, etc.) to be used when the memory is
 * read through this texture reference.
 *
 * \param pTexRef - Returned texture reference
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefDestroy
 */
CUresult CUDAAPI cuTexRefCreate(CUtexref *pTexRef);

/**
 * \brief Destroys a texture reference
 *
 * \deprecated
 *
 * Destroys the texture reference specified by \p hTexRef.
 *
 * \param hTexRef - Texture reference to destroy
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuTexRefCreate
 */
CUresult CUDAAPI cuTexRefDestroy(CUtexref hTexRef);

/** @} */ /* END CUDA_TEXREF_DEPRECATED */

/** @} */ /* END CUDA_TEXREF */


/**
 * \defgroup CUDA_SURFREF Surface Reference Management
 *
 * This section describes the surface reference management functions of the
 * low-level CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Sets the CUDA array for a surface reference.
 *
 * Sets the CUDA array \p hArray to be read and written by the surface reference
 * \p hSurfRef.  Any previous CUDA array state associated with the surface
 * reference is superseded by this function.  \p Flags must be set to 0.
 * The ::CUDA_ARRAY3D_SURFACE_LDST flag must have been set for the CUDA array.
 * Any CUDA array previously bound to \p hSurfRef is unbound.

 * \param hSurfRef - Surface reference handle
 * \param hArray - CUDA array handle
 * \param Flags - set to 0
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuModuleGetSurfRef, ::cuSurfRefGetArray
 */
CUresult CUDAAPI cuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags);

/**
 * \brief Passes back the CUDA array bound to a surface reference.
 *
 * Returns in \p *phArray the CUDA array bound to the surface reference
 * \p hSurfRef, or returns ::CUDA_ERROR_INVALID_VALUE if the surface reference
 * is not bound to any CUDA array.

 * \param phArray - Surface reference handle
 * \param hSurfRef - Surface reference handle
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 *
 * \sa ::cuModuleGetSurfRef, ::cuSurfRefSetArray
 */
CUresult CUDAAPI cuSurfRefGetArray(CUarray *phArray, CUsurfref hSurfRef);

/** @} */ /* END CUDA_SURFREF */

#if __CUDA_API_VERSION >= 4000
/**
 * \defgroup CUDA_PEER_ACCESS Peer Context Memory Access
 *
 * This section describes the direct peer context memory access functions 
 * of the low-level CUDA driver application programming interface.
 *
 * @{
 */
    
/**
 * \brief Queries if a device may directly access a peer device's memory.
 *
 * Returns in \p *canAccessPeer a value of 1 if contexts on \p dev are capable of
 * directly accessing memory from contexts on \p peerDev and 0 otherwise.
 * If direct access of \p peerDev from \p dev is possible, then access may be
 * enabled on two specific contexts by calling ::cuCtxEnablePeerAccess().
 *
 * \param canAccessPeer - Returned access capability
 * \param dev           - Device from which allocations on \p peerDev are to
 *                        be directly accessed.
 * \param peerDev       - Device on which the allocations to be directly accessed 
 *                        by \p dev reside.
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_DEVICE
 * \notefnerr
 *
 * \sa ::cuCtxEnablePeerAccess,
 * ::cuCtxDisablePeerAccess
 */
CUresult CUDAAPI cuDeviceCanAccessPeer(int *canAccessPeer, CUdevice dev, CUdevice peerDev);

/**
 * \brief Enables direct access to memory allocations in a peer context.
 *
  If both the current context and \p peerContext are on devices which support unified 
 * addressing (as may be queried using ::CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING), then
 * on success all allocations from \p peerContext will immediately be accessible
 * by the current context.  See \ref CUDA_UNIFIED for additional
 * details.
 *
 * Note that access granted by this call is unidirectional and that in order to access
 * memory from the current context in \p peerContext, a separate symmetric call 
 * to ::cuCtxEnablePeerAccess() is required.
 *
 * Returns ::CUDA_ERROR_INVALID_DEVICE if ::cuDeviceCanAccessPeer() indicates
 * that the ::CUdevice of the current context cannot directly access memory
 * from the ::CUdevice of \p peerContext.
 *
 * Returns ::CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED if direct access of
 * \p peerContext from the current context has already been enabled.
 *
 * Returns ::CUDA_ERROR_INVALID_CONTEXT if there is no current context, \p peerContext
 * is not a valid context, or if the current context is \p peerContext.
 *
 * Returns ::CUDA_ERROR_INVALID_VALUE if \p Flags is not 0.
 *
 * \param peerContext - Peer context to enable direct access to from the current context
 * \param Flags       - Reserved for future use and must be set to 0
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_DEVICE,
 * ::CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE
 * \notefnerr
 *
 * \sa ::cuDeviceCanAccessPeer,
 * ::cuCtxDisablePeerAccess
 */
CUresult CUDAAPI cuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags);

/**
 * \brief Disables direct access to memory allocations in a peer context and 
 * unregisters any registered allocations.
 *
  Returns ::CUDA_ERROR_PEER_ACCESS_NOT_ENABLED if direct peer access has 
 * not yet been enabled from \p peerContext to the current context.
 *
 * Returns ::CUDA_ERROR_INVALID_CONTEXT if there is no current context, or if
 * \p peerContext is not a valid context.
 *
 * \param peerContext - Peer context to disable direct access to
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_PEER_ACCESS_NOT_ENABLED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * \notefnerr
 *
 * \sa ::cuDeviceCanAccessPeer,
 * ::cuCtxEnablePeerAccess
 */
CUresult CUDAAPI cuCtxDisablePeerAccess(CUcontext peerContext);

/** @} */ /* END CUDA_PEER_ACCESS */
#endif /* __CUDA_API_VERSION >= 4000 */

/**
 * \defgroup CUDA_GRAPHICS Graphics Interoperability
 *
 * This section describes the graphics interoperability functions of the
 * low-level CUDA driver application programming interface.
 *
 * @{
 */

/**
 * \brief Unregisters a graphics resource for access by CUDA
 *
 * Unregisters the graphics resource \p resource so it is not accessible by
 * CUDA unless registered again.
 *
 * If \p resource is invalid then ::CUDA_ERROR_INVALID_HANDLE is
 * returned.
 *
 * \param resource - Resource to unregister
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsD3D9RegisterResource,
 * ::cuGraphicsD3D10RegisterResource,
 * ::cuGraphicsD3D11RegisterResource,
 * ::cuGraphicsGLRegisterBuffer,
 * ::cuGraphicsGLRegisterImage
 */
CUresult CUDAAPI cuGraphicsUnregisterResource(CUgraphicsResource resource);

/**
 * \brief Get an array through which to access a subresource of a mapped graphics resource.
 *
 * Returns in \p *pArray an array through which the subresource of the mapped
 * graphics resource \p resource which corresponds to array index \p arrayIndex
 * and mipmap level \p mipLevel may be accessed.  The value set in \p *pArray may
 * change every time that \p resource is mapped.
 *
 * If \p resource is not a texture then it cannot be accessed via an array and
 * ::CUDA_ERROR_NOT_MAPPED_AS_ARRAY is returned.
 * If \p arrayIndex is not a valid array index for \p resource then
 * ::CUDA_ERROR_INVALID_VALUE is returned.
 * If \p mipLevel is not a valid mipmap level for \p resource then
 * ::CUDA_ERROR_INVALID_VALUE is returned.
 * If \p resource is not mapped then ::CUDA_ERROR_NOT_MAPPED is returned.
 *
 * \param pArray      - Returned array through which a subresource of \p resource may be accessed
 * \param resource    - Mapped resource to access
 * \param arrayIndex  - Array index for array textures or cubemap face
 *                      index as defined by ::CUarray_cubemap_face for
 *                      cubemap textures for the subresource to access
 * \param mipLevel    - Mipmap level for the subresource to access
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_MAPPED
 * ::CUDA_ERROR_NOT_MAPPED_AS_ARRAY
 * \notefnerr
 *
 * \sa ::cuGraphicsResourceGetMappedPointer
 */
CUresult CUDAAPI cuGraphicsSubResourceGetMappedArray(CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel);

#if __CUDA_API_VERSION >= 3020
/**
 * \brief Get a device pointer through which to access a mapped graphics resource.
 *
 * Returns in \p *pDevPtr a pointer through which the mapped graphics resource
 * \p resource may be accessed.
 * Returns in \p pSize the size of the memory in bytes which may be accessed from that pointer.
 * The value set in \p pPointer may change every time that \p resource is mapped.
 *
 * If \p resource is not a buffer then it cannot be accessed via a pointer and
 * ::CUDA_ERROR_NOT_MAPPED_AS_POINTER is returned.
 * If \p resource is not mapped then ::CUDA_ERROR_NOT_MAPPED is returned.
 * *
 * \param pDevPtr    - Returned pointer through which \p resource may be accessed
 * \param pSize      - Returned size of the buffer accessible starting at \p *pPointer
 * \param resource   - Mapped resource to access
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_MAPPED
 * ::CUDA_ERROR_NOT_MAPPED_AS_POINTER
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsMapResources,
 * ::cuGraphicsSubResourceGetMappedArray
 */
CUresult CUDAAPI cuGraphicsResourceGetMappedPointer(CUdeviceptr *pDevPtr, size_t *pSize, CUgraphicsResource resource);
#endif /* __CUDA_API_VERSION >= 3020 */

/**
 * \brief Set usage flags for mapping a graphics resource
 *
 * Set \p flags for mapping the graphics resource \p resource.
 *
 * Changes to \p flags will take effect the next time \p resource is mapped.
 * The \p flags argument may be any of the following:

 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE: Specifies no hints about how this
 *   resource will be used. It is therefore assumed that this resource will be
 *   read from and written to by CUDA kernels.  This is the default value.
 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_READONLY: Specifies that CUDA kernels which
 *   access this resource will not write to this resource.
 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITEDISCARD: Specifies that CUDA kernels
 *   which access this resource will not read from this resource and will
 *   write over the entire contents of the resource, so none of the data
 *   previously stored in the resource will be preserved.
 *
 * If \p resource is presently mapped for access by CUDA then
 * ::CUDA_ERROR_ALREADY_MAPPED is returned.
 * If \p flags is not one of the above values then ::CUDA_ERROR_INVALID_VALUE is returned.
 *
 * \param resource - Registered resource to set flags for
 * \param flags    - Parameters for resource mapping
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_ALREADY_MAPPED
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsMapResources
 */
CUresult CUDAAPI cuGraphicsResourceSetMapFlags(CUgraphicsResource resource, unsigned int flags);

/**
 * \brief Map graphics resources for access by CUDA
 *
 * Maps the \p count graphics resources in \p resources for access by CUDA.
 *
 * The resources in \p resources may be accessed by CUDA until they
 * are unmapped. The graphics API from which \p resources were registered
 * should not access any resources while they are mapped by CUDA. If an
 * application does so, the results are undefined.
 *
 * This function provides the synchronization guarantee that any graphics calls
 * issued before ::cuGraphicsMapResources() will complete before any subsequent CUDA
 * work issued in \p stream begins.
 *
 * If \p resources includes any duplicate entries then ::CUDA_ERROR_INVALID_HANDLE is returned.
 * If any of \p resources are presently mapped for access by CUDA then ::CUDA_ERROR_ALREADY_MAPPED is returned.
 *
 * \param count      - Number of resources to map
 * \param resources  - Resources to map for CUDA usage
 * \param hStream    - Stream with which to synchronize
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_ALREADY_MAPPED,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsResourceGetMappedPointer
 * ::cuGraphicsSubResourceGetMappedArray
 * ::cuGraphicsUnmapResources
 */
CUresult CUDAAPI cuGraphicsMapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream);

/**
 * \brief Unmap graphics resources.
 *
 * Unmaps the \p count graphics resources in \p resources.
 *
 * Once unmapped, the resources in \p resources may not be accessed by CUDA
 * until they are mapped again.
 *
 * This function provides the synchronization guarantee that any CUDA work issued
 * in \p stream before ::cuGraphicsUnmapResources() will complete before any
 * subsequently issued graphics work begins.
 *
 *
 * If \p resources includes any duplicate entries then ::CUDA_ERROR_INVALID_HANDLE is returned.
 * If any of \p resources are not presently mapped for access by CUDA then ::CUDA_ERROR_NOT_MAPPED is returned.
 *
 * \param count      - Number of resources to unmap
 * \param resources  - Resources to unmap
 * \param hStream    - Stream with which to synchronize
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_MAPPED,
 * ::CUDA_ERROR_UNKNOWN
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsMapResources
 */
CUresult CUDAAPI cuGraphicsUnmapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream);

/** @} */ /* END CUDA_GRAPHICS */

CUresult CUDAAPI cuGetExportTable(const void **ppExportTable, const CUuuid *pExportTableId);


/** @} */ /* END CUDA_DRIVER */

/**
 * CUDA API versioning support
 */
#if defined(__CUDA_API_VERSION_INTERNAL)
    #undef cuDeviceTotalMem
    #undef cuCtxCreate
    #undef cuModuleGetGlobal
    #undef cuMemGetInfo
    #undef cuMemAlloc
    #undef cuMemAllocPitch
    #undef cuMemFree
    #undef cuMemGetAddressRange
    #undef cuMemAllocHost
    #undef cuMemHostGetDevicePointer
    #undef cuMemcpyHtoD
    #undef cuMemcpyDtoH
    #undef cuMemcpyDtoD
    #undef cuMemcpyDtoA
    #undef cuMemcpyAtoD
    #undef cuMemcpyHtoA
    #undef cuMemcpyAtoH
    #undef cuMemcpyAtoA
    #undef cuMemcpyHtoAAsync
    #undef cuMemcpyAtoHAsync
    #undef cuMemcpy2D
    #undef cuMemcpy2DUnaligned
    #undef cuMemcpy3D
    #undef cuMemcpyHtoDAsync
    #undef cuMemcpyDtoHAsync
    #undef cuMemcpyDtoDAsync
    #undef cuMemcpy2DAsync
    #undef cuMemcpy3DAsync
    #undef cuMemsetD8
    #undef cuMemsetD16
    #undef cuMemsetD32
    #undef cuMemsetD2D8
    #undef cuMemsetD2D16
    #undef cuMemsetD2D32
    #undef cuArrayCreate
    #undef cuArrayGetDescriptor
    #undef cuArray3DCreate
    #undef cuArray3DGetDescriptor
    #undef cuTexRefSetAddress
    #undef cuTexRefSetAddress2D
    #undef cuTexRefGetAddress
    #undef cuGraphicsResourceGetMappedPointer
    #undef cuCtxDestroy
    #undef cuCtxPopCurrent
    #undef cuCtxPushCurrent
    #undef cuStreamDestroy
    #undef cuEventDestroy
#endif /* __CUDA_API_VERSION_INTERNAL */

/**
 * CUDA API made obselete at API version 3020
 */
#if defined(__CUDA_API_VERSION_INTERNAL)
    #define CUdeviceptr                  CUdeviceptr_v1
    #define CUDA_MEMCPY2D_st             CUDA_MEMCPY2D_v1_st
    #define CUDA_MEMCPY2D                CUDA_MEMCPY2D_v1
    #define CUDA_MEMCPY3D_st             CUDA_MEMCPY3D_v1_st
    #define CUDA_MEMCPY3D                CUDA_MEMCPY3D_v1
    #define CUDA_ARRAY_DESCRIPTOR_st     CUDA_ARRAY_DESCRIPTOR_v1_st
    #define CUDA_ARRAY_DESCRIPTOR        CUDA_ARRAY_DESCRIPTOR_v1
    #define CUDA_ARRAY3D_DESCRIPTOR_st   CUDA_ARRAY3D_DESCRIPTOR_v1_st
    #define CUDA_ARRAY3D_DESCRIPTOR      CUDA_ARRAY3D_DESCRIPTOR_v1
#endif /* CUDA_FORCE_LEGACY32_INTERNAL */

#if defined(__CUDA_API_VERSION_INTERNAL) || __CUDA_API_VERSION < 3020

typedef unsigned int CUdeviceptr;

typedef struct CUDA_MEMCPY2D_st
{
    unsigned int srcXInBytes;   /**< Source X in bytes */
    unsigned int srcY;          /**< Source Y */
    CUmemorytype srcMemoryType; /**< Source memory type (host, device, array) */
    const void *srcHost;        /**< Source host pointer */
    CUdeviceptr srcDevice;      /**< Source device pointer */
    CUarray srcArray;           /**< Source array reference */
    unsigned int srcPitch;      /**< Source pitch (ignored when src is array) */

    unsigned int dstXInBytes;   /**< Destination X in bytes */
    unsigned int dstY;          /**< Destination Y */
    CUmemorytype dstMemoryType; /**< Destination memory type (host, device, array) */
    void *dstHost;              /**< Destination host pointer */
    CUdeviceptr dstDevice;      /**< Destination device pointer */
    CUarray dstArray;           /**< Destination array reference */
    unsigned int dstPitch;      /**< Destination pitch (ignored when dst is array) */

    unsigned int WidthInBytes;  /**< Width of 2D memory copy in bytes */
    unsigned int Height;        /**< Height of 2D memory copy */
} CUDA_MEMCPY2D;

typedef struct CUDA_MEMCPY3D_st
{
    unsigned int srcXInBytes;   /**< Source X in bytes */
    unsigned int srcY;          /**< Source Y */
    unsigned int srcZ;          /**< Source Z */
    unsigned int srcLOD;        /**< Source LOD */
    CUmemorytype srcMemoryType; /**< Source memory type (host, device, array) */
    const void *srcHost;        /**< Source host pointer */
    CUdeviceptr srcDevice;      /**< Source device pointer */
    CUarray srcArray;           /**< Source array reference */
    void *reserved0;            /**< Must be NULL */
    unsigned int srcPitch;      /**< Source pitch (ignored when src is array) */
    unsigned int srcHeight;     /**< Source height (ignored when src is array; may be 0 if Depth==1) */

    unsigned int dstXInBytes;   /**< Destination X in bytes */
    unsigned int dstY;          /**< Destination Y */
    unsigned int dstZ;          /**< Destination Z */
    unsigned int dstLOD;        /**< Destination LOD */
    CUmemorytype dstMemoryType; /**< Destination memory type (host, device, array) */
    void *dstHost;              /**< Destination host pointer */
    CUdeviceptr dstDevice;      /**< Destination device pointer */
    CUarray dstArray;           /**< Destination array reference */
    void *reserved1;            /**< Must be NULL */
    unsigned int dstPitch;      /**< Destination pitch (ignored when dst is array) */
    unsigned int dstHeight;     /**< Destination height (ignored when dst is array; may be 0 if Depth==1) */

    unsigned int WidthInBytes;  /**< Width of 3D memory copy in bytes */
    unsigned int Height;        /**< Height of 3D memory copy */
    unsigned int Depth;         /**< Depth of 3D memory copy */
} CUDA_MEMCPY3D;

typedef struct CUDA_ARRAY_DESCRIPTOR_st
{
    unsigned int Width;         /**< Width of array */
    unsigned int Height;        /**< Height of array */

    CUarray_format Format;      /**< Array format */
    unsigned int NumChannels;   /**< Channels per array element */
} CUDA_ARRAY_DESCRIPTOR;

typedef struct CUDA_ARRAY3D_DESCRIPTOR_st
{
    unsigned int Width;         /**< Width of 3D array */
    unsigned int Height;        /**< Height of 3D array */
    unsigned int Depth;         /**< Depth of 3D array */

    CUarray_format Format;      /**< Array format */
    unsigned int NumChannels;   /**< Channels per array element */
    unsigned int Flags;         /**< Flags */
} CUDA_ARRAY3D_DESCRIPTOR;

CUresult CUDAAPI cuDeviceTotalMem(unsigned int *bytes, CUdevice dev);
CUresult CUDAAPI cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev);
CUresult CUDAAPI cuModuleGetGlobal(CUdeviceptr *dptr, unsigned int *bytes, CUmodule hmod, const char *name);
CUresult CUDAAPI cuMemGetInfo(unsigned int *free, unsigned int *total);
CUresult CUDAAPI cuMemAlloc(CUdeviceptr *dptr, unsigned int bytesize);
CUresult CUDAAPI cuMemAllocPitch(CUdeviceptr *dptr, unsigned int *pPitch, unsigned int WidthInBytes, unsigned int Height, unsigned int ElementSizeBytes);
CUresult CUDAAPI cuMemFree(CUdeviceptr dptr);
CUresult CUDAAPI cuMemGetAddressRange(CUdeviceptr *pbase, unsigned int *psize, CUdeviceptr dptr);
CUresult CUDAAPI cuMemAllocHost(void **pp, unsigned int bytesize);
CUresult CUDAAPI cuMemHostGetDevicePointer(CUdeviceptr *pdptr, void *p, unsigned int Flags);
CUresult CUDAAPI cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyDtoD(CUdeviceptr dstDevice, CUdeviceptr srcDevice, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyDtoA(CUarray dstArray, unsigned int dstOffset, CUdeviceptr srcDevice, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyAtoD(CUdeviceptr dstDevice, CUarray srcArray, unsigned int srcOffset, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyHtoA(CUarray dstArray, unsigned int dstOffset, const void *srcHost, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyAtoH(void *dstHost, CUarray srcArray, unsigned int srcOffset, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyAtoA(CUarray dstArray, unsigned int dstOffset, CUarray srcArray, unsigned int srcOffset, unsigned int ByteCount);
CUresult CUDAAPI cuMemcpyHtoAAsync(CUarray dstArray, unsigned int dstOffset, const void *srcHost, unsigned int ByteCount, CUstream hStream);
CUresult CUDAAPI cuMemcpyAtoHAsync(void *dstHost, CUarray srcArray, unsigned int srcOffset, unsigned int ByteCount, CUstream hStream);
CUresult CUDAAPI cuMemcpy2D(const CUDA_MEMCPY2D *pCopy);
CUresult CUDAAPI cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy);
CUresult CUDAAPI cuMemcpy3D(const CUDA_MEMCPY3D *pCopy);
CUresult CUDAAPI cuMemcpyHtoDAsync(CUdeviceptr dstDevice, const void *srcHost, unsigned int ByteCount, CUstream hStream);
CUresult CUDAAPI cuMemcpyDtoHAsync(void *dstHost, CUdeviceptr srcDevice, unsigned int ByteCount, CUstream hStream);
CUresult CUDAAPI cuMemcpyDtoDAsync(CUdeviceptr dstDevice, CUdeviceptr srcDevice, unsigned int ByteCount, CUstream hStream);
CUresult CUDAAPI cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream);
CUresult CUDAAPI cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream);
CUresult CUDAAPI cuMemsetD8(CUdeviceptr dstDevice, unsigned char uc, unsigned int N);
CUresult CUDAAPI cuMemsetD16(CUdeviceptr dstDevice, unsigned short us, unsigned int N);
CUresult CUDAAPI cuMemsetD32(CUdeviceptr dstDevice, unsigned int ui, unsigned int N);
CUresult CUDAAPI cuMemsetD2D8(CUdeviceptr dstDevice, unsigned int dstPitch, unsigned char uc, unsigned int Width, unsigned int Height);
CUresult CUDAAPI cuMemsetD2D16(CUdeviceptr dstDevice, unsigned int dstPitch, unsigned short us, unsigned int Width, unsigned int Height);
CUresult CUDAAPI cuMemsetD2D32(CUdeviceptr dstDevice, unsigned int dstPitch, unsigned int ui, unsigned int Width, unsigned int Height);
CUresult CUDAAPI cuArrayCreate(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray);
CUresult CUDAAPI cuArrayGetDescriptor(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
CUresult CUDAAPI cuArray3DCreate(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray);
CUresult CUDAAPI cuArray3DGetDescriptor(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
CUresult CUDAAPI cuTexRefSetAddress(unsigned int *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, unsigned int bytes);
CUresult CUDAAPI cuTexRefSetAddress2D(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, unsigned int Pitch);
CUresult CUDAAPI cuTexRefGetAddress(CUdeviceptr *pdptr, CUtexref hTexRef);
CUresult CUDAAPI cuGraphicsResourceGetMappedPointer(CUdeviceptr *pDevPtr, unsigned int *pSize, CUgraphicsResource resource);
#endif /* __CUDA_API_VERSION_INTERNAL || __CUDA_API_VERSION < 3020 */
#if defined(__CUDA_API_VERSION_INTERNAL) || __CUDA_API_VERSION < 4000
CUresult CUDAAPI cuCtxDestroy(CUcontext ctx);
CUresult CUDAAPI cuCtxPopCurrent(CUcontext *pctx);
CUresult CUDAAPI cuCtxPushCurrent(CUcontext ctx);
CUresult CUDAAPI cuStreamDestroy(CUstream hStream);
CUresult CUDAAPI cuEventDestroy(CUevent hEvent);
#endif /* __CUDA_API_VERSION_INTERNAL || __CUDA_API_VERSION < 4000 */

#if defined(__CUDA_API_VERSION_INTERNAL)
    #undef CUdeviceptr
    #undef CUDA_MEMCPY2D_st
    #undef CUDA_MEMCPY2D
    #undef CUDA_MEMCPY3D_st
    #undef CUDA_MEMCPY3D
    #undef CUDA_ARRAY_DESCRIPTOR_st
    #undef CUDA_ARRAY_DESCRIPTOR
    #undef CUDA_ARRAY3D_DESCRIPTOR_st
    #undef CUDA_ARRAY3D_DESCRIPTOR
#endif /* __CUDA_API_VERSION_INTERNAL */

#ifdef __cplusplus
}
#endif

#undef __CUDA_API_VERSION

#endif /* __cuda_cuda_h__ */

