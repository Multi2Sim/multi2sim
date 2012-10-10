 /* Copyright 2010-2011 NVIDIA Corporation.  All rights reserved. 
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
#ifndef NV_NPPS_H
#define NV_NPPS_H
 
/**
 * \file npps.h
 * NPP Signal Processing Functionality.
 */
 
 #include "nppdefs.h"


#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup npps NPP Signal Processing
 * @{
 */


/** @name Memory Allocation
 * Signal-allocator methods for allocating 1D arrays of data in device memory.
 * All allocators have size parameters to specify the size of the signal (1D array)
 * being allocated.
 *
 * The allocator methods return a pointer to the newly allocated memory of appropriate
 * type. If device-memory allocation is not possible due to resource constaints
 * the allocators return 0 (i.e. NULL pointer). 
 *
 * All signal allocators allocate memory aligned such that it is  beneficial to the 
 * performance of the majority of the signal-processing primitives. 
 * It is no mandatory however to use these allocators. Any valid
 * CUDA device-memory pointers can be passed to NPP primitives. 
 */
//@{

/**
 * 8-bit unsigned signal allocator.
 * \param nSize Number of unsigned chars in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp8u * 
nppsMalloc_8u(int nSize);

/**
 * 16-bit unsigned signal allocator.
 * \param nSize Number of unsigned shorts in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp16u * 
nppsMalloc_16u(int nSize);

/**
 * 16-bit signal allocator.
 * \param nSize Number of shorts in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp16s * 
nppsMalloc_16s(int nSize);

/**
 * 16-bit complex-value signal allocator.
 * \param nSize Number of 16-bit complex numbers in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp16sc * 
nppsMalloc_16sc(int nSize);

/**
 * 32-bit unsigned signal allocator.
 * \param nSize Number of unsigned ints in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp32u * 
nppsMalloc_32u(int nSize);

/**
 * 32-bit integer signal allocator.
 * \param nSize Number of ints in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp32s * 
nppsMalloc_32s(int nSize);

/**
 * 32-bit complex integer signal allocator.
 * \param nSize Number of complex integner values in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp32sc * 
nppsMalloc_32sc(int nSize);

/**
 * 32-bit float signal allocator.
 * \param nSize Number of floats in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp32f * 
nppsMalloc_32f(int nSize);

/**
 * 32-bit complex float signal allocator.
 * \param nSize Number of complex float values in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp32fc * 
nppsMalloc_32fc(int nSize);

/**
 * 64-bit long integer signal allocator.
 * \param nSize Number of long ints in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp64s * 
nppsMalloc_64s(int nSize);

/**
 * 64-bit complex long integer signal allocator.
 * \param nSize Number of complex long int values in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp64sc * 
nppsMalloc_64sc(int nSize);

/**
 * 64-bit float (double) signal allocator.
 * \param nSize Number of doubles in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp64f * 
nppsMalloc_64f(int nSize);

/**
 * 64-bit complex complex signal allocator.
 * \param nSize Number of complex double valuess in the new signal.
 * \return A pointer to the new signal. 0 (NULL-pointer) indicates
 *         that an error occurred during allocation.
 */
Npp64fc * 
nppsMalloc_64fc(int nSize);

/**
 * Free method for any 2D allocated memory.
 * This method should be used to free memory allocated with any of the
 * nppiMalloc_<modifier> methods.
 * \param pValues A pointer to memory allocated using nppiMalloc_<modifier>.
 */
void nppsFree(void * pValues);

// end of Memory Allocaiton
//@}

/** @name Set 
 * Set methods for 1D vectors of various types. The copy methods operate on vector data given
 * as a pointer to the underlying data-type (e.g. 8-bit vectors would
 * be passed as pointers to Npp8u type) and length of the vectors, i.e. the number of items.
 */
//@{

/** 
 * 8-bit unsigned char, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_8u(Npp8u nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit integer, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_16s(Npp16s nValue, Npp16s * pDst, int nLength);

/** 
 * 16-bit integer complex, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_16sc(Npp16sc nValue, Npp16sc * pDst, int nLength);

/** 
 * 32-bit integer, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_32s(Npp32s nValue, Npp32s * pDst, int nLength);

/** 
 * 32-bit integer complex, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_32sc(Npp32sc nValue, Npp32sc * pDst, int nLength);

/** 
 * 32-bit float, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_32f(Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit float complex, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_32fc(Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit long long integer, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_64s(Npp64s nValue, Npp64s * pDst, int nLength);

/** 
 * 64-bit long long integer complex, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_64sc(Npp64sc nValue, Npp64sc * pDst, int nLength);

/** 
 * 64-bit double, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_64f(Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit double complex, vector set method.
 * \param nValue Value used to initialize the vector pDst.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSet_64fc(Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of Signal Set
//@}

/** @name Zero
 * Set signals to zero.
 */
//@{

/** 
 * 8-bit unsigned char, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_8u(Npp8u * pDst, int nLength);

/** 
 * 16-bit integer, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_16s(Npp16s * pDst, int nLength);

/** 
 * 16-bit integer complex, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_16sc(Npp16sc * pDst, int nLength);

/** 
 * 32-bit integer, vector zero method.
  * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_32s(Npp32s * pDst, int nLength);

/** 
 * 32-bit integer complex, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_32sc(Npp32sc * pDst, int nLength);

/** 
 * 32-bit float, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_32f(Npp32f * pDst, int nLength);

/** 
 * 32-bit float complex, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_32fc(Npp32fc * pDst, int nLength);

/** 
 * 64-bit long long integer, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_64s(Npp64s * pDst, int nLength);

/** 
 * 64-bit long long integer complex, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_64sc(Npp64sc * pDst, int nLength);

/** 
 * 64-bit double, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_64f(Npp64f * pDst, int nLength);

/** 
 * 64-bit double complex, vector zero method.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsZero_64fc(Npp64fc * pDst, int nLength);

// end of Zero
//@}

/** @name Copy
 * Copy methods for various type signals. Copy methods operate on
 * signal data given as a pointer to the underlying data-type (e.g. 8-bit
 * vectors would be passed as pointers to Npp8u type) and length of the
 * vectors, i.e. the number of items. 
 */
//@{

/** 
 * 8-bit unsigned char, vector copy method
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_8u(const Npp8u * pSrc, Npp8u * pDst, int len);

/** 
 * 16-bit signed short, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_16s(const Npp16s * pSrc, Npp16s * pDst, int len);

/** 
 * 32-bit signed integer, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_32s(const Npp32s * pSrc, Npp32s * pDst, int nLength);

/** 
 * 32-bit float, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_32f(const Npp32f * pSrc, Npp32f * pDst, int len);

/** 
 * 64-bit signed integer, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_64s(const Npp64s * pSrc, Npp64s * pDst, int len);

/** 
 * 16-bit complex short, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_16sc(const Npp16sc * pSrc, Npp16sc * pDst, int len);

/** 
 * 32-bit complex signed integer, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_32sc(const Npp32sc * pSrc, Npp32sc * pDst, int len);

/** 
 * 32-bit complex float, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_32fc(const Npp32fc * pSrc, Npp32fc * pDst, int len);

/** 
 * 64-bit complex signed integer, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_64sc(const Npp64sc * pSrc, Npp64sc * pDst, int len);

/** 
 * 64-bit complex double, vector copy method.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param len \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCopy_64fc(const Npp64fc * pSrc, Npp64fc * pDst, int len);

// end of Signal Copy
//@}

/** @name AddC
 * Adds a constant value to each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char in place signal add constant,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned charvector add constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal add constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short vector add constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place  signal add constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal add constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal add constant, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal add constant,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal add constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integersignal add constant and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * add constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) signal add constant
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal add constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal add constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) in
 * place signal add constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32fc_I(Npp32fc nValue, Npp32fc * pSrcDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) signal
 * add constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_32fc(const Npp32fc * pSrc, Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit floating point, in place signal add constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating pointsignal add constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) in
 * place signal add constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_64fc_I(Npp64fc nValue, Npp64fc * pSrcDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) signal
 * add constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be added to each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddC_64fc(const Npp64fc * pSrc, Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of AddC
//@}

/** @name AddProductC
 * Adds product of a constant and each sample of a source signal to each sample of the destination signal.
 */
//@{

/** 
 * 8-bit unsigned char in place signal add product of signal times constant to destination signal,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char add product of signal times constant to destination signal, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal add product of signal times constant to destination signal, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short add product of signal times constant to destination signal, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place  signal add product of signal times constant to destination signal, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal add product of signal times constant to destination signal, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary)signal add product of signal times constant to destination signal, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary)signal add product of signal times constant to destination signal,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal add product of signal times constant to destination signal and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal add product of signal times constant to destination signal and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * add product of signal times constant to destination signal and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary)signal add product of signal times constant to destination signal
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal add product of signal times constant to destination signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal add product of signal times constant to destination signal.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProductC_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);


// end of AddProductC
//@}

/** @name MulC
 * Multiplies each sample of a signal by a constant value.
 */
//@{

/** 
 * 8-bit unsigned char in place signal times constant,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal times constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal times constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal times constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place  signal times constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal times constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary)signal times constant, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary)signal times constant,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal times constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal times constant and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * times constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) signal times constant
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal times constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal times constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point signal times constant with output converted to 16-bit signed integer.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_Low_32f16s(const Npp32f * pSrc, Npp32f nValue, Npp16s * pDst, int nLength);

/** 
 * 32-bit floating point signal times constant with output converted to 16-bit signed integer
 * with scaling and saturation of output result.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nScaleFactor \ref integer_result_scaling.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32f16s_Sfs(const Npp32f * pSrc, Npp32f nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) in
 * place signal times constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32fc_I(Npp32fc nValue, Npp32fc * pSrcDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) signal
 * times constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_32fc(const Npp32fc * pSrc, Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit floating point, in place signal times constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal times constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point signal times constant with in place conversion to 64-bit signed integer
 * and with scaling and saturation of output result.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_64f64s_ISfs(Npp64f nValue, Npp64s * pDst, int nLength, int nScaleFactor);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) in
 * place signal times constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_64fc_I(Npp64fc nValue, Npp64fc * pSrcDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) signal
 * times constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be multiplied by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMulC_64fc(const Npp64fc * pSrc, Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of MulC
//@}                                                                                                                   

/** @name SubC
 * Subtracts a constant from each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char in place signal subtract constant,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal subtract constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal subtract constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal subtract constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal subtract constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal subtract constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal subtract constant, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal subtract constant,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal subtract constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal subtract constant and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * subtract constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) signal subtract constant
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal subtract constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal subtract constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) in
 * place signal subtract constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32fc_I(Npp32fc nValue, Npp32fc * pSrcDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) signal
 * subtract constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_32fc(const Npp32fc * pSrc, Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit floating point, in place signal subtract constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal subtract constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) in
 * place signal subtract constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_64fc_I(Npp64fc nValue, Npp64fc * pSrcDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) signal
 * subtract constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be subtracted from each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubC_64fc(const Npp64fc * pSrc, Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of SubC
//@}

/** @name SubCRev
 * Subtracts each sample of a signal from a constant.
 */
//@{

/** 
 * 8-bit unsigned char in place signal subtract from constant,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal subtract from constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal subtract from constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal subtract from constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal subtract from constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal subtract from constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal subtract from constant, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal subtract from constant,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal subtract from constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integersignal subtract from constant and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * subtract from constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) signal subtract from constant
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal subtract from constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal subtract from constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) in
 * place signal subtract from constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32fc_I(Npp32fc nValue, Npp32fc * pSrcDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) signal
 * subtract from constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_32fc(const Npp32fc * pSrc, Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit floating point, in place signal subtract from constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal subtract from constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) in
 * place signal subtract from constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_64fc_I(Npp64fc nValue, Npp64fc * pSrcDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) signal
 * subtract from constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value each vector element is to be subtracted from
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSubCRev_64fc(const Npp64fc * pSrc, Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of SubCRev
//@}

/** @name DivC
 * Divides each sample of a signal by a constant.
 */
//@{

/** 
 * 8-bit unsigned char in place signal divided by constant,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_8u_ISfs(Npp8u nValue, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal divided by constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_8u_Sfs(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal divided by constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16u_ISfs(Npp16u nValue, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal divided by constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16u_Sfs(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal divided by constant, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16s_ISfs(Npp16s nValue, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal divided by constant, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16s_Sfs(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal divided by constant, 
 * scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16sc_ISfs(Npp16sc nValue, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit integer complex number (16 bit real, 16 bit imaginary) signal divided by constant,
 * scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_16sc_Sfs(const Npp16sc * pSrc, Npp16sc nValue, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal divided by constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32s_ISfs(Npp32s nValue, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal divided by constant and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32s_Sfs(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) in place signal
 * divided by constant and scale.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32sc_ISfs(Npp32sc nValue, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit integer complex number (32 bit real, 32 bit imaginary) signal divided by constant
 * and scale.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32sc_Sfs(const Npp32sc * pSrc, Npp32sc nValue, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal divided by constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal divided by constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) in
 * place signal divided by constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32fc_I(Npp32fc nValue, Npp32fc * pSrcDst, int nLength);

/** 
 * 32-bit floating point complex number (32 bit real, 32 bit imaginary) signal
 * divided by constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_32fc(const Npp32fc * pSrc, Npp32fc nValue, Npp32fc * pDst, int nLength);

/** 
 * 64-bit floating point in place signal divided by constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal divided by constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) in
 * place signal divided by constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_64fc_I(Npp64fc nValue, Npp64fc * pSrcDst, int nLength);

/** 
 * 64-bit floating point complex number (64 bit real, 64 bit imaginary) signal
 * divided by constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided into each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivC_64fc(const Npp64fc * pSrc, Npp64fc nValue, Npp64fc * pDst, int nLength);

// end of DivC
//@}

/** @name DivCRev
 * Divides a constant by each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char signal in place constant divided by signal,
 * scale, then clamp to saturated value
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_8u_I(Npp8u nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char signal divided by constant, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_8u(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short in place constant divided by signal, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_16u_I(Npp16u nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short vector divided by constant, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_16u(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength);

/** 
 * 16-bit signed short in place constant divided by signal, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_16s_I(Npp16s nValue, Npp16s * pSrcDst, int nLength);

/** 
 * 16-bit signed short constant divided by signal, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_16s(const Npp16s * pSrc, Npp16s nValue, Npp16s * pDst, int nLength);

/** 
 * 32-bit signed integer in place constant divided by signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_32s_I(Npp32s nValue, Npp32s * pSrcDst, int nLength);

/** 
 * 32-bit signed integer constant divided by signal.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_32s(const Npp32s * pSrc, Npp32s nValue, Npp32s * pDst, int nLength);

/** 
 * 32-bit floating point in place constant divided by signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_32f_I(Npp32f nValue, Npp32f * pSrcDst, int nLength);

/** 
 * 32-bit floating point constant divided by signal.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_32f(const Npp32f * pSrc, Npp32f nValue, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point in place constant divided by signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param nLength Length of the vectors, number of items.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_64f_I(Npp64f nValue, Npp64f * pSrcDst, int nLength);

/** 
 * 64-bit floating point constant divided by signal.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be divided by each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDivCRev_64f(const Npp64f * pSrc, Npp64f nValue, Npp64f * pDst, int nLength);

// end of DivCRev
//@}

/** @name Add Signal
 * Sample by sample addition of two signals.
 */
//@{

/** 
 * 16-bit signed short signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength);

/** 
 * 16-bit unsigned short signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16u(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned int signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32u(const Npp32u * pSrc1, const Npp32u * pSrc2, Npp32u * pDst, int nLength);

/** 
 * 32-bit floating point signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32f(const Npp32f * pSrc1, const Npp32f * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_64f(const Npp64f * pSrc1, const Npp64f * pSrc2, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32fc(const Npp32fc * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal add signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_64fc(const Npp64fc * pSrc1, const Npp64fc * pSrc2, Npp64fc * pDst, int nLength);

/** 
 * 8-bit unsigned char signal add signal with 16-bit unsigned result,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_8u16u(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 16-bit signed short signal add signal with 32-bit floating point result,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be added to signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s32f(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 8-bit unsigned char add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_8u_Sfs(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16u_Sfs(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_64s_Sfs(const Npp64s * pSrc1, const Npp64s * pSrc2, Npp64s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed complex short add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16sc_Sfs(const Npp16sc * pSrc1, const Npp16sc * pSrc2, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed complex integer add signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be added to signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32sc_Sfs(const Npp32sc * pSrc1, const Npp32sc * pSrc2, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal add signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s_I(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit floating point in place signal add signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32f_I(const Npp32f * pSrc, Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point in place signal add signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_64f_I(const Npp64f * pSrc, Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point in place signal add signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32fc_I(const Npp32fc * pSrc, Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point in place signal add signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_64fc_I(const Npp64fc * pSrc, Npp64fc * pSrcDst, int nLength);

/** 
 * 16/32-bit signed short in place signal add signal with 32-bit signed integer results,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s32s_I(const Npp16s * pSrc, Npp32s * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char in place signal add signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_8u_ISfs(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal add signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16u_ISfs(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal add signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16s_ISfs(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal add signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32s_ISfs(const Npp32s * pSrc, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short in place signal add signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_16sc_ISfs(const Npp16sc * pSrc, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit complex signed integer in place signal add signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be added to signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAdd_32sc_ISfs(const Npp32sc * pSrc, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

// end of Add Signal
//@}

/** @name AddProduct Signal
 * Adds sample by sample product of two signals to the destination signal.
 */
//@{

/** 
 * 32-bit floating point signal add product of source signal times destination signal to destination signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_32f(const Npp32f * pSrc1, const Npp32f * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal add product of source signal times destination signal to destination signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_64f(const Npp64f * pSrc1, const Npp64f * pSrc2, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal add product of source signal times destination signal to destination signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_32fc(const Npp32fc * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal add product of source signal times destination signal to destination signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_64fc(const Npp64fc * pSrc1, const Npp64fc * pSrc2, Npp64fc * pDst, int nLength);

/** 
 * 16-bit signed short signal add product of source signal1 times source signal2 to destination signal,
 * with scaling, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed short signal add product of source signal1 times source signal2 to destination signal,
 * with scaling, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal add product of source signal1 times source signal2 to 32-bit signed integer destination signal,
 * with scaling, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer. product of source1 and source2 signal elements to be added to destination elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAddProduct_16s32s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

// end of AddProduct Signal
//@}

/** @name Mul Signal
 * Sample by sample multiplication the samples of two signals.
 */
//@{

/** 
 * 16-bit signed short signal times signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength);

/** 
 * 32-bit floating point signal times signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32f(const Npp32f * pSrc1, const Npp32f * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal times signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_64f(const Npp64f * pSrc1, const Npp64f * pSrc2, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal times signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32fc(const Npp32fc * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal times signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_64fc(const Npp64fc * pSrc1, const Npp64fc * pSrc2, Npp64fc * pDst, int nLength);

/** 
 * 8-bit unsigned char signal times signal with 16-bit unsigned result,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_8u16u(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 16-bit signed short signal times signal with 32-bit floating point result,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s32f(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point signal times 32-bit complex floating point signal with complex 32-bit floating point result,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32f32fc(const Npp32f * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 8-bit unsigned char signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_8u_Sfs(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal time signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16u_Sfs(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed complex short signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16sc_Sfs(const Npp16sc * pSrc1, const Npp16sc * pSrc2, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed complex integer signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32sc_Sfs(const Npp32sc * pSrc1, const Npp32sc * pSrc2, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal times 16-bit signed short signal, scale, then clamp to 16-bit signed saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16u16s_Sfs(const Npp16u * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal times signal, scale, then clamp to 32-bit signed saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s32s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal times 32-bit complex signed integer signal, scale, then clamp to 32-bit complex integer saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32s32sc_Sfs(const Npp32s * pSrc1, const Npp32sc * pSrc2, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal times signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal2 elements to be multiplied by signal1 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_Low_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal times signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s_I(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit floating point in place signal times signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32f_I(const Npp32f * pSrc, Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point in place signal times signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_64f_I(const Npp64f * pSrc, Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point in place signal times signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32fc_I(const Npp32fc * pSrc, Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point in place signal times signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_64fc_I(const Npp64fc * pSrc, Npp64fc * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point in place signal times 32-bit floating point signal,
 * then clamp to 32-bit complex floating point saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32f32fc_I(const Npp32f * pSrc, Npp32fc * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char in place signal times signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_8u_ISfs(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal times signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16u_ISfs(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal times signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16s_ISfs(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal times signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32s_ISfs(const Npp32s * pSrc, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short in place signal times signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_16sc_ISfs(const Npp16sc * pSrc, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit complex signed integer in place signal times signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32sc_ISfs(const Npp32sc * pSrc, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit complex signed integer in place signal times 32-bit signed integer signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be multiplied by signal1 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMul_32s32sc_ISfs(const Npp32s * pSrc, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

// end of Mul Signal
//@}

/** @name Sub Signal
 * Sample by sample subtraction of the samples of two signals.
 */
//@{

/** 
 * 16-bit signed short signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16s(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength);

/** 
 * 32-bit floating point signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32f(const Npp32f * pSrc1, const Npp32f * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_64f(const Npp64f * pSrc1, const Npp64f * pSrc2, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32fc(const Npp32fc * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_64fc(const Npp64fc * pSrc1, const Npp64fc * pSrc2, Npp64fc * pDst, int nLength);

/** 
 * 16-bit signed short signal subtract 16-bit signed short signal,
 * then clamp and convert to 32-bit floating point saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16s32f(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 8-bit unsigned char signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_8u_Sfs(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16u_Sfs(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed complex short signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16sc_Sfs(const Npp16sc * pSrc1, const Npp16sc * pSrc2, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed complex integer signal subtract signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 elements to be subtracted from signal2 elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32sc_Sfs(const Npp32sc * pSrc1, const Npp32sc * pSrc2, Npp32sc * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16s_I(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit floating point in place signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32f_I(const Npp32f * pSrc, Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point in place signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_64f_I(const Npp64f * pSrc, Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point in place signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32fc_I(const Npp32fc * pSrc, Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point in place signal subtract signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_64fc_I(const Npp64fc * pSrc, Npp64fc * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char in place signal subtract signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_8u_ISfs(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal subtract signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16u_ISfs(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal subtract signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16s_ISfs(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal subtract signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32s_ISfs(const Npp32s * pSrc, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short in place signal subtract signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_16sc_ISfs(const Npp16sc * pSrc, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit complex signed integer in place signal subtract signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 elements to be subtracted from signal2 elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSub_32sc_ISfs(const Npp32sc * pSrc, Npp32sc * pSrcDst, int nLength, int nScaleFactor);

// end of Sub Signal
//@}

/** @name Div Signal
 * Sample by sample division of the samples of two signals.
 */
//@{

/** 
 * 8-bit unsigned char signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_8u_Sfs(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16u_Sfs(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32s_Sfs(const Npp32s * pSrc1, const Npp32s * pSrc2, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed complex short signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16sc_Sfs(const Npp16sc * pSrc1, const Npp16sc * pSrc2, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal divided by 16-bit signed short signal, scale, then clamp to 16-bit signed short saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32s16s_Sfs(const Npp16s * pSrc1, const Npp32s * pSrc2, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point signal divide signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32f(const Npp32f * pSrc1, const Npp32f * pSrc2, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal divide signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_64f(const Npp64f * pSrc1, const Npp64f * pSrc2, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal divide signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32fc(const Npp32fc * pSrc1, const Npp32fc * pSrc2, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal divide signal,
 * then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_64fc(const Npp64fc * pSrc1, const Npp64fc * pSrc2, Npp64fc * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal divide signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_8u_ISfs(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal divide signal, with scaling,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16u_ISfs(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short in place signal divide signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16s_ISfs(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short in place signal divide signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_16sc_ISfs(const Npp16sc * pSrc, Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer in place signal divide signal, with scaling, 
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32s_ISfs(const Npp32s * pSrc, Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit floating point in place signal divide signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32f_I(const Npp32f * pSrc, Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point in place signal divide signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_64f_I(const Npp64f * pSrc, Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point in place signal divide signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_32fc_I(const Npp32fc * pSrc, Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point in place signal divide signal,
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_64fc_I(const Npp64fc * pSrc, Npp64fc * pSrcDst, int nLength);

// end of Div Signal
//@}

/** @name Div_Round Signal
 * Sample by sample division of the samples of two signals with rounding.
 */
//@{

/** 
 * 8-bit unsigned char signal divide signal, scale, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_8u_Sfs(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

/** 
 * 16-bit unsigned short signal divide signal, scale, round, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_16u_Sfs(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

/** 
 * 16-bit signed short signal divide signal, scale, round, then clamp to saturated value.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer, signal1 divisor elements to be divided into signal2 dividend elements.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_16s_Sfs(const Npp16s * pSrc1, const Npp16s * pSrc2, Npp16s * pDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

/** 
 * 8-bit unsigned char in place signal divide signal, with scaling, rounding
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_8u_ISfs(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

/** 
 * 16-bit unsigned short in place signal divide signal, with scaling, rounding
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_16u_ISfs(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

/** 
 * 16-bit signed short in place signal divide signal, with scaling, rounding
 * then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal1 divisor elements to be divided into signal2 dividend elements
 * \param nLength \ref length_specification.
 * \param nRndMode various rounding modes.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsDiv_Round_16s_ISfs(const Npp16s * pSrc, Npp16s * pSrcDst, int nLength, NppRoundMode nRndMode, int nScaleFactor);

// end of Div_Round Signal
//@}

/** @name Absolute Value Signal
 * Absolute value of each sample of a signal.
 */
//@{

/** 
 * 16-bit signed short signal absolute value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_16s(const Npp16s * pSrc, Npp16s * pDst, int nLength);

/** 
 * 32-bit signed integer signal absolute value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_32s(const Npp32s * pSrc, Npp32s * pDst, int nLength);

/** 
 * 32-bit floating point signal absolute value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal absolute value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 16-bit signed short signal absolute value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_16s_I(Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit signed integer signal absolute value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_32s_I(Npp32s * pSrcDst, int nLength);

/** 
 * 32-bit floating point signal absolute value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal absolute value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAbs_64f_I(Npp64f * pSrcDst, int nLength);

// end of Absolute Value Signal
//@}

/** @name Square Signal
 * Squares each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal squared.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal squared.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal squared.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_32fc(const Npp32fc * pSrc, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal squared.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_64fc(const Npp64fc * pSrc, Npp64fc * pDst, int nLength);

/** 
 * 32-bit floating point signal squared.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal squared.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_64f_I(Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point signal squared.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_32fc_I(Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point signal squared.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_64fc_I(Npp64fc * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char signal squared, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_8u_Sfs(const Npp8u * pSrc, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal squared, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16u_Sfs(const Npp16u * pSrc, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal squared, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16s_Sfs(const Npp16s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short signal squared, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16sc_Sfs(const Npp16sc * pSrc, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal squared, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_8u_ISfs(Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal squared, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16u_ISfs(Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal squared, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16s_ISfs(Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short signal squared, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqr_16sc_ISfs(Npp16sc * pSrcDst, int nLength, int nScaleFactor);

// end of Square Signal
//@}

/** @name Square Root Signal
 * Square root of each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal square root.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal square root.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 32-bit complex floating point signal square root.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_32fc(const Npp32fc * pSrc, Npp32fc * pDst, int nLength);

/** 
 * 64-bit complex floating point signal square root.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64fc(const Npp64fc * pSrc, Npp64fc * pDst, int nLength);

/** 
 * 32-bit floating point signal square root.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal square root.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64f_I(Npp64f * pSrcDst, int nLength);

/** 
 * 32-bit complex floating point signal square root.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_32fc_I(Npp32fc * pSrcDst, int nLength);

/** 
 * 64-bit complex floating point signal square root.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64fc_I(Npp64fc * pSrcDst, int nLength);

/** 
 * 8-bit unsigned char signal square root, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_8u_Sfs(const Npp8u * pSrc, Npp8u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal square root, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16u_Sfs(const Npp16u * pSrc, Npp16u * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal square root, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16s_Sfs(const Npp16s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short signal square root, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16sc_Sfs(const Npp16sc * pSrc, Npp16sc * pDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer signal square root, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64s_Sfs(const Npp64s * pSrc, Npp64s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal square root, scale, then clamp to 16-bit signed integer saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_32s16s_Sfs(const Npp32s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer signal square root, scale, then clamp to 16-bit signed integer saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64s16s_Sfs(const Npp64s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 8-bit unsigned char signal square root, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_8u_ISfs(Npp8u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit unsigned short signal square root, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16u_ISfs(Npp16u * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal square root, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16s_ISfs(Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 16-bit complex signed short signal square root, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_16sc_ISfs(Npp16sc * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer signal square root, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSqrt_64s_ISfs(Npp64s * pSrcDst, int nLength, int nScaleFactor);

// end of Square Root Signal
//@}

/** @name Cube Root Signal
 * Cube root of each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal cube root.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCubrt_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 32-bit signed integer signal cube root, scale, then clamp to 16-bit signed integer saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCubrt_32s16s_Sfs(const Npp32s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

// end of Cube Root Signal
//@}

/** @name Exponent Signal
 * E raised to the power of each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal exponent.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal exponent.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 32-bit floating point signal exponent with 64-bit floating point result.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_32f64f(const Npp32f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 32-bit floating point signal exponent.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal exponent.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_64f_I(Npp64f * pSrcDst, int nLength);

/** 
 * 16-bit signed short signal exponent, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_16s_Sfs(const Npp16s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal exponent, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_32s_Sfs(const Npp32s * pSrc, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer signal exponent, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_64s_Sfs(const Npp64s * pSrc, Npp64s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal exponent, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_16s_ISfs(Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal exponent, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_32s_ISfs(Npp32s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 64-bit signed integer signal exponent, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsExp_64s_ISfs(Npp64s * pSrcDst, int nLength, int nScaleFactor);

// end of Exponent Signal
//@}

/** @name Natural Logarithm Signal
 * Natural logarithm of each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal natural logarithm.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal natural logarithm.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 64-bit floating point signal natural logarithm with 32-bit floating point result.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_64f32f(const Npp64f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 32-bit floating point signal natural logarithm.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal natural logarithm.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_64f_I(Npp64f * pSrcDst, int nLength);

/** 
 * 16-bit signed short signal natural logarithm, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_16s_Sfs(const Npp16s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal natural logarithm, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_32s_Sfs(const Npp32s * pSrc, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal natural logarithm, scale, then clamp to 16-bit signed short saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_32s16s_Sfs(const Npp32s * pSrc, Npp16s * pDst, int nLength, int nScaleFactor);

/** 
 * 16-bit signed short signal natural logarithm, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_16s_ISfs(Npp16s * pSrcDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal natural logarithm, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLn_32s_ISfs(Npp32s * pSrcDst, int nLength, int nScaleFactor);

// end of Natural Logarithm Signal
//@}

/** @name Ten Times Base Ten Logarithm Signal
 * Ten times the decimal logarithm of each sample of a signal.
 */
//@{

/** 
 * 32-bit signed integer signal 10 times base 10 logarithm, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
npps10Log10_32s_Sfs(const Npp32s * pSrc, Npp32s * pDst, int nLength, int nScaleFactor);

/** 
 * 32-bit signed integer signal 10 times base 10 logarithm, scale, then clamp to saturated value.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
npps10Log10_32s_ISfs(Npp32s * pSrcDst, int nLength, int nScaleFactor);

// end of Ten Times Base Ten Logarithm Signal
//@}

/** @name Inverse Tangent Signal
 * Inverse tangent of each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal inverse tangent.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsArctan_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength);

/** 
 * 64-bit floating point signal inverse tangent.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsArctan_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength);

/** 
 * 32-bit floating point signal inverse tangent.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsArctan_32f_I(Npp32f * pSrcDst, int nLength);

/** 
 * 64-bit floating point signal inverse tangent.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsArctan_64f_I(Npp64f * pSrcDst, int nLength);

// end of Inverse Tangent Signal
//@}

/** @name Normalize Signal
 * Normalize each sample of a real or complex signal using offset and division operations.
 */
//@{

/** 
 * 32-bit floating point signal normalize.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_32f(const Npp32f * pSrc, Npp32f * pDst, int nLength, Npp32f vSub, Npp32f vDiv);

/** 
 * 32-bit complex floating point signal normalize.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_32fc(const Npp32fc * pSrc, Npp32fc * pDst, int nLength, Npp32fc vSub, Npp32f vDiv);

/** 
 * 64-bit floating point signal normalize.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_64f(const Npp64f * pSrc, Npp64f * pDst, int nLength, Npp64f vSub, Npp64f vDiv);

/** 
 * 64-bit complex floating point signal normalize.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_64fc(const Npp64fc * pSrc, Npp64fc * pDst, int nLength, Npp64fc vSub, Npp64f vDiv);

/** 
 * 16-bit signed short signal normalize, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_16s_Sfs(const Npp16s * pSrc, Npp16s * pDst, int nLength, Npp16s vSub, int vDiv, int nScaleFactor);

/** 
 * 16-bit complex signed short signal normalize, scale, then clamp to saturated value.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \param vSub value subtracted from each signal element before division
 * \param vDiv divisor of post-subtracted signal element dividend
 * \param nScaleFactor \ref integer_result_scaling.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNormalize_16sc_Sfs(const Npp16sc * pSrc, Npp16sc * pDst, int nLength, Npp16sc vSub, int vDiv, int nScaleFactor);

// end of Normalize Signal
//@}

/** @name Cauchy, CauchyD, and CauchyDD2 Signal
 * Determine Cauchy robust error function and its first and second derivatives for each sample of a signal.
 */
//@{

/** 
 * 32-bit floating point signal Cauchy error calculation.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nParam constant used in Cauchy formula
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCauchy_32f_I(Npp32f* pSrcDst, int nLength, Npp32f nParam);

/** 
 * 32-bit floating point signal Cauchy first derivative.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \param nParam constant used in Cauchy formula
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCauchyD_32f_I(Npp32f* pSrcDst, int nLength, Npp32f nParam);

/** 
 * 32-bit floating point signal Cauchy first and second derivatives.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param pD2FVal \ref source_signal_pointer. This signal contains the second derivative
 *		of the source signal.
 * \param nLength \ref length_specification.
 * \param nParam constant used in Cauchy formula
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsCauchyDD2_32f_I(Npp32f* pSrcDst, Npp32f* pD2FVal, int nLength, Npp32f nParam);

// end of Cauchy, CauchyD, and CauchyDD2 Signal
//@}

/** @name AndC
 * Bitwise AND of a constant and each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char signal and with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_8u(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal and with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_16u(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal and with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_32u(const Npp32u * pSrc, Npp32u nValue, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal and with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_8u_I(Npp8u nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal and with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_16u_I(Npp16u nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place signal and with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be anded with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAndC_32u_I(Npp32u nValue, Npp32u * pSrcDst, int nLength);

// end of AndC
//@}

/** @name And
 * Sample by sample bitwise AND of samples from two signals.
 */
//@{

/** 
 * 8-bit unsigned char signal and with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_8u(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal and with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_16u(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal and with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_32u(const Npp32u * pSrc1, const Npp32u * pSrc2, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal and with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_8u_I(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal and with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_16u_I(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned integer in place signal and with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be anded with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsAnd_32u_I(const Npp32u * pSrc, Npp32u * pSrcDst, int nLength);

// end of And
//@}

/** @name OrC
 * Bitwise OR of a constant and each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char signal or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_8u(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_16u(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_32u(const Npp32u * pSrc, Npp32u nValue, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_8u_I(Npp8u nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_16u_I(Npp16u nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place signal or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOrC_32u_I(Npp32u nValue, Npp32u * pSrcDst, int nLength);

// end of OrC
//@}

/** @name Or
 * Sample by sample bitwise OR of the samples from two signals.
 */
//@{

/** 
 * 8-bit unsigned char signal or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_8u(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_16u(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_32u(const Npp32u * pSrc1, const Npp32u * pSrc2, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_8u_I(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_16u_I(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned integer in place signal or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsOr_32u_I(const Npp32u * pSrc, Npp32u * pSrcDst, int nLength);

// end of Or
//@}

/** @name XorC
 * Bitwise XOR of a constant and each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char signal exclusive or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_8u(const Npp8u * pSrc, Npp8u nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal exclusive or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_16u(const Npp16u * pSrc, Npp16u nValue, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal exclusive or with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_32u(const Npp32u * pSrc, Npp32u nValue, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal exclusive or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_8u_I(Npp8u nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal exclusive or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_16u_I(Npp16u nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place signal exclusive or with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be exclusive ored with each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXorC_32u_I(Npp32u nValue, Npp32u * pSrcDst, int nLength);

// end of XorC
//@}

/** @name Xor
 * Sample by sample bitwise XOR of the samples from two signals.
 */
//@{

/** 
 * 8-bit unsigned char signal exclusive or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_8u(const Npp8u * pSrc1, const Npp8u * pSrc2, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal exclusive or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_16u(const Npp16u * pSrc1, const Npp16u * pSrc2, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal exclusive or with signal.
 * \param pSrc1 \ref source_signal_pointer.
 * \param pSrc2 \ref source_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_32u(const Npp32u * pSrc1, const Npp32u * pSrc2, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal exclusive or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_8u_I(const Npp8u * pSrc, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal exclusive or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_16u_I(const Npp16u * pSrc, Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned integer in place signal exclusive or with signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pSrcDst \ref in_place_signal_pointer. signal2 elements to be exclusive ored with signal1 elements
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsXor_32u_I(const Npp32u * pSrc, Npp32u * pSrcDst, int nLength);

// end of Xor
//@}

/** @name Not
 * Bitwise NOT of each sample of a signal.
 */
//@{

/** 
 * 8-bit unsigned char not signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_8u(const Npp8u * pSrc, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short not signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_16u(const Npp16u * pSrc, Npp16u * pDst, int nLength);

/** 
 * 32-bit unsigned integer not signal.
 * \param pSrc \ref source_signal_pointer.
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_32u(const Npp32u * pSrc, Npp32u * pDst, int nLength);

/** 
 * 8-bit unsigned char in place not signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_8u_I(Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place not signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_16u_I(Npp16u * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place not signal.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsNot_32u_I(Npp32u * pSrcDst, int nLength);

// end of Not
//@}

/** @name LShiftC
 * Left shifts the bits of each sample of a signal by a constant amount.
 */
//@{

/** 
 * 8-bit unsigned char signal left shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_8u(const Npp8u * pSrc, int nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal left shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_16u(const Npp16u * pSrc, int nValue, Npp16u * pDst, int nLength);

/** 
 * 16-bit signed short signal left shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_16s(const Npp16s * pSrc, int nValue, Npp16s * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal left shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_32u(const Npp32u * pSrc, int nValue, Npp32u * pDst, int nLength);

/** 
 * 32-bit signed integer signal left shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_32s(const Npp32s * pSrc, int nValue, Npp32s * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal left shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_8u_I(int nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal left shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_16u_I(int nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 16-bit signed short in place signal left shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_16s_I(int nValue, Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place signal left shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_32u_I(int nValue, Npp32u * pSrcDst, int nLength);

/** 
 * 32-bit signed signed integer in place signal left shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to left shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsLShiftC_32s_I(int nValue, Npp32s * pSrcDst, int nLength);

// end of LShiftC
//@}

/** @name RShiftC
 * Right shifts the bits of each sample of a signal by a constant amount.
 */
//@{

/** 
 * 8-bit unsigned char signal right shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_8u(const Npp8u * pSrc, int nValue, Npp8u * pDst, int nLength);

/** 
 * 16-bit unsigned short signal right shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_16u(const Npp16u * pSrc, int nValue, Npp16u * pDst, int nLength);

/** 
 * 16-bit signed short signal right shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_16s(const Npp16s * pSrc, int nValue, Npp16s * pDst, int nLength);

/** 
 * 32-bit unsigned integer signal right shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_32u(const Npp32u * pSrc, int nValue, Npp32u * pDst, int nLength);

/** 
 * 32-bit signed integer signal right shift with constant.
 * \param pSrc \ref source_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param pDst \ref destination_signal_pointer.
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_32s(const Npp32s * pSrc, int nValue, Npp32s * pDst, int nLength);

/** 
 * 8-bit unsigned char in place signal right shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_8u_I(int nValue, Npp8u * pSrcDst, int nLength);

/** 
 * 16-bit unsigned short in place signal right shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_16u_I(int nValue, Npp16u * pSrcDst, int nLength);

/** 
 * 16-bit signed short in place signal right shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_16s_I(int nValue, Npp16s * pSrcDst, int nLength);

/** 
 * 32-bit unsigned signed integer in place signal right shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_32u_I(int nValue, Npp32u * pSrcDst, int nLength);

/** 
 * 32-bit signed signed integer in place signal right shift with constant.
 * \param pSrcDst \ref in_place_signal_pointer.
 * \param nValue Constant value to be used to right shift each vector element
 * \param nLength \ref length_specification.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsRShiftC_32s_I(int nValue, Npp32s * pSrcDst, int nLength);

// end of RShiftC
//@}

/** @name Statistical Functions
 * Functions that provide global signal statistics like: average, standard
 * deviation, minimum, etc.
 */
//@{

/** 
 * Device-buffer size (in bytes) for 8u reductions.
 * This primitive provides the correct buffer size for nppsSum_8u, 
 * nppsMin_8u, nppsMax_8u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_8u(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 16s reductions.
 * This primitive provides the correct buffer size for nppsSum_16s, 
 * nppsMin_16s, nppsMax_16s.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_16s(int nLength, int * hpBufferSize /* host pointer */);

