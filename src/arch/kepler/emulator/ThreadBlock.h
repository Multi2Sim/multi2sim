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

#ifndef ARCH_KEPLER_EMU_THREADBLOCK_H
#define ARCH_KEPLER_EMU_THREADBLOCK_H

#ifdef __cplusplus

#include <cassert>
#include <list>
#include <memory>
#include <vector>

#include <memory/Memory.h>


namespace Kepler
{

class Emulator;
class Warp;
class Thread;
class Grid;

static const unsigned warp_size = 32;

/// This is a polymorphic class used to attach additional information
/// to a thread-block. It is used by the timing simulator to associate timing
/// simulation information per thread-block.
class ThreadBlockData
{
public:
	/// Virtual destructor to guarantee polymorphism
	virtual ~ThreadBlockData();
};

class ThreadBlock
{
	// Emulator
	Emulator *emulator;

	// ID
	int id;
	int id_3d[3];

	// Name
	std::string name;

	// Grid that it belongs to
	Grid *grid;

	// SM it is mapped to
//	struct Kepler_sm_t *sm; 		//make it clear

	// Iterators
	std::list<Kepler::ThreadBlock *>::iterator pending_list_iter;
	std::list<Kepler::ThreadBlock *>::iterator running_list_iter;
	std::list<Kepler::ThreadBlock *>::iterator finished_list_iter;

	// Warps
	std::vector<std::unique_ptr<Warp>> warps;

	// Threads
	std::vector<std::unique_ptr<Thread>> threads;

	// Barrier information
	unsigned num_warps_at_barrier;
	unsigned num_warps_completed_emu;
	unsigned num_warps_completed_timing;

	bool finished_emu;

	// Shared Memory
	std::unique_ptr<mem::Memory> shared_memory;

	// Constant Memory cache
	std::unique_ptr<mem::Memory> constant_memory_cache;

	// Shared memory size. Field initialized in constructor. Currently set as
	// 16MB
	unsigned shared_memory_size;

	// Shared memory top local address. Field initialized in constructor.
	unsigned shared_memory_top_address;

	// Shared memory top generic address. Field initialized in constructor.
	unsigned shared_memory_top_generic_address;

public:

	/// Constructor
	///
	/// \param grid Instance of class Grid that it belongs to.
	///
	/// \param id Thread-block global 1D ID
	ThreadBlock(Grid *grid, int id, unsigned *id_3d);

	/// Identifier in SM
	int id_in_sm = 0;

	/// Boolean value indicating the status of the thread block in the timing
	/// simulator
	bool finished_timing = false;

	/// Position of this thread block in the SM's list of thread blocks. This
	/// field is managed internally by the SM
	std::list<ThreadBlock*>::iterator sm_thread_block_iterator;

	/// Dump thread-block in human readable format into output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Equivalent to ThreadBlock::Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const ThreadBlock &thread_block) {
		thread_block.Dump(os);
		return os;
	}

	/// Getters
	///
	/// Get ID
	int getId() const { return id; }

	/// Get counter of warps in thread-block
	unsigned getWarpCount() const;

	/// Get counter of warps at barrier
	unsigned getNumWarpsAtBarrier() const { return num_warps_at_barrier; }

	/// Get shared memory size
	unsigned getSharedMemorySize() const { return shared_memory_size; }

	/// Get counter of completed warps
	unsigned getNumWarpsCompletedEmu() const
	{
		return num_warps_completed_emu;
	}

	/// Get counter of warps completed timing
	unsigned getNumWarpsCompletedTiming() const
	{
		return num_warps_completed_timing;
	}

	/// Get Grid that it belongs to
	Grid *getGrid() const { return grid; }

	/// Get pointer of a thread in this thread-block
	Thread *getThread(int id_in_thread_block)
	{
		assert(id_in_thread_block >= 0 && id_in_thread_block <
						(int)threads.size());
		return threads[id_in_thread_block].get();
	}

	/// Get pointer of the given warp in this thread block
	Warp *getWarp(int id_in_thread_block)
	{
		assert(id_in_thread_block >=0 && id_in_thread_block < (int)warps.size());
		return warps[id_in_thread_block].get();
	}

	/// Get counter of threads in thread-block
	int getThreadsCount() const { return threads.size(); }

	/// Get finished_emu
	bool getFinishedEmu() const { return finished_emu; }

	/// Increase warps_at_barrier counter
	void incWarpsAtBarrier() { num_warps_at_barrier++; }

	/// Set warp_at_barrier counter
	void setWarpsAtBarrier(unsigned counter) { num_warps_at_barrier = counter; }

	/// Increment warps_completed_emu counter
	void incWarpsCompletedEmu() { num_warps_completed_emu ++; }

	/// Increment warps_completed_timing counter
	void incWarpsCompletedTiming() { num_warps_completed_timing ++; }

	/// Set finished_emu
	void setFinishedEmu(bool value) { finished_emu = value; }

	/// Write to shared memory
	///
	/// \param addr the address the program will write to
	///
	/// \param buffer the variable the program will read from
	///
	/// \param length data length
	///
	/// \return
	/// No value is returned.
	void WriteToSharedMemory(unsigned address, unsigned length, char* buffer)
	{
		shared_memory->Write(address, length, buffer);
	}

	/// Read shared memory
	///
	/// \param addr the address the program will read from
	///
	/// \param buffer the variable the program will read to
	///
	/// \param length data length
	///
	/// \return
	/// No value is returned
	void ReadFromSharedMemory(unsigned address, unsigned length, char* buffer)
	{
		shared_memory->Read(address, length, buffer);
	}

	/// Write Constant Memory
	/// \param starting address to be written in
	/// \param size of data
	/// \param data buffer
	void WriteConstantMemory(unsigned address, unsigned size, const char *buffer)
	{
		constant_memory_cache->Write(address, size, buffer);
	}

	/// Read Global Memory
	/// \param starting address to be read in
	/// \param size of data
	/// \param data buffer
	void ReadConstantMemory(unsigned address, unsigned size, char *buffer)
	{
		constant_memory_cache->Read(address, size, buffer);
	}

	/// Clear barrier flag in all warps of the threadblock
	/// To continue simulation
	void clearWarpAtBarrier();

	/// Return an iterator to the first work-item in the work-group. The
	/// following code can then be used to iterate over all work-items (and
	/// print them)
	///
	/// \code
	/// for (auto i = work_group->ThreadsBegin(),
	///		e = work_group->ThreadsEnd(); i != e; ++i)
	///	i->Dump(std::cout);
	/// \endcode
	std::vector<std::unique_ptr<Thread>>::iterator ThreadsBegin() {
		return threads.begin();
	}

	///// Return a past-the-end iterator to the list of work-items
	std::vector<std::unique_ptr<Thread>>::iterator ThreadsEnd() {
		return threads.end();
	}

	/// Return an iterator to the first warp in the work-group. The
	/// following code can then be used to iterate over all warps (and
	/// print them)
	///
	/// \code
	/// for (auto i = work_group->WarpsBegin(),
	///		e = work_group->WarpsEnd(); i != e; ++i)
	///	i->Dump(std::cout);
	/// \endcode
	std::vector<std::unique_ptr<Warp>>::iterator WarpsBegin() {
		return warps.begin();
	}

	/// Return a past-the-end iterator to the list of warps
	std::vector<std::unique_ptr<Warp>>::iterator WarpsEnd() {
		return warps.end();
	}
};

}  // namespace

#endif
#endif
