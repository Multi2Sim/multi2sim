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

#ifndef ARCH_FERMI_EMU_WARP_H
#define ARCH_FERMI_EMU_WARP_H

#include <memory>
#include <vector>

#include <arch/fermi/asm/Inst.h>

namespace Frm
{

// Macros for special registers
#define Frm_M0 124
#define Frm_VCC 106
#define Frm_VCCZ 251
#define Frm_EXEC 126
#define Frm_EXECZ 252
#define Frm_SCC 253

class ThreadBlock;
class Thread;

/// Polymorphic class used to attach data to a work-group. The timing simulator
/// can use an object derived from this class, instead of adding fields to the
/// Warp class.
class WarpData
{
public:
	virtual ~WarpData();
};


/// This class represents a warp, the FrmMD execution unit. In AMD, a
/// warp is composed of 64 threads that fetch one instruction and
/// execute it multiple times.
class Warp
{
	// Global warp identifier
	int id;
	int id_in_thread_block;

	// Name
	String name;

	// grid and thread-block it belongs to
	Grid *grid;
	ThreadBlock *thread_block;

	// Additional data added by timing simulator
	std::unique_ptr<WarpData> data;

	// Program counter. Offset in 'inst_buffer' where we can find the next
	// instruction to be executed.
	unsigned pc;
	int inst_size;

	// Current instruction
	InstWrap *inst;

	// Starting/current position in buffer 
	unsigned long long int *inst_buffer;
	unsigned int inst_buffer_index;
	unsigned int inst_buffer_size;

	// Sync stack 
	WarpSyncStackEntry new_entry;
	WarpSyncStack sync_stack;
	int sync_stack_top;
	int sync_stack_pushed;
	int sync_stack_popped;
	unsigned int divergent;
	unsigned int taken;

	unsigned int at_barrier_thread_count;
	unsigned int finished_thread_count;

	// Predicate mask 
	struct bit_map_t *pred;  // thread_count elements 

	// Flags updated during instruction execution 
	unsigned int vector_mem_read : 1;
	unsigned int vector_mem_write : 1;
	unsigned int scalar_mem_read : 1;
	unsigned int lds_read : 1;
	unsigned int lds_write : 1;
	unsigned int mem_wait : 1;
	unsigned int at_barrier : 1;
	unsigned int barrier : 1;
	unsigned int finished : 1;
	unsigned int vector_mem_glc : 1;

	// Loop counters 
	// FIXME: Include this as part of the stack to handle nested loops 
	int loop_depth;
	int loop_max_trip_count;
	int loop_trip_count;
	int loop_start;
	int loop_step;
	int loop_index;

	// Flags updated during instruction execution 
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;
	unsigned int push_before_done : 1;  // Indicates whether the stack has been pushed after PRED_SET* instr. 
	unsigned int active_mask_update : 1;
	int active_mask_push;  // Number of entries the stack was pushed 
	int active_mask_pop;  // Number of entries the stack was popped 

	// Linked lists 
	std::list<Warp *> running_list_next;
	std::list<Warp *> running_list_prev;
	std::list<Warp *> barrier_list_next;
	std::list<Warp *> barrier_list_prev;
	std::list<Warp *> finished_list_next;
	std::list<Warp *> finished_list_prev;

	// To measure simulation performance 
	long long emu_inst_count;  // Total emulated instructions 
	long long emu_time_start;
	long long emu_time_end;


	// Fields introduced for architectural simulation 
	int id_in_sm;
	int alu_engine_in_flight;  // Number of in-flight uops in ALU engine 
	long long sched_when;  // GPU cycle when warp was last scheduled 
	int uop_id_counter;
	struct frm_warp_inst_queue_t *warp_inst_queue;
	struct frm_warp_inst_queue_entry_t *warp_inst_queue_entry;


