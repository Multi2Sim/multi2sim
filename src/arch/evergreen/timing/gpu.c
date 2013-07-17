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


#include <arch/evergreen/emu/emu.h>
#include <arch/evergreen/emu/ndrange.h>
#include <driver/opencl-old/evergreen/bin-file.h>
#include <driver/opencl-old/evergreen/kernel.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>

#include "calc.h"
#include "compute-unit.h"
#include "cycle-interval-report.h"
#include "faults.h"
#include "gpu.h"
#include "instruction-interval-report.h"
#include "mem-config.h"
#include "sched.h"



/*
 * Global variables
 */

char *evg_gpu_config_help =
	"The Evergreen GPU configuration file is a plain text INI file defining\n"
	"the parameters of the GPU model for a detailed (architectural) GPU configuration.\n"
	"This file is passed to Multi2Sim with the '--evg-config <file>' option, and\n"
	"should always be used together with option '--evg-sim detailed'.\n"
	"\n"
	"The following is a list of the sections allowed in the GPU configuration file,\n"
	"along with the list of variables for each section.\n"
	"\n"
	"Section '[ Device ]': parameters for the GPU.\n"
	"\n"
	"  Frequency = <value> (Default = 700)\n"
	"      Frequency domain for the Evergreen GPU in MHz.\n"
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
	"      Number of work-items in a wavefront, executing AMD Evergreen instructions in\n"
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

char *evg_gpu_config_file_name = "";
char *evg_gpu_report_file_name = "";

int evg_trace_category;

/* Default parameters based on the AMD Radeon HD 5870 */
int evg_gpu_num_compute_units = 20;
int evg_gpu_num_stream_cores = 16;
int evg_gpu_num_registers = 16384;
int evg_gpu_register_alloc_size = 32;

int evg_gpu_frequency = 700;

struct str_map_t evg_gpu_register_alloc_granularity_map =
{
	2, {
		{ "Wavefront", evg_gpu_register_alloc_wavefront },
		{ "WorkGroup", evg_gpu_register_alloc_work_group }
	}
};
enum evg_gpu_register_alloc_granularity_t evg_gpu_register_alloc_granularity;

int evg_gpu_max_work_groups_per_compute_unit = 8;
int evg_gpu_max_wavefronts_per_compute_unit = 32;

/* Local memory parameters */
int evg_gpu_local_mem_size = 32768;  /* 32 KB */
int evg_gpu_local_mem_alloc_size = 1024;  /* 1 KB */
int evg_gpu_local_mem_latency = 2;
int evg_gpu_local_mem_block_size = 256;
int evg_gpu_local_mem_num_ports = 2;

EvgGpu *evg_gpu;




/*
 * Private Functions
 */

static char *evg_err_stall =
	"\tThe Evergreen GPU has not completed execution of any in-flight\n"
	"\tinstruction for 1M cycles. Most likely, this means that a\n"
	"\tdeadlock condition occurred in the management of some modeled\n"
	"\tstructure (network, memory system, pipeline queues, etc.).\n";

/* Version of Evergreen trace producer.
 * See 'src/visual/evergreen/gpu.c' for Evergreen trace consumer. */

#define EVG_TRACE_VERSION_MAJOR		1
#define EVG_TRACE_VERSION_MINOR		671


