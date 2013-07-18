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
#include <lib/util/class.h>


/* Forward declarations */
struct x86_uop_t;




/*
 * Class 'X86Cpu'
 */

CLASS_BEGIN(X86Cpu, Timing)

	/* Associated emulator */
	X86Emu *emu;

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


void X86CpuCreate(X86Cpu *self, X86Emu *emu);
void X86CpuDestroy(X86Cpu *self);

void X86CpuDump(Object *self, FILE *f);
void X86CpuDumpSummary(Timing *self, FILE *f);
void X86CpuDumpReport(X86Cpu *self, FILE *f);
void X86CpuDumpUopReport(X86Cpu *self, FILE *f, long long *uop_stats,
		char *prefix, int peak_ipc);

int X86CpuRun(Timing *self);
void X86CpuRunStages(X86Cpu *self);
void X86CpuFastForward(X86Cpu *self);

void X86CpuAddToTraceList(X86Cpu *self, struct x86_uop_t *uop);
void X86CpuEmptyTraceList(X86Cpu *self);

void X86CpuUpdateOccupancyStats(X86Cpu *self);




/*
 * Public
 */

#define x86_cpu_error_debug(...) debug(x86_cpu_error_debug_category, __VA_ARGS__)
extern int x86_cpu_error_debug_category;

extern char *x86_config_help;

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


void X86CpuReadConfig(void);
void X86CpuInit(void);
void X86CpuDone(void);


#endif

