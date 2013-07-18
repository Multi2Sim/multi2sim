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

#ifndef X86_ARCH_TIMING_CORE_H
#define X86_ARCH_TIMING_CORE_H

#include <arch/x86/emu/uinst.h>
#include <lib/util/class.h>
#include <mem-system/prefetch-history.h>



/*
 * Class 'X86Core'
 */

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


CLASS_BEGIN(X86Core, Object)

	/* CPU that it belongs to */
	X86Cpu *cpu;

	/* Array of threads */
	X86Thread **threads;

	char *name;

	/* Unique ID in CPU */
	int id;

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

CLASS_END(X86Core)


void X86CoreCreate(X86Core *self, X86Cpu *cpu);
void X86CoreDestroy(X86Core *self);

void X86CoreSetName(X86Core *self, char *name);

#endif

