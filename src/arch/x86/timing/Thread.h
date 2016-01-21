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
#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/emulator/Context.h>

#include "Uop.h"
#include "BranchPredictor.h"
#include "RegisterFile.h"
#include "TraceCache.h"


namespace x86
{

// Forward declarations
class Core;
class Cpu;
class Timing;

/// X86 Thread
class Thread
{
private:

	// Name, assigned in constructor
	std::string name;

	// Cpu the thread belongs to, assigned in constructor
	Cpu *cpu;

	// Core the thread belongs to, assigned in constructor
	Core *core;

	// Thread index in the core, assigned in constructor
	int id_in_core;

	// Global thread ID in the CPU, assigned in constructor
	int id_in_cpu;




	//
	// Number of uops in private structures
	//

	// Uop count in instruction queue
	int uop_count_in_instruction_queue = 0;

	// Uop count in load/store queue
	int uop_count_in_load_store_queue = 0;




	//
	// Fetch queue
	//

	// Fetch queue
	std::list<std::shared_ptr<Uop>> fetch_queue;

	// Insert a uop into the tail of the fetch queue
	void InsertInFetchQueue(std::shared_ptr<Uop> uop);

	// Extract a uop from the fetch queue. The uop must be located either
	// at the head or at the tail of the fetch queue.
	void ExtractFromFetchQueue(Uop *uop);

	// Dump content of fetch queue
	void DumpFetchQueue(std::ostream &os = std::cout) const;




	//
	// Uop queue
	//

	// Uop queue
	std::list<std::shared_ptr<Uop>> uop_queue;

	// Insert a uop into the tail of the uop queue
	void InsertInUopQueue(std::shared_ptr<Uop> uop);

	// Extract a uop from the uop queue. The uop must be located either at
	// the head or at the tail of the uop queue.
	void ExtractFromUopQueue(Uop *uop);

	// Dump content of uop queue
	void DumpUopQueue(std::ostream &os = std::cout) const;




	//
	// Reorder buffer
	//

	// Reorder buffer
	std::list<std::shared_ptr<Uop>> reorder_buffer;

	// Insert a uop into the tail of the reorder buffer
	void InsertInReorderBuffer(std::shared_ptr<Uop> uop);

	// Determine whether a new uop can be inserted into this thread's
	// reorder buffer, based on whether it is private or shared among
	// threads.
	bool canInsertInReorderBuffer();

	// Extract a uop from the reorder buffer. The uop must be located either
	// at the head or at the tail of the reorder buffer.
	void ExtractFromReorderBuffer(Uop *uop);
	
	// Dump content of reorder buffer
	void DumpReorderBuffer(std::ostream &os = std::cout) const;





	//
	// Instruction Queue
	//

	// Instruction queue
	std::list<std::shared_ptr<Uop>> instruction_queue;

	// Insert a uop into the tail of the instruction queue
	void InsertInInstructionQueue(std::shared_ptr<Uop> uop);

	// Remove a uop from the instruction queue. The uop must be currently
	// present in said queue.
	void ExtractFromInstructionQueue(Uop *uop);
	
	// Determine whether a new uop can be inserted into this thread's
	// instruction queue, based on whether the queue was configured as
	// private per thread, or shared among threads.
	bool canInsertInInstructionQueue();
	
	// Dump content of instruction queue
	void DumpInstructionQueue(std::ostream &os = std::cout) const;




	//
	// Load-store queue
	//
	
	// Load queue
	std::list<std::shared_ptr<Uop>> load_queue;

	// Store queue
	std::list<std::shared_ptr<Uop>> store_queue;

	// Determine whether a new uop can be inserted into this thread's
	// load-store queue, based on whether the queue was configured as
	// private per thread, or shared among threads.
	bool canInsertInLoadStoreQueue();

	// Insert a uop into the tail of the load-store queue (it is in fact
	// inserted either at the tail of the load queue or the store queue,
	// depending on the uop kind).
	void InsertInLoadStoreQueue(std::shared_ptr<Uop> uop);

	// Remove a uop from the load queue. The uop must be currently present
	// in said queue.
	void ExtractFromLoadQueue(Uop *uop);

	// Remove a uop from the store queue. The uop must be currently present
	// in said queue.
	void ExtractFromStoreQueue(Uop *uop);

