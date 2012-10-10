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
 
/*
 * This is the public header file for the CUBLAS library, defining the API
 *
 * CUBLAS is an implementation of BLAS (Basic Linear Algebra Subroutines) 
 * on top of the CUDA runtime. 
 */

#if !defined(CUBLAS_API_H_)
#define CUBLAS_API_H_

#ifndef CUBLASAPI
#ifdef _WIN32
#define CUBLASAPI __stdcall
#else
#define CUBLASAPI 
#endif
#endif

#include "driver_types.h"
#include "cuComplex.h"   /* import complex data type */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


/* CUBLAS status type returns */
typedef enum{
    CUBLAS_STATUS_SUCCESS         =0,
    CUBLAS_STATUS_NOT_INITIALIZED =1,
    CUBLAS_STATUS_ALLOC_FAILED    =3,
    CUBLAS_STATUS_INVALID_VALUE   =7,
    CUBLAS_STATUS_ARCH_MISMATCH   =8,
    CUBLAS_STATUS_MAPPING_ERROR   =11,
    CUBLAS_STATUS_EXECUTION_FAILED=13,
    CUBLAS_STATUS_INTERNAL_ERROR  =14
} cublasStatus_t;


typedef enum {
    CUBLAS_FILL_MODE_LOWER=0, 
    CUBLAS_FILL_MODE_UPPER=1
} cublasFillMode_t;

typedef enum {
    CUBLAS_DIAG_NON_UNIT=0, 
    CUBLAS_DIAG_UNIT=1
} cublasDiagType_t; 

typedef enum {
    CUBLAS_SIDE_LEFT =0, 
    CUBLAS_SIDE_RIGHT=1
} cublasSideMode_t; 


typedef enum {
    CUBLAS_OP_N=0,  
    CUBLAS_OP_T=1,  
    CUBLAS_OP_C=2  
} cublasOperation_t;


typedef enum { 
    CUBLAS_POINTER_MODE_HOST   = 0,  
    CUBLAS_POINTER_MODE_DEVICE = 1        
} cublasPointerMode_t;



/* Opaque structure holding CUBLAS library context */
struct cublasContext;
typedef struct cublasContext *cublasHandle_t;

cublasStatus_t CUBLASAPI cublasCreate_v2 (cublasHandle_t *handle);
cublasStatus_t CUBLASAPI cublasDestroy_v2 (cublasHandle_t handle);
cublasStatus_t CUBLASAPI cublasGetVersion_v2(cublasHandle_t handle, int *version);
cublasStatus_t CUBLASAPI cublasSetStream_v2 (cublasHandle_t handle, cudaStream_t streamId); 
cublasStatus_t CUBLASAPI cublasGetStream_v2 (cublasHandle_t handle, cudaStream_t *streamId); 

cublasStatus_t  CUBLASAPI cublasGetPointerMode_v2 (cublasHandle_t handle, cublasPointerMode_t *mode);
cublasStatus_t  CUBLASAPI cublasSetPointerMode_v2 (cublasHandle_t handle, cublasPointerMode_t mode);         


