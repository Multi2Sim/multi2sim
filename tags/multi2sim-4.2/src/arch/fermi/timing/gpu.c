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


#include <arch/fermi/emu/emu.h>
#include <arch/fermi/emu/grid.h>
#include <arch/fermi/emu/thread-block.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "calc.h"
#include "cycle-interval-report.h"
#include "gpu.h"
#include "sm.h"
#include "mem-config.h"
#include "uop.h"


/*
 * Global variables
 */

static char *frm_err_stall =
	"\tThe Fermi GPU has not completed execution of any in-flight\n"
	"\tinstruction for 1M cycles. Most likely, this means that a\n"
	"\tdeadlock condition occurred in the management of some modeled\n"
	"\tstructure (network, memory system, pipeline queues, etc.).\n";

/* FIXME: check the default paramters */
char *frm_gpu_config_help =
	"The Fermi GPU configuration file is a plain text INI file\n"
	"defining the parameters of the Fermi model for a detailed\n"
	"(architectural) configuration. This file is passed to Multi2Sim with\n"
	"the '--frm-config <file>' option, and should always be used together \n"
	"with option '--frm-sim detailed'.\n" 
	"\n"
	"The following is a list of the sections allowed in the GPU configuration\n"
	"file, along with the list of variables for each section.\n"
	"The default values are calibrated to Tesla C2050.\n"
	"\n"
	"Section '[ Device ]': parameters for the GPU.\n"
	"\n"
	"  Frequency = <value> (Default = 1000)\n"
	"      Frequency for the Fermi GPU in MHz.\n"
	"  NumSMs = <num> (Default = 14)\n"
	"      Number of SMs in the GPU.\n"
	"  MaxWarpsPerThreadBlock = <num> (Default = 16)\n"
	"      The maximum size of a thread block.\n"
	"\n"
	"Section '[ SM ]': parameters for the SMs.\n"
	"\n"
	"  NumWarpSchedulersPerSM = <num> (Default = 2)\n"
	"      Number of warp schedulers per SM.\n"
	"  MaxThreadBlocksPerSM = <num> (Default = 8)\n"
	"      The maximum number of thread blocks that can be scheduled to a\n"
	"      SM at a time.\n"
	"  MaxWarpsPerSM = <num> (Default = 48)\n"
	"      The maximum number of warps that can be scheduled to a\n"
	"      SM at a time.\n"
	"  NumRegistersPerSM = <num> (Default = 32768)\n"
	"      Number of registers per SM.\n"
	"\n"
	"Section '[ FrontEnd ]': parameters for fetch and issue.\n"
	"\n"
	"  FetchLatency = <cycles> (Default = 1)\n"
	"      Latency of instruction memory in number of cycles.\n"
	"  FetchWidth = <num> (Default = 2)\n"
	"      Maximum number of instructions fetched per cycle.\n"
	"  FetchBufferSize = <num> (Default = 10)\n"
	"      Size of the buffer holding fetched instructions.\n"
	"  IssueLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  IssueWidth = <num> (Default = 5)\n"
	"      Number of instructions that can be issued per cycle.\n"
	"  MaxInstIssuedPerSched = <num> (Default = 1)\n"
	"      Maximum number of instructions that can be issued per\n"
	"      warp scheduler.\n"
	"\n"
	"Section '[ SIMDUnit ]': parameters for the SIMD Units.\n"
	"\n"
	"  NumSIMDLanes = <num> (Default = 16)\n"
	"      Number of lanes per SIMD.  This must divide the warp\n"
	"      size (64) evenly.\n" 
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  DecodeBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding decoded instructions.\n"
	"  ReadExecWriteLatency = <cycles> (Default = 8)\n"
	"      Number of cycles it takes to read operands from the register\n"
	"      files, execute the SIMD ALU operation, and write the results\n"
	"      out to the register file for a single subwarp. It makes\n"
	"      sense to combine the three stages since they warp is\n"
	"      pipelined across all of them and can therefore be in different\n"
	"      stages at the same time.\n"
	"  ReadExecWriteBufferSize = <num> (Default = 2)\n"
	"      Size of the buffer holding instructions that have began the\n"
	"      read-exec-write stages.\n"
	"\n"
	"Section '[ BranchUnit ]': parameters for the Branch Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  ExecLatency = <cycles> (Default = 1)\n"
	"      Latency of execution in number of cycles.\n"
	"  ExecBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding executing instructions.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ LDSUnit ]': parameters for the LDS Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  MaxInflightMem = <num> (Default = 32)\n"
	"      Maximum number of in-flight memory accesses.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ VectorMemUnit ]': parameters for the Vector Memory Units.\n"
	"\n"
	"  Width = <num> (Default = 1)\n"
	"      Maximum number of instructions processed per cycle.\n"
	"  IssueBufferSize = <num> (Default = 4)\n"
	"      Size of the buffer holding issued instructions.\n"
	"  DecodeLatency = <cycles> (Default = 1)\n"
	"      Latency of the decode stage in number of cycles.\n"
	"  DecodeWidth = <num> (Default = 1)\n"
	"      Number of instructions that can be decoded per cycle.\n"
	"  ReadLatency = <cycles> (Default = 1)\n"
	"      Latency of register file access in number of cycles for reads.\n"
	"  ReadBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register read instructions.\n"
	"  MaxInflightMem = <num> (Default = 32)\n"
	"      Maximum number of in-flight memory accesses.\n"
	"  WriteLatency = <cycles> (Default = 1)\n"
	"      Latency of register file writes in number of cycles.\n"
	"  WriteBufferSize = <num> (Default = 1)\n"
	"      Size of the buffer holding register write instructions.\n"
	"\n"
	"Section '[ SharedMem ]': defines the parameters of the shared memory\n"
	"on each SM.\n"
	"\n"
	"  Size = <bytes> (Default = 48 KB)\n"
	"      LDS capacity per SM. This value must be\n"
	"      equal to or larger than BlockSize * Banks.\n"
	"  NumBanks = <num> (Default = 32)\n"
	"      Number of shared memory banks.\n"
	"  BlockSize = <bytes> (Default = 64)\n"
	"      Access block size, used for access coalescing purposes\n"
	"      among work-items.\n"
	"  Latency = <num_cycles> (Default = 2)\n"
	"      Latency for an access in number of cycles.\n"
	"  Ports = <num> (Default = 4)\n"
	"      Number of ports.\n"
	"\n";

