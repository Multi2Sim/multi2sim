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

#ifndef X86_TIMING_H
#define X86_TIMING_H

#include <time.h>
#include <signal.h>
#include <list.h>
#include <linked-list.h>
#include <repos.h>

#include <x86-emu.h>
#include <mem-system.h>



/* Environment variables */
extern char **environ;


/* Error debug */
#define x86_cpu_error_debug(...) debug(x86_cpu_error_debug_category, __VA_ARGS__)
extern int x86_cpu_error_debug_category;



/* CPU variable */
extern struct x86_cpu_t *x86_cpu;

extern char *x86_cpu_config_help;



/* Processor parameters */

extern char *x86_cpu_config_file_name;
extern char *x86_cpu_report_file_name;

extern int x86_cpu_num_cores;
extern int x86_cpu_num_threads;

extern int x86_cpu_context_quantum;
extern int x86_cpu_context_switch;

extern int x86_cpu_thread_quantum;
extern int x86_cpu_thread_switch_penalty;

/* Recover_kind */
extern char *x86_cpu_recover_kind_map[];
extern enum x86_cpu_recover_kind_t
{
	x86_cpu_recover_kind_writeback = 0,
	x86_cpu_recover_kind_commit
} x86_cpu_recover_kind;
extern int x86_cpu_recover_penalty;

/* Fetch stage */
extern char *x86_cpu_fetch_kind_map[];
extern enum x86_cpu_fetch_kind_t
{
	x86_cpu_fetch_kind_shared = 0,
	x86_cpu_fetch_kind_timeslice,
	x86_cpu_fetch_kind_switchonevent
} x86_cpu_fetch_kind;

/* Decode stage */
extern int x86_cpu_decode_width;

/* Dispatch stage */
extern char *x86_cpu_dispatch_kind_map[];
extern enum x86_cpu_dispatch_kind_t
{
	x86_cpu_dispatch_kind_shared = 0,
	x86_cpu_dispatch_kind_timeslice,
} x86_cpu_dispatch_kind;
extern int x86_cpu_dispatch_width;

/* Issue stage */
extern char *x86_cpu_issue_kind_map[];
extern enum x86_cpu_issue_kind_t
{
	x86_cpu_issue_kind_shared = 0,
	x86_cpu_issue_kind_timeslice,
} x86_cpu_issue_kind;
extern int x86_cpu_issue_width;

/* Commit stage */
extern char *x86_cpu_commit_kind_map[];
extern enum x86_cpu_commit_kind_t
{
	x86_cpu_commit_kind_shared = 0,
	x86_cpu_commit_kind_timeslice
} x86_cpu_commit_kind;
extern int x86_cpu_commit_width;




/*
 * Micro Operations
 */

struct x86_uop_t
{
	/* Micro-instruction */
	struct x86_uinst_t *uinst;
	enum x86_uinst_flag_t flags;

	/* Name and sequence numbers */
	char name[40];
	long long magic;  /* Magic number for debugging */
	long long seq;  /* Sequence number - unique uop identifier */
	long long dispatch_seq;  /* Dispatch sequence number - unique per core */

	/* Context info */
	struct x86_ctx_t *ctx;
	int core;
	int thread;

	/* Fetch info */
	int fetch_trace_cache;  /* True if uop comes from trace cache */
	unsigned int eip;  /* Address of x86 macro-instruction */
	unsigned int neip;  /* Address of next non-speculative x86 macro-instruction */
	unsigned int pred_neip; /* Address of next predicted x86 macro-instruction (for branches) */
	unsigned int target_neip;  /* Address of target x86 macro-instruction assuming branch taken (for branches) */
	int specmode;
	unsigned int fetch_address;  /* Physical address of memory access to fetch this instruction */
	long long fetch_access;  /* Access identifier to fetch this instruction */

