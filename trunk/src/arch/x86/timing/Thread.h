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



	//
	// CPU and Core that it belongs to
	//

	// cpu the thread belongs to
	CPU *cpu;

	// core the thread belongs to
	Core *core;



	//
	// IDs
	//

	// thread ID in the core
	int id_in_core = 0;

	// global thread ID in the cpu
	int id_in_cpu = 0;




	// Context currently running in this thread. This is a context present
	// in the thread's 'mapped' list. 
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;




	//
	// Double-linked list of mapped contexts
	//
	std::list<std::unique_ptr<Context>> mapped_contexts_list;
	int mapped_list_count = 0;
	int mapped_list_max = 0;




	//
	// Reorder buffer parameters
	//

	// Micro-operation number in the reorder buffer
	int uop_count_in_rob = 0;

	// Left bound of the reorder buffer
	int reorder_buffer_left_bound = 0;

	// Right bound of the reorder buffer
	int reorder_buffer_right_bound = 0;

	// reorder buffer head index
	int reorder_buffer_head = 0;

	// reorder buffer tail index
	int reorder_buffer_tail = 0;



	//
	// Number of uops in private structures
	//
	int iq_count = 0;
	int lsq_count = 0;
	int reg_file_int_count = 0;
	int reg_file_fp_count = 0;
	int reg_file_xmm_count = 0;




	//
	// Queues
	//

	// Fetch queue
	std::vector<std::unique_ptr<Uop>> fetch_queue;

	// Uop queue
	std::vector<std::unique_ptr<Uop>> uop_queue;

	// Instruction queue
	std::list<std::unique_ptr<Uop>> instruction_queue;

	// Load queue
	std::list<std::unique_ptr<Uop>> load_queue;

	// Store queue
	std::list<std::unique_ptr<Uop>> store_queue;

	// Prefetch queue
	std::list<std::unique_ptr<Uop>> prefetch_queue;




	//
	// Component pointer
	//
	std::unique_ptr<BranchPredictor> branch_predictor;
	std::unique_ptr<TraceCache> trace_cache;
	std::unique_ptr<RegisterFile> reg_file; // physical register file



	//
	// Fetch
	//

	// eip and next eip
	unsigned int fetch_eip = 0, fetch_neip = 0;

	// Number of bytes occupied in the fetch queue
	int fetchq_occupied  = 0;

	// Number of uops occupied in the trace cache queue
	int trace_cache_queue_occupied  = 0;

	// Virtual address of last fetched block
	unsigned int fetch_block = 0;

	// Physical address of last instruction fetch
	unsigned int fetch_address = 0;

	// Module access ID of last instruction fetch
	long long fetch_access_id = 0;

	// Cycle until which fetching is stalled (inclusive)
	long long fetch_stall_until = 0;




	//
	// Entries to the memory system
	//
	mem::Module *data_module = nullptr;  // Entry for data
	mem::Module *inst_module = nullptr;  // Entry for instructions




	// Cycle in which last micro-instruction committed
	long long last_commit_cycle = 0;




	//
	// Statistics
	//
	long long num_fetched_uinst = 0;
	long long num_dispatched_uinst_array[UInstOpcodeCount];
	long long num_issued_uinst_array[UInstOpcodeCount];
	long long num_committed_uinst_array[UInstOpcodeCount];
	long long num_squashed_uinst = 0;
	long long num_branch_uinst = 0;
	long long num_mispred_branch_uinst = 0;




	//
	// Statistics for structures
	//
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




	//
	// Increment counters
	//
	void incUopCountInRob() { uop_count_in_rob++; }
	void incRegFileIntCount() { reg_file_int_count++; }
	void incRegFileFpCount() { reg_file_fp_count++; }
	void incRegFileXmmCount() { reg_file_xmm_count++; }
	void incRatIntReads() { rat_int_reads++; }
	void incRatFpReads() { rat_fp_reads++; }
	void incRatXmmReads() { rat_xmm_reads++; }
	void incRatIntWrites() { rat_int_writes++; }
	void incRatFpWrites() { rat_fp_writes++; }
	void incRatXmmWrites() { rat_xmm_writes++; }

	// Increment reorder buffer head index
	void incReorderBufferHead()
	{
		reorder_buffer_head == reorder_buffer_right_bound ?
				reorder_buffer_head = reorder_buffer_left_bound :
				reorder_buffer_head++;
	}

	// Increment reorder buffer tail index
	void incReorderBufferTail()
	{
		reorder_buffer_tail == reorder_buffer_right_bound ?
				reorder_buffer_tail = reorder_buffer_left_bound :
				reorder_buffer_tail++;
	}




	// Decrement counters
	//
	void decUopCountInRob() { uop_count_in_rob--; }
	void decRegFileIntCount() { reg_file_int_count--; }
	void decRegFileFpCount() { reg_file_fp_count--; }
	void decRegFileXmmCount() { reg_file_xmm_count--; }
	void decRatIntReads() { rat_int_reads--; }
	void decRatFpReads() { rat_fp_reads--; }
	void decRatXmmReads() { rat_xmm_reads--; }
	void decRatIntWrites() { rat_int_writes--; }
	void decRatFpWrites() { rat_fp_writes--; }
	void decRatXmmWrites() { rat_xmm_writes--; }




	//
	// Setters
	//

	// Set Tail of reorder buffer
	void setReorderBufferTail(int reorder_buffer_tail)
	{
		this->reorder_buffer_tail = reorder_buffer_tail;
	}




	//
	// Getters
	//
	int getIDInCore() const { return id_in_core; }
	int getUopCountInRob() const { return uop_count_in_rob; }
	int getRegFileIntCount() const { return reg_file_int_count; }
	int getRegFileFpCount() { return reg_file_fp_count; }
	int getRegFileXmmCount() { return reg_file_xmm_count; }
	int getRatIntReads() { return rat_int_reads; }
	int getRatFpReads() { return rat_fp_reads; }
	int getRatXmmReads() { return rat_xmm_reads; }
	int getRatIntWrites() { return rat_int_writes; }
	int getRatFpWrites() { return rat_fp_writes; }
	int getRatXmmWrites() { return rat_xmm_writes; }
	// Get reorder buffer head index
	int getReorderBufferHead() { return reorder_buffer_head; }

	// Get reorder buffer tail index
	int getReorderBufferTail() { return reorder_buffer_tail; }

	// Get reorder buffer left bound
	int getReorderBufferLeftBound() { return reorder_buffer_left_bound; }

	// Get reorder buffer right bound
	int getReorderBufferRightBound() { return reorder_buffer_right_bound; }




	/// Return the entry module to the memory hierarchy for data
	mem::Module *getDataModule() const { return data_module; }

	/// Assign the entry module to the memory hierarchy for data. This
	/// function should be invoked only once.
	void setDataModule(mem::Module *data_module)
	{
		assert(!this->data_module);
		this->data_module = data_module;
	}

	/// Return the entry module to the memory hierarchy for instructions
	mem::Module *getInstModule() const { return inst_module; }

	/// Assign the entry module to the memory hierarchy for instructions.
	/// This function should be invoked only once.
	void setInstModule(mem::Module *inst_module)
	{
		assert(!this->inst_module);
		this->inst_module = inst_module;
	}

	/// Check whether the pipeline is empty
	bool IsPipelineEmpty();

};

}

#endif // X86_ARCH_TIMING_THREAD_H
