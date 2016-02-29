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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WAVEFRONT_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WAVEFRONT_H

#include <memory>
#include <vector>

#include <arch/southern-islands/disassembler/Instruction.h>
#include <arch/southern-islands/emulator/WorkItem.h>

namespace SI
{

class WorkGroup;
class WorkItem;
class WavefrontPoolEntry;

/// Polymorphic class used to attach data to a work-group. The timing simulator
/// can use an object derived from this class, instead of adding fields to the
/// Wavefront class.
class WavefrontData
{
public:
	virtual ~WavefrontData();
};


/// This class represents a wavefront, the SIMD execution unit. In AMD, a
/// wavefront is composed of 64 work-items that fetch one instruction and
/// execute it multiple times.
class Wavefront
{
	// Global wavefront identifier
	int id;

	// IDs of work-items it contains
	int work_item_id_first = 0;
	int work_item_id_last = 0;
	int work_item_count = 0;

	// Work-group it belongs to
	WorkGroup *work_group = nullptr;

	// Additional data added by timing simulator
	std::unique_ptr<WavefrontData> data;

	// Program counter. Offset to 'inst_addr' in NDRange where we can find the next
	// instruction to be executed.
	unsigned pc = 0;

	// Current instruction
	std::unique_ptr<Instruction> instruction;
	int inst_size = 0;

	// Associated scalar work-item
	std::unique_ptr<WorkItem> scalar_work_item;

	// Iterator to the first work-item in the wavefront, pointing to a
	// work-item in the list of work-items from the work-group. Work-items
	// within the wavefront can be conveniently accessed with the []
	// operator on this iterator.
	std::vector<std::unique_ptr<WorkItem>>::iterator work_items_begin;

	// Past-the end iterator to the list of work-items forming the
	// work-group. This iterator could be an iterator to valid work-item in
	// the array of work-items of the work-group (pointing to the first
	// work-item that doesn't belong to this wavefront), or it could be a
	// past-the-end iterator to the work-group's work-item list.
	std::vector<std::unique_ptr<WorkItem>>::iterator work_items_end;

	// Scalar registers
	Instruction::Register sreg[256];

	// Associated wavefront pool entry
	WavefrontPoolEntry *wavefront_pool_entry = nullptr;

	// Fields introduced for timing simulation
	bool barrier_instruction = false;




	//
	// Statistics
	//

	// Number of scalar memory instructions executed
	long long scalar_memory_instruction_count = 0;
	
	// Number of scalar ALU instructions executed
	long long scalar_alu_instruction_count = 0;
	
	// Number of branch instructions executed
	long long branch_instruction_count = 0;
	
	// Number of vector memory instructions executed
	long long vector_memory_instruction_count = 0;
	
	// Number of vector ALU instructions executed
	long long vector_alu_instruction_count = 0;
	
	// Number of global memory instructions executed
	long long global_mem_instruction_count = 0;
	
	// Number of lds instructions executed
	long long lds_instruction_count = 0;
	
	// Number of export instructions executed
	long long export_instruction_count = 0;

public:

	/// Constructor
	///
	/// \param work_group
	///	Work-group that the wavefront belongs to
	///
	/// \param id
	///	Global 1D identifier of the wavefront
	///
	Wavefront(WorkGroup *work_group, int id);

	// Counter for per-wavefront identifiers assigned to uops in the timing
	// simulator.
	long long uop_id_counter = 0;

	// Field introduced for the timing simulator.  Contains the Wavefront
	// ID in the associated compute unit
	int id_in_compute_unit = 0;




	//
	// Flags updated during instruction execution
	//

	// Indicates if the instruction being performed is a vector memory read
	// instruction
	bool vector_memory_read = false;
	
	// Indicates if the instruction being performed is a vector memory write
	// instruction
	bool vector_memory_write = false;
	
	// Indicates if the instruction being performed is an atomic vector 
	// memory instruction
	bool vector_memory_atomic = false;
	
	// Indicates if the instruction being performed is a scalar memory read
	// instruction
	bool scalar_memory_read = false;
	
	// Indicates if the instruction being performed is an lds  read
	// instruction
	bool lds_read = false;
	
	// Indicates if the instruction being performed is an lds write
	// instruction
	bool lds_write = false;
	
	// Indicates if the instruction being performed is a wait instruction
	bool memory_wait = false;
	
	// Indicates if the instruction being performed is a barrier instruction
	bool at_barrier = false;
	
	// Indicates if the wavefront execution is done
	bool finished = false;
	
	// Indicates if the instruction being performed is using the GLC bit
	bool vector_memory_global_coherency = false;




	//
	// Getters
	//

	/// Return the global wavefront 1D identifier
	int getId() const { return id; }

	/// Return PC of wavefront
	unsigned getPC() const { return pc; }

	/// Return content in scalar register as unsigned integer
	unsigned getSregUint(int sreg_id) const;

	/// Return pointer to a workitem inside this wavefront
	WorkItem *getWorkItem(int id_in_wavefront)
	{
		assert(id_in_wavefront >= 0 && id_in_wavefront < (int) work_item_count);
		return work_items_begin[id_in_wavefront].get();
	}

	/// Return the associated wavefront pool entry
	WavefrontPoolEntry *getWavefrontPoolEntry() const
	{
		return wavefront_pool_entry;
	}

	/// Return pointer to the workgroup this wavefront belongs to
	WorkGroup *getWorkGroup() const { return work_group; }

	/// Return pointer to the scalar work item
	WorkItem *getScalarWorkItem() const
	{
		return scalar_work_item.get();
	}

	/// Get work_item_count
	unsigned getWorkItemCount() const { return work_item_count; }

	/// Get the associated instruction
	Instruction *getInstruction() const { return instruction.get(); }

