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

	// Program counter. Offset in 'inst_buffer' where we can find the next
	// instruction to be executed.
	unsigned pc;

	// Current instruction
	struct SIInstWrap *inst;
	int inst_size;

	// Associated scalar work-item
	WorkItem *scalar_work_item;

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
	bool vector_mem_glc;

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

	friend class WorkItem;

	/// Constructor
	///
	/// \param work_group Work-group that the wavefront belongs to
	/// \param id Global 1D identifier of the wavefront
	Wavefront(WorkGroup *work_group, int id);

	/// Return the global wavefront 1D identifier
	int getId() { return id; }

	/// Flag set during instruction emulation indicating that there was a
	/// vector memory read.
	void setVectorMemRead() { vector_mem_read = true; }

	/// Flag set during instruction emulation indicating that there was a
	/// vector memory write.
	void setVectorMemWrite() { vector_mem_write = true; }

	/// Flags set during instruction emulation to indicate that there was an
	/// atomic vector memory operation.
	void setVectorMemAtomic() { vector_mem_atomic = true; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a scalar memory read operation.
	void setScalarMemRead() { scalar_mem_read = true; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a local memory read.
	void setLDSRead() { lds_read = true; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a local memory write.
	void setLDSWrite() { lds_write = true; }

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a memory wait operation.
	void setMemWait() { mem_wait = true; }

	/// Flag set during instruction emulation to indicate that the wavefront
	/// got stalled at a barrier.
	void setAtBarrier() { at_barrier = true; }

	/// Flag set during instruction emulation to indicate that the wavefront
	/// finished execution.
	void setFinished() { finished = true; }

	/// Flag set during instruction emulation.
	// FIXME ??? Expand variable name to clarify meaning
	void setVectorMemGLC() { vector_mem_glc = true; }


	// Getters for statistics

	/// Statistics showing the total number of instuctions
	long long getInstCount() { return inst_count; }

	/// Statistics showing the total number of calar memory instuctions
	long long getScalarMemInstCount() { return scalar_mem_inst_count; }

	/// Statistics showing the total number of scalar alu instuctions
	long long getScalarAluInstCount() { return scalar_alu_inst_count; }

	/// Statistics showing the total number of branch instuctions
	long long getIBranchInstCount() { return branch_inst_count; }

	/// Statistics showing the total number of vector memory instuctions
	long long getVectorMemInstCount() { return vector_mem_inst_count; }

	/// Statistics showing the total number of instuctions
	long long getVectorAluInstCount() { return vector_alu_inst_count; }

	/// Statistics showing the total number of global memory instuctions
	long long getGlobalMemInstCount() { return global_mem_inst_count; }

	/// Statistics showing the total number of LDS instuctions
	long long getLdsInstCount() { return lds_inst_count; }

	/// Statistics showing the total number of export instuctions
	long long getExportInstCount() { return export_inst_count; }

	/// Statistics showing the total number of emulated instuctions
	long long getEmuInstCount() { return emu_inst_count; }

	/// Statistics showing the start time of emulation
	long long getEmuTimeStart() { return emu_time_start; }

	/// Statistics showing the end time of emulation
	long long getEmuTimeEnd() { return emu_time_end; }

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

	/// Return true if work-item is active. The work-item identifier is
	/// given relative to the first work-item in the wavefront
	bool getWorkItemActive(int id_in_wavefront);

	/// Assign additional data to the wavefront. This operation is typically
	/// done by the timing simulator. Argument \a data is given as a newly
	/// allocated pointer of a class derived from WavefrontData, that the
	/// wavefront will take ownership from.
	void setData(WavefrontData *data) { this->data.reset(data); }

	/// Set value of a scalar register
	/// \param id sreg Scalar register identifier
	/// \param value given as an \a unsigned typed value
	void setSReg(int sreg, unsigned value);

	/// ???
	/// \param first_reg
	/// \param num_regs
	/// \param cb
	void setSRegWithConstantBuffer(int first_reg, int num_regs,
			int cb);

	/// ???
	/// \param first_reg
	/// \param num_regs
	void setSRegWithConstantBufferTable(int first_reg, int num_regs);

	/// ???
	/// \param first_reg
	/// \param num_regs
	void setSRegWithUAVTable(int first_reg, int num_regs);

	/// ???
	/// \param first_reg
	/// \param num_regs
	/// \param uav
	void setSRegWithUAV(int first_reg, int num_regs, int uav);

	/// ???
	/// \param first_reg
	/// \param num_regs
	void setSRegWithBufferTable(int first_reg, int num_regs);

	/// ???
	/// \param first_reg
	/// \param num_regs
	void setSRegWithFetchShader(int first_reg, int num_regs);

	/// Return an iterator to the first work-item in the wavefront. The
	/// work-items can be conveniently traversed with a loop using these
	/// iterators. This is an example of how to dump all work-items in the
	/// wavefront:
	/// \code
	///	for (auto i = wavefront->WorkkItemsBegin(),
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

