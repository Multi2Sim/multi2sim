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

#ifndef RUNTIME_INCLUDE_CUDA_RUNTIME_H
#define RUNTIME_INCLUDE_CUDA_RUNTIME_H

#if !defined(__CUDA_RUNTIME_H__)
#define __CUDA_RUNTIME_H__

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

#include "host_config.h"

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

#include "builtin_types.h"
#include "channel_descriptor.h"
#include "cuda_runtime_api.h"
#include "driver_functions.h"
#include "host_defines.h"
#include "vector_functions.h"

#if defined(__CUDACC__)

#include "common_functions.h"
#include "cuda_surface_types.h"
#include "cuda_texture_types.h"
#include "device_functions.h"
#include "device_launch_parameters.h"

#endif /* __CUDACC__ */

#if defined(__cplusplus)

/*******************************************************************************
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/

namespace
{

/**
 * \addtogroup CUDART_HIGHLEVEL
 * @{
 */

/**
 * \brief \hl Configure a device launch
 *
 * Pushes \p size bytes of the argument pointed to by \p arg at \p offset
 * bytes from the start of the parameter passing area, which starts at
 * offset 0. The arguments are stored in the top of the execution stack.
 * \ref ::cudaSetupArgument(T, size_t) "cudaSetupArgument()" must be preceded
 * by a call to ::cudaConfigureCall().
 *
 * \param arg    - Argument to push for a kernel launch
 * \param offset - Offset in argument stack to push new arg
 *
 * \return
 * ::cudaSuccess
 * \notefnerr
 *
 * \sa ::cudaConfigureCall,
 * \ref ::cudaFuncGetAttributes(struct cudaFuncAttributes*, T*) "cudaFuncGetAttributes (C++ API)",
 * \ref ::cudaLaunch(T*) "cudaLaunch (C++ API)",
 * ::cudaSetDoubleForDevice,
 * ::cudaSetDoubleForHost,
 * \ref ::cudaSetupArgument(const void*, size_t, size_t) "cudaSetupArgument (C API)"
 */
template<class T>
__inline__ __host__ cudaError_t cudaSetupArgument(
  T      arg,
  size_t offset
)
{
  return ::cudaSetupArgument((const void*)&arg, sizeof(T), offset);
}

/**
 * \brief \hl Creates an event object with the specified flags
 *
 * Creates an event object with the specified flags. Valid flags include:
 * - ::cudaEventDefault: Default event creation flag.
 * - ::cudaEventBlockingSync: Specifies that event should use blocking
 *   synchronization. A host thread that uses ::cudaEventSynchronize() to wait
 *   on an event created with this flag will block until the event actually
 *   completes.
 * - ::cudaEventDisableTiming: Specifies that the created event does not need
 *   to record timing data.  Events created with this flag specified and
 *   the ::cudaEventBlockingSync flag not specified will provide the best
 *   performance when used with ::cudaStreamWaitEvent() and ::cudaEventQuery().
 *
 * \param event - Newly created event
 * \param flags - Flags for new event
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInitializationError,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorLaunchFailure,
 * ::cudaErrorMemoryAllocation
 * \notefnerr
 *
 * \sa \ref ::cudaEventCreate(cudaEvent_t*) "cudaEventCreate (C API)",
 * ::cudaEventCreateWithFlags, ::cudaEventRecord, ::cudaEventQuery,
 * ::cudaEventSynchronize, ::cudaEventDestroy, ::cudaEventElapsedTime,
 * ::cudaStreamWaitEvent
 */
static __inline__ __host__ cudaError_t cudaEventCreate(
  cudaEvent_t  *event,
  unsigned int  flags
)
{
  return ::cudaEventCreateWithFlags(event, flags);
}

