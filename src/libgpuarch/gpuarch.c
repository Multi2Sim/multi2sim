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
#include <options.h>
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
	"      Number of compute units in the GPU. These are the hardware components\n"
	"      where software work-groups are executed.\n"
	"\n"
	"Section '[ ComputeUnit ]':\n"
	"\n"
	"  WavefrontSize = <size> (Default = 64)\n"
	"      Number of work-items within a wavefront which execute AMD Evergreen\n"
	"      instructions in a SIMD fashion.\n"
	"  MaxWorkGroupSize = <size> (default = 256)\n"
	"      Maximum number of work-items within a work-group. This is a device-specific\n"
	"      architectural parameter return by some OpenCL calls.\n"
	"  NumStreamCores = <num> (Default = 16)\n"
	"      Number of stream cores within a compute unit. Each work-item is mapped to a\n"
	"      stream core. Stream cores are time-multiplexed to cover all work-items in a\n"
	"      wavefront.\n"
	"\n"
	"Section '[ LocalMemory ]': defines the parameters of the local memory associated to\n"
	"each compute unit.\n"
	"\n"
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
	"  FetchQueueSize = <size> (Default = 64)\n"
	"      Size in bytes of the fetch queue.\n"
	"  InstructionQueueSize = <num_inst> (Default = 4)\n"
	"      Number of CF instructions that can be stored in the instruction queue.\n"
	"\n"
	"Section '[ ALUEngine ]': parameters for the ALU Engine of the Compute Units.\n"
	"\n"
	"  InstructionMemoryLatency = <cycles> (Default = 2)\n"
	"      Latency of an access to the instruction memory in number of cycles.\n"
	"  FetchQueueSize = <size> (Default = 64)\n"
	"      Size in bytes of the fetch queue.\n"
	"  InstructionQueueSize = <num_inst> (Default = 4)\n"
	"      Number of VLIW bundles that can be stored in the instruction queue.\n"
	"  ProcessingElementLatency = <cycles> (Default = 4)\n"
	"      Latency of each processing element (x, y, z, w, t) of a Stream Core\n"
	"      in number of cycles. This is the time between an instruction is issued\n"
	"      to a Stream Core and the result of the operation is available.\n"
	"\n"
	"Section '[ TEXEngine ]': parameters for the TEX Engine of the Compute Units.\n"
	"\n"
	"  InstructionMemoryLatency = <cycles> (Default = 2)\n"
	"      Latency of an access to the instruction memory in number of cycles.\n"
	"\n";

enum gpu_sim_kind_enum gpu_sim_kind = gpu_sim_kind_functional;

char *gpu_config_file_name = "";

int gpu_pipeline_debug_category;

/* Default parameters based on the AMD Radeon HD 5870 */
int gpu_num_compute_units = 20;
int gpu_num_stream_cores = 16;

/* Number of time multiplexing slots for a stream core among different
 * portions of a wavefront. This parameter is computed as the ceiling
 * of the quotient between the wavefront size and number of stream cores. */
int gpu_compute_unit_time_slots;

/* Maximum number of wavefronts within a work-group */
int gpu_max_wavefront_count;

/* Local memory parameters */
int gpu_local_mem_latency = 2;
int gpu_local_mem_block_size = 16;
int gpu_local_mem_banks = 4;
int gpu_local_mem_read_ports = 2;
int gpu_local_mem_write_ports = 2;

struct gpu_t *gpu;

struct repos_t *gpu_uop_repos;

/* GPU-REL: insertion of faults into stack */
char *gpu_stack_faults_file_name = "";
struct gpu_stack_fault_t {
	long long cycle;
	int compute_unit_id;  /* 0, gpu_num_compute_units - 1 ] */
	int stack_id;  /* [ 0, gpu_max_work_group_size / gpu_wavefront_size - 1 ] */
	int active_mask_id;  /* [ 0, GPU_MAX_STACK_SIZE - 1 ] */
	int bit;  /* [ 0, gpu_wavefront_size - 1 ] */
};
struct lnlist_t *gpu_stack_faults;
int gpu_stack_faults_debug_category;
char *gpu_stack_faults_debug_file_name = "";






/*
 * GPU Uop
 */

static uint64_t gpu_uop_id_counter = 0;