/* 
 * cublasStatus_t 
 * cublasSetVector (int n, int elemSize, const void *x, int incx, 
 *                  void *y, int incy) 
 *
 * copies n elements from a vector x in CPU memory space to a vector y 
 * in GPU memory space. Elements in both vectors are assumed to have a 
 * size of elemSize bytes. Storage spacing between consecutive elements
 * is incx for the source vector x and incy for the destination vector
 * y. In general, y points to an object, or part of an object, allocated
 * via cublasAlloc(). Column major format for two-dimensional matrices
 * is assumed throughout CUBLAS. Therefore, if the increment for a vector 
 * is equal to 1, this access a column vector while using an increment 
 * equal to the leading dimension of the respective matrix accesses a 
 * row vector.
 *
 * Return Values
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if incx, incy, or elemSize <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if an error occurred accessing GPU memory   
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasSetVector (int n, int elemSize, const void *x, 
                                        int incx, void *devicePtr, int incy);

/* 
 * cublasStatus_t 
 * cublasGetVector (int n, int elemSize, const void *x, int incx, 
 *                  void *y, int incy)
 * 
 * copies n elements from a vector x in GPU memory space to a vector y 
 * in CPU memory space. Elements in both vectors are assumed to have a 
 * size of elemSize bytes. Storage spacing between consecutive elements
 * is incx for the source vector x and incy for the destination vector
 * y. In general, x points to an object, or part of an object, allocated
 * via cublasAlloc(). Column major format for two-dimensional matrices
 * is assumed throughout CUBLAS. Therefore, if the increment for a vector 
 * is equal to 1, this access a column vector while using an increment 
 * equal to the leading dimension of the respective matrix accesses a 
 * row vector.
 *
 * Return Values
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if incx, incy, or elemSize <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if an error occurred accessing GPU memory   
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasGetVector (int n, int elemSize, const void *x, 
                                        int incx, void *y, int incy);

/*
 * cublasStatus_t 
 * cublasSetMatrix (int rows, int cols, int elemSize, const void *A, 
 *                  int lda, void *B, int ldb)
 *
 * copies a tile of rows x cols elements from a matrix A in CPU memory
 * space to a matrix B in GPU memory space. Each element requires storage
 * of elemSize bytes. Both matrices are assumed to be stored in column 
 * major format, with the leading dimension (i.e. number of rows) of 
 * source matrix A provided in lda, and the leading dimension of matrix B
 * provided in ldb. In general, B points to an object, or part of an 
 * object, that was allocated via cublasAlloc().
 *
 * Return Values 
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library has not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if rows or cols < 0, or elemSize, lda, or 
 *                                ldb <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if error occurred accessing GPU memory
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasSetMatrix (int rows, int cols, int elemSize, 
                                        const void *A, int lda, void *B, 
                                        int ldb);

/*
 * cublasStatus_t 
 * cublasGetMatrix (int rows, int cols, int elemSize, const void *A, 
 *                  int lda, void *B, int ldb)
 *
 * copies a tile of rows x cols elements from a matrix A in GPU memory
 * space to a matrix B in CPU memory space. Each element requires storage
 * of elemSize bytes. Both matrices are assumed to be stored in column 
 * major format, with the leading dimension (i.e. number of rows) of 
 * source matrix A provided in lda, and the leading dimension of matrix B
 * provided in ldb. In general, A points to an object, or part of an 
 * object, that was allocated via cublasAlloc().
 *
 * Return Values 
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library has not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if rows, cols, eleSize, lda, or ldb <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if error occurred accessing GPU memory
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasGetMatrix (int rows, int cols, int elemSize, 
                                        const void *A, int lda, void *B,
                                        int ldb);

/* 
 * cublasStatus 
 * cublasSetVectorAsync ( int n, int elemSize, const void *x, int incx, 
 *                       void *y, int incy, cudaStream_t stream );
 *
 * cublasSetVectorAsync has the same functionnality as cublasSetVector
 * but the transfer is done asynchronously within the CUDA stream passed
 * in parameter.
 *
 * Return Values
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if incx, incy, or elemSize <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if an error occurred accessing GPU memory   
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasSetVectorAsync (int n, int elemSize, 
                                             const void *hostPtr, int incx, 
                                             void *devicePtr, int incy,
                                             cudaStream_t stream);
/* 
 * cublasStatus 
 * cublasGetVectorAsync( int n, int elemSize, const void *x, int incx, 
 *                       void *y, int incy, cudaStream_t stream)
 * 
 * cublasGetVectorAsync has the same functionnality as cublasGetVector
 * but the transfer is done asynchronously within the CUDA stream passed
 * in parameter.
 *
 * Return Values
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if incx, incy, or elemSize <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if an error occurred accessing GPU memory   
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasGetVectorAsync (int n, int elemSize,
                                             const void *devicePtr, int incx,
                                             void *hostPtr, int incy,
                                             cudaStream_t stream);

/*
 * cublasStatus_t 
 * cublasSetMatrixAsync (int rows, int cols, int elemSize, const void *A, 
 *                       int lda, void *B, int ldb, cudaStream_t stream)
 *
 * cublasSetMatrixAsync has the same functionnality as cublasSetMatrix
 * but the transfer is done asynchronously within the CUDA stream passed
 * in parameter.
 *
 * Return Values 
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library has not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if rows or cols < 0, or elemSize, lda, or 
 *                                ldb <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if error occurred accessing GPU memory
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasSetMatrixAsync (int rows, int cols, int elemSize,
                                             const void *A, int lda, void *B,
                                             int ldb, cudaStream_t stream);

/*
 * cublasStatus_t 
 * cublasGetMatrixAsync (int rows, int cols, int elemSize, const void *A, 
 *                       int lda, void *B, int ldb, cudaStream_t stream)
 *
 * cublasGetMatrixAsync has the same functionnality as cublasGetMatrix
 * but the transfer is done asynchronously within the CUDA stream passed
 * in parameter.
 *
 * Return Values 
 * -------------
 * CUBLAS_STATUS_NOT_INITIALIZED  if CUBLAS library has not been initialized
 * CUBLAS_STATUS_INVALID_VALUE    if rows, cols, eleSize, lda, or ldb <= 0
 * CUBLAS_STATUS_MAPPING_ERROR    if error occurred accessing GPU memory
 * CUBLAS_STATUS_SUCCESS          if the operation completed successfully
 */
cublasStatus_t CUBLASAPI cublasGetMatrixAsync (int rows, int cols, int elemSize,
                                             const void *A, int lda, void *B,
                                             int ldb, cudaStream_t stream);


void CUBLASAPI cublasXerbla (const char *srName, int info);
/* ---------------- CUBLAS BLAS1 functions ---------------- */
cublasStatus_t CUBLASAPI cublasSnrm2_v2(cublasHandle_t handle, 
                                        int n, 
                                        const float *x, 
                                        int incx, 
                                        float *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDnrm2_v2(cublasHandle_t handle, 
                                        int n, 
                                        const double *x, 
                                        int incx, 
                                        double *result);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasScnrm2_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         float *result);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDznrm2_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         double *result);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasSdot_v2 (cublasHandle_t handle,
                                        int n, 
                                        const float *x, 
                                        int incx, 
                                        const float *y, 
                                        int incy,
                                        float *result);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDdot_v2 (cublasHandle_t handle,
                                        int n, 
                                        const double *x, 
                                        int incx, 
                                        const double *y,
                                        int incy,
                                        double *result);  /* host or device pointer */
    
cublasStatus_t CUBLASAPI cublasCdotu_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         const cuComplex *y, 
                                         int incy,
                                         cuComplex *result);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasCdotc_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         const cuComplex *y, 
                                         int incy,
                                         cuComplex *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasZdotu_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         const cuDoubleComplex *y, 
                                         int incy,
                                         cuDoubleComplex *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasZdotc_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx,
                                         const cuDoubleComplex *y, 
                                         int incy,
                                         cuDoubleComplex *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasSscal_v2(cublasHandle_t handle, 
                                        int n, 
                                        const float *alpha,  /* host or device pointer */
                                        float *x, 
                                        int incx);

cublasStatus_t CUBLASAPI cublasDscal_v2(cublasHandle_t handle, 
                                        int n, 
                                        const double *alpha,  /* host or device pointer */
                                        double *x, 
                                        int incx);

