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

#ifndef CPUARCH_H
#define CPUARCH_H

#include <time.h>
#include <signal.h>
#include <list.h>
#include <linked-list.h>
#include <repos.h>
#include <cachesystem.h>
#include <cpukernel.h>



/* Environment variables */
extern char **environ;


/* Error debug */
#define error_debug(...) debug(error_debug_category, __VA_ARGS__)
extern int error_debug_category;



/* CPU variable */
extern struct cpu_t *cpu;

extern char *cpu_config_help;



/* Processor parameters */

extern char *cpu_config_file_name;
extern char *cpu_report_file_name;

extern int cpu_cores;
extern int cpu_threads;

extern int cpu_context_quantum;
extern int cpu_context_switch;

extern int cpu_thread_quantum;
extern int cpu_thread_switch_penalty;

/* Recover_kind */
extern char *cpu_recover_kind_map[];
extern enum cpu_recover_kind_t {
	cpu_recover_kind_writeback = 0,
	cpu_recover_kind_commit
} cpu_recover_kind;
extern int cpu_recover_penalty;

/* Fetch stage */
extern char *cpu_fetch_kind_map[];
extern enum cpu_fetch_kind_t {
	cpu_fetch_kind_shared = 0,
	cpu_fetch_kind_timeslice,
	cpu_fetch_kind_switchonevent
} cpu_fetch_kind;

/* Decode stage */
extern int cpu_decode_width;

/* Dispatch stage */
extern char *cpu_dispatch_kind_map[];
extern enum cpu_dispatch_kind_t {
	cpu_dispatch_kind_shared = 0,
	cpu_dispatch_kind_timeslice,
} cpu_dispatch_kind;
extern int cpu_dispatch_width;

/* Issue stage */
extern char *cpu_issue_kind_map[];
extern enum cpu_issue_kind_t {
	cpu_issue_kind_shared = 0,
	cpu_issue_kind_timeslice,
} cpu_issue_kind;
extern int cpu_issue_width;

/* Commit stage */
extern char *cpu_commit_kind_map[];
extern enum cpu_commit_kind_t {
	cpu_commit_kind_shared = 0,
	cpu_commit_kind_timeslice
} cpu_commit_kind;
extern int cpu_commit_width;




/*
 * Micro Operations
 */

struct uop_t
{
	/* Micro-instruction */
	struct x86_uinst_t *uinst;
	enum x86_uinst_flag_t flags;

	/* Name and sequence numbers */
	char name[40];
	uint32_t magic;  /* Magic number for debugging */
	uint64_t seq;  /* Sequence number - unique uop identifier */
	uint64_t di_seq;  /* Dispatch sequence number - unique per core */

	/* Context info */
	struct ctx_t *ctx;
	int core;
	int thread;

	/* Fetch info */
	int fetch_tcache;  /* True if uop comes from trace cache */
	uint32_t eip;  /* Address of x86 macro-instruction */
	uint32_t neip;  /* Address of next non-speculative x86 macro-instruction */
	uint32_t pred_neip; /* Address of next predicted x86 macro-instruction (for branches) */
	uint32_t target_neip;  /* Address of target x86 macro-instruction assuming branch taken (for branches) */
	int specmode;
	uint64_t fetch_access;  /* Access identifier to the instruction cache */

	/* Fields associated with macroinstruction */
	char mop_name[40];
	int mop_index;  /* Index of uop within macroinstruction */
	int mop_count;  /* Number of uops within macroinstruction */
	int mop_size;  /* Corresponding macroinstruction size */
	uint64_t mop_seq;  /* Sequence number of macroinstruction */

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
	int in_fetchq : 1;
	int in_uopq : 1;
	int in_iq : 1;
	int in_lq : 1;
	int in_sq : 1;
	int in_eventq : 1;
	int in_rob : 1;

	/* Instruction status */
	int ready;
	int issued;
	int completed;

	/* For memory uops */
	uint32_t physical_address;  /* ... corresponding to 'uop->uinst->address' */

	/* Cycles */
	uint64_t when;  /* cycle when ready */
	uint64_t issue_try_when;  /* first cycle when f.u. is tried to be reserved */
	uint64_t issue_when;  /* cycle when issued */