static void evg_config_dump(FILE *f)
{
	/* Device configuration */
	fprintf(f, "[ Config.Device ]\n");
	fprintf(f, "Frequency = %d\n", evg_gpu_frequency);
	fprintf(f, "NumComputeUnits = %d\n", evg_gpu_num_compute_units);
	fprintf(f, "NumStreamCores = %d\n", evg_gpu_num_stream_cores);
	fprintf(f, "NumRegisters = %d\n", evg_gpu_num_registers);
	fprintf(f, "RegisterAllocSize = %d\n", evg_gpu_register_alloc_size);
	fprintf(f, "RegisterAllocGranularity = %s\n", str_map_value(&evg_gpu_register_alloc_granularity_map, evg_gpu_register_alloc_granularity));
	fprintf(f, "WavefrontSize = %d\n", evg_emu_wavefront_size);
	fprintf(f, "MaxWorkGroupsPerComputeUnit = %d\n", evg_gpu_max_work_groups_per_compute_unit);
	fprintf(f, "MaxWavefrontsPerComputeUnit = %d\n", evg_gpu_max_wavefronts_per_compute_unit);
	fprintf(f, "SchedulingPolicy = %s\n", str_map_value(&evg_gpu_sched_policy_map, evg_gpu_sched_policy));
	fprintf(f, "\n");

	/* Local Memory */
	fprintf(f, "[ Config.LocalMemory ]\n");
	fprintf(f, "Size = %d\n", evg_gpu_local_mem_size);
	fprintf(f, "AllocSize = %d\n", evg_gpu_local_mem_alloc_size);
	fprintf(f, "BlockSize = %d\n", evg_gpu_local_mem_block_size);
	fprintf(f, "Latency = %d\n", evg_gpu_local_mem_latency);
	fprintf(f, "Ports = %d\n", evg_gpu_local_mem_num_ports);
	fprintf(f, "\n");

	/* CF Engine */
	fprintf(f, "[ Config.CFEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", evg_gpu_cf_engine_inst_mem_latency);
	fprintf(f, "\n");

	/* ALU Engine */
	fprintf(f, "[ Config.ALUEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", evg_gpu_alu_engine_inst_mem_latency);
	fprintf(f, "FetchQueueSize = %d\n", evg_gpu_alu_engine_fetch_queue_size);
	fprintf(f, "ProcessingElementLatency = %d\n", evg_gpu_alu_engine_pe_latency);
	fprintf(f, "\n");

	/* TEX Engine */
	fprintf(f, "[ Config.TEXEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", evg_gpu_tex_engine_inst_mem_latency);
	fprintf(f, "FetchQueueSize = %d\n", evg_gpu_tex_engine_fetch_queue_size);
	fprintf(f, "LoadQueueSize = %d\n", evg_gpu_tex_engine_load_queue_size);
	fprintf(f, "\n");

	/* End of configuration */
	fprintf(f, "\n");
}


static void evg_gpu_map_ndrange(struct evg_ndrange_t *ndrange)
{
	struct evg_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Assign current ND-Range */
	assert(!evg_gpu->ndrange);
	evg_gpu->ndrange = ndrange;

	/* Check that at least one work-group can be allocated per compute unit */
	evg_gpu->work_groups_per_compute_unit = evg_calc_get_work_groups_per_compute_unit(
		ndrange->kernel->local_size, ndrange->kernel->bin_file->enc_dict_entry_evergreen->num_gpr_used,
		ndrange->local_mem_top);
	if (!evg_gpu->work_groups_per_compute_unit)
		fatal("work-group resources cannot be allocated to a compute unit.\n"
			"\tA compute unit in the GPU has a limit in number of wavefronts, number\n"
			"\tof registers, and amount of local memory. If the work-group size\n"
			"\texceeds any of these limits, the ND-Range cannot be executed.\n");

	/* Derived from this, calculate limit of wavefronts and work-items per compute unit. */
	evg_gpu->wavefronts_per_compute_unit = evg_gpu->work_groups_per_compute_unit * ndrange->wavefronts_per_work_group;
	evg_gpu->work_items_per_compute_unit = evg_gpu->wavefronts_per_compute_unit * evg_emu_wavefront_size;
	assert(evg_gpu->work_groups_per_compute_unit <= evg_gpu_max_work_groups_per_compute_unit);
	assert(evg_gpu->wavefronts_per_compute_unit <= evg_gpu_max_wavefronts_per_compute_unit);

	/* Reset architectural state */
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = evg_gpu->compute_units[compute_unit_id];
		compute_unit->cf_engine.decode_index = 0;
		compute_unit->cf_engine.execute_index = 0;
	}
}


static void evg_gpu_unmap_ndrange(void)
{
	/* Dump stats */
	evg_ndrange_dump(evg_gpu->ndrange, evg_emu_report_file);

	/* Unmap */
	evg_gpu->ndrange = NULL;
}




/*
 * Public Functions
 */