struct gpu_uop_t *gpu_uop_create()
{
	struct gpu_uop_t *uop;

	uop = repos_create_object(gpu_uop_repos);
	uop->id = gpu_uop_id_counter++;
	return uop;
}


static void gpu_uop_add_src_idep(struct gpu_uop_t *uop, struct amd_inst_t *inst, int src_idx)
{
	int sel, rel, chan, neg, abs;

	assert(uop->idep_count < GPU_UOP_MAX_IDEP);
	amd_inst_get_op_src(inst, src_idx, &sel, &rel, &chan, &neg, &abs);

	/* sel = 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
		uop->idep[uop->idep_count++] = GPU_UOP_DEP_REG(sel);

	/* sel = ALU_SRC_PV */
	else if (sel == 254)
		uop->idep[uop->idep_count++] = GPU_UOP_DEP_PV;

	/* sel = ALU_SRC_PS */
	else if (sel == 255)
		uop->idep[uop->idep_count++] = GPU_UOP_DEP_PS;

	/* sel = 219..222: QA, QA.pop, QB, QB.pop */
	else if (IN_RANGE(sel, 219, 222))
		uop->idep[uop->idep_count++] = GPU_UOP_DEP_LDS;
}


struct gpu_uop_t *gpu_uop_create_from_alu_group(struct amd_alu_group_t *alu_group)
{
	struct gpu_uop_t *uop;
	struct amd_inst_t *inst;
	int i;

	/* Create uop */
	uop = gpu_uop_create();

	/* Update dependences */
	for (i = 0; i < alu_group->inst_count; i++) {
		inst = &alu_group->inst[i];

		/* Local memory access instruction */
		if (inst->info->fmt[0] == FMT_ALU_WORD0_LDS_IDX_OP) {

			/* Assume read and write to local memory */
			assert(uop->idep_count < GPU_UOP_MAX_IDEP);
			uop->idep[uop->idep_count++] = GPU_UOP_DEP_LDS;
			assert(uop->odep_count < GPU_UOP_MAX_ODEP);
			uop->odep[uop->odep_count++] = GPU_UOP_DEP_LDS;
		}

		/* Arithmetic instruction */
		else if (inst->info->fmt[0] == FMT_ALU_WORD0) {

			/* Add input dependences */
			gpu_uop_add_src_idep(uop, inst, 0);
			gpu_uop_add_src_idep(uop, inst, 1);
			if (inst->info->fmt[1] == FMT_ALU_WORD1_OP3)
				gpu_uop_add_src_idep(uop, inst, 2);

			/* Add register output dependence if not masked */
			if (inst->info->fmt[1] == FMT_ALU_WORD1_OP3 || inst->words[1].alu_word1_op2.write_mask) {
				assert(uop->odep_count < GPU_UOP_MAX_ODEP);
				uop->odep[uop->odep_count++] = GPU_UOP_DEP_REG(inst->words[1].alu_word1_op2.dst_gpr);
			}

			/* Add PV/PS output dependence */
			assert(uop->odep_count < GPU_UOP_MAX_ODEP);
			uop->odep[uop->odep_count++] = inst->alu == AMD_ALU_TRANS ? GPU_UOP_DEP_PS : GPU_UOP_DEP_PV;
		}
	}

	/* Return */
	return uop;
}


void gpu_uop_free(struct gpu_uop_t *gpu_uop)
{
	repos_free_object(gpu_uop_repos, gpu_uop);
}


void gpu_uop_dump_dep_list(char *buf, int size, int *dep_list, int dep_count)
{
	static struct string_map_t gpu_uop_dep_map = {
		4, {
			{ "none", GPU_UOP_DEP_NONE },
			{ "LDS", GPU_UOP_DEP_LDS },
			{ "PS", GPU_UOP_DEP_PS },
			{ "PV", GPU_UOP_DEP_PV }
		}
	};
	char *comma = "";
	char str[MAX_STRING_SIZE];
	int i;

	dump_buf(&buf, &size, "{");
	for (i = 0; i < dep_count; i++) {
		if (IN_RANGE(dep_list[i], GPU_UOP_DEP_REG_FIRST, GPU_UOP_DEP_REG_LAST))
			sprintf(str, "R%d", dep_list[i] - GPU_UOP_DEP_REG_FIRST);
		else
			strcpy(str, map_value(&gpu_uop_dep_map, dep_list[i]));
		dump_buf(&buf, &size, "%s%s", comma, str);
		comma = ",";
	}
	dump_buf(&buf, &size, "}");
}




