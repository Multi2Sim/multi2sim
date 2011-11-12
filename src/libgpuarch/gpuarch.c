/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel.h>
#include <gpuarch.h>
#include <cpukernel.h>
#include <config.h>
#include <debug.h>
#include <repos.h>
#include <esim.h>
#include <heap.h>



/*
 * Global variables
 */

char *gpu_config_help =
	"The GPU configuration file is a plain text file in the IniFile format, defining\n"
	"the parameters of the GPU model for a detailed (architectural) GPU configuration.\n"
	"This file is passed to Multi2Sim with the '--gpu-config <file>' option, and\n"
	"should always be used together with option '--gpu-sim detailed'.\n"
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
	"      Number of work-items in a wavefront, executing AMD Evergreen instructions in\n"
	"      a SIMD fashion.\n"
	"  MaxWorkGroupsPerComputeUnit = <num> (Default = 8)\n"
	"  MaxWavefrontsPerComputeUnit = <num> (Default = 32)\n"
	"      Maximum number of work-groups and wavefronts allocated at a time in a compute\n"
	"      unit. These are some of the factors limiting the number of work-groups mapped\n"
	"      to a compute unit.\n"
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
	"  BlockSize = <bytes> (Default = 16)\n"
	"      Access block size, used for access coalescing purposes among work-items.\n"
	"  Latency = <num_cycles> (Default = 2)\n"
	"      Hit latency in number of cycles.\n"
	"  Banks = <num> (Default = 4)\n"
	"      Number of banks.\n"
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports per bank.\n"
	"  WritePorts = <num> (Default = 2)\n"
	"      Number of write ports per bank.\n"
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
	"\n";

enum gpu_sim_kind_enum gpu_sim_kind = gpu_sim_kind_functional;

char *gpu_config_file_name = "";
char *gpu_report_file_name = "";

int gpu_pipeline_debug_category;

/* Default parameters based on the AMD Radeon HD 5870 */
int gpu_num_compute_units = 20;
int gpu_num_stream_cores = 16;
int gpu_num_registers = 16384;
int gpu_register_alloc_size = 32;
char *gpu_register_alloc_granularity_str = "WorkGroup";
enum gpu_register_alloc_granularity_enum gpu_register_alloc_granularity;
int gpu_max_work_groups_per_compute_unit = 8;
int gpu_max_wavefronts_per_compute_unit = 32;

/* Local memory parameters */
int gpu_local_mem_size = 32768;  /* 32 KB */
int gpu_local_mem_alloc_size = 1024;  /* 1 KB */
int gpu_local_mem_latency = 2;
int gpu_local_mem_block_size = 16;
int gpu_local_mem_banks = 4;
int gpu_local_mem_read_ports = 2;
int gpu_local_mem_write_ports = 2;

struct gpu_t *gpu;

/* GPU-REL: insertion of faults into stack */
char *gpu_faults_file_name = "";
enum gpu_fault_type_enum {
	gpu_fault_ams,
	gpu_fault_reg,
	gpu_fault_mem
};
struct gpu_fault_t {
	enum gpu_fault_type_enum type;
	long long cycle;
	int compute_unit_id;  /* 0, gpu_num_compute_units - 1 ] */
	int stack_id;  /* [ 0, gpu_max_wavefronts_per_compute_unit - 1 ] */
	int active_mask_id;  /* [ 0, GPU_MAX_STACK_SIZE - 1 ] */
	int bit;
	int reg_id;
	int byte;
};
int gpu_fault_errors = 0;  /* Number of faults causing error */
struct lnlist_t *gpu_fault_list;
int gpu_faults_debug_category;
char *gpu_faults_debug_file_name = "";




/*
 * GPU Device
 */

static void gpu_init_device()
{
	struct gpu_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Create device */
	gpu = calloc(1, sizeof(struct gpu_t));

	/* Create compute units */
	gpu->compute_units = calloc(gpu_num_compute_units, sizeof(void *));
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		gpu->compute_units[compute_unit_id] = gpu_compute_unit_create();
		compute_unit = gpu->compute_units[compute_unit_id];
		compute_unit->id = compute_unit_id;
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, ready, compute_unit);
	}
}


