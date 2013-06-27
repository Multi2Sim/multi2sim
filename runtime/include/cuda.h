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

#ifndef RUNTIME_INCLUDE_CUDA_H
#define RUNTIME_INCLUDE_CUDA_H

#include <stdlib.h>

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
#define cuTexRefGetAddress                  cuTexRefGetAddress_v2
#define cuGraphicsResourceGetMappedPointer  cuGraphicsResourceGetMappedPointer_v2
#define cuCtxDestroy                        cuCtxDestroy_v2
#define cuCtxPopCurrent                     cuCtxPopCurrent_v2
#define cuCtxPushCurrent                    cuCtxPushCurrent_v2
#define cuStreamDestroy                     cuStreamDestroy_v2
#define cuEventDestroy                      cuEventDestroy_v2
#define cuTexRefSetAddress2D                cuTexRefSetAddress2D_v3

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int CUdeviceptr;
typedef int CUdevice;
typedef struct CUctx_st *CUcontext;
typedef struct CUmod_st *CUmodule;
typedef struct CUfunc_st *CUfunction;
typedef struct CUarray_st *CUarray;
typedef struct CUmipmappedArray_st *CUmipmappedArray;
typedef struct CUtexref_st *CUtexref;
typedef struct CUsurfref_st *CUsurfref;
typedef struct CUevent_st *CUevent;
typedef struct CUstream_st *CUstream;
typedef struct CUgraphicsResource_st *CUgraphicsResource;
typedef unsigned long long CUtexObject;
typedef unsigned long long CUsurfObject;
typedef struct CUuuid_st {
    char bytes[16];
} CUuuid;
typedef struct CUipcEventHandle_st {
    char reserved[64];
} CUipcEventHandle;
typedef struct CUipcMemHandle_st {
    char reserved[64];
} CUipcMemHandle;
typedef enum CUipcMem_flags_enum {
    CU_IPC_MEM_LAZY_ENABLE_PEER_ACCESS = 0x1
} CUipcMem_flags;
typedef enum CUctx_flags_enum {
    CU_CTX_SCHED_AUTO = 0x00,
    CU_CTX_SCHED_SPIN = 0x01,
    CU_CTX_SCHED_YIELD = 0x02,
    CU_CTX_SCHED_BLOCKING_SYNC = 0x04,
    CU_CTX_BLOCKING_SYNC = 0x04,
    CU_CTX_SCHED_MASK = 0x07,
    CU_CTX_MAP_HOST = 0x08,
    CU_CTX_LMEM_RESIZE_TO_MAX = 0x10,
    CU_CTX_FLAGS_MASK = 0x1f
} CUctx_flags;
typedef enum CUstream_flags_enum {
    CU_STREAM_DEFAULT = 0x0,
    CU_STREAM_NON_BLOCKING = 0x1
} CUstream_flags;
typedef enum CUevent_flags_enum {
    CU_EVENT_DEFAULT = 0x0,
    CU_EVENT_BLOCKING_SYNC = 0x1,
    CU_EVENT_DISABLE_TIMING = 0x2,
    CU_EVENT_INTERPROCESS = 0x4
} CUevent_flags;
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
typedef enum CUaddress_mode_enum {
    CU_TR_ADDRESS_MODE_WRAP = 0,
    CU_TR_ADDRESS_MODE_CLAMP = 1,
    CU_TR_ADDRESS_MODE_MIRROR = 2,
    CU_TR_ADDRESS_MODE_BORDER = 3
} CUaddress_mode;
typedef enum CUfilter_mode_enum {
    CU_TR_FILTER_MODE_POINT = 0,
    CU_TR_FILTER_MODE_LINEAR = 1
} CUfilter_mode;
typedef enum CUdevice_attribute_enum {
    CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 1,
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X = 2,
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y = 3,
    CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z = 4,
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X = 5,
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y = 6,
    CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z = 7,
    CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK = 8,
    CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK = 8,
    CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY = 9,
    CU_DEVICE_ATTRIBUTE_WARP_SIZE = 10,
    CU_DEVICE_ATTRIBUTE_MAX_PITCH = 11,
    CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK = 12,
    CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK = 12,
    CU_DEVICE_ATTRIBUTE_CLOCK_RATE = 13,
    CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT = 14,
    CU_DEVICE_ATTRIBUTE_GPU_OVERLAP = 15,
    CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT = 16,
    CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT = 17,
    CU_DEVICE_ATTRIBUTE_INTEGRATED = 18,
    CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY = 19,
    CU_DEVICE_ATTRIBUTE_COMPUTE_MODE = 20,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH = 21,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH = 22,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT = 23,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH = 24,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT = 25,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH = 26,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH = 27,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT = 28,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS = 29,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_WIDTH = 27,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_HEIGHT = 28,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_NUMSLICES = 29,
    CU_DEVICE_ATTRIBUTE_SURFACE_ALIGNMENT = 30,
    CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS = 31,
    CU_DEVICE_ATTRIBUTE_ECC_ENABLED = 32,
    CU_DEVICE_ATTRIBUTE_PCI_BUS_ID = 33,
    CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID = 34,
    CU_DEVICE_ATTRIBUTE_TCC_DRIVER = 35,
    CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE = 36,
    CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH = 37,
    CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE = 38,
    CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR = 39,
    CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT = 40,
    CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING = 41,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH = 42,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS = 43,
    CU_DEVICE_ATTRIBUTE_CAN_TEX2D_GATHER = 44,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_WIDTH = 45,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_HEIGHT = 46,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH_ALTERNATE = 47,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT_ALTERNATE = 48,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH_ALTERNATE = 49,
    CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID = 50,
    CU_DEVICE_ATTRIBUTE_TEXTURE_PITCH_ALIGNMENT = 51,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_WIDTH = 52,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_WIDTH = 53,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_LAYERS = 54,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_WIDTH = 55,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_WIDTH = 56,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_HEIGHT = 57,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_WIDTH = 58,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_HEIGHT = 59,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_DEPTH = 60,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_WIDTH = 61,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_LAYERS = 62,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_WIDTH = 63,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_HEIGHT = 64,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_LAYERS = 65,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_WIDTH = 66,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_WIDTH = 67,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_LAYERS = 68,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LINEAR_WIDTH = 69,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_WIDTH = 70,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_HEIGHT = 71,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_PITCH = 72,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_WIDTH = 73,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_HEIGHT = 74,
    CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR = 75,
    CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR = 76,
    CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_MIPMAPPED_WIDTH = 77,
    CU_DEVICE_ATTRIBUTE_MAX
} CUdevice_attribute;
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
    int textureAlign;
} CUdevprop;
typedef enum CUpointer_attribute_enum {
    CU_POINTER_ATTRIBUTE_CONTEXT = 1,
    CU_POINTER_ATTRIBUTE_MEMORY_TYPE = 2,
    CU_POINTER_ATTRIBUTE_DEVICE_POINTER = 3,
    CU_POINTER_ATTRIBUTE_HOST_POINTER = 4,
    CU_POINTER_ATTRIBUTE_P2P_TOKENS = 5
} CUpointer_attribute;
typedef enum CUfunction_attribute_enum {
    CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 0,
    CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES = 1,
    CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES = 2,
    CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES = 3,
    CU_FUNC_ATTRIBUTE_NUM_REGS = 4,
    CU_FUNC_ATTRIBUTE_PTX_VERSION = 5,
    CU_FUNC_ATTRIBUTE_BINARY_VERSION = 6,
    CU_FUNC_ATTRIBUTE_MAX
} CUfunction_attribute;
typedef enum CUfunc_cache_enum {
    CU_FUNC_CACHE_PREFER_NONE = 0x00,
    CU_FUNC_CACHE_PREFER_SHARED = 0x01,
    CU_FUNC_CACHE_PREFER_L1 = 0x02,
    CU_FUNC_CACHE_PREFER_EQUAL = 0x03
} CUfunc_cache;
typedef enum CUsharedconfig_enum {
    CU_SHARED_MEM_CONFIG_DEFAULT_BANK_SIZE = 0x00,
    CU_SHARED_MEM_CONFIG_FOUR_BYTE_BANK_SIZE = 0x01,
    CU_SHARED_MEM_CONFIG_EIGHT_BYTE_BANK_SIZE = 0x02
} CUsharedconfig;
typedef enum CUmemorytype_enum {
    CU_MEMORYTYPE_HOST = 0x01,
    CU_MEMORYTYPE_DEVICE = 0x02,
    CU_MEMORYTYPE_ARRAY = 0x03,
    CU_MEMORYTYPE_UNIFIED = 0x04
} CUmemorytype;
typedef enum CUcomputemode_enum {
    CU_COMPUTEMODE_DEFAULT = 0,
    CU_COMPUTEMODE_EXCLUSIVE = 1,
    CU_COMPUTEMODE_PROHIBITED = 2,
    CU_COMPUTEMODE_EXCLUSIVE_PROCESS = 3
} CUcomputemode;
typedef enum CUjit_option_enum
{
    CU_JIT_MAX_REGISTERS = 0,
    CU_JIT_THREADS_PER_BLOCK,
    CU_JIT_WALL_TIME,
    CU_JIT_INFO_LOG_BUFFER,
    CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_ERROR_LOG_BUFFER,
    CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_OPTIMIZATION_LEVEL,
    CU_JIT_TARGET_FROM_CUCONTEXT,
    CU_JIT_TARGET,
    CU_JIT_FALLBACK_STRATEGY
} CUjit_option;
typedef enum CUjit_target_enum
{
    CU_TARGET_COMPUTE_10 = 0,
    CU_TARGET_COMPUTE_11,
    CU_TARGET_COMPUTE_12,
    CU_TARGET_COMPUTE_13,
    CU_TARGET_COMPUTE_20,
    CU_TARGET_COMPUTE_21,
    CU_TARGET_COMPUTE_30,
    CU_TARGET_COMPUTE_35
} CUjit_target;
typedef enum CUjit_fallback_enum
{
    CU_PREFER_PTX = 0,
    CU_PREFER_BINARY
} CUjit_fallback;
typedef enum CUgraphicsRegisterFlags_enum {
    CU_GRAPHICS_REGISTER_FLAGS_NONE = 0x00,
    CU_GRAPHICS_REGISTER_FLAGS_READ_ONLY = 0x01,
    CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD = 0x02,
    CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST = 0x04,
    CU_GRAPHICS_REGISTER_FLAGS_TEXTURE_GATHER = 0x08
} CUgraphicsRegisterFlags;
typedef enum CUgraphicsMapResourceFlags_enum {
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE = 0x00,
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_READ_ONLY = 0x01,
    CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITE_DISCARD = 0x02
} CUgraphicsMapResourceFlags;
typedef enum CUarray_cubemap_face_enum {
    CU_CUBEMAP_FACE_POSITIVE_X = 0x00,
    CU_CUBEMAP_FACE_NEGATIVE_X = 0x01,
    CU_CUBEMAP_FACE_POSITIVE_Y = 0x02,
    CU_CUBEMAP_FACE_NEGATIVE_Y = 0x03,
    CU_CUBEMAP_FACE_POSITIVE_Z = 0x04,
    CU_CUBEMAP_FACE_NEGATIVE_Z = 0x05
} CUarray_cubemap_face;
typedef enum CUlimit_enum {
    CU_LIMIT_STACK_SIZE = 0x00,
    CU_LIMIT_PRINTF_FIFO_SIZE = 0x01,
    CU_LIMIT_MALLOC_HEAP_SIZE = 0x02,
    CU_LIMIT_DEV_RUNTIME_SYNC_DEPTH = 0x03,
    CU_LIMIT_DEV_RUNTIME_PENDING_LAUNCH_COUNT = 0x04
} CUlimit;
typedef enum CUresourcetype_enum {
    CU_RESOURCE_TYPE_ARRAY = 0x00,
    CU_RESOURCE_TYPE_MIPMAPPED_ARRAY = 0x01,
    CU_RESOURCE_TYPE_LINEAR = 0x02,
    CU_RESOURCE_TYPE_PITCH2D = 0x03
} CUresourcetype;
typedef enum cudaError_enum {
    CUDA_SUCCESS = 0,
    CUDA_ERROR_INVALID_VALUE = 1,
    CUDA_ERROR_OUT_OF_MEMORY = 2,
    CUDA_ERROR_NOT_INITIALIZED = 3,
    CUDA_ERROR_DEINITIALIZED = 4,
    CUDA_ERROR_PROFILER_DISABLED = 5,
    CUDA_ERROR_PROFILER_NOT_INITIALIZED = 6,
    CUDA_ERROR_PROFILER_ALREADY_STARTED = 7,
    CUDA_ERROR_PROFILER_ALREADY_STOPPED = 8,
    CUDA_ERROR_NO_DEVICE = 100,
    CUDA_ERROR_INVALID_DEVICE = 101,
    CUDA_ERROR_INVALID_IMAGE = 200,
    CUDA_ERROR_INVALID_CONTEXT = 201,
    CUDA_ERROR_CONTEXT_ALREADY_CURRENT = 202,
    CUDA_ERROR_MAP_FAILED = 205,
    CUDA_ERROR_UNMAP_FAILED = 206,
    CUDA_ERROR_ARRAY_IS_MAPPED = 207,
    CUDA_ERROR_ALREADY_MAPPED = 208,
    CUDA_ERROR_NO_BINARY_FOR_GPU = 209,
    CUDA_ERROR_ALREADY_ACQUIRED = 210,
    CUDA_ERROR_NOT_MAPPED = 211,
    CUDA_ERROR_NOT_MAPPED_AS_ARRAY = 212,
    CUDA_ERROR_NOT_MAPPED_AS_POINTER = 213,
    CUDA_ERROR_ECC_UNCORRECTABLE = 214,
    CUDA_ERROR_UNSUPPORTED_LIMIT = 215,
    CUDA_ERROR_CONTEXT_ALREADY_IN_USE = 216,
    CUDA_ERROR_PEER_ACCESS_UNSUPPORTED = 217,
    CUDA_ERROR_INVALID_SOURCE = 300,
    CUDA_ERROR_FILE_NOT_FOUND = 301,
    CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND = 302,
    CUDA_ERROR_SHARED_OBJECT_INIT_FAILED = 303,
    CUDA_ERROR_OPERATING_SYSTEM = 304,
    CUDA_ERROR_INVALID_HANDLE = 400,
    CUDA_ERROR_NOT_FOUND = 500,
    CUDA_ERROR_NOT_READY = 600,
    CUDA_ERROR_LAUNCH_FAILED = 700,
    CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES = 701,
    CUDA_ERROR_LAUNCH_TIMEOUT = 702,
    CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING = 703,
    CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED = 704,
    CUDA_ERROR_PEER_ACCESS_NOT_ENABLED = 705,
    CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE = 708,
    CUDA_ERROR_CONTEXT_IS_DESTROYED = 709,
    CUDA_ERROR_ASSERT = 710,
    CUDA_ERROR_TOO_MANY_PEERS = 711,
    CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED = 712,
    CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED = 713,
    CUDA_ERROR_NOT_PERMITTED = 800,
    CUDA_ERROR_NOT_SUPPORTED = 801,
    CUDA_ERROR_UNKNOWN = 999
} CUresult;
typedef void ( *CUstreamCallback)(CUstream hStream, CUresult status, void *userData);
typedef struct CUDA_MEMCPY2D_st {
    size_t srcXInBytes;
    size_t srcY;
    CUmemorytype srcMemoryType;
    const void *srcHost;
    CUdeviceptr srcDevice;
    CUarray srcArray;
    size_t srcPitch;
    size_t dstXInBytes;
    size_t dstY;
    CUmemorytype dstMemoryType;
    void *dstHost;
    CUdeviceptr dstDevice;
    CUarray dstArray;
    size_t dstPitch;
    size_t WidthInBytes;
    size_t Height;
} CUDA_MEMCPY2D;
typedef struct CUDA_MEMCPY3D_st {
    size_t srcXInBytes;
    size_t srcY;
    size_t srcZ;
    size_t srcLOD;
    CUmemorytype srcMemoryType;
    const void *srcHost;
    CUdeviceptr srcDevice;
    CUarray srcArray;
    void *reserved0;
    size_t srcPitch;
    size_t srcHeight;
    size_t dstXInBytes;
    size_t dstY;
    size_t dstZ;
    size_t dstLOD;
    CUmemorytype dstMemoryType;
    void *dstHost;
    CUdeviceptr dstDevice;
    CUarray dstArray;
    void *reserved1;
    size_t dstPitch;
    size_t dstHeight;
    size_t WidthInBytes;
    size_t Height;
    size_t Depth;
} CUDA_MEMCPY3D;
typedef struct CUDA_MEMCPY3D_PEER_st {
    size_t srcXInBytes;
    size_t srcY;
    size_t srcZ;
    size_t srcLOD;
    CUmemorytype srcMemoryType;
    const void *srcHost;
    CUdeviceptr srcDevice;
    CUarray srcArray;
    CUcontext srcContext;
    size_t srcPitch;
    size_t srcHeight;
    size_t dstXInBytes;
    size_t dstY;
    size_t dstZ;
    size_t dstLOD;
    CUmemorytype dstMemoryType;
    void *dstHost;
    CUdeviceptr dstDevice;
    CUarray dstArray;
    CUcontext dstContext;
    size_t dstPitch;
    size_t dstHeight;
    size_t WidthInBytes;
    size_t Height;
    size_t Depth;
} CUDA_MEMCPY3D_PEER;
typedef struct CUDA_ARRAY_DESCRIPTOR_st
{
    size_t Width;
    size_t Height;
    CUarray_format Format;
    unsigned int NumChannels;
} CUDA_ARRAY_DESCRIPTOR;
typedef struct CUDA_ARRAY3D_DESCRIPTOR_st
{
    size_t Width;
    size_t Height;
    size_t Depth;
    CUarray_format Format;
    unsigned int NumChannels;
    unsigned int Flags;
} CUDA_ARRAY3D_DESCRIPTOR;
typedef struct CUDA_RESOURCE_DESC_st
{
    CUresourcetype resType;
    union {
        struct {
            CUarray hArray;
        } array;
        struct {
            CUmipmappedArray hMipmappedArray;
        } mipmap;
        struct {
            CUdeviceptr devPtr;
            CUarray_format format;
            unsigned int numChannels;
            size_t sizeInBytes;
        } linear;
        struct {
            CUdeviceptr devPtr;
            CUarray_format format;
            unsigned int numChannels;
            size_t width;
            size_t height;
            size_t pitchInBytes;
        } pitch2D;
        struct {
            int reserved[32];
        } reserved;
    } res;
    unsigned int flags;
} CUDA_RESOURCE_DESC;
typedef struct CUDA_TEXTURE_DESC_st
{
    CUaddress_mode addressMode[3];
    CUfilter_mode filterMode;
    unsigned int flags;
    unsigned int maxAnisotropy;
    CUfilter_mode mipmapFilterMode;
    float mipmapLevelBias;
    float minMipmapLevelClamp;
    float maxMipmapLevelClamp;
    int reserved[16];
} CUDA_TEXTURE_DESC;
typedef enum CUresourceViewFormat_enum
{
    CU_RES_VIEW_FORMAT_NONE = 0x00,
    CU_RES_VIEW_FORMAT_UINT_1X8 = 0x01,
    CU_RES_VIEW_FORMAT_UINT_2X8 = 0x02,
    CU_RES_VIEW_FORMAT_UINT_4X8 = 0x03,
    CU_RES_VIEW_FORMAT_SINT_1X8 = 0x04,
    CU_RES_VIEW_FORMAT_SINT_2X8 = 0x05,
    CU_RES_VIEW_FORMAT_SINT_4X8 = 0x06,
    CU_RES_VIEW_FORMAT_UINT_1X16 = 0x07,
    CU_RES_VIEW_FORMAT_UINT_2X16 = 0x08,
    CU_RES_VIEW_FORMAT_UINT_4X16 = 0x09,
    CU_RES_VIEW_FORMAT_SINT_1X16 = 0x0a,
    CU_RES_VIEW_FORMAT_SINT_2X16 = 0x0b,
    CU_RES_VIEW_FORMAT_SINT_4X16 = 0x0c,
    CU_RES_VIEW_FORMAT_UINT_1X32 = 0x0d,
    CU_RES_VIEW_FORMAT_UINT_2X32 = 0x0e,
    CU_RES_VIEW_FORMAT_UINT_4X32 = 0x0f,
    CU_RES_VIEW_FORMAT_SINT_1X32 = 0x10,
    CU_RES_VIEW_FORMAT_SINT_2X32 = 0x11,
    CU_RES_VIEW_FORMAT_SINT_4X32 = 0x12,
    CU_RES_VIEW_FORMAT_FLOAT_1X16 = 0x13,
    CU_RES_VIEW_FORMAT_FLOAT_2X16 = 0x14,
    CU_RES_VIEW_FORMAT_FLOAT_4X16 = 0x15,
    CU_RES_VIEW_FORMAT_FLOAT_1X32 = 0x16,
    CU_RES_VIEW_FORMAT_FLOAT_2X32 = 0x17,
    CU_RES_VIEW_FORMAT_FLOAT_4X32 = 0x18,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC1 = 0x19,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC2 = 0x1a,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC3 = 0x1b,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC4 = 0x1c,
    CU_RES_VIEW_FORMAT_SIGNED_BC4 = 0x1d,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC5 = 0x1e,
    CU_RES_VIEW_FORMAT_SIGNED_BC5 = 0x1f,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC6H = 0x20,
    CU_RES_VIEW_FORMAT_SIGNED_BC6H = 0x21,
    CU_RES_VIEW_FORMAT_UNSIGNED_BC7 = 0x22
} CUresourceViewFormat;
typedef struct CUDA_RESOURCE_VIEW_DESC_st
{
    CUresourceViewFormat format;
    size_t width;
    size_t height;
    size_t depth;
    unsigned int firstMipmapLevel;
    unsigned int lastMipmapLevel;
    unsigned int firstLayer;
    unsigned int lastLayer;
    unsigned int reserved[16];
} CUDA_RESOURCE_VIEW_DESC;
typedef struct CUDA_POINTER_ATTRIBUTE_P2P_TOKENS_st
{
    unsigned long long p2pToken;
    unsigned int vaSpaceToken;
} CUDA_POINTER_ATTRIBUTE_P2P_TOKENS;