cublasStatus_t CUBLASAPI cublasCscal_v2(cublasHandle_t handle, 
                                        int n, 
                                        const cuComplex *alpha, /* host or device pointer */
                                        cuComplex *x, 
                                        int incx);

cublasStatus_t CUBLASAPI cublasCsscal_v2(cublasHandle_t handle, 
                                         int n, 
                                         const float *alpha, /* host or device pointer */
                                         cuComplex *x, 
                                         int incx);

cublasStatus_t CUBLASAPI cublasZscal_v2(cublasHandle_t handle, 
                                        int n, 
                                        const cuDoubleComplex *alpha, /* host or device pointer */
                                        cuDoubleComplex *x, 
                                        int incx);

cublasStatus_t CUBLASAPI cublasZdscal_v2(cublasHandle_t handle, 
                                         int n, 
                                         const double *alpha, /* host or device pointer */
                                         cuDoubleComplex *x, 
                                         int incx);

cublasStatus_t CUBLASAPI cublasSaxpy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const float *alpha, /* host or device pointer */
                                         const float *x, 
                                         int incx, 
                                         float *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasDaxpy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const double *alpha, /* host or device pointer */
                                         const double *x, 
                                         int incx, 
                                         double *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasCaxpy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuComplex *alpha, /* host or device pointer */
                                         const cuComplex *x, 
                                         int incx, 
                                         cuComplex *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasZaxpy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuDoubleComplex *alpha, /* host or device pointer */
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         cuDoubleComplex *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasScopy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const float *x, 
                                         int incx, 
                                         float *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasDcopy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const double *x, 
                                         int incx, 
                                         double *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasCcopy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         cuComplex *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasZcopy_v2 (cublasHandle_t handle,
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         cuDoubleComplex *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasSswap_v2 (cublasHandle_t handle,
                                         int n, 
                                         float *x, 
                                         int incx, 
                                         float *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasDswap_v2 (cublasHandle_t handle,
                                         int n, 
                                         double *x, 
                                         int incx, 
                                         double *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasCswap_v2 (cublasHandle_t handle,
                                         int n, 
                                         cuComplex *x, 
                                         int incx, 
                                         cuComplex *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasZswap_v2 (cublasHandle_t handle,
                                         int n, 
                                         cuDoubleComplex *x, 
                                         int incx, 
                                         cuDoubleComplex *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasIsamax_v2(cublasHandle_t handle, 
                                         int n, 
                                         const float *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIdamax_v2(cublasHandle_t handle, 
                                         int n, 
                                         const double *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIcamax_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIzamax_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIsamin_v2(cublasHandle_t handle, 
                                         int n, 
                                         const float *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIdamin_v2(cublasHandle_t handle, 
                                         int n, 
                                         const double *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIcamin_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasIzamin_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         int *result); /* host or device pointer */
 
cublasStatus_t CUBLASAPI cublasSasum_v2(cublasHandle_t handle, 
                                        int n, 
                                        const float *x, 
                                        int incx, 
                                        float *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDasum_v2(cublasHandle_t handle, 
                                        int n, 
                                        const double *x, 
                                        int incx, 
                                        double *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasScasum_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuComplex *x, 
                                         int incx, 
                                         float *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDzasum_v2(cublasHandle_t handle, 
                                         int n, 
                                         const cuDoubleComplex *x, 
                                         int incx, 
                                         double *result); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasSrot_v2 (cublasHandle_t handle, 
                                        int n, 
                                        float *x, 
                                        int incx, 
                                        float *y, 
                                        int incy, 
                                        const float *c,  /* host or device pointer */
                                        const float *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDrot_v2 (cublasHandle_t handle, 
                                        int n, 
                                        double *x, 
                                        int incx, 
                                        double *y, 
                                        int incy, 
                                        const double *c,  /* host or device pointer */
                                        const double *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasCrot_v2 (cublasHandle_t handle, 
                                        int n, 
                                        cuComplex *x, 
                                        int incx, 
                                        cuComplex *y, 
                                        int incy, 
                                        const float *c,      /* host or device pointer */
                                        const cuComplex *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasCsrot_v2(cublasHandle_t handle, 
                                        int n, 
                                        cuComplex *x, 
                                        int incx, 
                                        cuComplex *y, 
                                        int incy, 
                                        const float *c,  /* host or device pointer */
                                        const float *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasZrot_v2 (cublasHandle_t handle, 
                                        int n, 
                                        cuDoubleComplex *x, 
                                        int incx, 
                                        cuDoubleComplex *y, 
                                        int incy, 
                                        const double *c,            /* host or device pointer */
                                        const cuDoubleComplex *s);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasZdrot_v2(cublasHandle_t handle, 
                                        int n, 
                                        cuDoubleComplex *x, 
                                        int incx, 
                                        cuDoubleComplex *y, 
                                        int incy, 
                                        const double *c,  /* host or device pointer */
                                        const double *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasSrotg_v2(cublasHandle_t handle, 
                                        float *a,   /* host or device pointer */
                                        float *b,   /* host or device pointer */
                                        float *c,   /* host or device pointer */
                                        float *s);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDrotg_v2(cublasHandle_t handle, 
                                        double *a,  /* host or device pointer */
                                        double *b,  /* host or device pointer */
                                        double *c,  /* host or device pointer */
                                        double *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasCrotg_v2(cublasHandle_t handle, 
                                        cuComplex *a,  /* host or device pointer */
                                        cuComplex *b,  /* host or device pointer */
                                        float *c,      /* host or device pointer */
                                        cuComplex *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasZrotg_v2(cublasHandle_t handle, 
                                        cuDoubleComplex *a,  /* host or device pointer */
                                        cuDoubleComplex *b,  /* host or device pointer */
                                        double *c,           /* host or device pointer */
                                        cuDoubleComplex *s); /* host or device pointer */

cublasStatus_t CUBLASAPI cublasSrotm_v2(cublasHandle_t handle, 
                                        int n, 
                                        float *x, 
                                        int incx, 
                                        float *y, 
                                        int incy, 
                                        const float* param);  /* host or device pointer */

cublasStatus_t CUBLASAPI cublasDrotm_v2(cublasHandle_t handle, 
                                        int n, 
                                        double *x, 
                                        int incx, 
                                        double *y, 
                                        int incy, 
                                        const double* param);  /* host or device pointer */
        
cublasStatus_t CUBLASAPI cublasSrotmg_v2(cublasHandle_t handle, 
                                         float *d1,        /* host or device pointer */
                                         float *d2,        /* host or device pointer */
                                         float *x1,        /* host or device pointer */
                                         const float *y1,  /* host or device pointer */
                                         float *param);    /* host or device pointer */
                                         
cublasStatus_t CUBLASAPI cublasDrotmg_v2(cublasHandle_t handle, 
                                         double *d1,        /* host or device pointer */  
                                         double *d2,        /* host or device pointer */  
                                         double *x1,        /* host or device pointer */  
                                         const double *y1,  /* host or device pointer */  
                                         double *param);    /* host or device pointer */  

/* --------------- CUBLAS BLAS2 functions  ---------------- */

/* GEMV */
cublasStatus_t CUBLASAPI cublasSgemv_v2 (cublasHandle_t handle, 
                                         cublasOperation_t trans, 
                                         int m, 
                                         int n, 
                                         const float *alpha, /* host or device pointer */
                                         const float *A, 
                                         int lda, 
                                         const float *x, 
                                         int incx, 
                                         const float *beta,  /* host or device pointer */
                                         float *y, 
                                         int incy);  
 
cublasStatus_t CUBLASAPI cublasDgemv_v2 (cublasHandle_t handle, 
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         const double *alpha, /* host or device pointer */ 
                                         const double *A,
                                         int lda,
                                         const double *x,
                                         int incx,
                                         const double *beta, /* host or device pointer */
                                         double *y, 
                                         int incy);

cublasStatus_t CUBLASAPI cublasCgemv_v2 (cublasHandle_t handle,
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */ 
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *x, 
                                         int incx,
                                         const cuComplex *beta, /* host or device pointer */ 
                                         cuComplex *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasZgemv_v2 (cublasHandle_t handle,
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *A,
                                         int lda, 
                                         const cuDoubleComplex *x, 
                                         int incx,
                                         const cuDoubleComplex *beta, /* host or device pointer */  
                                         cuDoubleComplex *y,
                                         int incy);
/* GBMV */                                
cublasStatus_t CUBLASAPI cublasSgbmv_v2 (cublasHandle_t handle, 
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         int kl,
                                         int ku, 
                                         const float *alpha, /* host or device pointer */  
                                         const float *A, 
                                         int lda, 
                                         const float *x,
                                         int incx,
                                         const float *beta, /* host or device pointer */  
                                         float *y,
                                         int incy);                                
                                
cublasStatus_t CUBLASAPI cublasDgbmv_v2 (cublasHandle_t handle,
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         int kl,
                                         int ku, 
                                         const double *alpha, /* host or device pointer */ 
                                         const double *A,
                                         int lda, 
                                         const double *x,
                                         int incx,
                                         const double *beta, /* host or device pointer */ 
                                         double *y,
                                         int incy);
                                         
cublasStatus_t CUBLASAPI cublasCgbmv_v2 (cublasHandle_t handle,
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         int kl,
                                         int ku, 
                                         const cuComplex *alpha, /* host or device pointer */ 
                                         const cuComplex *A,
                                         int lda, 
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *beta, /* host or device pointer */ 
                                         cuComplex *y,
                                         int incy);                                             
                                         
cublasStatus_t CUBLASAPI cublasZgbmv_v2 (cublasHandle_t handle,
                                         cublasOperation_t trans, 
                                         int m,
                                         int n,
                                         int kl,
                                         int ku, 
                                         const cuDoubleComplex *alpha, /* host or device pointer */ 
                                         const cuDoubleComplex *A,
                                         int lda, 
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *beta, /* host or device pointer */ 
                                         cuDoubleComplex *y,
                                         int incy);   
                                         
/* TRMV */
cublasStatus_t CUBLASAPI cublasStrmv_v2 (cublasHandle_t handle,cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const float *A, int lda, float *x, int incx);                                                 

cublasStatus_t CUBLASAPI cublasDtrmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const double *A, int lda, double *x, int incx);

cublasStatus_t CUBLASAPI cublasCtrmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuComplex *A, int lda, cuComplex *x, int incx);
                                        
cublasStatus_t CUBLASAPI cublasZtrmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuDoubleComplex *A, int lda, 
                                         cuDoubleComplex *x, int incx);
                                                                                                             
/* TBMV */
cublasStatus_t CUBLASAPI cublasStbmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const float *A, int lda, float *x, int incx);                                                 

cublasStatus_t CUBLASAPI cublasDtbmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const double *A, int lda, double *x, int incx);

cublasStatus_t CUBLASAPI cublasCtbmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const cuComplex *A, int lda, cuComplex *x, int incx);
                                        
cublasStatus_t CUBLASAPI cublasZtbmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const cuDoubleComplex *A, int lda, 
                                         cuDoubleComplex *x, int incx);
                                         

/* TPMV */
cublasStatus_t CUBLASAPI cublasStpmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const float *AP, float *x, int incx);                                                 

cublasStatus_t CUBLASAPI cublasDtpmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const double *AP, double *x, int incx);

cublasStatus_t CUBLASAPI cublasCtpmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuComplex *AP, cuComplex *x, int incx);
                                         
cublasStatus_t CUBLASAPI cublasZtpmv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuDoubleComplex *AP, 
                                         cuDoubleComplex *x, int incx);

/* TRSV */
cublasStatus_t CUBLASAPI cublasStrsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const float *A, int lda, float *x, int incx);                                                 

cublasStatus_t CUBLASAPI cublasDtrsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const double *A, int lda, double *x, int incx);

cublasStatus_t CUBLASAPI cublasCtrsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuComplex *A, int lda, cuComplex *x, int incx);

cublasStatus_t CUBLASAPI cublasZtrsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuDoubleComplex *A, int lda, 
                                         cuDoubleComplex *x, int incx);

/* TPSV */
cublasStatus_t CUBLASAPI cublasStpsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const float *AP, 
                                         float *x, int incx);  
                                                                                                            
cublasStatus_t CUBLASAPI cublasDtpsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const double *AP, double *x, int incx);

cublasStatus_t CUBLASAPI cublasCtpsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuComplex *AP, cuComplex *x, int incx);

