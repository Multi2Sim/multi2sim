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

#if !defined(CUBLAS_H_)
#define CUBLAS_H_

#include <cuda_runtime.h>
#include "cublas_api.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* CUBLAS data types */
#define cublasStatus cublasStatus_t

cublasStatus CUBLASAPI cublasInit (void);
cublasStatus CUBLASAPI cublasShutdown (void);
cublasStatus CUBLASAPI cublasGetError (void);
cublasHandle_t CUBLASAPI cublasGetCurrentCtx(void);
cublasStatus_t CUBLASAPI cublasSetError (cublasStatus_t error);



static __inline__ cublasStatus CUBLASAPI cublasGetVersion(int *version)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    return cublasGetVersion_v2(handle, version);
}

static __inline__ cublasStatus CUBLASAPI cublasAlloc (int n, int elemSize, void **devicePtr)
{
    cudaError_t cudaStat;
    unsigned long long int testSize;
    size_t allocSize;
    *devicePtr = 0;

    if ((n <= 0) || (elemSize <= 0)) {        
        return CUBLAS_STATUS_INVALID_VALUE;
    }
    allocSize = (size_t)elemSize * (size_t)n;
    testSize = (unsigned long long int)elemSize * (unsigned long long int)n;
    if (testSize != (unsigned long long int)allocSize) { /* size_t overflow! */
        return CUBLAS_STATUS_ALLOC_FAILED;
    }
    cudaStat = cudaMalloc (devicePtr, allocSize);
    if (cudaStat != cudaSuccess) {
        return CUBLAS_STATUS_ALLOC_FAILED;
    }
    return CUBLAS_STATUS_SUCCESS;
}

static __inline__ cublasStatus CUBLASAPI cublasFree (void *devicePtr)
{
    cudaError_t cudaStat;

   if (devicePtr) {
        cudaStat = cudaFree (devicePtr);
        if (cudaStat != cudaSuccess) {
            /* should never fail, except when there is internal corruption*/
            return CUBLAS_STATUS_INTERNAL_ERROR;
        }
    }
    return CUBLAS_STATUS_SUCCESS;
}


static __inline__ cublasStatus CUBLASAPI cublasSetKernelStream (cudaStream_t stream)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    return cublasSetStream_v2(handle, stream);
}


/* Helper functions */
static __inline__ cublasOperation_t convertToOp( char trans ) 
{
    switch(trans) {
        case 'N':
        case 'n':
            return CUBLAS_OP_N;
        case 't':
        case 'T':
            return CUBLAS_OP_T;
        case 'C':
        case 'c':
            return CUBLAS_OP_C;                        
        default:
            return CUBLAS_OP_N;
    }

}
static __inline__ cublasFillMode_t convertToFillMode( char uplo ) 
{
    switch (uplo) {
        case 'U':
        case 'u':
            return CUBLAS_FILL_MODE_UPPER;
        case 'L':
        case 'l':
        default:
         return CUBLAS_FILL_MODE_LOWER;
    }        
}

static __inline__ cublasDiagType_t convertToDiagType( char diag ) 
{
    switch (diag) {
        case 'U':
        case 'u':
            return CUBLAS_DIAG_UNIT;
        case 'N':
        case 'n':
        default:
         return CUBLAS_DIAG_NON_UNIT;
    }        
}

static __inline__ cublasSideMode_t convertToSideMode( char side ) 
{
    switch (side) {
        case 'R':
        case 'r':
            return CUBLAS_SIDE_RIGHT;
        case 'L':
        case 'l':
        default:
         return CUBLAS_SIDE_LEFT;
    }        
}



/* ---------------- CUBLAS BLAS1 functions ---------------- */
/* NRM2 */
static __inline__ float CUBLASAPI cublasSnrm2 (int n, const float *x, int incx)
{
    float  result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSnrm2_v2(handle, n, x, incx, &result);
    cublasSetError(error);
    return result;
}
static __inline__ double CUBLASAPI cublasDnrm2 (int n, const double *x, int incx)
{
    double  result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDnrm2_v2(handle, n, x, incx, &result);
    cublasSetError(error);
    return result;
}
static __inline__ float CUBLASAPI cublasScnrm2 (int n, const cuComplex *x, int incx)
{
    float  result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasScnrm2_v2(handle, n, x, incx, &result);
    cublasSetError(error);
    return result;
}
static __inline__ double CUBLASAPI cublasDznrm2 (int n, const cuDoubleComplex *x, int incx)
{
    double  result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDznrm2_v2(handle, n, x, incx, &result);
    cublasSetError(error);
    return result;
}
/*------------------------------------------------------------------------*/
/* DOT */
static __inline__ float CUBLASAPI cublasSdot (int n, const float *x, int incx, const float *y, 
                            int incy)
{
    float result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSdot_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
    return result;
}  
static __inline__ double CUBLASAPI cublasDdot (int n, const double *x, int incx, const double *y, 
                            int incy)
{
    double result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDdot_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
    return result;
}  
static __inline__ cuComplex CUBLASAPI cublasCdotu (int n, const cuComplex *x, int incx, const cuComplex *y, 
                            int incy)
{
    cuComplex result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCdotu_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
    return result;
} 
static __inline__ cuComplex CUBLASAPI cublasCdotc (int n, const cuComplex *x, int incx, const cuComplex *y, 
                            int incy)
{
    cuComplex result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCdotc_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
	return result;
} 
 