CUresult cuInit(unsigned int Flags);
CUresult cuDriverGetVersion(int *driverVersion);
CUresult cuDeviceGet(CUdevice *device, int ordinal);
CUresult cuDeviceGetCount(int *count);
CUresult cuDeviceGetName(char *name, int len, CUdevice dev);
CUresult cuDeviceTotalMem_v2(size_t *bytes, CUdevice dev);
CUresult cuDeviceGetAttribute(int *pi, CUdevice_attribute attrib, CUdevice dev);
CUresult cuDeviceGetProperties(CUdevprop *prop, CUdevice dev);
CUresult cuDeviceComputeCapability(int *major, int *minor, CUdevice dev);
CUresult cuCtxCreate_v2(CUcontext *pctx, unsigned int flags, CUdevice dev);
CUresult cuCtxDestroy_v2(CUcontext ctx);
CUresult cuCtxPushCurrent_v2(CUcontext ctx);
CUresult cuCtxPopCurrent_v2(CUcontext *pctx);
CUresult cuCtxSetCurrent(CUcontext ctx);
CUresult cuCtxGetCurrent(CUcontext *pctx);
CUresult cuCtxGetDevice(CUdevice *device);
CUresult cuCtxSynchronize(void);
CUresult cuCtxSetLimit(CUlimit limit, size_t value);
CUresult cuCtxGetLimit(size_t *pvalue, CUlimit limit);
CUresult cuCtxGetCacheConfig(CUfunc_cache *pconfig);
CUresult cuCtxSetCacheConfig(CUfunc_cache config);
CUresult cuCtxGetSharedMemConfig(CUsharedconfig *pConfig);
CUresult cuCtxSetSharedMemConfig(CUsharedconfig config);
CUresult cuCtxGetApiVersion(CUcontext ctx, unsigned int *version);
CUresult cuCtxAttach(CUcontext *pctx, unsigned int flags);
CUresult cuCtxDetach(CUcontext ctx);
CUresult cuModuleLoad(CUmodule *module, const char *fname);
CUresult cuModuleLoadData(CUmodule *module, const void *image);
CUresult cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues);
CUresult cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin);
CUresult cuModuleUnload(CUmodule hmod);
CUresult cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name);
CUresult cuModuleGetGlobal_v2(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod, const char *name);
CUresult cuModuleGetTexRef(CUtexref *pTexRef, CUmodule hmod, const char *name);
CUresult cuModuleGetSurfRef(CUsurfref *pSurfRef, CUmodule hmod, const char *name);
CUresult cuMemGetInfo_v2(size_t *free, size_t *total);
CUresult cuMemAlloc_v2(CUdeviceptr *dptr, size_t bytesize);
CUresult cuMemAllocPitch_v2(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes);
CUresult cuMemFree_v2(CUdeviceptr dptr);
CUresult cuMemGetAddressRange_v2(CUdeviceptr *pbase, size_t *psize, CUdeviceptr dptr);
CUresult cuMemAllocHost_v2(void **pp, size_t bytesize);
CUresult cuMemFreeHost(void *p);
CUresult cuMemHostAlloc(void **pp, size_t bytesize, unsigned int Flags);
CUresult cuMemHostGetDevicePointer_v2(CUdeviceptr *pdptr, void *p, unsigned int Flags);
CUresult cuMemHostGetFlags(unsigned int *pFlags, void *p);
CUresult cuDeviceGetByPCIBusId(CUdevice *dev, char *pciBusId);
CUresult cuDeviceGetPCIBusId(char *pciBusId, int len, CUdevice dev);
CUresult cuIpcGetEventHandle(CUipcEventHandle *pHandle, CUevent event);
CUresult cuIpcOpenEventHandle(CUevent *phEvent, CUipcEventHandle handle);
CUresult cuIpcGetMemHandle(CUipcMemHandle *pHandle, CUdeviceptr dptr);
CUresult cuIpcOpenMemHandle(CUdeviceptr *pdptr, CUipcMemHandle handle, unsigned int Flags);
CUresult cuIpcCloseMemHandle(CUdeviceptr dptr);
CUresult cuMemHostRegister(void *p, size_t bytesize, unsigned int Flags);
CUresult cuMemHostUnregister(void *p);
CUresult cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount);
CUresult cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount);
CUresult cuMemcpyHtoD_v2(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount);
CUresult cuMemcpyDtoH_v2(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount);
CUresult cuMemcpyDtoD_v2(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount);
CUresult cuMemcpyDtoA_v2(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount);
CUresult cuMemcpyAtoD_v2(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount);
CUresult cuMemcpyHtoA_v2(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount);
CUresult cuMemcpyAtoH_v2(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount);
CUresult cuMemcpyAtoA_v2(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount);
CUresult cuMemcpy2D_v2(const CUDA_MEMCPY2D *pCopy);
CUresult cuMemcpy2DUnaligned_v2(const CUDA_MEMCPY2D *pCopy);
CUresult cuMemcpy3D_v2(const CUDA_MEMCPY3D *pCopy);
CUresult cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy);
CUresult cuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyHtoDAsync_v2(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyDtoHAsync_v2(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyDtoDAsync_v2(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyHtoAAsync_v2(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, CUstream hStream);
CUresult cuMemcpyAtoHAsync_v2(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream);
CUresult cuMemcpy2DAsync_v2(const CUDA_MEMCPY2D *pCopy, CUstream hStream);
CUresult cuMemcpy3DAsync_v2(const CUDA_MEMCPY3D *pCopy, CUstream hStream);
CUresult cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream);
CUresult cuMemsetD8_v2(CUdeviceptr dstDevice, unsigned char uc, size_t N);
CUresult cuMemsetD16_v2(CUdeviceptr dstDevice, unsigned short us, size_t N);
CUresult cuMemsetD32_v2(CUdeviceptr dstDevice, unsigned int ui, size_t N);
CUresult cuMemsetD2D8_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height);
CUresult cuMemsetD2D16_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height);
CUresult cuMemsetD2D32_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height);
CUresult cuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream);
CUresult cuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream);
CUresult cuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream);
CUresult cuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream);
CUresult cuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream);
CUresult cuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream);
CUresult cuArrayCreate_v2(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray);
CUresult cuArrayGetDescriptor_v2(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
CUresult cuArrayDestroy(CUarray hArray);
CUresult cuArray3DCreate_v2(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray);
CUresult cuArray3DGetDescriptor_v2(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
CUresult cuMipmappedArrayCreate(CUmipmappedArray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pMipmappedArrayDesc, unsigned int numMipmapLevels);
CUresult cuMipmappedArrayGetLevel(CUarray *pLevelArray, CUmipmappedArray hMipmappedArray, unsigned int level);
CUresult cuMipmappedArrayDestroy(CUmipmappedArray hMipmappedArray);
CUresult cuPointerGetAttribute(void *data, CUpointer_attribute attribute, CUdeviceptr ptr);
CUresult cuStreamCreate(CUstream *phStream, unsigned int Flags);
CUresult cuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags);
CUresult cuStreamAddCallback(CUstream hStream, CUstreamCallback callback, void *userData, unsigned int flags);
CUresult cuStreamQuery(CUstream hStream);
CUresult cuStreamSynchronize(CUstream hStream);
CUresult cuStreamDestroy_v2(CUstream hStream);
CUresult cuEventCreate(CUevent *phEvent, unsigned int Flags);
CUresult cuEventRecord(CUevent hEvent, CUstream hStream);
CUresult cuEventQuery(CUevent hEvent);
CUresult cuEventSynchronize(CUevent hEvent);
CUresult cuEventDestroy_v2(CUevent hEvent);
CUresult cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd);
CUresult cuFuncGetAttribute(int *pi, CUfunction_attribute attrib, CUfunction hfunc);
CUresult cuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config);
CUresult cuFuncSetSharedMemConfig(CUfunction hfunc, CUsharedconfig config);
CUresult cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra);
CUresult cuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z);
CUresult cuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes);
CUresult cuParamSetSize(CUfunction hfunc, unsigned int numbytes);
CUresult cuParamSeti(CUfunction hfunc, int offset, unsigned int value);
CUresult cuParamSetf(CUfunction hfunc, int offset, float value);
CUresult cuParamSetv(CUfunction hfunc, int offset, void *ptr, unsigned int numbytes);
CUresult cuLaunch(CUfunction f);
CUresult cuLaunchGrid(CUfunction f, int grid_width, int grid_height);
CUresult cuLaunchGridAsync(CUfunction f, int grid_width, int grid_height, CUstream hStream);
CUresult cuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef);
CUresult cuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags);
CUresult cuTexRefSetMipmappedArray(CUtexref hTexRef, CUmipmappedArray hMipmappedArray, unsigned int Flags);
CUresult cuTexRefSetAddress_v2(size_t *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes);
CUresult cuTexRefSetAddress2D_v3(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch);
CUresult cuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents);
CUresult cuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am);
CUresult cuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm);
CUresult cuTexRefSetMipmapFilterMode(CUtexref hTexRef, CUfilter_mode fm);
CUresult cuTexRefSetMipmapLevelBias(CUtexref hTexRef, float bias);
CUresult cuTexRefSetMipmapLevelClamp(CUtexref hTexRef, float minMipmapLevelClamp, float maxMipmapLevelClamp);
CUresult cuTexRefSetMaxAnisotropy(CUtexref hTexRef, unsigned int maxAniso);
CUresult cuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags);
CUresult cuTexRefGetAddress_v2(CUdeviceptr *pdptr, CUtexref hTexRef);
CUresult cuTexRefGetArray(CUarray *phArray, CUtexref hTexRef);
CUresult cuTexRefGetMipmappedArray(CUmipmappedArray *phMipmappedArray, CUtexref hTexRef);
CUresult cuTexRefGetAddressMode(CUaddress_mode *pam, CUtexref hTexRef, int dim);
CUresult cuTexRefGetFilterMode(CUfilter_mode *pfm, CUtexref hTexRef);
CUresult cuTexRefGetFormat(CUarray_format *pFormat, int *pNumChannels, CUtexref hTexRef);
CUresult cuTexRefGetMipmapFilterMode(CUfilter_mode *pfm, CUtexref hTexRef);
CUresult cuTexRefGetMipmapLevelBias(float *pbias, CUtexref hTexRef);
CUresult cuTexRefGetMipmapLevelClamp(float *pminMipmapLevelClamp, float *pmaxMipmapLevelClamp, CUtexref hTexRef);
CUresult cuTexRefGetMaxAnisotropy(int *pmaxAniso, CUtexref hTexRef);
CUresult cuTexRefGetFlags(unsigned int *pFlags, CUtexref hTexRef);
CUresult cuTexRefCreate(CUtexref *pTexRef);
CUresult cuTexRefDestroy(CUtexref hTexRef);
CUresult cuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags);
CUresult cuSurfRefGetArray(CUarray *phArray, CUsurfref hSurfRef);
CUresult cuTexObjectCreate(CUtexObject *pTexObject, const CUDA_RESOURCE_DESC *pResDesc, const CUDA_TEXTURE_DESC *pTexDesc, const CUDA_RESOURCE_VIEW_DESC *pResViewDesc);
CUresult cuTexObjectDestroy(CUtexObject texObject);
CUresult cuTexObjectGetResourceDesc(CUDA_RESOURCE_DESC *pResDesc, CUtexObject texObject);
CUresult cuTexObjectGetTextureDesc(CUDA_TEXTURE_DESC *pTexDesc, CUtexObject texObject);
CUresult cuTexObjectGetResourceViewDesc(CUDA_RESOURCE_VIEW_DESC *pResViewDesc, CUtexObject texObject);
CUresult cuSurfObjectCreate(CUsurfObject *pSurfObject, const CUDA_RESOURCE_DESC *pResDesc);
CUresult cuSurfObjectDestroy(CUsurfObject surfObject);
CUresult cuSurfObjectGetResourceDesc(CUDA_RESOURCE_DESC *pResDesc, CUsurfObject surfObject);
CUresult cuDeviceCanAccessPeer(int *canAccessPeer, CUdevice dev, CUdevice peerDev);
CUresult cuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags);
CUresult cuCtxDisablePeerAccess(CUcontext peerContext);
CUresult cuGraphicsUnregisterResource(CUgraphicsResource resource);
CUresult cuGraphicsSubResourceGetMappedArray(CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel);
CUresult cuGraphicsResourceGetMappedMipmappedArray(CUmipmappedArray *pMipmappedArray, CUgraphicsResource resource);
CUresult cuGraphicsResourceGetMappedPointer_v2(CUdeviceptr *pDevPtr, size_t *pSize, CUgraphicsResource resource);
CUresult cuGraphicsResourceSetMapFlags(CUgraphicsResource resource, unsigned int flags);
CUresult cuGraphicsMapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream);
CUresult cuGraphicsUnmapResources(unsigned int count, CUgraphicsResource *resources, CUstream hStream);
CUresult cuGetExportTable(const void **ppExportTable, const CUuuid *pExportTableId);

#ifdef __cplusplus
}
#endif

#endif