cublasStatus_t CUBLASAPI cublasZtpsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, const cuDoubleComplex *AP, 
                                         cuDoubleComplex *x, int incx);
/* TBSV */                                         
cublasStatus_t CUBLASAPI cublasStbsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const float *A, 
                                         int lda, float *x, int incx);

cublasStatus_t CUBLASAPI cublasDtbsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const double *A, 
                                         int lda, double *x, int incx);
                                         
cublasStatus_t CUBLASAPI cublasCtbsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const cuComplex *A, 
                                         int lda, cuComplex *x, int incx);
                                         
cublasStatus_t CUBLASAPI cublasZtbsv_v2 (cublasHandle_t handle, cublasFillMode_t uplo, cublasOperation_t trans, 
                                         cublasDiagType_t diag, int n, int k, const cuDoubleComplex *A, 
                                         int lda, cuDoubleComplex *x, int incx);     
                                         
/* SYMV/HEMV */
cublasStatus_t CUBLASAPI cublasSsymv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo, 
                                         int n,
                                         const float *alpha, /* host or device pointer */ 
                                         const float *A,
                                         int lda,
                                         const float *x,
                                         int incx,
                                         const float *beta, /* host or device pointer */ 
                                         float *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasDsymv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, 
                                         int n,
                                         const double *alpha, /* host or device pointer */ 
                                         const double *A,
                                         int lda,
                                         const double *x,
                                         int incx,
                                         const double *beta, /* host or device pointer */ 
                                         double *y,
                                         int incy);
                                     
