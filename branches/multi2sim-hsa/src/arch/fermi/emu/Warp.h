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

#include <list>
#include <memory>
#include <vector>

#include <arch/fermi/asm/Inst.h>


namespace Frm
{

class Grid;
class ThreadBlock;
class Thread;
struct FrmInstWrap;

/// Polymorphic class used to attach data to a warp. The timing simulator
/// can use an object derived from this class, instead of adding fields to the
/// Warp class.
class WarpData
{
public:
	virtual ~WarpData();
};

class Warp
{
	struct SyncStackEntry
	{
		unsigned int reconv_pc;
		unsigned int next_path_pc;
		unsigned int active_thread_mask;
	};

	struct SyncStack
	{
		SyncStackEntry entries[32];
	};

	// IDs
	int id;
	int id_in_thread_block;

	// Name
	std::string name;

	// grid and thread-block it belongs to
	Grid *grid;
	ThreadBlock *thread_block;

	// threads
	unsigned thread_count;

	// Additional data added by timing simulator
	std::unique_ptr<WarpData> data;

	// Program counter. Offset in 'inst_buffer' where we can find the next
	// instruction to be executed.
	unsigned pc;
	int inst_size;

	// Current instruction
	FrmInstWrap *inst;

	// Starting/current position in buffer 
	unsigned long long *inst_buffer;
	unsigned inst_buffer_index;
	unsigned inst_buffer_size;

	// Sync stack 
	SyncStackEntry new_entry;
	SyncStack sync_stack;
	int sync_stack_top;
	int sync_stack_pushed;
	int sync_stack_popped;
	unsigned int divergent;
	unsigned int taken;

	// Predicate mask 
	struct bit_map_t *pred;

	// Flags updated during instruction execution 
	int active_mask_push;
	int active_mask_pop;
	unsigned at_barrier_thread_count;
	unsigned finished_thread_count;

	// iterators
	std::list<Warp *>::iterator running_list_iter;
	std::list<Warp *>::iterator barrier_list_iter;
	std::list<Warp *>::iterator finished_list_iter;

	// To measure simulation performance 
	long long emu_inst_count;
	long long emu_time_start;
	long long emu_time_end;

	// Statistics 
	long long inst_count;
	long long global_mem_inst_count;
	long long shared_mem_inst_count;

	// Iterator to the first thread in the warp, pointing to a
	// thread in the list of thread from the warp. Threads
	// within the warp can be conveniently accessed with the []
	// operator on this iterator.
	std::vector<std::unique_ptr<Thread>>::iterator threads_begin;

	// Past-the end iterator to the list of threads forming the
	// thread-block. This iterator could be an iterator to valid thread in
	// the array of threads of the thread-block (pointing to the first
	// thread that doesn't belong to this warp), or it could be a
	// past-the-end iterator to the thread-block's thread list.
	std::vector<std::unique_ptr<Thread>>::iterator threads_end;

public:
	/// Constructor
	///
	/// \param thread_block Thead-block that the warp belongs to
	/// \param id Global 1D identifier of the warp
	Warp(ThreadBlock *thread_block, int id);

	// Getters
	//
	// Return the global warp 1D ID
	int getId() const { return id; }

	// Return PC
	unsigned getPC() const { return pc; }

	// Return pointer to a thread inside this warp
	Thread *getThread(int id_in_warp) {
		assert(id_in_warp >= 0 && id_in_warp < (int) thread_count);
		return threads_begin[id_in_warp].get();
	}

	// Setters
	//
	// Set PC
	void setPC(unsigned pc) { this->pc = pc; }

	// Increase PC
	void incPC(int increment) { pc += increment; }

	/// Dump warp in a human-readable format into output stream \a os
	void Dump(std::ostream &os = std::cout) const;

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