	/* Branch prediction */
	int pred;  /* Global prediction (0=not taken, 1=taken) */
	int bimod_index, bimod_pred;
	int twolevel_bht_index, twolevel_pht_row, twolevel_pht_col, twolevel_pred;
	int choice_index, choice_pred;
};

struct uop_t *uop_create(void);
void uop_free_if_not_queued(struct uop_t *uop);
int uop_exists(struct uop_t *uop);

void uop_list_dump(struct list_t *uop_list, FILE *f);
void uop_lnlist_dump(struct linked_list_t *uop_list, FILE *f);
void uop_lnlist_check_if_ready(struct linked_list_t *uop_list);




/*
 * Functional Units
 */
 
#define FU_RES_MAX  10

enum fu_class_t
{
	fu_none = 0,

	fu_intadd,
	fu_intmult,
	fu_intdiv,
	fu_effaddr,
	fu_logic,

	fu_fpsimple,
	fu_fpadd,
	fu_fpmult,
	fu_fpdiv,
	fu_fpcomplex,

	fu_count
};

struct fu_t
{
	uint64_t cycle_when_free[fu_count][FU_RES_MAX];
	uint64_t accesses[fu_count];
	uint64_t denied[fu_count];
	uint64_t waiting_time[fu_count];
};

struct fu_res_t
{
	int count;
	int oplat;
	int issuelat;
	char *name;
};

extern struct fu_res_t fu_res_pool[fu_count];

void fu_init(void);
void fu_done(void);

int fu_reserve(struct uop_t *uop);
void fu_release(int core);




/*
 * Fetch Queue
 */

extern int fetchq_size;

void fetchq_init(void);
void fetchq_done(void);

void fetchq_recover(int core, int thread);
struct uop_t *fetchq_remove(int core, int thread, int index);




/*
 * Uop Queue
 */

extern int uopq_size;

void uopq_init(void);
void uopq_done(void);

void uopq_recover(int core, int thread);




/*
 * Reorder Buffer
 */

extern char *rob_kind_map[];
extern enum rob_kind_t {
	rob_kind_private = 0,
	rob_kind_shared
} rob_kind;
extern int rob_size;

void rob_init(void);
void rob_done(void);
void rob_dump(int core, FILE *f);

int rob_can_enqueue(struct uop_t *uop);
void rob_enqueue(struct uop_t *uop);
int rob_can_dequeue(int core, int thread);
struct uop_t *rob_head(int core, int thread);
void rob_remove_head(int core, int thread);
struct uop_t *rob_tail(int core, int thread);
void rob_remove_tail(int core, int thread);
struct uop_t *rob_get(int core, int thread, int index);




/*
 * Instruction Queue
 */

extern char *iq_kind_map[];
extern enum iq_kind_t {
	iq_kind_shared = 0,
	iq_kind_private
} iq_kind;
extern int iq_size;

void iq_init(void);
void iq_done(void);

int iq_can_insert(struct uop_t *uop);
void iq_insert(struct uop_t *uop);
void iq_remove(int core, int thread);
void iq_recover(int core, int thread);




/*
 * Load/Store Queue
 */

extern char *lsq_kind_map[];
extern enum lsq_kind_t {
	lsq_kind_shared = 0,
	lsq_kind_private
} lsq_kind;
extern int lsq_size;

void lsq_init(void);
void lsq_done(void);

int lsq_can_insert(struct uop_t *uop);
void lsq_insert(struct uop_t *uop);
void lsq_recover(int core, int thread);

void lq_remove(int core, int thread);
void sq_remove(int core, int thread);




/*
 * Event Queue
 */

void eventq_init(void);
void eventq_done(void);

int eventq_longlat(int core, int thread);
int eventq_cachemiss(int core, int thread);
void eventq_insert(struct linked_list_t *eventq, struct uop_t *uop);
struct uop_t *eventq_extract(struct linked_list_t *eventq);
void eventq_recover(int core, int thread);




/*
 * Physical Register File
 */

#define RF_MIN_INT_SIZE  (x86_dep_int_count + X86_UINST_MAX_ODEPS)
#define RF_MIN_FP_SIZE  (x86_dep_fp_count + X86_UINST_MAX_ODEPS)

