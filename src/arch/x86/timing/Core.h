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

#include <vector>

#include <arch/x86/emu/UInst.h>

#include "CPU.h"
#include "Thread.h"

namespace x86
{

// Forward declaration
class Uop;

// Class Core
class Core
{
public:

	// Dispatch stall reasons 
	enum DispatchStallReason
	{
		DispatchStallReasonInvalid = 0,
		DispatchStallReasonUsed,  // Dispatch slot was used with a finally committed inst. 
		DispatchStallReasonSpec,  // Used with a speculative inst. 
		DispatchStallReasonUopQueue,  // No instruction in the uop queue 
		DispatchStallReasonRob,  // No space in the rob 
		DispatchStallReasonIq,  // No space in the iq 
		DispatchStallReasonLsq,  // No space in the lsq 
		DispatchStallReasonRename,  // No free physical register 
		DispatchStallReasonCtx,  // No running ctx 
		DispatchStallReasonMax
	};

private:

	// CPU that it belongs to 
	CPU *cpu;

	// Array of threads 
	std::vector<std::unique_ptr<Thread>> threads;

	char *name;

	// Unique ID in CPU 
	int id;

	// Shared structures 
	std::list<std::unique_ptr<Uop>> event_queue;

	// FIXME
	//struct x86_fu_t *fu;
	//struct prefetch_history_t *prefetch_history;

	// Per core counters 
	long long uop_id_counter;  // Counter for uop ID assignment 
	long long dispatch_seq;  // Counter for uop ID assignment 
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;
	int reg_file_xmm_count;

	// Reorder Buffer 
	struct list_t *rob;
	int rob_count;
	int rob_head;
	int rob_tail;

	// Stages 
	int fetch_current;  // Currently fetching thread 
	long long fetch_switch_when;  // Cycle for last thread switch (for SwitchOnEvent) 
	int decode_current;
	int dispatch_current;
	int issue_current;
	int commit_current;

	// Stats 
	long long dispatch_stall[DispatchStallReasonMax];
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;

	// Statistics for shared structures 
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

public:

	/// Create
	void Create(std::string &name);

};

}

#endif // X86_ARCH_TIMING_CORE_H
