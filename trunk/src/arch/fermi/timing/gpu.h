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

#ifndef FERMI_TIMING_GPU_H
#define FERMI_TIMING_GPU_H

#include <arch/common/timing.h>


/* Trace */
#define frm_tracing() trace_status(frm_trace_category)
#define frm_trace(...) trace(frm_trace_category, __VA_ARGS__)
#define frm_trace_header(...) trace_header(frm_trace_category, __VA_ARGS__)
extern int frm_trace_category;

extern char *frm_gpu_config_help;
extern char *frm_gpu_config_file_name;
extern char *frm_gpu_dump_default_config_file_name;
extern char *frm_gpu_report_file_name;
extern char *frm_gpu_calc_file_name;

/* OpenCL Device Info */
extern unsigned long long frm_gpu_device_type;
extern unsigned int frm_gpu_device_vendor_id;

extern char *frm_gpu_device_profile;
extern char *frm_gpu_device_name;
extern char *frm_gpu_device_vendor;
extern char *frm_gpu_device_extensions;
extern char *frm_gpu_device_version;
extern char *frm_gpu_driver_version;
extern char *frm_gpu_opencl_version;

extern unsigned int frm_gpu_thread_block_size;
extern unsigned int frm_gpu_thread_dimensions;
extern unsigned int frm_gpu_thread_sizes[3];

extern unsigned int frm_gpu_image_support;
extern unsigned int frm_gpu_max_read_image_args;
extern unsigned int frm_gpu_max_write_image_args;

extern unsigned int frm_gpu_image2d_max_width;
extern unsigned int frm_gpu_image2d_max_height;
extern unsigned int frm_gpu_image3d_max_width;
extern unsigned int frm_gpu_image3d_max_height;
extern unsigned int frm_gpu_image3d_max_depth;
extern unsigned int frm_gpu_max_samplers;

extern unsigned int frm_gpu_max_parameter_size;
extern unsigned int frm_gpu_mem_base_addr_align;
extern unsigned int frm_gpu_min_data_type_align_size;

extern unsigned int frm_gpu_single_fp_config;
extern unsigned int frm_gpu_double_fp_config;

extern unsigned int frm_gpu_max_clock_frequency;
extern unsigned int frm_gpu_address_bits;

extern unsigned int frm_gpu_global_mem_cache_type;
extern unsigned int frm_gpu_global_mem_cacheline_size;
extern unsigned long long frm_gpu_global_mem_cache_size;
extern unsigned long long frm_gpu_global_mem_size;
extern unsigned long long frm_gpu_max_mem_alloc_size;

extern unsigned int frm_gpu_local_mem_type;

extern unsigned long long frm_gpu_max_constant_buffer_size;
extern unsigned int frm_gpu_max_constant_args;

extern unsigned int frm_gpu_vector_width_half;

extern unsigned int frm_gpu_error_correction_support;
extern unsigned int frm_gpu_host_unified_memory;
extern unsigned int frm_gpu_profiling_timer_resolution;
extern unsigned int frm_gpu_endian_little;

extern unsigned int frm_gpu_device_available;
extern unsigned int frm_gpu_compiler_available;
extern unsigned int frm_gpu_linker_available;
extern unsigned int frm_gpu_execution_capabilities;

extern unsigned int frm_gpu_queue_properties;

extern unsigned int frm_gpu_platform;

extern unsigned int frm_gpu_register_alloc_size;
extern struct str_map_t frm_gpu_register_alloc_granularity_map;
extern enum frm_gpu_register_alloc_granularity_t
{
	frm_gpu_register_alloc_invalid = 0,  /* For invalid user input */
	frm_gpu_register_alloc_warp,
	frm_gpu_register_alloc_thread_block
} frm_gpu_register_alloc_granularity;

/* User configurable options */

extern int frm_gpu_num_sms;
extern int frm_gpu_num_warp_inst_queues;

extern int frm_gpu_max_thread_blocks_per_sm;
extern int frm_gpu_max_warps_per_sm;
extern int frm_gpu_max_threads_per_sm;
extern int frm_gpu_num_registers_per_sm;
extern int frm_gpu_num_registers_per_thread;

extern int frm_gpu_fe_fetch_latency;
extern int frm_gpu_fe_fetch_width;
extern int frm_gpu_fe_fetch_buffer_size;
extern int frm_gpu_fe_issue_latency;
extern int frm_gpu_fe_issue_width;
extern int frm_gpu_fe_max_inst_issued_per_type;

extern int frm_gpu_simd_num_simd_lanes;
extern int frm_gpu_simd_width;
extern int frm_gpu_simd_issue_buffer_size;
extern int frm_gpu_simd_decode_latency;
extern int frm_gpu_simd_decode_buffer_size;
extern int frm_gpu_simd_exec_latency;
extern int frm_gpu_simd_exec_buffer_size;