extern char *rf_kind_map[];
extern enum rf_kind_t {
	rf_kind_shared = 0,
	rf_kind_private
} rf_kind;
extern int rf_int_size;
extern int rf_fp_size;

struct phreg_t {
	int pending;  /* not completed (bit) */
	int busy;  /* number of mapped logical registers */
};

struct rf_t {

	/* Integer registers */
	int int_rat[x86_dep_int_count];
	struct phreg_t *int_phreg;
	int int_phreg_count;
	int *int_free_phreg;
	int int_free_phreg_count;

	/* FP registers */
	int fp_top_of_stack;  /* Value between 0 and 7 */
	int fp_rat[x86_dep_fp_count];
	struct phreg_t *fp_phreg;
	int fp_phreg_count;
	int *fp_free_phreg;
	int fp_free_phreg_count;
};

void rf_init(void);
void rf_done(void);

struct rf_t *rf_create(int int_size, int fp_size);
void rf_free(struct rf_t *rf);

void rf_dump(int core, int thread, FILE *f);
void rf_count_deps(struct uop_t *uop);
int rf_can_rename(struct uop_t *uop);
void rf_rename(struct uop_t *uop);
int rf_ready(struct uop_t *uop);
void rf_write(struct uop_t *uop);
void rf_undo(struct uop_t *uop);
void rf_commit(struct uop_t *uop);
void rf_check_integrity(int core, int thread);




/*
 * Branch Predictor
 */

extern char *bpred_kind_map[];
extern enum bpred_kind_t {
	bpred_kind_perfect = 0,
	bpred_kind_taken,
	bpred_kind_nottaken,
	bpred_kind_bimod,
	bpred_kind_twolevel,
	bpred_kind_comb
} bpred_kind;

extern int bpred_btb_sets;
extern int bpred_btb_assoc;
extern int bpred_ras_size;
extern int bpred_bimod_size;
extern int bpred_choice_size;

extern int bpred_twolevel_l1size;
extern int bpred_twolevel_l2size;
extern int bpred_twolevel_hist_size;

struct bpred_t;

void bpred_init(void);
void bpred_done(void);

struct bpred_t *bpred_create(void);
void bpred_free(struct bpred_t *bpred);
int bpred_lookup(struct bpred_t *bpred, struct uop_t *uop);
int bpred_lookup_multiple(struct bpred_t *bpred, uint32_t eip, int count);
void bpred_update(struct bpred_t *bpred, struct uop_t *uop);

uint32_t bpred_btb_lookup(struct bpred_t *bpred, struct uop_t *uop);
void bpred_btb_update(struct bpred_t *bpred, struct uop_t *uop);
uint32_t bpred_btb_next_branch(struct bpred_t *bpred, uint32_t eip, uint32_t bsize);




/*
 * Trace cache
 */

#define TCACHE_ENTRY_SIZE  (sizeof(struct tcache_entry_t) + sizeof(uint32_t) * tcache_trace_size)
#define TCACHE_ENTRY(SET, WAY)  ((struct tcache_entry_t *) (((unsigned char *) tcache->entry) + TCACHE_ENTRY_SIZE * ((SET) * tcache_assoc + (WAY))))

struct tcache_entry_t {
	int counter;  /* lru counter */
	uint32_t tag;
	int uop_count, mop_count;
	int branch_mask, branch_flags, branch_count;
	uint32_t fall_through;
	uint32_t target;

	/* Last field. This is a list of 'tcache_trace_size' elements containing
	 * the addresses of the microinst located in the trace. Only in the case that
	 * all macroinst are decoded into just one uop can this array be filled up. */
	uint32_t mop_array[0];
};

struct tcache_t {
	
	/* Entries (sets * assoc) */
	struct tcache_entry_t *entry;
	struct tcache_entry_t *temp;  /* temporary trace */

	/* Stats */
	char name[20];
	uint64_t accesses;
	uint64_t hits;
	uint64_t committed;
	uint64_t squashed;
	uint64_t trace_length_acc;
	uint64_t trace_length_count;
};