cublasStatus_t CUBLASAPI cublasChemv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, 
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */ 
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *beta, /* host or device pointer */ 
                                         cuComplex *y,
                                         int incy);                                     
                                     
cublasStatus_t CUBLASAPI cublasZhemv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo, 
                                         int n,
                                         const cuDoubleComplex *alpha,  /* host or device pointer */ 
                                         const cuDoubleComplex *A,
                                         int lda,
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *beta,   /* host or device pointer */ 
                                         cuDoubleComplex *y,
                                         int incy);   
                                     
/* SBMV/HBMV */
cublasStatus_t CUBLASAPI cublasSsbmv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, 
                                         int n,
                                         int k,
                                         const float *alpha,   /* host or device pointer */ 
                                         const float *A,
                                         int lda,
                                         const float *x, 
                                         int incx,
                                         const float *beta,  /* host or device pointer */ 
                                         float *y,
                                         int incy);
                                      
cublasStatus_t CUBLASAPI cublasDsbmv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo, 
                                         int n,
                                         int k,
                                         const double *alpha,   /* host or device pointer */ 
                                         const double *A,
                                         int lda,
                                         const double *x, 
                                         int incx,
                                         const double *beta,   /* host or device pointer */ 
                                         double *y,
                                         int incy);
                                      
cublasStatus_t CUBLASAPI cublasChbmv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, 
                                         int n,
                                         int k,
                                         const cuComplex *alpha, /* host or device pointer */ 
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *x, 
                                         int incx,
                                         const cuComplex *beta, /* host or device pointer */ 
                                         cuComplex *y,
                                         int incy);
                                      
cublasStatus_t CUBLASAPI cublasZhbmv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, 
                                         int n,
                                         int k,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *A,
                                         int lda,
                                         const cuDoubleComplex *x, 
                                         int incx,
                                         const cuDoubleComplex *beta, /* host or device pointer */ 
                                         cuDoubleComplex *y,
                                         int incy);                                                                            
                                                                                                                                                   
/* SPMV/HPMV */
cublasStatus_t CUBLASAPI cublasSspmv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo,
                                         int n, 
                                         const float *alpha,  /* host or device pointer */                                           
                                         const float *AP,
                                         const float *x,
                                         int incx,
                                         const float *beta,   /* host or device pointer */  
                                         float *y,
                                         int incy);

cublasStatus_t CUBLASAPI cublasDspmv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo,
                                         int n,
                                         const double *alpha, /* host or device pointer */  
                                         const double *AP,
                                         const double *x,
                                         int incx,
                                         const double *beta,  /* host or device pointer */  
                                         double *y,
                                         int incy);                                     
                                     
cublasStatus_t CUBLASAPI cublasChpmv_v2 (cublasHandle_t handle, 
                                         cublasFillMode_t uplo,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *AP,
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *beta, /* host or device pointer */  
                                         cuComplex *y,
                                         int incy);
                                     
cublasStatus_t CUBLASAPI cublasZhpmv_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *AP,
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *beta, /* host or device pointer */  
                                         cuDoubleComplex *y, 
                                         int incy);