/** 
 * Device-buffer size (in bytes) for 16u reductions.
 * This primitive provides the correct buffer size for nppsSum_16u, 
 * nppsMin_16u, nppsMax_16u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
inline
NppStatus 
nppsReductionGetBufferSize_16u(int nLength, int * hpBufferSize /* host pointer */)
{
    return nppsReductionGetBufferSize_16s(nLength, hpBufferSize);
}

/** 
 * Device-buffer size (in bytes) for 16s reductions with integer-results scaling.
 * This primitive provides the correct buffer size for nppsSum_16s_Sfs.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_16s_Sfs(int nLength, int * hpBufferSize /* host pointer */);

/** 
 * Device-buffer size (in bytes) for 16sc reductions.
 * This primitive provides the correct buffer size for nppsSum_16sc, 
 * nppsMin_16sc, nppsMax_16sc.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_16sc(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 16sc reductions with integer-results scaling.
 * This primitive provides the correct buffer size for nppsSum_16sc_Sfs.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_16sc_Sfs(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 32s reductions.
 * This primitive provides the correct buffer size for nppsSum_32sc, 
 * nppsMin_32sc, nppsMax_32sc.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_32s(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 32u reductions.
 * This primitive provides the correct buffer size for nppsSum_32u, 
 * nppsMin_32u, nppsMax_32u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
inline
NppStatus 
nppsReductionGetBufferSize_32u(int nLength, int * hpBufferSize /* host pointer */)
{
    return nppsReductionGetBufferSize_32s(nLength, hpBufferSize);
}
 
