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

#ifndef ARCH_FERMI_EMU_THREADBLOCK_H
#define ARCH_FERMI_EMU_THREADBLOCK_H

#include <cassert>
#include <memory>
#include <vector>

#include <mem-system/Memory.h>


namespace Frm
{

class Warp;
class Thread;
class Grid;

const unsigned warp_size = 64;

/// This is a polymorphic class used to attach additional information
/// to a work-group. It is used by the timing simulator to associate timing
/// simulation information per work-group.
class ThreadBlockData
{
public:
	/// Virtual destructor to guarantee polymorphism
	virtual ~ThreadBlockData();
};


/// This class represents an CUDA work-group, as set up by the host program
/// running on Multi2Sim with a call to \a clEnqueueGridKernel(). The
/// work-group is composed of a set of work-items (grouped in chunks of 64,
/// forming warps). Work-groups can run in any order.
class ThreadBlock
{
	// ID
	int id;
	int id_3d[3];

	// Name
	String name;

	// Status
	ThreadBlockState state;

	unsigned warps_at_barrier;
	unsigned warps_completed_emu;
	unsigned warps_completed_timing;
	bool finished_emu;
	bool finished_timing;

	// Grid that it belongs to
	Grid *grid;

	// SM it is mapped to
	struct frm_sm_t *sm;

	/* Double linked lists of thread_blocks */
	FrmThreadBlock *pending_list_prev;
	FrmThreadBlock *pending_list_next;
	FrmThreadBlock *running_list_prev;
	FrmThreadBlock *running_list_next;
	FrmThreadBlock *finished_list_prev;
	FrmThreadBlock *finished_list_next;

	/* Array of warps */
	int warp_count;
	std::vector<std::unique_ptr<Warp>> warps;

	/* List of warps */
	std::list<Warp *> running_warps;
	std::list<Warp *> finished_warps;

	/* Array of threads */
	int thread_count;
	std::vector<std::unique_ptr<Thread>> threads;

	/* Barrier information */
	unsigned int num_warps_at_barrier;

	/* Fields introduced for architectural simulation */
	int id_in_sm;
	int sm_finished_count;  /* like 'finished_list_count', but when warp reaches Complete stage */

	/* Shared memory */
	Memory::Memory shared_mem;

public:

	/// Constructor
	///
	/// \param grid Instance of class Grid that it belongs to.
	/// \param id Thread-block global 1D identifier
	ThreadBlock(Grid *grid);

	/// Dump thread-block in human readable format into output stream
	void Dump(std::ostream &os) const;

	/// Equivalent to ThreadBlock::Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const ThreadBlock &thread_block) {
		thread_block.Dump(os);
		return os;
	}


	/// Getters
	///
	/// Get threadblock ID
	int getId() const { return id; }

	/// Get counter of warps in threadblock
	int getWarpsInWorkgroup() const { return warps.size(); }

	/// Get counter of warps at barrier
	int getWarpsAtBarrier() const { return num_warps_at_barrier; }

	/// Get Grid that it belongs to
	const Grid *getGrid() const { return grid; }

	/// Get pointer of a workitem in this threadblock
	Thread *getThread(int id_in_thread_block) {
		assert(id_in_thread_block >= 0 && id_in_thread_block < (int)threads.size());
		return threads[id_in_thread_block].get();
	}

	/// Setters
	///
	/// Increase warps_at_barrier counter
	void incWarpsAtBarrier() { num_warps_at_barrier++; }

	/// Set warp_at_barrier counter
	void setWarpsAtBarrier(unsigned counter) { num_warps_at_barrier = counter; }

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
	
	/// Return a past-the-end iterator to the list of work-items
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