/**
 * \brief \hl Allocates page-locked memory on the host
 *
 * Allocates \p size bytes of host memory that is page-locked and accessible
 * to the device. The driver tracks the virtual memory ranges allocated with
 * this function and automatically accelerates calls to functions such as
 * ::cudaMemcpy(). Since the memory can be accessed directly by the device, it
 * can be read or written with much higher bandwidth than pageable memory
 * obtained with functions such as ::malloc(). Allocating excessive amounts of
 * pinned memory may degrade system performance, since it reduces the amount
 * of memory available to the system for paging. As a result, this function is
 * best used sparingly to allocate staging areas for data exchange between host
 * and device.
 *
 * The \p flags parameter enables different options to be specified that affect
 * the allocation, as follows.
 * - ::cudaHostAllocDefault: This flag's value is defined to be 0.
 * - ::cudaHostAllocPortable: The memory returned by this call will be
 * considered as pinned memory by all CUDA contexts, not just the one that
 * performed the allocation.
 * - ::cudaHostAllocMapped: Maps the allocation into the CUDA address space.
 * The device pointer to the memory may be obtained by calling
 * ::cudaHostGetDevicePointer().
 * - ::cudaHostAllocWriteCombined: Allocates the memory as write-combined (WC).
 * WC memory can be transferred across the PCI Express bus more quickly on some
 * system configurations, but cannot be read efficiently by most CPUs.  WC
 * memory is a good option for buffers that will be written by the CPU and read
 * by the device via mapped pinned memory or host->device transfers.
 *
 * All of these flags are orthogonal to one another: a developer may allocate
 * memory that is portable, mapped and/or write-combined with no restrictions.
 *
 * ::cudaSetDeviceFlags() must have been called with the ::cudaDeviceMapHost
 * flag in order for the ::cudaHostAllocMapped flag to have any effect.
 *
 * The ::cudaHostAllocMapped flag may be specified on CUDA contexts for devices
 * that do not support mapped pinned memory. The failure is deferred to
 * ::cudaHostGetDevicePointer() because the memory may be mapped into other
 * CUDA contexts via the ::cudaHostAllocPortable flag.
 *
 * Memory allocated by this function must be freed with ::cudaFreeHost().
 *
 * \param ptr   - Device pointer to allocated memory
 * \param size  - Requested allocation size in bytes
 * \param flags - Requested properties of allocated memory
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorMemoryAllocation
 * \notefnerr
 *
 * \sa ::cudaSetDeviceFlags,
 * \ref ::cudaMallocHost(void**, size_t) "cudaMallocHost (C API)",
 * ::cudaFreeHost, ::cudaHostAlloc
 */
__inline__ __host__ cudaError_t cudaMallocHost(
  void         **ptr,
  size_t         size,
  unsigned int   flags
)
{
  return ::cudaHostAlloc(ptr, size, flags);
}

template<class T>
__inline__ __host__ cudaError_t cudaHostAlloc(
  T            **ptr,
  size_t         size,
  unsigned int   flags
)
{
  return ::cudaHostAlloc((void**)(void*)ptr, size, flags);
}

template<class T>
__inline__ __host__ cudaError_t cudaHostGetDevicePointer(
  T            **pDevice,
  void          *pHost,
  unsigned int   flags
)
{
  return ::cudaHostGetDevicePointer((void**)(void*)pDevice, pHost, flags);
}

template<class T>
__inline__ __host__ cudaError_t cudaMalloc(
  T      **devPtr,
  size_t   size
)
{
  return ::cudaMalloc((void**)(void*)devPtr, size);
}

template<class T>
__inline__ __host__ cudaError_t cudaMallocHost(
  T            **ptr,
  size_t         size,
  unsigned int   flags = 0
)
{
  return cudaMallocHost((void**)(void*)ptr, size, flags);
}

template<class T>
__inline__ __host__ cudaError_t cudaMallocPitch(
  T      **devPtr,
  size_t  *pitch,
  size_t   width,
  size_t   height
)
{
  return ::cudaMallocPitch((void**)(void*)devPtr, pitch, width, height);
}

#if defined(__CUDACC__)

/**
 * \brief \hl Copies data to the given symbol on the device
 *
 * Copies \p count bytes from the memory area pointed to by \p src
 * to the memory area \p offset bytes from the start of symbol
 * \p symbol. The memory areas may not overlap. \p symbol is a variable that
 * resides in global or constant memory space. \p kind can be either
 * ::cudaMemcpyHostToDevice or ::cudaMemcpyDeviceToDevice.
 *
 * \param symbol - Device symbol reference
 * \param src    - Source memory address
 * \param count  - Size in bytes to copy
 * \param offset - Offset from start of symbol in bytes
 * \param kind   - Type of transfer
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSymbol,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidMemcpyDirection
 * \notefnerr
 * \note_sync
 * \note_string_api_deprecation
 *
 * \sa ::cudaMemcpy, ::cudaMemcpy2D, ::cudaMemcpyToArray,
 * ::cudaMemcpy2DToArray, ::cudaMemcpyFromArray, ::cudaMemcpy2DFromArray,
 * ::cudaMemcpyArrayToArray, ::cudaMemcpy2DArrayToArray,
 * ::cudaMemcpyFromSymbol, ::cudaMemcpyAsync, ::cudaMemcpy2DAsync,
 * ::cudaMemcpyToArrayAsync, ::cudaMemcpy2DToArrayAsync,
 * ::cudaMemcpyFromArrayAsync, ::cudaMemcpy2DFromArrayAsync,
 * ::cudaMemcpyToSymbolAsync, ::cudaMemcpyFromSymbolAsync
 */
