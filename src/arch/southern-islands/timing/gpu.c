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

#include <arch/common/arch.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/file.h>
#include <lib/util/misc.h>
#include <lib/util/timer.h>

#include "mem-config.h"
#include <arch/southern-islands/timing/gpu.h>
#include <arch/southern-islands/timing/calc.h>
#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/opencl-kernel.h>


static char *si_err_stall =
	"\tThe Southern Islands GPU has not completed execution of any in-flight\n"
	"\tinstruction for 1M cycles. Most likely, this means that a\n"
	"\tdeadlock condition occurred in the management of some modeled\n"
	"\tstructure (network, memory system, pipeline queues, etc.).\n";

/*
 * Global variables
 */

char *si_gpu_config_help =
	"The Southern Islands GPU configuration file is a plain text INI file defining\n"
	"the parameters of the GPU model for a detailed (architectural) GPU configuration.\n"
	"This file is passed to Multi2Sim with the '--si-config <file>' option, and\n"
	"should always be used together with option '--si-sim detailed'.\n"
	"\n"
	"The following is a list of the sections allowed in the GPU configuration file,\n"
	"along with the list of variables for each section.\n"
	"\n"
	"Section '[ Device ]': parameters for the GPU.\n"
	"\n"
	"  NumComputeUnits = <num> (Default = 20)\n"
	"      Number of compute units in the GPU. A compute unit runs one or more\n"
	"      work-groups at a time.\n"
	"  NumStreamCores = <num> (Default = 16)\n"
	"      Number of stream cores in the ALU engine of a compute unit. Each work-item\n"
	"      is mapped to a stream core when a VLIW bundle is executed. Stream cores are\n"
	"      time-multiplexed to cover all work-items in a wavefront.\n"
	"  NumRegisters = <num> (Default = 16K)\n"
	"      Number of registers in a compute unit. These registers are shared among all\n"
	"      work-items running in a compute unit. This is one of the factors limiting the\n"
	"      number of work-groups mapped to a compute unit.\n"
	"  RegisterAllocSize = <num> (Default = 32)\n"
	"  RegisterAllocGranularity = {Wavefront|WorkGroup} (Default = WorkGroup)\n"
	"      Minimum amount of registers allocated as a chunk for each wavefront or\n"
	"      work-group, depending on the granularity. These parameters have an impact\n"
	"      in the allocation of work-groups to compute units.\n"
	"  WavefrontSize = <size> (Default = 64)\n"
	"      Number of work-items in a wavefront, executing AMD Southern Islands instructions in\n"
	"      a SIMD fashion.\n"
	"  MaxWorkGroupsPerComputeUnit = <num> (Default = 8)\n"
	"  MaxWavefrontsPerComputeUnit = <num> (Default = 32)\n"
	"      Maximum number of work-groups and wavefronts allocated at a time in a compute\n"
	"      unit. These are some of the factors limiting the number of work-groups mapped\n"
	"      to a compute unit.\n"
	"  SchedulingPolicy = {RoundRobin|Greedy} (Default = RoundRobin)\n"
	"      Wavefront scheduling algorithm.\n"
	"      'RoundRobin' selects wavefronts in a cyclic fashion.\n"
	"      'Greedy' selects the most recently used wavefront.\n"
	"\n"
	"Section '[ LocalMemory ]': defines the parameters of the local memory associated to\n"
	"each compute unit.\n"
	"\n"
	"  Size = <bytes> (Default = 32 KB)\n"
	"      Local memory capacity per compute unit. This value must be equal or larger\n"
	"      than BlockSize * Banks. This is one of the factors limiting the number of\n"
	"      work-groups mapped to a compute unit.\n"
	"  AllocSize = <bytes> (Default = 1 KB)\n"
	"      Minimum amount of local memory allocated at a time for each work-group.\n"
	"      This parameter impact on the allocation of work-groups to compute units.\n"
	"  BlockSize = <bytes> (Default = 256)\n"
	"      Access block size, used for access coalescing purposes among work-items.\n"
	"  Latency = <num_cycles> (Default = 2)\n"
	"      Hit latency in number of cycles.\n"
	"  Ports = <num> (Default = 4)\n"
	"\n"
	"Section '[ CFEngine ]': parameters for the CF Engine of the Compute Units.\n"
	"\n"
	"  InstructionMemoryLatency = <cycles> (Default = 2)\n"
	"      Latency of an access to the instruction memory in number of cycles.\n"
	"\n"
	"Section '[ ALUEngine ]': parameters for the ALU Engine of the Compute Units.\n"
	"\n"
	"  InstructionMemoryLatency = <cycles> (Default = 2)\n"
	"      Latency of an access to the instruction memory in number of cycles.\n"
	"  FetchQueueSize = <size> (Default = 64)\n"
	"      Size in bytes of the fetch queue.\n"
	"  ProcessingElementLatency = <cycles> (Default = 4)\n"
	"      Latency of each processing element (x, y, z, w, t) of a Stream Core\n"
	"      in number of cycles. This is the time between an instruction is issued\n"
	"      to a Stream Core and the result of the operation is available.\n"
	"\n"
	"Section '[ TEXEngine ]': parameters for the TEX Engine of the Compute Units.\n"
	"\n"
	"  InstructionMemoryLatency = <cycles> (Default = 2)\n"
	"      Latency of an access to the instruction memory in number of cycles.\n"
	"  FetchQueueSize = <size> (Default = 32)\n"
	"      Size in bytes of the fetch queue.\n"
	"  LoadQueueSize = <size> (Default = 8)\n"
	"      Size of the load queue in number of uops. This size is equal to the\n"
	"      maximum number of load uops in flight.\n"
	"\n";