	/* Fields associated with macroinstruction */
	char mop_name[40];
	int mop_index;  /* Index of uop within macroinstruction */
	int mop_count;  /* Number of uops within macroinstruction */
	int mop_size;  /* Corresponding macroinstruction size */
	long long mop_seq;  /* Sequence number of macroinstruction */

	/* Logical dependencies */
	int idep_count;
	int odep_count;

	/* Physical mappings */
	int ph_int_idep_count, ph_fp_idep_count;
	int ph_int_odep_count, ph_fp_odep_count;
	int ph_idep[X86_UINST_MAX_IDEPS];
	int ph_odep[X86_UINST_MAX_ODEPS];
	int ph_oodep[X86_UINST_MAX_ODEPS];

	/* Queues where instruction is */
	int in_fetch_queue : 1;
	int in_uop_queue : 1;
	int in_iq : 1;
	int in_lq : 1;
	int in_sq : 1;
	int in_event_queue : 1;
	int in_rob : 1;

	/* Instruction status */
	int ready;
	int issued;
	int completed;

	/* For memory uops */
	uint32_t phy_addr;  /* ... corresponding to 'uop->uinst->address' */

	/* Cycles */
	long long when;  /* cycle when ready */
	long long issue_try_when;  /* first cycle when f.u. is tried to be reserved */
	long long issue_when;  /* cycle when issued */

	/* Branch prediction */
	int pred;  /* Global prediction (0=not taken, 1=taken) */
	int bimod_index, bimod_pred;
	int twolevel_bht_index, twolevel_pht_row, twolevel_pht_col, twolevel_pred;
	int choice_index, choice_pred;
};

struct x86_uop_t *x86_uop_create(void);
void x86_uop_free_if_not_queued(struct x86_uop_t *uop);
int x86_uop_exists(struct x86_uop_t *uop);

void x86_uop_list_dump(struct list_t *uop_list, FILE *f);
void x86_uop_linked_list_dump(struct linked_list_t *uop_list, FILE *f);
void x86_uop_linked_list_check_if_ready(struct linked_list_t *uop_list);




/*
 * Functional Units
 */
 
#define X86_FU_RES_MAX  10

enum x86_fu_class_t
{
	x86_fu_none = 0,

	x86_fu_intadd,
	x86_fu_intmult,
	x86_fu_intdiv,
	x86_fu_effaddr,
	x86_fu_logic,

	x86_fu_fpsimple,
	x86_fu_fpadd,
	x86_fu_fpmult,
	x86_fu_fpdiv,
	x86_fu_fpcomplex,

	x86_fu_count
};

struct x86_fu_t
{
	long long cycle_when_free[x86_fu_count][X86_FU_RES_MAX];
	long long accesses[x86_fu_count];
	long long denied[x86_fu_count];
	long long waiting_time[x86_fu_count];
};

struct x86_fu_res_t
{
	int count;
	int oplat;
	int issuelat;
	char *name;
};

extern struct x86_fu_res_t x86_fu_res_pool[x86_fu_count];

void x86_fu_init(void);
void x86_fu_done(void);

int x86_fu_reserve(struct x86_uop_t *uop);
void x86_fu_release(int core);




/*
 * Fetch Queue
 */

extern int x86_fetch_queue_size;

void x86_fetch_queue_init(void);
void x86_fetch_queue_done(void);

void x86_fetch_queue_recover(int core, int thread);
struct x86_uop_t *x86_fetch_queue_remove(int core, int thread, int index);




/*
 * Uop Queue
 */

extern int x86_uop_queue_size;

void x86_uop_queue_init(void);
void x86_uop_queue_done(void);

void x86_uop_queue_recover(int core, int thread);




/*
 * Reorder Buffer
 */

extern char *x86_rob_kind_map[];
extern enum x86_rob_kind_t
{
	x86_rob_kind_private = 0,
	x86_rob_kind_shared
} x86_rob_kind;
extern int x86_rob_size;

void x86_rob_init(void);
void x86_rob_done(void);
void x86_rob_dump(int core, FILE *f);

