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

#ifndef SOUTHERN_ISLANDS_EMU_H
#define SOUTHERN_ISLANDS_EMU_H

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <arch/southern-islands/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>


/* 
 * Memory configuration 
 */

#define UAV_TABLE_START 0
#define UAV_TABLE_SIZE 1024
#define CONSTANT_MEMORY_START (UAV_TABLE_START + UAV_TABLE_SIZE)
#define CONSTANT_BUFFER_SIZE 1024
#define CONSTANT_BUFFERS 2
#define GLOBAL_MEMORY_START (CONSTANT_MEMORY_START + CONSTANT_BUFFERS*CONSTANT_BUFFER_SIZE)


/* OpenCL Image */

struct si_opencl_image_format_t
{
	uint32_t image_channel_order;
	uint32_t image_channel_data_type;
};

/*
 * GPU Write Tasks
 */

enum si_isa_write_task_kind_t
{
	SI_ISA_WRITE_TASK_NONE = 0,
	SI_ISA_WRITE_TASK_WRITE_LDS,
	SI_ISA_WRITE_TASK_WRITE_DEST,
	SI_ISA_WRITE_TASK_PUSH_BEFORE,
	SI_ISA_WRITE_TASK_SET_PRED
};


struct si_isa_write_task_t
{
	/* All */
	enum si_isa_write_task_kind_t kind;
	struct si_inst_t *inst;
	
	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	uint32_t value;

	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_LDS */
	uint32_t lds_addr;
	uint32_t lds_value;
        size_t   lds_value_size;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};


/* Repository for 'struct si_isa_write_task_t' objects */
extern struct repos_t *si_isa_write_task_repos;


/* Functions to handle deferred tasks */
void si_isa_enqueue_write_lds(uint32_t addr, uint32_t value, size_t value_size);
void si_isa_enqueue_write_dest(uint32_t value);
void si_isa_enqueue_write_dest_float(float value);
void si_isa_enqueue_push_before(void);
void si_isa_enqueue_pred_set(int cond);
void si_isa_write_task_commit(void);



/*
 * GPU Work-Group
 */

enum si_work_group_status_t
{
	si_work_group_pending		= 0x0001,
	si_work_group_running		= 0x0002,
	si_work_group_finished		= 0x0004
};

struct si_work_group_t
{
	char name[30];

	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum si_work_group_status_t status;

	/* NDRange it belongs to */
	struct si_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct si_work_item_t **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	struct si_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */
	struct si_wavefront_pool_t *wavefront_pool;

	/* Double linked lists of work-groups */
	struct si_work_group_t *pending_list_prev;
	struct si_work_group_t *pending_list_next;
	struct si_work_group_t *running_list_prev;
	struct si_work_group_t *running_list_next;
	struct si_work_group_t *finished_list_prev;
	struct si_work_group_t *finished_list_next;

	/* List of running wavefronts */
	struct si_wavefront_t *running_list_head;
	struct si_wavefront_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	struct si_wavefront_t *barrier_list_head;
	struct si_wavefront_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	struct si_wavefront_t *finished_list_head;
	struct si_wavefront_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define SI_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct si_work_group_t *si_work_group_create();
void si_work_group_free(struct si_work_group_t *work_group);
void si_work_group_dump(struct si_work_group_t *work_group, FILE *f);

int si_work_group_get_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_set_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_clear_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);




/*
 * GPU Wavefront
 */

/* Wavefront */
struct si_wavefront_t
{
	/* ID */
	char name[30];
	int id;
	int id_in_work_group;

	/* IDs of work-items it contains */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* NDRange and Work-group it belongs to */
	struct si_ndrange_t *ndrange;
	struct si_work_group_t *work_group;

	/* Instruction buffer (code, not architectural instruction buffer) */
	void *wavefront_pool;
	void *wavefront_pool_start;

	/* Current instruction */
	struct si_inst_t inst;
	int inst_size;

	/* Pointer to work_items */
	struct si_work_item_t *scalar_work_item;  
	struct si_work_item_t **work_items;  /* Pointer to first work-items in 'kernel->work_items' */
	union si_reg_t sreg[256];  /* Scalar registers--used by scalar work items */

	/* Predicate mask */
	struct bit_map_t *pred;  /* work_item_count elements */

	/* Flags updated during instruction execution */
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;
	unsigned int mem_wait : 1;
	unsigned int barrier : 1;
	unsigned int finished : 1;

	/* Linked lists */
	struct si_wavefront_t *running_list_next;
	struct si_wavefront_t *running_list_prev;
	struct si_wavefront_t *barrier_list_next;
	struct si_wavefront_t *barrier_list_prev;
	struct si_wavefront_t *finished_list_next;
	struct si_wavefront_t *finished_list_prev;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;