	// Periodic report - used by architectural simulation 
	FILE *periodic_report_file;  // File where report is dumped 
	long long periodic_report_cycle;  // Last cycle when periodic report was updated 
	int periodic_report_inst_count;  // Number of instructions in this interval 
	int periodic_report_local_mem_accesses;  // Number of local memory accesses in this interval 
	int periodic_report_global_mem_writes;  // Number of global memory writes in this interval 
	int periodic_report_global_mem_reads;  // Number of global memory reads in this interval 


	// Statistics 
	long long inst_count;  // Total number of instructions 
	long long global_mem_inst_count;  // Instructions accessing global memory 
	long long local_mem_inst_count;  // Instructions accessing local memory 


public:

	/// Constructor
	///
	/// \param thread_block Thead-block that the warp belongs to
	/// \param id Global 1D identifier of the warp
	Warp(ThreadBlock *thread_block, int id);

	/// Getters
	///
	/// Return the global warp 1D identifier
	int getId() const { return id; }

	/// Return PC of warp
	unsigned getPC() const { return pc; }

	/// Return content in scalar register as unsigned integer
	unsigned getSregUint(int sreg_id) const;

	/// Return pointer to a workitem inside this warp
	Thread *getThread(int id_in_warp) {
		assert(id_in_warp >= 0 && id_in_warp < (int) thread_count);
		return threads_begin[id_in_warp].get();
	}

	/// Setters
	///
	/// Set PC
	void setPC(unsigned pc) { this->pc = pc; }

	/// Increase PC
	void incPC(int increment) { pc += increment; }

	/// Flag set during instruction emulation indicating that there was a
	/// barrier instruction
	void setBarrierInst(bool barrier_inst) { this->barrier_inst = barrier_inst; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a scalar memory read operation.
	void setScalarMemRead(bool scalar_mem_read) { this->scalar_mem_read = scalar_mem_read; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a memory wait operation.
	void setMemWait(bool mem_wait) { this->mem_wait = mem_wait; }

	/// Flag set during instruction emulation to indicate that the warp
	/// got stalled at a barrier.
	void setAtBarrier(bool at_barrier) { this->at_barrier = at_barrier; }

	/// Flag set during instruction emulation to indicate that the warp
	/// finished execution.
	void setFinished(bool finished) { this->finished = finished; }

	/// Flag set during instruction emulation.
	void setVectorMemGlobalCoherency(bool vector_mem_global_coherency) 
		{ this->vector_mem_global_coherency = vector_mem_global_coherency; }

	/// Set scalar register as an unsigned int
	void setSregUint(int id, unsigned int value);

	/// Dump warp in a human-readable format into output stream \a os
	void Dump(std::ostream &os) const;

	/// Dump warp into output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Warp &warp) {
		os << warp;
		return os;
	}

	/// Emulate the next instruction in the warp at the current
	/// position of the program counter
	void Execute();

	/// Return true if thread is active. The thread identifier is
	/// given relative to the first thread in the warp
	bool getThreadActive(int id_in_warp);

	/// Assign additional data to the warp. This operation is typically
	/// done by the timing simulator. Argument \a data is given as a newly
	/// allocated pointer of a class derived from WarpData, that the
	/// warp will take ownership from.
	void setData(WarpData *data) { this->data.reset(data); }

	/// Set value of a scalar register
	/// \param sreg Scalar register identifier
	/// \param value given as an \a unsigned typed value
	void setSReg(int sreg, unsigned value);

	/// Return an iterator to the first thread in the warp. The
	/// threads can be conveniently traversed with a loop using these
	/// iterators. This is an example of how to dump all threads in the
	/// warp:
	/// \code
	///	for (auto i = warp->ThreadsBegin(),
	///			e = warp->ThreadsEnd(); i != e; ++i)
	///		i->Dump(std::cout);
	/// \endcode
	std::vector<std::unique_ptr<Thread>>::iterator ThreadsBegin() {
		return threads_begin;
	}

	/// Return a past-the-end iterator for the list of threads in the
	/// warp.
	std::vector<std::unique_ptr<Thread>>::iterator ThreadsEnd() {
		return threads_end;
	}
};


}  // namespace Frm

#endif