char *frm_gpu_config_file_name = "";
char *frm_gpu_dump_default_config_file_name = "";
char *frm_gpu_report_file_name = "";

int frm_trace_category;

/* Default parameters based on the AMD Radeon HD 7970 */
unsigned long long frm_gpu_device_type = 4; /* CL_DEVICE_TYPE_GPU */
unsigned int frm_gpu_device_vendor_id = 1234; /* Completely arbitrary */

char *frm_gpu_device_profile = "FULL_PROFILE";
char *frm_gpu_device_name = "Multi2Sim Fermi GPU";
char *frm_gpu_device_vendor = "www.multi2sim.org";
char *frm_gpu_device_extensions = "cl_amd_fp64 cl_khr_global_int32_base_atomics "
	"cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "
	"cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "
	"cl_khr_gl_sharing cl_ext_device_fission cl_amd_device_attribute_query "
	"cl_amd_media_ops cl_amd_popcnt cl_amd_printf ";
char *frm_gpu_device_version = "CUDA 1.2 AMD-APP-SDK-v2.7";
char *frm_gpu_driver_version = VERSION;
char *frm_gpu_opencl_version = "CUDA C 1.2";

/* CUDA Device Query Information */
unsigned int frm_gpu_thread_dimensions = 3;  /* FIXME */
unsigned int frm_gpu_thread_sizes[3] = {256, 256, 256};  /* FIXME */
unsigned int frm_gpu_thread_block_size = 256 * 256 * 256;  /* FIXME */

unsigned int frm_gpu_image_support = 1; /* CL_TRUE */
unsigned int frm_gpu_max_read_image_args = 128;  /* The minimum value */
unsigned int frm_gpu_max_write_image_args = 8;  /* The minimum value */

unsigned int frm_gpu_image2d_max_width = 8192;  /* The minimum value */
unsigned int frm_gpu_image2d_max_height = 8192;  /* The minimum value */
unsigned int frm_gpu_image3d_max_width = 2048;  /* The minimum value */
unsigned int frm_gpu_image3d_max_height = 2048;  /* The minimum value */
unsigned int frm_gpu_image3d_max_depth = 2048;  /* The minimum value */
unsigned int frm_gpu_max_samplers = 16;  /* The minimum value */

unsigned int frm_gpu_max_parameter_size = 1024;  /* The minimum value */
/* FIXME */
unsigned int frm_gpu_mem_base_addr_align = 16 * 8;  /* size of long16 in bits */ 
/* FIXME */
unsigned int frm_gpu_min_data_type_align_size = 16;  /* size of long16 in bytes 
							deprecated in CUDA 1.2 */

/* bit field, all single floating point capabilities supported */ /* FIXME */
unsigned int frm_gpu_single_fp_config = 255;  
/* bit field, all double floating point capabilities supported */ /* FIXME */
unsigned int frm_gpu_double_fp_config = 255;  

unsigned int frm_gpu_max_clock_frequency = 925;
unsigned int frm_gpu_address_bits = 32;

unsigned int frm_gpu_global_mem_cache_type = 2;  /* CL_READ_WRITE_CACHE */
unsigned int frm_gpu_global_mem_cacheline_size = 256; /* FIXME */
unsigned long long frm_gpu_global_mem_cache_size = 1ull << 23;  /* 8MB */ /* FIXME */
unsigned long long frm_gpu_global_mem_size = 1ull << 32;  /* 4GB of global memory reported */
unsigned long long frm_gpu_max_mem_alloc_size = (1ull << 32) / 4; /* FIXME */

unsigned int frm_gpu_local_mem_type = 1;  /* CL_LOCAL */

unsigned long long frm_gpu_max_constant_buffer_size = 1ull << 16;  /* 64KB */ /* The minimum value acceptable */
unsigned int frm_gpu_max_constant_args = 8;  /* The minimum value acceptable */

unsigned int frm_gpu_vector_width_half = 0; /* No support for cl_khr_fp16 */

unsigned int frm_gpu_error_correction_support = 1;  /* FIXME */
unsigned int frm_gpu_host_unified_memory = 0;  /* FIXME */
unsigned int frm_gpu_profiling_timer_resolution = 10;  /* ns */ /* FIXME */
unsigned int frm_gpu_endian_little = 1;  /* FIXME */

unsigned int frm_gpu_device_available = 1;  /* FIXME */
unsigned int frm_gpu_compiler_available = 1;  /* FIXME */
unsigned int frm_gpu_linker_available = 1;  /* FIXME */
unsigned int frm_gpu_execution_capabilities = 1;  /* bit field, CL_EXEC_KERNEL only */ /* FIXME */

unsigned int frm_gpu_queue_properties = 2;  /* bit field, profiling enabled, no out of order execution */

unsigned int frm_gpu_platform = 0;  /* FIXME */

struct str_map_t frm_gpu_register_alloc_granularity_map =
{
	2, {
		{ "Warp", frm_gpu_register_alloc_warp },
		{ "ThreadBlock", frm_gpu_register_alloc_thread_block }
	}
};
enum frm_gpu_register_alloc_granularity_t frm_gpu_register_alloc_granularity;

/* Device parameters */
int frm_gpu_frequency = 1000;
int frm_gpu_num_sms = 1;
int frm_gpu_num_warp_inst_queues = 2;