/* GER */
cublasStatus_t CUBLASAPI cublasSger_v2 (cublasHandle_t handle,
                                        int m,
                                        int n,
                                        const float *alpha, /* host or device pointer */  
                                        const float *x,
                                        int incx,
                                        const float *y,
                                        int incy,
                                        float *A,
                                        int lda);
                                    
cublasStatus_t CUBLASAPI cublasDger_v2 (cublasHandle_t handle, 
                                        int m,
                                        int n,
                                        const double *alpha, /* host or device pointer */   
                                        const double *x,
                                        int incx,
                                        const double *y,
                                        int incy,
                                        double *A,
                                        int lda);
                                    
cublasStatus_t CUBLASAPI cublasCgeru_v2 (cublasHandle_t handle, 
                                         int m,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *y,
                                         int incy,
                                         cuComplex *A,
                                         int lda);

cublasStatus_t CUBLASAPI cublasCgerc_v2 (cublasHandle_t handle,
                                         int m,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *y,
                                         int incy,
                                         cuComplex *A,
                                         int lda);                                   

cublasStatus_t CUBLASAPI cublasZgeru_v2 (cublasHandle_t handle, 
                                         int m,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *y,
                                         int incy,
                                         cuDoubleComplex *A,
                                         int lda);

cublasStatus_t CUBLASAPI cublasZgerc_v2 (cublasHandle_t handle,
                                         int m,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *y,
                                         int incy,
                                         cuDoubleComplex *A,
                                         int lda); 
                                    
/* SYR/HER */
cublasStatus_t CUBLASAPI cublasSsyr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const float *alpha, /* host or device pointer */  
                                        const float *x,
                                        int incx,
                                        float *A, 
                                        int lda);
                                    
cublasStatus_t CUBLASAPI cublasDsyr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const double *alpha, /* host or device pointer */  
                                        const double *x,
                                        int incx,
                                        double *A, 
                                        int lda);  
                                                                      
cublasStatus_t CUBLASAPI cublasCher_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const float *alpha, /* host or device pointer */  
                                        const cuComplex *x,
                                        int incx,
                                        cuComplex *A, 
                                        int lda); 
                                    
cublasStatus_t CUBLASAPI cublasZher_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const double *alpha, /* host or device pointer */  
                                        const cuDoubleComplex *x,
                                        int incx,
                                        cuDoubleComplex *A, 
                                        int lda); 

/* SPR/HPR */                                    
cublasStatus_t CUBLASAPI cublasSspr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const float *alpha, /* host or device pointer */  
                                        const float *x,
                                        int incx,
                                        float *AP);
                                    
cublasStatus_t CUBLASAPI cublasDspr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const double *alpha, /* host or device pointer */  
                                        const double *x,
                                        int incx,
                                        double *AP);

cublasStatus_t CUBLASAPI cublasChpr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const float *alpha, /* host or device pointer */  
                                        const cuComplex *x,
                                        int incx,
                                        cuComplex *AP);

cublasStatus_t CUBLASAPI cublasZhpr_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n,
                                        const double *alpha, /* host or device pointer */  
                                        const cuDoubleComplex *x,
                                        int incx,
                                        cuDoubleComplex *AP);                       
                                    
/* SYR2/HER2 */                                    
cublasStatus_t CUBLASAPI cublasSsyr2_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        int n, 
                                        const float *alpha, /* host or device pointer */  
                                        const float *x,
                                        int incx,
                                        const float *y,
                                        int incy,
                                        float *A,
                                        int lda);
                                    
cublasStatus_t CUBLASAPI cublasDsyr2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n, 
                                         const double *alpha, /* host or device pointer */  
                                         const double *x,
                                         int incx,
                                         const double *y,
                                         int incy,
                                         double *A,
                                         int lda);

cublasStatus_t CUBLASAPI cublasCher2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo, int n, 
                                         const cuComplex *alpha,  /* host or device pointer */  
                                         const cuComplex *x,
                                         int incx, 
                                         const cuComplex *y,
                                         int incy, 
                                         cuComplex *A, 
                                         int lda);   

cublasStatus_t CUBLASAPI cublasZher2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n, 
                                         const cuDoubleComplex *alpha,  /* host or device pointer */  
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *y,
                                         int incy,
                                         cuDoubleComplex *A,
                                         int lda);                       

/* SPR2/HPR2 */
cublasStatus_t CUBLASAPI cublasSspr2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n,
                                         const float *alpha,  /* host or device pointer */  
                                         const float *x,
                                         int incx,
                                         const float *y,
                                         int incy,
                                         float *AP);
                                                                          
cublasStatus_t CUBLASAPI cublasDspr2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n,
                                         const double *alpha,  /* host or device pointer */  
                                         const double *x,
                                         int incx, 
                                         const double *y,
                                         int incy,
                                         double *AP);
                                     

cublasStatus_t CUBLASAPI cublasChpr2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *x,
                                         int incx,
                                         const cuComplex *y,
                                         int incy,
                                         cuComplex *AP);
                                     
cublasStatus_t CUBLASAPI cublasZhpr2_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *x,
                                         int incx,
                                         const cuDoubleComplex *y,
                                         int incy,
                                         cuDoubleComplex *AP);                                     
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
/* ---------------- CUBLAS BLAS3 functions ---------------- */

/* GEMM */
cublasStatus_t CUBLASAPI cublasSgemm_v2 (cublasHandle_t handle, 
                                        cublasOperation_t transa,
                                        cublasOperation_t transb, 
                                        int m,
                                        int n,
                                        int k,
                                        const float *alpha, /* host or device pointer */  
                                        const float *A, 
                                        int lda,
                                        const float *B,
                                        int ldb, 
                                        const float *beta, /* host or device pointer */  
                                        float *C,
                                        int ldc);