char *si_gpu_config_file_name = "";
char *si_gpu_report_file_name = "";

int si_trace_category;

/* Default parameters based on the AMD Radeon HD 7970 */
unsigned int si_gpu_device_type = 4; /* CL_DEVICE_TYPE_GPU */
unsigned int si_gpu_device_vendor_id = 1234; /* Completely arbitrary */

char *si_gpu_device_profile = "FULL_PROFILE";
char *si_gpu_device_name = "Multi2Sim Virtual GPU Device";
char *si_gpu_device_vendor = "www.multi2sim.org";
char *si_gpu_device_extensions = "cl_amd_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics "
	"cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "
	"cl_khr_gl_sharing cl_ext_device_fission cl_amd_device_attribute_query cl_amd_media_ops cl_amd_popcnt "
	"cl_amd_printf ";
char *si_gpu_device_version = "OpenCL 1.1 AMD-APP-SDK-v2.6";
char *si_gpu_driver_version = VERSION;
char *si_gpu_opencl_version = "OpenCL C 1.2";

unsigned int si_gpu_num_compute_units = 32;
unsigned int si_gpu_num_wavefront_pools = 4; /* Per CU */
unsigned int si_gpu_num_stream_cores = 16; /* Per SIMD */
unsigned int si_gpu_num_registers = 65536; /* Per SIMD */
unsigned int si_gpu_register_alloc_size = 32;

unsigned int si_gpu_work_item_dimensions = 3;  /* FIXME */
unsigned int si_gpu_work_item_sizes[3] = {256, 256, 256};  /* FIXME */
unsigned int si_gpu_work_group_size = 256 * 256 * 256;  /* FIXME */

unsigned int si_gpu_image_support = 1; /* CL_TRUE */
unsigned int si_gpu_max_read_image_args = 128;  /* The minimum value acceptable */
unsigned int si_gpu_max_write_image_args = 8;  /* The minimum value acceptable */

unsigned int si_gpu_image2d_max_width = 8192;  /* The minimum value acceptable */
unsigned int si_gpu_image2d_max_height = 8192;  /* The minimum value acceptable */
unsigned int si_gpu_image3d_max_width = 2048;  /* The minimum value acceptable */
unsigned int si_gpu_image3d_max_height = 2048;  /* The minimum value acceptable */
unsigned int si_gpu_image3d_max_depth = 2048;  /* The minimum value acceptable */
unsigned int si_gpu_max_samplers = 16;  /* The minimum value acceptable */

unsigned int si_gpu_max_parameter_size = 1024;  /* The minimum value acceptable */
unsigned int si_gpu_mem_base_addr_align = 16 * 8;  /* size of long16 in bits */ /* FIXME */
unsigned int si_gpu_min_data_type_align_size = 16;  /* size of long16 in bytes, deprecated in OpenCL 1.2 */ /* FIXME */

unsigned int si_gpu_single_fp_config = 255;  /* bit field, all single floating point capabilities supported */ /* FIXME */
unsigned int si_gpu_double_fp_config = 255;  /* bit field, all double floating point capabilities supported */ /* FIXME */

unsigned int si_gpu_max_clock_frequency = 850;  /* FIXME */
unsigned int si_gpu_address_bits = 32;

unsigned int si_gpu_global_mem_cache_type = 2;  /* CL_READ_WRITE_CACHE */
unsigned int si_gpu_global_mem_cacheline_size = 256; /* FIXME */
unsigned long long si_gpu_global_mem_cache_size = 1ull << 23;  /* 8MB */ /* FIXME */
unsigned long long si_gpu_global_mem_size = 1ull << 32;  /* 4GB of global memory reported */
unsigned long long si_gpu_max_mem_alloc_size = (1ull << 32) / 4; /* FIXME */

unsigned int si_gpu_local_mem_type = 1;  /* CL_LOCAL */

unsigned long long si_gpu_max_constant_buffer_size = 1ull << 16;  /* 64KB */ /* The minimum value acceptable */
unsigned int si_gpu_max_constant_args = 8;  /* The minimum value acceptable */

unsigned int si_gpu_vector_width_half = 0; /* No support for cl_khr_fp16 */

unsigned int si_gpu_error_correction_support = 1;  /* FIXME */
unsigned int si_gpu_host_unified_memory = 0;  /* FIXME */
unsigned int si_gpu_profiling_timer_resolution = 10;  /* ns */ /* FIXME */
unsigned int si_gpu_endian_little = 1;  /* FIXME */

unsigned int si_gpu_device_available = 1;  /* FIXME */
unsigned int si_gpu_compiler_available = 1;  /* FIXME */
unsigned int si_gpu_linker_available = 1;  /* FIXME */
unsigned int si_gpu_execution_capabilities = 1;  /* bit field, CL_EXEC_KERNEL only */ /* FIXME */

unsigned int si_gpu_queue_properties = 2;  /* bit field, profiling enabled, no out of order execution */

unsigned int si_gpu_platform = 0;  /* FIXME */

struct str_map_t si_gpu_register_alloc_granularity_map =
{
	2, {
		{ "Wavefront", si_gpu_register_alloc_wavefront },
		{ "WorkGroup", si_gpu_register_alloc_work_group }
	}
};
enum si_gpu_register_alloc_granularity_t si_gpu_register_alloc_granularity;

