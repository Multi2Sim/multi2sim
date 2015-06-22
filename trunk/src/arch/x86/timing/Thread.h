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

#ifndef ARCH_X86_TIMING_THREAD_H
#define ARCH_X86_TIMING_THREAD_H

#include <deque>
#include <string>

#include <memory/Module.h>
#include <arch/x86/emulator/UInst.h>
#include <arch/x86/emulator/Context.h>

#include "Uop.h"
#include "BranchPredictor.h"
#include "TraceCache.h"
#include "RegisterFile.h"

namespace x86
{

// Forward declarations
class Core;
class CPU;
class Timing;

/// X86 Thread
class Thread
{
private:

	// Name
	std::string name;



	//
	// Timing simulator, CPU and Core that it belongs to
	//

	// CPU the thread belongs to
	CPU *cpu;

	// Core the thread belongs to
	Core *core;



	//
	// IDs
	//

	// Thread ID in the core
	int id_in_core = 0;

	// Global thread ID in the cpu
	int id_in_cpu = 0;




	//
	// List of mapped contexts
	//

	// Mapped context list
	std::list<std::unique_ptr<Context>> mapped_contexts_list;

	// Context currently running in this thread. This is a context present
	// in the thread's 'mapped' list.
	std::list<std::unique_ptr<Context>>::iterator contexts_iter;

	// Count of context in the list
	int mapped_list_count = 0;

	// The maximum number of context in the list
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

	// Reorder buffer head index
	int reorder_buffer_head = 0;

	// Reorder buffer tail index
	int reorder_buffer_tail = 0;



	//
	// Number of uops in private structures
	//

	// Uop count in instruction queue
	int uop_count_in_instruction_queue = 0;

	// Uop count in load/store queue
	int uop_count_in_load_store_queue = 0;

	// Number of occupied integer registers
	int num_integer_registers_occupied = 0;

	// Number of occupied float point registers
	int num_float_point_registers_occupied = 0;

	// Number of XMM registers
	int num_xmm_registers_occupied = 0;




	//
	// Queues
	//

	// Fetch queue
	std::deque<std::shared_ptr<Uop>> fetch_queue;

	// Uop queue
	std::deque<std::shared_ptr<Uop>> uop_queue;

	// Instruction queue
	std::deque<std::shared_ptr<Uop>> instruction_queue;

	// Load queue
	std::deque<std::shared_ptr<Uop>> load_queue;

	// Store queue
	std::deque<std::shared_ptr<Uop>> store_queue;




	//
	// Component pointer
	//

	// Branch predictor
	std::unique_ptr<BranchPredictor> branch_predictor;

	// Trace cache
	std::unique_ptr<TraceCache> trace_cache;

	// Physical register file
	std::unique_ptr<RegisterFile> reg_file;



	//
	// Fetch
	//

	// eip and next eip
	unsigned int fetch_eip = 0, fetch_neip = 0;

	// Number of bytes occupied in the fetch queue
	int num_bytes_in_fetch_queue  = 0;

	// Number of uops occupied in the trace cache queue
	int num_uop_in_trace_cache_queue  = 0;

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

	// Data memory module entry
	mem::Module *data_module = nullptr;

	// Instructions memory module entry
	mem::Module *inst_module = nullptr;




	// Cycle in which last micro-instruction committed
	long long last_commit_cycle = 0;




	//
	// Statistics
	//

	// Number of fectched micro-instructions
	long long num_fetched_uinst = 0;

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[UInstOpcodeCount] = { };

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[UInstOpcodeCount] = { };

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[UInstOpcodeCount] = { };

	// Number of squashed micro-instructions
	long long num_squashed_uinst = 0;

	// Number of branch micro-instructions
	long long num_branch_uinst = 0;

	// Number of mis-predicted branch micro-instructions
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
	Thread(const std::string &name, CPU *cpu,
			Core *core, int id_in_core);




	//
	// Increment counters
	//

