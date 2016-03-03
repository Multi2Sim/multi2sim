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

#include <deque>
#include <list>
#include <vector>

#include <memory/Mmu.h>
#include <memory/Module.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Uinst.h>

#include "Core.h"
#include "Thread.h"
#include "Uop.h"


namespace x86
{

// Forward declaration
class Timing;

// Class Cpu
class Cpu
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
		FetchKindTimeslice
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

	/// Load/Store queue kind
	enum LoadStoreQueueKind
	{
		LoadStoreQueueKindInvalid = 0,
		LoadStoreQueueKindShared,
		LoadStoreQueueKindPrivate
	};

	/// Load/Store queue kind string map
	static misc::StringMap load_store_queue_kind_map;

	// Maximum number of cycles to simulate
	static long long max_cycles;


private:

	//
	// Static fields
	//

	// Number of cores
	static int num_cores;

	// Number of threads
	static int num_threads;

	// Context quantum
	static int context_quantum;

	// Thread quantum
	static int thread_quantum;

	// Thread swtich penalty
	static int thread_switch_penalty;

	// Number of fast forward instructions
	static long long num_fast_forward_instructions;



	//
	// Class members
	//

	// Associated emulator 
	Emulator *emulator;

	// Associated timing simulator, initialized in constructor
	Timing *timing;

	// Array of cores 
	std::vector<std::unique_ptr<Core>> cores;

	// MMU used by this CPU
	std::shared_ptr<mem::Mmu> mmu;

	// Name of currently simulated stage 
	std::string stage;

	// List containing uops that need to report an 'end_inst' trace event 
	std::list<std::shared_ptr<Uop>> trace_list;




	//
	// Statistics 
	//

	// Number of fectched micro-instructions
	long long num_fetched_uinsts = 0;

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[Uinst::OpcodeCount] = { };

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[Uinst::OpcodeCount] = { };

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[Uinst::OpcodeCount] = { };

	// Number of dispatched micro-instructions
	long long num_dispatched_uinsts = 0;

	// Number of issued micro-instructions
	long long num_issued_uinsts = 0;

	// Number of committed micro-instructions
	long long num_committed_uinsts = 0;


	// Committed macro-instructions
	long long num_committed_instructions = 0;

	// Number of squashed micro-instructions
	long long num_squashed_uinsts = 0;

	// Number of branch micro-instructions
	long long num_branches = 0;

	// Number of mis-predicted branch micro-instructions
	long long num_mispredicted_branches = 0;




	//
	// For dumping 
	//

	// Cycle of last dump
	long long last_committed = 0;

	// IPC since last dump
	long long last_dump = 0;




	//
	// Memory accesses
	//

	// Frame for memory access events
	struct MemoryAccessFrame : public esim::Frame
	{
		// Module to access
		mem::Module *module = nullptr;

		// Access type
		mem::Module::AccessType access_type = mem::Module::AccessInvalid;

		// Physical address to access
		unsigned address = -1;

		// Uop associated with the memory access
		std::shared_ptr<Uop> uop;
	};

	// Event scheduled to start a memory access
	static esim::Event *event_memory_access_start;

	// Event scheduled when a memory access finishes
	static esim::Event *event_memory_access_end;

	// Event handler for memory accesses
	static void MemoryAccessHandler(esim::Event *event, esim::Frame *frame);




	//
	// CPU parameters
	//

	// Recover penalty
	static int recover_penalty;

	// Recover penalty kind
	static RecoverKind recover_kind;

	// Cpu fetch parameter
	static FetchKind fetch_kind;

	// Cpu decode stage parameter
	static int decode_width;

	// Dispatch width
	static int dispatch_width;

	// Dispatch kind
	static DispatchKind dispatch_kind;

	// Issue width
	static int issue_width;

	// Issue kind
	static IssueKind issue_kind;

	// Commit width
	static int commit_width;

	// Commit kind
	static CommitKind commit_kind;

	// Flag that indicates Cpu to calculate structures occupancy statistics
	static bool occupancy_stats;




	//
	// Queue/Buffer parameters
	//

	// reorder buffer size
	static int reorder_buffer_size;