extern int frm_gpu_scalar_unit_width;
extern int frm_gpu_scalar_unit_issue_buffer_size;
extern int frm_gpu_scalar_unit_decode_latency;
extern int frm_gpu_scalar_unit_decode_buffer_size;
extern int frm_gpu_scalar_unit_read_latency;
extern int frm_gpu_scalar_unit_read_buffer_size;
extern int frm_gpu_scalar_unit_exec_latency;
extern int frm_gpu_scalar_unit_exec_buffer_size;
extern int frm_gpu_scalar_unit_write_latency;
extern int frm_gpu_scalar_unit_write_buffer_size;
extern int frm_gpu_scalar_unit_inflight_mem_accesses;

extern int frm_gpu_branch_unit_width;
extern int frm_gpu_branch_unit_issue_buffer_size;
extern int frm_gpu_branch_unit_decode_latency;
extern int frm_gpu_branch_unit_decode_buffer_size;
extern int frm_gpu_branch_unit_read_latency;
extern int frm_gpu_branch_unit_read_buffer_size;
extern int frm_gpu_branch_unit_exec_latency;
extern int frm_gpu_branch_unit_exec_buffer_size;
extern int frm_gpu_branch_unit_write_latency;
extern int frm_gpu_branch_unit_write_buffer_size;

extern int frm_gpu_lds_width;
extern int frm_gpu_lds_issue_buffer_size;
extern int frm_gpu_lds_decode_latency;
extern int frm_gpu_lds_decode_buffer_size;
extern int frm_gpu_lds_read_latency;
extern int frm_gpu_lds_read_buffer_size;
extern int frm_gpu_lds_exec_latency;
extern int frm_gpu_lds_exec_buffer_size;
extern int frm_gpu_lds_write_latency;
extern int frm_gpu_lds_write_buffer_size;
extern int frm_gpu_lds_max_inflight_mem_accesses;

extern int frm_gpu_vector_mem_width;
extern int frm_gpu_vector_mem_issue_buffer_size;
extern int frm_gpu_vector_mem_decode_latency;
extern int frm_gpu_vector_mem_decode_buffer_size;
extern int frm_gpu_vector_mem_read_latency;
extern int frm_gpu_vector_mem_read_buffer_size;
extern int frm_gpu_vector_mem_exec_latency;
extern int frm_gpu_vector_mem_exec_buffer_size;
extern int frm_gpu_vector_mem_write_latency;
extern int frm_gpu_vector_mem_write_buffer_size;
extern int frm_gpu_vector_mem_max_inflight_mem_accesses;

extern int frm_gpu_shared_mem_size;
extern int frm_gpu_lds_latency;
extern int frm_gpu_lds_block_size;
extern int frm_gpu_lds_num_ports;

#define FRM_GPU_FOREACH_SM(SM_ID) \
	for ((SM_ID) = 0; (SM_ID) < frm_gpu_num_sms; (SM_ID)++)

#define FRM_GPU_FOREACH_THREAD_IN_SUBWARP(WARP, SUBWARP_ID, THREAD_ID) \
	for ((THREAD_ID) = (WARP)->thread_id_first + (SUBWARP_ID) * frm_gpu_num_sps; \
		(THREAD_ID) <= MIN((WARP)->thread_id_first + ((SUBWARP_ID) + 1) \
			* frm_gpu_num_sps - 1, (WARP)->thread_id_last); \
		(THREAD_ID)++)

/* Forward declaration */
struct frm_uop_t;
struct frm_sm_t;
struct frm_scalar_unit_t;
struct frm_simd_t;
struct frm_branch_unit_t;
struct frm_vector_mem_unit_t;
struct frm_lds_t;



/*
 * Class 'FrmGpu
 */

CLASS_BEGIN(FrmGpu, Timing)

	/* Grids */
	struct frm_grid_t *grid;
	int thread_blocks_per_sm;
	int warps_per_sm;
	int threads_per_sm;

	/* Streaming multiprocessors */
	struct frm_sm_t **sms;

	/* Lists */
	struct list_t *sm_ready_list;
	struct list_t *sm_busy_list;

	/* List of deleted instructions */
	struct linked_list_t *trash_uop_list;

	long long int last_complete_cycle;

CLASS_END(FrmGpu)


void FrmGpuCreate(FrmGpu *self);
void FrmGpuDestroy(FrmGpu *self);

void FrmGpuDump(Object *self, FILE *f);
void FrmGpuDumpSummary(Timing *self, FILE *f);

int FrmGpuRun(Timing *self);




/*
 * Public Functions
 */

extern FrmGpu *frm_gpu;

void frm_gpu_read_config(void);

void frm_gpu_init(void);
void frm_gpu_done(void);

void frm_gpu_dump_default_config(char *filename);
void frm_gpu_dump_report(void);

void frm_gpu_uop_trash_add(struct frm_uop_t *uop);
void frm_gpu_uop_trash_empty(void);

void frm_sm_run_simd(struct frm_sm_t *sm);
void frm_sm_run_scalar_unit(struct frm_sm_t *sm);
void frm_sm_run_branch_unit(struct frm_sm_t *sm);

void frm_simd_run(struct frm_simd_t *simd);
void frm_scalar_unit_run(struct frm_scalar_unit_t *scalar_unit);
void frm_branch_unit_run(struct frm_branch_unit_t *branch_unit);
void frm_vector_mem_run(struct frm_vector_mem_unit_t *vector_mem);
void frm_lds_run(struct frm_lds_t *lds);

#endif