/** 
 * Device-buffer size (in bytes) for 32s reductions with integer-results scaling.
 * This primitive provides the correct buffer size for nppsSum_32s_Sfs.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size. Important: hpBufferSize is a 
 *        <em>host pointer.</em> 
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_32s_Sfs(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 32sc reductions.
 * This primitive provides the correct buffer size for nppsSum_32sc, 
 * nppsMin_32sc, nppsMax_32sc.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_32sc(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 32f reductions.
 * This primitive provides the correct buffer size for nppsSum_32f, 
 * nppsMin_32f, nppsMax_32f.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_32f(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 32fc reductions.
 * This primitive provides the correct buffer size for nppsSum_32fc, 
 * nppsMin_32fc, nppsMax_32fc.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_32fc(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 64s reductions.
 * This primitive provides the correct buffer size for nppsSum_64s, 
 * nppsMin_64s, nppsMax_64s.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_64s(int nLength, int * hpBufferSize /* host pointer */);
 
/** 
 * Device-buffer size (in bytes) for 64f reductions.
 * This primitive provides the correct buffer size for nppsSum_64f, 
 * nppsMin_64f, nppsMax_64f.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_64f(int nLength, int * hpBufferSize /* host pointer */);

/** 
 * Device-buffer size (in bytes) for 64fc reductions.
 * This primitive provides the correct buffer size for nppsSum_64fc, 
 * nppsMin_64fc, nppsMax_64fc.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: hpBufferSize is a 
 *        <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus 
nppsReductionGetBufferSize_64fc(int nLength, int * hpBufferSize /* host pointer */);

