/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef EVERGREEN_TIMING_H
#define EVERGREEN_TIMING_H

#include <evergreen-emu.h>
#include <mem-system.h>



/* Public variables */

extern char *gpu_config_help;
extern char *gpu_config_file_name;
extern char *gpu_report_file_name;
extern char *mem_report_file_name;

extern int gpu_pipeline_debug_category;

extern int gpu_num_stream_cores;
extern int gpu_num_compute_units;
extern int gpu_num_registers;
extern int gpu_register_alloc_size;

extern struct string_map_t gpu_register_alloc_granularity_map;
extern enum gpu_register_alloc_granularity_t
{
	gpu_register_alloc_invalid = 0,  /* For invalid user input */
	gpu_register_alloc_wavefront,
	gpu_register_alloc_work_group
} gpu_register_alloc_granularity;

extern int gpu_max_work_groups_per_compute_unit;
extern int gpu_max_wavefronts_per_compute_unit;

extern struct string_map_t gpu_sched_policy_map;
extern enum gpu_sched_policy_t
{
	gpu_sched_invalid = 0,  /* For invalid user input */
	gpu_sched_round_robin,
	gpu_sched_greedy
} gpu_sched_policy;

extern char *gpu_calc_file_name;

extern int gpu_local_mem_size;
extern int gpu_local_mem_alloc_size;
extern int gpu_local_mem_latency;
extern int gpu_local_mem_block_size;
extern int gpu_local_mem_num_ports;

extern int gpu_cf_engine_inst_mem_latency;

extern int gpu_alu_engine_inst_mem_latency;
extern int gpu_alu_engine_fetch_queue_size;
extern int gpu_alu_engine_pe_latency;

extern int gpu_tex_engine_inst_mem_latency;
extern int gpu_tex_engine_fetch_queue_size;
extern int gpu_tex_engine_load_queue_size;

extern struct gpu_t *gpu;



/*
 * GPU-REL
 */

#define gpu_faults_debug(...) debug(gpu_faults_debug_category, __VA_ARGS__)
extern int gpu_faults_debug_category;

extern char *gpu_faults_debug_file_name;
extern char *gpu_faults_file_name;

void gpu_faults_init(void);
void gpu_faults_done(void);

void gpu_faults_insert(void);





/*
 * GPU uop
 */

/* Debugging */
#define gpu_stack_debug(...) debug(gpu_stack_debug_category, __VA_ARGS__)
extern int gpu_stack_debug_category;


/* Part of a GPU instruction specific for each work-item within wavefront. */
struct gpu_work_item_uop_t
{
	/* For global memory accesses */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Flags */
	unsigned int active : 1;  /* Active after instruction emulation */

