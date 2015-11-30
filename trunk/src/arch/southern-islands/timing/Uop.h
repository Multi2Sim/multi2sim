/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_UOP_H

#include <arch/southern-islands/disassembler/Instruction.h>
#include<arch/southern-islands/emulator/WorkItem.h>


namespace SI
{

// Forward declarations
class ComputeUnit;
class Wavefront;
class WavefrontPoolEntry;
class WorkGroup;


/// Class representing an instruction flowing through the pipelines of the
/// GPU compute units.
class Uop
{
	//
	// Static fields
	//

	// Counter tracking the ID assigned to the last uop created
	static long long id_counter;




	//
	// Class members
	//

	// Unique identifier of the instruction, assigned when created
	long long id;

	// Unique identifier of the instruction in the wavefront that it
	// belongs to. This field is initialized in the constructor.
	long long id_in_wavefront;

	// Unique identifier of the instruction in the compute unit that it
	// belongs to. This field is initialized in the constructor.
	long long id_in_compute_unit;

	// Associated instruction
	Instruction instruction;

	// Associated wavefront, assigned in constructor
	Wavefront *wavefront;

	// Compute unit that the uop belongs to, assigned in constructor
	ComputeUnit *compute_unit;

	// Associated wavefront pool entry, assigned in constructor
	WavefrontPoolEntry *wavefront_pool_entry;

	// Cycle Uop was created
	long long cycle_created;

	// Associated work group
	WorkGroup *work_group;

	// Unique identifier of the associated wavefront pool
	int wavefront_pool_id;

public:

	/// Constructor
	Uop(Wavefront *wavefront, WavefrontPoolEntry *wavefront_pool_entry,
			long long cycle_created,
			WorkGroup *work_group,
			int wavefront_pool_id);

	/// Flags updated during instruction execution
	bool vector_memory_read;
	bool vector_memory_write;
	bool vector_memory_atomic;
	bool scalar_memory_read;
	bool lds_read;
	bool lds_write;
	bool memory_wait;
	bool at_barrier;
	bool finished;
	bool vector_memory_global_coherency;
	bool wavefront_last_instruction;

	/// Part of a GPU instruction specific for each work-item within wavefront
	struct WorkItemInfo
	{
		// For global memory accesses
		unsigned int global_memory_access_address;
		unsigned int global_memory_access_size;

		// Flags
		// Active after instruction emulation
		bool active = true;

		// Mark a work item that has successfully made a cache access
		bool accessed_cache = false;

		// Number of lds_accesses
		int lds_access_count;

		// Information for each lds_access
		WorkItem::MemoryAccess lds_access[WorkItem::MaxLdsAccessesPerInst];
	};

	/// Vector containing work item specific information.  Indices of this
	/// vector must be the same as the corresponding work item's id
	/// in wavefront.
	std::vector<WorkItemInfo> work_item_info_list;

	/// Return the unique identifier assigned in sequential order to the
	/// uop when it was created.
	long long getId() const { return id; }

	/// Return a unique sequential identifier of the uop in the wavefront
	/// that it belongs to.
	long long getIdInWavefront() const { return id_in_wavefront; }

	/// Return a unique sequential identifier of the uop in the compute
	/// unit that it belongs to.
	long long getIdInComputeUnit() const { return id_in_compute_unit; }

	/// Return the unique identifier to the associated wavefront pool
	int getWavefrontPoolId() const { return wavefront_pool_id; }

	/// Return the instruction associated with the uop
	Instruction *getInstruction() { return &instruction; }

	/// Return the associated wavefront
	Wavefront *getWavefront() const { return wavefront; }

	/// Return the associated work group
	WorkGroup *getWorkGroup() const { return work_group; }

	/// Return the cycle the uop was created
	long long getCycleCreated() const { return cycle_created; }

	/// Return the associated compute unit
	ComputeUnit *getComputeUnit() const { return compute_unit; }

	/// Return the associated wavefront pool entry
	WavefrontPoolEntry *getWavefrontPoolEntry() const
	{
		return wavefront_pool_entry;
	}

	/// Set the instruction
	void setInstruction(Instruction *instruction)
	{
		this->instruction = *instruction;
	}

	/// Cycle in which the uop is first ready after fetch
	long long fetch_ready = 0;

	/// Cycle in which the uop is first ready after being issued to its
	/// corresponding execution unit
	long long issue_ready = 0;

	/// Cycle when the uop is first ready after writeback
	long long write_ready = 0;

	/// Cycle when uop is first ready after decode completes
	long long decode_ready = 0;

	/// Cycle when uop is first ready after register access completes
	long long read_ready = 0;

	/// Cycle when uop is first ready after execution completes
	long long execute_ready = 0;

	/// Witness memory access
	int global_memory_witness = 0;
	
	/// Last scalar memory access address
	unsigned int global_memory_access_address = 0;

	/// Last scalar memory access size
	unsigned int global_memory_access_size = 0;

	/// Lds access witness
	int lds_witness = 0;
};

}

#endif

