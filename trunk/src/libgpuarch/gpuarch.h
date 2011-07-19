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

#ifndef GPUARCH_H
#define GPUARCH_H

#include <gpukernel.h>



/* Public variables */

extern char *gpu_config_help;
extern char *gpu_config_file_name;
extern char *gpu_report_file_name;
extern char *gpu_cache_report_file_name;

extern int gpu_pipeline_debug_category;

extern enum gpu_sim_kind_enum {
	gpu_sim_kind_functional,
	gpu_sim_kind_detailed
} gpu_sim_kind;

extern int gpu_num_stream_cores;
extern int gpu_num_compute_units;
extern int gpu_num_registers;
extern int gpu_register_alloc_size;
extern enum gpu_register_alloc_granularity_enum {
	gpu_register_alloc_wavefront,
	gpu_register_alloc_work_group
} gpu_register_alloc_granularity;
extern int gpu_max_work_groups_per_compute_unit;
extern int gpu_max_wavefronts_per_compute_unit;
extern char *gpu_calc_file_name;

extern int gpu_local_mem_size;
extern int gpu_local_mem_alloc_size;
extern int gpu_local_mem_latency;
extern int gpu_local_mem_block_size;
extern int gpu_local_mem_banks;
extern int gpu_local_mem_read_ports;
extern int gpu_local_mem_write_ports;

extern int gpu_cf_engine_inst_mem_latency;

extern int gpu_alu_engine_inst_mem_latency;
extern int gpu_alu_engine_fetch_queue_size;
extern int gpu_alu_engine_pe_latency;

extern int gpu_tex_engine_inst_mem_latency;
extern int gpu_tex_engine_fetch_queue_size;

extern struct gpu_t *gpu;

/* GPU-REL: insertion of faults into stack */
#define gpu_stack_faults_debug(...) debug(gpu_stack_faults_debug_category, __VA_ARGS__)
extern int gpu_stack_faults_debug_category;
extern char *gpu_stack_faults_debug_file_name;
extern char *gpu_stack_faults_file_name;




/*
 * GPU uop
 */

/* Part of a GPU instruction specific for each work-item within wavefront. */
struct gpu_work_item_uop_t {
	