	// reorder buffer kind
	static ReorderBufferKind reorder_buffer_kind;

	// Fetch queue size in bytes
	static int fetch_queue_size;

	// Instruction queue kind
	static InstructionQueueKind instruction_queue_kind;

	// Instruction queue size
	static int instruction_queue_size;

	// Load/Store queue kind
	static LoadStoreQueueKind load_store_queue_kind;

	// Load/Store queue size
	static int load_store_queue_size;

	// Uop queue size
	static int uop_queue_size;

	
	

	//
	// Scheduler
	//

	// From all contexts currently allocated to hardware threads, minimum
	// value of their field 'allocate_cycle', used to decide whether the
	// scheduler should be called at all to check for any context whose
	// execution quantum has expired. This value is updated with a call to
	// UpdateContextAllocationCycle().
	long long min_context_allocate_cycle = 0;

public:

	//
	// Static functions
	//

	/// Get number of cores
	static int getNumCores() { return num_cores; }

	/// Get number of threads
	static int getNumThreads() { return num_threads; }

	/// Get context quantum
	static int getContextQuantum() { return context_quantum; }

	/// Get thread quantum
	static int getThreadQuantum() { return thread_quantum; }

	/// Get thread switch penalty
	static int getThreadSwitchPenalty() { return thread_switch_penalty; }

	/// Get reorder buffer size
	static int getReorderBufferSize() { return reorder_buffer_size; }

	/// Get reorder buffer kind
	static ReorderBufferKind getReorderBufferKind() { return reorder_buffer_kind; }

	/// Return the size of the fetch queue in bytes
	static int getFetchQueueSize() { return fetch_queue_size; }

	/// Get instruction queue kind
	static InstructionQueueKind getInstructionQueueKind() { return instruction_queue_kind; }

	/// Get instruction queue size
	static int getInstructionQueueSize() { return instruction_queue_size; }

	/// Get load/store queue kind
	static LoadStoreQueueKind getLoadStoreQueueKind() { return load_store_queue_kind; }

	/// Get load/store queue size
	static int getLoadStoreQueueSize() { return load_store_queue_size; }

	/// Return the size of the uop queue, as configured by the user
	static int getUopQueueSize() { return uop_queue_size; }

	/// Return the type of instruction fetch, as configured by the user
	static FetchKind getFetchKind() { return fetch_kind; }

	/// Return the decode width, as configured by the user
	static int getDecodeWidth() { return decode_width; }

	/// Return the type of instruction dispatch, as configured by the user
	static DispatchKind getDispatchKind() { return dispatch_kind; }

	/// Return the dispatch width, as configured by the user
	static int getDispatchWidth() { return dispatch_width; }

	/// Return the type of instruction issue, as configured by the user
	static IssueKind getIssueKind() { return issue_kind; }

	/// Return the issue width, as configured by the user
	static int getIssueWidth() { return issue_width; }

	/// Return the kind of recovery upon mispeculation
	static RecoverKind getRecoverKind() { return recover_kind; }

	/// Return the penalty of recovery upon mispeculation
	static int getRecoverPenalty() { return recover_penalty; }

	/// Return the type of instruction commit, as configured by the user
	static CommitKind getCommitKind() { return commit_kind; }

	/// Return the commit width, as configured by the user
	static int getCommitWidth() { return commit_width; }

	/// Return the number of fast foward instructions, as configured by
	/// the user.
	static long long getNumFastForwardInstructions()
	{
		return num_fast_forward_instructions;
	}

	/// Return the maximum number of cycles to simulate, as configured by
	/// the user
	static long long getMaxCycles() { return max_cycles; }
	
	/// Read branch predictor configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);




	//
	// Class members
	//

	/// Constructor
	Cpu(Timing *timing);

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

	/// Get the MMU
	mem::Mmu *getMmu() { return mmu.get(); }

	/// Return the current cycle in the CPU's frequency domain. We cannot
	/// make this function inline to avoid the cross-dependency between
	/// classes Cpu and Timing.
	long long getCycle() const;

	/// Insert an uop into a list of uops that still need to dump an
	/// 'end_inst' trace event. This will happen when the trace list is
	/// emptied with a call to EmptyUopTraceList().
	void InsertInTraceList(std::shared_ptr<Uop> uop);