	// Dump content of load_store queue
	void DumpLoadStoreQueue(std::ostream &os = std::cout) const;




	//
	// Hardware structures
	//

	// Branch predictor
	std::unique_ptr<BranchPredictor> branch_predictor;

	// Trace cache
	std::unique_ptr<TraceCache> trace_cache;

	// Physical register file
	std::unique_ptr<RegisterFile> register_file;




	//
	// Fetch stage
	//

	// Current occupancy of the fetch queue in bytes
	int fetch_queue_occupancy = 0;

	// Current occupancy of the trace cache queue in bytes
	int trace_cache_queue_occupancy = 0;

	// Current instruction pointer
	unsigned int fetch_eip = 0;
	
	// Next instruction pointer
	unsigned int fetch_neip = 0;

	// Number of bytes occupied in the fetch queue
	int num_bytes_in_fetch_queue  = 0;

	// Number of uops occupied in the trace cache queue
	int num_uop_in_trace_cache_queue  = 0;

	// Virtual base address of the first byte in the cache block that was
	// fetched last
	unsigned int fetch_block_address = -1;

	// Physical address of last instruction fetch
	unsigned int fetch_address = 0;

	// Access identifier for of last instruction fetch
	long long fetch_access = 0;

	// Cycle in which last micro-instruction committed
	long long last_commit_cycle = 0;




	//
	// Scheduler
	//

	// List of contexts mapped to this hardware thread
	std::list<Context *> mapped_contexts;




	//
	// Statistics
	//

	// Number of fetched micro-instructions
	long long num_fetched_uinsts = 0;

	// Number of dispatched micro-instructions
	long long num_dispatched_uinsts = 0;

	// Number of issued micro-instructions
	long long num_issued_uinsts = 0;

	// Number of committed micro-instructions
	long long num_committed_uinsts = 0;

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinst_array[Uinst::OpcodeCount] = { };

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinst_array[Uinst::OpcodeCount] = { };

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinst_array[Uinst::OpcodeCount] = { };

	// Number of squashed micro-instructions
	long long num_squashed_uinsts = 0;

	// Number of branch micro-instructions
	long long num_branches = 0;

	// Number of mis-predicted branch micro-instructions
	long long num_mispredicted_branches = 0;




	//
	// Statistics for structures
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

	long long num_btb_reads = 0;
	long long num_btb_writes = 0;

public:

	/// Constructor
	Thread(Core *core, int id_in_core);

	/// Return the thread's name
	const std::string &getName() const { return name; }

	/// Return the core that this thread belongs to
	Core *getCore() const { return core; }

	/// Get thread index within the core
	int getIdInCore() const { return id_in_core; }

	/// Get the thread index within the CPU
	int getIdInCpu() const { return id_in_cpu; }

	/// Return true if there is no uop in the pipeline for this thread
	bool isPipelineEmpty() const
	{
		return fetch_queue.empty()
				&& uop_queue.empty()
				&& reorder_buffer.empty();
	}
	
	/// Dump a plain-text representation of the object into the given output
	/// stream, or into the standard output if argument \a os is committed.
	void Dump(std::ostream &os = std::cout) const;

	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const Thread &thread)
	{
		thread.Dump(os);
		return os;
	}





	//
	// Register file
	//

	/// Return the thread's trace cache
	TraceCache *getTraceCache() const { return trace_cache.get(); }

	/// Return the thread's register file
	RegisterFile *getRegisterFile() const { return register_file.get(); }

	/// Increment the number of writes to integer registers
	void incNumIntegerRegisterWrites(int count = 1)
	{
		num_integer_register_writes += count;
	}

	/// Increment the number of writes to floating-point registers
	void incNumFloatingPointRegisterWrites(int count = 1)
	{
		num_floating_point_register_writes += count;
	}

	/// Increment the number of writes to XMM registers
	void incNumXmmRegisterWrites(int count = 1)
	{
		num_xmm_register_writes += count;
	}




	//
	// Fetch stage (ThreadFetch.cc)
	//

