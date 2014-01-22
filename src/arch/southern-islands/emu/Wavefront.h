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

#include <arch/southern-islands/asm/Inst.h>

namespace SI
{

// Macros for special registers
#define SI_M0 124
#define SI_VCC 106
#define SI_VCCZ 251
#define SI_EXEC 126
#define SI_EXECZ 252
#define SI_SCC 253

class WorkGroup;
class WorkItem;

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
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	// Work-group it belongs to
	WorkGroup *work_group;

	// Additional data added by timing simulator
	std::unique_ptr<WavefrontData> data;

	// Program counter. Offset to 'inst_addr' in NDRange where we can find the next
	// instruction to be executed.
	unsigned pc;

	// Current instruction
	Inst *inst;
	int inst_size;

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
	InstReg sreg[256];

	// Flags updated during instruction execution
	bool vector_mem_read;
	bool vector_mem_write;
	bool vector_mem_atomic;
	bool scalar_mem_read;
	bool lds_read;
	bool lds_write;
	bool mem_wait;
	bool at_barrier;
	bool finished;
	bool vector_mem_global_coherency;

	// Fields introduced for timing simulation
	int id_in_compute_unit;
	int uop_id_counter;
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;
	bool barrier_inst;

	// Statistics
	long long inst_count;  // Total number of instructions
	long long scalar_mem_inst_count;
	long long scalar_alu_inst_count;
	long long branch_inst_count;
	long long vector_mem_inst_count;
	long long vector_alu_inst_count;
	long long global_mem_inst_count;
	long long lds_inst_count;
	long long export_inst_count;

	// Statistics to measure simulation performance
	long long emu_inst_count;  // Total emulated instructions
	long long emu_time_start;
	long long emu_time_end;

public:

	/// Constructor
	///
	/// \param work_group Work-group that the wavefront belongs to
	/// \param id Global 1D identifier of the wavefront
	Wavefront(WorkGroup *work_group, int id);

	/// Getters
	///
	/// Return the global wavefront 1D identifier
	int getId() const { return id; }

	/// Return PC of wavefront
	unsigned getPC() const { return pc; }

	/// Return content in scalar register as unsigned integer
	unsigned getSregUint(int sreg_id) const;

	/// Return pointer to a workitem inside this wavefront
	WorkItem *getWorkItem(int id_in_wavefront) {
		assert(id_in_wavefront >= 0 && id_in_wavefront < (int) work_item_count);
		return work_items_begin[id_in_wavefront].get();
	}

	/// Return pointer to the workgroup this wavefront belongs to
	WorkGroup *getWorkgroup() const { return work_group; }

	/// Get work_item_count
	unsigned getWorkItemCount() const { return work_item_count; }

	/// Return true if work-item is active. The work-item identifier is
	/// given relative to the first work-item in the wavefront
	bool getWorkItemActive(int id_in_wavefront);

	/// Setters
	///
	/// Set work_item_count
	void setWorkItemCount(unsigned work_item_count) { this->work_item_count = work_item_count; }

	/// Set work_item_count++
	void incWorkItemCount() { work_item_count++; }

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

	/// Flag set during instruction emulation to indicate that the wavefront
	/// got stalled at a barrier.
	void setAtBarrier(bool at_barrier) { this->at_barrier = at_barrier; }

	/// Flag set during instruction emulation to indicate that the wavefront
	/// finished execution.
	void setFinished(bool finished) { this->finished = finished; }

	/// Flag set during instruction emulation.
	void setVectorMemGlobalCoherency(bool vector_mem_global_coherency) 
		{ this->vector_mem_global_coherency = vector_mem_global_coherency; }

	/// Set scalar register as an unsigned int
	void setSregUint(int id, unsigned int value);

	/// Set work_items_begin iterator
	void setWorkItemsBegin(std::vector<std::unique_ptr<WorkItem>>::iterator work_items_begin) {
		this->work_items_begin = work_items_begin;
	}

	/// Set work_items_end iterator
	void setWorkItemsEnd(std::vector<std::unique_ptr<WorkItem>>::iterator work_items_end) {
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

	/// Put a pointer to the Vertex Buffer table into 2 consecutive sregs
	/// \param first_reg Id of the first scalar register
	/// \param num_regs Number of scalar registers to be used, must be 2
	void setSRegWithVertexBufferTable(int first_reg, int num_regs);

	/// Put a pointer to the Fetch Shader into 2 consecutive sregs
	/// \param first_reg Id of the first scalar register
	/// \param num_regs Number of scalar registers to be used, must be 2
	void setSRegWithFetchShader(int first_reg, int num_regs);

	/// Dump wavefront in a human-readable format into output stream \a os
	void Dump(std::ostream &os) const;

	/// Dump wavefront into output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Wavefront &wavefront) {
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
	///	for (auto i = wavefront->WorkItemsBegin(),
	///			e = wavefront->WorkItemsEnd(); i != e; ++i)
	///		i->Dump(std::cout);
	/// \endcode
	std::vector<std::unique_ptr<WorkItem>>::iterator WorkItemsBegin() {
		return work_items_begin;
	}

	/// Return a past-the-end iterator for the list of work-items in the
	/// wavefront.
	std::vector<std::unique_ptr<WorkItem>>::iterator WorkItemsEnd() {
		return work_items_end;
	}
};


}  // namespace SI

#endif

