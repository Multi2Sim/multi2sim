/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <stdio.h>

#include <lib/util/hash-table.h>
#include <lib/mhandle/mhandle.h>

#include "built-in-funcs.h"
#include "type.h"
#include "val.h"
#include "arg.h"
#include "function.h"
#include "cl2llvm.h"
#include "format.h"

#define BUILT_IN_FUNC_COUNT 42

extern LLVMModuleRef cl2llvm_module;
extern struct hash_table_t *cl2llvm_symbol_table;
extern struct hash_table_t *cl2llvm_built_in_func_table;
extern struct hash_table_t *cl2llvm_declared_built_in_funcs_table;




struct hash_table_t *built_in_func_table_create(void)
{
	int i;
	struct cl2llvm_built_in_func_info_t *built_in_func_info;
	
	char built_in_func_info_list[BUILT_IN_FUNC_COUNT][3][1400] = {
		{"get_work_dim", "1", "void u32 get_work_dim"},
		{"get_global_id", "1", "u32 u32 get_global_id"},
		{"get_global_size", "1", "u32 u32 get_global_size"},
		{"get_local_size", "1", "u32 u32 get_local_size"},
		{"get_local_id", "1", "u32 u32 get_local_id"},
		{"get_num_groups", "1", "u32 u32 get_num_groups"},
		{"get_group_id", "1", "u32 u32 get_group_id"},
		{"get_global_offset", "1", "u32 u32 get_global_offset"},
		{"barrier", "1", "u32 void barrier"},
		{"acos", "1", "f32 f32 __acos_f32 f32v2 f32v2 __acos_2f32 f32v3 f32v3 __acos_3f32 f32v4 f32v4 __acos_4f32 f32v8 f32v8 __acos_8f32 f32v16 f32v16 __acos_16f32 f64 f64 __acos_f64 f64v2 f64v2 __acos_2f64 f64v3 f64v3 __acos_3f64 f64v4 f64v4 __acos_4f64 f64v8 f64v8 __acos_8f64 f64v16 f64v16 __acos_16f64"},
		{"acosh", "1", "f32 f32 __acosh_f32 f32v2 f32v2 __acosh_2f32 f32v3 f32v3 __acosh_3f32 f32v4 f32v4 __acosh_4f32 f32v8 f32v8 __acosh_8f32 f32v16 f32v16 __acosh_16f32 f64 f64 __acosh_f64 f64v2 f64v2 __acosh_2f64 f64v3 f64v3 __acosh_3f64 f64v4 f64v4 __acosh_4f64 f64v8 f64v8 __acosh_8f64 f64v16 f64v16 __acosh_16f64"},
		{"acospi", "1", "f32 f32 __acospi_f32 f32v2 f32v2 __acospi_2f32 f32v3 f32v3 __acospi_3f32 f32v4 f32v4 __acospi_4f32 f32v8 f32v8 __acospi_8f32 f32v16 f32v16 __acospi_16f32 f64 f64 __acospi_f64 f64v2 f64v2 __acospi_2f64 f64v3 f64v3 __acospi_3f64 f64v4 f64v4 __acospi_4f64 f64v8 f64v8 __acospi_8f64 f64v16 f64v16 __acospi_16f64"},
		{"asin", "1", "f32 f32 __asin_f32 f32v2 f32v2 __asin_2f32 f32v3 f32v3 __asin_3f32 f32v4 f32v4 __asin_4f32 f32v8 f32v8 __asin_8f32 f32v16 f32v16 __asin_16f32 f64 f64 __asin_f64 f64v2 f64v2 __asin_2f64 f64v3 f64v3 __asin_3f64 f64v4 f64v4 __asin_4f64 f64v8 f64v8 __asin_8f64 f64v16 f64v16 __asin_16f64"},
		{"asinh", "1", "f32 f32 __asinh_f32 f32v2 f32v2 __asinh_2f32 f32v3 f32v3 __asinh_3f32 f32v4 f32v4 __asinh_4f32 f32v8 f32v8 __asinh_8f32 f32v16 f32v16 __asinh_16f32 f64 f64 __asinh_f64 f64v2 f64v2 __asinh_2f64 f64v3 f64v3 __asinh_3f64 f64v4 f64v4 __asinh_4f64 f64v8 f64v8 __asinh_8f64 f64v16 f64v16 __asinh_16f64"},
		{"asinpi", "1", "f32 f32 __asinpi_f32 f32v2 f32v2 __asinpi_2f32 f32v3 f32v3 __asinpi_3f32 f32v4 f32v4 __asinpi_4f32 f32v8 f32v8 __asinpi_8f32 f32v16 f32v16 __asinpi_16f32 f64 f64 __asinpi_f64 f64v2 f64v2 __asinpi_2f64 f64v3 f64v3 __asinpi_3f64 f64v4 f64v4 __asinpi_4f64 f64v8 f64v8 __asinpi_8f64 f64v16 f64v16 __asinpi_16f64"},
		{"atan", "1", "f32 f32 __atan_f32 f32v2 f32v2 __atan_2f32 f32v3 f32v3 __atan_3f32 f32v4 f32v4 __atan_4f32 f32v8 f32v8 __atan_8f32 f32v16 f32v16 __atan_16f32 f64 f64 __atan_f64 f64v2 f64v2 __atan_2f64 f64v3 f64v3 __atan_3f64 f64v4 f64v4 __atan_4f64 f64v8 f64v8 __atan_8f64 f64v16 f64v16 __atan_16f64"},
		{"atan2", "2", "f32 f32 f32 __atan2_f32 f32v2 f32v2 f32v2 __atan2_2f32 f32v3 f32v3 f32v3 __atan2_3f32 f32v4 f32v4 f32v4 __atan2_4f32 f32v8 f32v8 f32v8 __atan2_8f32 f32v16 f32v16 f32v16 __atan2_16f32 f64 f64 f64 __atan2_f64 f64v2 f64v2 f62v2 __atan2_2f64 f64v3 f64v3 f64v3 __atan2_3f64 f64v4 f64v4 f64v4 __atan2_4f64 f64v8 f64v8 f64v8 __atan2_8f64 f64v16 f64v16 f64v16 __atan2_16f64"},
		{"atanh", "1", "f32 f32 __atanh_f32 f32v2 f32v2 __atanh_2f32 f32v3 f32v3 __atanh_3f32 f32v4 f32v4 __atanh_4f32 f32v8 f32v8 __atanh_8f32 f32v16 f32v16 __atanh_16f32 f64 f64 __atanh_f64 f64v2 f64v2 __atanh_2f64 f64v3 f64v3 __atanh_3f64 f64v4 f64v4 __atanh_4f64 f64v8 f64v8 __atanh_8f64 f64v16 f64v16 __atanh_16f64"},
		{"atanpi", "1", "f32 f32 __atanpi_f32 f32v2 f32v2 __atanpi_2f32 f32v3 f32v3 __atanpi_3f32 f32v4 f32v4 __atanpi_4f32 f32v8 f32v8 __atanpi_8f32 f32v16 f32v16 __atanpi_16f32 f64 f64 __atanpi_f64 f64v2 f64v2 __atanpi_2f64 f64v3 f64v3 __atanpi_3f64 f64v4 f64v4 __atanpi_4f64 f64v8 f64v8 __atanpi_8f64 f64v16 f64v16 __atanpi_16f64"},
		{"atan2pi", "2", "f32 f32 f32 __atan2pi_f32 f32v2 f32v2 f32v2 __atan2pi_2f32 f32v3 f32v3 f32v3 __atan2pi_3f32 f32v4 f32v4 f32v4 __atan2pi_4f32 f32v8 f32v8 f32v8 __atan2pi_8f32 f32v16 f32v16 f32v16 __atan2pi_16f32 f64 f64 f64 __atan2pi_f64 f64v2 f64v2 f62v2 __atan2pi_2f64 f64v3 f64v3 f64v3 __atan2pi_3f64 f64v4 f64v4 f64v4 __atan2pi_4f64 f64v8 f64v8 f64v8 __atan2pi_8f64 f64v16 f64v16 f64v16 __atan2pi_16f64"},
		{"chrt", "1", "f32 f32 __chrt_f32 f32v2 f32v2 __chrt_2f32 f32v3 f32v3 __chrt_3f32 f32v4 f32v4 __chrt_4f32 f32v8 f32v8 __chrt_8f32 f32v16 f32v16 __chrt_16f32 f64 f64 __chrt_f64 f64v2 f64v2 __chrt_2f64 f64v3 f64v3 __chrt_3f64 f64v4 f64v4 __chrt_4f64 f64v8 f64v8 __chrt_8f64 f64v16 f64v16 __chrt_16f64"},
		{"ceil", "1", "f32 f32 __ceil_f32 f32v2 f32v2 __ceil_2f32 f32v3 f32v3 __ceil_3f32 f32v4 f32v4 __ceil_4f32 f32v8 f32v8 __ceil_8f32 f32v16 f32v16 __ceil_16f32 f64 f64 __ceil_f64 f64v2 f64v2 __ceil_2f64 f64v3 f64v3 __ceil_3f64 f64v4 f64v4 __ceil_4f64 f64v8 f64v8 __ceil_8f64 f64v16 f64v16 __ceil_16f64"},
		{"ceil", "1", "f32 f32 __ceil_f32 f32v2 f32v2 __ceil_2f32 f32v3 f32v3 __ceil_3f32 f32v4 f32v4 __ceil_4f32 f32v8 f32v8 __ceil_8f32 f32v16 f32v16 __ceil_16f32 f64 f64 __ceil_f64 f64v2 f64v2 __ceil_2f64 f64v3 f64v3 __ceil_3f64 f64v4 f64v4 __ceil_4f64 f64v8 f64v8 __ceil_8f64 f64v16 f64v16 __ceil_16f64"},
		{"ceil", "1", "f32 f32 __ceil_f32 f32v2 f32v2 __ceil_2f32 f32v3 f32v3 __ceil_3f32 f32v4 f32v4 __ceil_4f32 f32v8 f32v8 __ceil_8f32 f32v16 f32v16 __ceil_16f32 f64 f64 __ceil_f64 f64v2 f64v2 __ceil_2f64 f64v3 f64v3 __ceil_3f64 f64v4 f64v4 __ceil_4f64 f64v8 f64v8 __ceil_8f64 f64v16 f64v16 __ceil_16f64"},
		{"copysign", "2", "f32 f32 f32 __copysign_f32 f32v2 f32v2 f32v2 __copysign_2f32 f32v3 f32v3 f32v3 __copysign_3f32 f32v4 f32v4 f32v4 __copysign_4f32 f32v8 f32v8 f32v8 __copysign_8f32 f32v16 f32v16 f32v16 __copysign_16f32 f64 f64 f64 __copysign_f64 f64v2 f64v2 f62v2 __copysign_2f64 f64v3 f64v3 f64v3 __copysign_3f64 f64v4 f64v4 f64v4 __copysign_4f64 f64v8 f64v8 f64v8 __copysign_8f64 f64v16 f64v16 f64v16 __copysign_16f64"},
		{"cos", "1", "f32 f32 __cos_f32 f32v2 f32v2 __cos_2f32 f32v3 f32v3 __cos_3f32 f32v4 f32v4 __cos_4f32 f32v8 f32v8 __cos_8f32 f32v16 f32v16 __cos_16f32 f64 f64 __cos_f64 f64v2 f64v2 __cos_2f64 f64v3 f64v3 __cos_3f64 f64v4 f64v4 __cos_4f64 f64v8 f64v8 __cos_8f64 f64v16 f64v16 __cos_16f64"},
		{"cosh", "1", "f32 f32 __cosh_f32 f32v2 f32v2 __cosh_2f32 f32v3 f32v3 __cosh_3f32 f32v4 f32v4 __cosh_4f32 f32v8 f32v8 __cosh_8f32 f32v16 f32v16 __cosh_16f32 f64 f64 __cosh_f64 f64v2 f64v2 __cosh_2f64 f64v3 f64v3 __cosh_3f64 f64v4 f64v4 __cosh_4f64 f64v8 f64v8 __cosh_8f64 f64v16 f64v16 __cosh_16f64"},
		{"cospi", "1", "f32 f32 __cospi_f32 f32v2 f32v2 __cospi_2f32 f32v3 f32v3 __cospi_3f32 f32v4 f32v4 __cospi_4f32 f32v8 f32v8 __cospi_8f32 f32v16 f32v16 __cospi_16f32 f64 f64 __cospi_f64 f64v2 f64v2 __cospi_2f64 f64v3 f64v3 __cospi_3f64 f64v4 f64v4 __cospi_4f64 f64v8 f64v8 __cospi_8f64 f64v16 f64v16 __cospi_16f64"},
		{"erfc", "1", "f32 f32 __erfc_f32 f32v2 f32v2 __erfc_2f32 f32v3 f32v3 __erfc_3f32 f32v4 f32v4 __erfc_4f32 f32v8 f32v8 __erfc_8f32 f32v16 f32v16 __erfc_16f32 f64 f64 __erfc_f64 f64v2 f64v2 __erfc_2f64 f64v3 f64v3 __erfc_3f64 f64v4 f64v4 __erfc_4f64 f64v8 f64v8 __erfc_8f64 f64v16 f64v16 __erfc_16f64"},
		{"erf", "1", "f32 f32 __erf_f32 f32v2 f32v2 __erf_2f32 f32v3 f32v3 __erf_3f32 f32v4 f32v4 __erf_4f32 f32v8 f32v8 __erf_8f32 f32v16 f32v16 __erf_16f32 f64 f64 __erf_f64 f64v2 f64v2 __erf_2f64 f64v3 f64v3 __erf_3f64 f64v4 f64v4 __erf_4f64 f64v8 f64v8 __erf_8f64 f64v16 f64v16 __erf_16f64"},
		{"exp", "1", "f32 f32 __exp_f32 f32v2 f32v2 __exp_2f32 f32v3 f32v3 __exp_3f32 f32v4 f32v4 __exp_4f32 f32v8 f32v8 __exp_8f32 f32v16 f32v16 __exp_16f32 f64 f64 __exp_f64 f64v2 f64v2 __exp_2f64 f64v3 f64v3 __exp_3f64 f64v4 f64v4 __exp_4f64 f64v8 f64v8 __exp_8f64 f64v16 f64v16 __exp_16f64"},
		{"exp2", "1", "f32 f32 __exp2_f32 f32v2 f32v2 __exp2_2f32 f32v3 f32v3 __exp2_3f32 f32v4 f32v4 __exp2_4f32 f32v8 f32v8 __exp2_8f32 f32v16 f32v16 __exp2_16f32 f64 f64 __exp2_f64 f64v2 f64v2 __exp2_2f64 f64v3 f64v3 __exp2_3f64 f64v4 f64v4 __exp2_4f64 f64v8 f64v8 __exp2_8f64 f64v16 f64v16 __exp2_16f64"},
		{"exp10", "1", "f32 f32 __exp10_f32 f32v2 f32v2 __exp10_2f32 f32v3 f32v3 __exp10_3f32 f32v4 f32v4 __exp10_4f32 f32v8 f32v8 __exp10_8f32 f32v16 f32v16 __exp10_16f32 f64 f64 __exp10_f64 f64v2 f64v2 __exp10_2f64 f64v3 f64v3 __exp10_3f64 f64v4 f64v4 __exp10_4f64 f64v8 f64v8 __exp10_8f64 f64v16 f64v16 __exp10_16f64"},
		{"expm1", "1", "f32 f32 __expm1_f32 f32v2 f32v2 __expm1_2f32 f32v3 f32v3 __expm1_3f32 f32v4 f32v4 __expm1_4f32 f32v8 f32v8 __expm1_8f32 f32v16 f32v16 __expm1_16f32 f64 f64 __expm1_f64 f64v2 f64v2 __expm1_2f64 f64v3 f64v3 __expm1_3f64 f64v4 f64v4 __expm1_4f64 f64v8 f64v8 __expm1_8f64 f64v16 f64v16 __expm1_16f64"},
		{"fabs", "1", "f32 f32 __fabs_f32 f32v2 f32v2 __fabs_2f32 f32v3 f32v3 __fabs_3f32 f32v4 f32v4 __fabs_4f32 f32v8 f32v8 __fabs_8f32 f32v16 f32v16 __fabs_16f32 f64 f64 __fabs_f64 f64v2 f64v2 __fabs_2f64 f64v3 f64v3 __fabs_3f64 f64v4 f64v4 __fabs_4f64 f64v8 f64v8 __fabs_8f64 f64v16 f64v16 __fabs_16f64"},
		{"fdim", "2", "f32 f32 f32 __fdim_f32 f32v2 f32v2 f32v2 __fdim_2f32 f32v3 f32v3 f32v3 __fdim_3f32 f32v4 f32v4 f32v4 __fdim_4f32 f32v8 f32v8 f32v8 __fdim_8f32 f32v16 f32v16 f32v16 __fdim_16f32 f64 f64 f64 __fdim_f64 f64v2 f64v2 f62v2 __fdim_2f64 f64v3 f64v3 f64v3 __fdim_3f64 f64v4 f64v4 f64v4 __fdim_4f64 f64v8 f64v8 f64v8 __fdim_8f64 f64v16 f64v16 f64v16 __fdim_16f64"},
		{"floor", "1", "f32 f32 __floor_f32 f32v2 f32v2 __floor_2f32 f32v3 f32v3 __floor_3f32 f32v4 f32v4 __floor_4f32 f32v8 f32v8 __floor_8f32 f32v16 f32v16 __floor_16f32 f64 f64 __floor_f64 f64v2 f64v2 __floor_2f64 f64v3 f64v3 __floor_3f64 f64v4 f64v4 __floor_4f64 f64v8 f64v8 __floor_8f64 f64v16 f64v16 __floor_16f64"},
		{"fma", "2", "f32 f32 f32 f32 __fma_f32 f32v2 f32v2 f32v2 f32v2 __fma_2f32 f32v3 f32v3 f32v3 f32v3 __fma_3f32 f32v4 f32v4 f32v4 f32v4 __fma_4f32 f32v8 f32v8 f32v8 f32v8 __fma_8f32 f32v16 f32v16 f32v16 f32v16 __fma_16f32 f64 f64 f64 f64 __fma_f64 f64v2 f64v2 f64v2 f64v2 __fma_2f64 f64v3 f64v3 f64v3 f64v3 __fma_3f64 f64v4 f64v4 f64v4 f64v4 __fma_4f64 f64v8 f64v8 f64v8 f64v8 __fma_8f64 f64v16 f64v16 f64v16 f64v16 __fma_16f64"},
		{"fmax", "2", "f32 f32 f32 __fmax_f32 f32v2 f32v2 f32v2 __fmax_2f32 f32v3 f32v3 f32v3 __fmax_3f32 f32v4 f32v4 f32v4 __fmax_4f32 f32v8 f32v8 f32v8 __fmax_8f32 f32v16 f32v16 f32v16 __fmax_16f32 f64 f64 f64 __fmax_f64 f64v2 f64v2 f62v2 __fmax_2f64 f64v3 f64v3 f64v3 __fmax_3f64 f64v4 f64v4 f64v4 __fmax_4f64 f64v8 f64v8 f64v8 __fmax_8f64 f64v16 f64v16 f64v16 __fmax_16f64"},
		{"fmin", "2", "f32 f32 f32 __fmin_f32 f32v2 f32v2 f32v2 __fmin_2f32 f32v3 f32v3 f32v3 __fmin_3f32 f32v4 f32v4 f32v4 __fmin_4f32 f32v8 f32v8 f32v8 __fmin_8f32 f32v16 f32v16 f32v16 __fmin_16f32 f64 f64 f64 __fmin_f64 f64v2 f64v2 f62v2 __fmin_2f64 f64v3 f64v3 f64v3 __fmin_3f64 f64v4 f64v4 f64v4 __fmin_4f64 f64v8 f64v8 f64v8 __fmin_8f64 f64v16 f64v16 f64v16 __fmin_16f64 f32v2 f32 f32v2 __fmin_2f32 f32v3 f32 f32v3 __fmin_3f32 f32v4 f32 f32v4 __fmin_4f32 f32v8 f32 f32v8 __fmin_8f32 f32v16 f32 f32v16 __fmin_16f32 f64v2 f64 f62v2 __fmin_2f64 f64v3 f64 f64v3 __fmin_3f64 f64v4 f64 f64v4 __fmin_4f64 f64v8 f64 f64v8 __fmin_8f64 f64v16 f64 f64v16 __fmin_16f64"},
		{"fmod", "2", "f32 f32 f32 __fmod_f32 f32v2 f32v2 f32v2 __fmod_2f32 f32v3 f32v3 f32v3 __fmod_3f32 f32v4 f32v4 f32v4 __fmod_4f32 f32v8 f32v8 f32v8 __fmod_8f32 f32v16 f32v16 f32v16 __fmod_16f32 f64 f64 f64 __fmod_f64 f64v2 f64v2 f62v2 __fmod_2f64 f64v3 f64v3 f64v3 __fmod_3f64 f64v4 f64v4 f64v4 __fmod_4f64 f64v8 f64v8 f64v8 __fmod_8f64 f64v16 f64v16 f64v16 __fmod_16f64"},
		{"fract", "2", "f32 f32g* f32 __fract_f32 f32v2 f32v2g* f32v2 __fract_2f32 f32v3 f32v3g* f32v3 __fract_3f32 f32v4 f32v4g* f32v4 __fract_4f32 f32v8 f32v8g* f32v8 __fract_8f32 f32v16 f32v16g* f32v16 __fract_16f32 f64 f64g* f64 __fract_f64 f64v2 f64v2g* f64v2 __fract_2f64 f64v3 f64v3g* f64v3 __fract_3f64 f64v4 f64v4g* f64v4 __fract_4f64 f64v8 f64v8g* f64v8 __fract_8f64 f64v16 f64v16g* f64v16 __fract_16f64 f32 f32l* f32 __fract_f32 f32v2 f32v2l* f32v2 __fract_2f32 f32v3 f32v3l* f32v3 __fract_3f32 f32v4 f32v4l* f32v4 __fract_4f32 f32v8 f32v8l* f32v8 __fract_8f32 f32v16 f32v16l* f32v16 __fract_16f32 f64 f64l* f64 __fract_f64 f64v2 f64v2l* f64v2 __fract_2f64 f64v3 f64v3l* f64v3 __fract_3f64 f64v4 f64v4l* f64v4 __fract_4f64 f64v8 f64v8l* f64v8 __fract_8f64 f64v16 f64v16l* f64v16 __fract_16f64f32 f32* f32 __fract_f32 f32v2 f32v2* f32v2 __fract_2f32 f32v3 f32v3* f32v3 __fract_3f32 f32v4 f32v4* f32v4 __fract_4f32 f32v8 f32v8* f32v8 __fract_8f32 f32v16 f32v16* f32v16 __fract_16f32 f64 f64* f64 __fract_f64 f64v2 f64v2* f64v2 __fract_2f64 f64v3 f64v3* f64v3 __fract_3f64 f64v4 f64v4* f64v4 __fract_4f64 f64v8 f64v8* f64v8 __fract_8f64 f64v16 f64v16* f64v16 __fract_16f64"},
	};