/*
 * GPU Compute Unit
 */

struct gpu_compute_unit_t *gpu_compute_unit_create()
{
	struct gpu_compute_unit_t *compute_unit;
	struct gpu_cache_t *local_memory;

	/* Create compute unit */
	compute_unit = calloc(1, sizeof(struct gpu_compute_unit_t));

	/* Local memory */
	compute_unit->local_memory = gpu_cache_create(gpu_local_mem_banks,
		gpu_local_mem_read_ports, gpu_local_mem_write_ports,
		gpu_local_mem_block_size, gpu_local_mem_latency);
	local_memory = compute_unit->local_memory;
	snprintf(local_memory->name, sizeof(local_memory->name),
		"LocalMemory[%d]", compute_unit->id);

	/* Initialize CF Engine */
	compute_unit->cf_engine.wavefront_pool = lnlist_create();
	compute_unit->cf_engine.fetch_buffer = calloc(gpu_max_wavefront_count, sizeof(void *));
	compute_unit->cf_engine.inst_buffer = calloc(gpu_max_wavefront_count, sizeof(void *));
	compute_unit->cf_engine.complete_queue = lnlist_create();

	/* Initialize ALU Engine */
	compute_unit->alu_engine.fetch_queue = lnlist_create();
	compute_unit->alu_engine.event_queue = heap_create(10);

	/* Return */
	return compute_unit;
}


void gpu_compute_unit_free(struct gpu_compute_unit_t *compute_unit)
{
	lnlist_free(compute_unit->cf_engine.wavefront_pool);
	free(compute_unit->cf_engine.fetch_buffer);
	free(compute_unit->cf_engine.inst_buffer);
	lnlist_free(compute_unit->cf_engine.complete_queue);

	lnlist_free(compute_unit->alu_engine.fetch_queue);
	heap_free(compute_unit->alu_engine.event_queue);

	gpu_cache_free(compute_unit->local_memory);
	free(compute_unit);
}


void gpu_compute_unit_map_work_group(struct gpu_compute_unit_t *compute_unit, struct gpu_work_group_t *work_group)
{
	struct gpu_ndrange_t *ndrange = work_group->ndrange;
	struct gpu_wavefront_t *wavefront;
	int wavefront_id;

	/* Map work-group */
	assert(!compute_unit->work_group);
	compute_unit->work_group = work_group;

	/* Delete compute unit from 'idle' list and insert it to 'busy' list. */
	DOUBLE_LINKED_LIST_REMOVE(gpu, idle, compute_unit);
	DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, busy, compute_unit);

	/* Change work-group status to running */
	gpu_work_group_clear_status(work_group, gpu_work_group_pending);
	gpu_work_group_set_status(work_group, gpu_work_group_running);

	/* Initialize compute unit */
	compute_unit->cf_engine.decode_index = 0;
	compute_unit->cf_engine.execute_index = 0;
	compute_unit->cf_engine.finished_wavefronts = 0;

	/* Insert all wavefront into the CF Engine's wavefront pool */
	assert(!lnlist_count(compute_unit->cf_engine.wavefront_pool));
	FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id) {
		wavefront = ndrange->wavefronts[wavefront_id];
		lnlist_add(compute_unit->cf_engine.wavefront_pool, wavefront);
	}

	/* Debug */
	gpu_pipeline_debug("cu a=\"map\" "
		"cu=%d "
		"wg=%d\n",
		compute_unit->id,
		work_group->id);
}


