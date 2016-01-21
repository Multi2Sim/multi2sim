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

#include "Alu.h"
#include "Thread.h"


namespace x86
{

// Forward declaration
class Timing;
class Cpu;

// Class Core
class Core
{
private:

	// Name of this core
	std::string name;

	// CPU that it belongs to 
	Cpu *cpu;

	// Array of threads 
	std::vector<std::unique_ptr<Thread>> threads;

	// Unique identifier in the CPU
	int id;

	// Arithmetic-logic unit
	Alu alu;

	// Event queue
	std::list<std::shared_ptr<Uop>> event_queue;




	//
	// Counters per core
	//

	// Counter used to assign per-core identifiers to uops
	long long uop_id_counter = 0;

	// Number of occupied integer registers
	int num_occupied_integer_registers = 0;

	// Number of occupied floating point registers
	int num_occupied_floating_point_registers = 0;

	// Number of XMM registers
	int num_occupied_xmm_registers = 0;

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
	// Dispatch stage
	//

	// Currently dispatching thread
	int current_dispatch_thread = 0;




	//
	// Issue stage
	//

	// Currently issued thread
	int current_issue_thread = 0;




	//
	// Commit stage
	//

	// Currently committed thread
	int current_commit_thread = 0;




	//
	// Statistics
	//

	// Number of stalled micro-instruction when dispatch divded by reason
	long long dispatch_stall[Thread::DispatchStallMax] = {};

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[Uinst::OpcodeCount] = {};

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[Uinst::OpcodeCount] = {};

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[Uinst::OpcodeCount] = {};

	// Number of dispatched micro-instructions
	long long num_dispatched_uinsts = 0;

	// Number of issued micro-instructions
	long long num_issued_uinsts = 0;

	// Number of committed micro-instructions
	long long num_committed_uinsts = 0;

	// Number of squashed micro-instructions
	long long num_squashed_uinsts = 0;

	// Number of branch micro-instructions
	long long num_branches = 0;

	// Number of mis-predicted branch micro-instructions
	long long num_mispredicted_branches = 0;




	//
	// Statistics for shared structures 
	//

	long long num_reorder_buffer_reads = 0;
	long long num_reorder_buffer_writes = 0;

	long long num_instruction_queue_reads = 0;
	long long num_instruction_queue_writes = 0;

	long long num_load_store_queue_reads = 0;
	long long num_load_store_queue_writes = 0;

	long long num_integer_register_reads = 0;
	long long num_integer_register_writes = 0;

	long long num_floating_point_register_reads = 0;
	long long num_floating_point_register_writes = 0;
	
	long long num_xmm_register_reads = 0;
	long long num_xmm_register_writes = 0;

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

	/// Return the core's name
	const std::string &getName() const { return name; }

	/// Return a new unique identifier for a uop in this core
	long long getUopId() { return ++uop_id_counter; }

	/// Return the core's arithmetic-logic unit
	Alu *getAlu() { return &alu; }

	/// Dump a plain-text representation of the object into the given output
	/// stream, or into the standard output if argument \a os is committed.
	void Dump(std::ostream &os = std::cout) const;

	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const Core &core)
	{
		core.Dump(os);
		return os;
	}



	//
	// Register file
	//

	/// Get the number of occupied physical integer registers
	int getNumOccupiedIntegerRegisters() { return num_occupied_integer_registers; }

	/// Get the number of occupied physical float point registers
	int getNumOccupiedFloatingPointRegisters() { return num_occupied_floating_point_registers; }

	/// Get the number of occupied physical xmm registers
	int getNumOccupiedXmmRegisters() { return num_occupied_xmm_registers; }

	/// Increment the number of occupied physical integer registers
	void incNumOccupiedIntegerRegisters() { num_occupied_integer_registers++; }

	/// Increment the number of occupied physical float point registers
	void incNumOccupiedFloatingPointRegisters() { num_occupied_floating_point_registers++; }

	/// Increment the number of occupied physical xmm registers
	void incNumOccupiedXmmRegisters() { num_occupied_xmm_registers++; }

	/// Decrement the number of occupied physical integer registers
	void decNumOccupiedIntegerRegisters()
	{
		assert(num_occupied_integer_registers > 0);
		num_occupied_integer_registers--;
	}

	/// Decrement the number of occupied physical float point registers
	void decNumOccupiedFloatingPointRegisters()
	{
		assert(num_occupied_floating_point_registers > 0);
		num_occupied_floating_point_registers--;
	}

	/// Decrement the number of occupied physical xmm registers
	void decNumOccupiedXmmRegisters()
	{
		assert(num_occupied_xmm_registers > 0);
		num_occupied_xmm_registers--;
	}





	//
	// Event queue
	//

	/// Insert uop into event queue, making it ready to be extract in
	/// \a latency cycles from now. The uop's field `complete_when` is
	/// set to the current cycle plus \a latency in the function.
	void InsertInEventQueue(std::shared_ptr<Uop> uop, int latency);

	/// Extract uop from event queue. The given uop must be placed at the
	/// head of the event queue.
	void ExtractFromEventQueue(Uop *uop);

	/// Return an iterator to the first element of the event queue
	std::list<std::shared_ptr<Uop>>::iterator getEventQueueBegin()
	{
		return event_queue.begin();
	}