template<class T>
__inline__ __host__ cudaError_t cudaMemcpyToSymbol(
  const T                   &symbol,
  const void                *src,
        size_t               count,
        size_t               offset = 0,
        enum cudaMemcpyKind  kind   = cudaMemcpyHostToDevice
)
{
  return ::cudaMemcpyToSymbol((const void*)&symbol, src, count, offset, kind);
}

/**
 * \brief \hl Copies data to the given symbol on the device
 *
 * Copies \p count bytes from the memory area pointed to by \p src
 * to the memory area \p offset bytes from the start of symbol
 * \p symbol. The memory areas may not overlap. \p symbol is a variable that
 * resides in global or constant memory space. \p kind can be either
 * ::cudaMemcpyHostToDevice or ::cudaMemcpyDeviceToDevice.
 *
 * ::cudaMemcpyToSymbolAsync() is asynchronous with respect to the host, so
 * the call may return before the copy is complete. The copy can optionally
 * be associated to a stream by passing a non-zero \p stream argument. If
 * \p kind is ::cudaMemcpyHostToDevice and \p stream is non-zero, the copy
 * may overlap with operations in other streams.
 *
 * \param symbol - Device symbol reference
 * \param src    - Source memory address
 * \param count  - Size in bytes to copy
 * \param offset - Offset from start of symbol in bytes
 * \param kind   - Type of transfer
 * \param stream - Stream identifier
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSymbol,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidMemcpyDirection
 * \notefnerr
 * \note_async
 * \note_string_api_deprecation
 *
 * \sa ::cudaMemcpy, ::cudaMemcpy2D, ::cudaMemcpyToArray,
 * ::cudaMemcpy2DToArray, ::cudaMemcpyFromArray, ::cudaMemcpy2DFromArray,
 * ::cudaMemcpyArrayToArray, ::cudaMemcpy2DArrayToArray, ::cudaMemcpyToSymbol,
 * ::cudaMemcpyFromSymbol, ::cudaMemcpyAsync, ::cudaMemcpy2DAsync,
 * ::cudaMemcpyToArrayAsync, ::cudaMemcpy2DToArrayAsync,
 * ::cudaMemcpyFromArrayAsync, ::cudaMemcpy2DFromArrayAsync,
 * ::cudaMemcpyFromSymbolAsync
 */
template<class T>
__inline__ __host__ cudaError_t cudaMemcpyToSymbolAsync(
  const T                   &symbol,
  const void                *src,
        size_t               count,
        size_t               offset = 0,
        enum cudaMemcpyKind  kind   = cudaMemcpyHostToDevice,
        cudaStream_t         stream = 0
)
{
  return ::cudaMemcpyToSymbolAsync((const void*)&symbol, src, count, offset, kind, stream);
}

/**
 * \brief \hl Copies data from the given symbol on the device
 *
 * Copies \p count bytes from the memory area \p offset bytes
 * from the start of symbol \p symbol to the memory area pointed to by \p dst.
 * The memory areas may not overlap. \p symbol is a variable that
 * resides in global or constant memory space. \p kind can be either
 * ::cudaMemcpyDeviceToHost or ::cudaMemcpyDeviceToDevice.
 *
 * \param dst    - Destination memory address
 * \param symbol - Device symbol reference
 * \param count  - Size in bytes to copy
 * \param offset - Offset from start of symbol in bytes
 * \param kind   - Type of transfer
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSymbol,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidMemcpyDirection
 * \notefnerr
 * \note_sync
 * \note_string_api_deprecation
 *
 * \sa ::cudaMemcpy, ::cudaMemcpy2D, ::cudaMemcpyToArray,
 * ::cudaMemcpy2DToArray, ::cudaMemcpyFromArray, ::cudaMemcpy2DFromArray,
 * ::cudaMemcpyArrayToArray, ::cudaMemcpy2DArrayToArray, ::cudaMemcpyToSymbol,
 * ::cudaMemcpyAsync, ::cudaMemcpy2DAsync,
 * ::cudaMemcpyToArrayAsync, ::cudaMemcpy2DToArrayAsync,
 * ::cudaMemcpyFromArrayAsync, ::cudaMemcpy2DFromArrayAsync,
 * ::cudaMemcpyToSymbolAsync, ::cudaMemcpyFromSymbolAsync
 */
template<class T>
__inline__ __host__ cudaError_t cudaMemcpyFromSymbol(
        void                *dst,
  const T                   &symbol,
        size_t               count,
        size_t               offset = 0,
        enum cudaMemcpyKind  kind   = cudaMemcpyDeviceToHost
)
{
  return ::cudaMemcpyFromSymbol(dst, (const void*)&symbol, count, offset, kind);
}