	/* Create hash table */
	struct hash_table_t *built_in_func_table = hash_table_create(200, 1);
	
	/* Insert function names and id numbers into hash table. */
	for(i = 0; i < BUILT_IN_FUNC_COUNT; i++)
	{
		built_in_func_info = cl2llvm_built_in_func_info_create(i,
			atoi(built_in_func_info_list[i][1]), 
			built_in_func_info_list[i][2]);
		hash_table_insert(built_in_func_table, built_in_func_info_list[i][0],
			built_in_func_info);
	}
	return built_in_func_table;
}

void cl2llvm_built_in_func_table_free(struct hash_table_t *built_in_func_table)
{
	char* name;
	struct cl2llvm_built_in_func_info_t *built_in_func;

	HASH_TABLE_FOR_EACH(built_in_func_table, name, built_in_func)
	{
		free(built_in_func->arg_string);
		free(built_in_func);
	}
	hash_table_free(built_in_func_table);
}

struct cl2llvm_built_in_func_info_t *cl2llvm_built_in_func_info_create(
	int func_id, int arg_count, char* arg_string)
{
	struct cl2llvm_built_in_func_info_t *built_in_func_info;
	
	built_in_func_info = xcalloc(1, sizeof(struct cl2llvm_built_in_func_info_t));
	
	built_in_func_info->func_id = func_id;
	built_in_func_info->arg_count = arg_count;
	built_in_func_info->arg_string = xstrdup(arg_string);

	return built_in_func_info;
}