	/// Possible reasons for a fetch stall
	enum FetchStall
	{
		FetchStallInvalid = 0,		// Shouldn't happen
		FetchStallUsed,			// No stall, fetch slot used
		FetchStallContext,		// No context mapped to thread
		FetchStallSuspended,		// Mapped context is suspended
		FetchStallFetchQueue,		// Fetch queue is full
		FetchStallInstructionMemory	// Instruction memory is busy
	};

	/// String map for values of type FetchStall
	static const misc::StringMap fetch_stall_map;

	/// Set the address in instruction memory used for the next instruction
	/// fetch cycle.
	void setFetchNeip(unsigned fetch_neip) { this->fetch_neip = fetch_neip; }

	/// Check whether an instruction can be fetch. If not, return the
	/// reason why fetch is stalled.
	FetchStall canFetch();

	/// Fetch one x86 macro-instruction, run emulation for it, and create
	/// its corresponding set of uops, which are stored at the end of the
	/// fetch queue.
	///
	/// \param fetch_from_trace_cache
	///	Flag indicating whether the uops for the fetched macro-
	///	instruction are considered to come from the trace cache (true)
	///	or from instruction memory (false).
	///
	/// \return
	///	If any of the uops is a branch, the function returns that uop.
	///	Otherwise, it returns the first uop created, or nullptr if no
	///	uop was created.
	///
	Uop *FetchInstruction(bool fetch_from_trace_cache);

	/// Try to fetch instruction from trace cache.
	/// Return true if there was a hit and fetching succeeded.
	bool FetchFromTraceCache();

	/// Fetch stage function
	void Fetch();

	/// Get the fetch queue size in number of uops
	int getFetchQueueSize() const { return fetch_queue.size(); }

	/// Get the fetch queue occupancy
	int getFetchQueueOccupency() const { return fetch_queue_occupancy; }

	/// Get the uop queue size in number of uops
	int getUopQueueSize() const { return uop_queue.size(); }




	//
	// Decode stage (ThreadDecode.cc)
	//

	/// Run decode stage
	void Decode();




	//
	// Dispatch stage (ThreadDispatch.cc)
	//

	/// Possible reasons for a dispatch stall
	enum DispatchStall
	{
		DispatchStallInvalid = 0,
		DispatchStallUsed,		// Used with a committed instruction
		DispatchStallSpeculative,	// Used with a speculative instruction
		DispatchStallUopQueue,		// Stall due to no instruction in the uop queue
		DispatchStallReorderBuffer,	// Stall due to no space in the reorder buffer
		DispatchStallInstructionQueue,	// Stall due to no space in the instruction queue
		DispatchStallLoadStoreQueue,	// Stall due to no space in the load-store queue
		DispatchStallRename,		// Stall due to no free physical register
		DispatchStallContext,		// Stall due to no running context
		DispatchStallMax
	};

	/// String map for values of type DispatchStall
	static const misc::StringMap dispatch_stall_map;

	/// Determine if it is possible to dispatch an instruction for the
	/// thread. If so, the function returns `DispatchStallUsed`. If not
	/// possible, the function returns the reason why it was not possible
	/// to dispatch an instruction. The instruction considered as a
	/// candidate for dispatch is the one at the head of the uop queue.
	DispatchStall canDispatch();

	/// Dispatch \a quantum instructions for the thread. The function
	/// returns the remaining dispatch quantum.
	int Dispatch(int quantum);




	//
	// Issue stage (ThreadIssue.cc)
	//

	/// Issue \a quantum instructions for the thread's load queue, returning
	/// the remaining qunatum.
	int IssueLoadQueue(int quantum);

	/// Issue \a quantum instructions for the thread's store queue,
	/// returning the remaining quantum.
	int IssueStoreQueue(int quantum);

	/// Issue \a quantum instructions for the thread's load-store queue,
	/// giving priority to the load instructions. The function returns the
	/// remaining quantum.
	int IssueLoadStoreQueue(int quantum);

	/// Issue \a quantum instructions for the thread's instruction queue.
	/// The function returns the remaining quantum.
	int IssueInstructionQueue(int quantum);




	//
	// Commit stage (ThreadCommit.cc)
	//

	/// Error message for stalls
	static const char *commit_stall_error;

	/// Return true if at least one uop can be committed for the thread
	bool canCommit();

	/// Commit stage for the thread
	void Commit(int quantum);



	
	//
	// Recovery from mispeculation (ThreadRecover.cc)
	//