extern int tcache_present;
extern int tcache_sets;
extern int tcache_assoc;
extern int tcache_trace_size;
extern int tcache_branch_max;
extern int tcache_queue_size;

struct tcache_t;

void tcache_init(void);
void tcache_done(void);
void tcache_dump_report(struct tcache_t *tcache, FILE *f);

struct tcache_t *tcache_create(void);
void tcache_free(struct tcache_t *tcache);
void tcache_new_uop(struct tcache_t *tcache, struct uop_t *uop);
int tcache_lookup(struct tcache_t *tcache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip);



/*
 * Pipeline Trace
 */

enum ptrace_stage_t
{
	ptrace_fetch = 0,
	ptrace_dispatch,
	ptrace_issue,
	ptrace_execution,
	ptrace_memory,
	ptrace_writeback,
	ptrace_commit
};

void ptrace_init(void);
void ptrace_done(void);

void ptrace_new_uop(struct uop_t *uop);
void ptrace_end_uop(struct uop_t *uop);
void ptrace_new_stage(struct uop_t *uop, enum ptrace_stage_t stage);
void ptrace_new_cycle(void);





/*
 * Multi-Core Multi-Thread Processor
 */


/* Fast access macros */
#define CORE			(cpu->core[core])
#define THREAD			(cpu->core[core].thread[thread])
#define ICORE(I)		(cpu->core[(I)])
#define ITHREAD(I)		(cpu->core[core].thread[(I)])
#define FOREACH_CORE		for (core = 0; core < cpu_cores; core++)
#define FOREACH_THREAD		for (thread = 0; thread < cpu_threads; thread++)


/* Dispatch stall reasons */
enum di_stall_t
{
	di_stall_used = 0,  /* Dispatch slot was used with a finally committed inst. */
	di_stall_spec,  /* Used with a speculative inst. */
	di_stall_uopq,  /* No instruction in the uop queue */
	di_stall_rob,  /* No space in the rob */
	di_stall_iq,  /* No space in the iq */
	di_stall_lsq,  /* No space in the lsq */
	di_stall_rename,  /* No free physical register */
	di_stall_ctx,  /* No running ctx */
	di_stall_max
};


/* Thread */
struct cpu_thread_t
{
	struct ctx_t *ctx;  /* allocated kernel context */
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
	int rf_int_count;
	int rf_fp_count;

	/* Private structures */
	struct list_t *fetchq;
	struct list_t *uopq;
	struct linked_list_t *iq;
	struct linked_list_t *lq;
	struct linked_list_t *sq;
	struct bpred_t *bpred;  /* branch predictor */
	struct tcache_t *tcache;  /* trace cache */
	struct rf_t *rf;  /* physical register file */

	/* Fetch */
	uint32_t fetch_eip, fetch_neip;  /* eip and next eip */
	int fetchq_occ;  /* Number of bytes occupied in the fetch queue */
	int tcacheq_occ;  /* Number of uops occupied in the trace cache queue */
	int fetch_stall;
	int fetch_bsize;  /* Block size of instruction cache for this thread */
	uint32_t fetch_block;  /* Virtual address of last fetched block */
	uint64_t fetch_access;  /* Cache access id of last instruction cache access */

	/* Stats */
	uint64_t fetched;
	uint64_t dispatched[x86_uinst_opcode_count];
	uint64_t issued[x86_uinst_opcode_count];
	uint64_t committed[x86_uinst_opcode_count];
	uint64_t squashed;
	uint64_t branches;
	uint64_t mispred;
	uint64_t last_commit_cycle;
	
	/* Statistics for structures */
	uint64_t rob_occupancy;
	uint64_t rob_full;
	uint64_t rob_reads;
	uint64_t rob_writes;

	uint64_t iq_occupancy;
	uint64_t iq_full;
	uint64_t iq_reads;
	uint64_t iq_writes;
	uint64_t iq_wakeup_accesses;

	uint64_t lsq_occupancy;
	uint64_t lsq_full;
	uint64_t lsq_reads;
	uint64_t lsq_writes;
	uint64_t lsq_wakeup_accesses;

	uint64_t rf_int_occupancy;
	uint64_t rf_int_full;
	uint64_t rf_int_reads;
	uint64_t rf_int_writes;