	/// Increment the Uop count in reorder buffer
	void incUopCountInRob() { uop_count_in_rob++; }

	/// Increment the number of occupied physical integer registers
	void incNumIntegerRegistersOccupied() { num_integer_registers_occupied++; }

	/// Increment the number of occupied physical float point registers
	void incNumFloatPointRegistersOccupied() { num_float_point_registers_occupied++; }

	/// Increment the number of occupied physical xmm registers
	void incNumXmmRegistersOccupied() { num_xmm_registers_occupied++; }

	/// Increment the read count of Register Aliasing Table for INT registers
	void incRatIntReads() { rat_int_reads++; }

	/// Increment the read count of Register Aliasing Table for FP registers
	void incRatFpReads() { rat_fp_reads++; }

	/// Increment the read count of Register Aliasing Table for XMM registers
	void incRatXmmReads() { rat_xmm_reads++; }

	/// Increment the write count of Register Aliasing Table for INT registers
	void incRatIntWrites() { rat_int_writes++; }

	/// Increment the write count of Register Aliasing Table for FP registers
	void incRatFpWrites() { rat_fp_writes++; }

	/// Increment the write count of Register Aliasing Table for XMM registers
	void incRatXmmWrites() { rat_xmm_writes++; }

	/// Increment reorder buffer head index
	void incReorderBufferHead()
	{
		reorder_buffer_head == reorder_buffer_right_bound ?
				reorder_buffer_head = reorder_buffer_left_bound :
				reorder_buffer_head++;
	}

	/// Increment reorder buffer tail index
	void incReorderBufferTail()
	{
		reorder_buffer_tail == reorder_buffer_right_bound ?
				reorder_buffer_tail = reorder_buffer_left_bound :
				reorder_buffer_tail++;
	}




	//
	// Decrement counters
	//

	/// Decrement the Uop count in reorder buffer
	void decUopCountInRob() { uop_count_in_rob--; }

	/// Decrement the number of occupied physical integer registers
	void decNumIntegerRegistersOccupied() { num_integer_registers_occupied--; }

	/// Decrement the number of occupied physical float point registers
	void decNumFloatPointRegistersOccupied() { num_float_point_registers_occupied--; }

	/// Decrement the number of occupied physical xmm registers
	void decNumXmmRegistersOccupied() { num_xmm_registers_occupied--; }

	/// Decrement the read count of Register Aliasing Table for INT registers
	void decRatIntReads() { rat_int_reads--; }

	/// Decrement the read count of Register Aliasing Table for FP registers
	void decRatFpReads() { rat_fp_reads--; }

	/// Decrement the read count of Register Aliasing Table for XMM registers
	void decRatXmmReads() { rat_xmm_reads--; }

	/// Increment the write count of Register Aliasing Table for INT registers
	void decRatIntWrites() { rat_int_writes--; }

	/// Increment the write count of Register Aliasing Table for INT registers
	void decRatFpWrites() { rat_fp_writes--; }

	/// Increment the write count of Register Aliasing Table for INT registers
	void decRatXmmWrites() { rat_xmm_writes--; }




	//
	// Setters
	//

	/// Set Tail of reorder buffer
	void setReorderBufferTail(int reorder_buffer_tail)
	{
		this->reorder_buffer_tail = reorder_buffer_tail;
	}

	/// Set cycle until which fetch is stalled
	void setFetchStallUntil(long long fetch_stall_until)
	{
		this->fetch_stall_until = fetch_stall_until;
	}

	/// Set number of fetched micro-instructions
	void setNumFetchedUinst(long long num_fetched_uinst)
	{
		this->num_fetched_uinst = num_fetched_uinst;
	}




	//
	// Getters
	//

	/// Get thread ID in core
	int getIDInCore() const { return id_in_core; }

	/// Get the Uop count in reorder buffer
	int getUopCountInRob() const { return uop_count_in_rob; }

	/// Get the number of occupied physical integer registers
	int getNumIntegerRegistersOccupied() { return num_integer_registers_occupied; }