/* GPU-REL: stack faults */
void gpu_stack_faults_init(void)
{
	FILE *f;
	char line[MAX_STRING_SIZE];
	char *line_ptr;
	struct gpu_fault_t *fault;
	int line_num;
	long long last_cycle;

	gpu_fault_list = lnlist_create();
	if (!*gpu_faults_file_name)
		return;

	f = fopen(gpu_faults_file_name, "rt");
	if (!f)
		fatal("%s: cannot open file", gpu_faults_file_name);
	
	line_num = 0;
	last_cycle = 0;
	while (!feof(f))
	{
		const char *delim = " ";
	
		/* Read a line */
		line_num++;
		line_ptr = fgets(line, MAX_STRING_SIZE, f);
		if (!line_ptr)
			break;

		/* Allocate new fault */
		fault = calloc(1, sizeof(struct gpu_fault_t));

		/* Read <cycle> field */
		line_ptr = strtok(line_ptr, delim);
		if (!line_ptr)
			goto wrong_format;
		fault->cycle = atoll(line_ptr);
		if (fault->cycle < 1)
			fatal("%s: line %d: lowest possible cycle is 1",
				gpu_faults_file_name, line_num);
		if (fault->cycle < last_cycle)
			fatal("%s: line %d: cycles must be ordered",
				gpu_faults_file_name, line_num);

		/* <fault> - Type of fault */
		line_ptr = strtok(NULL, delim);
		if (!line_ptr)
			goto wrong_format;
		if (!strcmp(line_ptr, "ams"))
			fault->type = gpu_fault_ams;
		else if (!strcmp(line_ptr, "reg"))
			fault->type = gpu_fault_reg;
		else if (!strcmp(line_ptr, "mem"))
			fault->type = gpu_fault_mem;
		else
			fatal("%s: line %d: invalid value for <fault> ('%s')",
				gpu_faults_file_name, line_num, line_ptr);

		/* <cu_id> - Compute unit */
		line_ptr = strtok(NULL, delim);
		if (!line_ptr)
			goto wrong_format;
		fault->compute_unit_id = atoi(line_ptr);
		if (fault->compute_unit_id >= gpu_num_compute_units || fault->compute_unit_id < 0)
			fatal("%s: line %d: invalid compute unit ID",
				gpu_faults_file_name, line_num);

		/* Analyze rest of the line depending on fault type */
		switch (fault->type) {

		case gpu_fault_ams:

			/* <stack_id> - Stack ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->stack_id = atoi(line_ptr);
			if (fault->stack_id >= gpu_max_wavefronts_per_compute_unit)
				fatal("%s: line %d: invalid stack ID",
					gpu_faults_file_name, line_num);

			/* <am_id> - Active mask ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->active_mask_id = atoi(line_ptr);
			if (fault->active_mask_id >= GPU_MAX_STACK_SIZE)
				fatal("%s: line %d: invalid active mask ID",
					gpu_faults_file_name, line_num);

			/* <bit> */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit >= gpu_wavefront_size)
				fatal("%s: line %d: invalid bit index",
					gpu_faults_file_name, line_num);

			/* No more tokens */
			if (strtok(NULL, delim))
				fatal("%s: line %d: too many arguments",
					gpu_faults_file_name, line_num);

			break;

		case gpu_fault_reg:

			/* <reg_id> - Register ID */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->reg_id = atoi(line_ptr);
			if (fault->reg_id >= gpu_num_registers || fault->reg_id < 0)
				fatal("%s: line %d: invalid compute unit ID",
					gpu_faults_file_name, line_num);

			/* <bit> */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit < 0 || fault->bit >= 128)
				fatal("%s: line %d: invalid bit index",
					gpu_faults_file_name, line_num);

			break;

		case gpu_fault_mem:

			/* <byte> - Byte position in local memory */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->byte = atoi(line_ptr);
			if (fault->byte >= gpu_local_mem_size || fault->byte < 0)
				fatal("%s: line %d: invalid byte position",
					gpu_faults_file_name, line_num);

			/* <bit> - Bit position */
			line_ptr = strtok(NULL, delim);
			if (!line_ptr)
				goto wrong_format;
			fault->bit = atoi(line_ptr);
			if (fault->bit > 7 || fault->bit < 0)
				fatal("%s: line %d: invalid bit position",
					gpu_faults_file_name, line_num);

			break;
		}

		/* Insert fault in fault list */
		lnlist_out(gpu_fault_list);
		lnlist_insert(gpu_fault_list, fault);
		last_cycle = fault->cycle;
		continue;

wrong_format:
		fatal("%s: line %d: not enough arguments",
			gpu_faults_file_name, line_num);
	}
	lnlist_head(gpu_fault_list);
}


/* GPU-REL: stack faults */
void gpu_stack_faults_done(void)
{
	while (lnlist_count(gpu_fault_list)) {
		lnlist_head(gpu_fault_list);
		free(lnlist_get(gpu_fault_list));
		lnlist_remove(gpu_fault_list);
	}
	lnlist_free(gpu_fault_list);
}