void cl2llvm_built_in_func_analyze(char* name, struct list_t *param_list)
{
	struct cl2llvm_built_in_func_info_t *built_in_func_info;
	int end_of_string;
	int match_found;
	int args_match;
	int i;
	int index1, index2;
	char error_message[1000];
	char* arg_string;
	char type_string[20];
	char param_spec_name[50];
	struct cl2llvm_type_t *type;
	struct cl2llvm_type_t *ret_type;
	struct cl2llvm_val_t *param;
	struct list_t *arg_types_list;

	built_in_func_info = hash_table_get(cl2llvm_built_in_func_table, name);
	arg_string = built_in_func_info->arg_string;

	end_of_string = 0;
	index1 = 0;
	match_found = 0;

	while(!end_of_string && !match_found)
	{
		arg_types_list = list_create();
		args_match = 1;
		for (i = 0; i < built_in_func_info->arg_count; i++)
		{
			param = list_get(param_list, i);
			index2 = 0;
			while(arg_string[index1] == ' ')
			{
				index1++;
			}
			while(arg_string[index1] != ' ')
			{
				type_string[index2] = arg_string[index1];
				index1++;
				index2++;
			}
			type_string[index2] = '\00';
			type = string_to_type(type_string);
			if (args_match)	
			{
				/* If type is void, Create special comparison */
				if (type->llvm_type == LLVMVoidType() && param == NULL)
					args_match = 1;
				/* Only compare signs if type is a vector */
				else if (LLVMGetTypeKind(param->type->llvm_type) ==
					LLVMVectorTypeKind)
				{
					if (type->llvm_type == 
						param->type->llvm_type && type->sign == 
						param->type->sign)
						{
						args_match = 1;
						}
					else
						args_match = 0;
				}
				else if (type->llvm_type == param->type->llvm_type)
					args_match = 1;
				else
					args_match = 0;
			}
			list_add(arg_types_list, type);
		}

		if (args_match)
		{
			index2 = 0;
			while (arg_string[index1] == ' ')
			{
				index1++;
			}
			while (arg_string[index1] != ' ')
			{
				type_string[index2] = arg_string[index1];
				index2++;
				index1++;
			}
			ret_type = string_to_type(type_string);
			
			index2 = 0;
			while (arg_string[index1] == ' ')
			{
				index1++;
			}
			while (arg_string[index1] != ' ' && arg_string[index1] != '\00')
			{
				param_spec_name[index2] = arg_string[index1];
				index2++;
				index1++;
			}
			/* NULL terminate param_spec_name */
			param_spec_name[index2] = '\00';
			match_found = 1;
			
			/* Declare function */
			func_declare(arg_types_list, ret_type, name, param_spec_name);

			cl2llvm_type_free(ret_type);
		}
		else
		{
			while (arg_string[index1] == ' ')
				index1++;
			while (arg_string[index1] != ' ')
				index1++;
			while (arg_string[index1] == ' ')
				index1++;
			while (arg_string[index1] != ' ' && arg_string[index1] != '\00')
				index1++;

		}
		/* Free types stored in list */
		LIST_FOR_EACH(arg_types_list, i)
		{
			type = list_get(arg_types_list, i);
			cl2llvm_type_free(type);
		}
		list_free(arg_types_list);

		/* Check for end of string */
		if (arg_string[index1] == '\00')
			end_of_string = 1;
	}
	if (!match_found)
	{
		cl2llvm_error_built_in_func_arg_mismatch(param_list, built_in_func_info,  
			name, error_message);
		cl2llvm_yyerror_fmt("%s", error_message);
	}
}