/** 
 * 32-bit float vector sum method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param eHint Suggests using specific code.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_32f(const Npp32f * pSrc, int nLength, Npp32f * pSum, NppHintAlgorithm eHint, 
            Npp8u * pDeviceBuffer);

/** 
 * 32-bit float complex vector sum method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param eHint Suggests using specific code.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_32fc(const Npp32fc * pSrc, int nLength, Npp32fc * pSum, NppHintAlgorithm eHint, 
             Npp8u * pDeviceBuffer);

/** 
 * 64-bit double vector sum method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_64f(const Npp64f * pSrc, int nLength, Npp64f * pSum, Npp8u * pDeviceBuffer);

/** 
 * 64-bit double complex vector sum method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_64fc(const Npp64fc * pSrc, int nLength, Npp64fc * pSum, Npp8u * pDeviceBuffer);

/** 
 * 16-bit short vector sum with integer scaling method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \param nScaleFactor Integer-result scale factor.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_16s_Sfs(const Npp16s * pSrc, int nLength, Npp16s * pSum, int nScaleFactor, 
                Npp8u * pDeviceBuffer);

/** 
 * 32-bit integer vector sum with integer scaling method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \param nScaleFactor Integer-result scale factor.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_32s_Sfs(const Npp32s * pSrc, int nLength, Npp32s * pSum, int nScaleFactor, 
                Npp8u * pDeviceBuffer);

/** 
 * 16-bit short complex vector sum with integer scaling method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \param nScaleFactor Integer-result scale factor.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_16sc_Sfs(const Npp16sc * pSrc, int nLength, Npp16sc * pSum, int nScaleFactor, 
                 Npp8u * pDeviceBuffer);

/** 
 * 16-bit short complex vector sum (32bit int complex) with integer scaling
 * method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \param nScaleFactor Integer-result scale factor.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_16sc32sc_Sfs(const Npp16sc * pSrc, int nLength, Npp32sc * pSum, int nScaleFactor, 
                     Npp8u * pDeviceBuffer);

/** 
 * 16-bit integer vector sum (32bit) with integer scaling method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pSum Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \param nScaleFactor Integer-result scale factor.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsSum_16s32s_Sfs(const Npp16s * pSrc, int nLength, Npp32s * pSum, int nScaleFactor,
                   Npp8u * pDeviceBuffer);

/** 
 * 16-bit integer vector max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMax Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMax_16s(const Npp16s * pSrc, int nLength, Npp16s * pMax, Npp8u * pDeviceBuffer);

/** 
 * 32-bit integer vector max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMax Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMax_32s(const Npp32s * pSrc, int nLength, Npp32s * pMax, Npp8u * pDeviceBuffer);

/** 
 * 32-bit float vector max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMax Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMax_32f(const Npp32f * pSrc, int nLength, Npp32f * pMax, Npp8u * pDeviceBuffer);

/** 
 * 64-bit float vector max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMax Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMax_64f(const Npp64f * pSrc, int nLength, Npp64f * pMax, Npp8u * pDeviceBuffer);

/** 
 * 16-bit integer vector min method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMin_16s(const Npp16s * pSrc, int nLength, Npp16s * pMin, Npp8u * pDeviceBuffer);

/** 
 * 32-bit integer vector min method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMin_32s(const Npp32s * pSrc, int nLength, Npp32s * pMin, Npp8u * pDeviceBuffer);

/** 
 * 32-bit integer vector min method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMin_32f(const Npp32f * pSrc, int nLength, Npp32f * pMin, Npp8u * pDeviceBuffer);

/** 
 * 64-bit integer vector min method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMin_64f(const Npp64f * pSrc, int nLength, Npp64f * pMin, Npp8u * pDeviceBuffer);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_8u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_8u(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_16s.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_16s(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_16u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_16u(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_32s.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_32s(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_32u.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_32u(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_32f.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_32f(int nLength,  int * hpBufferSize);

/** 
 * Device-buffer size (in bytes) for nppsMinMax_64f.
 * \param nLength \ref length_specification.
 * \param hpBufferSize Required buffer size.  Important: 
 *        hpBufferSize is a <em>host pointer.</em>
 * \return NPP_SUCCESS
 */