/* GPU-REL: insert stack faults */
void gpu_stack_faults_insert(void)
{
	struct gpu_fault_t *fault;
	struct gpu_compute_unit_t *compute_unit;

	for (;;) {
		lnlist_head(gpu_fault_list);
		fault = lnlist_get(gpu_fault_list);
		if (!fault || fault->cycle > gpu->cycle)
			break;

		/* Insert fault depending on fault type */
		switch (fault->type) {

		case gpu_fault_ams:
		{
			struct gpu_work_group_t *work_group;
			struct gpu_wavefront_t *wavefront;
			struct gpu_work_item_t *work_item;

			int work_group_id;  /* in compute unit */
			int wavefront_id;  /* in compute unit */
			int value;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"ams\" stack=%d am=%d bit=%d ",
				(long long) gpu->cycle,
				fault->compute_unit_id, fault->stack_id,
				fault->active_mask_id, fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count) {
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Get work-group and wavefront. If wavefront ID exceeds current number, dismiss */
			work_group_id = fault->stack_id / gpu->ndrange->wavefronts_per_work_group;
			wavefront_id = fault->stack_id % gpu->ndrange->wavefronts_per_work_group;
			if (work_group_id >= gpu_max_work_groups_per_compute_unit || !compute_unit->work_groups[work_group_id]) {
				gpu_faults_debug("effect=\"wf_idle\"");
				goto end_loop;
			}
			work_group = compute_unit->work_groups[work_group_id];
			wavefront = work_group->wavefronts[wavefront_id];

			/* If active_mask_id exceeds stack top, dismiss */
			if (fault->active_mask_id > wavefront->stack_top) {
				gpu_faults_debug("effect=\"am_idle\"");
				goto end_loop;
			}

			/* If 'bit' exceeds number of work-items in wavefront, dismiss */
			if (fault->bit >= wavefront->work_item_count) {
				gpu_faults_debug("effect=\"wi_idle\"");
				goto end_loop;
			}

			/* Fault caused an error, show affected software entities */
			work_item = wavefront->work_items[fault->bit];
			gpu_faults_debug("effect=\"error\" wg=%d wf=%d wi=%d",
				work_group->id,
				wavefront->id,
				work_item->id);

			/* Inject fault */
			value = bit_map_get(wavefront->active_stack, fault->active_mask_id * wavefront->work_item_count
				+ fault->bit, 1);
			bit_map_set(wavefront->active_stack, fault->active_mask_id * wavefront->work_item_count
				+ fault->bit, 1, !value);
			gpu_fault_errors++;

			break;
		}

		case gpu_fault_reg:
		{
			struct opencl_kernel_t *kernel = gpu->ndrange->kernel;

			int work_group_id_in_compute_unit;
			struct gpu_work_group_t *work_group;

			int num_registers_per_work_group;

			int work_item_id_in_compute_unit;
			int work_item_id_in_work_group;
			struct gpu_work_item_t *work_item;

			int lo_reg;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"reg\" reg=%d bit=%d ",
				(long long) gpu->cycle,
				fault->compute_unit_id,
				fault->reg_id,
				fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count) {
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Get work-group */
			num_registers_per_work_group = kernel->cal_abi->num_gpr_used * kernel->local_size;
			work_group_id_in_compute_unit = fault->reg_id / num_registers_per_work_group;
			if (work_group_id_in_compute_unit >= gpu_max_work_groups_per_compute_unit) {
				gpu_faults_debug("effect=\"reg_idle\"");
				goto end_loop;
			}

			/* Get work-group (again) */
			work_group = compute_unit->work_groups[work_group_id_in_compute_unit];
			if (!work_group) {
				gpu_faults_debug("effect=\"reg_idle\"");
				goto end_loop;
			}

			/* Fault caused error - get affected entities */
			gpu_faults_debug("effect=\"error\" ");
			work_item_id_in_compute_unit = fault->reg_id / kernel->cal_abi->num_gpr_used;
			work_item_id_in_work_group = work_item_id_in_compute_unit % kernel->local_size;
			work_item = work_group->work_items[work_item_id_in_work_group];
			lo_reg = fault->reg_id % kernel->cal_abi->num_gpr_used;
			gpu_faults_debug("wg=%d wf=%d wi=%d lo_reg=%d ",
				work_group->id, work_item->wavefront->id, work_item->id, lo_reg);

			/* Insert the fault */
			if (fault->bit < 32)
				work_item->gpr[lo_reg].elem[0] ^= 1 << fault->bit;
			else if (fault->bit < 64)
				work_item->gpr[lo_reg].elem[1] ^= 1 << (fault->bit - 32);
			else if (fault->bit < 96)
				work_item->gpr[lo_reg].elem[2] ^= 1 << (fault->bit - 64);
			else
				work_item->gpr[lo_reg].elem[3] ^= 1 << (fault->bit - 96);
			gpu_fault_errors++;

			break;

		}

		case gpu_fault_mem:
		{
			struct gpu_work_group_t *work_group;
			struct gpu_wavefront_t *wavefront;
			struct gpu_work_item_t *work_item;

			int work_group_id_in_compute_unit;
			unsigned char value;

			/* Initial debug */
			gpu_faults_debug("fault clk=%lld cu=%d type=\"mem\" byte=%d bit=%d ",
				(long long) gpu->cycle,
				fault->compute_unit_id,
				fault->byte,
				fault->bit);
			assert(fault->cycle == gpu->cycle);
			compute_unit = gpu->compute_units[fault->compute_unit_id];

			/* If compute unit is idle, dismiss */
			if (!compute_unit->work_group_count) {
				gpu_faults_debug("effect=\"cu_idle\"");
				goto end_loop;
			}

			/* Get work-group */
			work_group_id_in_compute_unit = fault->byte / gpu->ndrange->local_mem_top;
			if (work_group_id_in_compute_unit >= gpu_max_work_groups_per_compute_unit) {
				gpu_faults_debug("effect=\"mem_idle\"");
				goto end_loop;
			}

			/* Get work-group (again) */
			work_group = compute_unit->work_groups[work_group_id_in_compute_unit];
			if (!work_group) {
				gpu_faults_debug("effect=\"mem_idle\"");
				goto end_loop;
			}

			/* Inject fault */
			gpu_faults_debug("effect=\"error\" wg=%d ",
				work_group->id);
			mem_read(work_group->local_mem, fault->byte, 1, &value);
			value ^= 1 << fault->bit;
			mem_write(work_group->local_mem, fault->byte, 1, &value);
			gpu_fault_errors++;

			break;

		}

		default:
			panic("invalid fault type");

		}

end_loop:
		/* Extract and free */
		free(fault);
		lnlist_remove(gpu_fault_list);
		gpu_faults_debug("\n");

		/* If all faults were inserted and no error was caused, end simulation */
		if (!lnlist_count(gpu_fault_list) && !gpu_fault_errors)
			ke_sim_finish = ke_sim_finish_gpu_no_faults;
	}
}