cublasStatus_t CUBLASAPI cublasDgemm_v2 (cublasHandle_t handle, 
                                        cublasOperation_t transa,
                                        cublasOperation_t transb, 
                                        int m,
                                        int n,
                                        int k,
                                        const double *alpha, /* host or device pointer */  
                                        const double *A, 
                                        int lda,
                                        const double *B,
                                        int ldb, 
                                        const double *beta, /* host or device pointer */  
                                        double *C,
                                        int ldc);
                                        
cublasStatus_t CUBLASAPI cublasCgemm_v2 (cublasHandle_t handle, 
                                        cublasOperation_t transa,
                                        cublasOperation_t transb, 
                                        int m,
                                        int n,
                                        int k,
                                        const cuComplex *alpha, /* host or device pointer */  
                                        const cuComplex *A, 
                                        int lda,
                                        const cuComplex *B,
                                        int ldb, 
                                        const cuComplex *beta, /* host or device pointer */  
                                        cuComplex *C,
                                        int ldc);
                                        
cublasStatus_t CUBLASAPI cublasZgemm_v2 (cublasHandle_t handle, 
                                        cublasOperation_t transa,
                                        cublasOperation_t transb, 
                                        int m,
                                        int n,
                                        int k,
                                        const cuDoubleComplex *alpha, /* host or device pointer */  
                                        const cuDoubleComplex *A, 
                                        int lda,
                                        const cuDoubleComplex *B,
                                        int ldb, 
                                        const cuDoubleComplex *beta, /* host or device pointer */  
                                        cuDoubleComplex *C,
                                        int ldc);                                                                                
                            
/* SYRK */
cublasStatus_t CUBLASAPI cublasSsyrk_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        int n,
                                        int k,
                                        const float *alpha, /* host or device pointer */  
                                        const float *A,
                                        int lda,
                                        const float *beta, /* host or device pointer */  
                                        float *C,
                                        int ldc);
                                     
cublasStatus_t CUBLASAPI cublasDsyrk_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         int n,
                                         int k,
                                         const double *alpha,  /* host or device pointer */  
                                         const double *A,
                                         int lda,
                                         const double *beta,  /* host or device pointer */  
                                         double *C,
                                         int ldc);   
                                     
cublasStatus_t CUBLASAPI cublasCsyrk_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         int n,
                                         int k,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *beta, /* host or device pointer */  
                                         cuComplex *C,
                                         int ldc);         
                                     
cublasStatus_t CUBLASAPI cublasZsyrk_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         int n,
                                         int k,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *A,
                                         int lda,
                                         const cuDoubleComplex *beta, /* host or device pointer */  
                                         cuDoubleComplex *C, 
                                         int ldc);
/* HERK */
cublasStatus_t CUBLASAPI cublasCherk_v2 (cublasHandle_t handle,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         int n,
                                         int k,
                                         const float *alpha,  /* host or device pointer */  
                                         const cuComplex *A,
                                         int lda,
                                         const float *beta,   /* host or device pointer */  
                                         cuComplex *C,
                                         int ldc);
                                     
cublasStatus_t CUBLASAPI cublasZherk_v2 (cublasHandle_t handle,
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        int n,
                                        int k,
                                        const double *alpha,  /* host or device pointer */  
                                        const cuDoubleComplex *A,
                                        int lda,
                                        const double *beta,  /* host or device pointer */  
                                        cuDoubleComplex *C,
                                        int ldc);    

/* SYR2K */                                     
cublasStatus_t CUBLASAPI cublasSsyr2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans,
                                          int n,
                                          int k,
                                          const float *alpha, /* host or device pointer */  
                                          const float *A,
                                          int lda,
                                          const float *B,
                                          int ldb,
                                          const float *beta, /* host or device pointer */  
                                          float *C,
                                          int ldc);  
                                      
cublasStatus_t CUBLASAPI cublasDsyr2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans,
                                          int n,
                                          int k,
                                          const double *alpha, /* host or device pointer */  
                                          const double *A,
                                          int lda,
                                          const double *B,
                                          int ldb,
                                          const double *beta, /* host or device pointer */  
                                          double *C,
                                          int ldc);
                                      
cublasStatus_t CUBLASAPI cublasCsyr2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans,
                                          int n,
                                          int k,
                                          const cuComplex *alpha, /* host or device pointer */  
                                          const cuComplex *A,
                                          int lda,
                                          const cuComplex *B,
                                          int ldb,
                                          const cuComplex *beta, /* host or device pointer */  
                                          cuComplex *C,
                                          int ldc);
                                      
cublasStatus_t CUBLASAPI cublasZsyr2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans,
                                          int n,
                                          int k,
                                          const cuDoubleComplex *alpha,  /* host or device pointer */  
                                          const cuDoubleComplex *A,
                                          int lda,
                                          const cuDoubleComplex *B,
                                          int ldb,
                                          const cuDoubleComplex *beta,  /* host or device pointer */  
                                          cuDoubleComplex *C,
                                          int ldc);  
/* HER2K */                                       
cublasStatus_t CUBLASAPI cublasCher2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans,
                                          int n,
                                          int k,
                                          const cuComplex *alpha, /* host or device pointer */  
                                          const cuComplex *A,
                                          int lda,
                                          const cuComplex *B,
                                          int ldb,
                                          const float *beta,   /* host or device pointer */  
                                          cuComplex *C,
                                          int ldc);  
                                      
cublasStatus_t CUBLASAPI cublasZher2k_v2 (cublasHandle_t handle,
                                          cublasFillMode_t uplo,
                                          cublasOperation_t trans, 
                                          int n,
                                          int k,
                                          const cuDoubleComplex *alpha, /* host or device pointer */  
                                          const cuDoubleComplex *A, 
                                          int lda,
                                          const cuDoubleComplex *B,
                                          int ldb,
                                          const double *beta, /* host or device pointer */  
                                          cuDoubleComplex *C,
                                          int ldc);        
                                      