/**
 * \brief \hl Copies data from the given symbol on the device
 *
 * Copies \p count bytes from the memory area \p offset bytes
 * from the start of symbol \p symbol to the memory area pointed to by \p dst.
 * The memory areas may not overlap. \p symbol is a variable that resides in
 * global or constant memory space. \p kind can be either
 * ::cudaMemcpyDeviceToHost or ::cudaMemcpyDeviceToDevice.
 *
 * ::cudaMemcpyFromSymbolAsync() is asynchronous with respect to the host, so
 * the call may return before the copy is complete. The copy can optionally be
 * associated to a stream by passing a non-zero \p stream argument. If \p kind
 * is ::cudaMemcpyDeviceToHost and \p stream is non-zero, the copy may overlap
 * with operations in other streams.
 *
 * \param dst    - Destination memory address
 * \param symbol - Device symbol reference
 * \param count  - Size in bytes to copy
 * \param offset - Offset from start of symbol in bytes
 * \param kind   - Type of transfer
 * \param stream - Stream identifier
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSymbol,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidMemcpyDirection
 * \notefnerr
 * \note_async
 * \note_string_api_deprecation
 *
 * \sa ::cudaMemcpy, ::cudaMemcpy2D, ::cudaMemcpyToArray,
 * ::cudaMemcpy2DToArray, ::cudaMemcpyFromArray, ::cudaMemcpy2DFromArray,
 * ::cudaMemcpyArrayToArray, ::cudaMemcpy2DArrayToArray, ::cudaMemcpyToSymbol,
 * ::cudaMemcpyFromSymbol, ::cudaMemcpyAsync, ::cudaMemcpy2DAsync,
 * ::cudaMemcpyToArrayAsync, ::cudaMemcpy2DToArrayAsync,
 * ::cudaMemcpyFromArrayAsync, ::cudaMemcpy2DFromArrayAsync,
 * ::cudaMemcpyToSymbolAsync
 */
template<class T>
__inline__ __host__ cudaError_t cudaMemcpyFromSymbolAsync(
        void                *dst,
  const T                   &symbol,
        size_t               count,
        size_t               offset = 0,
        enum cudaMemcpyKind  kind   = cudaMemcpyDeviceToHost,
        cudaStream_t         stream = 0
)
{
  return ::cudaMemcpyFromSymbolAsync(dst, (const void*)&symbol, count, offset, kind, stream);
}

/**
 * \brief \hl Finds the address associated with a CUDA symbol
 *
 * Returns in \p *devPtr the address of symbol \p symbol on the device.
 * \p symbol can either be a variable that resides in global or constant memory space.
 * If \p symbol cannot be found, or if \p symbol is not declared
 * in the global or constant memory space, \p *devPtr is unchanged and the error
 * ::cudaErrorInvalidSymbol is returned.
 *
 * \param devPtr - Return device pointer associated with symbol
 * \param symbol - Device symbol reference
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidSymbol
 * \notefnerr
 *
 * \sa \ref ::cudaGetSymbolAddress(void**, const void*) "cudaGetSymbolAddress (C API)",
 * \ref ::cudaGetSymbolSize(size_t*, const T&) "cudaGetSymbolSize (C++ API)"
 */
template<class T>
__inline__ __host__ cudaError_t cudaGetSymbolAddress(
        void **devPtr,
  const T     &symbol
)
{
  return ::cudaGetSymbolAddress(devPtr, (const void*)&symbol);
}

/**
 * \brief \hl Finds the size of the object associated with a CUDA symbol
 *
 * Returns in \p *size the size of symbol \p symbol. \p symbol must be a
 * variable that resides in global or constant memory space.
 * If \p symbol cannot be found, or if \p symbol is not declared
 * in global or constant memory space, \p *size is unchanged and the error
 * ::cudaErrorInvalidSymbol is returned.
 *
 * \param size   - Size of object associated with symbol
 * \param symbol - Device symbol reference
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidSymbol
 * \notefnerr
 *
 * \sa \ref ::cudaGetSymbolAddress(void**, const T&) "cudaGetSymbolAddress (C++ API)",
 * \ref ::cudaGetSymbolSize(size_t*, const void*) "cudaGetSymbolSize (C API)"
 */
template<class T>
__inline__ __host__ cudaError_t cudaGetSymbolSize(
        size_t *size,
  const T      &symbol
)
{
  return ::cudaGetSymbolSize(size, (const void*)&symbol);
}

