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

#ifndef ARCH_KEPLER_TIMING_UOP_H
#define ARCH_KEPLER_TIMING_UOP_H

#include <vector>

#include <arch/kepler/disassembler/Instruction.h>


namespace Kepler
{

// Forward declarations
class FetchBuffer;
class Warp;
class WarpPoolEntry;
class ThreadBlock;
class SM;


/// Class representing an instruction flowing through the pipelines of the GPU
/// SM
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

	// Unique identifier of the instruction in the warp that it belongs to. This
	// field is initialized in the constructor.
	long long id_in_warp;

	// Unique identifier of the instruction in the SM that it belongs to. This
	// field is initialized in the constructor.
	long long id_in_sm;

	// Associated fetch buffer, assigned in constructor
	FetchBuffer *fetch_buffer;

	// Associated warp, assigned in constructor
	Warp *warp;

	// Associated warp pool entry, assigned in constructor
	WarpPoolEntry *warp_pool_entry;

	// Cycle Uop was created
	long long cycle_created;

	// Associated thread block
	ThreadBlock *block;

	// SM that the uop belongs to, assigned in constructor
	SM *sm;

	// Unique identifier of the associated warp pool
	int warp_pool_id;

	// Associated Instruction pc
	unsigned instruction_pc;

	// Associated Instruction opcode
	//const char* instruction_name;
	Instruction::Opcode instruction_opcode;

	// Source register index
	int source_register_index[4];

	// Destination register index
	int destination_register_index[4];

	// Source predicate index
	int source_predicate_index[4];

	// Destination predicate index
	int destination_predicate_index[2];

public:

	/// Constructor
	Uop(FetchBuffer *fetch_buffer, Warp *warp, WarpPoolEntry *warp_pool_entry,
			long long cycle_created, ThreadBlock *block, int warp_pool_id,
			int instruction_pc);

	/// Flags updated during instruction execution
	bool at_barrier;
	bool finished;
	bool last_instruction_in_warp = false;
	bool memory_read;
	bool memory_write;


	/// Part of a GPU instruction specific for each thread within warp
	struct ThreadInfo
	{
		// For global memory accesses
		unsigned int global_memory_access_address;
		unsigned int global_memory_access_size;

		// Flags
		// Active after instruction emulation
		bool active = true;

		// Mark a thread that has successfully made a cache access
		bool accessed_cache = false;

		// Number of local memory accesses
		int local_memory_count;

		// Information for each local access
	};

	/// Vector containing thread specific information. Indices of this vector
	/// must be the same as the corresponding thread's id in warp.
	std::vector<ThreadInfo> thread_info_list;

	/// Return the unique identifier assigned in sequential order to the
	/// uop when it was created.
	long long getId() const { return id; }

	/// Return a unique sequential identifier of the uop in the warp that it
	/// belongs to.
	long long getIdInWarp() const { return id_in_warp; }

	/// Return a unique sequential identifier of the uop in the compute unit
	/// that it belongs to.
	long long getIdInSM() const { return id_in_sm; }

	/// Return the unique identifier to the associated warp pool
	int getWarpPoolId() const { return warp_pool_id; }

	/// Return the associated fetch buffer
	FetchBuffer *getFetchBuffer() const { return fetch_buffer; }

	/// Return the associated warp
	Warp *getWarp() const { return warp; }

	/// Return the associated thread block
	ThreadBlock *getThreadBlock() const { return block; }

	/// Return the cycle the uop was created
	long long getCycleCreated() const { return cycle_created; }

	/// Return the associated warp pool entry
	WarpPoolEntry *getWarpPoolEntry() const
	{
		return warp_pool_entry;
	}

	/// Get Instruction pc
	unsigned getInstructionPC() { return instruction_pc; }

	/// Get Instruction opcode
	Instruction::Opcode getInstructionOpcode() { return instruction_opcode; }

	/// Get destination register index by the given index
	int getDestinationRegisterIndex(unsigned index)
	{
		return destination_register_index[index];
	}

	/// Get source register index by the given index
	int getSourceRegisterIndex(unsigned index)
	{
		return source_register_index[index];
	}

	/// Get destination predicate register index by the given index
	int getDestinationPredicateIndex(unsigned index)
	{
		return destination_predicate_index[index];
	}

	/// Get source predicate register index by the given index
	int getSourcePredicateIndex(unsigned index)
	{
		return source_predicate_index[index];
	}

	/// Set source register index
	void setSourceRegisterIndex(unsigned index, int value)
	{
		source_register_index[index] = value;
	}

	/// Set destination register index
	void setDestinationRegisterIndex(unsigned index, int value)
	{
		destination_register_index[index] = value;
	}

	/// Set source predicate register index
	void setSourcePredicateIndex(unsigned index, int value)
	{
		source_predicate_index[index] = value;
	}

	/// Set destination predicate register index
	void setDestinationPredicateIndex(unsigned index, int value)
	{
		destination_register_index[index] = value;
	}

	// Set instruction opcode
	void setInstructionOpcode(Instruction::Opcode opcode)
	{
		instruction_opcode = opcode;
	}

	/// Cycle in which the uop is first ready after fetch
	long long fetch_ready = 0;

	/// Cycle in which the uop is first ready after being issued to its
	/// corresponding execution unit
	long long dispatch_ready = 0;

	/// Cycle when uop is first ready after execution complete
	long long execute_ready = 0;

	// Cycle when the uop is first ready after writeback
	long long write_ready = 0;

	// Cycle when the uop is first ready after read
	long long read_ready = 0;

	/// Witness memory access
	int memory_witness = 0;
};

}

#endif