void gpu_compute_unit_unmap_work_group(struct gpu_compute_unit_t *compute_unit)
{
	/* Reset mapped work-group */
	assert(compute_unit->work_group);
	compute_unit->work_group = NULL;

	/* Empty CF Engine's wavefront pool */
	lnlist_clear(compute_unit->cf_engine.wavefront_pool);

	/* Delete compute unit from 'busy' list and insert it to 'idle' list. */
	DOUBLE_LINKED_LIST_REMOVE(gpu, busy, compute_unit);
	DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, idle, compute_unit);

	/* Debug */
	gpu_pipeline_debug("cu a=\"unmap\" "
		"cu=%d\n",
		compute_unit->id);
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void gpu_compute_unit_run(struct gpu_compute_unit_t *compute_unit)
{
	/* Checks */
	assert(compute_unit->work_group);

	/* Run ALU/TEX Engines */
	gpu_alu_engine_run(compute_unit);
	gpu_tex_engine_run(compute_unit);

	/* CF Engine */
	gpu_cf_engine_run(compute_unit);
}





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
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, idle, compute_unit);
	}
}


/* GPU-REL: stack faults */
void gpu_stack_faults_init(void)
{
	FILE *f;
	char line[MAX_STRING_SIZE];
	char *line_ptr;
	struct gpu_stack_fault_t *stack_fault;
	int nelem, line_num;
	long long last_cycle;

	gpu_stack_faults = lnlist_create();
	if (!*gpu_stack_faults_file_name)
		return;

	f = fopen(gpu_stack_faults_file_name, "rt");
	if (!f)
		fatal("%s: cannot open file", gpu_stack_faults_file_name);
	
	line_num = 0;
	last_cycle = 0;
	while (!feof(f)) {
	
		/* Read a line */
		line_num++;
		line_ptr = fgets(line, MAX_STRING_SIZE, f);
		if (!line_ptr)
			break;

		/* Read fields */
		stack_fault = calloc(1, sizeof(struct gpu_stack_fault_t));
		nelem = sscanf(line, "%lld %d %d %d %d",
			&stack_fault->cycle,
			&stack_fault->compute_unit_id,
			&stack_fault->stack_id,
			&stack_fault->active_mask_id,
			&stack_fault->bit);
		if (nelem != 5)
			fatal("%s: line %d: wrong format\n",
				gpu_stack_faults_file_name, line_num);

		/* Check fields */
		if (stack_fault->cycle < 1)
			fatal("%s: line %d: lowest possible cycle is 1",
				gpu_stack_faults_file_name, line_num);
		if (stack_fault->cycle < last_cycle)
			fatal("%s: line %d: cycles must be ordered",
				gpu_stack_faults_file_name, line_num);
		if (stack_fault->compute_unit_id >= gpu_num_compute_units)
			fatal("%s: line %d: invalid compute unit ID",
				gpu_stack_faults_file_name, line_num);
		if (stack_fault->stack_id >= gpu_max_work_group_size / gpu_wavefront_size)
			fatal("%s: line %d: invalid stack ID",
				gpu_stack_faults_file_name, line_num);
		if (stack_fault->active_mask_id >= GPU_MAX_STACK_SIZE)
			fatal("%s: line %d: invalid active mask ID",
				gpu_stack_faults_file_name, line_num);
		if (stack_fault->bit >= gpu_wavefront_size)
			fatal("%s: line %d: invalid bit index",
				gpu_stack_faults_file_name, line_num);

		/* Insert fault */
		lnlist_out(gpu_stack_faults);
		lnlist_insert(gpu_stack_faults, stack_fault);
		last_cycle = stack_fault->cycle;
	}
	lnlist_head(gpu_stack_faults);
}


/* GPU-REL: stack faults */
void gpu_stack_faults_done(void)
{
	while (lnlist_count(gpu_stack_faults)) {
		lnlist_head(gpu_stack_faults);
		free(lnlist_get(gpu_stack_faults));
		lnlist_remove(gpu_stack_faults);
	}
	lnlist_free(gpu_stack_faults);
}