/* Local memory parameters */
int si_gpu_local_mem_size = 65536;  /* 64 KB */
int si_gpu_local_mem_alloc_size = 1024;  /* 1 KB */
int si_gpu_local_mem_latency = 2;
int si_gpu_local_mem_block_size = 64;
int si_gpu_local_mem_num_ports = 2;

struct si_gpu_t *si_gpu;



/*
 * Private Functions
 */

/* Version of Southern Islands trace producer.
 * See 'src/visual/southern-islands/gpu.c' for Southern Islands trace consumer. */

#define SI_TRACE_VERSION_MAJOR		1
#define SI_TRACE_VERSION_MINOR		1


static void si_gpu_device_init()
{
	struct si_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Initialize */
	si_gpu = xcalloc(1, sizeof(struct si_gpu_t));
	si_gpu->trash_uop_list = linked_list_create();

	/* Initialize compute units */
	si_gpu->compute_units = xcalloc(si_gpu_num_compute_units, sizeof(void *));
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		si_gpu->compute_units[compute_unit_id] = si_compute_unit_create();
		compute_unit = si_gpu->compute_units[compute_unit_id];
		compute_unit->id = compute_unit_id;
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	}

	/* Trace */
	si_trace_header("si.init version=\"%d.%d\" num_compute_units=%d\n",
		SI_TRACE_VERSION_MAJOR, SI_TRACE_VERSION_MINOR,
		si_gpu_num_compute_units);
}