	/* Fields introduced for timing simulation */
	int id_in_compute_unit;
	int uop_id_counter;
	long long sched_when;  /* GPU cycle when wavefront was last scheduled */
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;

	/* Statistics */
	long long inst_count;  /* Total number of instructions */
	long long scalar_mem_inst_count;
	long long scalar_alu_inst_count;
	long long branch_inst_count;
	long long vector_mem_inst_count;
	long long vector_alu_inst_count;
	long long global_mem_inst_count;  
	long long local_mem_inst_count;  
};

#define SI_FOREACH_WAVEFRONT_IN_NDRANGE(NDRANGE, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (NDRANGE)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (NDRANGE)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

#define SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (WORK_GROUP)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (WORK_GROUP)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

struct si_wavefront_t *si_wavefront_create();
void si_wavefront_sreg_init(struct si_wavefront_t *wavefront);
void si_wavefront_free(struct si_wavefront_t *wavefront);
void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f);

void si_wavefront_stack_push(struct si_wavefront_t *wavefront);
void si_wavefront_stack_pop(struct si_wavefront_t *wavefront, int count);
void si_wavefront_execute(struct si_wavefront_t *wavefront);

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, int id_in_wavefront);



/*
 * GPU work_item (Pixel)
 */

#define SI_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

/* Structure describing a memory access definition */
struct si_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	uint32_t addr;
	int size;
};

struct si_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	struct si_ndrange_t *ndrange;

	/* Work-item state */
	union si_reg_t vreg[256];  /* Vector general purpose registers */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* Last global memory access */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	uint32_t local_mem_access_addr[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define SI_FOREACH_WORK_ITEM_IN_NDRANGE(NDRANGE, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (NDRANGE)->work_item_id_first; \
		(WORK_ITEM_ID) <= (NDRANGE)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define SI_FOREACH_WORK_ITEM_IN_WORK_GROUP(WORK_GROUP, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WORK_GROUP)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WORK_GROUP)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WAVEFRONT)->work_item_id_last; \
		(WORK_ITEM_ID)++)

struct si_work_item_t *si_work_item_create(void);
void si_work_item_free(struct si_work_item_t *work_item);

/* Consult and change predicate bits */  /* FIXME Remove */
void si_work_item_set_pred(struct si_work_item_t *work_item, int pred);
int si_work_item_get_pred(struct si_work_item_t *work_item);

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, int sreg, unsigned int value);
void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, int first_reg, int num_regs, 
	int cb);
void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, int first_reg, 
	int num_regs);

/*
 * Southern Islands GPU Emulator
 */

struct si_emu_t
{
	/* Timer */
	struct m2s_timer_t *timer;

	/* OpenCL objects */
	struct si_opencl_repo_t *opencl_repo;
	struct si_opencl_platform_t *opencl_platform;
	struct si_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_head;
	struct si_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct si_ndrange_t *pending_ndrange_list_head;
	struct si_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct si_ndrange_t *running_ndrange_list_head;
	struct si_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct si_ndrange_t *finished_ndrange_list_head;
	struct si_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* Statistics */
	int ndrange_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by wavefronts */
	long long scalar_alu_inst_count;  /* Number of scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Number of scalar memory instructions executed */
	long long branch_inst_count; /* Number of branch instructions executed */
	long long vector_alu_inst_count;  /* Number of vector ALU instructions executed */
	long long local_mem_inst_count; /* Number of LDS instructions executed */
	long long vector_mem_inst_count;  /* Number of scalar memory instructions executed */
};


extern enum arch_sim_kind_t si_emu_sim_kind;

extern long long si_emu_max_cycles;
extern long long si_emu_max_inst;
extern int si_emu_max_kernels;

extern char *si_emu_opencl_binary_name;
extern char *si_emu_report_file_name;
extern FILE *si_emu_report_file;

extern int si_emu_wavefront_size;

extern char *si_err_opencl_note;
extern char *si_err_opencl_param_note;

extern struct si_emu_t *si_emu;
extern struct arch_t *si_emu_arch;

void si_emu_init(void);
void si_emu_done(void);

void si_emu_dump_summary(FILE *f);

int si_emu_run(void);

void si_emu_libopencl_redirect(struct x86_ctx_t *ctx, char *path, int size);
void si_emu_libopencl_failed(int pid);

void si_emu_disasm(char *path);
void si_emu_opengl_disasm(char *path, int opengl_shader_index);

#endif