/* GPU-REL: insert stack faults */
void gpu_stack_faults_insert(void)
{
	struct gpu_stack_fault_t *stack_fault;
	struct gpu_compute_unit_t *compute_unit;

	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;

	int value;

	for (;;) {
		lnlist_head(gpu_stack_faults);
		stack_fault = lnlist_get(gpu_stack_faults);
		if (!stack_fault || stack_fault->cycle > gpu->cycle)
			break;

		/* Insert fault */
		gpu_stack_faults_debug("fault cu=%d stack=%d am=%d bit=%d ",
			stack_fault->compute_unit_id, stack_fault->stack_id,
			stack_fault->active_mask_id, stack_fault->bit);
		assert(stack_fault->cycle == gpu->cycle);
		compute_unit = gpu->compute_units[stack_fault->compute_unit_id];

		/* If compute unit is idle, dismiss */
		if (DOUBLE_LINKED_LIST_MEMBER(gpu, idle, compute_unit)) {
			gpu_stack_faults_debug("effect=\"cu_idle\"");
			goto end_loop;
		}

		/* Get work-group and wavefront. If wavefront ID exceeds current number, dimiss */
		work_group = compute_unit->work_group;
		assert(work_group);
		if (stack_fault->stack_id >= work_group->wavefront_count) {
			gpu_stack_faults_debug("effect=\"wf_idle\"");
			goto end_loop;
		}
		wavefront = work_group->wavefronts[stack_fault->stack_id];

		/* If active_mask_id exceeds stack top, dismiss */
		if (stack_fault->active_mask_id > wavefront->stack_top) {
			gpu_stack_faults_debug("effect=\"am_idle\"");
			goto end_loop;
		}

		/* If 'bit' exceeds number of work-items in wavefront, dismiss */
		if (stack_fault->bit >= wavefront->work_item_count) {
			gpu_stack_faults_debug("effect=\"wi_idle\"");
			goto end_loop;
		}

		/* Invert bit */
		value = bit_map_get(wavefront->active_stack, stack_fault->active_mask_id * wavefront->work_item_count
			+ stack_fault->bit, 1);
		bit_map_set(wavefront->active_stack, stack_fault->active_mask_id * wavefront->work_item_count
			+ stack_fault->bit, 1, !value);
		gpu_stack_faults_debug("effect=\"error\"");

end_loop:
		/* Extract and free */
		free(stack_fault);
		lnlist_remove(gpu_stack_faults);
		gpu_stack_faults_debug("\n");
	}
}


