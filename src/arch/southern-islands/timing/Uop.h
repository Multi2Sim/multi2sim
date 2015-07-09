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

	// Unitque identifier of the instruction in the compute unit that it
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

	// Associated work group
	WorkGroup *work_group;

	// Cycle Uop was created
	long long cycle_created;

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
	bool wavefront_last_instruction;

public:

	/// Constructor
	Uop(Wavefront *wavefront, WavefrontPoolEntry *wavefront_pool_entry);

	/// Return the unique identifier assigned in sequential order to the
	/// uop when it was created.
	long long getId() const { return id; }

	/// Return a unique sequential identifier of the uop in the wavefront
	/// that it belongs to.
	long long getIdInWavefront() const { return id_in_wavefront; }

	/// Return a unique sequential identifier of the uop in the compute
	/// unit that it belongs to.
	long long getIdInComputeUnit() const { return id_in_compute_unit; }

	/// Return the instruction associated with the uop
	Instruction *getInstruction() { return &instruction; }

	/// Return the associated wavefront
	Wavefront *getWavefront() const { return wavefront; }

	/// Return the associated work group
	WorkGroup *getWorkGroup() const { return work_group; }

	/// Return the associated compute unit
	ComputeUnit *getComputeUnit() const { return compute_unit; }

	/// Return the associated wavefront pool entry
	WavefrontPoolEntry *getWavefrontPoolEntry() const
	{
		return wavefront_pool_entry;
	}

	/// Set wavefront
	void setWavefront(Wavefront *wavefront)
	{
		this->wavefront = wavefront;
	}

	/// Set work group
	void setWorkGroup(WorkGroup *work_group)
	{
		this->work_group = work_group;
	}

	/// Set compute unit
	void setComputeUnit(ComputeUnit *compute_unit)
	{
		this->compute_unit = compute_unit;
	}

	/// Set the associated wavefront pool entry
	void setWavefrontPoolEntry(WavefrontPoolEntry *wavefront_pool_entry)
	{
		this->wavefront_pool_entry = wavefront_pool_entry;
	}

	/// Set id in compute unit
	void setIdInComputeUnit(long long id_in_compute_unit)
	{
		this->id_in_compute_unit = id_in_compute_unit;
	}

	/// Set cycle created
	void setCycleCreated(long long cycle_created)
	{
		this->cycle_created = cycle_created;
	}

	/// Set the id in associated wavefront
	void setIdInWavefront(long long id_in_wavefront)
	{
		this->id_in_wavefront = id_in_wavefront;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a vector mem read operation.
	void setVectorMemRead(bool vector_mem_read)
	{
		this->vector_mem_read = vector_mem_read;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a vector mem write operation.
	void setVectorMemWrite(bool vector_mem_write)
	{
		this->vector_mem_write = vector_mem_write;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a atomic vector memory operation.
	void setVectorMemAtomic(bool vector_mem_atomic)
	{
		this->vector_mem_atomic = vector_mem_atomic;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a Scalar mem read operation.
	void setScalarMemRead(bool scalar_mem_read)
	{
		this->scalar_mem_read = scalar_mem_read;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a LDS read operation.
	void setLdsRead(bool lds_read)
	{
		this->lds_read = lds_read;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a memory wait operation.
	void setLdsWrite(bool lds_write)
	{
		this->lds_write = lds_write;
	}

	/// Flag set during instruction emulation
	void setWavefrontLastInstruction(bool wavefront_last_instruction)
	{
		this->wavefront_last_instruction = wavefront_last_instruction;
	}

	/// Flag set during instruction emulation to indicate that the
	/// instruction performed a memory wait operation.
	void setMemWait(bool mem_wait)
	{
		this->mem_wait = mem_wait;
	}

	/// Flag set during execution indicating that uop is at a barrier
	void setAtBarrier(bool at_barrier)
	{
		this->at_barrier = at_barrier;
	}

	/// Set the instruction
	void setInstruction(Instruction *instruction)
	{
		this->instruction = *instruction;
	}

	/// Flag set during instruction emulation
	void setVectorMemGlobalCoherency(bool vector_mem_global_coherency)
	{
		this->vector_mem_global_coherency =
				vector_mem_global_coherency;
	}

	/// Cycle in which the uop is first ready after fetch
	long long fetch_ready = 0;

	/// Cycle in which the uop is first ready after being issued to its
	/// corresponding execution unit
	long long issue_ready = 0;
};

}

#endif