	/* For global memory accesses */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Local memory access */
	int local_mem_access_count;
	int local_mem_access_type[MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
	uint32_t local_mem_access_addr[MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[MAX_LOCAL_MEM_ACCESSES_PER_INST];
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
	uint64_t id;
	struct gpu_wavefront_t *wavefront;  /* Wavefront it belongs to */
	struct gpu_work_group_t *work_group;  /* Work-group it belongs to */
	int length;  /* Number of bytes occupied by ALU group */

	/* CF instruction flags */
	unsigned int alu_clause_trigger : 1;  /* Instruction triggers ALU clause */
	unsigned int tex_clause_trigger : 1;  /* Instruction triggers TEX clause */
	unsigned int no_clause_trigger : 1;  /* Instruction does not trigger secondary clause */

	/* ALU group flags */
	unsigned int ready : 1;
	unsigned int last : 1;

	/* Flags */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;

	/* Witness memory accesses */
	uint64_t inst_mem_ready;  /* Cycle when instruction memory access completes */
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
	struct gpu_uop_t *dep_next, *dep_prev;
	struct gpu_uop_t *dep_list_head, *dep_list_tail;
	int dep_count, dep_max;

	/* Per stream-core data. This space is dynamically allocated for an uop.
	 * It should be always the last field of the structure. */
	struct gpu_work_item_uop_t work_item_uop[0];
};

struct gpu_uop_t *gpu_uop_create();
struct gpu_uop_t *gpu_uop_create_from_alu_group(struct amd_alu_group_t *alu_group);
void gpu_uop_free(struct gpu_uop_t *gpu_uop);
void gpu_uop_list_free(struct lnlist_t *gpu_uop_list);

void gpu_uop_dump_dep_list(char *buf, int size, int *dep_list, int dep_count);




/* GPU Compute Unit */

struct gpu_compute_unit_t
{
	/* ID */
	int id;

	/* Double linked list of compute units */
	struct gpu_compute_unit_t *ready_prev, *ready_next;
	struct gpu_compute_unit_t *busy_prev, *busy_next;

	/* Memory */
	struct gpu_cache_t *data_cache;  /* Entry point to global memory */
	struct gpu_cache_t *local_memory;  /* Local */

	/* List of currently mapped work-groups */
	int work_group_count;
	struct gpu_work_group_t **work_groups;

	/* Stats */
	uint64_t mapped_work_groups;
	uint64_t cycle;
	uint64_t inst_count;

	/* Fields for CF Engine */
	struct {
		
		/* Ready wavefront pool.
		 * It includes suspended wavefronts, but excludes wavefronts in
		 * flight in the CF pipeline or running on the ALU/TEX Engines. */
		struct lnlist_t *wavefront_pool;

		/* Buffers */
		struct gpu_uop_t **fetch_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */
		struct gpu_uop_t **inst_buffer;  /* Array of uops (MaxWavefrontsPerComputeUnit elements) */

		/* Wavefront selectors */
		int decode_index;  /* Next uop in 'fetch_buffer' to decode */
		int execute_index;  /* Next uop in 'inst_buffer' to execute */

		/* Complete queue */
		struct lnlist_t *complete_queue;  /* Queue of completed instructions */

		/* Stats */
		uint64_t inst_count;

	} cf_engine;

	/* Fields for ALU Engine */
	struct {
		
		/* Current wavefront running in ALU Engine */
		struct gpu_wavefront_t *wavefront;
		struct gpu_uop_t *cf_uop;  /* CF instruction triggering ALU clause */

		/* Queues */
		struct lnlist_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct gpu_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct gpu_uop_t *exec_buffer;  /* Uop from read to execute stage */
		struct heap_t *event_queue;  /* Events for instruction execution */

		/* Table storing the in-flight uop that produced an output
		 * dependence. If the producer is not in flight, the value is NULL. */
		struct gpu_uop_t *producers[GPU_UOP_DEP_COUNT];

		/* Stats */
		uint64_t wavefront_count;
		uint64_t cycle;
		uint64_t inst_count;

	} alu_engine;

	/* Fields for TEX Engine */
	struct {
		struct gpu_uop_t *cf_uop;  /* CF instruction triggering TEX clause */
		struct gpu_wavefront_t *wavefront;

		/* Queues */
		struct lnlist_t *fetch_queue;  /* Uops from fetch to decode stage */
		int fetch_queue_length;  /* Number of bytes occupied in fetch queue */
		struct gpu_uop_t *inst_buffer;  /* Uop from decode to read stage */
		struct gpu_uop_t *write_buffer;  /* Uop from read to write stage */

		/* Stats */
		uint64_t wavefront_count;
		uint64_t cycle;
		uint64_t inst_count;

	} tex_engine;

};

struct gpu_compute_unit_t *gpu_compute_unit_create();
void gpu_compute_unit_free(struct gpu_compute_unit_t *gpu_compute_unit);
void gpu_compute_unit_map_work_group(struct gpu_compute_unit_t *compute_unit, struct gpu_work_group_t *work_group);
void gpu_compute_unit_unmap_work_group(struct gpu_compute_unit_t *compute_unit, struct gpu_work_group_t *work_group);

void gpu_cf_engine_run(struct gpu_compute_unit_t *compute_unit);
void gpu_alu_engine_run(struct gpu_compute_unit_t *compute_unit);
void gpu_tex_engine_run(struct gpu_compute_unit_t *compute_unit);




/*
 * Cache system
 */

extern char *gpu_cache_config_file_name;
extern char *gpu_cache_config_help;


/* GPU Cache Port */
struct gpu_cache_port_t
{
	/* Port lock status */
	int locked;
	uint64_t lock_when;
	struct gpu_cache_stack_t *stack;  /* Current access */
	
	/* Waiting list */
	struct gpu_cache_stack_t *waiting_list_head, *waiting_list_tail;
	int waiting_count, waiting_max;

};


/* GPU Cache Bank */
struct gpu_cache_bank_t
{
	/* Stats */
	uint64_t accesses;

	/* Ports */
	struct gpu_cache_port_t ports[0];
};


#define SIZEOF_GPU_CACHE_BANK(CACHE) (sizeof(struct gpu_cache_bank_t) + sizeof(struct gpu_cache_port_t) \
	* ((CACHE)->read_port_count + (CACHE)->write_port_count))

#define GPU_CACHE_BANK_INDEX(CACHE, I)  ((struct gpu_cache_bank_t *) ((void *) (CACHE)->banks + SIZEOF_GPU_CACHE_BANK(CACHE) * (I)))

#define GPU_CACHE_READ_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(I)])
#define GPU_CACHE_WRITE_PORT_INDEX(CACHE, BANK, I)  (&(BANK)->ports[(CACHE)->read_port_count + (I)])


struct gpu_cache_t
{
	/* Actual cache structure */
	struct cache_t *cache;

	/* Parameters */
	char name[30];
	int block_size, log_block_size;
	int latency;

	/* Banks and ports */
	struct gpu_cache_bank_t *banks;
	int bank_count;
	int read_port_count;  /* Number of read ports (per bank) */
	int write_port_count;  /* Number of write ports (per bank) */