	/// Get the number of occupied physical float point registers
	int getNumFloatPointRegistersOccupied() { return num_float_point_registers_occupied; }

	/// Get the number of occupied physical xmm registers
	int getNumXmmRegistersOccupied() { return num_xmm_registers_occupied; }

	/// Get the read count of Register Aliasing Table for INT registers
	int getRatIntReads() { return rat_int_reads; }

	/// Get the read count of Register Aliasing Table for INT registers
	int getRatFpReads() { return rat_fp_reads; }

	/// Get the read count of Register Aliasing Table for INT registers
	int getRatXmmReads() { return rat_xmm_reads; }

	/// Get the write count of Register Aliasing Table for INT registers
	int getRatIntWrites() { return rat_int_writes; }

	/// Get the write count of Register Aliasing Table for INT registers
	int getRatFpWrites() { return rat_fp_writes; }

	/// Get the write count of Register Aliasing Table for INT registers
	int getRatXmmWrites() { return rat_xmm_writes; }

	/// Get reorder buffer head index
	int getReorderBufferHead() { return reorder_buffer_head; }

	/// Get reorder buffer tail index
	int getReorderBufferTail() { return reorder_buffer_tail; }

	/// Get reorder buffer left bound
	int getReorderBufferLeftBound() { return reorder_buffer_left_bound; }

	/// Get reorder buffer right bound
	int getReorderBufferRightBound() { return reorder_buffer_right_bound; }

	/// Get cycle until which fetch is stalled
	long long getFetchStallUntil() { return fetch_stall_until; }

	/// Get number of fetched micro-instructions
	long long getNumFetchedUinst() { return num_fetched_uinst; }




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




	//
	// Fetch queue functions
	//

	/// Recover fetch queue
	void RecoverFetchQueue();




	//
	// Instruction queue functions
	//

	/// Check whether Uop can be inserted in the inst queue
	bool CanInsertInInstructionQueue();

	/// Insert Uop into the inst queue
	void InsertInInstructionQueue(std::shared_ptr<Uop> &uop);

	/// Remove Uop from the inst queue
	void RemoveFromInstructionQueue(int index);

	/// Recover inst queue
	void RecoverInstructionQueue();




	//
	// Load/Store queue functions
	//

	/// Check whether Uop can be inserted in the load/store queue
	bool CanInsertInLoadStoreQueue();

	/// Insert Uop into the load/store queue
	void InsertInLoadStoreQueue(std::shared_ptr<Uop> &uop);

	/// Remove Uop from the load/store queue
	void RemoveFromLoadQueue(int index);

	/// Remove Uop from the load/store queue
	void RemoveFromStoreQueue(int index);

	// Remove Uop from the prefetch queue
	void RemoveFromPrefetchQueue(int index);

	/// Recover load/store queue
	void RecoverLoadStoreQueue();




	//
	// Uop queue functions
	//

	/// Recover Uop queue
	void RecoverUopQueue();





	//
	// Event queue functions
	//

	/// Check whether there is uop within current thread with long latency
	bool isLongLatencyInEventQueue();

	/// Check whether there is uop within current thread that caused cache miss
	bool isCacheMissInEventQueue();

	/// Recover the event queue
	void RecoverEventQueue();




	//
	// Fetch stages
	//

	/// Check whether or not the fecth is allowed
	bool CanFetch();

	/// Fetch instructions
	/// Run the emulation of one x86 macro-instruction and create its uops.
	/// If any of the uops is a control uop, this uop will be the return value of
	/// the function. Otherwise, the first decoded uop is returned
	std::shared_ptr<Uop> FetchInstruction(bool fetch_from_trace_cache);

	/// Try to fetch instruction from trace cache.
	/// Return true if there was a hit and fetching succeeded.
	bool FetchFromTraceCache();

	/// Fetch stage function
	void Fetch();
};

}

#endif // ARCH_X86_TIMING_THREAD_H
