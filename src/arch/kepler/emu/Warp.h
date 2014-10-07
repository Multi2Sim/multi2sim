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

#ifndef ARCH_KEPLER_EMU_WARP_H
#define ARCH_KEPLER_EMU_WARP_H

#include <cassert>
#include <list>
#include <memory>
#include <vector>

#include <arch/kepler/asm/Inst.h>
#include <lib/cpp/Bitmap.h>
#include <lib/util/bit-map.h>

#include "Grid.h"
#include "ThreadBlock.h"
#include "Warp.h"


namespace Kepler
{

class Grid;
class ThreadBlock;
class Thread;


/// Polymorphic class used to attach data to a warp. The timing simulator
/// can use an object derived from this class, instead of adding fields to the
/// Warp class.
class WarpData
{
public:

	/// Virtual destructor for polymorphic class.
	virtual ~WarpData();
};

enum EntryType
{
	NOTCONTROL = 0,
	BRA = 1,
	SSY,
	PBK,
	PCNT
};

struct SyncStackEntry
{
	// SSY and BRA's Reconvergence pc for current stack entry
	// or CNT and BRK's jumping pc
	unsigned int reconv_pc;

	// Active mask for BRA, SSY and BRK
	// Or stored active mask bit to be continued for PCTN
	unsigned int active_thread_mask;

	// Instruction type. Use to mark BRA and PBK for now.
	// BRK will use this mark to clear the specific active mask bit
	// From all entries before the PBK entry.
	EntryType entry_type;

	SyncStackEntry(unsigned rec, unsigned am, EntryType et)
	{
		reconv_pc = rec;
		active_thread_mask = am;
		entry_type = et;
	}
	SyncStackEntry()
	{
		reconv_pc = unsigned(-1);
		active_thread_mask = unsigned(-1);
		entry_type = NOTCONTROL;
	}
};

/// The class Warp represents a warp in CUDA.
class Warp
{
	// Unique identifier of the warp in the grid
	int id;

	// Unique identifier of the warp in the thread block.
	int id_in_thread_block;

	// Name  TODO Is it needed?
	//std::string name;

	// Grid the warp belongs to
	Grid *grid;

	// Thread block the warp belongs to
	ThreadBlock *thread_block;

	// threads
	unsigned thread_count;

	// Additional data added by timing simulator
	std::unique_ptr<WarpData> data;

	// Program counter
	unsigned pc;

	// Instruction size by byte
	int inst_size;

	// Target PC for next instruction
	int target_pc;

	// Current instruction
	Inst inst;

	// Pointer points to the starting position of instruction buffer
	std::vector<unsigned long long>::iterator inst_buffer;

	// The whole instruction buffer size in bytes
	unsigned inst_buffer_size;
	
	// current active mask
	unsigned active_mask;

	// Synchronization stack
	std::vector<SyncStackEntry> sync_stack;

	// A temp but must be stored entry of sync_stack
	// used by BRA to set individual bit of active mask
	SyncStackEntry temp_entry;


	// Flags updated during instruction execution TODO still needed?
	int active_mask_push;
	int active_mask_pop;
	unsigned at_barrier_thread_count;

	unsigned finished_thread;
	bool finished_emu;
	bool at_barrier;

	// Flag indicates how many threads are taken at a branch
	unsigned taken_thread;

	// Iterators
	std::list<Warp *>::iterator running_list_iter;
	std::list<Warp *>::iterator barrier_list_iter;
	std::list<Warp *>::iterator finished_list_iter;

	// To measure simulation performance
	// FIXME here and above and below: initializers to fields of basic
	// types, if they are not initialized in the constructor through
	// arguments.
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

/*	// Predicate mask        // Do we still need this in warp? TODO
	//misc::Bitmap pred;  */

public:
	/// Constructor
	///
	/// \param thread_block
	///	Thead-block that the warp belongs to
	///
	/// \param id
	///	Global 1D identifier of the warp
	Warp(ThreadBlock *thread_block, unsigned id);

	/// Return the global warp 1D ID
	int getId() const { return id; }

	// Get Grid that it belongs to
	Grid *getGrid() const { return grid; }

	/// Return the threadblock it belongs to
	ThreadBlock *getThreadBlock() const { return thread_block; }

	/// Return PC
	unsigned getPC() const { return pc; }