	/* Number of locked read/write ports (adding up all banks) */
	int locked_read_port_count;
	int locked_write_port_count;

	/* Waiting list of events */
	struct gpu_cache_stack_t *waiting_list_head, *waiting_list_tail;
	int waiting_count, waiting_max;

	/* Lower level cache (NULL for global memory) */
	struct gpu_cache_t *gpu_cache_next;

	/* Interconnects and IDs */
	struct net_t *net_hi;
	struct net_t *net_lo;
	int id_hi;
	int id_lo;

	/* Stats */
	uint64_t reads;
	uint64_t effective_reads;
	uint64_t effective_read_hits;
	uint64_t writes;
	uint64_t effective_writes;
	uint64_t effective_write_hits;
	uint64_t evictions;
};

#define gpu_cache_debugging() debug_status(gpu_cache_debug_category)
#define gpu_cache_debug(...) debug(gpu_cache_debug_category, __VA_ARGS__)
extern int gpu_cache_debug_category;

struct gpu_cache_t *gpu_cache_create(int bank_count, int read_port_count, int write_port_count,
	int block_size, int latency);
void gpu_cache_free(struct gpu_cache_t *gpu_cache);
void gpu_cache_dump(struct gpu_cache_t *gpu_cache, FILE *f);

void gpu_cache_init(void);
void gpu_cache_done(void);
void gpu_cache_dump_report(void);

void gpu_cache_access(struct gpu_cache_t *gpu_cache, int access, uint32_t addr, uint32_t size, int *witness_ptr);




/*
 * GPU Cache system - event driven simulation
 */

extern int EV_GPU_CACHE_READ;
extern int EV_GPU_CACHE_READ_REQUEST;
extern int EV_GPU_CACHE_READ_REQUEST_RECEIVE;
extern int EV_GPU_CACHE_READ_REQUEST_REPLY;
extern int EV_GPU_CACHE_READ_REQUEST_FINISH;
extern int EV_GPU_CACHE_READ_UNLOCK;
extern int EV_GPU_CACHE_READ_FINISH;

extern int EV_GPU_CACHE_WRITE;
extern int EV_GPU_CACHE_WRITE_REQUEST_RECEIVE;
extern int EV_GPU_CACHE_WRITE_UNLOCK;
extern int EV_GPU_CACHE_WRITE_FINISH;

/* Stack for event-driven simulation */
struct gpu_cache_stack_t
{
	uint64_t id;
	int *witness_ptr;
	struct gpu_cache_t *gpu_cache;
	struct gpu_cache_bank_t *bank;
	struct gpu_cache_port_t *port;
	uint32_t addr;
	uint32_t tag;
	uint32_t set;
	uint32_t way;
	int status;
	uint32_t block_index;
	uint32_t bank_index;
	int read_port_index;
	int write_port_index;
	int pending;
	int hit;

	/* Linked list for waiting events */
	int waiting_list_event;  /* Event to schedule when stack is woken up */
	struct gpu_cache_stack_t *waiting_prev, *waiting_next;

	/* Return stack */
	struct gpu_cache_stack_t *ret_stack;
	int ret_event;
};

extern struct repos_t *gpu_cache_stack_repos;
extern uint64_t gpu_cache_stack_id;

struct gpu_cache_stack_t *gpu_cache_stack_create(uint64_t id, struct gpu_cache_t *gpu_cache,
	uint32_t addr, int ret_event, void *ret_stack);
void gpu_cache_stack_return(struct gpu_cache_stack_t *stack);

void gpu_cache_handler_read(int event, void *data);
void gpu_cache_handler_write(int event, void *data);




/*
 * GPU Calculator
 */

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
	uint64_t cycle;

	/* ND-Range running on it */
	struct gpu_ndrange_t *ndrange;
	int work_groups_per_compute_unit;
	int wavefronts_per_compute_unit;
	int work_items_per_compute_unit;

	/* Compute units */
	struct gpu_compute_unit_t **compute_units;
	struct gpu_compute_unit_t *ready_list_head, *ready_list_tail;  /* CUs accepting work-groups */
	struct gpu_compute_unit_t *busy_list_head, *busy_list_tail;  /* CUs with at least one work-group */
	int ready_count, ready_max;
	int busy_count, busy_max;

	/* Global memory hierarchy - Caches and interconnects */
	struct gpu_cache_t **gpu_caches;  /* Array of GPU caches */
	struct net_t **networks;  /* Array of interconnects */
	int gpu_cache_count;
	int network_count;
	struct gpu_cache_t *global_memory;  /* Last element in cache array */
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

void gpu_run(struct gpu_ndrange_t *ndrange);


#endif