static void si_config_read(void)
{
	struct config_t *gpu_config;
	char *section;
	char *err_note =
		"\tPlease run 'm2s --si-help' or consult the Multi2Sim Guide for a\n"
		"\tdescription of the GPU configuration file format.";

	char *gpu_register_alloc_granularity_str;

	/* Load GPU configuration file */
	gpu_config = config_create(si_gpu_config_file_name);
	if (*si_gpu_config_file_name)
		config_load(gpu_config);
	
	/* Device */
	section = "Device";
	si_gpu_num_compute_units = config_read_int(
		gpu_config, section, "NumComputeUnits", si_gpu_num_compute_units);
	if (si_gpu_num_compute_units < 1)
		fatal("%s: invalid value for 'NumComputeUnits'.\n%s", si_gpu_config_file_name,
			err_note);

	si_emu_wavefront_size = config_read_int(
		gpu_config, section, "WavefrontSize", si_emu_wavefront_size);
	if (si_emu_wavefront_size < 1)
		fatal("%s: invalid value for 'WavefrontSize'.\n%s", si_gpu_config_file_name,
			err_note);

	si_gpu_num_stream_cores = config_read_int(
		gpu_config, section, "NumStreamCores", si_gpu_num_stream_cores);
	if (si_gpu_num_stream_cores < 1)
		fatal("%s: invalid value for 'NumStreamCores'.\n%s", si_gpu_config_file_name,
			err_note);

	si_gpu_num_registers = config_read_int(
		gpu_config, section, "NumRegisters", si_gpu_num_registers);
	if (si_gpu_num_registers < 1)
		fatal("%s: invalid value for 'NumRegisters'.\n%s", si_gpu_config_file_name,
			err_note);

	si_gpu_register_alloc_size = config_read_int(
		gpu_config, section, "RegisterAllocSize", si_gpu_register_alloc_size);
	if (si_gpu_register_alloc_size < 1)
		fatal("%s: invalid value for 'RegisterAllocSize'.\n%s", si_gpu_config_file_name,
			err_note);
	if (si_gpu_num_registers % si_gpu_register_alloc_size)
		fatal("%s: 'NumRegisters' must be a multiple of 'RegisterAllocSize'.\n%s",
			si_gpu_config_file_name, err_note);

	gpu_register_alloc_granularity_str = config_read_string(
		gpu_config, section, "RegisterAllocGranularity", "WorkGroup");
	si_gpu_register_alloc_granularity = str_map_string_case(
		&si_gpu_register_alloc_granularity_map, gpu_register_alloc_granularity_str);
	if (si_gpu_register_alloc_granularity == si_gpu_register_alloc_invalid)
		fatal("%s: invalid value for 'RegisterAllocGranularity'.\n%s",
			si_gpu_config_file_name, err_note);


	/* Compute Unit */
	section = "ComputeUnit";
	si_gpu_num_wavefront_pools = config_read_int(
		gpu_config, section, "NumWavefrontPools", si_gpu_num_wavefront_pools);
	if (si_gpu_num_wavefront_pools < 1)
		fatal("%s: invalid value for 'NumWavefrontPools'.\n%s", si_gpu_config_file_name,
			err_note);
	if (si_gpu_num_stream_cores % si_gpu_num_wavefront_pools ||
		si_emu_wavefront_size % (si_gpu_num_stream_cores / si_gpu_num_wavefront_pools))
		fatal("%s: invalid value for 'NumWavefrontPools' or 'NumStreamCores'.\n%s",
			si_gpu_config_file_name, err_note);
	assert(si_emu_wavefront_size % si_gpu_num_stream_cores == 0);
	si_gpu_simd_num_subwavefronts = si_emu_wavefront_size / si_gpu_num_stream_cores;

	si_gpu_max_work_groups_per_wavefront_pool = config_read_int(
		gpu_config, section, "MaxWorkGroupsPerWavefrontPool",
		si_gpu_max_work_groups_per_wavefront_pool);
	if (si_gpu_max_work_groups_per_wavefront_pool < 1)
		fatal("%s: invalid value for 'MaxWorkGroupsPerWavefrontPool'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_max_wavefronts_per_wavefront_pool = config_read_int(
		gpu_config, section, "MaxWavefrontsPerWavefrontPool",
		si_gpu_max_wavefronts_per_wavefront_pool);
	if (si_gpu_max_wavefronts_per_wavefront_pool < 1)
		fatal("%s: invalid value for 'MaxWavefrontsPerWavefrontPool'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_fetch_latency = config_read_int(
		gpu_config, section, "FetchLatency", si_gpu_fetch_latency);
	if (si_gpu_fetch_latency < 1)
		fatal("%s: invalid value for 'FetchLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_fetch_width = config_read_int(
		gpu_config, section, "FetchWidth", si_gpu_fetch_width);
	if (si_gpu_fetch_width < 1)
		fatal("%s: invalid value for 'FetchWidth'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_fetch_buffer_size = config_read_int(
		gpu_config, section, "FetchBufferSize", si_gpu_fetch_buffer_size);
	if (si_gpu_fetch_buffer_size < si_gpu_fetch_width)
		fatal("%s: invalid value for 'FetchBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_decode_latency = config_read_int(
		gpu_config, section, "DecodeLatency", si_gpu_decode_latency);
	if (si_gpu_decode_latency < 1)
		fatal("%s: invalid value for 'DecodeLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_decode_width = config_read_int(
		gpu_config, section, "DecodeWidth", si_gpu_decode_width);
	if (si_gpu_decode_width < 1)
		fatal("%s: invalid value for 'DecodeWidth'.\n%s",
			si_gpu_config_file_name, err_note);

	/* If the decode latency is greater than the number of SIMDs, then the buffer
	 * size must be larger. TODO Add a check for that situation. */
	assert(si_gpu_decode_latency <= si_gpu_num_wavefront_pools);
	si_gpu_decode_buffer_size = config_read_int(
		gpu_config, section, "DecodeBufferSize", si_gpu_decode_buffer_size);
	if (si_gpu_decode_buffer_size < si_gpu_decode_width)
		fatal("%s: invalid value for 'DecodeBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);


	si_gpu_issue_latency = config_read_int(
		gpu_config, section, "IssueLatency", si_gpu_issue_latency);
	if (si_gpu_issue_latency < 1)
		fatal("%s: invalid value for 'IssueLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_issue_width = config_read_int(
		gpu_config, section, "IssueWidth", si_gpu_issue_width);
	if (si_gpu_issue_width < 1)
		fatal("%s: invalid value for 'IssueWidth'.\n%s",
			si_gpu_config_file_name, err_note);

	/* SIMD Unit */
	section = "SIMDUnit";
	si_gpu_simd_width = config_read_int(
		gpu_config, section, "Width", si_gpu_simd_width);
	if (si_gpu_simd_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
			si_gpu_config_file_name, err_note);

	/* If the decode latency is greater than the number of SIMDs, then the buffer
	 * size must be larger. TODO Add a check for that situation. */
	si_gpu_simd_issue_buffer_size = config_read_int(
		gpu_config, section, "IssueBufferSize", si_gpu_simd_issue_buffer_size);
	if (si_gpu_simd_issue_buffer_size < si_gpu_simd_width)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_simd_read_latency = config_read_int(
		gpu_config, section, "ReadLatency", si_gpu_simd_read_latency);
	if (si_gpu_simd_read_latency < 1)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_simd_read_buffer_size = config_read_int(
		gpu_config, section, "ReadBufferSize", si_gpu_simd_read_buffer_size);
	/* Register reads are not pipelined */
	if (si_gpu_simd_read_buffer_size < si_gpu_simd_width)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_simd_alu_latency = config_read_int(
		gpu_config, section, "StreamCoreLatency", si_gpu_simd_alu_latency);
	if (si_gpu_simd_alu_latency < 1)
		fatal("%s: invalid value for 'StreamCoreLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	/* Scalar Unit */
	section = "ScalarUnit";
	si_gpu_scalar_unit_width = config_read_int(
		gpu_config, section, "Width", si_gpu_scalar_unit_width);
	if (si_gpu_scalar_unit_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_scalar_unit_issue_buffer_size = config_read_int(
		gpu_config, section, "IssueBufferSize", si_gpu_scalar_unit_issue_buffer_size);
	if (si_gpu_scalar_unit_issue_buffer_size < si_gpu_scalar_unit_width * si_gpu_issue_latency)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_scalar_unit_read_latency = config_read_int(
		gpu_config, section, "ReadLatency", si_gpu_scalar_unit_read_latency);
	if (si_gpu_scalar_unit_read_latency < 1)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_scalar_unit_read_buffer_size = config_read_int(
		gpu_config, section, "ReadBufferSize", si_gpu_scalar_unit_read_buffer_size);
	/* Register reads are not pipelined */
	if (si_gpu_scalar_unit_read_buffer_size < si_gpu_scalar_unit_width)
			fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
				si_gpu_config_file_name, err_note);

	si_gpu_scalar_unit_exec_latency = config_read_int(
		gpu_config, section, "ALULatency", si_gpu_scalar_unit_exec_latency);
	if (si_gpu_scalar_unit_exec_latency < 1)
		fatal("%s: invalid value for 'ALULatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_scalar_unit_inflight_mem_accesses = config_read_int(
		gpu_config, section, "MaxInflightMem", si_gpu_scalar_unit_inflight_mem_accesses);
	if (si_gpu_scalar_unit_inflight_mem_accesses < 1)
		fatal("%s: invalid value for 'MaxInflightMem'.\n%s",
			si_gpu_config_file_name, err_note);

	/* Branch Unit */
	section = "BranchUnit";
	si_gpu_branch_unit_width = config_read_int(
		gpu_config, section, "Width", si_gpu_branch_unit_width);
	if (si_gpu_branch_unit_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_branch_unit_issue_buffer_size = config_read_int(
		gpu_config, section, "IssueBufferSize", si_gpu_branch_unit_issue_buffer_size);
	if (si_gpu_branch_unit_issue_buffer_size < si_gpu_branch_unit_width * si_gpu_issue_latency)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_branch_unit_read_latency = config_read_int(
		gpu_config, section, "ReadLatency", si_gpu_branch_unit_read_latency);
	if (si_gpu_branch_unit_read_latency < 1)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_branch_unit_read_buffer_size = config_read_int(
		gpu_config, section, "ReadBufferSize", si_gpu_branch_unit_read_buffer_size);
	/* Register reads are not pipelined */
	if (si_gpu_branch_unit_read_buffer_size < si_gpu_branch_unit_width)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_branch_unit_exec_latency = config_read_int(
		gpu_config, section, "BranchLatency", si_gpu_branch_unit_exec_latency);
	if (si_gpu_branch_unit_exec_latency < 1)
		fatal("%s: invalid value for 'BranchLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	/* LDS Unit */
	section = "LDSUnit";
	si_gpu_lds_width = config_read_int(
		gpu_config, section, "Width", si_gpu_lds_width);
	if (si_gpu_lds_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_lds_issue_buffer_size = config_read_int(
		gpu_config, section, "IssueBufferSize", si_gpu_lds_issue_buffer_size);
	if (si_gpu_lds_issue_buffer_size < si_gpu_lds_width * si_gpu_issue_latency)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_lds_read_latency = config_read_int(
		gpu_config, section, "ReadLatency", si_gpu_lds_read_latency);
	if (si_gpu_lds_read_latency < 1)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_lds_read_buffer_size = config_read_int(
		gpu_config, section, "ReadBufferSize", si_gpu_lds_read_buffer_size);
	/* Register reads are not pipelined */
	if (si_gpu_lds_read_buffer_size < si_gpu_lds_width)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_lds_inflight_mem_accesses = config_read_int(
		gpu_config, section, "MaxInflightMem", si_gpu_lds_inflight_mem_accesses);
	if (si_gpu_lds_inflight_mem_accesses < 1)
		fatal("%s: invalid value for 'MaxInflightMem'.\n%s",
			si_gpu_config_file_name, err_note);

	/* VectorMem Unit */
	section = "VectorMemUnit";
	si_gpu_vector_mem_width = config_read_int(
		gpu_config, section, "Width", si_gpu_vector_mem_width);
	if (si_gpu_vector_mem_width < 1)
		fatal("%s: invalid value for 'Width'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_vector_mem_issue_buffer_size = config_read_int(
		gpu_config, section, "IssueBufferSize", si_gpu_vector_mem_issue_buffer_size);
	if (si_gpu_vector_mem_issue_buffer_size < si_gpu_vector_mem_width * si_gpu_issue_latency)
		fatal("%s: invalid value for 'IssueBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_vector_mem_read_latency = config_read_int(
		gpu_config, section, "ReadLatency", si_gpu_vector_mem_read_latency);
	if (si_gpu_vector_mem_read_latency < 1)
		fatal("%s: invalid value for 'ReadLatency'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_vector_mem_read_buffer_size = config_read_int(
		gpu_config, section, "ReadBufferSize", si_gpu_vector_mem_read_buffer_size);
	/* Register reads are not pipelined */
	if (si_gpu_vector_mem_read_buffer_size < si_gpu_vector_mem_width)
		fatal("%s: invalid value for 'ReadBufferSize'.\n%s",
			si_gpu_config_file_name, err_note);

	si_gpu_vector_mem_inflight_mem_accesses = config_read_int(
		gpu_config, section, "MaxInflightMem", si_gpu_vector_mem_inflight_mem_accesses);
	if (si_gpu_vector_mem_inflight_mem_accesses < 1)
		fatal("%s: invalid value for 'MaxInflightMem'.\n%s",
			si_gpu_config_file_name, err_note);
	

	/* Local memory */
	section = "LocalMemory";
	si_gpu_local_mem_size = config_read_int(gpu_config, section, "Size", si_gpu_local_mem_size);
	si_gpu_local_mem_alloc_size = config_read_int(gpu_config, section, "AllocSize", 
		si_gpu_local_mem_alloc_size);
	si_gpu_local_mem_block_size = config_read_int(gpu_config, section, "BlockSize", 
		si_gpu_local_mem_block_size);
	si_gpu_local_mem_latency = config_read_int(gpu_config, section, "Latency", 
		si_gpu_local_mem_latency);
	si_gpu_local_mem_num_ports = config_read_int(gpu_config, section, "Ports", 
		si_gpu_local_mem_num_ports);

	if ((si_gpu_local_mem_size & (si_gpu_local_mem_size - 1)) || si_gpu_local_mem_size < 4)
		fatal("%s: %s->Size must be a power of two and at least 4.\n%s",
			si_gpu_config_file_name, section, err_note);
	if (si_gpu_local_mem_alloc_size < 1)
		fatal("%s: invalid value for %s->Allocsize.\n%s", si_gpu_config_file_name, section,
			err_note);
	if (si_gpu_local_mem_size % si_gpu_local_mem_alloc_size)
		fatal("%s: %s->Size must be a multiple of %s->AllocSize.\n%s", 
			si_gpu_config_file_name, section, section, err_note);
	if ((si_gpu_local_mem_block_size & (si_gpu_local_mem_block_size - 1)) || 
		si_gpu_local_mem_block_size < 4)
		fatal("%s: %s->BlockSize must be a power of two and at least 4.\n%s",
			si_gpu_config_file_name, section, err_note);
	if (si_gpu_local_mem_alloc_size % si_gpu_local_mem_block_size)
		fatal("%s: %s->AllocSize must be a multiple of %s->BlockSize.\n%s", 
			si_gpu_config_file_name, section, section, err_note);
	if (si_gpu_local_mem_latency < 1)
		fatal("%s: invalid value for %s->Latency.\n%s", si_gpu_config_file_name, section, 
			err_note);
	if (si_gpu_local_mem_size < si_gpu_local_mem_block_size)
		fatal("%s: %s->Size cannot be smaller than %s->BlockSize * %s->Banks.\n%s", 
			si_gpu_config_file_name, section, section, section, err_note);
	
	
	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);
}


static void si_config_dump(FILE *f)
{
	/* Device configuration */
	fprintf(f, "[ Config.Device ]\n");
	fprintf(f, "NumComputeUnits = %d\n", si_gpu_num_compute_units);
	fprintf(f, "NumWavefrontPools = %d\n", si_gpu_num_wavefront_pools);
	fprintf(f, "NumStreamCores = %d\n", si_gpu_num_stream_cores);
	fprintf(f, "NumRegisters = %d\n", si_gpu_num_registers);
	fprintf(f, "RegisterAllocSize = %d\n", si_gpu_register_alloc_size);
	fprintf(f, "RegisterAllocGranularity = %s\n", 
		str_map_value(&si_gpu_register_alloc_granularity_map, 
		si_gpu_register_alloc_granularity));
	fprintf(f, "WavefrontSize = %d\n", si_emu_wavefront_size);
	fprintf(f, "MaxWorkGroupsPerWavefrontPool = %d\n", si_gpu_max_work_groups_per_wavefront_pool);
	fprintf(f, "MaxWavefrontsPerWavefrontPool = %d\n", si_gpu_max_wavefronts_per_wavefront_pool);
	fprintf(f, "SIMDALULatency = %d\n", si_gpu_simd_alu_latency);
	fprintf(f, "SIMDWidth = %d\n", si_gpu_simd_width);
	fprintf(f, "ScalarUnitExecLatency = %d\n", si_gpu_scalar_unit_exec_latency);
	fprintf(f, "ScalarUnitWidth = %d\n", si_gpu_scalar_unit_width);
	fprintf(f, "BranchUnitExecLatency = %d\n", si_gpu_branch_unit_exec_latency);
	fprintf(f, "BranchUnitWidth = %d\n", si_gpu_branch_unit_width);
	fprintf(f, "\n");

	/* Local Memory */
	fprintf(f, "[ Config.LocalMemory ]\n");
	fprintf(f, "Size = %d\n", si_gpu_local_mem_size);
	fprintf(f, "AllocSize = %d\n", si_gpu_local_mem_alloc_size);
	fprintf(f, "BlockSize = %d\n", si_gpu_local_mem_block_size);
	fprintf(f, "Latency = %d\n", si_gpu_local_mem_latency);
	fprintf(f, "Ports = %d\n", si_gpu_local_mem_num_ports);
	fprintf(f, "\n");

	/* End of configuration */
	fprintf(f, "\n");
}


static void si_gpu_map_ndrange(struct si_ndrange_t *ndrange)
{
	int compute_unit_id;

	/* Assign current ND-Range */
	assert(!si_gpu->ndrange);
	si_gpu->ndrange = ndrange;

	/* Check that at least one work-group can be allocated per Wavefront Pool */
	si_gpu->work_groups_per_wavefront_pool = si_calc_get_work_groups_per_wavefront_pool(
		ndrange->kernel->local_size, 
		ndrange->kernel->bin_file->enc_dict_entry_southern_islands->num_gpr_used,
		ndrange->local_mem_top);
	if (!si_gpu->work_groups_per_wavefront_pool)
		fatal("work-group resources cannot be allocated to a compute unit.\n"
			"\tA compute unit in the GPU has a limit in number of wavefronts, number\n"
			"\tof registers, and amount of local memory. If the work-group size\n"
			"\texceeds any of these limits, the ND-Range cannot be executed.\n");

	/* Calculate limit of wavefronts and work-items per Wavefront Pool */
	si_gpu->wavefronts_per_wavefront_pool = si_gpu->work_groups_per_wavefront_pool * 
		ndrange->wavefronts_per_work_group;
	si_gpu->work_items_per_wavefront_pool = si_gpu->wavefronts_per_wavefront_pool * 
		si_emu_wavefront_size;

	/* Calculate limit of work groups, wavefronts and work-items per compute unit */
	si_gpu->work_groups_per_compute_unit = si_gpu->work_groups_per_wavefront_pool * 
		si_gpu_num_wavefront_pools;
	si_gpu->wavefronts_per_compute_unit = si_gpu->wavefronts_per_wavefront_pool * 
		si_gpu_num_wavefront_pools;
	si_gpu->work_items_per_compute_unit = si_gpu->work_items_per_wavefront_pool * 
		si_gpu_num_wavefront_pools;
	assert(si_gpu->work_groups_per_wavefront_pool <= si_gpu_max_work_groups_per_wavefront_pool);
	assert(si_gpu->wavefronts_per_wavefront_pool <= si_gpu_max_wavefronts_per_wavefront_pool);

	/* Reset architectural state */
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		// FIXME
		//compute_unit = si_gpu->compute_units[compute_unit_id];
		//compute_unit->simd.decode_index = 0;
		//compute_unit->simd.execute_index = 0;
	}
}


static void si_gpu_unmap_ndrange(void)
{
	/* Dump stats */
	si_ndrange_dump(si_gpu->ndrange, si_emu_report_file);

	/* Unmap */
	si_gpu->ndrange = NULL;
}




/*
 * Public Functions
 */

void si_gpu_init(void)
{
	/* Trace */
	si_trace_category = trace_new_category();

	/* Register functions for architecture */
	si_emu_arch->mem_config_check_func = si_mem_config_check;
	si_emu_arch->mem_config_default_func = si_mem_config_default;
	si_emu_arch->mem_config_parse_entry_func = si_mem_config_parse_entry;

	/* Try to open report file */
	if (si_gpu_report_file_name[0] && !file_can_open_for_write(si_gpu_report_file_name))
		fatal("%s: cannot open GPU pipeline report file",
			si_gpu_report_file_name);

	/* Read configuration file */
	si_config_read();

	/* Initializations */
	si_gpu_device_init();
	si_uop_init();
}


void si_gpu_done()
{
	struct si_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* GPU pipeline report */
	si_gpu_dump_report();

	/* Free stream cores, compute units, and device */
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = si_gpu->compute_units[compute_unit_id];
		si_compute_unit_free(compute_unit);
	}
	free(si_gpu->compute_units);

	/* List of removed instructions */
	si_gpu_uop_trash_empty();
	linked_list_free(si_gpu->trash_uop_list);

	/* Free GPU */
	free(si_gpu);

	/* Finalizations */
	si_uop_done();
}


void si_gpu_dump_report(void)
{
	struct si_compute_unit_t *compute_unit;
	struct mod_t *local_mod;
	int compute_unit_id;

	FILE *f;

	double inst_per_cycle;

	long long coalesced_reads;
	long long coalesced_writes;

	/* Open file */
	f = file_open_for_write(si_gpu_report_file_name);
	if (!f)
		return;

	/* Dump GPU configuration */
	fprintf(f, ";\n; GPU Configuration\n;\n\n");
	si_config_dump(f);

	/* Report for device */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	inst_per_cycle = si_gpu->cycle ? (double) si_emu->inst_count / si_gpu->cycle : 0.0;
	fprintf(f, "[ Device ]\n\n");
	fprintf(f, "NDRangeCount = %d\n", si_emu->ndrange_count);
	fprintf(f, "Instructions = %lld\n", si_emu->inst_count);
	fprintf(f, "Scalar ALU Instructions = %lld\n", si_emu->scalar_alu_inst_count);
	fprintf(f, "Scalar Mem Instructions = %lld\n", si_emu->scalar_mem_inst_count);
	fprintf(f, "Branch Instructions = %lld\n", si_emu->branch_inst_count);
	fprintf(f, "Vector ALU Instructions = %lld\n", si_emu->vector_alu_inst_count);
	fprintf(f, "Local Memory Instructions = %lld\n", si_emu->local_mem_inst_count);
	fprintf(f, "Vector Mem Instructions = %lld\n", si_emu->vector_mem_inst_count);
	fprintf(f, "Cycles = %lld\n", si_gpu->cycle);
	fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "\n\n");

	/* Report for compute units */
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = si_gpu->compute_units[compute_unit_id];
		local_mod = compute_unit->local_memory;

		inst_per_cycle = compute_unit->cycle ? (double) compute_unit->inst_count
			/ compute_unit->cycle : 0.0;
		coalesced_reads = local_mod->reads - local_mod->effective_reads;
		coalesced_writes = local_mod->writes - local_mod->effective_writes;

		fprintf(f, "[ ComputeUnit %d ]\n\n", compute_unit_id);

		fprintf(f, "WorkGroupCount = %lld\n", compute_unit->mapped_work_groups);
		fprintf(f, "Instructions = %lld\n", compute_unit->inst_count);
		fprintf(f, "Scalar ALU Instructions = %lld\n", compute_unit->scalar_alu_inst_count);
		fprintf(f, "Scalar Mem Instructions = %lld\n", compute_unit->scalar_mem_inst_count);
		fprintf(f, "Branch Instructions = %lld\n", compute_unit->branch_inst_count);
		fprintf(f, "SIMD Instructions = %lld\n", compute_unit->simd_inst_count);
		fprintf(f, "Vector Mem Instructions = %lld\n", compute_unit->vector_mem_inst_count);
		fprintf(f, "Local Mem Instructions = %lld\n", compute_unit->local_mem_inst_count);
		fprintf(f, "Cycles = %lld\n", compute_unit->cycle);
		fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "LocalMemory.Accesses = %lld\n", local_mod->reads + local_mod->writes);
		fprintf(f, "LocalMemory.Reads = %lld\n", local_mod->reads);
		fprintf(f, "LocalMemory.EffectiveReads = %lld\n", local_mod->effective_reads);
		fprintf(f, "LocalMemory.CoalescedReads = %lld\n", coalesced_reads);
		fprintf(f, "LocalMemory.Writes = %lld\n", local_mod->writes);
		fprintf(f, "LocalMemory.EffectiveWrites = %lld\n", local_mod->effective_writes);
		fprintf(f, "LocalMemory.CoalescedWrites = %lld\n", coalesced_writes);
		fprintf(f, "\n\n");
	}
}