	/// Return true if work-item is active. The work-item identifier is
	/// given relative to the first work-item in the wavefront
	bool isWorkItemActive(int id_in_wavefront);

	/// Return true if the wavefront has completed
	bool getFinished() const { return finished; }

	/// Return true if the instruction performed a memory
	/// wait operation
	bool isMemoryWait() const { return memory_wait; }

	bool isBarrierInstruction() const { return barrier_instruction; }

	bool isVectorMemoryGlobalCoherency() const
	{
		return vector_memory_global_coherency;
	}




	//
	// Setters
	//

	/// Set number of work-items
	void setWorkItemCount(unsigned work_item_count)
	{
		this->work_item_count = work_item_count;
	}

	/// Increment the number of work-items
	void incWorkItemCount() { work_item_count++; }

	/// Set PC
	void setPC(unsigned pc) { this->pc = pc; }

	/// Increase PC
	void incPC(int increment) { pc += increment; }

	/// Flag set during instruction emulation indicating that there was a
	/// barrier instruction
	void setBarrierInstruction(bool barrier_instruction) { this->barrier_instruction = barrier_instruction; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a scalar memory read operation.
	void setScalarMemoryRead(bool scalar_memory_read)
	{
		this->scalar_memory_read = scalar_memory_read;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a memory wait operation.
	void setMemoryWait(bool mem_wait) { this->memory_wait = memory_wait; }

	/// Flag set during instruction emulation to indicate that the wavefront
	/// got stalled at a barrier.
	void setAtBarrier(bool at_barrier) { this->at_barrier = at_barrier; }

	/// Flag set during instruction emulation to indicate that the wavefront
	/// finished execution.
	void setFinished(bool finished) { this->finished = finished; }

	/// Flag set during instruction emulation.
	void setVectorMemoryGlobalCoherency(bool vector_mem_global_coherency)
	{
		this->vector_memory_global_coherency = vector_memory_global_coherency;
	}

	/// Set scalar register as an unsigned int
	void setSregUint(int id, unsigned int value);

	/// Set the wavefront pool entry associated with the wavefront
	void setWavefrontPoolEntry(WavefrontPoolEntry *entry)
	{
		wavefront_pool_entry = entry;
	}

	/// Set work_items_begin iterator
	void setWorkItemsBegin(std::vector<std::unique_ptr<WorkItem>>::iterator work_items_begin)
	{
		this->work_items_begin = work_items_begin;
	}

	/// Set work_items_end iterator
	void setWorkItemsEnd(std::vector<std::unique_ptr<WorkItem>>::iterator work_items_end)
	{
		this->work_items_end = work_items_end;
	}

	/// Assign additional data to the wavefront. This operation is typically
	/// done by the timing simulator. Argument \a data is given as a newly
	/// allocated pointer of a class derived from WavefrontData, that the
	/// wavefront will take ownership from.
	void setData(WavefrontData *data) { this->data.reset(data); }

	/// Set value of a scalar register
	/// \param sreg Scalar register identifier
	/// \param value given as an \a unsigned typed value
	void setSReg(int sreg, unsigned value);

	/// Puts a memory descriptor for a constant buffer (e.g. CB0) into sregs
	/// It requires 4 consecutive registers to store the 128-bit structure
	/// \param first_reg Id of the first scalar register
	/// \param num_regs Number of scalar registers to be used, must be 4
	/// \param cb Id of constant buffer, must be 0 or 1
	void setSRegWithConstantBuffer(int first_reg, int num_regs,
			int cb);

	/// Put a pointer to the constant buffer table into 2 consecutive sregs
	/// \param first_reg Id of first scalar register 
	/// \param num_regs Number of scalar registers to be used, must be 2
	void setSRegWithConstantBufferTable(int first_reg, int num_regs);

	/// Puts a memory descriptor for a UAV into sregs
	/// It requires 4 consecutive registers to store the 128-bit structure
	/// \param first_reg Id of the first scalar register
	/// \param num_regs Number of scalar registers to be used, must be 4
	/// \param uav Id of UAV
	void setSRegWithUAV(int first_reg, int num_regs, int uav);

	/// Puts a memory descriptor for a UAV table into sregs
	/// It requires 2 consecutive registers to store the 128-bit structure
	/// \param first_reg Id of the first scalar register
	/// \param num_regs Number of scalar registers to be used, must be 2
	void setSRegWithUAVTable(int first_reg, int num_regs);

	/// Dump wavefront in a human-readable format into output stream \a os
	void Dump(std::ostream &os) const;

	/// Dump wavefront into output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Wavefront &wavefront)
	{
		os << wavefront;
		return os;
	}

	/// Emulate the next instruction in the wavefront at the current
	/// position of the program counter
	void Execute();	

	/// Return an iterator to the first work-item in the wavefront. The
	/// work-items can be conveniently traversed with a loop using these
	/// iterators. This is an example of how to dump all work-items in the
	/// wavefront:
	/// \code
	///	for (auto it = wavefront->getWorkItemsBegin(),
	///			e = wavefront->getWorkItemsEnd();
	///			it != e;
	///			++it)
	/// \endcode
	std::vector<std::unique_ptr<WorkItem>>::iterator getWorkItemsBegin()
	{
		return work_items_begin;
	}

	/// Return a past-the-end iterator for the list of work-items in the
	/// wavefront.
	std::vector<std::unique_ptr<WorkItem>>::iterator getWorkItemsEnd()
	{
		return work_items_end;
	}

	/// Return a new unique sequential identifier for a uop associated with
	/// the wavefront. This function is used by the timing simulator.
	long long getUopId() { return ++uop_id_counter; }
};


}  // namespace SI

#endif