void func_declare(struct list_t *arg_types_list, struct cl2llvm_type_t *ret_type, 
	char* name, char* param_spec_name)
{
	LLVMTypeRef args_array[50];
	struct list_t *arg_list;
	struct cl2llvm_decl_list_t *arg_decl;
	struct cl2llvm_arg_t *arg;
	struct cl2llvm_function_t *function;
	struct cl2llvm_function_t *test_function;
	struct cl2llvm_type_t *current_arg_type;
	struct cl2llvm_type_t *type_spec;
	int i;
	
	function = hash_table_get(cl2llvm_declared_built_in_funcs_table, param_spec_name);
	
	/* If specific function has already been declared, exit */
	if(function)
	{
		/* Check function already exists under same name and
		   replace it */
		test_function = hash_table_get(cl2llvm_symbol_table, name);
		if (test_function)
			cl2llvm_function_free(test_function);

		/* Insert function in global symbol table */
		hash_table_insert(cl2llvm_symbol_table, name, 
			cl2llvm_func_cpy(function));
		return;
	}
	
	arg_list = list_create();
	
	for (i = 0; i < list_count(arg_types_list); i++)
	{
		/* Arguments */
		current_arg_type = list_get(arg_types_list, i);
		args_array[i] = current_arg_type->llvm_type;
		arg_decl = cl2llvm_decl_list_create();
		type_spec = cl2llvm_type_create_w_init(current_arg_type->llvm_type,
		current_arg_type->sign);
		arg_decl->type_spec = type_spec;
		arg = cl2llvm_arg_create(arg_decl, "arg");
		list_add(arg_list, arg);

		cl2llvm_decl_list_struct_free(arg_decl);
	}
	/* Arg list is empty */
	type_spec = list_get(arg_types_list, 0); 
	if (type_spec->llvm_type == LLVMVoidType())
	{
		cl2llvm_arg_free(list_get(arg_list, 0));
		list_set(arg_list, 0, NULL);
	}
	
	/* Function */
	function = cl2llvm_function_create(name, arg_list);
		
	function->func_type = LLVMFunctionType(ret_type->llvm_type, 
 		args_array, function->arg_count, 0);
	function->func = LLVMAddFunction(cl2llvm_module, 
		param_spec_name, function->func_type);
	function->sign = ret_type->sign;
	LLVMSetFunctionCallConv(function->func, LLVMCCallConv);
	LLVMAddFunctionAttr(function->func, 1 << 5);
	
	/* Insert function in global symbol table */
	hash_table_insert(cl2llvm_symbol_table, name, 
		function);
	
	/* Insert specific function into declared built-in 
	   functions table */
	hash_table_insert(cl2llvm_declared_built_in_funcs_table, 
		param_spec_name, cl2llvm_func_cpy(function));
}