void evg_gpu_read_config(void)
{
	struct config_t *gpu_config;
	char *section;
	char *err_note =
		"\tPlease run 'm2s --evg-help' or consult the Multi2Sim Guide for a\n"
		"\tdescription of the GPU configuration file format.";

	char *gpu_register_alloc_granularity_str;
	char *gpu_sched_policy_str;

	/* Load GPU configuration file */
	gpu_config = config_create(evg_gpu_config_file_name);
	if (*evg_gpu_config_file_name)
		config_load(gpu_config);
	
	/* Device */
	section = "Device";

	/* Frequency */
	evg_gpu_frequency = config_read_int(gpu_config, section, "Frequency", evg_gpu_frequency);
	if (!IN_RANGE(evg_gpu_frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: invalid value for 'Frequency'.\n%s",
				evg_gpu_config_file_name, err_note);

	evg_gpu_num_compute_units = config_read_int(gpu_config, section, "NumComputeUnits", evg_gpu_num_compute_units);
	evg_gpu_num_stream_cores = config_read_int(gpu_config, section, "NumStreamCores", evg_gpu_num_stream_cores);
	evg_gpu_num_registers = config_read_int(gpu_config, section, "NumRegisters", evg_gpu_num_registers);
	evg_gpu_register_alloc_size = config_read_int(gpu_config, section, "RegisterAllocSize", evg_gpu_register_alloc_size);
	gpu_register_alloc_granularity_str = config_read_string(gpu_config, section, "RegisterAllocGranularity", "WorkGroup");
	evg_emu_wavefront_size = config_read_int(gpu_config, section, "WavefrontSize", evg_emu_wavefront_size);
	evg_gpu_max_work_groups_per_compute_unit = config_read_int(gpu_config, section, "MaxWorkGroupsPerComputeUnit",
		evg_gpu_max_work_groups_per_compute_unit);
	evg_gpu_max_wavefronts_per_compute_unit = config_read_int(gpu_config, section, "MaxWavefrontsPerComputeUnit",
		evg_gpu_max_wavefronts_per_compute_unit);
	gpu_sched_policy_str = config_read_string(gpu_config, section, "SchedulingPolicy", "RoundRobin");
	if (evg_gpu_num_compute_units < 1)
		fatal("%s: invalid value for 'NumComputeUnits'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_num_stream_cores < 1)
		fatal("%s: invalid value for 'NumStreamCores'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_register_alloc_size < 1)
		fatal("%s: invalid value for 'RegisterAllocSize'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_num_registers < 1)
		fatal("%s: invalid value for 'NumRegisters'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_num_registers % evg_gpu_register_alloc_size)
		fatal("%s: 'NumRegisters' must be a multiple of 'RegisterAllocSize'.\n%s", evg_gpu_config_file_name, err_note);

	evg_gpu_register_alloc_granularity = str_map_string_case(&evg_gpu_register_alloc_granularity_map, gpu_register_alloc_granularity_str);
	if (evg_gpu_register_alloc_granularity == evg_gpu_register_alloc_invalid)
		fatal("%s: invalid value for 'RegisterAllocGranularity'.\n%s", evg_gpu_config_file_name, err_note);

	evg_gpu_sched_policy = str_map_string_case(&evg_gpu_sched_policy_map, gpu_sched_policy_str);
	if (evg_gpu_sched_policy == evg_gpu_sched_invalid)
		fatal("%s: invalid value for 'SchedulingPolicy'.\n%s", evg_gpu_config_file_name, err_note);

	if (evg_emu_wavefront_size < 1)
		fatal("%s: invalid value for 'WavefrontSize'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_max_work_groups_per_compute_unit < 1)
		fatal("%s: invalid value for 'MaxWorkGroupsPerComputeUnit'.\n%s", evg_gpu_config_file_name, err_note);
	if (evg_gpu_max_wavefronts_per_compute_unit < 1)
		fatal("%s: invalid value for 'MaxWavefrontsPerComputeUnit'.\n%s", evg_gpu_config_file_name, err_note);
	
	/* Local memory */
	section = "LocalMemory";
	evg_gpu_local_mem_size = config_read_int(gpu_config, section, "Size", evg_gpu_local_mem_size);
	evg_gpu_local_mem_alloc_size = config_read_int(gpu_config, section, "AllocSize", evg_gpu_local_mem_alloc_size);
	evg_gpu_local_mem_block_size = config_read_int(gpu_config, section, "BlockSize", evg_gpu_local_mem_block_size);
	evg_gpu_local_mem_latency = config_read_int(gpu_config, section, "Latency", evg_gpu_local_mem_latency);
	evg_gpu_local_mem_num_ports = config_read_int(gpu_config, section, "Ports", evg_gpu_local_mem_num_ports);
	if ((evg_gpu_local_mem_size & (evg_gpu_local_mem_size - 1)) || evg_gpu_local_mem_size < 4)
		fatal("%s: %s->Size must be a power of two and at least 4.\n%s",
			evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_local_mem_alloc_size < 1)
		fatal("%s: invalid value for %s->Allocsize.\n%s", evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_local_mem_size % evg_gpu_local_mem_alloc_size)
		fatal("%s: %s->Size must be a multiple of %s->AllocSize.\n%s", evg_gpu_config_file_name,
			section, section, err_note);
	if ((evg_gpu_local_mem_block_size & (evg_gpu_local_mem_block_size - 1)) || evg_gpu_local_mem_block_size < 4)
		fatal("%s: %s->BlockSize must be a power of two and at least 4.\n%s",
			evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_local_mem_alloc_size % evg_gpu_local_mem_block_size)
		fatal("%s: %s->AllocSize must be a multiple of %s->BlockSize.\n%s", evg_gpu_config_file_name,
			section, section, err_note);
	if (evg_gpu_local_mem_latency < 1)
		fatal("%s: invalid value for %s->Latency.\n%s", evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_local_mem_size < evg_gpu_local_mem_block_size)
		fatal("%s: %s->Size cannot be smaller than %s->BlockSize * %s->Banks.\n%s", evg_gpu_config_file_name,
			section, section, section, err_note);
	
	/* CF Engine */
	section = "CFEngine";
	evg_gpu_cf_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		evg_gpu_cf_engine_inst_mem_latency);
	if (evg_gpu_cf_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", evg_gpu_config_file_name, section, err_note);
	
	/* ALU Engine */
	section = "ALUEngine";
	evg_gpu_alu_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		evg_gpu_alu_engine_inst_mem_latency);
	evg_gpu_alu_engine_fetch_queue_size = config_read_int(gpu_config, section, "FetchQueueSize",
		evg_gpu_alu_engine_fetch_queue_size);
	evg_gpu_alu_engine_pe_latency = config_read_int(gpu_config, section, "ProcessingElementLatency",
		evg_gpu_alu_engine_pe_latency);
	if (evg_gpu_alu_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_alu_engine_fetch_queue_size < 56)
		fatal("%s: the minimum value for %s->FetchQueueSize is 56.\n"
			"This is the maximum size of one VLIW bundle, including 5 ALU instructions\n"
			"(2 words each), and 4 literal constants (1 word each).\n%s",
			evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_alu_engine_pe_latency < 1)
		fatal("%s: invalid value for %s->ProcessingElementLatency.\n%s", evg_gpu_config_file_name, section, err_note);

	/* TEX Engine */
	section = "TEXEngine";
	evg_gpu_tex_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		evg_gpu_tex_engine_inst_mem_latency);
	evg_gpu_tex_engine_fetch_queue_size = config_read_int(gpu_config, section, "FetchQueueSize",
		evg_gpu_tex_engine_fetch_queue_size);
	evg_gpu_tex_engine_load_queue_size = config_read_int(gpu_config, section, "LoadQueueSize",
		evg_gpu_tex_engine_load_queue_size);
	if (evg_gpu_tex_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s.InstructionMemoryLatency.\n%s", evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_tex_engine_fetch_queue_size < 16)
		fatal("%s: the minimum value for %s.FetchQueueSize is 16.\n"
			"This size corresponds to the 4 words comprising a TEX Evergreen instruction.\n%s",
			evg_gpu_config_file_name, section, err_note);
	if (evg_gpu_tex_engine_load_queue_size < 1)
		fatal("%s: the minimum value for %s.LoadQueueSize is 1.\n%s",
			evg_gpu_config_file_name, section, err_note);
	
	/* Periodic report */
	evg_periodic_report_config_read(gpu_config);
	evg_spatial_report_config_read(gpu_config);

	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);
}


void evg_gpu_init(void)
{
	/* Classes */
	CLASS_REGISTER(EvgGpu);

	/* Trace */
	evg_trace_category = trace_new_category();

	/* Try to open report file */
	if (evg_gpu_report_file_name[0] && !file_can_open_for_write(evg_gpu_report_file_name))
		fatal("%s: cannot open GPU pipeline report file",
			evg_gpu_report_file_name);

	/* Trace */
	evg_trace_header("evg.init version=\"%d.%d\" num_compute_units=%d\n",
		EVG_TRACE_VERSION_MAJOR, EVG_TRACE_VERSION_MINOR,
		evg_gpu_num_compute_units);

	/* Create GPU */
	evg_gpu = new(EvgGpu);

	/* Initializations */
	evg_periodic_report_init();
	evg_uop_init();

	/* GPU-REL: read stack faults file */
	evg_faults_init();
}


void evg_gpu_done(void)
{
	/* GPU pipeline report */
	evg_gpu_dump_report();

	/* List of removed instructions */
	evg_gpu_uop_trash_empty();
	linked_list_free(evg_gpu->trash_uop_list);

	/* Finalizations */
	evg_uop_done();
	evg_periodic_report_done();

	/* Free GPU */
	delete(evg_gpu);

	/* Spatial report */
	if (evg_spatial_report_active)
		evg_cu_spatial_report_done();

	/* GPU-REL: read stack faults file */
	evg_faults_done();
}


void evg_gpu_dump_report(void)
{
	struct evg_compute_unit_t *compute_unit;
	struct mod_t *local_mod;
	int compute_unit_id;

	FILE *f;

	double inst_per_cycle;
	double cf_inst_per_cycle;
	double alu_inst_per_cycle;
	double tex_inst_per_cycle;

	long long coalesced_reads;
	long long coalesced_writes;

	char vliw_occupancy[MAX_STRING_SIZE];

	/* Open file */
	f = file_open_for_write(evg_gpu_report_file_name);
	if (!f)
		return;

	/* Dump GPU configuration */
	fprintf(f, ";\n; GPU Configuration\n;\n\n");
	evg_config_dump(f);

	/* Report for device */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	inst_per_cycle = asTiming(evg_gpu)->cycle ? (double) asEmu(evg_emu)->instructions
			/ asTiming(evg_gpu)->cycle : 0.0;
	fprintf(f, "[ Device ]\n\n");
	fprintf(f, "NDRangeCount = %d\n", evg_emu->ndrange_count);
	fprintf(f, "Instructions = %lld\n", asEmu(evg_emu)->instructions);
	fprintf(f, "Cycles = %lld\n", asTiming(evg_gpu)->cycle);
	fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "\n\n");

	/* Report for compute units */
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = evg_gpu->compute_units[compute_unit_id];
		local_mod = compute_unit->local_memory;

		inst_per_cycle = compute_unit->cycle ? (double) compute_unit->inst_count
			/ compute_unit->cycle : 0.0;
		cf_inst_per_cycle = compute_unit->cycle ? (double) compute_unit->cf_engine.inst_count
			/ compute_unit->cycle : 0.0;
		alu_inst_per_cycle = compute_unit->alu_engine.cycle ? (double) compute_unit->alu_engine.inst_count
			/ compute_unit->alu_engine.cycle : 0.0;
		tex_inst_per_cycle = compute_unit->tex_engine.cycle ? (double) compute_unit->tex_engine.inst_count
			/ compute_unit->tex_engine.cycle : 0.0;
		coalesced_reads = local_mod->reads - local_mod->effective_reads;
		coalesced_writes = local_mod->writes - local_mod->effective_writes;
		snprintf(vliw_occupancy, MAX_STRING_SIZE, "%lld %lld %lld %lld %lld",
			compute_unit->alu_engine.vliw_slots[0],
			compute_unit->alu_engine.vliw_slots[1],
			compute_unit->alu_engine.vliw_slots[2],
			compute_unit->alu_engine.vliw_slots[3],
			compute_unit->alu_engine.vliw_slots[4]);

		fprintf(f, "[ ComputeUnit %d ]\n\n", compute_unit_id);

		fprintf(f, "WorkGroupCount = %lld\n", compute_unit->mapped_work_groups);
		fprintf(f, "Instructions = %lld\n", compute_unit->inst_count);
		fprintf(f, "Cycles = %lld\n", compute_unit->cycle);
		fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "CFEngine.Instructions = %lld\n", compute_unit->cf_engine.inst_count);
		fprintf(f, "CFEngine.InstructionsPerCycle = %.4g\n", cf_inst_per_cycle);
		fprintf(f, "CFEngine.ALUClauseTriggers = %lld\n", compute_unit->cf_engine.alu_clause_trigger_count);
		fprintf(f, "CFEngine.TEXClauseTriggers = %lld\n", compute_unit->cf_engine.tex_clause_trigger_count);
		fprintf(f, "CFEngine.GlobalMemWrites = %lld\n", compute_unit->cf_engine.global_mem_write_count);
		fprintf(f, "\n");

		fprintf(f, "ALUEngine.WavefrontCount = %lld\n", compute_unit->alu_engine.wavefront_count);
		fprintf(f, "ALUEngine.Instructions = %lld\n", compute_unit->alu_engine.inst_count);
		fprintf(f, "ALUEngine.InstructionSlots = %lld\n", compute_unit->alu_engine.inst_slot_count);
		fprintf(f, "ALUEngine.LocalMemorySlots = %lld\n", compute_unit->alu_engine.local_mem_slot_count);
		fprintf(f, "ALUEngine.VLIWOccupancy = %s\n", vliw_occupancy);
		fprintf(f, "ALUEngine.Cycles = %lld\n", compute_unit->alu_engine.cycle);
		fprintf(f, "ALUEngine.InstructionsPerCycle = %.4g\n", alu_inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "TEXEngine.WavefrontCount = %lld\n", compute_unit->tex_engine.wavefront_count);
		fprintf(f, "TEXEngine.Instructions = %lld\n", compute_unit->tex_engine.inst_count);
		fprintf(f, "TEXEngine.Cycles = %lld\n", compute_unit->tex_engine.cycle);
		fprintf(f, "TEXEngine.InstructionsPerCycle = %.4g\n", tex_inst_per_cycle);
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


void evg_gpu_uop_trash_empty(void)
{
	struct evg_uop_t *uop;

	while (evg_gpu->trash_uop_list->count)
	{
		linked_list_head(evg_gpu->trash_uop_list);
		uop = linked_list_get(evg_gpu->trash_uop_list);
		linked_list_remove(evg_gpu->trash_uop_list);

		evg_trace("evg.end_inst id=%lld cu=%d\n",
			uop->id_in_compute_unit, uop->compute_unit->id);

		evg_uop_free(uop);
	}
}


void evg_gpu_uop_trash_add(struct evg_uop_t *uop)
{
	linked_list_add(evg_gpu->trash_uop_list, uop);
}





/*
 * Class 'EvgGpu'
 */

CLASS_IMPLEMENTATION(EvgGpu);

void EvgGpuCreate(EvgGpu *self)
{
	struct evg_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Parent */
	TimingCreate(asTiming(self));

	/* Frequency */
	asTiming(self)->frequency = evg_gpu_frequency;
	asTiming(self)->frequency_domain = esim_new_domain(evg_gpu_frequency);

	/* Initialize */
	self->trash_uop_list = linked_list_create();
	self->compute_units = xcalloc(evg_gpu_num_compute_units, sizeof(void *));
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		self->compute_units[compute_unit_id] = evg_compute_unit_create();
		compute_unit = self->compute_units[compute_unit_id];
		compute_unit->id = compute_unit_id;
		DOUBLE_LINKED_LIST_INSERT_TAIL(self, ready, compute_unit);
	}

	/* Virtual functions */
	asObject(self)->Dump = EvgGpuDump;
	asTiming(self)->DumpSummary = EvgGpuDumpSummary;
	asTiming(self)->Run = EvgGpuRun;
	asTiming(self)->MemConfigCheck = EvgGpuMemConfigCheck;
	asTiming(self)->MemConfigDefault = EvgGpuMemConfigDefault;
	asTiming(self)->MemConfigParseEntry = EvgGpuMemConfigParseEntry;
}


void EvgGpuDestroy(EvgGpu *self)
{
	struct evg_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Free stream cores, compute units, and device */
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = self->compute_units[compute_unit_id];
		evg_compute_unit_free(compute_unit);
	}
	free(self->compute_units);
}


void EvgGpuDump(Object *self, FILE *f)
{
}


void EvgGpuDumpSummary(Timing *self, FILE *f)
{
	double inst_per_cycle;

	/* Call parent */
	TimingDumpSummary(asTiming(self), f);

	/* Additional statistics */
	inst_per_cycle = asTiming(evg_gpu)->cycle ?
			(double) asEmu(evg_emu)->instructions
			/ asTiming(evg_gpu)->cycle : 0.0;
	fprintf(f, "IPC = %.4g\n", inst_per_cycle);
}


int EvgGpuRun(Timing *self)
{
	EvgGpu *gpu = asEvgGpu(self);

	struct evg_ndrange_t *ndrange;

	struct evg_compute_unit_t *compute_unit;
	struct evg_compute_unit_t *compute_unit_next;

	/* For efficiency when no Evergreen emulation is selected, exit here
	 * if the list of existing ND-Ranges is empty. */
	if (!evg_emu->ndrange_list_count)
		return FALSE;

	/* Start one ND-Range in state 'pending' */
	while ((ndrange = evg_emu->pending_ndrange_list_head))
	{
		/* Currently not supported for more than 1 ND-Range */
		if (gpu->ndrange)
			fatal("%s: Evergreen GPU timing simulation not supported for multiple ND-Ranges",
				__FUNCTION__);

		/* Set ND-Range status to 'running' */
		evg_ndrange_clear_status(ndrange, evg_ndrange_pending);
		evg_ndrange_set_status(ndrange, evg_ndrange_running);

		/* Trace */
		evg_trace("evg.new_ndrange "
			"id=%d "
			"wg_first=%d "
			"wg_count=%d\n",
			ndrange->id,
			ndrange->work_group_id_first,
			ndrange->work_group_count);

		/* Map ND-Range to GPU */
		evg_gpu_map_ndrange(ndrange);
		evg_calc_plot();
	}

	/* Mapped ND-Range */
	ndrange = gpu->ndrange;
	assert(ndrange);

	/* Allocate work-groups to compute units */
	while (gpu->ready_list_head && ndrange->pending_list_head)
		evg_compute_unit_map_work_group(gpu->ready_list_head,
			ndrange->pending_list_head);

	/* One more cycle */
	asTiming(evg_gpu)->cycle++;

	/* Stop if maximum number of GPU cycles exceeded */
	if (evg_emu_max_cycles && asTiming(evg_gpu)->cycle >= evg_emu_max_cycles)
		esim_finish = esim_finish_evg_max_cycles;

	/* Stop if maximum number of GPU instructions exceeded */
	if (evg_emu_max_inst && asEmu(evg_emu)->instructions >= evg_emu_max_inst)
		esim_finish = esim_finish_evg_max_inst;
	
	/* Stop if there was a simulation stall */
	if (asTiming(evg_gpu)->cycle - gpu->last_complete_cycle > 1000000)
	{
		warning("Evergreen GPU simulation stalled.\n%s",
			evg_err_stall);
		esim_finish = esim_finish_stall;
	}

	/* Stop if any reason met */
	if (esim_finish)
		return TRUE;

	/* Free instructions in trash */
	evg_gpu_uop_trash_empty();

	/* Run one loop iteration on each busy compute unit */
	for (compute_unit = gpu->busy_list_head; compute_unit;
		compute_unit = compute_unit_next)
	{
		/* Store next busy compute unit, since this can change
		 * during the compute unit simulation loop iteration. */
		compute_unit_next = compute_unit->busy_list_next;

		/* Run one cycle */
		evg_compute_unit_run(compute_unit);
	}

	/* GPU-REL: insert stack faults */
	evg_faults_insert();

	/* If ND-Range finished execution in all compute units, free it. */
	if (!gpu->busy_list_count)
	{
		/* Dump ND-Range report */
		evg_ndrange_dump(ndrange, evg_emu_report_file);

		/* Stop if maximum number of kernels reached */
		if (evg_emu_max_kernels && evg_emu->ndrange_count >= evg_emu_max_kernels)
			esim_finish = esim_finish_evg_max_kernels;

		/* Finalize and free ND-Range */
		assert(evg_ndrange_get_status(ndrange, evg_ndrange_finished));
		evg_gpu_uop_trash_empty();
		evg_gpu_unmap_ndrange();
		evg_ndrange_free(ndrange);
	}

	/* Still simulating */
	return TRUE;
}