	/// Return pointer to a thread inside this warp
	Thread *getThread(int id_in_warp)
	{
		assert(misc::inRange(id_in_warp, 0, (int) thread_count - 1));
		return threads_begin[id_in_warp].get();
	}

	/// Return finished_emu
	bool getFinishedEmu() const { return finished_emu; }

	/// Return at barrier
	bool getAtBarrier() const { return at_barrier; }

	/// Get acitive mask
	unsigned getActiveMask() const { return active_mask; }

	/// Get Sync stack top
	SyncStackEntry getStackTop() const
	{
		return sync_stack.back();
	}

	/// Get number of 1s in active mask
	unsigned getActiveMaskBitCount() const;

	/// Get Sync stack top
	unsigned getStackTopReconvPc() const
	{
		if (sync_stack.size())
               return sync_stack.back().reconv_pc;
		else
			return 0;
	}

	/// Get Sync stack top active thread mask
	unsigned getStackTopActiveThreadMask() const
	{
		return sync_stack.back().active_thread_mask;
	}

	/// Get Sync stack top active thread mask
	EntryType getStackTopEntryType() const
	{
		return sync_stack.back().entry_type;
	}

	/// Get active thread mask stored in temp_entry
	unsigned getTempEntryActiveThreadMask() const
	{
		return temp_entry.active_thread_mask;
	}

	/// Get the number of threads forming the warp.
	unsigned getThreadCount() const { return thread_count; }

	/// Get the number of threads that have finished execution.
	unsigned getFinishedThreadCount() const;

	// FIXME apply formatting changes below, as done above. You continue.
	/// Get inst_size

	int getInstSize() const {return inst_size;}

	/// Get taken thread
	unsigned getTakenThread() const { return taken_thread; }
	//////////////////////////////////////////////////////////////

	// Setters
	//
	/// Set PC
	void setPC(unsigned pc) { this->pc = pc; }

	/// Increase PC
	void incPC(int increment) { pc += increment; }

	/// set threads_begin
	void setThreadBegin(std::vector<std::unique_ptr<Thread>>::iterator value)
	{ threads_begin = value; }

	/// set threads_end
	void setThreadEnd(std::vector<std::unique_ptr<Thread>>::iterator value)
	{ threads_end = value; }

    /// set taken thread
    void setTakenThread(unsigned value) { taken_thread = value; }

    /// increase taken thread by value
    void incrementTakenThread() { taken_thread++; }

    /// Set active mask
    void setActiveMask(unsigned am) { active_mask = am; }

    /// Set temp_entry's active mask bit
    /// \param num
    /// the bit number to be set
    void setTempEntryActiveMaskBit(unsigned num)
    {
    	temp_entry.active_thread_mask |= 1u << num;
    }

    void setTempEntryAddressAndType(unsigned rec_pc, EntryType type)
    {
    	temp_entry.entry_type = type;
    	temp_entry.reconv_pc = rec_pc;
    }

	////////////////////////////////////////////////////////////////////
	// Set finished_emu
    //

    /// Set finished_thread's bit
    /// \param num
    /// the bit number to be set
    void setFinishedThreadBit(unsigned num)
    {
    	finished_thread |= 1u << num;
    }

	void setFinishedEmu (bool value) { finished_emu = value;}

	void setTargetpc ( int target)  {target_pc = target; }

	/// Dump warp in a human-readable format into output stream \a os
	void Dump(std::ostream &os = std::cout) const;

	/// Dump warp into output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Warp &warp) {
		os << warp;
		return os;
	}

    /// Push Synchronization stack
    /// element is inserted instead of "push back" keeping the stack
    /// ordered by reconvergence pc
    void pushStack(SyncStackEntry element);

    /// Push Synchronization stack with temp_entry
    void pushTempEntry();

    /// Pop Synchronization stack
    void popStack();

    /// The atom operation of each thread to clear its bits
    /// in all entries in stack before the latest PBK
    /// used in BRK, declared in warp scope to avoid being verbose
    void BRKStack(unsigned id_in_warp);

    /// Initial (or reset) temp_entry
    void resetTempEntry()
    {
    	temp_entry.active_thread_mask = 0u;
    	temp_entry.entry_type = NOTCONTROL;
        temp_entry.reconv_pc = unsigned(-1);
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

}  // namespace Kepler

#endif