void gpu_config_read(void)
{
	struct config_t *gpu_config;
	char *section;
	char *err_note =
		"\tPlease run 'm2s --help-gpu-config' or consult the Multi2Sim Guide for a\n"
		"\tdescription of the GPU configuration file format.";

	/* Load GPU configuration file */
	gpu_config = config_create(gpu_config_file_name);
	if (*gpu_config_file_name && !config_load(gpu_config))
		fatal("%s: cannot load GPU configuration file", gpu_config_file_name);
	
	/* Device */
	section = "Device";
	gpu_num_compute_units = config_read_int(gpu_config, section, "NumComputeUnits", gpu_num_compute_units);
	gpu_num_stream_cores = config_read_int(gpu_config, section, "NumStreamCores", gpu_num_stream_cores);
	gpu_num_registers = config_read_int(gpu_config, section, "NumRegisters", gpu_num_registers);
	gpu_register_alloc_size = config_read_int(gpu_config, section, "RegisterAllocSize", gpu_register_alloc_size);
	gpu_register_alloc_granularity_str = config_read_string(gpu_config, section, "RegisterAllocGranularity",
		gpu_register_alloc_granularity_str);
	gpu_wavefront_size = config_read_int(gpu_config, section, "WavefrontSize", gpu_wavefront_size);
	gpu_max_work_groups_per_compute_unit = config_read_int(gpu_config, section, "MaxWorkGroupsPerComputeUnit",
		gpu_max_work_groups_per_compute_unit);
	gpu_max_wavefronts_per_compute_unit = config_read_int(gpu_config, section, "MaxWavefrontsPerComputeUnit",
		gpu_max_wavefronts_per_compute_unit);
	if (gpu_num_compute_units < 1)
		fatal("%s: invalid value for 'NumComputeUnits'.\n%s", gpu_config_file_name, err_note);
	if (gpu_num_stream_cores < 1)
		fatal("%s: invalid value for 'NumStreamCores'.\n%s", gpu_config_file_name, err_note);
	if (gpu_register_alloc_size < 1)
		fatal("%s: invalid value for 'RegisterAllocSize'.\n%s", gpu_config_file_name, err_note);
	if (gpu_num_registers < 1)
		fatal("%s: invalid value for 'NumRegisters'.\n%s", gpu_config_file_name, err_note);
	if (gpu_num_registers % gpu_register_alloc_size)
		fatal("%s: 'NumRegisters' must be a multiple of 'RegisterAllocSize'.\n%s", gpu_config_file_name, err_note);
	if (!strcasecmp(gpu_register_alloc_granularity_str, "Wavefront"))
		gpu_register_alloc_granularity = gpu_register_alloc_wavefront;
	else if (!strcasecmp(gpu_register_alloc_granularity_str, "WorkGroup"))
		gpu_register_alloc_granularity = gpu_register_alloc_work_group;
	else
		fatal("%s: invalid value for 'RegisterAllocGranularity'.\n%s", gpu_config_file_name, err_note);
	if (gpu_wavefront_size < 1)
		fatal("%s: invalid value for 'WavefrontSize'.\n%s", gpu_config_file_name, err_note);
	if (gpu_max_work_groups_per_compute_unit < 1)
		fatal("%s: invalid value for 'MaxWorkGroupsPerComputeUnit'.\n%s", gpu_config_file_name, err_note);
	if (gpu_max_wavefronts_per_compute_unit < 1)
		fatal("%s: invalid value for 'MaxWavefrontsPerComputeUnit'.\n%s", gpu_config_file_name, err_note);
	
	/* Local memory */
	section = "LocalMemory";
	gpu_local_mem_size = config_read_int(gpu_config, section, "Size", gpu_local_mem_size);
	gpu_local_mem_alloc_size = config_read_int(gpu_config, section, "AllocSize", gpu_local_mem_alloc_size);
	gpu_local_mem_block_size = config_read_int(gpu_config, section, "BlockSize", gpu_local_mem_block_size);
	gpu_local_mem_latency = config_read_int(gpu_config, section, "Latency", gpu_local_mem_latency);
	gpu_local_mem_banks = config_read_int(gpu_config, section, "Banks", gpu_local_mem_banks);
	gpu_local_mem_read_ports = config_read_int(gpu_config, section, "ReadPorts", gpu_local_mem_read_ports);
	gpu_local_mem_write_ports = config_read_int(gpu_config, section, "WritePorts", gpu_local_mem_write_ports);
	if ((gpu_local_mem_size & (gpu_local_mem_size - 1)) || gpu_local_mem_size < 4)
		fatal("%s: %s->Size must be a power of two and at least 4.\n%s",
			gpu_config_file_name, section, err_note);
	if (gpu_local_mem_alloc_size < 1)
		fatal("%s: invalid value for %s->Allocsize.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_size % gpu_local_mem_alloc_size)
		fatal("%s: %s->Size must be a multiple of %s->AllocSize.\n%s", gpu_config_file_name,
			section, section, err_note);
	if ((gpu_local_mem_block_size & (gpu_local_mem_block_size - 1)) || gpu_local_mem_block_size < 4)
		fatal("%s: %s->BlockSize must be a power of two and at least 4.\n%s",
			gpu_config_file_name, section, err_note);
	if (gpu_local_mem_alloc_size % gpu_local_mem_block_size)
		fatal("%s: %s->AllocSize must be a multiple of %s->BlockSize.\n%s", gpu_config_file_name,
			section, section, err_note);
	if (gpu_local_mem_latency < 1)
		fatal("%s: invalid value for %s->Latency.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_banks < 1 || (gpu_local_mem_banks & (gpu_local_mem_banks - 1)))
		fatal("%s: %s->Banks must be a power of 2 greater than 1.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_read_ports < 1)
		fatal("%s: invalid value for %s->ReadPorts.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_write_ports < 1)
		fatal("%s: invalid value for %s->WritePorts.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_size < gpu_local_mem_block_size * gpu_local_mem_banks)
		fatal("%s: %s->Size cannot be smaller than %s->BlockSize * %s->Banks.\n%s", gpu_config_file_name,
			section, section, section, err_note);
	
	/* CF Engine */
	section = "CFEngine";
	gpu_cf_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		gpu_cf_engine_inst_mem_latency);
	if (gpu_cf_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", gpu_config_file_name, section, err_note);
	
	/* ALU Engine */
	section = "ALUEngine";
	gpu_alu_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		gpu_alu_engine_inst_mem_latency);
	gpu_alu_engine_fetch_queue_size = config_read_int(gpu_config, section, "FetchQueueSize",
		gpu_alu_engine_fetch_queue_size);
	gpu_alu_engine_pe_latency = config_read_int(gpu_config, section, "ProcessingElementLatency",
		gpu_alu_engine_pe_latency);
	if (gpu_alu_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_alu_engine_fetch_queue_size < 56)
		fatal("%s: the minimum value for %s->FetchQueueSize is 56.\n"
			"This is the maximum size of one VLIW bundle, including 5 ALU instructions\n"
			"(2 words each), and 4 literal constants (1 word each).\n%s",
			gpu_config_file_name, section, err_note);
	if (gpu_alu_engine_pe_latency < 1)
		fatal("%s: invalud value for %s->ProcessingElementLatency.\n%s", gpu_config_file_name, section, err_note);

	/* TEX Engine */
	section = "TEXEngine";
	gpu_tex_engine_inst_mem_latency = config_read_int(gpu_config, section, "InstructionMemoryLatency",
		gpu_tex_engine_inst_mem_latency);
	gpu_tex_engine_fetch_queue_size = config_read_int(gpu_config, section, "FetchQueueSize",
		gpu_tex_engine_fetch_queue_size);
	if (gpu_tex_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_tex_engine_fetch_queue_size < 16)
		fatal("%s: the minimum value for %s->FetchQueueSize is 16.\n"
			"This size corresponds to the 4 words comprising a TEX Evergreen instruction.\n%s",
			gpu_config_file_name, section, err_note);
	
	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);
}


void gpu_config_dump(FILE *f)
{
	/* Device configuration */
	fprintf(f, "[ Config.Device ]\n");
	fprintf(f, "NumComputeUnits = %d\n", gpu_num_compute_units);
	fprintf(f, "NumStreamCores = %d\n", gpu_num_stream_cores);
	fprintf(f, "NumRegisters = %d\n", gpu_num_registers);
	fprintf(f, "RegisterAllocSize = %d\n", gpu_register_alloc_size);
	fprintf(f, "RegisterAllocGranularity = %s\n", gpu_register_alloc_granularity_str);
	fprintf(f, "WavefrontSize = %d\n", gpu_wavefront_size);
	fprintf(f, "MaxWorkGroupsPerComputeUnit = %d\n", gpu_max_work_groups_per_compute_unit);
	fprintf(f, "MaxWavefrontsPerComputeUnit = %d\n", gpu_max_wavefronts_per_compute_unit);
	fprintf(f, "\n");

	/* Local Memory */
	fprintf(f, "[ Config.LocalMemory ]\n");
	fprintf(f, "Size = %d\n", gpu_local_mem_size);
	fprintf(f, "AllocSize = %d\n", gpu_local_mem_alloc_size);
	fprintf(f, "BlockSize = %d\n", gpu_local_mem_block_size);
	fprintf(f, "Latency = %d\n", gpu_local_mem_latency);
	fprintf(f, "Banks = %d\n", gpu_local_mem_banks);
	fprintf(f, "ReadPorts = %d\n", gpu_local_mem_read_ports);
	fprintf(f, "WritePorts = %d\n", gpu_local_mem_write_ports);
	fprintf(f, "\n");

	/* CF Engine */
	fprintf(f, "[ Config.CFEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", gpu_cf_engine_inst_mem_latency);
	fprintf(f, "\n");

	/* ALU Engine */
	fprintf(f, "[ Config.ALUEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", gpu_alu_engine_inst_mem_latency);
	fprintf(f, "FetchQueueSize = %d\n", gpu_alu_engine_fetch_queue_size);
	fprintf(f, "ProcessingElementLatency = %d\n", gpu_alu_engine_pe_latency);
	fprintf(f, "\n");

	/* TEX Engine */
	fprintf(f, "[ Config.TEXEngine ]\n");
	fprintf(f, "InstructionMemoryLatency = %d\n", gpu_tex_engine_inst_mem_latency);
	fprintf(f, "FetchQueueSize = %d\n", gpu_tex_engine_fetch_queue_size);
	fprintf(f, "\n");
	
	/* End of configuration */
	fprintf(f, "\n");
}


void gpu_init(void)
{
	/* Try to open report file */
	if (gpu_report_file_name[0] && !can_open_write(gpu_report_file_name))
		fatal("%s: cannot open GPU pipeline report file",
			gpu_report_file_name);

	/* Read configuration file */
	gpu_config_read();

	/* Initialize GPU */
	gpu_init_device();

	/* Uops */
	gpu_uop_init();
	
	/* Cache system */
	gpu_cache_init();

	/* GPU-REL: read stack faults file */
	gpu_stack_faults_init();
}


void gpu_done()
{
	struct gpu_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* GPU pipeline report */
	gpu_dump_report();

	/* Cache system */
	gpu_cache_done();

	/* Free stream cores, compute units, and device */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		compute_unit = gpu->compute_units[compute_unit_id];
		gpu_compute_unit_free(compute_unit);
	}
	free(gpu->compute_units);
	free(gpu);
	
	/* Uops */
	gpu_uop_done();

	/* GPU-REL: read stack faults file */
	gpu_stack_faults_done();
}


void gpu_dump_report(void)
{
	struct gpu_compute_unit_t *compute_unit;
	struct gpu_cache_t *local_memory;
	int compute_unit_id;

	FILE *f;

	double inst_per_cycle;
	double cf_inst_per_cycle;
	double alu_inst_per_cycle;
	double tex_inst_per_cycle;
	uint64_t coalesced_reads;
	uint64_t coalesced_writes;
	char vliw_occupancy[MAX_STRING_SIZE];

	/* Open file */
	f = open_write(gpu_report_file_name);
	if (!f)
		return;
	
	/* Dump GPU configuration */
	fprintf(f, ";\n; GPU Configuration\n;\n\n");
	gpu_config_dump(f);
	
	/* Report for device */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	inst_per_cycle = gpu->cycle ? (double) gk->inst_count / gpu->cycle : 0.0;
	fprintf(f, "[ Device ]\n\n");
	fprintf(f, "NDRangeCount = %d\n", gk->ndrange_count);
	fprintf(f, "Instructions = %lld\n", (long long) gk->inst_count);
	fprintf(f, "Cycles = %lld\n", (long long) gpu->cycle);
	fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "\n\n");

	/* Report for compute units */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {

		compute_unit = gpu->compute_units[compute_unit_id];
		local_memory = compute_unit->local_memory;

		inst_per_cycle = compute_unit->cycle ? (double) compute_unit->inst_count
			/ compute_unit->cycle : 0.0;
		cf_inst_per_cycle = compute_unit->cycle ? (double) compute_unit->cf_engine.inst_count
			/ compute_unit->cycle : 0.0;
		alu_inst_per_cycle = compute_unit->alu_engine.cycle ? (double) compute_unit->alu_engine.inst_count
			/ compute_unit->alu_engine.cycle : 0.0;
		tex_inst_per_cycle = compute_unit->tex_engine.cycle ? (double) compute_unit->tex_engine.inst_count
			/ compute_unit->tex_engine.cycle : 0.0;
		coalesced_reads = local_memory->reads - local_memory->effective_reads;
		coalesced_writes = local_memory->writes - local_memory->effective_writes;
		snprintf(vliw_occupancy, MAX_STRING_SIZE, "%lld %lld %lld %lld %lld",
			(long long) compute_unit->alu_engine.vliw_slots[0],
			(long long) compute_unit->alu_engine.vliw_slots[1],
			(long long) compute_unit->alu_engine.vliw_slots[2],
			(long long) compute_unit->alu_engine.vliw_slots[3],
			(long long) compute_unit->alu_engine.vliw_slots[4]);

		fprintf(f, "[ ComputeUnit %d ]\n\n", compute_unit_id);

		fprintf(f, "WorkGroupCount = %lld\n", (long long) compute_unit->mapped_work_groups);
		fprintf(f, "Instructions = %lld\n", (long long) compute_unit->inst_count);
		fprintf(f, "Cycles = %lld\n", (long long) compute_unit->cycle);
		fprintf(f, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "CFEngine.Instructions = %lld\n", (long long) compute_unit->cf_engine.inst_count);
		fprintf(f, "CFEngine.InstructionsPerCycle = %.4g\n", cf_inst_per_cycle);
		fprintf(f, "CFEngine.ALUClauseTriggers = %lld\n", (long long) compute_unit->cf_engine.alu_clause_trigger_count);
		fprintf(f, "CFEngine.TEXClauseTriggers = %lld\n", (long long) compute_unit->cf_engine.tex_clause_trigger_count);
		fprintf(f, "CFEngine.GlobalMemWrites = %lld\n", (long long) compute_unit->cf_engine.global_mem_write_count);
		fprintf(f, "\n");

		fprintf(f, "ALUEngine.WavefrontCount = %lld\n", (long long) compute_unit->alu_engine.wavefront_count);
		fprintf(f, "ALUEngine.Instructions = %lld\n", (long long) compute_unit->alu_engine.inst_count);
		fprintf(f, "ALUEngine.InstructionSlots = %lld\n", (long long) compute_unit->alu_engine.inst_slot_count);
		fprintf(f, "ALUEngine.LocalMemorySlots = %lld\n", (long long) compute_unit->alu_engine.local_mem_slot_count);
		fprintf(f, "ALUEngine.VLIWOccupancy = %s\n", vliw_occupancy);
		fprintf(f, "ALUEngine.Cycles = %lld\n", (long long) compute_unit->alu_engine.cycle);
		fprintf(f, "ALUEngine.InstructionsPerCycle = %.4g\n", alu_inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "TEXEngine.WavefrontCount = %lld\n", (long long) compute_unit->tex_engine.wavefront_count);
		fprintf(f, "TEXEngine.Instructions = %lld\n", (long long) compute_unit->tex_engine.inst_count);
		fprintf(f, "TEXEngine.Cycles = %lld\n", (long long) compute_unit->tex_engine.cycle);
		fprintf(f, "TEXEngine.InstructionsPerCycle = %.4g\n", tex_inst_per_cycle);
		fprintf(f, "\n");

		fprintf(f, "LocalMemory.Accesses = %lld\n", (long long) (local_memory->reads + local_memory->writes));
		fprintf(f, "LocalMemory.Reads = %lld\n", (long long) local_memory->reads);
		fprintf(f, "LocalMemory.EffectiveReads = %lld\n", (long long) local_memory->effective_reads);
		fprintf(f, "LocalMemory.CoalescedReads = %lld\n", (long long) coalesced_reads);
		fprintf(f, "LocalMemory.Writes = %lld\n", (long long) local_memory->writes);
		fprintf(f, "LocalMemory.EffectiveWrites = %lld\n", (long long) local_memory->effective_writes);
		fprintf(f, "LocalMemory.CoalescedWrites = %lld\n", (long long) coalesced_writes);
		fprintf(f, "\n\n");
	}
}


void gpu_map_ndrange(struct gpu_ndrange_t *ndrange)
{
	struct gpu_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Assign current ND-Range */
	assert(!gpu->ndrange);
	gpu->ndrange = ndrange;

	/* Check that at least one work-group can be allocated per compute unit */
	gpu->work_groups_per_compute_unit = gpu_calc_get_work_groups_per_compute_unit(
		ndrange->kernel->local_size, ndrange->kernel->cal_abi->num_gpr_used,
		ndrange->local_mem_top);
	if (!gpu->work_groups_per_compute_unit)
		fatal("work-group resources cannot be allocated to a compute unit.\n"
			"\tA compute unit in the GPU has a limit in number of wavefronts, number\n"
			"\tof registers, and amount of local memory. If the work-group size\n"
			"\texceeds any of these limits, the ND-Range cannot be executed.\n");

	/* Derived from this, calculate limit of wavefronts and work-items per compute unit. */
	gpu->wavefronts_per_compute_unit = gpu->work_groups_per_compute_unit * ndrange->wavefronts_per_work_group;
	gpu->work_items_per_compute_unit = gpu->wavefronts_per_compute_unit * gpu_wavefront_size;
	assert(gpu->work_groups_per_compute_unit <= gpu_max_work_groups_per_compute_unit);
	assert(gpu->wavefronts_per_compute_unit <= gpu_max_wavefronts_per_compute_unit);

	/* Reset architectural state */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		compute_unit = gpu->compute_units[compute_unit_id];
		compute_unit->cf_engine.decode_index = 0;
		compute_unit->cf_engine.execute_index = 0;
	}
}