/* Streaming multiprocessor parameters */
int frm_gpu_max_thread_blocks_per_sm = 8;
int frm_gpu_max_warps_per_sm = 48;
int frm_gpu_max_threads_per_sm = 1536;
int frm_gpu_num_registers_per_sm = 32768;
int frm_gpu_num_registers_per_thread = 63;

/* Front-end parameters */
int frm_gpu_fe_fetch_latency = 5;
int frm_gpu_fe_fetch_width = 4;
int frm_gpu_fe_fetch_buffer_size = 10;
int frm_gpu_fe_issue_latency = 1;
int frm_gpu_fe_issue_width = 5;
int frm_gpu_fe_max_inst_issued_per_type = 1;

/* SIMD unit parameters */
int frm_gpu_simd_num_simd_lanes = 16;
int frm_gpu_simd_width = 1;
int frm_gpu_simd_issue_buffer_size = 1;
int frm_gpu_simd_decode_latency = 1;
int frm_gpu_simd_decode_buffer_size = 1;
int frm_gpu_simd_exec_latency = 8;
int frm_gpu_simd_exec_buffer_size = 2;

/* Scalar unit parameters */
int frm_gpu_scalar_unit_width = 1;
int frm_gpu_scalar_unit_issue_buffer_size = 1;
int frm_gpu_scalar_unit_decode_latency = 1;
int frm_gpu_scalar_unit_decode_buffer_size = 1;
int frm_gpu_scalar_unit_read_latency = 1;
int frm_gpu_scalar_unit_read_buffer_size = 1;
int frm_gpu_scalar_unit_exec_latency = 1;
int frm_gpu_scalar_unit_exec_buffer_size = 16;
int frm_gpu_scalar_unit_write_latency = 1;
int frm_gpu_scalar_unit_write_buffer_size = 1;

/* Branch unit parameters */
int frm_gpu_branch_unit_width = 1;
int frm_gpu_branch_unit_issue_buffer_size = 1;
int frm_gpu_branch_unit_decode_latency = 1;
int frm_gpu_branch_unit_decode_buffer_size = 1;
int frm_gpu_branch_unit_read_latency = 1;
int frm_gpu_branch_unit_read_buffer_size = 1;
int frm_gpu_branch_unit_exec_latency = 1;
int frm_gpu_branch_unit_exec_buffer_size = 1;
int frm_gpu_branch_unit_write_latency = 1;
int frm_gpu_branch_unit_write_buffer_size = 1;

/* LDS unit parameters */
int frm_gpu_lds_width = 1;
int frm_gpu_lds_issue_buffer_size = 1;
int frm_gpu_lds_decode_latency = 1;
int frm_gpu_lds_decode_buffer_size = 1;
int frm_gpu_lds_read_latency = 1;
int frm_gpu_lds_read_buffer_size = 1;
int frm_gpu_lds_max_inflight_mem_accesses = 32;
int frm_gpu_lds_write_latency = 1;
int frm_gpu_lds_write_buffer_size = 1;

/* Vector mem unit parameters */
int frm_gpu_vector_mem_width = 1;
int frm_gpu_vector_mem_issue_buffer_size = 1;
int frm_gpu_vector_mem_decode_latency = 1;
int frm_gpu_vector_mem_decode_buffer_size = 1;
int frm_gpu_vector_mem_read_latency = 1;
int frm_gpu_vector_mem_read_buffer_size = 1;
int frm_gpu_vector_mem_max_inflight_mem_accesses = 32;
int frm_gpu_vector_mem_write_latency = 1;
int frm_gpu_vector_mem_write_buffer_size = 1;

/* Shared memory parameters */
int frm_gpu_shared_mem_size = 49152;
int frm_gpu_lds_latency = 2;
int frm_gpu_lds_block_size = 64;
int frm_gpu_lds_num_ports = 2;

FrmGpu *frm_gpu;



/*
 * Private Functions
 */

/* Version of Fermi trace producer.
 * See 'src/visual/fermi/gpu.c' for Fermi trace consumer. */

#define FRM_TRACE_VERSION_MAJOR		1
#define FRM_TRACE_VERSION_MINOR		1


