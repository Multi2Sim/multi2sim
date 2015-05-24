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

#include <vector>
#include <list>

#include <memory/MMU.h>
#include <arch/x86/emu/UInst.h>
#include <arch/x86/emu/Emu.h>

#include "Core.h"
#include "Uop.h"

namespace x86
{

// Forward declaration
class Uop;
class Emu;
class Core;
class MMU;

// Class CPU
class CPU
{
public:

	// Recover_kind 
	enum RecoverKind
	{
		RecoverKindInvalid = 0,
		RecoverKindWriteback,
		RecoverKindCommit
	};
	static misc::StringMap RecoverKindMap;

	// Fetch stage 
	enum FetchKind
	{
		FetchKindInvalid = 0,
		FetchKindShared,
		FetchKindTimeslice,
		FetchKindSwitchonevent
	};
	static misc::StringMap FetchKindMap;

	// Dispatch stage 
	enum DispatchKind
	{
		DispatchKindInvalid = 0,
		DispatchKindShared,
		DispatchKindTimeslice,
	};
	static misc::StringMap DispatchKindMap;

	// Issue stage 
	enum IssueKind
	{
		IssueKindInvalid = 0,
		IssueKindShared,
		IssueKindTimeslice,
	};
	static misc::StringMap IssueKindMap;

	// Commit stage 
	enum CommitKind
	{
		CommitKindShared = 0,
		CommitKindTimeslice
	};
	static misc::StringMap CommitKindMap;


private:

	// Associated emulator 
	Emu *emu;

	// Array of cores 
	std::vector<std::unique_ptr<Core>> cores;

	// MMU used by this CPU 
	std::shared_ptr<mem::MMU> mmu;

	// Some fields 
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
	std::list<std::unique_ptr<Uop>> uop_trace_list;

	// Statistics 
	long long num_fast_forward_inst = 0;  // Fast-forwarded x86 instructions
	long long num_fetched_uinst = 0;
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst = 0;  // Committed micro-instructions
	long long num_committed_inst = 0;  // Committed x86 instructions
	long long num_squashed_uinst = 0;
	long long num_branch_uinst = 0;
	long long num_mispred_branch_uinst = 0;
	double time = 0.0;

	// For dumping 
	long long last_committed = 0;
	long long last_dump = 0;

	// CPU parameter
	static int num_cores;
	static int num_threads;
	static int context_quantum;
	static int thread_quantum;
	static int thread_switch_penalty;

	// CPU recover parameter
	static int recover_penalty;
	static RecoverKind recover_kind;

	// CPU fetch parameter
	static FetchKind fetch_kind;

	// CPU decode stage parameter
	static int decode_width;

	// CPU dispatch stage parameter
	static int dispatch_width;
	static DispatchKind dispatch_kind;

	// CPU issue stage parameter
	static int issue_width;
	static IssueKind issue_kind;

	// CPU commit stage parameter
	static int commit_width;
	static CommitKind commit_kind;

	// Other CPU parameter
	static bool process_prefetch_hints;
	static bool use_nc_store;
	static bool prefetch_history_size;
	static int occupancy_stats;

	// Queue parameter


public:

	/// Create CPU
	CPU();

	/// Static member getters
	static int getNumCores() { return num_cores; }
	static int getNumThreads() { return num_threads; }
	static int getContextQuantum() { return context_quantum; }
	static int getThreadQuantum() { return thread_quantum; }
	static int getThreadSwitchPenalty() { return thread_switch_penalty; }

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
	static void ParseConfiguration(const std::string &section,
			misc::IniFile &config);
};

}

#endif // X86_ARCH_TIMING_CPU_H
