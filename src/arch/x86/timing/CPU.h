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

#ifndef ARCH_X86_TIMING_CPU_H
#define ARCH_X86_TIMING_CPU_H

#include <vector>
#include <list>

#include <memory/MMU.h>
#include <arch/x86/emu/UInst.h>
#include <arch/x86/emu/Emu.h>

#include "Core.h"
#include "Thread.h"
#include "Uop.h"

namespace x86
{

// Forward declaration
//class Uop;
//class Emu;
class Core;
class MMU;

// Class CPU
class CPU
{
public:

	/// Recover kind
	enum RecoverKind
	{
		RecoverKindInvalid = 0,
		RecoverKindWriteback,
		RecoverKindCommit
	};

	/// Recover kind string map
	static misc::StringMap recover_kind_map;

	/// Fetch stage kind
	enum FetchKind
	{
		FetchKindInvalid = 0,
		FetchKindShared,
		FetchKindTimeslice,
		FetchKindSwitchonevent
	};

	/// Fetch kind string map
	static misc::StringMap fetch_kind_map;

	/// Dispatch stage kind
	enum DispatchKind
	{
		DispatchKindInvalid = 0,
		DispatchKindShared,
		DispatchKindTimeslice,
	};

	/// Dispatch kind string map
	static misc::StringMap dispatch_kind_map;

	/// Issue stage kind
	enum IssueKind
	{
		IssueKindInvalid = 0,
		IssueKindShared,
		IssueKindTimeslice,
	};

	/// Issue kind string map
	static misc::StringMap issue_kind_map;

	/// Commit stage kind
	enum CommitKind
	{
		CommitKindInvalid = 0,
		CommitKindShared,
		CommitKindTimeslice
	};

	/// Commit kind string map
	static misc::StringMap commit_kind_map;

	/// Reorder buffer kind
	enum ReorderBufferKind
	{
		ReorderBufferKindInvalid = 0,
		ReorderBufferKindPrivate,
		ReorderBufferKindShared
	};

	/// Reorder buffer kind string map
	static misc::StringMap reorder_buffer_kind_map;

	/// Instruction queue kind
	enum InstructionQueueKind
	{
		InstructionQueueKindInvalid = 0,
		InstructionQueueKindShared,
		InstructionQueueKindPrivate
	};

	/// Instruction queue kind string map
	static misc::StringMap instruction_queue_kind_map;


private:

	// Associated emulator 
	Emu *emu;

	// Array of cores 
	std::vector<std::unique_ptr<Core>> cores;

	// MMU used by this CPU 
	std::shared_ptr<mem::MMU> mmu;

	// Counter of uop ID assignment 
	long long uop_id_counter = 0;

	// Name of currently simulated stage 
	std::string stage;

	// From all contexts in the 'alloc' list of 'x86_emu', minimum value
	// of variable 'ctx->alloc_cycle'. This value is used to decide whether
	// the scheduler should be called at all to check for any context whose
	// execution quantum has expired. These variables are updated by calling
	// 'x86_cpu_update_min_alloc_cycle'
	long long min_alloc_cycle = 0;

	// List containing uops that need to report an 'end_inst' trace event 
	std::list<std::shared_ptr<Uop>> uop_trace_list;




	//
	// Statistics 
	//

	// Number of fectched micro-instructions
	long long num_fetched_uinst = 0;

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[UInstOpcodeCount];

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[UInstOpcodeCount];

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[UInstOpcodeCount];

	// Committed micro-instructions
	long long num_committed_uinst = 0;

	// Committed macro-instructions
	long long num_committed_inst = 0;

	// Number of squashed micro-instructions
	long long num_squashed_uinst = 0;

	// Number of branch micro-instructions
	long long num_branch_uinst = 0;

	// Number of mis-predicted branch micro-instructions
	long long num_mispred_branch_uinst = 0;




	//
	// For dumping 
	//

	// Cycle of last dump
	long long last_committed = 0;

	// IPC since last dump
	long long last_dump = 0;




	//
	// CPU parameters
	//

	// Number of Cores
	static int num_cores;

	// Number of Threads
	static int num_threads;

	// Context quantum
	static int context_quantum;

	// Thread quantum
	static int thread_quantum;

	// Thread swtich penalty
	static int thread_switch_penalty;




	//
	// CPU stage parameters
	//

	// Recover penalty
	static int recover_penalty;

	// Recover penalty kind
	static RecoverKind recover_kind;

	// CPU fetch parameter
	static FetchKind fetch_kind;

	// CPU decode stage parameter
	static int decode_width;

	// Dispatch width
	static int dispatch_width;

	// Dispath kind
	static DispatchKind dispatch_kind;

	// Issue width
	static int issue_width;

	// Issue kind
	static IssueKind issue_kind;

	// Commit width
	static int commit_width;

	// Commit kind
	static CommitKind commit_kind;




	//
	// Other CPU parameters
	//

	// Flag that indicates CPU ignore any prefetch hints/instructions
	static bool process_prefetch_hints;

	// Flag that indicates CPU to reduce protocol overhead
	static bool use_nc_store;

	// Prefetch history size
	static int prefetch_history_size;

	// Flag that indicates CPU to calculate structures occupancy statistics
	static bool occupancy_stats;




	//
	// Queue/Buffer parameters
	//

	// reorder buffer size
	static int reorder_buffer_size;

	// reorder buffer kind
	static ReorderBufferKind reorder_buffer_kind;

	// Fetch queue
	static int fetch_queue_size;

	// Instruction queue kind
	static InstructionQueueKind instruction_queue_kind;

	// Instruction queue size
	static int instruction_queue_size;

public:

	/// CPU constructor
	CPU();




	//
	// Static member getters
	//

	// Get number of cores
	static int getNumCores() { return num_cores; }

	// Get number of threads
	static int getNumThreads() { return num_threads; }

	// Get context quantum
	static int getContextQuantum() { return context_quantum; }

	// Get thread quantum
	static int getThreadQuantum() { return thread_quantum; }

	// Get thread switch penalty
	static int getThreadSwitchPenalty() { return thread_switch_penalty; }

	// Get reorder buffer size
	static int getReorderBufferSize() { return reorder_buffer_size; }

	// Get reorder buffer kind
	static ReorderBufferKind getReorderBufferKind() { return reorder_buffer_kind; }

	// Get fetch queue size
	static int getFetchQueueSize() { return fetch_queue_size; }

	// Get instruction queue kind
	static InstructionQueueKind getInstructionQueueKind() { return instruction_queue_kind; }

	// Get instruction queue size
	static int getInstructionQueueSize() { return instruction_queue_size; }




	/// Return the core with the given index
	Core *getCore(int index) const
	{
		assert(index >= 0 && index < (int) cores.size());
		return cores[index].get();
	}

	/// Given a core and thread index, return the associated thread object
	Thread *getThread(int core_index, int thread_index) const
	{
		Core *core = getCore(core_index);
		return core->getThread(thread_index);
	}

	/// Dump functions
	void Dump();
	void DumpSummary();
	void DumpReport();
	void DumpUopReport(long long &uop_stats,
			std::string &prefix, int peak_ipc);

	/// Run functions
	int Run();
	void RunStages();
	void FastForward();

	/// Trace functions
	void AddToTraceList(Uop &uop);
	void EmptyTraceList();

	/// Update Occupancy statistic
	void UpdateOccupancyStats();

	/// Read branch predictor configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);
};

}

#endif // ARCH_X86_TIMING_CPU_H