int x86_rob_can_enqueue(struct x86_uop_t *uop);
void x86_rob_enqueue(struct x86_uop_t *uop);
int x86_rob_can_dequeue(int core, int thread);
struct x86_uop_t *x86_rob_head(int core, int thread);
void x86_rob_remove_head(int core, int thread);
struct x86_uop_t *x86_rob_tail(int core, int thread);
void x86_rob_remove_tail(int core, int thread);
struct x86_uop_t *x86_rob_get(int core, int thread, int index);




/*
 * Instruction Queue
 */

extern char *x86_iq_kind_map[];
extern enum x86_iq_kind_t
{
	x86_iq_kind_shared = 0,
	x86_iq_kind_private
} x86_iq_kind;
extern int x86_iq_size;

void x86_iq_init(void);
void x86_iq_done(void);

int x86_iq_can_insert(struct x86_uop_t *uop);
void x86_iq_insert(struct x86_uop_t *uop);
void x86_iq_remove(int core, int thread);
void x86_iq_recover(int core, int thread);




/*
 * Load/Store Queue
 */

extern char *x86_lsq_kind_map[];
extern enum x86_lsq_kind_t
{
	x86_lsq_kind_shared = 0,
	x86_lsq_kind_private
} x86_lsq_kind;
extern int x86_lsq_size;

void x86_lsq_init(void);
void x86_lsq_done(void);

int x86_lsq_can_insert(struct x86_uop_t *uop);
void x86_lsq_insert(struct x86_uop_t *uop);
void x86_lsq_recover(int core, int thread);

void x86_lq_remove(int core, int thread);
void x86_sq_remove(int core, int thread);




/*
 * Event Queue
 */

void x86_event_queue_init(void);
void x86_event_queue_done(void);

int x86_event_queue_long_latency(int core, int thread);
int x86_event_queue_cache_miss(int core, int thread);
void x86_event_queue_insert(struct linked_list_t *event_queue, struct x86_uop_t *uop);
struct x86_uop_t *x86_event_queue_extract(struct linked_list_t *event_queue);
void x86_event_queue_recover(int core, int thread);




/*
 * Physical Register File
 */

#define X86_REG_FILE_MIN_INT_SIZE  (x86_dep_int_count + X86_UINST_MAX_ODEPS)
#define X86_REG_FILE_MIN_FP_SIZE  (x86_dep_fp_count + X86_UINST_MAX_ODEPS)

extern char *x86_reg_file_kind_map[];
extern enum x86_reg_file_kind_t
{
	x86_reg_file_kind_shared = 0,
	x86_reg_file_kind_private
} x86_reg_file_kind;
extern int x86_reg_file_int_size;
extern int x86_reg_file_fp_size;

struct x86_phreg_t
{
	int pending;  /* not completed (bit) */
	int busy;  /* number of mapped logical registers */
};

struct x86_reg_file_t
{
	/* Integer registers */
	int int_rat[x86_dep_int_count];
	struct x86_phreg_t *int_phreg;
	int int_phreg_count;
	int *int_free_phreg;
	int int_free_phreg_count;

	/* FP registers */
	int fp_top_of_stack;  /* Value between 0 and 7 */
	int fp_rat[x86_dep_fp_count];
	struct x86_phreg_t *fp_phreg;
	int fp_phreg_count;
	int *fp_free_phreg;
	int fp_free_phreg_count;
};

void x86_reg_file_init(void);
void x86_reg_file_done(void);

struct x86_reg_file_t *x86_reg_file_create(int int_size, int fp_size);
void x86_reg_file_free(struct x86_reg_file_t *reg_file);

void x86_reg_file_dump(int core, int thread, FILE *f);
void x86_reg_file_count_deps(struct x86_uop_t *uop);
int x86_reg_file_can_rename(struct x86_uop_t *uop);
void x86_reg_file_rename(struct x86_uop_t *uop);
int x86_reg_file_ready(struct x86_uop_t *uop);
void x86_reg_file_write(struct x86_uop_t *uop);
void x86_reg_file_undo(struct x86_uop_t *uop);
void x86_reg_file_commit(struct x86_uop_t *uop);
void x86_reg_file_check_integrity(int core, int thread);