/**
 * \brief \hl Binds a memory area to a texture
 *
 * Binds \p size bytes of the memory area pointed to by \p devPtr to texture
 * reference \p tex. \p desc describes how the memory is interpreted when
 * fetching values from the texture. The \p offset parameter is an optional
 * byte offset as with the low-level
 * \ref ::cudaBindTexture(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t) "cudaBindTexture()"
 * function. Any memory previously bound to \p tex is unbound.
 *
 * \param offset - Offset in bytes
 * \param tex    - Texture to bind
 * \param devPtr - Memory area on device
 * \param desc   - Channel format
 * \param size   - Size of the memory area pointed to by devPtr
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t) "cudaBindTexture (C API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode>&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTexture(
        size_t                           *offset,
  const struct texture<T, dim, readMode> &tex,
  const void                             *devPtr,
  const struct cudaChannelFormatDesc     &desc,
        size_t                            size = UINT_MAX
)
{
  return ::cudaBindTexture(offset, &tex, devPtr, &desc, size);
}

/**
 * \brief \hl Binds a memory area to a texture
 *
 * Binds \p size bytes of the memory area pointed to by \p devPtr to texture
 * reference \p tex. The channel descriptor is inherited from the texture
 * reference type. The \p offset parameter is an optional byte offset as with
 * the low-level
 * ::cudaBindTexture(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t)
 * function. Any memory previously bound to \p tex is unbound.
 *
 * \param offset - Offset in bytes
 * \param tex    - Texture to bind
 * \param devPtr - Memory area on device
 * \param size   - Size of the memory area pointed to by devPtr
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t) "cudaBindTexture (C API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode>&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTexture(
        size_t                           *offset,
  const struct texture<T, dim, readMode> &tex,
  const void                             *devPtr,
        size_t                            size = UINT_MAX
)
{
  return cudaBindTexture(offset, tex, devPtr, tex.channelDesc, size);
}

/**
 * \brief \hl Binds a 2D memory area to a texture
 *
 * Binds the 2D memory area pointed to by \p devPtr to the
 * texture reference \p tex. The size of the area is constrained by
 * \p width in texel units, \p height in texel units, and \p pitch in byte
 * units. \p desc describes how the memory is interpreted when fetching values
 * from the texture. Any memory previously bound to \p tex is unbound.
 *
 * Since the hardware enforces an alignment requirement on texture base
 * addresses,
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D()"
 * returns in \p *offset a byte offset that
 * must be applied to texture fetches in order to read from the desired memory.
 * This offset must be divided by the texel size and passed to kernels that
 * read from the texture so they can be applied to the ::tex2D() function.
 * If the device memory pointer was returned from ::cudaMalloc(), the offset is
 * guaranteed to be 0 and NULL may be passed as the \p offset parameter.
 *
 * \param offset - Offset in bytes
 * \param tex    - Texture reference to bind
 * \param devPtr - 2D memory area on device
 * \param desc   - Channel format
 * \param width  - Width in texel units
 * \param height - Height in texel units
 * \param pitch  - Pitch in bytes
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t, size_t, size_t) "cudaBindTexture2D (C API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode>&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTexture2D(
        size_t                           *offset,
  const struct texture<T, dim, readMode> &tex,
  const void                             *devPtr,
  const struct cudaChannelFormatDesc     &desc,
  size_t                                  width,
  size_t                                  height,
  size_t                                  pitch
)
{
  return ::cudaBindTexture2D(offset, &tex, devPtr, &desc, width, height, pitch);
}

/**
 * \brief \hl Binds a 2D memory area to a texture
 *
 * Binds the 2D memory area pointed to by \p devPtr to the
 * texture reference \p tex. The size of the area is constrained by
 * \p width in texel units, \p height in texel units, and \p pitch in byte
 * units. The channel descriptor is inherited from the texture reference
 * type. Any memory previously bound to \p tex is unbound.
 *
 * Since the hardware enforces an alignment requirement on texture base
 * addresses,
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D()"
 * returns in \p *offset a byte offset that
 * must be applied to texture fetches in order to read from the desired memory.
 * This offset must be divided by the texel size and passed to kernels that
 * read from the texture so they can be applied to the ::tex2D() function.
 * If the device memory pointer was returned from ::cudaMalloc(), the offset is
 * guaranteed to be 0 and NULL may be passed as the \p offset parameter.
 *
 * \param offset - Offset in bytes
 * \param tex    - Texture reference to bind
 * \param devPtr - 2D memory area on device
 * \param width  - Width in texel units
 * \param height - Height in texel units
 * \param pitch  - Pitch in bytes
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct textureReference*, const void*, const struct cudaChannelFormatDesc*, size_t, size_t, size_t) "cudaBindTexture2D (C API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode>&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTexture2D(
        size_t                           *offset,
  const struct texture<T, dim, readMode> &tex,
  const void                             *devPtr,
  size_t                                  width,
  size_t                                  height,
  size_t                                  pitch
)
{
  return ::cudaBindTexture2D(offset, &tex, devPtr, &tex.channelDesc, width, height, pitch);
}

/**
 * \brief \hl Binds an array to a texture
 *
 * Binds the CUDA array \p array to the texture reference \p tex.
 * \p desc describes how the memory is interpreted when fetching values from
 * the texture. Any CUDA array previously bound to \p tex is unbound.
 *
 * \param tex   - Texture to bind
 * \param array - Memory array on device
 * \param desc  - Channel format
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct textureReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindTextureToArray (C API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode >&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTextureToArray(
  const struct texture<T, dim, readMode> &tex,
  cudaArray_const_t                       array,
  const struct cudaChannelFormatDesc     &desc
)
{
  return ::cudaBindTextureToArray(&tex, array, &desc);
}

/**
 * \brief \hl Binds an array to a texture
 *
 * Binds the CUDA array \p array to the texture reference \p tex.
 * The channel descriptor is inherited from the CUDA array. Any CUDA array
 * previously bound to \p tex is unbound.
 *
 * \param tex   - Texture to bind
 * \param array - Memory array on device
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct textureReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindTextureToArray (C API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode >&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTextureToArray(
  const struct texture<T, dim, readMode> &tex,
  cudaArray_const_t                       array
)
{
  struct cudaChannelFormatDesc desc;
  cudaError_t                  err = ::cudaGetChannelDesc(&desc, array);

  return err == cudaSuccess ? cudaBindTextureToArray(tex, array, desc) : err;
}

/**
 * \brief \hl Binds a mipmapped array to a texture
 *
 * Binds the CUDA mipmapped array \p mipmappedArray to the texture reference \p tex.
 * \p desc describes how the memory is interpreted when fetching values from
 * the texture. Any CUDA mipmapped array previously bound to \p tex is unbound.
 *
 * \param tex            - Texture to bind
 * \param mipmappedArray - Memory mipmapped array on device
 * \param desc           - Channel format
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct textureReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindTextureToArray (C API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode >&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTextureToMipmappedArray(
  const struct texture<T, dim, readMode> &tex,
  cudaMipmappedArray_const_t              mipmappedArray,
  const struct cudaChannelFormatDesc     &desc
)
{
  return ::cudaBindTextureToMipmappedArray(&tex, mipmappedArray, &desc);
}

/**
 * \brief \hl Binds a mipmapped array to a texture
 *
 * Binds the CUDA mipmapped array \p mipmappedArray to the texture reference \p tex.
 * The channel descriptor is inherited from the CUDA array. Any CUDA mipmapped array
 * previously bound to \p tex is unbound.
 *
 * \param tex            - Texture to bind
 * \param mipmappedArray - Memory mipmapped array on device
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidDevicePointer,
 * ::cudaErrorInvalidTexture
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct textureReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindTextureToArray (C API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode >&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaBindTextureToMipmappedArray(
  const struct texture<T, dim, readMode> &tex,
  cudaMipmappedArray_const_t              mipmappedArray
)
{
  struct cudaChannelFormatDesc desc;
  cudaArray_t                  levelArray;
  cudaError_t                  err = ::cudaGetMipmappedArrayLevel(&levelArray, mipmappedArray, 0);
  
  if (err != cudaSuccess) {
      return err;
  }
  err = ::cudaGetChannelDesc(&desc, levelArray);

  return err == cudaSuccess ? cudaBindTextureToMipmappedArray(tex, mipmappedArray, desc) : err;
}

/**
 * \brief \hl Unbinds a texture
 *
 * Unbinds the texture bound to \p tex.
 *
 * \param tex - Texture to unbind
 *
 * \return ::cudaSuccess
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct textureReference*) "cudaUnbindTexture (C API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct texture<T, dim, readMode >&) "cudaGetTextureAlignmentOffset (C++ API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaUnbindTexture(
  const struct texture<T, dim, readMode> &tex
)
{
  return ::cudaUnbindTexture(&tex);
}

/**
 * \brief \hl Get the alignment offset of a texture
 *
 * Returns in \p *offset the offset that was returned when texture reference
 * \p tex was bound.
 *
 * \param offset - Offset of texture reference in bytes
 * \param tex    - Texture to get offset of
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidTexture,
 * ::cudaErrorInvalidTextureBinding
 * \notefnerr
 *
 * \sa \ref ::cudaCreateChannelDesc(void) "cudaCreateChannelDesc (C++ API)",
 * ::cudaGetChannelDesc, ::cudaGetTextureReference,
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t) "cudaBindTexture (C++ API)",
 * \ref ::cudaBindTexture(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t) "cudaBindTexture (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, const struct cudaChannelFormatDesc&, size_t, size_t, size_t) "cudaBindTexture2D (C++ API)",
 * \ref ::cudaBindTexture2D(size_t*, const struct texture<T, dim, readMode>&, const void*, size_t, size_t, size_t) "cudaBindTexture2D (C++ API, inherited channel descriptor)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindTextureToArray (C++ API)",
 * \ref ::cudaBindTextureToArray(const struct texture<T, dim, readMode>&, cudaArray_const_t) "cudaBindTextureToArray (C++ API, inherited channel descriptor)",
 * \ref ::cudaUnbindTexture(const struct texture<T, dim, readMode>&) "cudaUnbindTexture (C++ API)",
 * \ref ::cudaGetTextureAlignmentOffset(size_t*, const struct textureReference*) "cudaGetTextureAlignmentOffset (C API)"
 */