/* SYMM */
cublasStatus_t CUBLASAPI cublasSsymm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m,
                                         int n,
                                         const float *alpha, /* host or device pointer */  
                                         const float *A,
                                         int lda,
                                         const float *B,
                                         int ldb,
                                         const float *beta, /* host or device pointer */  
                                         float *C,
                                         int ldc);
                                     
cublasStatus_t CUBLASAPI cublasDsymm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m, 
                                         int n,
                                         const double *alpha, /* host or device pointer */  
                                         const double *A,
                                         int lda,
                                         const double *B,
                                         int ldb,
                                         const double *beta, /* host or device pointer */  
                                         double *C,
                                         int ldc);                                     

cublasStatus_t CUBLASAPI cublasCsymm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *B,
                                         int ldb,
                                         const cuComplex *beta, /* host or device pointer */  
                                         cuComplex *C,
                                         int ldc);
                                                   
cublasStatus_t CUBLASAPI cublasZsymm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *A,
                                         int lda,
                                         const cuDoubleComplex *B,
                                         int ldb,
                                         const cuDoubleComplex *beta, /* host or device pointer */  
                                         cuDoubleComplex *C,
                                         int ldc);   
                                     
/* HEMM */
cublasStatus_t CUBLASAPI cublasChemm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m,
                                         int n,
                                         const cuComplex *alpha, /* host or device pointer */  
                                         const cuComplex *A,
                                         int lda,
                                         const cuComplex *B,
                                         int ldb,
                                         const cuComplex *beta, /* host or device pointer */  
                                         cuComplex *C, 
                                         int ldc);                                                                                                                                                                                                                                                                                                                                                                                                                                                         

cublasStatus_t CUBLASAPI cublasZhemm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         int m,
                                         int n,
                                         const cuDoubleComplex *alpha, /* host or device pointer */  
                                         const cuDoubleComplex *A,
                                         int lda,
                                         const cuDoubleComplex *B,
                                         int ldb,
                                         const cuDoubleComplex *beta, /* host or device pointer */  
                                         cuDoubleComplex *C,
                                         int ldc); 
                                     
/* TRSM */                                                                         
cublasStatus_t CUBLASAPI cublasStrsm_v2 (cublasHandle_t handle, 
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         cublasDiagType_t diag,
                                         int m,
                                         int n,
                                         const float *alpha, /* host or device pointer */  
                                         const float *A,
                                         int lda,
                                         float *B,
                                         int ldb);
                            

cublasStatus_t CUBLASAPI cublasDtrsm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         cublasDiagType_t diag,
                                         int m,
                                         int n,
                                         const double *alpha, /* host or device pointer */  
                                         const double *A, 
                                         int lda, 
                                         double *B,
                                         int ldb);
                            
cublasStatus_t CUBLASAPI cublasCtrsm_v2(cublasHandle_t handle,
                                        cublasSideMode_t side,
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        cublasDiagType_t diag,
                                        int m,
                                        int n,
                                        const cuComplex *alpha, /* host or device pointer */  
                                        const cuComplex *A,
                                        int lda,
                                        cuComplex *B,
                                        int ldb );
                  
cublasStatus_t CUBLASAPI cublasZtrsm_v2(cublasHandle_t handle, 
                                        cublasSideMode_t side,
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        cublasDiagType_t diag,
                                        int m,
                                        int n,
                                        const cuDoubleComplex *alpha, /* host or device pointer */  
                                        const cuDoubleComplex *A,                                        
                                        int lda,
                                        cuDoubleComplex *B,
                                        int ldb );              
                                                
 /* TRMM */  
cublasStatus_t CUBLASAPI cublasStrmm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         cublasDiagType_t diag,
                                         int m,
                                         int n,
                                         const float *alpha, /* host or device pointer */  
                                         const float *A,
                                         int lda, 
                                         const float *B,
                                         int ldb,
                                         float *C,
                                         int ldc);
                                               
cublasStatus_t CUBLASAPI cublasDtrmm_v2 (cublasHandle_t handle,
                                         cublasSideMode_t side,
                                         cublasFillMode_t uplo,
                                         cublasOperation_t trans,
                                         cublasDiagType_t diag,
                                         int m,
                                         int n,
                                         const double *alpha, /* host or device pointer */  
                                         const double *A,
                                         int lda,
                                         const double *B,
                                         int ldb,
                                         double *C,
                                         int ldc)  ;
                                     
cublasStatus_t CUBLASAPI cublasCtrmm_v2(cublasHandle_t handle,
                                        cublasSideMode_t side,
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        cublasDiagType_t diag,
                                        int m,
                                        int n,
                                        const cuComplex *alpha, /* host or device pointer */  
                                        const cuComplex *A,
                                        int lda,
                                        const cuComplex *B,
                                        int ldb,
                                        cuComplex *C,
                                        int ldc);
                  
cublasStatus_t CUBLASAPI cublasZtrmm_v2(cublasHandle_t handle, cublasSideMode_t side, 
                                        cublasFillMode_t uplo,
                                        cublasOperation_t trans,
                                        cublasDiagType_t diag,
                                        int m,
                                        int n,
                                        const cuDoubleComplex *alpha, /* host or device pointer */  
                                        const cuDoubleComplex *A,
                                        int lda,
                                        const cuDoubleComplex *B,
                                        int ldb,
                                        cuDoubleComplex *C,
                                        int ldc  );                                                                                                                                                                                                                                                                                             
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* !defined(CUBLAS_API_H_) */
