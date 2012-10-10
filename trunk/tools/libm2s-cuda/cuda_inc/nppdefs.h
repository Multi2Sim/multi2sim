 /* Copyright 2008-2011 NVIDIA Corporation.  All rights reserved.
  *
  * NOTICE TO LICENSEE:
  *
  * The source code and/or documentation ("Licensed Deliverables") are
  * subject to NVIDIA intellectual property rights under U.S. and
  * international Copyright laws.
  *
  * The Licensed Deliverables contained herein are PROPRIETARY and
  * CONFIDENTIAL to NVIDIA and are being provided under the terms and
  * conditions of a form of NVIDIA software license agreement by and
  * between NVIDIA and Licensee ("License Agreement") or electronically
  * accepted by Licensee.  Notwithstanding any terms or conditions to
  * the contrary in the License Agreement, reproduction or disclosure
  * of the Licensed Deliverables to any third party without the express
  * written consent of NVIDIA is prohibited.
  *
  * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
  * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
  * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  THEY ARE
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
  * C.F.R. 12.212 (SEPT 1995) and are provided to the U.S. Government
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
#ifndef NV_NPPIDEFS_H
#define NV_NPPIDEFS_H

#include <stdlib.h>

/**
 * \file nppdefs.h
 * Typedefinitions and macros for NPP library.
 */
 

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup typedefs_npp NPP Type Definitions and Constants
 * @{
 */

/** 
 * Filtering methods.
 */
typedef enum 
{
    NPPI_INTER_UNDEFINED    = 0,
    NPPI_INTER_NN           = 1,         ///< Nearest neighbor filtering.
    NPPI_INTER_LINEAR       = 2,         ///< Linear interpolation.
    NPPI_INTER_CUBIC        = 4,         ///< Cubic interpolation.
    NPPI_INTER_SUPER        = 8,         ///< Super sampling.
    NPPI_INTER_LANCZOS      = 16,        ///< Lanczos filtering.
    NPPI_SMOOTH_EDGE        = (1 << 31)  ///< Smooth edge filtering.
} NppiInterpolationMode; 

/**
 * Error Status Codes
 *
 * Almost all NPP function return error-status information using
 * these return codes.
 * Negative return codes indicate errors, positive return codes indicate
 * warnings, a return code of 0 indicates success.
 */
typedef enum 
{
    // negative return-codes indicate errors
    NPP_NOT_SUPPORTED_MODE_ERROR            = -9999,  
    NPP_ROUND_MODE_NOT_SUPPORTED_ERROR      = -213,
    NPP_RESIZE_NO_OPERATION_ERROR           = -50,
    NPP_NOT_SUFFICIENT_COMPUTE_CAPABILITY   = -27,
    NPP_BAD_ARG_ERROR                       = -26,
    NPP_LUT_NUMBER_OF_LEVELS_ERROR          = -25,
    NPP_TEXTURE_BIND_ERROR                  = -24,
    NPP_COEFF_ERROR                         = -23,
    NPP_RECT_ERROR                          = -22,
    NPP_QUAD_ERROR                          = -21,
    NPP_WRONG_INTERSECTION_ROI_ERROR        = -20,
    NPP_NOT_EVEN_STEP_ERROR                 = -19,
    NPP_INTERPOLATION_ERROR                 = -18,
    NPP_RESIZE_FACTOR_ERROR                 = -17,
    NPP_HAAR_CLASSIFIER_PIXEL_MATCH_ERROR   = -16,
    NPP_MEMFREE_ERR                         = -15,
    NPP_MEMSET_ERR                          = -14,
    NPP_MEMCPY_ERROR                        = -13,
    NPP_MEM_ALLOC_ERR                       = -12,
    NPP_HISTO_NUMBER_OF_LEVELS_ERROR        = -11,
    NPP_MIRROR_FLIP_ERR                     = -10,
    NPP_INVALID_INPUT                       = -9,
    NPP_ALIGNMENT_ERROR                     = -8,
    NPP_STEP_ERROR                          = -7,       ///< Step is less or equal zero
    NPP_SIZE_ERROR                          = -6,
    NPP_POINTER_ERROR                       = -5,
    NPP_NULL_POINTER_ERROR                  = -4,
    NPP_CUDA_KERNEL_EXECUTION_ERROR         = -3,
    NPP_NOT_IMPLEMENTED_ERROR               = -2,
    NPP_ERROR                               = -1,
    
    // success
    NPP_NO_ERROR                            = 0,        ///< Error free operation
    NPP_SUCCESS = NPP_NO_ERROR,                         ///< Successful operation (same as NPP_NO_ERROR)

    // positive return-codes indicate warnings
    NPP_WARNING                             = 1,
    NPP_WRONG_INTERSECTION_QUAD_WARNING     = 2,
    NPP_MISALIGNED_DST_ROI_WARNING          = 3,        ///< Speed reduction due to uncoalesced memory accesses warning.
    NPP_AFFINE_QUAD_INCORRECT_WARNING       = 4,        ///< Indicates that the quadrangle passed to one of affine warping functions doesn't have necessary properties. First 3 vertices are used, the fourth vertex discarded.
    NPP_DOUBLE_SIZE_WARNING                 = 5,        ///< Indicates that in case of 422/411/420 sampling the ROI width/height was modified for proper processing.
    NPP_ODD_ROI_WARNING                     = 6,        ///< Indicates that for 422/411/420 sampling the ROI width/height was forced to even value.
} NppStatus;

typedef enum
{
    NPP_CUDA_UNKNOWN_VERSION = -1, ///< Indicates that the compute-capability query failed
    NPP_CUDA_NOT_CAPABLE,          ///< Indicates that no CUDA capable device was found on machine
    NPP_CUDA_1_0,                  ///< Indicates that CUDA 1.0 capable device is default device on machine
    NPP_CUDA_1_1,                  ///< Indicates that CUDA 1.1 capable device
    NPP_CUDA_1_2,                  ///< Indicates that CUDA 1.2 capable device
    NPP_CUDA_1_3,                  ///< Indicates that CUDA 1.3 capable device
    NPP_CUDA_2_0                   ///< Indicates that CUDA 2.0 or better is default device on machine
} NppGpuComputeCapability;

typedef struct 
{
    int    major;   ///< Major version number
    int    minor;   ///< Minor version number
    int    build;   ///< Build number. This reflects the nightly build this release was made from.
} NppLibraryVersion;

/** \defgroup npp_basic_types Basic NPP Data Types
 * @{
 */


typedef unsigned char       Npp8u;     ///< 8-bit unsigned chars
typedef signed char         Npp8s;     ///< 8-bit signed chars
typedef unsigned short      Npp16u;    ///< 16-bit unsigned integers
typedef short               Npp16s;    ///< 16-bit signed integers
typedef unsigned int        Npp32u;    ///< 32-bit unsigned integers
typedef int                 Npp32s;    ///< 32-bit signed integers
typedef unsigned long long  Npp64u;    ///< 64-bit unsigned integers
typedef long long           Npp64s;    ///< 64-bit signed integers
typedef float               Npp32f;    ///< 32-bit (IEEE) floating-point numbers
typedef double              Npp64f;    ///< 64-bit floating-point numbers

/**
 * Complex Number
 * This struct represents a short complex number.
 */
typedef struct 
{
    Npp16s  re;     ///< Real part
    Npp16s  im;     ///< Imaginary part
} Npp16sc;

/**
 * Complex Number
 * This struct represents a signed int complex number.
 */
typedef struct 
{
    Npp32s  re;     ///< Real part
    Npp32s  im;     ///< Imaginary part
} Npp32sc;

/**
 * Complex Number
 * This struct represents a single floating-point complex number.
 */
typedef struct 
{
    Npp32f  re;     ///< Real part
    Npp32f  im;     ///< Imaginary part
} Npp32fc;

/**
 * Complex Number
 * This struct represents a long long complex number.
 */
typedef struct 
{
    Npp64s  re;     ///< Real part
    Npp64s  im;     ///< Imaginary part
} Npp64sc;

/**
 * Complex Number
 * This struct represents a double floating-point complex number.
 */
typedef struct 
{
    Npp64f  re;     ///< Real part
    Npp64f  im;     ///< Imaginary part
} Npp64fc;

/*@}*/

#define NPP_MIN_8U      ( 0 )                       ///< Minimum 8-bit unsigned integer
#define NPP_MAX_8U      ( 255 )                     ///< Maximum 8-bit unsigned integer
#define NPP_MIN_16U     ( 0 )                       ///< Minimum 16-bit unsigned integer
#define NPP_MAX_16U     ( 65535 )                   ///< Maximum 16-bit unsigned integer
#define NPP_MIN_32U     ( 0 )                       ///< Minimum 32-bit unsigned integer
#define NPP_MAX_32U     ( 4294967295 )              ///< Maximum 32-bit unsigned integer

#define NPP_MIN_8S      (-128 )                     ///< Minimum 8-bit signed integer
#define NPP_MAX_8S      ( 127 )                     ///< Maximum 8-bit signed integer
#define NPP_MIN_16S     (-32768 )                   ///< Minimum 16-bit signed integer
#define NPP_MAX_16S     ( 32767 )                   ///< Maximum 16-bit signed integer
#define NPP_MIN_32S     (-2147483648 )              ///< Minimum 32-bit signed integer
#define NPP_MAX_32S     ( 2147483647 )              ///< Maximum 32-bit signed integer
#define NPP_MAX_64S     ( 9223372036854775807LL )   ///< Minimum 64-bit signed integer
#define NPP_MIN_64S     (-9223372036854775808LL)    ///< Maximum 64-bit signed integer

#define NPP_MINABS_32F ( 1.175494351e-38f )         ///< Smallest positive 32-bit floating point value
#define NPP_MAXABS_32F ( 3.402823466e+38f )         ///< Largest  positive 32-bit floating point value
#define NPP_MINABS_64F ( 2.2250738585072014e-308 )  ///< Smallest positive 64-bit floating point value
#define NPP_MAXABS_64F ( 1.7976931348623158e+308 )  ///< Largest  positive 64-bit floating point value


/** 
 * 2D Point
 */
typedef struct 
{
    int x;      ///< x-coordinate.
    int y;      ///< y-coordinate.
} NppiPoint;

/**
 * 2D Size
 * This struct typically represents the size of a a rectangular region in
 * two space.
 */
typedef struct 
{
    int width;  ///< Rectangle width.
    int height; ///< Rectangle height.
} NppiSize;

/**
 * 2D Rectangle
 * This struct contains position and size information of a rectangle in 
 * two space.
 * The rectangle's position is usually signified by the coordinate of its
 * upper-left corner.
 */
typedef struct
{
    int x;          ///< x-coordinate of upper left corner.
    int y;          ///< y-coordinate of upper left corner.
    int width;      ///< Rectangle width.
    int height;     ///< Rectangle height.
} NppiRect;

typedef enum 
{
    NPP_HORIZONTAL_AXIS,
    NPP_VERTICAL_AXIS,
    NPP_BOTH_AXIS
} NppiAxis;

typedef enum 
{
    NPP_CMP_LESS,
    NPP_CMP_LESS_EQ,
    NPP_CMP_EQ,
    NPP_CMP_GREATER_EQ,
    NPP_CMP_GREATER
} NppCmpOp;

typedef enum 
{
    NPP_RND_ZERO,
    NPP_RND_NEAR,
    NPP_RND_FINANCIAL
} NppRoundMode;


typedef struct
{
    int      numClassifiers;    ///< number of classifiers
    Npp32s * classifiers;       ///< packed classifier data 40 bytes each
    size_t   classifierStep;
    NppiSize classifierSize;
    Npp32s * counterDevice;
} NppiHaarClassifier_32f;

typedef struct
{
    int      haarBufferSize;    ///< size of the buffer
    Npp32s * haarBuffer;        ///< buffer
    
} NppiHaarBuffer;

typedef enum {
    nppAlgHintNone,
    nppAlgHintFast,
    nppAlgHintAccurate
} NppHintAlgorithm;


#ifdef __cplusplus
} // extern "C"
#endif

/*@}*/
 
#endif // NV_NPPIDEFS_H