struct cl2llvm_type_t *string_to_type(char* info_str)
{
	int addr_space;
	int is_int;
	int is_signed;
	int ptr_count;
	int i;
	int j;
	LLVMTypeRef type;
	struct cl2llvm_type_t *ret_type;

	i = 0;
	is_int = 0;
	addr_space = 0;

	/* Check if type is void */
	if (info_str[i] == 'v'
		&& info_str[++i] == 'o'
		&& info_str[++i] == 'i'
		&& info_str[++i] == 'd')
		type = LLVMVoidType();

	/* Check for scalar or element type */
	if (info_str[i] == 'i')
	{
		is_int = 1;
		is_signed = 1;
		i++;
	}
	else if (info_str[i] == 'f')
	{
		is_signed = 1;
		i++;
	}
	else if (info_str[i] == 'u')
	{
		is_int = 1;
		is_signed = 0;
		i++;
	}
	/* read bitwidth */
	if (info_str[i] == '1')
	{
		i++;
		if (info_str[i] == '6')
		{
			type = LLVMInt16Type();
			i++;
		}
		else
		{
			type = LLVMInt1Type();
			i++;
		}
	}
	else if (info_str[i] == '8')
	{
		type = LLVMInt8Type();
		i++;
	}
	else if (info_str[i] == '3')
	{
		i++;
		if (info_str[i] == '2')
		{
			i++;
			if (is_int)
				type = LLVMInt32Type();
			else
				type = LLVMFloatType();
		}
	}
	else if (info_str[i] == '6')
	{
		i++;
		if (info_str[i] == '4')
		{
			i++;
			if (is_int)
				type = LLVMInt64Type();
			else
				type = LLVMDoubleType();
		}
	}
	/* If type is vector, get size */
	if (info_str[i] == 'v' || info_str[i] == 'V')
	{
		i++;
		if (info_str[i] == '2')
		{
			type = LLVMVectorType(type, 2);
			i++;
		}
		else if (info_str[i] == '3')
		{
			type = LLVMVectorType(type, 3);
			i++;
		}
		else if (info_str[i] == '4')
		{
			type = LLVMVectorType(type, 4);
			i++;
		}
		else if (info_str[i] == '8')
		{
			type = LLVMVectorType(type, 8);
			i++;
		}
		else if (info_str[i] == '1' && info_str[++i] == '6')
		{
			type = LLVMVectorType(type, 16);
			i++;
		}
	}
	/* If type is a pointer */
	if (info_str[i] == 'l')
	{
		addr_space = 2;
		i++;
	}
	if (info_str[i] == 'g')
	{
		addr_space = 1;
		i++;
	}
	if (info_str[i] == 'p')
		i++;