static __inline__ cuDoubleComplex CUBLASAPI cublasZdotu (int n, const cuDoubleComplex *x, int incx, const cuDoubleComplex *y, 
                            int incy)
{
    cuDoubleComplex result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZdotu_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
	return result;
}   
static __inline__ cuDoubleComplex CUBLASAPI cublasZdotc (int n, const cuDoubleComplex *x, int incx, const cuDoubleComplex *y, 
                            int incy)
{
    cuDoubleComplex result;
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZdotc_v2(handle, n, x, incx, y, incy, &result);
    cublasSetError(error);
	return result;
}                 
/*------------------------------------------------------------------------*/
/* SCAL */
static __inline__ void CUBLASAPI cublasSscal (int n, float alpha, float *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasDscal (int n, double alpha, double *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasCscal (int n, cuComplex alpha, cuComplex *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasZscal (int n, cuDoubleComplex alpha, cuDoubleComplex *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}

static __inline__ void CUBLASAPI cublasCsscal (int n, float alpha, cuComplex *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCsscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasZdscal (int n, double alpha, cuDoubleComplex *x, int incx)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZdscal_v2(handle, n, &alpha, x, incx);
    cublasSetError(error);
}      
/*------------------------------------------------------------------------*/
/* AXPY */
static __inline__ void CUBLASAPI cublasSaxpy (int n, float alpha, const float *x, int incx, 
                            float *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSaxpy_v2(handle, n, &alpha, x, incx, y, incy);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasDaxpy (int n, double alpha, const double *x, 
                            int incx, double *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDaxpy_v2(handle, n, &alpha, x, incx, y, incy);
    cublasSetError(error);
} 
static __inline__ void CUBLASAPI cublasCaxpy (int n, cuComplex alpha, const cuComplex *x, 
                            int incx, cuComplex *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCaxpy_v2(handle, n, &alpha, x, incx, y, incy);
    cublasSetError(error);
} 
static __inline__ void CUBLASAPI cublasZaxpy (int n, cuDoubleComplex alpha, const cuDoubleComplex *x, 
                            int incx, cuDoubleComplex *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZaxpy_v2(handle, n, &alpha, x, incx, y, incy);
    cublasSetError(error);
}                            
/*------------------------------------------------------------------------*/
/* COPY */
static __inline__ void CUBLASAPI cublasScopy (int n, const float *x, int incx, float *y, 
                            int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasScopy_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}  
static __inline__ void CUBLASAPI cublasDcopy (int n, const double *x, int incx, double *y, 
                            int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDcopy_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}                            
static __inline__ void CUBLASAPI cublasCcopy (int n, const cuComplex *x, int incx, cuComplex *y,
                            int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCcopy_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}  
static __inline__ void CUBLASAPI cublasZcopy (int n, const cuDoubleComplex *x, int incx, cuDoubleComplex *y,
                            int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZcopy_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}     
/*------------------------------------------------------------------------*/
/* SWAP */
static __inline__ void CUBLASAPI cublasSswap (int n, float *x, int incx, float *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSswap_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}         
static __inline__ void CUBLASAPI cublasDswap (int n, double *x, int incx, double *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDswap_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}         
static __inline__ void CUBLASAPI cublasCswap (int n, cuComplex *x, int incx, cuComplex *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCswap_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
} 
static __inline__ void CUBLASAPI cublasZswap (int n, cuDoubleComplex *x, int incx, cuDoubleComplex *y, int incy)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZswap_v2(handle,n,x,incx,y,incy);
    cublasSetError(error);
}                                                                                                                         
/*------------------------------------------------------------------------*/
/* AMAX */
static __inline__ int CUBLASAPI cublasIsamax (int n, const float *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIsamax_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}

static __inline__ int CUBLASAPI cublasIdamax (int n, const double *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIdamax_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;   
}

static __inline__ int CUBLASAPI cublasIcamax (int n, const cuComplex *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIcamax_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
    
}

static __inline__ int CUBLASAPI cublasIzamax (int n, const cuDoubleComplex *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIzamax_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;    
}
/*------------------------------------------------------------------------*/
/* AMIN */
static __inline__ int CUBLASAPI cublasIsamin (int n, const float *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIsamin_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}

static __inline__ int CUBLASAPI cublasIdamin (int n, const double *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIdamin_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}

static __inline__ int CUBLASAPI cublasIcamin (int n, const cuComplex *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIcamin_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}

static __inline__ int CUBLASAPI cublasIzamin (int n, const cuDoubleComplex *x, int incx)
{
    int result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasIzamin_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}
/*------------------------------------------------------------------------*/
/* ASUM */
static __inline__ float CUBLASAPI cublasSasum (int n, const float *x, int incx)
{
    float result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasSasum_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}
static __inline__ double CUBLASAPI cublasDasum (int n, const double *x, int incx)
{
    double result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasDasum_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}
static __inline__ float CUBLASAPI cublasScasum (int n, const cuComplex *x, int incx)
{
    float result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasScasum_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}
static __inline__ double CUBLASAPI cublasDzasum (int n, const cuDoubleComplex *x, int incx)
{
    double result;
    cublasHandle_t handle = cublasGetCurrentCtx();    
    cublasStatus_t error = cublasDzasum_v2(handle, n, x, incx, &result);
    cublasSetError(error);
	return result;
}
/*------------------------------------------------------------------------*/
/* ROT */
static __inline__ void CUBLASAPI cublasSrot (int n, float *x, int incx, float *y, int incy, 
                           float sc, float ss)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSrot_v2(handle, n, x, incx, y, incy, &sc, &ss);
    cublasSetError(error);
}    
static __inline__ void CUBLASAPI cublasDrot (int n, double *x, int incx, double *y, int incy, 
                           double sc, double ss)
{
    cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDrot_v2(handle, n, x, incx, y, incy, &sc, &ss);
    cublasSetError(error);
}      
static __inline__ void CUBLASAPI cublasCrot (int n, cuComplex *x, int incx, cuComplex *y, 
                           int incy, float c, cuComplex s)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCrot_v2(handle, n, x, incx, y, incy, &c, &s);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasZrot (int n, cuDoubleComplex *x, int incx, 
                           cuDoubleComplex *y, int incy, double sc, 
                           cuDoubleComplex cs)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZrot_v2(handle, n, x, incx, y, incy, &sc, &cs);
    cublasSetError(error);
}                                                                                                  

static __inline__ void CUBLASAPI cublasCsrot (int n, cuComplex *x, int incx, cuComplex *y,
                            int incy, float c, float s)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCsrot_v2(handle, n, x, incx, y, incy, &c, &s);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasZdrot (int n, cuDoubleComplex *x, int incx, 
                            cuDoubleComplex *y, int incy, double c, double s)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZdrot_v2(handle, n, x, incx, y, incy, &c, &s);
    cublasSetError(error);
}        
/*------------------------------------------------------------------------*/
/* ROTG */
static __inline__ void CUBLASAPI cublasSrotg (float *sa, float *sb, float *sc, float *ss)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSrotg_v2(handle, sa, sb, sc, ss);
    cublasSetError(error);
} 
static __inline__ void CUBLASAPI cublasDrotg (double *sa, double *sb, double *sc, double *ss)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDrotg_v2(handle, sa, sb, sc, ss);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasCrotg (cuComplex *ca, cuComplex cb, float *sc,
                                     cuComplex *cs)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCrotg_v2(handle, ca, &cb, sc, cs);
    cublasSetError(error);
}
static __inline__ void CUBLASAPI cublasZrotg (cuDoubleComplex *ca, cuDoubleComplex cb, double *sc,
                            cuDoubleComplex *cs)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZrotg_v2(handle, ca, &cb, sc, cs);
    cublasSetError(error);
}                                                                                                               
/*------------------------------------------------------------------------*/
/* ROTM */
static __inline__ void CUBLASAPI cublasSrotm(int n, float *x, int incx, float *y, int incy, 
                           const float* sparam)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSrotm_v2(handle, n, x, incx, y, incy, sparam);
    cublasSetError(error);
}                           
static __inline__ void CUBLASAPI cublasDrotm(int n, double *x, int incx, double *y, int incy, 
                           const double* sparam)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDrotm_v2(handle, n, x, incx, y, incy, sparam);
    cublasSetError(error);
}
/*------------------------------------------------------------------------*/
/* ROTMG */
static __inline__ void CUBLASAPI cublasSrotmg (float *sd1, float *sd2, float *sx1, 
                             const float *sy1, float* sparam)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSrotmg_v2(handle, sd1, sd2, sx1, sy1, sparam);
    cublasSetError(error);
}

