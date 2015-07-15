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

#include <arch/x86/emulator/Uinst.h>

#include "Thread.h"
#include "FunctionalUnit.h"


namespace x86
{

// Forward declaration
class Timing;
class Cpu;

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

	// Cpu that it belongs to 
	Cpu *cpu;

	// Array of threads 
	std::vector<std::unique_ptr<Thread>> threads;

	// Unique ID in Cpu 
	int id;

	// Event queue
	std::list<std::shared_ptr<Uop>> event_queue;




	//
	// Counters per core
	//

	// Counter used to assign per-core identifiers to uops
	long long uop_id_counter = 0;

	// Number of occupied integer registers
	int num_integer_registers_occupied = 0;

	// Number of occupied float point registers
	int num_float_point_registers_occupied = 0;

	// Number of XMM registers
	int num_xmm_registers_occupied = 0;

	// Total number of instructions in all threads' ROBs
	int reorder_buffer_occupancy = 0;

	// Total number of instructions in all threads' IQs
	int instruction_queue_occupancy = 0;

	// Total number of instructions in all threads' LSQs
	int load_store_queue_occupancy = 0;






	//
	// Fetch stage
	//

	// Currently fetching thread
	int current_fetch_thread = 0;






	//
	// Statistics
	//

	// Number of stalled micro-instruction when dispatch divded by reason
	long long num_dispatch_stall[DispatchStallReasonMax] = { };

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[Uinst::OpcodeCount] = { };

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[Uinst::OpcodeCount] = { };

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[Uinst::OpcodeCount] = { };

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
	Core(Cpu *cpu, int index);

	/// Return the number of threads
	int getNumThreads() const { return threads.size(); }

	/// Return the thread with the given index
	Thread *getThread(int index) const
	{
		assert(index >= 0 && index < (int) threads.size());
		return threads[index].get();
	}

	/// Get the CPU object that this core belongs to
	Cpu *getCpu() const { return cpu; }

	/// Get core index within the CPU
	int getId() const { return id; }

	/// Return a new unique identifier for a uop in this core
	long long getUopId() { return ++uop_id_counter; }



	/// Register file

	/// Get the number of occupied physical integer registers
	int getNumIntegerRegistersOccupied() { return num_integer_registers_occupied; }

	/// Get the number of occupied physical float point registers
	int getNumFloatPointRegistersOccupied() { return num_float_point_registers_occupied; }

	/// Get the number of occupied physical xmm registers
	int getNumXmmRegistersOccupied() { return num_xmm_registers_occupied; }

	/// Increment the number of occupied physical integer registers
	void incNumIntegerRegistersOccupied() { num_integer_registers_occupied++; }

	/// Increment the number of occupied physical float point registers
	void incNumFloatPointRegistersOccupied() { num_float_point_registers_occupied++; }

	/// Increment the number of occupied physical xmm registers
	void incNumXmmRegistersOccupied() { num_xmm_registers_occupied++; }

	/// Decrement the number of occupied physical integer registers
	void decNumIntegerRegistersOccupied()
	{
		assert(num_integer_registers_occupied > 0);
		num_integer_registers_occupied--;
	}

	/// Decrement the number of occupied physical float point registers
	void decNumFloatPointRegistersOccupied()
	{
		assert(num_float_point_registers_occupied > 0);
		num_float_point_registers_occupied--;
	}

	/// Decrement the number of occupied physical xmm registers
	void decNumXmmRegistersOccupied()
	{
		assert(num_xmm_registers_occupied > 0);
		num_xmm_registers_occupied--;
	}





	//
	// Event queue
	//

	/// Insert uop into event queue
	void InsertInEventQueue(std::shared_ptr<Uop> uop);

	/// Extract uop from event queue
	std::shared_ptr<Uop> ExtractFromEventQueue();




	//
	// Reorder buffer
	//

	/// Increment the total number of instructions in all threads' ROBs
	void incReorderBufferOccupancy() { reorder_buffer_occupancy++; }

	/// Decrement the total number of instructions in all threads' ROBs
	void decReorderBufferOccupancy()
	{
		assert(reorder_buffer_occupancy > 0);
		reorder_buffer_occupancy--;
	}

	/// Return the total number of instructions in all threads' ROBs
	int getReorderBufferOccupancy() const { return reorder_buffer_occupancy; }




	//
	// Instruction queue
	//
	
	/// Increment the total number of instructions in all threads' IQs
	void incInstructionQueueOccupancy() { instruction_queue_occupancy++; }

	/// Decrement the total number of instructions in all threads' IQs
	void decInstructionQueueOccupancy()
	{
		assert(instruction_queue_occupancy > 0);
		instruction_queue_occupancy--;
	}

	/// Return the total number of instructions in all threads' IQs
	int getInstructionQueueOccupancy() const
	{
		return instruction_queue_occupancy;
	}




	//
	// Load-Store queue
	//
	
	/// Increment the total number of instructions in all threads' LSQs
	void incLoadStoreQueueOccupancy() { load_store_queue_occupancy++; }

	/// Decrement the total number of instructions in all threads' LSQs
	void decLoadStoreQueueOccupancy()
	{
		assert(load_store_queue_occupancy > 0);
		load_store_queue_occupancy--;
	}

	/// Return the total number of instructions in all threads' LSQs
	int getLoadStoreQueueOccupancy() const
	{
		return load_store_queue_occupancy;
	}




	//
	// Pipeline stages
	//

	/// Run one simulation cycle for all pipeline stages of the core.
	void Run();

	/// Fetch stage
	void Fetch();

	/// Decode stage
	void Decode();

	/// Dispatch stage
	void Dispatch();
};

}

#endif // ARCH_X86_TIMING_CORE_H