	/* Local memory access */
	int local_mem_access_count;
	enum mod_access_kind_t local_mem_access_kind[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_addr[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
};

#define GPU_UOP_MAX_IDEP      (3 * 5)
#define GPU_UOP_MAX_ODEP      (3 * 5)

#define GPU_UOP_DEP_NONE         0
#define GPU_UOP_DEP_REG_FIRST    1
#define GPU_UOP_DEP_REG(X)       ((X) + 1)
#define GPU_UOP_DEP_REG_LAST     128
#define GPU_UOP_DEP_PV           129
#define GPU_UOP_DEP_PS           130
#define GPU_UOP_DEP_LDS          131
#define GPU_UOP_DEP_COUNT        132

/* Structure representing a GPU instruction fetched in common for a wavefront.
 * This is the structure passed from stage to stage in the compute unit pipeline. */
struct gpu_uop_t
{
	/* Fields */
	long long id;
	long long id_in_compute_unit;
	struct evg_wavefront_t *wavefront;  /* Wavefront it belongs to */
	struct evg_work_group_t *work_group;  /* Work-group it belongs to */
	struct gpu_compute_unit_t *compute_unit;  /* Compute unit it belongs to */
	struct gpu_uop_t *cf_uop;  /* For ALU/TEX uops, CF uop that triggered clause */
	int length;  /* Number of bytes occupied by ALU group */

	/* CF instruction flags */
	unsigned int alu_clause_trigger : 1;  /* Instruction triggers ALU clause */
	unsigned int tex_clause_trigger : 1;  /* Instruction triggers TEX clause */
	unsigned int no_clause_trigger : 1;  /* Instruction does not trigger secondary clause */

	/* ALU group flags */
	unsigned int ready : 1;

	/* Flags */
	unsigned int last : 1;  /* Last instruction in the clause */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int active_mask_update : 1;
	int active_mask_push;  /* Number of entries the stack was pushed */
	int active_mask_pop;  /* Number of entries the stack was popped */
	int active_mask_stack_top;  /* Top of stack */

	/* Witness memory accesses */
	long long inst_mem_ready;  /* Cycle when instruction memory access completes */
	int global_mem_witness;
	int local_mem_witness;

	/* ALU Engine - subwavefronts */
	int subwavefront_count;
	int exec_subwavefront_count;
	int write_subwavefront_count;

	/* ALU instructions - input/output dependencies */
	int idep[GPU_UOP_MAX_IDEP];
	int odep[GPU_UOP_MAX_ODEP];
	int idep_count;
	int odep_count;

	/* Double linked lists of producer-consumers */
	struct gpu_uop_t *dep_list_next;
	struct gpu_uop_t *dep_list_prev;
	struct gpu_uop_t *dep_list_head;
	struct gpu_uop_t *dep_list_tail;
	int dep_list_count;
	int dep_list_max;

	/* Per stream-core data. This space is dynamically allocated for an uop.
	 * It should be always the last field of the structure. */
	struct gpu_work_item_uop_t work_item_uop[0];
};

void gpu_uop_init(void);
void gpu_uop_done(void);

struct gpu_uop_t *gpu_uop_create(void);
struct gpu_uop_t *gpu_uop_create_from_alu_group(struct evg_alu_group_t *alu_group);
void gpu_uop_free(struct gpu_uop_t *gpu_uop);

void gpu_uop_list_free(struct linked_list_t *gpu_uop_list);
void gpu_uop_dump_dep_list(char *buf, int size, int *dep_list, int dep_count);

void gpu_uop_save_active_mask(struct gpu_uop_t *uop);
void gpu_uop_debug_active_mask(struct gpu_uop_t *uop);




/*
 * Register file in Compute Unit
 */

struct gpu_reg_file_t;

void gpu_reg_file_init(struct gpu_compute_unit_t *compute_unit);
void gpu_reg_file_done(struct gpu_compute_unit_t *compute_unit);

void gpu_reg_file_map_work_group(struct gpu_compute_unit_t *compute_unit,
	struct evg_work_group_t *work_group);
void gpu_reg_file_unmap_work_group(struct gpu_compute_unit_t *compute_unit,
	struct evg_work_group_t *work_group);

int gpu_reg_file_rename(struct gpu_compute_unit_t *compute_unit,
	struct evg_work_item_t *work_item, int logical_register);
void gpu_reg_file_inverse_rename(struct gpu_compute_unit_t *compute_unit,
	int physical_register, struct evg_work_item_t **work_item, int *logical_register);





/*
 * GPU Compute Unit
 */

struct gpu_compute_unit_t
{
	/* IDs */
	int id;
	long long gpu_uop_id_counter;  /* Counter to assign 'id_in_compute_unit' to uops */

	/* Double linked list of compute units */
	struct gpu_compute_unit_t *ready_list_prev;
	struct gpu_compute_unit_t *ready_list_next;
	struct gpu_compute_unit_t *busy_list_prev;
	struct gpu_compute_unit_t *busy_list_next;

	/* Entry points to memory hierarchy */
	struct mod_t *global_memory;
	struct mod_t *local_memory;

	/* List of currently mapped work-groups */
	int work_group_count;
	struct evg_work_group_t **work_groups;

	/* Statistics */
	long long mapped_work_groups;
	long long cycle;
	long long inst_count;

	/* Ready wavefront pool.
	 * It includes suspended wavefronts, but excludes wavefronts in
	 * flight in the CF pipeline or running on the ALU/TEX Engines. */
	struct linked_list_t *wavefront_pool;

	/* Fields for CF Engine */
	struct
	{
		/* Buffers */
		struct gpu_uop_t **fetch_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */
		struct gpu_uop_t **inst_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */

		/* Wavefront selectors */
		int decode_index;  /* Next uop in 'fetch_buffer' to decode */
		int execute_index;  /* Next uop in 'inst_buffer' to execute */

		/* Complete queue */
		struct linked_list_t *complete_queue;  /* Queue of completed instructions */

		/* Statistics */
		long long inst_count;
		long long alu_clause_trigger_count;
		long long tex_clause_trigger_count;
		long long global_mem_write_count;

	} cf_engine;

	/* Fields for ALU Engine */
	struct
	{
		/* CF uop queues.
		 * The CF engine enqueues a CF uop that triggers an ALU clause into 'pending_queue'.
		 * When the secondary clause finishes fetching, the CF uop is moved to
		 * 'finished_queue'. When secondary clause exists pipeline, it is extracted from
		 " 'finished_queue'. */
		struct linked_list_t *pending_queue;
		struct linked_list_t *finished_queue;

		/* Queues */
		struct linked_list_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct gpu_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct gpu_uop_t *exec_buffer;  /* Uop from read to execute stage */
		struct heap_t *event_queue;  /* Events for instruction execution */

		/* Table storing the in-flight uop that produced an output
		 * dependence. If the producer is not in flight, the value is NULL. */
		struct gpu_uop_t *producers[GPU_UOP_DEP_COUNT];

		/* Statistics */
		long long wavefront_count;
		long long cycle;
		long long inst_count;
		long long inst_slot_count;
		long long local_mem_slot_count;
		long long vliw_slots[5];

	} alu_engine;

	/* Fields for TEX Engine */
	struct
	{
		/* CF uop queues (see comment for ALU engine) */
		struct linked_list_t *pending_queue;
		struct linked_list_t *finished_queue;

		/* Queues */
		struct linked_list_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct gpu_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct linked_list_t *load_queue;  /* Uops from read to write stage */

		/* Statistics */
		long long wavefront_count;
		long long cycle;
		long long inst_count;

	} tex_engine;

};

struct gpu_compute_unit_t *gpu_compute_unit_create();
void gpu_compute_unit_free(struct gpu_compute_unit_t *gpu_compute_unit);
void gpu_compute_unit_map_work_group(struct gpu_compute_unit_t *compute_unit, struct evg_work_group_t *work_group);
void gpu_compute_unit_unmap_work_group(struct gpu_compute_unit_t *compute_unit, struct evg_work_group_t *work_group);

struct evg_wavefront_t *gpu_schedule(struct gpu_compute_unit_t *compute_unit);

void gpu_cf_engine_run(struct gpu_compute_unit_t *compute_unit);
void gpu_alu_engine_run(struct gpu_compute_unit_t *compute_unit);
void gpu_tex_engine_run(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_run(struct gpu_compute_unit_t *compute_unit);




/*
 * GPU Calculator
 */

int gpu_calc_get_work_groups_per_compute_unit(int work_items_per_work_group,
	int registers_per_work_item, int local_mem_per_work_group);
void gpu_calc_plot(void);




/*
 * GPU Device
 */

/* Debugging */
#define gpu_pipeline_debug(...) debug(gpu_pipeline_debug_category, __VA_ARGS__)
#define gpu_pipeline_debugging() debug_status(gpu_pipeline_debug_category)
extern int gpu_pipeline_debug_category;

struct gpu_t
{
	/* Current cycle */
	long long cycle;

	/* ND-Range running on it */
	struct evg_ndrange_t *ndrange;
	int work_groups_per_compute_unit;
	int wavefronts_per_compute_unit;
	int work_items_per_compute_unit;

	/* Compute units */
	struct gpu_compute_unit_t **compute_units;

	/* List of ready compute units accepting work-groups */
	struct gpu_compute_unit_t *ready_list_head;
	struct gpu_compute_unit_t *ready_list_tail;
	int ready_list_count;
	int ready_list_max;

	/* List of busy compute units */
	struct gpu_compute_unit_t *busy_list_head;
	struct gpu_compute_unit_t *busy_list_tail;
	int busy_list_count;
	int busy_list_max;
};

#define FOREACH_COMPUTE_UNIT(COMPUTE_UNIT_ID) \
	for ((COMPUTE_UNIT_ID) = 0; (COMPUTE_UNIT_ID) < gpu_num_compute_units; (COMPUTE_UNIT_ID)++)

#define FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(WAVEFRONT, SUBWAVEFRONT_ID, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first + (SUBWAVEFRONT_ID) * gpu_num_stream_cores; \
		(WORK_ITEM_ID) <= MIN((WAVEFRONT)->work_item_id_first + ((SUBWAVEFRONT_ID) + 1) \
			* gpu_num_stream_cores - 1, (WAVEFRONT)->work_item_id_last); \
		(WORK_ITEM_ID)++)

void gpu_init(void);
void gpu_done(void);

void gpu_dump_report(void);

void gpu_run(struct evg_ndrange_t *ndrange);


#endif