template<class T, int dim, enum cudaTextureReadMode readMode>
__inline__ __host__ cudaError_t cudaGetTextureAlignmentOffset(
        size_t                           *offset,
  const struct texture<T, dim, readMode> &tex
)
{
  return ::cudaGetTextureAlignmentOffset(offset, &tex);
}

/**
 * \brief \hl Sets the preferred cache configuration for a device function
 *
 * On devices where the L1 cache and shared memory use the same hardware
 * resources, this sets through \p cacheConfig the preferred cache configuration
 * for the function specified via \p func. This is only a preference. The
 * runtime will use the requested configuration if possible, but it is free to
 * choose a different configuration if required to execute \p func.
 *
 * \p func must be a pointer to a function that executes on the device.
 * The parameter specified by \p func must be declared as a \p __global__
 * function. If the specified function does not exist,
 * then ::cudaErrorInvalidDeviceFunction is returned.
 *
 * This setting does nothing on devices where the size of the L1 cache and
 * shared memory are fixed.
 *
 * Launching a kernel with a different preference than the most recent
 * preference setting may insert a device-side synchronization point.
 *
 * The supported cache configurations are:
 * - ::cudaFuncCachePreferNone: no preference for shared memory or L1 (default)
 * - ::cudaFuncCachePreferShared: prefer larger shared memory and smaller L1 cache
 * - ::cudaFuncCachePreferL1: prefer larger L1 cache and smaller shared memory
 *
 * \param func        - device function pointer
 * \param cacheConfig - Requested cache configuration
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInitializationError,
 * ::cudaErrorInvalidDeviceFunction
 * \notefnerr
 *
 * \sa ::cudaConfigureCall,
 * \ref ::cudaFuncSetCacheConfig(const void*, enum cudaFuncCache) "cudaFuncSetCacheConfig (C API)",
 * \ref ::cudaFuncGetAttributes(struct cudaFuncAttributes*, T*) "cudaFuncGetAttributes (C++ API)",
 * \ref ::cudaLaunch(const void*) "cudaLaunch (C API)",
 * ::cudaSetDoubleForDevice,
 * ::cudaSetDoubleForHost,
 * \ref ::cudaSetupArgument(T, size_t) "cudaSetupArgument (C++ API)",
 * ::cudaThreadGetCacheConfig,
 * ::cudaThreadSetCacheConfig
 */