	/// Squash mispredicted instructions in fetch queue
	void RecoverFetchQueue();

	/// Squash mispredicted instructions in uop queue
	void RecoverUopQueue();

	/// Squash mispredicted instructions in instruction queue
	void RecoverInstructionQueue();

	/// Squash mispredicted instructions in load queue
	void RecoverLoadQueue();

	/// Squash mispredicted instructions in the store queue
	void RecoverStoreQueue();

	/// Squash mispredicted instructions in event queue
	void RecoverEventQueue();

	/// Recover from mispeculation
	void Recover();




	//
	// Scheduler (ThreadScheduler.cc)
	//

	/// Return the number of contexts currently mapped to the thread.
	int getNumMappedContexts() const { return mapped_contexts.size(); }

	// Map a context to the hardware thread. This means that it gets
	// inserted into the list of mapped contexts.
	void MapContext(Context *context);

	// Unmap a context from a hardware thread, i.e., remove it from the list
	// of contexts mapped to the thread. A context is unmapped from a thread
	// either because it lost affinity with it, or because it finished
	// execution. A context must have been evicted from the thread before
	// being unmapped.
	void UnmapContext(Context *context);
	
	// Activate the 'evict_signal' flag of an allocated context and start
	// flushing the thread's pipeline. Once the last instruction reaches the
	// commit stage, the context will be effectively evicted.
	void EvictContextSignal();

	/// Evict the context currently allocated to the thread. There must
	/// be such a context currently allocated.
	void EvictContext();

	/// Scheduling actions for all contexts currently mapped to a thread.
	void Schedule();




	//
	// Statistics
	//

	/// Increment the number of dispatched micro-instructions of a kind
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

	/// Increment the number of committed micro-instructions of a kind
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

	/// Return the number of squashed micro-instructions
	long long getNumSquashedUinsts() const { return num_squashed_uinsts; }

	/// Return the number of committed branches
	long long getNumBranches() const { return num_branches; }

	/// Return the number of mispredicted branches
	long long getNumMispredictedBranches() const { return num_mispredicted_branches; }

	/// Return the number of reads in the reorder buffers
	long long getNumReorderBufferReads() const { return num_reorder_buffer_reads; }

	/// Return the number of writes in the reorder buffers
	long long getNumReorderBufferWrites() const { return num_reorder_buffer_writes; }

	/// Return the number of reads from instruction queues
	long long getNumInstructionQueueReads() const { return num_instruction_queue_reads; }

	/// Return the number of writes to instruction queues
	long long getNumInstructionQueueWrites() const { return num_instruction_queue_writes; }

	/// Return the number of reads from load-store queues
	long long getNumLoadStoreQueueReads() const { return num_load_store_queue_reads; }

	/// Return the number of writes to load_store queues
	long long getNumLoadStoreQueueWrites() const { return num_load_store_queue_writes; }

	/// Return the number of reads on integer registers
	long long getNumIntegerRegisterReads() const { return num_integer_register_reads; }

	/// Return the number of writes on integer registers
	long long getNumIntegerRegisterWrites() const { return num_integer_register_writes; }

	/// Return the number of reads on floating-point registers
	long long getNumFloatingPointRegisterReads() const { return num_floating_point_register_reads; }

	/// Return the number of writes on floating-point registers
	long long getNumFloatingPointRegisterWrites() const { return num_floating_point_register_writes; }

	/// Return the number of XMM register reads
	long long getNumXmmRegisterReads() const { return num_xmm_register_reads; }

	/// Return the number of XMM register writes
	long long getNumXmmRegisterWrites() const { return num_xmm_register_writes; }

	/// Return the number of reads to the BTB
	long long getNumBtbReads() const { return num_btb_reads; }

	/// Return the number of writes to the BTB
	long long getNumBtbWrites() const { return num_btb_writes; }





	//
	// Public fields
	//

	/// Software context currently mapped to this hardware thread
	Context *context = nullptr;

	/// Memory module used as an entry in the memory hierarchy for data
	/// accesses
	mem::Module *data_module = nullptr;

	/// Memory module used as an entry in the memory hierarchy for
	/// instruction accesses
	mem::Module *instruction_module = nullptr;
};

}

#endif

