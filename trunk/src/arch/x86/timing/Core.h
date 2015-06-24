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

#ifndef ARCH_X86_TIMING_CORE_H
#define ARCH_X86_TIMING_CORE_H

#include <vector>
#include <list>
#include <string>

#include <arch/x86/emulator/UInst.h>

#include "Thread.h"
#include "FunctionalUnit.h"


namespace x86
{

// Forward declaration
class Timing;
class CPU;

// Class Core
class Core
{
public:

	/// Dispatch stall reasons
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
	int id;

	// Event queue
	std::list<std::shared_ptr<Uop>> event_queue;

	// Functional unit
	std::unique_ptr<FunctionalUnit> functional_unit;




	//
	// Counters per core
	//

	// Counter for uop ID assignment
	long long num_uop = 0;

	// Counter for uop ID assignment
	long long dispatch_seq = 0;

	// Uop count in instruction queue
	int instruction_queue_count = 0;

	// Uop count in load/store queue
	int load_store_queue_count = 0;

	// Number of occupied integer registers
	int num_integer_registers_occupied = 0;

	// Number of occupied float point registers
	int num_float_point_registers_occupied = 0;

	// Number of XMM registers
	int num_xmm_registers_occupied = 0;




	//
	// Reorder Buffer 
	//

	// Actual reorder buffer in core
	std::vector<std::unique_ptr<Uop>> reorder_buffer;

	// Micro-operation number in the reorder buffer
	int uop_count_in_rob = 0;

	// Total reorder buffer size
	int reorder_buffer_total_size = 0;

	// Reorder buffer head index
	int reorder_buffer_head = 0;

	// Reorder buffer tail index
	int reorder_buffer_tail = 0;




	//
	// Stages
	//

	// Currently fetching thread
	int current_fetch_thread = 0;

	// Cycle for last thread switch (for SwitchOnEvent)
	long long fetch_switch_when = 0;

	// Currently decoding thread
	int current_decode_thread = 0;

	// Currently dispatching thread
	int current_dispatch_thread = 0;

	// Currently issuing thread
	int current_issue_thread = 0;

	// Currently committing thread
	int current_commit_thread = 0;




	//
	// Statistics
	//

	// Number of stalled micro-instruction when dispatch divded by reason
	long long num_dispatch_stall[DispatchStallReasonMax] = { };

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
	// Statistics for shared structures 
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

public:

	/// Constructor
	Core(const std::string &name, CPU *cpu, int id);

	/// Return the number of threads
	int getNumThreads() const { return threads.size(); }

	/// Return the thread with the given index
	Thread *getThread(int index) const
	{
		assert(index >= 0 && index < (int) threads.size());
		return threads[index].get();
	}




	//
	// Increment counters
	//

	/// Increment the number of Uop
	void incNumUop() { num_uop++; }

	/// Increment the number of occupied physical integer registers
	void incNumIntegerRegistersOccupied() { num_integer_registers_occupied++; }

	/// Increment the number of occupied physical float point registers
	void incNumFloatPointRegistersOccupied() { num_float_point_registers_occupied++; }

	/// Increment the number of occupied physical xmm registers
	void incNumXmmRegistersOccupied() { num_xmm_registers_occupied++; }

	/// Increment the Uop count in instruction queue
	void incInstructionQueueCount() { instruction_queue_count++; }

	/// Increment the Uop count in load/store queue
	void incLoadStoreQueueCount() { load_store_queue_count++; }




	//
	// Decrement counters
	//

	/// Decrement the number of occupied physical integer registers
	void decNumIntegerRegistersOccupied() { num_integer_registers_occupied--; }

	/// Decrement the number of occupied physical float point registers
	void decNumFloatPointRegistersOccupied() { num_float_point_registers_occupied--; }

	/// Decrement the number of occupied physical xmm registers
	void decNumXmmRegistersOccupied() { num_xmm_registers_occupied--; }

	/// Decrement the Uop count in instruction queue
	void decInstructionQueueCount() { instruction_queue_count--; }

	/// Decrement the Uop count in load/store queue
	void decLoadStoreQueueCount() { load_store_queue_count--; }




	//
	// Getters
	//

	/// Get core ID
	int getID() { return id; }

	/// Get number of Uop
	int getNumUop() { return num_uop; }

	/// Get event queue
	std::list<std::shared_ptr<Uop>> &getEventQueue() { return event_queue; }

	/// Get the number of occupied physical integer registers
	int getNumIntegerRegistersOccupied() { return num_integer_registers_occupied; }

	/// Get the number of occupied physical float point registers
	int getNumFloatPointRegistersOccupied() { return num_float_point_registers_occupied; }

	/// Get the number of occupied physical xmm registers
	int getNumXmmRegistersOccupied() { return num_xmm_registers_occupied; }

	/// Get the Uop count in instruction queue
	int getInstructionQueueCount() { return instruction_queue_count; }

	/// Get the Uop count in load/store queue
	int getLoadStoreQueueCount() { return load_store_queue_count; }




	//
	// Reorder Buffer Functions
	//

	/// Initialze the reorder buffer
	void InitializeReorderBuffer();

	/// Trim the reorder buffer by excluding the entry containing only nullptr
	void TrimReorderBuffer();

	/// Check whether or not the ROB can be enqueued
	bool CanEnqueueInReorderBuffer(Uop *uop);

	/// Enqueue Uop to reorder buffer
	void EnqueueInReorderBuffer(Uop *uop);

	/// Check whether or not Uop can be dequeued from ROB
	bool CanDequeueFromReorderBuffer(int thread_id);

	/// Get head Uop of the ROB
	Uop *getReorderBufferHead(int thread_id);

	/// Remove the head Uop of the ROB
	void RemoveReorderBufferHead(int thread_id);

	/// get tail Uop of the ROB
	Uop *getReorderBufferTail(int thread_id);

	/// Remove tail Uop of the ROB
	void RemoveReorderBufferTail(int thread_id);

	/// Get ROB entry based on the index
	Uop *getReorderBufferEntry(int index, int thread_id);




	//
	// Event queue functions
	//

	/// Insert Uop into event queue
	void InsertInEventQueue(std::shared_ptr<Uop> uop);

	/// Extract Uop from event queue
	std::shared_ptr<Uop> ExtractFromEventQueue();




	//
	// Pipeline stages
	//

	/// Fetch stage
	void Fetch();
};

}

#endif // ARCH_X86_TIMING_CORE_H