	/// Return a past-the-end iterator to the event queue
	std::list<std::shared_ptr<Uop>>::iterator getEventQueueEnd()
	{
		return event_queue.end();
	}




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

	/// Issue stage
	void Issue();

	/// Writeback stage
	void Writeback();

	/// Commit stage
	void Commit();




	//
	// Statistics
	//

	/// Increment the counter for reasons of dispatch stalls by the given
	/// quantum.
	void incDispatchStall(Thread::DispatchStall stall, int quantum)
	{
		assert(stall > Thread::DispatchStallInvalid && stall < Thread::DispatchStallMax);
		dispatch_stall[stall] += quantum;
	}

	/// Return the counter for dispatch stalls of a particular reason.
	long long getDispatchStall(Thread::DispatchStall stall) const
	{
		assert(stall > Thread::DispatchStallInvalid && stall < Thread::DispatchStallMax);
		return dispatch_stall[stall];
	}

	/// Increment the number of reads on reorder buffers
	void incNumReorderBufferReads() { num_reorder_buffer_reads++; }

	/// Return the number of reads in the reorder buffers
	long long getNumReorderBufferReads() const { return num_reorder_buffer_reads; }

	/// Increment the number of writes to a thread's reorder buffer
	void incNumReorderBufferWrites() { num_reorder_buffer_writes++; }

	/// Return the number of writes in the reorder buffers
	long long getNumReorderBufferWrites() const { return num_reorder_buffer_writes; }

	/// Increment the number of reads to a thread's instruction queue
	void incNumInstructionQueueReads() { num_instruction_queue_reads++; }

	/// Return the number of reads from instruction queues
	long long getNumInstructionQueueReads() const { return num_instruction_queue_reads; }

	/// Increment the number of writes to a thread's instruction queue
	void incNumInstructionQueueWrites() { num_instruction_queue_writes++; }

	/// Return the number of writes to instruction queues
	long long getNumInstructionQueueWrites() const { return num_instruction_queue_writes; }

	/// Increment the number of reads from the load-store queue
	void incNumLoadStoreQueueReads() { num_load_store_queue_reads++; }

	/// Return the number of reads from load-store queues
	long long getNumLoadStoreQueueReads() const { return num_load_store_queue_reads; }

	/// Increment the number of writes to a thread's load-store queue
	void incNumLoadStoreQueueWrites() { num_load_store_queue_writes++; }

	/// Return the number of writes to load_store queues
	long long getNumLoadStoreQueueWrites() const { return num_load_store_queue_writes; }
	
	/// Increment the number of dispatched micro-instructions of a given
	/// kind.
	void incNumDispatchedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_dispatched_uinst_array[opcode]++;
		num_dispatched_uinsts++;
	}

	/// Return the array of dispatched micro-instructions.
	const long long *getNumDispatchedUinstArray() const
	{
		return num_dispatched_uinst_array;
	}
	
	/// Return the number of dispatched micro-instructions
	long long getNumDispatchedUinsts() const
	{
		return num_dispatched_uinsts;
	}

	/// Increment the number of issued micro-instructions of a type
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

	/// Increment the number of reads to integer registers
	void incNumIntegerRegisterReads(int count = 1)
	{
		num_integer_register_reads += count;
	}

	/// Return the number of reads on integer registers
	long long getNumIntegerRegisterReads() const
	{
		return num_integer_register_reads;
	}

	/// Increment the number of writes to integer registers
	void incNumIntegerRegisterWrites(int count = 1)
	{
		num_integer_register_writes += count;
	}

	/// Return the number of writes on integer registers
	long long getNumIntegerRegisterWrites() const
	{
		return num_integer_register_writes;
	}

	/// Increment the number of reads to floating-point registers
	void incNumFloatingPointRegisterReads(int count = 1)
	{
		num_floating_point_register_reads += count;
	}

	/// Return the number of reads on floating-point registers
	long long getNumFloatingPointRegisterReads() const
	{
		return num_floating_point_register_reads;
	}

	/// Increment the number of writes to floating-point registers
	void incNumFloatingPointRegisterWrites(int count = 1)
	{
		num_floating_point_register_writes += count;
	}

	/// Return the number of writes on floating-point registers
	long long getNumFloatingPointRegisterWrites() const
	{
		return num_floating_point_register_writes;
	}

	/// Increment the number of reads to XMM registers
	void incNumXmmRegisterReads(int count = 1)
	{
		num_xmm_register_reads += count;
	}

	/// Return the number of XMM register reads
	long long getNumXmmRegisterReads() const
	{
		return num_xmm_register_reads;
	}

	/// Increment the number of writes to XMM registers
	void incNumXmmRegisterWrites(int count = 1)
	{
		num_xmm_register_writes += count;
	}

	/// Return the number of XMM register writes
	long long getNumXmmRegisterWrites() const
	{
		return num_xmm_register_writes;
	}

	/// Increment the number of squashed micro-instructions
	void incNumSquashedUinsts() { num_squashed_uinsts++; }

	/// Return the number of squashed micro-instructions
	long long getNumSquashedUinsts() const { return num_squashed_uinsts; }

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

#endif // ARCH_X86_TIMING_CORE_H