template<class T>
__inline__ __host__ cudaError_t cudaFuncSetCacheConfig(
  T                  *func,
  enum cudaFuncCache  cacheConfig
)
{
  return ::cudaFuncSetCacheConfig((const void*)func, cacheConfig);
}

template<class T>
__inline__ __host__ cudaError_t cudaFuncSetSharedMemConfig(
  T                        *func,
  enum cudaSharedMemConfig  config
)
{
  return ::cudaFuncSetSharedMemConfig((const void*)func, config);
}

/**
 * \brief \hl Launches a device function
 *
 * Launches the function \p entry on the device. The parameter \p entry must
 * be a function that executes on the device. The parameter specified by \p entry
 * must be declared as a \p __global__ function.
 * \ref ::cudaLaunch(T*) "cudaLaunch()" must be preceded by a call to
 * ::cudaConfigureCall() since it pops the data that was pushed by
 * ::cudaConfigureCall() from the execution stack.
 *
 * \param entry - Device function pointer
 * to execute
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidDeviceFunction,
 * ::cudaErrorInvalidConfiguration,
 * ::cudaErrorLaunchFailure,
 * ::cudaErrorLaunchTimeout,
 * ::cudaErrorLaunchOutOfResources,
 * ::cudaErrorSharedObjectSymbolNotFound,
 * ::cudaErrorSharedObjectInitFailed
 * \notefnerr
 *
 * \sa ::cudaConfigureCall,
 * \ref ::cudaFuncSetCacheConfig(T*, enum cudaFuncCache) "cudaFuncSetCacheConfig (C++ API)",
 * \ref ::cudaFuncGetAttributes(struct cudaFuncAttributes*, T*) "cudaFuncGetAttributes (C++ API)",
 * \ref ::cudaLaunch(const void*) "cudaLaunch (C API)",
 * ::cudaSetDoubleForDevice,
 * ::cudaSetDoubleForHost,
 * \ref ::cudaSetupArgument(T, size_t) "cudaSetupArgument (C++ API)",
 * ::cudaThreadGetCacheConfig,
 * ::cudaThreadSetCacheConfig
 */