/*
 * Branch Predictor
 */

extern char *x86_bpred_kind_map[];
extern enum x86_bpred_kind_t
{
	x86_bpred_kind_perfect = 0,
	x86_bpred_kind_taken,
	x86_bpred_kind_nottaken,
	x86_bpred_kind_bimod,
	x86_bpred_kind_twolevel,
	x86_bpred_kind_comb
} x86_bpred_kind;

extern int x86_bpred_btb_sets;
extern int x86_bpred_btb_assoc;
extern int x86_bpred_ras_size;
extern int x86_bpred_bimod_size;
extern int x86_bpred_choice_size;

extern int x86_bpred_twolevel_l1size;
extern int x86_bpred_twolevel_l2size;
extern int x86_bpred_twolevel_hist_size;


void x86_bpred_init(void);
void x86_bpred_done(void);

struct x86_bpred_t *x86_bpred_create(char *name);
void x86_bpred_free(struct x86_bpred_t *bpred);
int x86_bpred_lookup(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
int x86_bpred_lookup_multiple(struct x86_bpred_t *bpred, uint32_t eip, int count);
void x86_bpred_update(struct x86_bpred_t *bpred, struct x86_uop_t *uop);

unsigned int x86_bpred_btb_lookup(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
void x86_bpred_btb_update(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
unsigned int x86_bpred_btb_next_branch(struct x86_bpred_t *bpred, uint32_t eip, uint32_t bsize);




/*
 * Trace cache
 */

#define X86_TRACE_CACHE_ENTRY_SIZE \
	(sizeof(struct x86_trace_cache_entry_t) + \
	sizeof(uint32_t) * x86_trace_cache_trace_size)
#define X86_TRACE_CACHE_ENTRY(SET, WAY) \
	((struct x86_trace_cache_entry_t *) (((unsigned char *) trace_cache->entry) + \
	X86_TRACE_CACHE_ENTRY_SIZE * ((SET) * x86_trace_cache_assoc + (WAY))))

struct x86_trace_cache_entry_t
{
	int counter;  /* lru counter */
	unsigned int tag;
	int uop_count, mop_count;
	int branch_mask, branch_flags, branch_count;
	unsigned int fall_through;
	unsigned int target;

	/* Last field. This is a list of 'x86_trace_cache_trace_size' elements containing
	 * the addresses of the microinst located in the trace. Only in the case that
	 * all macroinst are decoded into just one uop can this array be filled up. */
	unsigned int mop_array[0];
};

struct x86_trace_cache_t
{
	/* Entries (sets * assoc) */
	struct x86_trace_cache_entry_t *entry;
	struct x86_trace_cache_entry_t *temp;  /* Temporary trace */

	/* Stats */
	char name[20];
	long long accesses;
	long long hits;
	long long committed;
	long long squashed;
	long long trace_length_acc;
	long long trace_length_count;
};


extern int x86_trace_cache_present;
extern int x86_trace_cache_num_sets;
extern int x86_trace_cache_assoc;
extern int x86_trace_cache_trace_size;
extern int x86_trace_cache_branch_max;
extern int x86_trace_cache_queue_size;

void x86_trace_cache_init(void);
void x86_trace_cache_done(void);
void x86_trace_cache_dump_report(struct x86_trace_cache_t *trace_cache, FILE *f);

struct x86_trace_cache_t *x86_trace_cache_create(void);
void x86_trace_cache_free(struct x86_trace_cache_t *trace_cache);

void x86_trace_cache_new_uop(struct x86_trace_cache_t *trace_cache, struct x86_uop_t *uop);
int x86_trace_cache_lookup(struct x86_trace_cache_t *trace_cache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip);




/*
 * Multi-Core Multi-Thread Processor
 */

/* Trace */
#define x86_tracing() trace_status(x86_trace_category)
#define x86_trace(...) trace(x86_trace_category, __VA_ARGS__)
#define x86_trace_header(...) trace_header(x86_trace_category, __VA_ARGS__)
extern int x86_trace_category;


/* Fast access macros */
#define X86_CORE		(x86_cpu->core[core])
#define X86_THREAD		(x86_cpu->core[core].thread[thread])
#define X86_CORE_IDX(I)		(x86_cpu->core[(I)])
#define X86_THREAD_IDX(I)	(x86_cpu->core[core].thread[(I)])
#define X86_CORE_FOR_EACH	for (core = 0; core < x86_cpu_num_cores; core++)
#define X86_THREAD_FOR_EACH	for (thread = 0; thread < x86_cpu_num_threads; thread++)


/* Dispatch stall reasons */
enum x86_dispatch_stall_t
{
	x86_dispatch_stall_used = 0,  /* Dispatch slot was used with a finally committed inst. */
	x86_dispatch_stall_spec,  /* Used with a speculative inst. */
	x86_dispatch_stall_uop_queue,  /* No instruction in the uop queue */
	x86_dispatch_stall_rob,  /* No space in the rob */
	x86_dispatch_stall_iq,  /* No space in the iq */
	x86_dispatch_stall_lsq,  /* No space in the lsq */
	x86_dispatch_stall_rename,  /* No free physical register */
	x86_dispatch_stall_ctx,  /* No running ctx */
	x86_dispatch_stall_max
};


/* Thread */
struct x86_thread_t
{
	struct x86_ctx_t *ctx;  /* allocated kernel context */
	int last_alloc_pid;  /* pid of last allocated context */

	/* Reorder buffer */
	int rob_count;
	int rob_left_bound;
	int rob_right_bound;
	int rob_head;
	int rob_tail;

	/* Number of uops in private structures */
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;

	/* Private structures */
	struct list_t *fetch_queue;
	struct list_t *uop_queue;
	struct linked_list_t *iq;
	struct linked_list_t *lq;
	struct linked_list_t *sq;
	struct x86_bpred_t *bpred;  /* branch predictor */
	struct x86_trace_cache_t *trace_cache;  /* trace cache */
	struct x86_reg_file_t *reg_file;  /* physical register file */

	/* Fetch */
	unsigned int fetch_eip, fetch_neip;  /* eip and next eip */
	int fetchq_occ;  /* Number of bytes occupied in the fetch queue */
	int trace_cache_queue_occ;  /* Number of uops occupied in the trace cache queue */
	unsigned int fetch_block;  /* Virtual address of last fetched block */
	unsigned int fetch_address;  /* Physical address of last instruction fetch */
	long long fetch_access;  /* Module access ID of last instruction fetch */
	long long fetch_stall_until;  /* Cycle until which fetching is stalled (inclussive) */

	/* Entries to the memory system */
	struct mod_t *data_mod;  /* Entry for data */
	struct mod_t *inst_mod;  /* Entry for instructions */

	/* Statistics */
	long long fetched;
	long long dispatched[x86_uinst_opcode_count];
	long long issued[x86_uinst_opcode_count];
	long long committed[x86_uinst_opcode_count];
	long long squashed;
	long long branches;
	long long mispred;
	long long last_commit_cycle;
	
	/* Statistics for structures */
	long long rob_occupancy;
	long long rob_full;
	long long rob_reads;
	long long rob_writes;

	long long iq_occupancy;
	long long iq_full;
	long long iq_reads;
	long long iq_writes;
	long long iq_wakeup_accesses;

	long long lsq_occupancy;
	long long lsq_full;
	long long lsq_reads;
	long long lsq_writes;
	long long lsq_wakeup_accesses;

	long long reg_file_int_occupancy;
	long long reg_file_int_full;
	long long reg_file_int_reads;
	long long reg_file_int_writes;

	long long reg_file_fp_occupancy;
	long long reg_file_fp_full;
	long long reg_file_fp_reads;
	long long reg_file_fp_writes;

	long long rat_int_reads;
	long long rat_int_writes;
	long long rat_fp_reads;
	long long rat_fp_writes;

	long long btb_reads;
	long long btb_writes;
};


/* Cores */
struct x86_core_t
{
	/* Array of threads */
	struct x86_thread_t *thread;

	/* Shared structures */
	struct linked_list_t *event_queue;
	struct x86_fu_t *fu;

	/* Per core counters */
	long long dispatch_seq;  /* Sequence number for dispatch stage */
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;

	/* Reorder Buffer */
	struct list_t *rob;
	int rob_count;
	int rob_head;
	int rob_tail;

	/* Stages */
	int fetch_current;  /* Currently fetching thread */
	long long fetch_switch_when;  /* Cycle for last thread switch (for SwitchOnEvent) */
	int decode_current;
	int dispatch_current;
	int issue_current;
	int commit_current;

	/* Stats */
	long long dispatch_stall[x86_dispatch_stall_max];
	long long dispatched[x86_uinst_opcode_count];
	long long issued[x86_uinst_opcode_count];
	long long committed[x86_uinst_opcode_count];
	long long squashed;
	long long branches;
	long long mispred;
	
	/* Statistics for shared structures */
	long long rob_occupancy;
	long long rob_full;
	long long rob_reads;
	long long rob_writes;

	long long iq_occupancy;
	long long iq_full;
	long long iq_reads;
	long long iq_writes;
	long long iq_wakeup_accesses;

	long long lsq_occupancy;
	long long lsq_full;
	long long lsq_reads;
	long long lsq_writes;
	long long lsq_wakeup_accesses;

	long long reg_file_int_occupancy;
	long long reg_file_int_full;
	long long reg_file_int_reads;
	long long reg_file_int_writes;

	long long reg_file_fp_occupancy;
	long long reg_file_fp_full;
	long long reg_file_fp_reads;
	long long reg_file_fp_writes;
};


/* Processor */
struct x86_cpu_t
{
	/* Array of cores */
	struct x86_core_t *core;

	/* Cycle and instruction counters */
	long long cycle;
	long long inst;

	/* Some fields */
	long long seq;  /* Seq num assigned to last instr (with pre-incr) */
	char *stage;  /* Name of currently simulated stage */

	/* Context allocations */
	long long ctx_alloc_oldest;  /* Time when oldest context was allocated */
	int ctx_dealloc_signals;  /* Sent deallocation signals */
	
	/* Statistics */
	long long fetched;
	long long dispatched[x86_uinst_opcode_count];
	long long issued[x86_uinst_opcode_count];
	long long committed[x86_uinst_opcode_count];
	long long squashed;
	long long branches;
	long long mispred;
	double time;

	/* For dumping */
	long long last_committed;
	long long last_dump;
};


/* Procedures and functions */
void x86_cpu_init(void);
void x86_cpu_done(void);
void x86_cpu_dump(FILE *f);

void x86_cpu_load_progs(int argc, char **argv, char *ctxfile);

void x86_cpu_update_occupancy_stats(void);

int x86_cpu_pipeline_empty(int core, int thread);
void x86_cpu_map_context(int core, int thread, struct x86_ctx_t *ctx);
void x86_cpu_unmap_context(int core, int thread);
void x86_cpu_static_schedule(void);
void x86_cpu_dynamic_schedule(void);

void x86_cpu_run_stages(void);
void x86_cpu_fetch(void);
void x86_cpu_decode(void);
void x86_cpu_dispatch(void);
void x86_cpu_issue(void);
void x86_cpu_writeback(void);
void x86_cpu_commit(void);
void x86_cpu_recover(int core, int thread);

void x86_cpu_run(void);

#endif

