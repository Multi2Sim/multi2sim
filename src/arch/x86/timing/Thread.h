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

#ifndef X86_ARCH_TIMING_THREAD_H
#define X86_ARCH_TIMING_THREAD_H

#include <vector>
#include <list>
#include <string>

#include <memory/Module.h>
#include <arch/x86/emu/UInst.h>
#include <arch/x86/emu/Context.h>

#include "Uop.h"
#include "BranchPredictor.h"
#include "TraceCache.h"

namespace x86
{

// Forward declarations
class Context;
class Core;
class Cpu;
class BranchPredictor;
class TraceCache;
class RegFile;

/// X86 Thread
class Thread
{
private:

	// Core and CPU that it belongs to
	Core *core;
	Cpu *cpu;

	// IDs
	int id_in_core;
	int id_in_cpu;

	// Name
	std::string name;

	// Context currently running in this thread. This is a context present
	// in the thread's 'mapped' list. 
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;

	// Double-linked list of mapped contexts
	std::list<std::unique_ptr<Context>> mapped_contexts_list;
	int mapped_list_count;
	int mapped_list_max;

	// Reorder buffer
	int rob_count;
	int rob_left_bound;
	int rob_right_bound;
	int rob_head;
	int rob_tail;

	// Number of uops in private structures
	int iq_count;
	int lsq_count;
	int reg_file_int_count;
	int reg_file_fp_count;
	int reg_file_xmm_count;

	// Private structures
	std::vector<std::unique_ptr<Uop>> fetch_queue;
	std::vector<std::unique_ptr<Uop>> uop_queue;

	// Instruction queue
	std::list<std::unique_ptr<Uop>> iq;

	// Load queue
	std::list<std::unique_ptr<Uop>> lq;

	// Store queue
	std::list<std::unique_ptr<Uop>> sq;

	// Prefetch queue?
	std::list<std::unique_ptr<Uop>> preq;

	// Component pointer
	std::unique_ptr<BranchPredictor> branch_predictor;
	std::unique_ptr<TraceCache> trace_cache;
	std::unique_ptr<RegFile> reg_file; // physical register file

	// Fetch
	unsigned int fetch_eip, fetch_neip;  // eip and next eip
	int fetchq_occ;  // Number of bytes occupied in the fetch queue
	int trace_cache_queue_occ;  // Number of uops occupied in the trace cache queue
	unsigned int fetch_block;  // Virtual address of last fetched block
	unsigned int fetch_address;  // Physical address of last instruction fetch
	long long fetch_access;  // Module access ID of last instruction fetch
	long long fetch_stall_until;  // Cycle until which fetching is stalled (inclussive)

	// Entries to the memory system
	std::shared_ptr<mem::Module> data_mod;  // Entry for data
	std::shared_ptr<mem::Module> inst_mod;  // Entry for instructions

	// Cycle in which last micro-instruction committed
	long long last_commit_cycle;

	// Statistics
	long long num_fetched_uinst;
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_squashed_uinst;
	long long num_branch_uinst;
	long long num_mispred_branch_uinst;

	// Statistics for structures
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

public:

	/// Constructor
	Thread();

	/// Destructor
	~Thread();

	/// Set the name of a thread
	void SetName(std::string &name);

	/// Check whether the pipeline is empty
	bool IsPipelineEmpty();

	///////////////////////////////////////////////////////////////////////
	//
	// Functions implemented in ThreadBranchPredictor.cc. These are the functions
	// related with the Branch prediction.
	//
	///////////////////////////////////////////////////////////////////////

	int LookupBranchPred(Uop &uop);
	int LookupBranchPredMultiple(unsigned int eip, int count);
	void UpdateBranchPred(Uop &uop);

	unsigned int LookupBTB(Uop &uop);
	void UpdateBTB(Uop &uop);
	unsigned int GetNextBranch(unsigned int eip, unsigned int bsize);

	///////////////////////////////////////////////////////////////////////
	//
	// Functions implemented in ThreadTraceCache.cc. These are the functions
	// related with the Trace cache.
	//
	///////////////////////////////////////////////////////////////////////
	void DumpTraceCacheReport();

	void RecordUopInTraceCache(Uop &uop);
	int LookupTraceCache(unsigned int eip, int pred,
			int &mop_count, unsigned int mop_array[], unsigned int &neip);

	///////////////////////////////////////////////////////////////////////
	//
	// Functions implemented in ThreadRegFile.cc. These are the functions
	// related with the register file.
	//
	///////////////////////////////////////////////////////////////////////
	void DumpRegFile();

	int RequestIntReg();
	int RequestFPReg();
	int RequestXMMReg();

	bool CanRenameUop(Uop &uop);
	void RenameUop(Uop &uop);

	bool IsUopReady(Uop &uop);

	void WriteUop(Uop &uop);
	void UndoUop(Uop &uop);
	void CommitUop(Uop &uop);

	void CheckRegFile();


};

}

#endif // X86_ARCH_TIMING_THREAD_H
