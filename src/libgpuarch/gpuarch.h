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

extern int gpu_pipeline_debug_category;

extern enum gpu_sim_kind_enum {
	gpu_sim_kind_functional,
	gpu_sim_kind_detailed
} gpu_sim_kind;

extern int gpu_num_stream_cores;
extern int gpu_num_compute_units;
extern int gpu_compute_unit_time_slots;

extern struct gpu_t *gpu;

/* GPU-REL: insertion of faults into stack */
#define gpu_stack_faults_debug(...) debug(gpu_stack_faults_debug_category, __VA_ARGS__)
extern int gpu_stack_faults_debug_category;
extern char *gpu_stack_faults_debug_file_name;
extern char *gpu_stack_faults_file_name;




/* GPU Stream Core */

struct gpu_stream_core_t {
	
	/* ID */
	int id;

	/* Compute unit it belongs to */
	struct gpu_compute_unit_t *compute_unit;

};

struct gpu_stream_core_t *gpu_stream_core_create();
void gpu_stream_core_free(struct gpu_stream_core_t *gpu_stream_core);




/* GPU Compute Unit */

struct gpu_compute_unit_t
{
	/* ID */
	int id;

	/* Stream cores */
	struct gpu_stream_core_t **stream_cores;

	/* Double linked list of compute units */
	struct gpu_compute_unit_t *idle_prev, *idle_next;
	struct gpu_compute_unit_t *busy_prev, *busy_next;

	/* Entry points to global memory hierarchy */
	struct gpu_cache_t *gpu_cache_inst;
	struct gpu_cache_t *gpu_cache_data;

	/* Initial pipe register (for Schedule stage state) */
	struct {
		
		int input_ready;

		/* Programmable */
		int work_group_id;
		int wavefront_id;
		int subwavefront_id;

		/* State */
		struct gpu_wavefront_t *wavefront_running_next;
		struct gpu_uop_t *uop;

	} init_schedule;

	/* Schedule/Fetch pipe register */
	struct {
		
		int input_ready;

		/* Programmable by 'schedule' stage */
		struct gpu_uop_t *uop;
		int subwavefront_id;

	} schedule_fetch;

	/* Fetch/Decode pipe register */
	struct {
		
		int input_ready;

		/* Programmable by 'fetch' stage */
		struct gpu_uop_t *uop;
		int subwavefront_id;

	} fetch_decode;

	/* Decode/Read pipe register */
	struct {
		
		int input_ready;
		int stall_cycle;

		/* Programmable by 'decode' stage */
		struct gpu_uop_t *uop;
		int subwavefront_id;
	
	} decode_read;

	/* Read/Execute pipe register */
	struct {
		
		int input_ready;

		/* Programmable by 'read' stage */
		struct gpu_uop_t *uop;
		int subwavefront_id;

	} read_execute;

	/* Execute/Write pipe register */
	struct {
		
		int input_ready;

		/* Programmable by 'execute' stage */
		struct gpu_uop_t *uop;
		int subwavefront_id;

	} execute_write;
};

/* Macros for quick access to pipe registers */
#define INIT_SCHEDULE  (compute_unit->init_schedule)
#define SCHEDULE_FETCH  (compute_unit->schedule_fetch)
#define FETCH_DECODE  (compute_unit->fetch_decode)
#define DECODE_READ  (compute_unit->decode_read)
#define READ_EXECUTE  (compute_unit->read_execute)
#define EXECUTE_WRITE  (compute_unit->execute_write)


struct gpu_compute_unit_t *gpu_compute_unit_create();
void gpu_compute_unit_free(struct gpu_compute_unit_t *gpu_compute_unit);

void gpu_compute_unit_schedule(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_fetch(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_decode(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_read(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_execute(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_write(struct gpu_compute_unit_t *compute_unit);
void gpu_compute_unit_next_cycle(struct gpu_compute_unit_t *compute_unit);




/* GPU uop */

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

/* Structure representing a GPU instruction fetched in common for a wavefront.
 * This is the structure passed from stage to stage in the compute unit pipeline. */
struct gpu_uop_t
{
	/* IDs */
	uint64_t id;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	struct gpu_compute_unit_t *compute_unit;
	int subwavefront_count;
	int last;  /* 1 if last uop in work-group */

	/* Instruction */
	struct amd_inst_t inst;
	struct amd_alu_group_t alu_group;
	
	/* Clause kind */
	enum gpu_clause_kind_enum clause_kind;

	/* Flags copied after instruction emulation */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;

	/* Witness for global memory access */
	int global_mem_access_witness;

	/* Per stream-core data. This space is dynamically allocated for an uop.
	 * It should be always the last field of the structure. */
	struct gpu_work_item_uop_t work_item_uop[0];
};

struct gpu_uop_t *gpu_uop_create();
void gpu_uop_free(struct gpu_uop_t *gpu_uop);




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
	uint32_t block_size, log_block_size;
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
};

#define gpu_cache_debugging() debug_status(gpu_cache_debug_category)
#define gpu_cache_debug(...) debug(gpu_cache_debug_category, __VA_ARGS__)
extern int gpu_cache_debug_category;

struct gpu_cache_t *gpu_cache_create(int bank_count, int read_port_count, int write_port_count);
void gpu_cache_free(struct gpu_cache_t *gpu_cache);
void gpu_cache_dump(struct gpu_cache_t *gpu_cache, FILE *f);

void gpu_cache_init(void);
void gpu_cache_done(void);

void gpu_cache_access(int compute_unit_id, int access, uint32_t addr, uint32_t size, int *witness_ptr);



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

	/* Compute units */
	struct gpu_compute_unit_t **compute_units;

	/* Double linked lists of compute units */
	struct gpu_compute_unit_t *idle_list_head, *idle_list_tail;
	struct gpu_compute_unit_t *busy_list_head, *busy_list_tail;
	int idle_count, idle_max;
	int busy_count, busy_max;

	/* Global memory hierarchy - Caches and interconnects */
	struct gpu_cache_t **gpu_caches;  /* Array of GPU caches */
	struct net_t **networks;  /* Array of interconnects */
	int gpu_cache_count;
	int network_count;
	struct gpu_cache_t *global_memory;  /* Last element in cache array */
};

#define FOREACH_STREAM_CORE(STREAM_CORE_ID) \
	for ((STREAM_CORE_ID) = 0; (STREAM_CORE_ID) < gpu_num_stream_cores; (STREAM_CORE_ID)++)

#define FOREACH_COMPUTE_UNIT(COMPUTE_UNIT_ID) \
	for ((COMPUTE_UNIT_ID) = 0; (COMPUTE_UNIT_ID) < gpu_num_compute_units; (COMPUTE_UNIT_ID)++)

#define FOREACH_WORK_ITEM_IN_SUBWAVEFRONT(WAVEFRONT, SUBWAVEFRONT_ID, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first + (SUBWAVEFRONT_ID) * gpu_num_stream_cores; \
		(WORK_ITEM_ID) <= MIN((WAVEFRONT)->work_item_id_first + ((SUBWAVEFRONT_ID) + 1) \
			* gpu_num_stream_cores - 1, (WAVEFRONT)->work_item_id_last); \
		(WORK_ITEM_ID)++)

void gpu_init(void);
void gpu_done(void);

void gpu_run(struct gpu_ndrange_t *ndrange);


#endif