/* FIXME */
void frm_config_dump(FILE *f)
{
	/* Device */
	fprintf(f, "[ Config.Device ]\n");
	fprintf(f, "NumSMs = %d\n", frm_gpu_num_sms);
	fprintf(f, "\n");

	/* SM */
	fprintf(f, "[ Config.SM ]\n");
	fprintf(f, "NumRegistersPerSM = %d\n", frm_gpu_num_registers_per_sm);
	fprintf(f, "MaxThreadBlocksPerSM = %d\n",
			frm_gpu_max_thread_blocks_per_sm);
	fprintf(f, "MaxWarpsPerSM = %d\n",
			frm_gpu_max_warps_per_sm);
	fprintf(f, "\n");

	/* Front-End */
	fprintf(f, "[ Config.FrontEnd ]\n");
	fprintf(f, "FetchLatency = %d\n", frm_gpu_fe_fetch_latency);
	fprintf(f, "FetchWidth = %d\n", frm_gpu_fe_fetch_width);
	fprintf(f, "FetchBufferSize = %d\n", frm_gpu_fe_fetch_buffer_size);
	fprintf(f, "IssueLatency = %d\n", frm_gpu_fe_issue_latency);
	fprintf(f, "IssueWidth = %d\n", frm_gpu_fe_issue_width);
	fprintf(f, "MaxInstIssuedPerType = %d\n",
			frm_gpu_fe_max_inst_issued_per_type);
	fprintf(f, "\n");

	/* SIMD Unit */
	fprintf(f, "[ Config.SIMDUnit ]\n");
	fprintf(f, "NumSIMDLanes = %d\n", frm_gpu_simd_num_simd_lanes);
	fprintf(f, "Width = %d\n", frm_gpu_simd_width);
	fprintf(f, "IssueBufferSize = %d\n", frm_gpu_simd_issue_buffer_size);
	fprintf(f, "DecodeLatency = %d\n", frm_gpu_simd_decode_latency);
	fprintf(f, "DecodeBufferSize = %d\n", frm_gpu_simd_decode_buffer_size);
	fprintf(f, "ReadExecWriteLatency = %d\n", frm_gpu_simd_exec_latency);
	fprintf(f, "ReadExecWriteBufferSize = %d\n",
			frm_gpu_simd_exec_buffer_size);
	fprintf(f, "\n");

	/* Branch Unit */
	fprintf(f, "[ Config.BranchUnit ]\n");
	fprintf(f, "Width = %d\n", frm_gpu_branch_unit_width);
	fprintf(f, "IssueBufferSize = %d\n",
			frm_gpu_branch_unit_issue_buffer_size);
	fprintf(f, "DecodeLatency = %d\n", frm_gpu_branch_unit_decode_latency);
	fprintf(f, "DecodeBufferSize = %d\n",
			frm_gpu_branch_unit_decode_buffer_size);
	fprintf(f, "ReadLatency = %d\n", frm_gpu_branch_unit_read_latency);
	fprintf(f, "ReadBufferSize = %d\n",
			frm_gpu_branch_unit_read_buffer_size);
	fprintf(f, "ExecLatency = %d\n", frm_gpu_branch_unit_exec_latency);
	fprintf(f, "ExecBufferSize = %d\n",
			frm_gpu_branch_unit_exec_buffer_size);
	fprintf(f, "WriteLatency = %d\n", frm_gpu_branch_unit_write_latency);
	fprintf(f, "WriteBufferSize = %d\n",
			frm_gpu_branch_unit_write_buffer_size);
	fprintf(f, "\n");

	/* LDS */
	fprintf(f, "[ Config.LDSUnit ]\n");
	fprintf(f, "Width = %d\n", frm_gpu_lds_width);
	fprintf(f, "IssueBufferSize = %d\n", frm_gpu_lds_issue_buffer_size);
	fprintf(f, "DecodeLatency = %d\n", frm_gpu_lds_decode_latency);
	fprintf(f, "DecodeBufferSize = %d\n",
			frm_gpu_lds_decode_buffer_size);
	fprintf(f, "ReadLatency = %d\n", frm_gpu_lds_read_latency);
	fprintf(f, "ReadBufferSize = %d\n", frm_gpu_lds_read_buffer_size);
	fprintf(f, "MaxInflightMem = %d\n",
			frm_gpu_lds_max_inflight_mem_accesses);
	fprintf(f, "WriteLatency = %d\n", frm_gpu_lds_write_latency);
	fprintf(f, "WriteBufferSize = %d\n", frm_gpu_lds_write_buffer_size);
	fprintf(f, "\n");

	/* Vector Memory */
	fprintf(f, "[ Config.VectorMemUnit ]\n");
	fprintf(f, "Width = %d\n", frm_gpu_vector_mem_width);
	fprintf(f, "IssueBufferSize = %d\n",
			frm_gpu_vector_mem_issue_buffer_size);
	fprintf(f, "DecodeLatency = %d\n", frm_gpu_vector_mem_decode_latency);
	fprintf(f, "DecodeBufferSize = %d\n",
			frm_gpu_vector_mem_decode_buffer_size);
	fprintf(f, "ReadLatency = %d\n", frm_gpu_vector_mem_read_latency);
	fprintf(f, "ReadBufferSize = %d\n", frm_gpu_vector_mem_read_buffer_size);
	fprintf(f, "MaxInflightMem = %d\n",
			frm_gpu_vector_mem_max_inflight_mem_accesses);
	fprintf(f, "WriteLatency = %d\n", frm_gpu_vector_mem_write_latency);
	fprintf(f, "WriteBufferSize = %d\n",
			frm_gpu_vector_mem_write_buffer_size);
	fprintf(f, "\n");

	/* LDS */
	fprintf(f, "[ Config.SharedMem ]\n");
	fprintf(f, "Size = %d\n", frm_gpu_shared_mem_size);
	fprintf(f, "BlockSize = %d\n", frm_gpu_lds_block_size);
	fprintf(f, "Latency = %d\n", frm_gpu_lds_latency);
	fprintf(f, "Ports = %d\n", frm_gpu_lds_num_ports);
	fprintf(f, "\n");

	/* End of configuration */
	fprintf(f, "\n");
}


static void frm_gpu_map_grid(struct frm_grid_t *grid)
{
	/* Assign current grid */
	assert(!frm_gpu->grid);
	frm_gpu->grid = grid;

	/* Calculate the number of thread blocks per SM */
	frm_gpu->thread_blocks_per_sm =
		frm_calc_get_thread_blocks_per_sm(
				grid->block_size, grid->num_gpr_used,
				grid->local_mem_top);

	/* Thread block cannot be assigned to SM */
	if (!frm_gpu->thread_blocks_per_sm)
	{
		fatal("Thread blocks cannot be assigned to SMs.\n"
				"\tA SM in the GPU has a limit in number of warps,\n"
				"\tnumber of registers, and amount of shared memory.\n"
				"\tIf a thread block requires more resources than\n"
				"\tthese limits, it cannot be assigned to a SM.\n");
	}

	/* Calculate limit of warps and threads per SM */
	frm_gpu->warps_per_sm = frm_gpu->thread_blocks_per_sm *
		grid->thread_blocks[0]->warp_count;
	frm_gpu->threads_per_sm = frm_gpu->warps_per_sm * frm_emu_warp_size;
}


static void frm_gpu_unmap_grid(void)
{
	/* Dump stats */
	frm_grid_dump(frm_gpu->grid, frm_emu_report_file);

	/* Unmap */
	frm_gpu->grid = NULL;
}




/*
 * Public Functions
 */

