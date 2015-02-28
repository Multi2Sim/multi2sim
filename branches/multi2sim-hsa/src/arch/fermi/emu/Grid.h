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

#ifndef ARCH_FERMI_EMU_GRID_H
#define ARCH_FERMI_EMU_GRID_H

#include <list>
#include <memory>

#include "Emu.h"


namespace Frm
{

class Emu;
class ThreadBlock;

enum GridState
{
	GridStateInvalid = 0,
	GridPending = 0x1,
    GridRunning = 0x2,
    GridFinished = 0x4
};

class Grid
{
	// Emulator
	Emu *emu;

	// ID
	int id;

	// Name
	std::string name;

	// State
	GridState state;

	// 3D counters
	unsigned thread_count3[3];
	unsigned thread_block_size3[3];
	unsigned thread_block_count3[3];

	// 1D counters. Each counter is equal to the multiplication
	// of each component in the corresponding 3D counter
	unsigned thread_count;
	unsigned thread_block_size;
	unsigned thread_block_count;

	// GPR usage by a thread
	unsigned gpr_count;

	// Thread-blocks
	std::list<std::unique_ptr<ThreadBlock>> pending_thread_blocks;
	std::list<std::unique_ptr<ThreadBlock>> running_thread_blocks;
	std::list<std::unique_ptr<ThreadBlock>> finished_thread_blocks;

	// Iterators
	std::list<Grid *>::iterator grid_list_iter;
	std::list<Grid *>::iterator pending_grid_list_iter;
	std::list<Grid *>::iterator running_grid_list_iter;
	std::list<Grid *>::iterator finished_grid_list_iter;

	// Instruction buffer
	void *inst_buffer;
	unsigned inst_buffer_size;

	// Shared memory top pointer
	unsigned shared_mem_top;

public:
	/// Constructor
	Grid(Emu *emu);

	/// Dump the state of the grid in a plain-text format into an output
	/// stream.
	void Dump(std::ostream &os = std::cout) const;

	/// Short-hand notation for dumping grid.
	friend std::ostream &operator<<(std::ostream &os,
			const Grid &grid) {
		grid.Dump(os);
		return os;
	}

	// Getters
	//
	// Get shared memory top address
	unsigned getSharedMemTop() const { return shared_mem_top; }

	// Setters
	//

	/// Set new sizes of a grid before it is launched.
	///
	/// \param global_size Array of 3 elements
	///        representing the global size.
	/// \param local_size Array of 3 elements
	///        representing the local size.
	void SetupSize(unsigned *global_size, unsigned *local_size);
};

}  // namespace

#endif