void gpu_unmap_ndrange(void)
{
	/* Dump stats */
	gpu_ndrange_dump(gpu->ndrange, gpu_kernel_report_file);

	/* Unmap */
	gpu->ndrange = NULL;
}


void gpu_pipeline_debug_disasm(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	FILE *f = debug_file(gpu_pipeline_debug_category);

	void *cf_buf;
	int inst_count;
	int cf_inst_count;
	int sec_inst_count;
	int loop_idx;

	/* Initialize */
	cf_buf = kernel->cal_abi->text_buffer;
	inst_count = 0;
	cf_inst_count = 0;
	sec_inst_count = 0;
	loop_idx = 0;

	/* Disassemble */
	while (cf_buf)
	{
		struct amd_inst_t cf_inst;

		/* CF Instruction */
		cf_buf = amd_inst_decode_cf(cf_buf, &cf_inst);
                if (cf_inst.info->flags & AMD_INST_FLAG_DEC_LOOP_IDX) {
                        assert(loop_idx > 0);
                        loop_idx--;
                }

		fprintf(f, "asm i=%d cl=\"cf\" ", inst_count);
		amd_inst_dump_debug(&cf_inst, cf_inst_count, loop_idx, f);
		fprintf(f, "\n");

		cf_inst_count++;
		inst_count++;

		/* ALU Clause */
		if (cf_inst.info->fmt[0] == FMT_CF_ALU_WORD0)
		{
			void *alu_buf, *alu_buf_end;
			struct amd_alu_group_t alu_group;

			alu_buf = kernel->cal_abi->text_buffer + cf_inst.words[0].cf_alu_word0.addr * 8;
			alu_buf_end = alu_buf + (cf_inst.words[1].cf_alu_word1.count + 1) * 8;
			while (alu_buf < alu_buf_end)
			{
				alu_buf = amd_inst_decode_alu_group(alu_buf, sec_inst_count, &alu_group);

				fprintf(f, "asm i=%d cl=\"alu\" ", inst_count);
				amd_alu_group_dump_debug(&alu_group, sec_inst_count, loop_idx, f);
				fprintf(f, "\n");

				sec_inst_count++;
				inst_count++;
			}
		}

		/* TEX Clause */
		if (cf_inst.info->inst == AMD_INST_TC)
		{
			char *tex_buf, *tex_buf_end;
			struct amd_inst_t inst;

			tex_buf = kernel->cal_abi->text_buffer + cf_inst.words[0].cf_word0.addr * 8;
			tex_buf_end = tex_buf + (cf_inst.words[1].cf_word1.count + 1) * 16;
			while (tex_buf < tex_buf_end)
			{
				tex_buf = amd_inst_decode_tc(tex_buf, &inst);

				fprintf(f, "asm i=%d cl=\"tex\" ", inst_count);
				amd_inst_dump_debug(&inst, sec_inst_count, loop_idx, f);
				fprintf(f, "\n");

				sec_inst_count++;
				inst_count++;
			}
		}

		/* Increase loop depth counter */
                if (cf_inst.info->flags & AMD_INST_FLAG_INC_LOOP_IDX)
                        loop_idx++;
	}
}