void si_gpu_dump_summary(FILE *f)
{
	double time_in_sec;
	double inst_per_cycle;
	double cycles_per_sec;

	/* Calculate statistics */
	time_in_sec = (double) m2s_timer_get_value(si_emu->timer) / 1.0e6;
	inst_per_cycle = si_gpu->cycle ? (double) si_emu->inst_count / si_gpu->cycle : 0.0;
	cycles_per_sec = time_in_sec > 0.0 ? (double) si_gpu->cycle / time_in_sec : 0.0;

	/* Print statistics */
	fprintf(f, "Cycles = %lld\n", si_gpu->cycle);
	//fprintf(f, "Simulated time at 880MHz: %f ms\n", si_gpu->cycle/880000.0);
	//fprintf(f, "Simulated time at 1000MHz: %f ms\n", si_gpu->cycle/1000000.0);
	fprintf(f, "CyclesPerSecond = %.0f\n", cycles_per_sec);
	fprintf(f, "IPC = %.4g\n", inst_per_cycle);
}

void si_gpu_uop_trash_empty(void)
{
	struct si_uop_t *uop;

	while (si_gpu->trash_uop_list->count)
	{
		linked_list_head(si_gpu->trash_uop_list);
		uop = linked_list_get(si_gpu->trash_uop_list);
		linked_list_remove(si_gpu->trash_uop_list);

		si_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_compute_unit, 
			uop->compute_unit->id);

		si_uop_free(uop);
	}
}


