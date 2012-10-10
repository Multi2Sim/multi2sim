/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:   
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and 
 * international Copyright laws.  Users and possessors of this source code 
 * are hereby granted a nonexclusive, royalty-free license to use this code 
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOURCE CODE.  
 *
 * U.S. Government End Users.   This source code is a "commercial item" as 
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of 
 * "commercial computer  software"  and "commercial computer software 
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995) 
 * and is provided to the U.S. Government only as a commercial end item.  
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein. 
 *
 * Any use of this source code in individual and commercial software must 
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

#if !defined(__CUDA_INTERNAL_COMPILATION__)

#define __CUDA_INTERNAL_COMPILATION__
#define __text__
#define __surf__
#define __name__shadow_var(c, cpp) \
        #cpp
#define __name__text_var(c, cpp) \
        #cpp
#define __host__shadow_var(c, cpp) \
        cpp
#define __text_var(c, cpp) \
        cpp
#define __device_fun(fun) \
        #fun
#define __device_var(var) \
        #var
#define __device__text_var(c, cpp) \
        #c
#define __device__shadow_var(c, cpp) \
        #c

#if defined(_WIN32) && !defined(_WIN64)

#define __pad__(f) \
        f

#else /* _WIN32 && !_WIN64 */

#define __pad__(f)

#endif /* _WIN32 && !_WIN64 */

#include "builtin_types.h"
#include "storage_class.h"

#else /* !__CUDA_INTERNAL_COMPILATION__ */

template <typename T>
static inline void *__cudaAddressOf(T &val) 
{
    return (void *)(&(const_cast<char &>(reinterpret_cast<const volatile char &>(val))));
}

#define __cudaRegisterBinary()                                                   \
        __cudaFatCubinHandle = __cudaRegisterFatBinary((void*)&__fatDeviceText); \
        atexit(__cudaUnregisterBinaryUtil)
#define __cudaRegisterVariable(var, ext, size, constant, global) \
        __cudaRegisterVar(__cudaFatCubinHandle, (char*)&__host##var, (char*)__device##var, __name##var, ext, size, constant, global)
#define __cudaRegisterGlobalTexture(tex, dim, norm, ext) \
        __cudaRegisterTexture(__cudaFatCubinHandle, (const struct textureReference*)&tex, (const void**)__device##tex, __name##tex, dim, norm, ext)
#define __cudaRegisterGlobalSurface(surf, dim, ext) \
        __cudaRegisterSurface(__cudaFatCubinHandle, (const struct surfaceReference*)&surf, (const void**)__device##surf, __name##surf, dim, ext)
#define __cudaRegisterEntry(funptr, fun, thread_limit) \
        __cudaRegisterFunction(__cudaFatCubinHandle, (const char*)funptr, (char*)__device_fun(fun), #fun, -1, (uint3*)0, (uint3*)0, (dim3*)0, (dim3*)0, (int*)0)
          
#define __cudaSetupArg(arg, offset) \
        if (cudaSetupArgument(__cudaAddressOf(arg), sizeof(arg), (size_t)offset) != cudaSuccess) \
          return
          
#define __cudaSetupArgSimple(arg, offset) \
        if (cudaSetupArgument((void *)(char *)&arg, sizeof(arg), (size_t)offset) != cudaSuccess) \
          return
          
#define __cudaLaunch(fun) \
        { volatile static char *__f; __f = fun; (void)cudaLaunch(fun); }

extern "C" {

extern void** CUDARTAPI __cudaRegisterFatBinary(
  void *fatCubin
);

extern void CUDARTAPI __cudaUnregisterFatBinary(
  void **fatCubinHandle
);

extern void CUDARTAPI __cudaRegisterVar(
        void **fatCubinHandle,
        char  *hostVar,
        char  *deviceAddress,
  const char  *deviceName,
        int    ext,
        int    size,
        int    constant,
        int    global
);

extern void CUDARTAPI __cudaRegisterTexture(
        void                    **fatCubinHandle,
  const struct textureReference  *hostVar,
  const void                    **deviceAddress,
  const char                     *deviceName,
        int                       dim,       
        int                       norm,      
        int                       ext        
);

extern void CUDARTAPI __cudaRegisterSurface(
        void                    **fatCubinHandle,
  const struct surfaceReference  *hostVar,
  const void                    **deviceAddress,
  const char                     *deviceName,
        int                       dim,       
        int                       ext        
);

extern void CUDARTAPI __cudaRegisterFunction(
        void   **fatCubinHandle,
  const char    *hostFun,
        char    *deviceFun,
  const char    *deviceName,
        int      thread_limit,
        uint3   *tid,
        uint3   *bid,
        dim3    *bDim,
        dim3    *gDim,
        int     *wSize
);

#if defined(__GNUC__)

extern int atexit(void(*)(void)) throw();

#else /* __GNUC__ */

extern int __cdecl atexit(void(__cdecl *)(void));

#endif /* __GNUC__ */

}

static void **__cudaFatCubinHandle;

static void __cdecl __cudaUnregisterBinaryUtil(void)
{
  __cudaUnregisterFatBinary(__cudaFatCubinHandle);
}

#include "common_functions.h"

#if defined(__APPLE__)

#pragma options align=natural

#else /* __APPLE__ */

#pragma pack()

#if defined(_WIN32)

#pragma warning(disable: 4099)

#if !defined(_WIN64)

#pragma warning(disable: 4408)

#endif /* !_WIN64 */

#endif /* _WIN32 */

#endif /* __APPLE__ */

#endif /* !__CUDA_INTERNAL_COMPILATION__ */