void gpu_pipeline_debug_ndrange(struct gpu_ndrange_t *ndrange)
{
	int work_group_id;
	struct gpu_work_group_t *work_group;

	int wavefront_id;
	struct gpu_wavefront_t *wavefront;

	/* Work-groups */
	FOREACH_WORK_GROUP_IN_NDRANGE(ndrange, work_group_id) {
		work_group = ndrange->work_groups[work_group_id];
		gpu_pipeline_debug("new item=\"wg\" "
			"id=%d "
			"wi_first=%d "
			"wi_count=%d "
			"wf_first=%d "
			"wf_count=%d\n",
			work_group->id,
			work_group->work_item_id_first,
			work_group->work_item_count,
			work_group->wavefront_id_first,
			work_group->wavefront_count);
	}
	
	/* Wavefronts */
	FOREACH_WAVEFRONT_IN_NDRANGE(ndrange, wavefront_id) {
		wavefront = ndrange->wavefronts[wavefront_id];
		gpu_pipeline_debug("new item=\"wf\" "
			"id=%d "
			"wg_id=%d "
			"wi_first=%d "
			"wi_count=%d\n",
			wavefront->id,
			wavefront->work_group->id,
			wavefront->work_item_id_first,
			wavefront->work_item_count);
	}
}


void gpu_pipeline_debug_intro(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;

	/* Initial */
	gpu_pipeline_debug("init "
		"global_size=%d "
		"local_size=%d "
		"group_count=%d "
		"wavefront_size=%d "
		"wavefronts_per_work_group=%d "
		"compute_units=%d "
		"\n",
		kernel->global_size,
		kernel->local_size,
		kernel->group_count,
		gpu_wavefront_size,
		ndrange->wavefronts_per_work_group,
		gpu_num_compute_units);
	
}


