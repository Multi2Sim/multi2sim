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

#include <arch/common/timing.h>
#include <arch/x86/emu/uinst.h>


/* Forward declarations */
CLASS_FORWARD_DECLARATION(X86Core);
struct x86_uop_t;


/* Error debug */
#define x86_cpu_error_debug(...) debug(x86_cpu_error_debug_category, __VA_ARGS__)
extern int x86_cpu_error_debug_category;



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
#define X86_CORE  (*x86_cpu->cores[core])
#define X86_THREAD  (*X86_CORE.threads[thread])
#define X86_CORE_IDX(x)  (*x86_cpu->cores[(x)])
#define X86_THREAD_IDX(x)  (*X86_CORE.threads[(x)])
#define X86_CORE_THREAD_IDX(x, y)  (*x86_cpu->cores[(x)]->threads[(y)])
#define X86_CORE_FOR_EACH  for (core = 0; core < x86_cpu_num_cores; core++)
#define X86_THREAD_FOR_EACH  for (thread = 0; thread < x86_cpu_num_threads; thread++)



/*
 * Public Functions
 */

void x86_cpu_read_config(void);

void x86_cpu_init(void);
void x86_cpu_done(void);

void x86_cpu_update_occupancy_stats(void);

void x86_cpu_uop_trace_list_add(struct x86_uop_t *uop);
void x86_cpu_uop_trace_list_empty(void);

void x86_cpu_run_stages(void);




/*
 * Class 'X86Cpu'
 */

CLASS_BEGIN(X86Cpu, Timing)

	/* Array of cores */
	X86Core **cores;

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

CLASS_END(X86Cpu)


void X86CpuCreate(X86Cpu *self);
void X86CpuDestroy(X86Cpu *self);

void X86CpuDump(Object *self, FILE *f);
void X86CpuDumpSummary(Timing *self, FILE *f);

int X86CpuRun(Timing *self);




/*
 * Public
 */

extern X86Cpu *x86_cpu;


#endif