static __inline__ void CUBLASAPI cublasDrotmg (double *sd1, double *sd2, double *sx1, 
                             const double *sy1, double* sparam)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDrotmg_v2(handle, sd1, sd2, sx1, sy1, sparam);
    cublasSetError(error);
}                                                          
                           
/* --------------- CUBLAS BLAS2 functions  ---------------- */
/* GEMV */
static __inline__ void CUBLASAPI cublasSgemv (char trans, int m, int n, float alpha,
                            const float *A, int lda, const float *x, int incx,
                            float beta, float *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSgemv_v2(handle, convertToOp(trans), m,n,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasDgemv (char trans, int m, int n, double alpha,
                            const double *A, int lda, const double *x, int incx,
                            double beta, double *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDgemv_v2(handle, convertToOp(trans), m,n,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasCgemv (char trans, int m, int n, cuComplex alpha,
                            const cuComplex *A, int lda, const cuComplex *x, int incx,
                            cuComplex beta, cuComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCgemv_v2(handle, convertToOp(trans), m,n,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasZgemv (char trans, int m, int n, cuDoubleComplex alpha,
                            const cuDoubleComplex *A, int lda, const cuDoubleComplex *x, int incx,
                            cuDoubleComplex beta, cuDoubleComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZgemv_v2(handle, convertToOp(trans), m,n,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
} 
/*------------------------------------------------------------------------*/
/* GBMV */
static __inline__ void CUBLASAPI cublasSgbmv (char trans, int m, int n, int kl, int ku, 
                            float alpha, const float *A, int lda, 
                            const float *x, int incx, float beta, float *y, 
                            int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSgbmv_v2(handle, convertToOp(trans), m,n,kl,ku,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasDgbmv (char trans, int m, int n, int kl, int ku, 
                            double alpha, const double *A, int lda, 
                            const double *x, int incx, double beta, double *y, 
                            int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDgbmv_v2(handle, convertToOp(trans), m,n,kl,ku,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasCgbmv (char trans, int m, int n, int kl, int ku, 
                            cuComplex alpha, const cuComplex *A, int lda, 
                            const cuComplex *x, int incx, cuComplex beta, cuComplex *y, 
                            int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCgbmv_v2(handle, convertToOp(trans), m,n,kl,ku,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasZgbmv (char trans, int m, int n, int kl, int ku, 
                            cuDoubleComplex alpha, const cuDoubleComplex *A, int lda, 
                            const cuDoubleComplex *x, int incx, cuDoubleComplex beta, cuDoubleComplex *y, 
                            int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZgbmv_v2(handle, convertToOp(trans), m,n,kl,ku,
                                    &alpha, A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                                                       
/*------------------------------------------------------------------------*/
/* TRMV */
static __inline__ void CUBLASAPI cublasStrmv (char uplo, char trans, char diag, int n, 
                            const float *A, int lda, float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStrmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );    
    cublasSetError(error);                                   
} 
static __inline__ void CUBLASAPI cublasDtrmv (char uplo, char trans, char diag, int n, 
                            const double *A, int lda, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtrmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );    
    cublasSetError(error);                                   
} 
static __inline__ void CUBLASAPI cublasCtrmv (char uplo, char trans, char diag, int n, 
                            const cuComplex *A, int lda, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtrmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );    
    cublasSetError(error);                                   
} 
static __inline__ void CUBLASAPI cublasZtrmv (char uplo, char trans, char diag, int n, 
                            const cuDoubleComplex *A, int lda, cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtrmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );    
    cublasSetError(error);                                   
} 
/*------------------------------------------------------------------------*/
/* TBMV */
static __inline__ void CUBLASAPI cublasStbmv (char uplo, char trans, char diag, int n, int k, 
                            const float *A, int lda, float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStbmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                      
} 
static __inline__ void CUBLASAPI cublasDtbmv (char uplo, char trans, char diag, int n, int k, 
                            const double *A, int lda, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtbmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                      
} 
static __inline__ void CUBLASAPI cublasCtbmv (char uplo, char trans, char diag, int n, int k, 
                            const cuComplex *A, int lda, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtbmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                      
} 
static __inline__ void CUBLASAPI cublasZtbmv (char uplo, char trans, char diag, int n, int k, 
                            const cuDoubleComplex *A, int lda, cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtbmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx );    
    cublasSetError(error);                                   
}   
/*------------------------------------------------------------------------*/
/* TPMV */                                                    
static __inline__ void CUBLASAPI cublasStpmv(char uplo, char trans, char diag, int n, const float *AP, float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStpmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   
}                                                                                         

static __inline__ void CUBLASAPI cublasDtpmv(char uplo, char trans, char diag, int n, const double *AP, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtpmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   

}                                         

static __inline__ void CUBLASAPI cublasCtpmv(char uplo, char trans, char diag, int n, const cuComplex *AP, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtpmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   

}                                         
                                         
static __inline__ void CUBLASAPI cublasZtpmv(char uplo, char trans, char diag, int n, const cuDoubleComplex *AP, cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtpmv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   

}                                         
/*------------------------------------------------------------------------*/
/* TRSV */
static __inline__ void CUBLASAPI cublasStrsv(char uplo, char trans, char diag, int n, const float *A, int lda, float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStrsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );    
    cublasSetError(error);                                   
}                                                

static __inline__ void CUBLASAPI cublasDtrsv(char uplo, char trans, char diag, int n, const double *A, int lda, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtrsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );   
    cublasSetError(error);                                   
}                                   

static __inline__ void CUBLASAPI cublasCtrsv(char uplo, char trans, char diag, int n, const cuComplex *A, int lda, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtrsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );   
    cublasSetError(error);                                   
}                                   

static __inline__ void CUBLASAPI cublasZtrsv(char uplo, char trans, char diag, int n, const cuDoubleComplex *A, int lda, 
                                         cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtrsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, A, lda, x, incx );   
    cublasSetError(error);                                   
}                                                                            
/*------------------------------------------------------------------------*/
/* TPSV */
static __inline__ void CUBLASAPI cublasStpsv(char uplo, char trans, char diag, int n, const float *AP, 
                          float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStpsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   
}                          
                                                                                                            
static __inline__ void CUBLASAPI cublasDtpsv(char uplo, char trans, char diag, int n, const double *AP, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtpsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   
}

static __inline__ void CUBLASAPI cublasCtpsv(char uplo, char trans, char diag, int n, const cuComplex *AP, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtpsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   
}

static __inline__ void CUBLASAPI cublasZtpsv(char uplo, char trans, char diag, int n, const cuDoubleComplex *AP, 
                                         cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtpsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, AP, x, incx );  
    cublasSetError(error);                                   
}                                         
/*------------------------------------------------------------------------*/                                         
/* TBSV */                                         
static __inline__ void CUBLASAPI cublasStbsv(char uplo, char trans, 
                                         char diag, int n, int k, const float *A, 
                                         int lda, float *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStbsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                   
}                                         

static __inline__ void CUBLASAPI cublasDtbsv(char uplo, char trans, 
                                         char diag, int n, int k, const double *A, 
                                         int lda, double *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtbsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                   
}                                         
static __inline__ void CUBLASAPI cublasCtbsv(char uplo, char trans, 
                                         char diag, int n, int k, const cuComplex *A, 
                                         int lda, cuComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtbsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                   
}                                                                            
                                         
static __inline__ void CUBLASAPI cublasZtbsv(char uplo, char trans, 
                                         char diag, int n, int k, const cuDoubleComplex *A, 
                                         int lda, cuDoubleComplex *x, int incx)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtbsv_v2(handle, convertToFillMode(uplo), convertToOp(trans), 
                                   convertToDiagType(diag), n, k, A, lda, x, incx ); 
    cublasSetError(error);                                   
}                                                                                
/*------------------------------------------------------------------------*/                                         
/* SYMV/HEMV */
static __inline__ void CUBLASAPI cublasSsymv (char uplo, int n, float alpha, const float *A,
                            int lda, const float *x, int incx, float beta, 
                            float *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsymv_v2(handle, convertToFillMode(uplo),
                                   n, &alpha, A, lda, x, incx, &beta, y, incy ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasDsymv (char uplo, int n, double alpha, const double *A,
                            int lda, const double *x, int incx, double beta, 
                            double *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsymv_v2(handle, convertToFillMode(uplo),
                                   n, &alpha, A, lda, x, incx, &beta, y, incy ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasChemv (char uplo, int n, cuComplex alpha, const cuComplex *A,
                            int lda, const cuComplex *x, int incx, cuComplex beta, 
                            cuComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChemv_v2(handle, convertToFillMode(uplo),
                                   n, &alpha, A, lda, x, incx, &beta, y, incy ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasZhemv (char uplo, int n, cuDoubleComplex alpha, const cuDoubleComplex *A,
                            int lda, const cuDoubleComplex *x, int incx, cuDoubleComplex beta, 
                            cuDoubleComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhemv_v2(handle, convertToFillMode(uplo),
                                   n, &alpha, A, lda, x, incx, &beta, y, incy ); 
    cublasSetError(error);                                   
}                            
/*------------------------------------------------------------------------*/       
/* SBMV/HBMV */
static __inline__ void CUBLASAPI cublasSsbmv (char uplo, int n, int k, float alpha, 
                            const float *A, int lda, const float *x, int incx, 
                            float beta, float *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsbmv_v2(handle, convertToFillMode(uplo),n,k, &alpha,
                                    A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasDsbmv (char uplo, int n, int k, double alpha, 
                            const double *A, int lda, const double *x, int incx, 
                            double beta, double *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsbmv_v2(handle, convertToFillMode(uplo),n,k, &alpha,
                                    A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasChbmv (char uplo, int n, int k, cuComplex alpha, 
                            const cuComplex *A, int lda, const cuComplex *x, int incx, 
                            cuComplex beta, cuComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChbmv_v2(handle, convertToFillMode(uplo),n,k, &alpha,
                                    A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasZhbmv (char uplo, int n, int k, cuDoubleComplex alpha, 
                            const cuDoubleComplex *A, int lda, const cuDoubleComplex *x, int incx, 
                            cuDoubleComplex beta, cuDoubleComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhbmv_v2(handle, convertToFillMode(uplo),n,k, &alpha,
                                    A, lda, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                            
/*------------------------------------------------------------------------*/       
/* SPMV/HPMV */
static __inline__ void CUBLASAPI cublasSspmv(char uplo, int n, float alpha,
                                     const float *AP, const float *x,
                                     int incx, float beta, float *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSspmv_v2(handle, convertToFillMode(uplo),n, &alpha,
                                    AP, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                                     
static __inline__ void CUBLASAPI cublasDspmv(char uplo, int n, double alpha,
                                     const double *AP, const double *x,
                                     int incx, double beta, double *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDspmv_v2(handle, convertToFillMode(uplo),n, &alpha,
                                    AP, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                                     
static __inline__ void CUBLASAPI cublasChpmv(char uplo, int n, cuComplex alpha,
                                     const cuComplex *AP, const cuComplex *x,
                                     int incx, cuComplex beta, cuComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChpmv_v2(handle, convertToFillMode(uplo),n, &alpha,
                                    AP, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                                     
static __inline__ void CUBLASAPI cublasZhpmv(char uplo, int n, cuDoubleComplex alpha,
                                     const cuDoubleComplex *AP, const cuDoubleComplex *x,
                                     int incx, cuDoubleComplex beta, cuDoubleComplex *y, int incy)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhpmv_v2(handle, convertToFillMode(uplo),n, &alpha,
                                    AP, x, incx, &beta, y, incy);
    cublasSetError(error);                                    
}                                     

/*------------------------------------------------------------------------*/       
/* GER */
static __inline__ void CUBLASAPI cublasSger (int m, int n, float alpha, const float *x, int incx,
                           const float *y, int incy, float *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSger_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasDger (int m, int n, double alpha, const double *x, int incx,
                           const double *y, int incy, double *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDger_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}                           

static __inline__ void CUBLASAPI cublasCgeru (int m, int n, cuComplex alpha, const cuComplex *x,
                            int incx, const cuComplex *y, int incy,
                            cuComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCgeru_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}                            
static __inline__ void CUBLASAPI cublasCgerc (int m, int n, cuComplex alpha, const cuComplex *x,
                            int incx, const cuComplex *y, int incy,
                            cuComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCgerc_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}                            
static __inline__ void CUBLASAPI cublasZgeru (int m, int n, cuDoubleComplex alpha, const cuDoubleComplex *x,
                            int incx, const cuDoubleComplex *y, int incy,
                            cuDoubleComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZgeru_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}                            
static __inline__ void CUBLASAPI cublasZgerc (int m, int n, cuDoubleComplex alpha, const cuDoubleComplex *x,
                            int incx, const cuDoubleComplex *y, int incy,
                            cuDoubleComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();                            
    cublasStatus_t error = cublasZgerc_v2(handle, m, n, &alpha,
                                    x, incx, y, incy, A, lda);
    cublasSetError(error);                                    
}
/*------------------------------------------------------------------------*/       
/* SYR/HER */
static __inline__ void CUBLASAPI cublasSsyr (char uplo, int n, float alpha, const float *x,
                           int incx, float *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsyr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, A, lda);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasDsyr (char uplo, int n, double alpha, const double *x,
                           int incx, double *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsyr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, A, lda);
    cublasSetError(error);                                    
} 

static __inline__ void CUBLASAPI cublasCher (char uplo, int n, float alpha, 
                           const cuComplex *x, int incx, cuComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCher_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, A, lda);
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasZher (char uplo, int n, double alpha, 
                           const cuDoubleComplex *x, int incx, cuDoubleComplex *A, int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZher_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, A, lda);
    cublasSetError(error);                                    
}                            

/*------------------------------------------------------------------------*/       
/* SPR/HPR */
static __inline__ void CUBLASAPI cublasSspr (char uplo, int n, float alpha, const float *x,
                           int incx, float *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSspr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, AP);    
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasDspr (char uplo, int n, double alpha, const double *x,
                           int incx, double *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDspr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, AP);    
    cublasSetError(error);                                    
}                           
static __inline__ void CUBLASAPI cublasChpr (char uplo, int n, float alpha, const cuComplex *x,
                           int incx, cuComplex *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChpr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, AP);    
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasZhpr (char uplo, int n, double alpha, const cuDoubleComplex *x,
                           int incx, cuDoubleComplex *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhpr_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, AP);    
    cublasSetError(error);                                    
}      
/*------------------------------------------------------------------------*/       
/* SYR2/HER2 */
static __inline__ void CUBLASAPI cublasSsyr2 (char uplo, int n, float alpha, const float *x, 
                            int incx, const float *y, int incy, float *A, 
                            int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsyr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, A, lda);    
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasDsyr2 (char uplo, int n, double alpha, const double *x, 
                            int incx, const double *y, int incy, double *A, 
                            int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsyr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, A, lda);    
    cublasSetError(error);                                    
}                            
static __inline__ void CUBLASAPI cublasCher2 (char uplo, int n, cuComplex alpha, const cuComplex *x, 
                            int incx, const cuComplex *y, int incy, cuComplex *A, 
                            int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCher2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, A, lda);    
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasZher2 (char uplo, int n, cuDoubleComplex alpha, const cuDoubleComplex *x, 
                            int incx, const cuDoubleComplex *y, int incy, cuDoubleComplex *A, 
                            int lda)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZher2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, A, lda);    
    cublasSetError(error);                                    
}   

/*------------------------------------------------------------------------*/       
/* SPR2/HPR2 */
static __inline__ void CUBLASAPI cublasSspr2 (char uplo, int n, float alpha, const float *x, 
                            int incx, const float *y, int incy, float *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSspr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, AP);   
    cublasSetError(error);                                    
}                            
static __inline__ void CUBLASAPI cublasDspr2 (char uplo, int n, double alpha,
                            const double *x, int incx, const double *y,
                            int incy, double *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDspr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, AP);   
    cublasSetError(error);                                    
} 
static __inline__ void CUBLASAPI cublasChpr2 (char uplo, int n, cuComplex alpha,
                            const cuComplex *x, int incx, const cuComplex *y,
                            int incy, cuComplex *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChpr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, AP);   
    cublasSetError(error);                                    
}       
static __inline__ void CUBLASAPI cublasZhpr2 (char uplo, int n, cuDoubleComplex alpha,
                            const cuDoubleComplex *x, int incx, const cuDoubleComplex *y,
                            int incy, cuDoubleComplex *AP)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhpr2_v2(handle, convertToFillMode(uplo), n, &alpha,
                                    x, incx, y, incy, AP);   
    cublasSetError(error);                                    

}                                                 
/* ------------------------BLAS3 Functions ------------------------------- */
/* GEMM */
static __inline__ void CUBLASAPI cublasSgemm (char transa, char transb, int m, int n, int k, 
                            float alpha, const float *A, int lda, 
                            const float *B, int ldb, float beta, float *C, 
                            int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();

   cublasStatus_t error = cublasSgemm_v2(handle, convertToOp(transa), convertToOp(transb),
                                    m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    
}   
static __inline__ void CUBLASAPI cublasDgemm (char transa, char transb, int m, int n, int k,
                            double alpha, const double *A, int lda, 
                            const double *B, int ldb, double beta, double *C, 
                            int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDgemm_v2(handle, convertToOp(transa), convertToOp(transb),
                                    m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);
}                                                  
static __inline__ void CUBLASAPI cublasCgemm (char transa, char transb, int m, int n, int k, 
                            cuComplex alpha, const cuComplex *A, int lda,
                            const cuComplex *B, int ldb, cuComplex beta,
                            cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCgemm_v2(handle, convertToOp(transa), convertToOp(transb),
                                    m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

} 
static __inline__ void CUBLASAPI cublasZgemm (char transa, char transb, int m, int n,
                            int k, cuDoubleComplex alpha,
                            const cuDoubleComplex *A, int lda,
                            const cuDoubleComplex *B, int ldb,
                            cuDoubleComplex beta, cuDoubleComplex *C,
                            int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZgemm_v2(handle, convertToOp(transa), convertToOp(transb),
                                    m, n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}                                                      
/* -------------------------------------------------------*/
/* SYRK */
static __inline__ void CUBLASAPI cublasSsyrk (char uplo, char trans, int n, int k, float alpha, 
                            const float *A, int lda, float beta, float *C, 
                            int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsyrk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    
}  
static __inline__ void CUBLASAPI cublasDsyrk (char uplo, char trans, int n, int k,
                            double alpha, const double *A, int lda,
                            double beta, double *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsyrk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    
}                            

static __inline__ void CUBLASAPI cublasCsyrk (char uplo, char trans, int n, int k,
                            cuComplex alpha, const cuComplex *A, int lda,
                            cuComplex beta, cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCsyrk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    
}
static __inline__ void CUBLASAPI cublasZsyrk (char uplo, char trans, int n, int k,
                            cuDoubleComplex alpha,
                            const cuDoubleComplex *A, int lda,
                            cuDoubleComplex beta,
                            cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZsyrk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    
}                            
/* ------------------------------------------------------- */
/* HERK */
static __inline__ void CUBLASAPI cublasCherk (char uplo, char trans, int n, int k,
                            float alpha, const cuComplex *A, int lda,
                            float beta, cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCherk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}   
static __inline__ void CUBLASAPI cublasZherk (char uplo, char trans, int n, int k,
                            double alpha,
                            const cuDoubleComplex *A, int lda,
                            double beta,
                            cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZherk_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}                                                     
/* ------------------------------------------------------- */
/* SYR2K */
static __inline__ void CUBLASAPI cublasSsyr2k (char uplo, char trans, int n, int k, float alpha, 
                             const float *A, int lda, const float *B, int ldb, 
                             float beta, float *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsyr2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}
      
static __inline__ void CUBLASAPI cublasDsyr2k (char uplo, char trans, int n, int k,
                             double alpha, const double *A, int lda,
                             const double *B, int ldb, double beta,
                             double *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsyr2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}                                   
static __inline__ void CUBLASAPI cublasCsyr2k (char uplo, char trans, int n, int k,
                             cuComplex alpha, const cuComplex *A, int lda,
                             const cuComplex *B, int ldb, cuComplex beta,
                             cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCsyr2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}   

static __inline__ void CUBLASAPI cublasZsyr2k (char uplo, char trans, int n, int k,
                             cuDoubleComplex alpha, const cuDoubleComplex *A, int lda,
                             const cuDoubleComplex *B, int ldb, cuDoubleComplex beta,
                             cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZsyr2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}                                                                                    
/* ------------------------------------------------------- */
/* HER2K */
static __inline__ void CUBLASAPI cublasCher2k (char uplo, char trans, int n, int k,
                             cuComplex alpha, const cuComplex *A, int lda,
                             const cuComplex *B, int ldb, float beta,
                             cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCher2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);                                 
    cublasSetError(error);                                    

}

static __inline__ void CUBLASAPI cublasZher2k (char uplo, char trans, int n, int k,
                             cuDoubleComplex alpha, const cuDoubleComplex *A, int lda,
                             const cuDoubleComplex *B, int ldb, double beta,
                             cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZher2k_v2(handle, convertToFillMode(uplo), convertToOp(trans),
                                    n, k, &alpha, A, lda, B, ldb, &beta, C, ldc);  
    cublasSetError(error);                                                                       
}                                                          

/*------------------------------------------------------------------------*/       
/* SYMM*/
static __inline__ void CUBLASAPI cublasSsymm (char side, char uplo, int m, int n, float alpha, 
                            const float *A, int lda, const float *B, int ldb,
                            float beta, float *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasSsymm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasDsymm (char side, char uplo, int m, int n, double alpha, 
                            const double *A, int lda, const double *B, int ldb,
                            double beta, double *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDsymm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasCsymm (char side, char uplo, int m, int n, cuComplex alpha, 
                            const cuComplex *A, int lda, const cuComplex *B, int ldb,
                            cuComplex beta, cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCsymm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   
}   
static __inline__ void CUBLASAPI cublasZsymm (char side, char uplo, int m, int n, cuDoubleComplex alpha, 
                            const cuDoubleComplex *A, int lda, const cuDoubleComplex *B, int ldb,
                            cuDoubleComplex beta, cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZsymm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   
}                            
/*------------------------------------------------------------------------*/       
/* HEMM*/
static __inline__ void CUBLASAPI cublasChemm (char side, char uplo, int m, int n,
                            cuComplex alpha, const cuComplex *A, int lda,
                            const cuComplex *B, int ldb, cuComplex beta,
                            cuComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasChemm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   

}
static __inline__ void CUBLASAPI cublasZhemm (char side, char uplo, int m, int n,
                            cuDoubleComplex alpha, const cuDoubleComplex *A, int lda,
                            const cuDoubleComplex *B, int ldb, cuDoubleComplex beta,
                            cuDoubleComplex *C, int ldc)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZhemm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   m, n, &alpha, A, lda, B, ldb, &beta, C, ldc ); 
    cublasSetError(error);                                   
}                                                        

/*------------------------------------------------------------------------*/       
/* TRSM*/
static __inline__ void CUBLASAPI cublasStrsm (char side, char uplo, char transa, char diag,
                            int m, int n, float alpha, const float *A, int lda,
                            float *B, int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStrsm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb ); 
    cublasSetError(error);                                   

}

static __inline__ void CUBLASAPI cublasDtrsm (char side, char uplo, char transa,
                            char diag, int m, int n, double alpha,
                            const double *A, int lda, double *B,
                            int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtrsm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb ); 
    cublasSetError(error);                                   

}

static __inline__ void CUBLASAPI cublasCtrsm (char side, char uplo, char transa, char diag,
                            int m, int n, cuComplex alpha, const cuComplex *A,
                            int lda, cuComplex *B, int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtrsm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb ); 
    cublasSetError(error);                                   

}

static __inline__ void CUBLASAPI cublasZtrsm (char side, char uplo, char transa,
                            char diag, int m, int n, cuDoubleComplex alpha,
                            const cuDoubleComplex *A, int lda,
                            cuDoubleComplex *B, int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtrsm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb ); 
    cublasSetError(error);                                   

}                                                                                                                
/*------------------------------------------------------------------------*/       
/* TRMM*/
static __inline__ void CUBLASAPI cublasStrmm (char side, char uplo, char transa, char diag,
                            int m, int n, float alpha, const float *A, int lda,
                            float *B, int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasStrmm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb, B, ldb); 
    cublasSetError(error);                                   
} 
static __inline__ void CUBLASAPI cublasDtrmm (char side, char uplo, char transa,
                            char diag, int m, int n, double alpha,
                            const double *A, int lda, double *B,
                            int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasDtrmm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb, B, ldb ); 
    cublasSetError(error);                                   
}
static __inline__ void CUBLASAPI cublasCtrmm (char side, char uplo, char transa, char diag,
                            int m, int n, cuComplex alpha, const cuComplex *A,
                            int lda, cuComplex *B, int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasCtrmm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb, B, ldb ); 
    cublasSetError(error);                                   

}
static __inline__ void CUBLASAPI cublasZtrmm (char side, char uplo, char transa,
                            char diag, int m, int n, cuDoubleComplex alpha,
                            const cuDoubleComplex *A, int lda, cuDoubleComplex *B,
                            int ldb)
{
	cublasHandle_t handle = cublasGetCurrentCtx();
    cublasStatus_t error = cublasZtrmm_v2(handle, convertToSideMode(side),convertToFillMode(uplo),
                                   convertToOp(transa), convertToDiagType(diag), m, n, &alpha, A, lda, B, ldb, B, ldb ); 
    cublasSetError(error);                                   

}                                                                                                               

                                                                                                                                                                                                 
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* !defined(CUBLAS_H_) */