template<class T>
__inline__ __host__ cudaError_t cudaLaunch(
  T *func
)
{
  return ::cudaLaunch((const void*)func);
}

/**
 * \brief \hl Find out attributes for a given function
 *
 * This function obtains the attributes of a function specified via \p entry.
 * The parameter \p entry must be a pointer to a function that executes
 * on the device. The parameter specified by \p entry must be declared as a \p __global__
 * function. The fetched attributes are placed in \p attr. If the specified
 * function does not exist, then ::cudaErrorInvalidDeviceFunction is returned.
 *
 * Note that some function attributes such as
 * \ref ::cudaFuncAttributes::maxThreadsPerBlock "maxThreadsPerBlock"
 * may vary based on the device that is currently being used.
 *
 * \param attr  - Return pointer to function's attributes
 * \param entry - Function to get attributes of
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInitializationError,
 * ::cudaErrorInvalidDeviceFunction
 * \notefnerr
 *
 * \sa ::cudaConfigureCall,
 * \ref ::cudaFuncSetCacheConfig(T*, enum cudaFuncCache) "cudaFuncSetCacheConfig (C++ API)",
 * \ref ::cudaFuncGetAttributes(struct cudaFuncAttributes*, const void*) "cudaFuncGetAttributes (C API)",
 * \ref ::cudaLaunch(T*) "cudaLaunch (C++ API)",
 * ::cudaSetDoubleForDevice,
 * ::cudaSetDoubleForHost,
 * \ref ::cudaSetupArgument(T, size_t) "cudaSetupArgument (C++ API)"
 */
template<class T>
__inline__ __host__ cudaError_t cudaFuncGetAttributes(
  struct cudaFuncAttributes *attr,
  T                         *entry
)
{
  return ::cudaFuncGetAttributes(attr, (const void*)entry);
}

/**
 * \brief \hl Binds an array to a surface
 *
 * Binds the CUDA array \p array to the surface reference \p surf.
 * \p desc describes how the memory is interpreted when dealing with
 * the surface. Any CUDA array previously bound to \p surf is unbound.
 *
 * \param surf  - Surface to bind
 * \param array - Memory array on device
 * \param desc  - Channel format
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSurface
 * \notefnerr
 *
 * \sa \ref ::cudaBindSurfaceToArray(const struct surfaceReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindSurfaceToArray (C API)",
 * \ref ::cudaBindSurfaceToArray(const struct surface<T, dim>&, cudaArray_const_t) "cudaBindSurfaceToArray (C++ API, inherited channel descriptor)"
 */
template<class T, int dim>
__inline__ __host__ cudaError_t cudaBindSurfaceToArray(
  const struct surface<T, dim>       &surf,
  cudaArray_const_t                   array,
  const struct cudaChannelFormatDesc &desc
)
{
  return ::cudaBindSurfaceToArray(&surf, array, &desc);
}

/**
 * \brief \hl Binds an array to a surface
 *
 * Binds the CUDA array \p array to the surface reference \p surf.
 * The channel descriptor is inherited from the CUDA array. Any CUDA array
 * previously bound to \p surf is unbound.
 *
 * \param surf  - Surface to bind
 * \param array - Memory array on device
 *
 * \return
 * ::cudaSuccess,
 * ::cudaErrorInvalidValue,
 * ::cudaErrorInvalidSurface
 * \notefnerr
 *
 * \sa \ref ::cudaBindSurfaceToArray(const struct surfaceReference*, cudaArray_const_t, const struct cudaChannelFormatDesc*) "cudaBindSurfaceToArray (C API)",
 * \ref ::cudaBindSurfaceToArray(const struct surface<T, dim>&, cudaArray_const_t, const struct cudaChannelFormatDesc&) "cudaBindSurfaceToArray (C++ API)"
 */
template<class T, int dim>
__inline__ __host__ cudaError_t cudaBindSurfaceToArray(
  const struct surface<T, dim> &surf,
  cudaArray_const_t             array
)
{
  struct cudaChannelFormatDesc desc;
  cudaError_t                  err = ::cudaGetChannelDesc(&desc, array);

  return err == cudaSuccess ? cudaBindSurfaceToArray(surf, array, desc) : err;
}

#endif /* __CUDACC__ */

/** @} */ /* END CUDART_HIGHLEVEL */

} // namespace anonymous

#endif /* __cplusplus */

#endif /* !__CUDA_RUNTIME_H__ */

#endif