	/// Empty the uop trace list and make every uop contained in it dump
	/// its last 'end_inst' trace event.
	void EmptyTraceList();

	/// Simulate one cycle of the CPU for all its cores and threads.
	void Run();

	/// Update structure occupancy statistics
	void UpdateOccupancyStats();

	/// Get occupancy statistics flag
	static bool getOccupancyStats() { return occupancy_stats; }

	/// Perform a memory access on the given module for the given address.
	/// When the access completes, the \a uop is inserted in the event
	/// queue of the corresponding core.
	void MemoryAccess(mem::Module *module,
			mem::Module::AccessType access_type,
			unsigned address,
			std::shared_ptr<Uop> uop);




	//
	// Scheduler (CpuScheduler.cc)
	//

	/// Allocate (effectively start running) a context that is already
	/// mapped to a hardware thread.
	void AllocateContext(Context *context);

	/// Map a context to a thread. The thread is chosen with the minimum
	/// number contexts currently mapped to it.
	void MapContext(Context *context);

	/// Recalculate the oldest allocation cycle from all allocated contexts
	/// (i.e., contexts currently occupying the nodes' pipelines). Discard
	/// from the calculation those contexts that have received an eviction
	/// signal (pipelines are being flushed for impending eviction. By
	/// looking at this variable later, we can know right away whether there
	/// is any allocated context that has exceeded its quantum.
	void UpdateContextAllocationCycle();

	/// Main scheduler function. This function should be called every timing
	/// simulation cycle. If no scheduling is required, the function will
	/// exit with practically no cost.
	void Schedule();




	//
	// Stats
	//

	/// Increment the number of fetched micro-instructions
	void incNumFetchedUinsts() { num_fetched_uinsts++; }

	/// Increment the number of dispatched micro-instructions of a kind
	void incNumDispatchedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_dispatched_uinst_array[opcode]++;
		num_dispatched_uinsts++;
	}

	/// Return the array of dispatched micro-instructions
	const long long *getNumDispatchedUinstArray() const
	{
		return num_dispatched_uinst_array;
	}

	/// Return the number of dispatched micro-instructions
	long long getNumDispatchedUinsts() const
	{
		return num_dispatched_uinsts;
	}

	/// Increment the number of issued micro-instructions of a kind
	void incNumIssuedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_issued_uinst_array[opcode]++;
		num_issued_uinsts++;
	}

	/// Return the array of issued micro-instructions
	const long long *getNumIssuedUinstArray() const
	{
		return num_issued_uinst_array;
	}

	/// Return the number of issued micro-instructions
	long long getNumIssuedUinsts() const
	{
		return num_issued_uinsts;
	}

	/// Increment the number of committed micro-instructions of a type
	void incNumCommittedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_committed_uinst_array[opcode]++;
		num_committed_uinsts++;
	}

	/// Return the array of committed micro-instructions
	const long long *getNumCommittedUinstArray() const
	{
		return num_committed_uinst_array;
	}

	/// Return the number of committed micro-instructions
	long long getNumCommittedUinsts() const
	{
		return num_committed_uinsts;
	}

	/// Increment the number of squashed micro-instructions
	void incNumSquashedUinsts() { num_squashed_uinsts++; }

	/// Return the number of squashed micro-instructions
	long long getNumSquashedUinsts() const { return num_squashed_uinsts; }

	/// Increment the number of committed instructions
	void incNumCommittedInstructions() { num_committed_instructions++; }
	
	/// Return the number of committed macro-instructions
	long long getNumCommittedInstructions() const { return num_committed_instructions; }

	/// Increment the number of branches
	void incNumBranches() { num_branches++; }

	/// Return the number of committed branches
	long long getNumBranches() const { return num_branches; }

	/// Increment the number of mispredicted branches
	void incNumMispredictedBranches() { num_mispredicted_branches++; }

	/// Return the number of mispredicted branches
	long long getNumMispredictedBranches() const { return num_mispredicted_branches; }
};

}

#endif // ARCH_X86_TIMING_CPU_H