/* FIXME - some copied from Southern Islands, check if required in Fermi */
void frm_gpu_read_config(void)
{
	struct config_t *gpu_config;
	char *section;
	const char *err_note =
		"\tPlease run 'm2s --frm-help' or consult the Multi2Sim Guide "
		"for a\n\tdescription of the GPU configuration file format.";

	/* Load GPU configuration file */
	gpu_config = config_create(frm_gpu_config_file_name);
	if (*frm_gpu_config_file_name)
		config_load(gpu_config);

	/* Device */
	section = "Device";

	frm_gpu_frequency = config_read_int(gpu_config, section, "Frequency", frm_gpu_frequency);
	if (!IN_RANGE(frm_gpu_frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: invalid value for 'Frequency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_num_sms = config_read_int(gpu_config, section, "NumSMs", frm_gpu_num_sms);
	if (frm_gpu_num_sms < 1)
		fatal("%s: invalid value for 'NumSMs'.\n%s", 
				frm_gpu_config_file_name, err_note);

	/* SM */
	section = "SM";

	frm_gpu_max_thread_blocks_per_sm = config_read_int(
			gpu_config, section, "MaxThreadBlocksPerSM",
			frm_gpu_max_thread_blocks_per_sm);
	if (frm_gpu_max_thread_blocks_per_sm < 1)
		fatal("%s: invalid value for 'MaxThreadBlocksPerSM'.\n%s", 
				frm_gpu_config_file_name, err_note);

	frm_gpu_max_warps_per_sm = config_read_int(
			gpu_config, section, "MaxWarpsPerSM",
			frm_gpu_max_warps_per_sm);
	if (frm_gpu_max_warps_per_sm < 1)
		fatal("%s: invalid value for 'MaxWarpsPerSM'.\n%s", 
				frm_gpu_config_file_name, err_note);

	frm_gpu_num_registers_per_sm = config_read_int(
			gpu_config, section, "NumRegistersPerSM", 
			frm_gpu_num_registers_per_sm);
	if (frm_gpu_num_registers_per_sm < 1)
		fatal("%s: invalid value for 'NumRegistersPerSM'.\n%s", 
				frm_gpu_config_file_name, err_note);

	/* Front End */
	section = "FrontEnd";

	frm_gpu_fe_fetch_latency = config_read_int(
			gpu_config, section, "FetchLatency", frm_gpu_fe_fetch_latency);
	if (frm_gpu_fe_fetch_latency < 0)
		fatal("%s: invalid value for 'FetchLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_fe_fetch_width = config_read_int(
			gpu_config, section, "FetchWidth", frm_gpu_fe_fetch_width);
	if (frm_gpu_fe_fetch_width < 1)
		fatal("%s: invalid value for 'FetchWidth'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_fe_fetch_buffer_size = config_read_int(
			gpu_config, section, "FetchBufferSize", 
			frm_gpu_fe_fetch_buffer_size);
	if (frm_gpu_fe_fetch_buffer_size < 1)
		fatal("%s: invalid value for 'FetchBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_fe_issue_latency = config_read_int(
			gpu_config, section, "IssueLatency", frm_gpu_fe_issue_latency);
	if (frm_gpu_fe_issue_latency < 0)
		fatal("%s: invalid value for 'IssueLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_fe_issue_width = config_read_int(
			gpu_config, section, "IssueWidth", frm_gpu_fe_issue_width);
	if (frm_gpu_fe_issue_width < 1)
		fatal("%s: invalid value for 'IssueWidth'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* SIMD Unit */
	section = "SIMDUnit";

	frm_gpu_simd_num_simd_lanes = config_read_int(
			gpu_config, section, "NumSIMDLanes", frm_gpu_simd_num_simd_lanes);
	if (frm_gpu_simd_num_simd_lanes < 1)
		fatal("%s: invalid value for 'NumSIMDLanes'.\n%s", 
				frm_gpu_config_file_name, err_note);

	frm_gpu_simd_width = config_read_int(
			gpu_config, section, "Width", frm_gpu_simd_width);
	if (frm_gpu_simd_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_simd_issue_buffer_size = config_read_int(
			gpu_config, section, "IssueBufferSize", 
			frm_gpu_simd_issue_buffer_size);
	if (frm_gpu_simd_issue_buffer_size < 1)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_simd_decode_latency = config_read_int(
			gpu_config, section, "DecodeLatency", 
			frm_gpu_simd_decode_latency);
	if (frm_gpu_simd_decode_latency < 0)
		fatal("%s: invalid value for 'DecodeLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_simd_decode_buffer_size = config_read_int(
			gpu_config, section, "DecodeBufferSize", 
			frm_gpu_simd_decode_buffer_size);
	if (frm_gpu_simd_decode_buffer_size < 1)
		fatal("%s: invalid value for 'DecodeBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* FIXME Need to compute this dynamically based on the number
	 * of the number of SIMD lanes */
	frm_gpu_simd_exec_latency = config_read_int(
			gpu_config, section, "ReadExecWriteLatency", 
			frm_gpu_simd_exec_latency);
	if (frm_gpu_simd_exec_latency < 0)
		fatal("%s: invalid value for 'ReadExecWriteLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_simd_exec_buffer_size = config_read_int(
			gpu_config, section, "ReadExecWriteBufferSize", 
			frm_gpu_simd_exec_buffer_size);
	if (frm_gpu_simd_exec_buffer_size < 1)
		fatal("%s: invalid value for 'ReadExecWriteBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* Branch Unit */
	section = "BranchUnit";

	frm_gpu_branch_unit_width = config_read_int(
			gpu_config, section, "Width", frm_gpu_branch_unit_width);
	if (frm_gpu_branch_unit_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_issue_buffer_size = config_read_int(
			gpu_config, section, "IssueBufferSize", 
			frm_gpu_branch_unit_issue_buffer_size);
	if (frm_gpu_branch_unit_issue_buffer_size < 1)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_decode_latency = config_read_int(
			gpu_config, section, "DecodeLatency", 
			frm_gpu_branch_unit_decode_latency);
	if (frm_gpu_branch_unit_decode_latency < 0)
		fatal("%s: invalid value for 'DecodeLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_decode_buffer_size = config_read_int(
			gpu_config, section, "DecodeBufferSize", 
			frm_gpu_branch_unit_decode_buffer_size);
	if (frm_gpu_branch_unit_decode_buffer_size < 1)
		fatal("%s: invalid value for 'DecodeBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_read_latency = config_read_int(
			gpu_config, section, "ReadLatency", 
			frm_gpu_branch_unit_read_latency);
	if (frm_gpu_branch_unit_read_latency < 0)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_read_buffer_size = config_read_int(
			gpu_config, section, "ReadBufferSize", 
			frm_gpu_branch_unit_read_buffer_size);
	if (frm_gpu_branch_unit_read_buffer_size < 1)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_exec_latency = config_read_int(
			gpu_config, section, "ExecLatency", 
			frm_gpu_branch_unit_exec_latency);
	if (frm_gpu_branch_unit_exec_latency < 0)
		fatal("%s: invalid value for 'ExecLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_exec_buffer_size= config_read_int(
			gpu_config, section, "ExecBufferSize", 
			frm_gpu_branch_unit_exec_buffer_size);
	if (frm_gpu_branch_unit_exec_buffer_size < 1)
		fatal("%s: invalid value for 'ExecBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_write_latency = config_read_int(
			gpu_config, section, "WriteLatency", 
			frm_gpu_branch_unit_write_latency);
	if (frm_gpu_branch_unit_write_latency < 0)
		fatal("%s: invalid value for 'WriteLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_branch_unit_write_buffer_size = config_read_int(
			gpu_config, section, "WriteBufferSize", 
			frm_gpu_branch_unit_write_buffer_size);
	if (frm_gpu_branch_unit_write_buffer_size < 1)
		fatal("%s: invalid value for 'WriteBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* LDS Unit */
	section = "LDSUnit";

	frm_gpu_lds_width = config_read_int(
			gpu_config, section, "Width", frm_gpu_lds_width);
	if (frm_gpu_lds_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_issue_buffer_size = config_read_int(
			gpu_config, section, "IssueBufferSize", 
			frm_gpu_lds_issue_buffer_size);
	if (frm_gpu_lds_issue_buffer_size < 1)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_decode_latency = config_read_int(
			gpu_config, section, "DecodeLatency", 
			frm_gpu_lds_decode_latency);
	if (frm_gpu_lds_decode_latency < 0)
		fatal("%s: invalid value for 'DecodeLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_decode_buffer_size = config_read_int(
			gpu_config, section, "DecodeBufferSize", 
			frm_gpu_lds_decode_buffer_size);
	if (frm_gpu_lds_decode_buffer_size < 1)
		fatal("%s: invalid value for 'DecodeBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_read_latency = config_read_int(
			gpu_config, section, "ReadLatency", frm_gpu_lds_read_latency);
	if (frm_gpu_lds_read_latency < 0)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_read_buffer_size = config_read_int(
			gpu_config, section, "ReadBufferSize", 
			frm_gpu_lds_read_buffer_size);
	if (frm_gpu_lds_read_buffer_size < 1)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_max_inflight_mem_accesses = config_read_int(
			gpu_config, section, "MaxInflightMem", 
			frm_gpu_lds_max_inflight_mem_accesses);
	if (frm_gpu_lds_max_inflight_mem_accesses < 1)
		fatal("%s: invalid value for 'MaxInflightMem'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_write_latency = config_read_int(
			gpu_config, section, "WriteLatency", frm_gpu_lds_write_latency);
	if (frm_gpu_lds_write_latency < 0)
		fatal("%s: invalid value for 'WriteLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_lds_write_buffer_size = config_read_int(
			gpu_config, section, "WriteBufferSize", 
			frm_gpu_lds_write_buffer_size);
	if (frm_gpu_lds_write_buffer_size < 1)
		fatal("%s: invalid value for 'WriteBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* VectorMem Unit */
	section = "VectorMemUnit";

	frm_gpu_vector_mem_width = config_read_int(
			gpu_config, section, "Width", frm_gpu_vector_mem_width);
	if (frm_gpu_vector_mem_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_issue_buffer_size = config_read_int(
			gpu_config, section, "IssueBufferSize", 
			frm_gpu_vector_mem_issue_buffer_size);
	if (frm_gpu_vector_mem_issue_buffer_size < 1)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_decode_latency = config_read_int(
			gpu_config, section, "DecodeLatency", 
			frm_gpu_vector_mem_decode_latency);
	if (frm_gpu_vector_mem_decode_latency < 0)
		fatal("%s: invalid value for 'DecodeLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_decode_buffer_size = config_read_int(
			gpu_config, section, "DecodeBufferSize", 
			frm_gpu_vector_mem_decode_buffer_size);
	if (frm_gpu_vector_mem_decode_buffer_size < 1)
		fatal("%s: invalid value for 'DecodeBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_read_latency = config_read_int(
			gpu_config, section, "ReadLatency", 
			frm_gpu_vector_mem_read_latency);
	if (frm_gpu_vector_mem_read_latency < 0)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_read_buffer_size = config_read_int(
			gpu_config, section, "ReadBufferSize", 
			frm_gpu_vector_mem_read_buffer_size);
	if (frm_gpu_vector_mem_read_buffer_size < 1)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_max_inflight_mem_accesses = config_read_int(
			gpu_config, section, "MaxInflightMem", 
			frm_gpu_vector_mem_max_inflight_mem_accesses);
	if (frm_gpu_vector_mem_max_inflight_mem_accesses < 1)
		fatal("%s: invalid value for 'MaxInflightMem'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_write_latency = config_read_int(
			gpu_config, section, "WriteLatency", 
			frm_gpu_vector_mem_write_latency);
	if (frm_gpu_vector_mem_write_latency < 0)
		fatal("%s: invalid value for 'WriteLatency'.\n%s",
				frm_gpu_config_file_name, err_note);

	frm_gpu_vector_mem_write_buffer_size = config_read_int(
			gpu_config, section, "WriteBufferSize", 
			frm_gpu_vector_mem_write_buffer_size);
	if (frm_gpu_vector_mem_write_buffer_size < 1)
		fatal("%s: invalid value for 'WriteBufferSize'.\n%s",
				frm_gpu_config_file_name, err_note);

	/* Shared Memory Unit */
	section = "SharedMemUnit";

	frm_gpu_shared_mem_size = config_read_int(
			gpu_config, section, "Size", frm_gpu_shared_mem_size);
	frm_gpu_lds_block_size = config_read_int(
			gpu_config, section, "BlockSize", frm_gpu_lds_block_size);
	frm_gpu_lds_latency = config_read_int(
			gpu_config, section, "Latency", frm_gpu_lds_latency);
	frm_gpu_lds_num_ports = config_read_int(
			gpu_config, section, "Ports", frm_gpu_lds_num_ports);

	if ((frm_gpu_lds_block_size & (frm_gpu_lds_block_size - 1)) || 
			frm_gpu_lds_block_size < 4)
		fatal("%s: %s->BlockSize must be a power of two and at "
				"least 4.\n%s", frm_gpu_config_file_name, section, 
				err_note);
	if (frm_gpu_lds_latency < 1)
		fatal("%s: invalid value for %s->Latency.\n%s", 
				frm_gpu_config_file_name, section, err_note);
	if (frm_gpu_shared_mem_size < frm_gpu_lds_block_size)
		fatal("%s: %s->Size cannot be smaller than %s->BlockSize * "
				"%s->Banks.\n%s", frm_gpu_config_file_name, section, 
				section, section, err_note);

	/* Cycle Interval report */
	frm_spatial_report_config_read(gpu_config);

	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);
}


void frm_gpu_init(void)
{
	/* Classes */
	CLASS_REGISTER(FrmGpu);

	/* Trace */
	frm_trace_category = trace_new_category();

	/* Try to open report file */
	if (frm_gpu_report_file_name[0] && 
			!file_can_open_for_write(frm_gpu_report_file_name))
		fatal("%s: cannot open GPU pipeline report file",
				frm_gpu_report_file_name);

	/* Trace */
	frm_trace_header("frm.init version=\"%d.%d\" num_sms=%d\n",
			FRM_TRACE_VERSION_MAJOR, FRM_TRACE_VERSION_MINOR,
			frm_gpu_num_sms);

	/* Create GPU */
	frm_gpu = new(FrmGpu);

	/* Initializations */
	frm_uop_init();
}


void frm_gpu_done()
{
	/* GPU pipeline report */
	frm_gpu_dump_report();

	/* Free GPU */
	delete(frm_gpu);

	/* Spatial report */
	if (frm_spatial_report_active)
		frm_sm_spatial_report_done();

	/* Finalizations */
	frm_uop_done();
}


void frm_gpu_dump_default_config(char *filename)
{
	FILE *f;

	/* Open file */
	f = file_open_for_write(filename);
	if (!f)
		return;

	frm_config_dump(f);

	file_close(f);
}


void frm_gpu_dump_report(void)
{
	struct frm_sm_t *sm;
	struct mod_t *lds_mod;
	int sm_id;

	FILE *f;

	double inst_per_cycle;
	long long coalesced_reads;
	long long coalesced_writes;

	/* Open file */
	f = file_open_for_write(frm_gpu_report_file_name);
	if (!f)
		return;

	/* Dump GPU configuration */
	fprintf(f, ";\n; GPU Configuration\n;\n\n");
	frm_config_dump(f);

	/* Report for device */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	inst_per_cycle = asTiming(frm_gpu)->cycle ? 
		(double)(asEmu(frm_emu)->instructions / asTiming(frm_gpu)->cycle) : 0.0;
	fprintf(f, "[ Device ]\n\n");
	fprintf(f, "GridCount = %d\n", frm_emu->grid_count);
	fprintf(f, "Instructions = %lld\n", asEmu(frm_emu)->instructions);
	fprintf(f, "BranchInstructions = %lld\n", frm_emu->branch_inst_count);
	fprintf(f, "ALUInstructions = %lld\n", frm_emu->vector_alu_inst_count);
	fprintf(f, "SharedMemInstructions = %lld\n", frm_emu->lds_inst_count);
	fprintf(f, "GlobalMemInstructions = %lld\n", frm_emu->vector_mem_inst_count);
	fprintf(f, "Cycles = %lld\n", asTiming(frm_gpu)->cycle);
	fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "\n\n");

	/* Report for SMs */
	FRM_GPU_FOREACH_SM(sm_id)
	{
		sm = frm_gpu->sms[sm_id];
		lds_mod = sm->lds_module;

		inst_per_cycle = sm->cycle ? 
			(double)(sm->inst_count/sm->cycle) :
			0.0;
		coalesced_reads = lds_mod->reads - lds_mod->effective_reads;
		coalesced_writes = lds_mod->writes - lds_mod->effective_writes;

		fprintf(f, "[ SM%d ]\n\n", sm_id);

		fprintf(f, "ThreadBlockCount = %lld\n", sm->mapped_thread_blocks);
		fprintf(f, "Instructions = %lld\n", sm->inst_count);
		fprintf(f, "BranchInstructions = %lld\n", sm->branch_inst_count);
		fprintf(f, "ALUInstructions = %lld\n", 	sm->simd_inst_count);
		fprintf(f, "SharedMemInstructions = %lld\n", sm->lds_inst_count);
		fprintf(f, "GlobalMemInstructions = %lld\n", sm->vector_mem_inst_count);
		fprintf(f, "Cycles = %lld\n", sm->cycle);
		fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "SharedMem.Accesses = %lld\n", lds_mod->reads + lds_mod->writes);
		fprintf(f, "SharedMem.Reads = %lld\n", lds_mod->reads);
		fprintf(f, "SharedMem.EffectiveReads = %lld\n", lds_mod->effective_reads);
		fprintf(f, "SharedMem.CoalescedReads = %lld\n", coalesced_reads);
		fprintf(f, "SharedMem.Writes = %lld\n", lds_mod->writes);
		fprintf(f, "SharedMem.EffectiveWrites = %lld\n", lds_mod->effective_writes);
		fprintf(f, "SharedMem.CoalescedWrites = %lld\n", coalesced_writes);
		fprintf(f, "\n\n");
	}

	file_close(f);
}



/*
 * Class 'FrmGpu'
 */

CLASS_IMPLEMENTATION(FrmGpu);


void FrmGpuCreate(FrmGpu *self)
{
	struct frm_sm_t *sm;
	int sm_id;

	/* Parent */
	TimingCreate(asTiming(self));

	/* Frequency */
	asTiming(self)->frequency = frm_gpu_frequency;
	asTiming(self)->frequency_domain = esim_new_domain(frm_gpu_frequency);

	/* Initialize SMs */
	self->sms = xcalloc(frm_gpu_num_sms, sizeof(struct frm_sm_t *));
	self->sm_ready_list = list_create();
	FRM_GPU_FOREACH_SM(sm_id)
	{
		self->sms[sm_id] = frm_sm_create();
		sm = self->sms[sm_id];
		sm->id = sm_id;
		list_add(self->sm_ready_list, sm);
	}

	/* Virtual functions */
	asObject(self)->Dump = FrmGpuDump;
	asTiming(self)->DumpSummary = FrmGpuDumpSummary;
	asTiming(self)->Run = FrmGpuRun;
	asTiming(self)->MemConfigCheck = FrmGpuMemConfigCheck;
	asTiming(self)->MemConfigDefault = FrmGpuMemConfigDefault;
	asTiming(self)->MemConfigParseEntry = FrmGpuMemConfigParseEntry;
}


void FrmGpuDestroy(FrmGpu *self)
{
	struct frm_sm_t *sm;
	int sm_id;

	FRM_GPU_FOREACH_SM(sm_id)
	{
		sm = self->sms[sm_id];
		frm_sm_free(sm);
	}
	list_free(self->sm_ready_list);
	free(self->sms);
}


void FrmGpuDump(Object *self, FILE *f)
{
}


void FrmGpuDumpSummary(Timing *self, FILE *f)
{
	/* Call parent */
	TimingDumpSummary(self, f);
}


int FrmGpuRun(Timing *self)
{
	FrmGpu *gpu = asFrmGpu(self);

	struct frm_grid_t *grid;

	struct frm_sm_t *sm;
	struct frm_sm_t *sm_next;
	int sm_id;

	/* FIXME - temporarily disabled due to segfault.
	 * Feature not available for 4.2 anyway. */
	fatal("%s: Fermi timing simulation not supported yet",
			__FUNCTION__);

	/* For efficiency when no Fermi emulation is selected, 
	 * exit here if the list of existing grids is empty. */
	if (!list_count(frm_emu->grids))
		return FALSE;

	/* Map grids */
	while ((grid = list_head(frm_emu->pending_grids)))
	{
		/* Set grid status to 'running' */
		list_remove(frm_emu->pending_grids, grid);
		list_add(frm_emu->running_grids, grid);

		/* Trace */
		frm_trace("frm.new_grid id=%d tb_first=%d tb_count=%d\n", 
				grid->id, grid->thread_blocks[0]->id, 
				grid->thread_block_count);

		/* Map grid to GPU */
		frm_gpu_map_grid(grid);
		frm_calc_plot();
	}

	/* Get mapped grids */
	grid = gpu->grid;
	assert(grid);

	/* Assign thread blocks to SMs */
	while (list_head(gpu->sm_ready_list) && 
			list_head(grid->pending_thread_blocks))
	{
		frm_sm_map_thread_block(list_head(gpu->sm_ready_list),
				list_head(grid->pending_thread_blocks));
	}

	/* One more cycle */
	asTiming(frm_gpu)->cycle++;

	/* Stop if maximum number of GPU cycles exceeded */
	if (frm_emu_max_cycles && asTiming(frm_gpu)->cycle >= frm_emu_max_cycles)
		esim_finish = esim_finish_frm_max_cycles;

	/* Stop if maximum number of GPU instructions exceeded */
	if (frm_emu_max_inst && asEmu(frm_emu)->instructions >= frm_emu_max_inst)
		esim_finish = esim_finish_frm_max_inst;

	/* Stop if there was a simulation stall */
	if ((asTiming(frm_gpu)->cycle - gpu->last_complete_cycle) > 1000000)
	{
		warning("Fermi GPU simulation stalled.\n%s", frm_err_stall);
		esim_finish = esim_finish_stall;
	}

	/* Stop if any reason met */
	if (esim_finish)
		return TRUE;

	/* Run one loop iteration on each busy SM */
	for (sm = list_head(gpu->sm_busy_list), sm_id = 0; sm; 
			sm = sm_next, sm_id++)
	{
		/* Store next busy SM, since this can change
		 * during the SM simulation loop iteration. */
		sm_next = list_get(gpu->sm_busy_list, sm_id + 1);

		/* Run one cycle */
		frm_sm_run(sm);
	}

	/* Finish execution */
	if (!list_count(gpu->sm_busy_list))
	{
		/* Dump Grid report */
		frm_grid_dump(grid, frm_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (frm_emu_max_functions && list_count(frm_emu->grids) >= 
				frm_emu_max_functions)
		{
			esim_finish = esim_finish_frm_max_functions;
		}

		/* Finalize and free Grid */
		assert(grid->status == frm_grid_finished);
		frm_gpu_unmap_grid();
		frm_grid_free(grid);
	}

	/* Still simulating */
	return TRUE;
}