	uint64_t rf_fp_occupancy;
	uint64_t rf_fp_full;
	uint64_t rf_fp_reads;
	uint64_t rf_fp_writes;

	uint64_t rat_int_reads;
	uint64_t rat_int_writes;
	uint64_t rat_fp_reads;
	uint64_t rat_fp_writes;

	uint64_t btb_reads;
	uint64_t btb_writes;
};


/* Cores */
struct cpu_core_t
{
	/* Array of threads */
	struct cpu_thread_t *thread;

	/* Shared structures */
	struct linked_list_t *eventq;
	struct fu_t *fu;

	/* Per core counters */
	uint64_t di_seq;  /* Sequence number for dispatch stage */
	int iq_count;
	int lsq_count;
	int rf_int_count;
	int rf_fp_count;

	/* Reorder Buffer */
	struct list_t *rob;
	int rob_count;
	int rob_head;
	int rob_tail;

	/* Stages */
	int fetch_current;		/* for thread switch policy */
	int64_t fetch_switch;		/* for switchonevent */
	int decode_current;
	int dispatch_current;
	int issue_current;
	int commit_current;

	/* Stats */
	uint64_t di_stall[di_stall_max];
	uint64_t dispatched[x86_uinst_opcode_count];
	uint64_t issued[x86_uinst_opcode_count];
	uint64_t committed[x86_uinst_opcode_count];
	uint64_t squashed;
	uint64_t branches;
	uint64_t mispred;
	
	/* Statistics for shared structures */
	uint64_t rob_occupancy;
	uint64_t rob_full;
	uint64_t rob_reads;
	uint64_t rob_writes;

	uint64_t iq_occupancy;
	uint64_t iq_full;
	uint64_t iq_reads;
	uint64_t iq_writes;
	uint64_t iq_wakeup_accesses;

	uint64_t lsq_occupancy;
	uint64_t lsq_full;
	uint64_t lsq_reads;
	uint64_t lsq_writes;
	uint64_t lsq_wakeup_accesses;

	uint64_t rf_int_occupancy;
	uint64_t rf_int_full;
	uint64_t rf_int_reads;
	uint64_t rf_int_writes;

	uint64_t rf_fp_occupancy;
	uint64_t rf_fp_full;
	uint64_t rf_fp_reads;
	uint64_t rf_fp_writes;
};


/* Processor */
struct cpu_t
{
	/* Array of cores */
	struct cpu_core_t *core;

	/* Cycle and instruction counters */
	uint64_t cycle;
	uint64_t inst;

	/* Some fields */
	uint64_t seq;  /* Seq num assigned to last instr (with pre-incr) */
	char *stage;  /* Name of currently simulated stage */

	/* Context allocations */
	uint64_t ctx_alloc_oldest;  /* Time when oldest context was allocated */
	int ctx_dealloc_signals;  /* Sent deallocation signals */
	
	/* Structures */
	struct mm_t *mm;  /* Memory management unit */
	
	/* Statistics */
	uint64_t fetched;
	uint64_t dispatched[x86_uinst_opcode_count];
	uint64_t issued[x86_uinst_opcode_count];
	uint64_t committed[x86_uinst_opcode_count];
	uint64_t squashed;
	uint64_t branches;
	uint64_t mispred;
	double time;

	/* For dumping */
	uint64_t last_committed;
	uint64_t last_dump;
};


/* Procedures and functions */
void cpu_init(void);
void cpu_done(void);

void cpu_load_progs(int argc, char **argv, char *ctxfile);
void cpu_dump(FILE *f);
void cpu_update_occupancy_stats(void);
uint32_t cpu_tlb_address(int ctx, uint32_t vaddr);

int cpu_pipeline_empty(int core, int thread);
void cpu_map_context(int core, int thread, struct ctx_t *ctx);
void cpu_unmap_context(int core, int thread);
void cpu_static_schedule(void);
void cpu_dynamic_schedule(void);

void cpu_stages(void);
void cpu_fetch(void);
void cpu_decode(void);
void cpu_dispatch(void);
void cpu_issue(void);
void cpu_writeback(void);
void cpu_commit(void);
void cpu_recover(int core, int thread);

void cpu_run(void);

#endif

