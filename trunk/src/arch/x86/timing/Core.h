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
#include <string>

#include <arch/x86/emu/UInst.h>

#include "CPU.h"
#include "Thread.h"

namespace x86
{

// Forward declaration
class Uop;
class CPU;

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

	// name of this Core
	std::string name;

	// CPU that it belongs to 
	CPU *cpu;

	// Array of threads 
	std::vector<std::unique_ptr<Thread>> threads;

	// Unique ID in CPU 
	int id = 0;

	// Shared structures 
	std::list<std::unique_ptr<Uop>> event_queue;

	// FIXME
	//struct x86_fu_t *fu;
	//struct prefetch_history_t *prefetch_history;

	// Per core counters 
	long long uop_id_counter = 0;  // Counter for uop ID assignment
	long long dispatch_seq = 0;  // Counter for uop ID assignment
	int iq_count = 0;
	int lsq_count = 0;
	int reg_file_int_count = 0;
	int reg_file_fp_count = 0;
	int reg_file_xmm_count = 0;

	// Reorder Buffer 
	std::list<std::unique_ptr<Uop>> rob;
	int rob_count = 0;

	// Stages 
	int fetch_current = 0;  // Currently fetching thread
	long long fetch_switch_when = 0;  // Cycle for last thread switch (for SwitchOnEvent)
	int decode_current = 0;
	int dispatch_current = 0;
	int issue_current = 0;
	int commit_current = 0;

	// Stats 
	long long dispatch_stall[DispatchStallReasonMax];
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_squashed_uinst = 0;
	long long num_branch_uinst = 0;
	long long num_mispred_branch_uinst = 0;

	// Statistics for shared structures 
	long long rob_occupancy = 0;
	long long rob_full = 0;
	long long rob_reads = 0;
	long long rob_writes = 0;

	long long iq_occupancy = 0;
	long long iq_full = 0;
	long long iq_reads = 0;
	long long iq_writes = 0;
	long long iq_wakeup_accesses = 0;

	long long lsq_occupancy = 0;
	long long lsq_full = 0;
	long long lsq_reads = 0;
	long long lsq_writes = 0;
	long long lsq_wakeup_accesses = 0;

	long long reg_file_int_occupancy = 0;
	long long reg_file_int_full = 0;
	long long reg_file_int_reads = 0;
	long long reg_file_int_writes = 0;

	long long reg_file_fp_occupancy = 0;
	long long reg_file_fp_full = 0;
	long long reg_file_fp_reads = 0;
	long long reg_file_fp_writes = 0;

	long long reg_file_xmm_occupancy = 0;
	long long reg_file_xmm_full = 0;
	long long reg_file_xmm_reads = 0;
	long long reg_file_xmm_writes = 0;

public:

	/// Constructor
	Core(const std::string &name, CPU *cpu);

	/// Setters
	void setID(int id) { this->id = id; }

	/// Getters
	int getID() { return id; }

};

}

#endif // X86_ARCH_TIMING_CORE_H