void gpu_run(struct gpu_ndrange_t *ndrange)
{
	struct gpu_compute_unit_t *compute_unit, *compute_unit_next;

	/* Debug */
	if (debug_status(gpu_pipeline_debug_category)) {
		gpu_pipeline_debug_intro(ndrange);
		gpu_pipeline_debug_ndrange(ndrange);
		gpu_pipeline_debug_disasm(ndrange);
	}

	/* Initialize */
	gpu_map_ndrange(ndrange);
	gpu_calc_plot();
	gk_timer_start();

	/* Execution loop */
	for (;;) {
		
		/* Allocate work-groups to compute units */
		while (gpu->ready_list_head && ndrange->pending_list_head)
			gpu_compute_unit_map_work_group(gpu->ready_list_head, ndrange->pending_list_head);
		
		/* If no compute unit is busy, done */
		if (!gpu->busy_list_head)
			break;
		
		/* Stop if maximum number of GPU cycles exceeded */
		if (gpu_max_cycles && gpu->cycle >= gpu_max_cycles)
			ke_sim_finish = ke_sim_finish_max_gpu_cycles;

		/* Stop if maximum number of GPU instructions exceeded */
		if (gpu_max_inst && gk->inst_count >= gpu_max_inst)
			ke_sim_finish = ke_sim_finish_max_gpu_inst;

		/* Stop if any reason met */
		if (ke_sim_finish)
			break;

		/* Next cycle */
		gpu->cycle++;
		gpu_pipeline_debug("clk c=%lld\n", (long long) gpu->cycle);
		
		/* Advance one cycle on each busy compute unit */
		for (compute_unit = gpu->busy_list_head; compute_unit; compute_unit = compute_unit_next) {

			/* Store next busy compute unit, since this can change
			 * during 'gpu_compute_unit_run' */
			compute_unit_next = compute_unit->busy_next;

			/* Run one cycle */
			gpu_compute_unit_run(compute_unit);
		}

		/* GPU-REL: insert stack faults */
		gpu_stack_faults_insert();
		
		/* Event-driven module */
		esim_process_events();
	}

	/* Finalize */
	gk_timer_stop();
	gpu_unmap_ndrange();

	/* Stop if maximum number of kernels reached */
	if (gpu_max_kernels && gk->ndrange_count >= gpu_max_kernels)
		ke_sim_finish = ke_sim_finish_max_gpu_kernels;
}

