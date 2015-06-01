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
#include "RegisterFile.h"

namespace x86
{

// Forward declarations
class Context;
class Core;
class CPU;
class BranchPredictor;
class TraceCache;
class RegisterFile;
class Uop;

/// X86 Thread
class Thread
{
private:

	// Name
	std::string name;

	// CPU and Core that it belongs to
	CPU *cpu;
	Core *core;

	// IDs
	int id_in_core = 0;
	int id_in_cpu = 0;

	// Context currently running in this thread. This is a context present
	// in the thread's 'mapped' list. 
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;

	// Double-linked list of mapped contexts
	std::list<std::unique_ptr<Context>> mapped_contexts_list;
	int mapped_list_count = 0;
	int mapped_list_max = 0;

	// Reorder buffer
	int rob_count = 0;

	// Number of uops in private structures
	int iq_count = 0;
	int lsq_count = 0;
	int reg_file_int_count = 0;
	int reg_file_fp_count = 0;
	int reg_file_xmm_count = 0;

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
	std::unique_ptr<RegisterFile> reg_file; // physical register file

	// Fetch
	unsigned int fetch_eip = 0, fetch_neip = 0;  // eip and next eip
	int fetchq_occupied  = 0;  // Number of bytes occupied in the fetch queue
	int trace_cache_queue_occupied  = 0;  // Number of uops occupied in the trace cache queue
	unsigned int fetch_block = 0;  // Virtual address of last fetched block
	unsigned int fetch_address = 0;  // Physical address of last instruction fetch
	long long fetch_access_id = 0;  // Module access ID of last instruction fetch
	long long fetch_stall_until = 0;  // Cycle until which fetching is stalled (inclussive)

	// Entries to the memory system
	std::shared_ptr<mem::Module> data_mod;  // Entry for data
	std::shared_ptr<mem::Module> inst_mod;  // Entry for instructions

	// Cycle in which last micro-instruction committed
	long long last_commit_cycle = 0;

	// Statistics
	long long num_fetched_uinst = 0;
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_squashed_uinst = 0;
	long long num_branch_uinst = 0;
	long long num_mispred_branch_uinst = 0;

	// Statistics for structures
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

	long long rat_int_reads = 0;
	long long rat_int_writes = 0;
	long long rat_fp_reads = 0;
	long long rat_fp_writes = 0;
	long long rat_xmm_reads = 0;
	long long rat_xmm_writes = 0;

	long long btb_reads = 0;
	long long btb_writes = 0;

public:

	/// Constructor
	Thread(const std::string &name, CPU *cpu, Core *core, int id_in_core);

	/// Setters


	/// Increment counters
	void incRegFileIntCount() { reg_file_int_count++; }
	void incRegFileFpCount() { reg_file_fp_count++; }
	void incRegFileXmmCount() { reg_file_xmm_count++; }
	void incRatIntReads() { rat_int_reads++; }
	void incRatFpReads() { rat_fp_reads++; }
	void incRatXmmReads() { rat_xmm_reads++; }
	void incRatIntWrites() { rat_int_writes++; }
	void incRatFpWrites() { rat_fp_writes++; }
	void incRatXmmWrites() { rat_xmm_writes++; }

	/// Decrement counters
	void decRegFileIntCount() { reg_file_int_count--; }
	void decRegFileFpCount() { reg_file_fp_count--; }
	void decRegFileXmmCount() { reg_file_xmm_count--; }
	void decRatIntReads() { rat_int_reads--; }
	void decRatFpReads() { rat_fp_reads--; }
	void decRatXmmReads() { rat_xmm_reads--; }
	void decRatIntWrites() { rat_int_writes--; }
	void decRatFpWrites() { rat_fp_writes--; }
	void decRatXmmWrites() { rat_xmm_writes--; }

	/// Getters
	int getIDInCore() const { return id_in_core; }
	int getRegFileIntCount() { return reg_file_int_count; }
	int getRegFileFpCount() { return reg_file_fp_count; }
	int getRegFileXmmCount() { return reg_file_xmm_count; }
	int getRatIntReads() { return rat_int_reads; }
	int getRatFpReads() { return rat_fp_reads; }
	int getRatXmmReads() { return rat_xmm_reads; }
	int getRatIntWrites() { return rat_int_writes; }
	int getRatFpWrites() { return rat_fp_writes; }
	int getRatXmmWrites() { return rat_xmm_writes; }

	/// Check whether the pipeline is empty
	bool IsPipelineEmpty();

};

}

#endif // X86_ARCH_TIMING_THREAD_H