void gpu_init()
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
	
	/*
	 * Read configuration file
	 */
	
	/* Device */
	section = "Device";
	gpu_num_compute_units = config_read_int(gpu_config, section, "NumComputeUnits", gpu_num_compute_units);
	if (gpu_num_compute_units < 1)
		fatal("%s: invalid value for 'NumComputeUnits'.\n%s", gpu_config_file_name, err_note);
	
	/* Compute Unit */
	section = "ComputeUnit";
	gpu_wavefront_size = config_read_int(gpu_config, section, "WavefrontSize", gpu_wavefront_size);
	gpu_max_work_group_size = config_read_int(gpu_config, section, "MaxWorkGroupSize", gpu_max_work_group_size);
	gpu_num_stream_cores = config_read_int(gpu_config, section, "NumStreamCores", gpu_num_stream_cores);
	if (gpu_wavefront_size < 1)
		fatal("%s: invalid value for 'WavefrontSize'.\n%s", gpu_config_file_name, err_note);
	if ((gpu_max_work_group_size & (gpu_max_work_group_size - 1)) || gpu_max_work_group_size < 1)
		fatal("%s: 'MaxWorkGroupSize' must be a power of two greater than 0.\n%s",
			gpu_config_file_name, err_note);
	if (gpu_num_stream_cores < 1)
		fatal("%s: invalid value for 'NumStreamCores'.\n%s", gpu_config_file_name, err_note);
	if (gpu_wavefront_size > gpu_max_work_group_size)
		fatal("%s: 'WavefrontSize' should not be larget than 'MaxWorkGroupSize'.\n%s",
			gpu_config_file_name, err_note);
	gpu_max_wavefront_count = (gpu_max_work_group_size + gpu_wavefront_size - 1) / gpu_wavefront_size;
	gpu_compute_unit_time_slots = (gpu_wavefront_size + gpu_num_stream_cores - 1) / gpu_num_stream_cores;
	
	/* Local memory */
	section = "LocalMemory";
	gpu_local_mem_block_size = config_read_int(gpu_config, section, "BlockSize", gpu_local_mem_block_size);
	gpu_local_mem_latency = config_read_int(gpu_config, section, "Latency", gpu_local_mem_latency);
	gpu_local_mem_banks = config_read_int(gpu_config, section, "Banks", gpu_local_mem_banks);
	gpu_local_mem_read_ports = config_read_int(gpu_config, section, "ReadPorts", gpu_local_mem_read_ports);
	gpu_local_mem_write_ports = config_read_int(gpu_config, section, "WritePorts", gpu_local_mem_write_ports);
	if ((gpu_local_mem_block_size & (gpu_local_mem_block_size - 1)) || gpu_local_mem_block_size < 4)
		fatal("%s: %s->BlockSize must be a power of two and at least 4.\n%s",
			gpu_config_file_name, section, err_note);
	if (gpu_local_mem_latency < 1)
		fatal("%s: invalid value for %s->Latency.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_banks < 1 || (gpu_local_mem_banks & (gpu_local_mem_banks - 1)))
		fatal("%s: %s->Banks must be a power of 2 greater than 1.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_read_ports < 1)
		fatal("%s: invalid value for %s->ReadPorts.\n%s", gpu_config_file_name, section, err_note);
	if (gpu_local_mem_write_ports < 1)
		fatal("%s: invalid value for %s->WritePorts.\n%s", gpu_config_file_name, section, err_note);
	
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
	if (gpu_tex_engine_inst_mem_latency < 1)
		fatal("%s: invalid value for %s->InstructionMemoryLatency.\n%s", gpu_config_file_name, section, err_note);
	
	/* Close GPU configuration file */
	config_check(gpu_config);
	config_free(gpu_config);

	/* Initialize GPU */
	gpu_init_device();

	/* GPU uop repository.
	 * The size assigned for each 'gpu_uop_t' is equals to the baseline structure size plus the
	 * size of a 'gpu_work_item_uop_t' element for each work-item in the wavefront. */
	gpu_uop_repos = repos_create(sizeof(struct gpu_uop_t) + sizeof(struct gpu_work_item_uop_t)
		* gpu_wavefront_size, "gpu_uop_repos");
	
	/* Cache system */
	gpu_cache_init();

	/* GPU-REL: read stack faults file */
	gpu_stack_faults_init();
}


void gpu_done()
{
	struct gpu_compute_unit_t *compute_unit;
	int compute_unit_id;

	/* Cache system */
	gpu_cache_done();

	/* Free stream cores, compute units, and device */
	FOREACH_COMPUTE_UNIT(compute_unit_id) {
		compute_unit = gpu->compute_units[compute_unit_id];
		gpu_compute_unit_free(compute_unit);
	}
	free(gpu->compute_units);
	free(gpu);
	
	/* GPU uop repository */
	repos_free(gpu_uop_repos);

	/* GPU-REL: read stack faults file */
	gpu_stack_faults_done();
}


void gpu_run(struct gpu_ndrange_t *ndrange)
{
	struct opencl_kernel_t *kernel = ndrange->kernel;
	struct gpu_compute_unit_t *compute_unit, *compute_unit_next;

	/* Debug */
	gpu_pipeline_debug("init "
		"global_size=%d "
		"local_size=%d "
		"group_count=%d "
		"wavefront_size=%d "
		"wavefronts_per_work_group=%d"
		"\n",
		kernel->global_size,
		kernel->local_size,
		kernel->group_count,
		gpu_wavefront_size,
		ndrange->wavefronts_per_work_group);

	gpu->ndrange = ndrange;

	for (;;) {
		
		/* Assign pending work-items to idle compute units. */
		while (gpu->idle_list_head && ndrange->pending_list_head)
			gpu_compute_unit_map_work_group(gpu->idle_list_head, ndrange->pending_list_head);
		
		/* If no compute unit got any work, ND-Range finished execution. */
		if (!gpu->busy_list_head)
			break;

		/* Next cycle */
		gpu->cycle++;
		gpu_pipeline_debug("clk c=%lld\n", (long long) gpu->cycle);
		
		/* Advance one cycle on each busy compute unit */
		for (compute_unit = gpu->busy_list_head; compute_unit; compute_unit = compute_unit_next) {
			
			/* Save next non-idle compute unit */
			compute_unit_next = compute_unit->busy_next;

			/* Simulate cycle in compute unit */
			gpu_compute_unit_run(compute_unit);
		}

		/* GPU-REL: insert stack faults */
		gpu_stack_faults_insert();
		
		/* Event-driven module */
		esim_process_events();
	}

	/* Dump stats */
	gpu_ndrange_dump(ndrange, gpu_report_file);
}