	/* determine number of pointers */
	while (info_str[i] == '*')
		ptr_count++;

	/* Create pointer */
	if (info_str[i] == '*')
	{
		for (j = 0; j < ptr_count; j++)
		{
			if (j + 1 == ptr_count)
				type = LLVMPointerType(type, addr_space);
			else
				type = LLVMPointerType(type, 0);
		}
	}

	ret_type = cl2llvm_type_create_w_init(type, is_signed);
	
	return ret_type;
}

/* This function creates an error message for argument type mismatches based
   on and arg_info string and a list of the attempted argument types. */
char *cl2llvm_error_built_in_func_arg_mismatch(struct list_t *param_list,
	struct cl2llvm_built_in_func_info_t *func_info,  char *func_name, 
	char *error_message)
{
	int index1, index2;
	int i, j;
	int end_of_string;
	char format_arg_str[50];
	char error_message_cpy[1000];
	char arg_type_string[50];
	char* info_string;
	struct cl2llvm_val_t *param;

	index1 = 0;
	index1 = 0;
	j = 0;
	end_of_string = 0;
	info_string = func_info->arg_string;
	error_message[0] = '\00';

	while (!end_of_string)
	{
		strcpy(error_message_cpy, error_message);
		snprintf(error_message, 1000 * sizeof(char),
			"%s\t\t%s(", error_message_cpy, func_name);

		for (i = 0; i < func_info->arg_count; i++)
		{
			index2 = 0;
					while (info_string[index1] == ' ')
				index1++;
			while (info_string[index1] != ' ')
			{
				arg_type_string[index2] = info_string[index1];
				index1++;
				index2++;
			}
			arg_type_string[++index2] = '\00';

cl2llvm_type_to_string(string_to_type(arg_type_string), format_arg_str);

			strcpy(error_message_cpy, error_message);
			snprintf(error_message, 1000 * sizeof(char),
				"%s%s", error_message_cpy, format_arg_str);

			if (i == func_info->arg_count - 1)
			{
				strcpy(error_message_cpy, error_message);
				snprintf(error_message, 1000 * sizeof(char),
					"%s)\n", error_message_cpy);
			}
			else
			{
				strcpy(error_message_cpy, error_message);
				snprintf(error_message, 1000 * sizeof(char),
					"%s, ", error_message_cpy);
			}
		}
		j++;
		while (info_string[index1] == ' ')
			index1++;
		while (info_string[index1] != ' ')
			index1++;
		while (info_string[index1] == ' ')
			index1++;
		while (info_string[index1] != ' ' && info_string[index1] != '\00')
			index1++;
		if (info_string[index1] == '\00')
			end_of_string = 1;
	}
	if (j > 1)
	{
		strcpy(error_message_cpy, error_message);
		snprintf(error_message, 1000 * sizeof(char),
			"none of the following instances of overloaded\n"
			"function '%s' match the argument list\n%s"
			"arguments are:  "
			, func_name, error_message_cpy);
	}
	else
	{
		strcpy(error_message_cpy, error_message);
		snprintf(error_message, 1000 * sizeof(char),
			"invalid type of argument for function '%s'\n"
			"expected:%s"
			"you have: ", func_name, error_message_cpy);
	}

	strcpy(error_message_cpy, error_message);
	snprintf(error_message, 1000 * sizeof(char),
		"%s%s(", error_message_cpy, func_name);
	
	for (i = 0; i < func_info->arg_count; i++)
	{
		param = list_get(param_list, i);
		cl2llvm_type_to_string(param->type, format_arg_str);

		strcpy(error_message_cpy, error_message);
		snprintf(error_message, 1000 * sizeof(char),
			"%s%s", error_message_cpy, format_arg_str);


		if (i == func_info->arg_count - 1)
		{
			strcpy(error_message_cpy, error_message);
			snprintf(error_message, 1000 * sizeof(char),
				"%s)\n", error_message_cpy);
		}
		else
		{
			strcpy(error_message_cpy, error_message);
			snprintf(error_message, 1000 * sizeof(char),
				"%s, ", error_message_cpy);
		}
	}
	return error_message;
}
