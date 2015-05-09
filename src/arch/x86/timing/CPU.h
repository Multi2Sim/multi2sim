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

#include <arch/common/Timing.h>
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
	misc::StringMap RecoverKindMap
	{
		{"Writeback", RecoverKindWriteback},
		{"Commit", RecoverKindCommit}
	};

	// Fetch stage 
	enum FetchKind
	{
		FetchKindInvalid = 0,
		FetchKindShared,
		FetchKindTimeslice,
		FetchKindSwitchonevent
	};
	misc::StringMap FetchKindMap
	{
		{"Shared", FetchKindShared},
		{"TimeSlice", FetchKindTimeslice},
		{"SwitchOnEvent", FetchKindSwitchonevent},
	};

	// Dispatch stage 
	enum DispatchKind
	{
		DispatchKindInvalid = 0,
		DispatchKindShared,
		DispatchKindTimeslice,
	};
	misc::StringMap DispatchKindMap
	{
		{"Shared", DispatchKindShared},
		{"TimeSlice", DispatchKindTimeslice},
	};

	// Issue stage 
	enum IssueKind
	{
		IssueKindInvalid = 0,
		IssueKindShared,
		IssueKindTimeslice,
	};
	misc::StringMap IssueKindMap
	{
		{"Shared", IssueKindShared},
		{"TimeSlice", IssueKindTimeslice},
	};

	// Commit stage 
	enum CommitKind
	{
		CommitKindShared = 0,
		CommitKindTimeslice
	};
	misc::StringMap CommitKindMap
	{
		{"Shared", CommitKindShared},
		{"TimeSlice", CommitKindTimeslice},
	};


private:

	// Associated emulator 
	Emu *emu;

	// Array of cores 
	std::vector<std::unique_ptr<Core>> cores;

	// MMU used by this CPU 
	std::unique_ptr<mem::MMU> mmu;

	// Some fields 
	// Counter of uop ID assignment 
	long long uop_id_counter;

	// Name of currently simulated stage 
	std::string stage;

	// Used to prevent fetching of new instructions while timing
	// simulator pipeline completes in-flight instructions 
	int flushing;

	// From all contexts in the 'alloc' list of 'x86_emu', minimum value
	// of variable 'ctx->alloc_cycle'. This value is used to decide whether
	// the scheduler should be called at all to check for any context whose
	// execution quantum has expired. These variables are updated by calling
	// 'x86_cpu_update_min_alloc_cycle'
	long long min_alloc_cycle;

	// List containing uops that need to report an 'end_inst' trace event 
	std::list<std::unique_ptr<Uop>> uop_trace_list;

	// Count of current OpenCL ND-Ranges executing on this CPU 
	volatile int ndranges_running;

	// Statistics 
	long long num_fast_forward_inst;  // Fast-forwarded x86 instructions 
	long long num_fetched_uinst;
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst;  // Committed micro-instructions 
	long long num_committed_inst;  // Committed x86 instructions 
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;
	double time;

	// For dumping 
	long long last_committed;
	long long last_dump;

	// CPU parameter
	static int num_cores;
	static int num_threads;
	static int context_quantum;
	static int thread_quantum;
	static int thread_switch_penalty;

	// CPU recover parameter
	static int recover_penalty;

	// CPU decode stage parameter
	static int decode_width;

	// CPU dispatch stage parameter
	static int dispatch_width;

	// CPU issue stage parameter
	static int issue_width;

	// CPU commit stage parameter
	static int commit_width;

public:

	/// Create CPU
	void Create();

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
	void FastForwardOpenCL();

	/// Trace functions
	void AddToTraceList(Uop &uop);
	void EmptyTraceList();

	/// Update Occupancy statistic
	void UpdateOccupancyStats();

	/// Read branch predictor configuration from configuration file
	void ParseConfiguration(const std::string &section,
			misc::IniFile &config);
};

}

#endif // X86_ARCH_TIMING_CPU_H
