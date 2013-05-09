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

#ifndef X86_ARCH_TIMING_CPU_H
#define X86_ARCH_TIMING_CPU_H

#include <arch/x86/emu/uinst.h>


/* Forward types */
struct x86_uop_t;


/* Error debug */
#define x86_cpu_error_debug(...) debug(x86_cpu_error_debug_category, __VA_ARGS__)
extern int x86_cpu_error_debug_category;



/* CPU variable */
extern struct x86_cpu_t *x86_cpu;

extern char *x86_config_help;



/* Processor parameters */

extern char *x86_config_file_name;
extern char *x86_cpu_report_file_name;

extern int x86_cpu_num_cores;
extern int x86_cpu_num_threads;

extern int x86_cpu_context_quantum;

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


/* Trace */
#define x86_tracing() trace_status(x86_trace_category)
#define x86_trace(...) trace(x86_trace_category, __VA_ARGS__)
#define x86_trace_header(...) trace_header(x86_trace_category, __VA_ARGS__)
extern int x86_trace_category;


/* Fast access macros */
#define X86_CORE  (x86_cpu->core[core])
#define X86_THREAD  (x86_cpu->core[core].thread[thread])
#define X86_CORE_IDX(x)  (x86_cpu->core[(x)])
#define X86_THREAD_IDX(x)  (x86_cpu->core[core].thread[(x)])
#define X86_CORE_THREAD_IDX(x, y)  (x86_cpu->core[(x)].thread[(y)])
#define X86_CORE_FOR_EACH  for (core = 0; core < x86_cpu_num_cores; core++)
#define X86_THREAD_FOR_EACH  for (thread = 0; thread < x86_cpu_num_threads; thread++)


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
	/* Context currently running in this thread. This is a context present
	 * in the thread's 'mapped' list. */
	struct x86_ctx_t *ctx;

	/* Double-linked list of mapped contexts */
	struct x86_ctx_t *mapped_list_head;
	struct x86_ctx_t *mapped_list_tail;
	int mapped_list_count;
	int mapped_list_max;

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
	int reg_file_xmm_count;

	/* Private structures */
	struct list_t *fetch_queue;
	struct list_t *uop_queue;
	struct linked_list_t *iq;
	struct linked_list_t *lq;
	struct linked_list_t *sq;
	struct linked_list_t *preq;
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

	/* Cycle in which last micro-instruction committed */
	long long last_commit_cycle;

	/* Statistics */
	long long num_fetched_uinst;
	long long num_dispatched_uinst_array[x86_uinst_opcode_count];
	long long num_issued_uinst_array[x86_uinst_opcode_count];
	long long num_committed_uinst_array[x86_uinst_opcode_count];
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;
	
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

	long long reg_file_xmm_occupancy;
	long long reg_file_xmm_full;
	long long reg_file_xmm_reads;
	long long reg_file_xmm_writes;

	long long rat_int_reads;
	long long rat_int_writes;
	long long rat_fp_reads;
	long long rat_fp_writes;
	long long rat_xmm_reads;
	long long rat_xmm_writes;

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
	struct prefetch_history_t *prefetch_history;

	/* Per core counters */
	long long uop_id_counter;  /* Counter for uop ID assignment */
	long long dispatch_seq;  /* Counter for uop ID assignment */
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;
	int reg_file_xmm_count;

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
	long long num_dispatched_uinst_array[x86_uinst_opcode_count];
	long long num_issued_uinst_array[x86_uinst_opcode_count];
	long long num_committed_uinst_array[x86_uinst_opcode_count];
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;
	
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
	
	long long reg_file_xmm_occupancy;
	long long reg_file_xmm_full;
	long long reg_file_xmm_reads;
	long long reg_file_xmm_writes;
};


/* Processor */
struct x86_cpu_t
{
	/* Array of cores */
	struct x86_core_t *core;

	/* Some fields */
	long long uop_id_counter;  /* Counter of uop ID assignment */
	char *stage;  /* Name of currently simulated stage */

	/* From all contexts in the 'alloc' list of 'x86_emu', minimum value
	 * of variable 'ctx->alloc_cycle'. This value is used to decide whether
	 * the scheduler should be called at all to check for any context whose
	 * execution quantum has expired. These variables are updated by calling
	 * 'x86_cpu_update_min_alloc_cycle' */
	long long min_alloc_cycle;
	
	/* List containing uops that need to report an 'end_inst' trace event */
	struct linked_list_t *uop_trace_list;

	/* Statistics */
	long long num_fast_forward_inst;  /* Fast-forwarded x86 instructions */
	long long num_fetched_uinst;
	long long num_dispatched_uinst_array[x86_uinst_opcode_count];
	long long num_issued_uinst_array[x86_uinst_opcode_count];
	long long num_committed_uinst_array[x86_uinst_opcode_count];
	long long num_committed_uinst;  /* Committed micro-instructions */
	long long num_committed_inst;  /* Committed x86 instructions */
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;
	double time;

	/* For dumping */
	long long last_committed;
	long long last_dump;
};




/*
 * Public Functions
 */

void x86_cpu_read_config(void);

void x86_cpu_init(void);
void x86_cpu_done(void);

void x86_cpu_dump(FILE *f);
void x86_cpu_dump_summary(FILE *f);

void x86_cpu_update_occupancy_stats(void);

int x86_cpu_pipeline_empty(int core, int thread);
void x86_cpu_evict_context(int core, int thread);
void x86_cpu_schedule(void);

void x86_cpu_uop_trace_list_add(struct x86_uop_t *uop);
void x86_cpu_uop_trace_list_empty(void);

void x86_cpu_run_stages(void);
void x86_cpu_fetch(void);
void x86_cpu_decode(void);
void x86_cpu_dispatch(void);
void x86_cpu_issue(void);
void x86_cpu_writeback(void);
void x86_cpu_commit(void);
void x86_cpu_recover(int core, int thread);

int x86_cpu_run(void);


#endif