void si_gpu_uop_trash_add(struct si_uop_t *uop)
{
	linked_list_add(si_gpu->trash_uop_list, uop);
}


/* Run one iteration of the Southern Islands GPU timing simulation loop. */
int si_gpu_run(void)
{
	struct si_ndrange_t *ndrange;

	struct si_compute_unit_t *compute_unit;
	struct si_compute_unit_t *compute_unit_next;

	/* For efficiency when no Southern Islands emulation is selected, exit here
	 * if the list of existing ND-Ranges is empty. */
	if (!si_emu->ndrange_list_count)
		return 0;

	/* Start one ND-Range in state 'pending' */
	while ((ndrange = si_emu->pending_ndrange_list_head))
	{
		/* Currently not supported for more than 1 ND-Range */
		if (si_gpu->ndrange)
			fatal("%s: Southern Islands GPU timing simulation not supported "
				"for multiple ND-Ranges", __FUNCTION__);

		/* Set ND-Range status to 'running' */
		si_ndrange_clear_status(ndrange, si_ndrange_pending);
		si_ndrange_set_status(ndrange, si_ndrange_running);

		/* Trace */
		si_trace("si.new_ndrange id=%d wg_first=%d wg_count=%d\n", ndrange->id,
			ndrange->work_group_id_first, ndrange->work_group_count);

		/* Map ND-Range to GPU */
		si_gpu_map_ndrange(ndrange);
		si_calc_plot();
	}

	/* Mapped ND-Range */
	ndrange = si_gpu->ndrange;
	assert(ndrange);

	/* Allocate work-groups to compute units */
	while (si_gpu->compute_unit_ready_list_head && ndrange->pending_list_head)
		si_compute_unit_map_work_group(si_gpu->compute_unit_ready_list_head,
			ndrange->pending_list_head);

	/* One more cycle */
	si_gpu->cycle++;

	/* Stop if maximum number of GPU cycles exceeded */
	if (si_emu_max_cycles && si_gpu->cycle >= si_emu_max_cycles)
		esim_finish = esim_finish_si_max_cycles;

	/* Stop if maximum number of GPU instructions exceeded */
	if (si_emu_max_inst && si_emu->inst_count >= si_emu_max_inst)
		esim_finish = esim_finish_si_max_inst;

	/* Stop if there was a simulation stall */
	if (esim_cycle - si_gpu->last_complete_cycle > 1000000)
	{
		warning("Southern Islands GPU simulation stalled.\n%s", si_err_stall);
		esim_finish = esim_finish_stall;
	}

	/* Stop if any reason met */
	if (esim_finish)
		return 1;

	/* Free instructions in trash */
	si_gpu_uop_trash_empty();

	/* Run one loop iteration on each busy compute unit */
	for (compute_unit = si_gpu->compute_unit_busy_list_head; compute_unit;
		compute_unit = compute_unit_next)
	{
		/* Store next busy compute unit, since this can change
		 * during the compute unit simulation loop iteration. */
		compute_unit_next = compute_unit->compute_unit_busy_list_next;

		/* Run one cycle */
		si_compute_unit_run(compute_unit);
	}

	/* If ND-Range finished execution in all compute units, free it. */
	if (!si_gpu->compute_unit_busy_list_count)
	{
		/* Dump ND-Range report */
		si_ndrange_dump(ndrange, si_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (si_emu_max_kernels && si_emu->ndrange_count >= si_emu_max_kernels)
			esim_finish = esim_finish_si_max_kernels;

		/* Finalize and free ND-Range */
		assert(si_ndrange_get_status(ndrange, si_ndrange_finished));
		si_gpu_uop_trash_empty();
		si_gpu_unmap_ndrange();
		si_ndrange_free(ndrange);
	}

	/* Return true */
	return 1;
}