NppStatus
nppsMinMaxGetBufferSize_64f(int nLength,  int * hpBufferSize);

/** 
 * 8-bit char vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_8u(const Npp8u * pSrc, int nLength, Npp8u * pMin, Npp8u * pMax, 
              Npp8u * pDeviceBuffer);

/** 
 * 16-bit signed short vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_16s(const Npp16s * pSrc, int nLength, Npp16s * pMin, Npp16s * pMax, 
               Npp8u * pDeviceBuffer);

/** 
 * 16-bit unsigned short vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_16u(const Npp16u * pSrc, int nLength, Npp16u * pMin, Npp16u * pMax, 
               Npp8u * pDeviceBuffer);

/** 
 * 32-bit unsigned int vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_32u(const Npp32u * pSrc, int nLength, Npp32u * pMin, Npp32u * pMax, 
               Npp8u * pDeviceBuffer);

/** 
 * 32-bit signed int vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_32s(const Npp32s * pSrc, int nLength, Npp32s * pMin, Npp32s * pMax, 
               Npp8u * pDeviceBuffer);

/** 
 * 32-bit float vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_32f(const Npp32f * pSrc, int nLength, Npp32f * pMin, Npp32f * pMax, 
               Npp8u * pDeviceBuffer);

/** 
 * 64-bit double vector min and max method
 * \param pSrc \ref source_signal_pointer.
 * \param nLength \ref length_specification.
 * \param pMin Pointer to the min output result.
 * \param pMax Pointer to the max output result.
 * \param pDeviceBuffer Pointer to the required device memory allocation.
 * \return \ref signal_data_error_codes, \ref length_error_codes.
 */
NppStatus 
nppsMinMax_64f(const Npp64f * pSrc, int nLength, Npp64f * pMin, Npp64f * pMax, 
               Npp8u * pDeviceBuffer);

// end of Statistical Functions
//@}


/** @name Filtering Functions
 * Functions that provide functionality of generating output signal
 * based on the input signal like signal integral, etc.
 */
//@{

NppStatus
nppsIntegralGetBufferSize_32s(int nLength,  int * hpBufferSize);


NppStatus 
nppsIntegral_32s(const Npp32s* pSrc, Npp32s* pDst, int nLength, Npp8u * pDeviceBuffer);

//end of Filtering Functions
//@}

// end of Signal Processing module
//@}
 

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NV_NPPS_H
