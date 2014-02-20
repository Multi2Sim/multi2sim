/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_EMU_GRID_H
#define ARCH_KEPLER_EMU_GRID_H

#ifdef __cplusplus
#include <iostream>
#include <list>
#include <memory>
#include <vector>
#include <mem-system/Memory.h>

#include "Emu.h"
#include "ThreadBlock.h"

namespace Kepler
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
	/// Get ID
	int getID() const { return id; }

	/// Get assambler
	Asm *getAsm() const { return emu->getAsm(); }

	/// Get shared memory top address
	unsigned getSharedMemTop() const { return shared_mem_top; }

	/// Get 1D thread block size
	unsigned getThreadBlockSize() const { return thread_block_size; }

	/// Get 3D thread block size
	/// \param index range from 0 to 2
	unsigned getThreadBlockSize3(int index) const
	{
		//assert(index < 3);
		return thread_block_size3[index];
	}

	/// Get 3D thread block count
	/// \param index range from 0 to 2
	unsigned getThreadBlockCount3(int index) const
	{
		//assert(index < 3);
		return thread_block_count3[index];
	}

	///get instruction function
	InstFunc getInstFunc(InstOpcode inst) { return emu->getInstFunc(inst); }

	/// Get instruction buffer
	long long unsigned *getInstBuffer() const { return (long long unsigned *)inst_buffer; }

	/// Get instruction buffer size
	unsigned getInstBufferSize() const { return inst_buffer_size; }

	/// Get pending_thread_blocks size
	unsigned getPendThreadBlocksize() const { return pending_thread_blocks.size();}

	/// Get running_thread_blocks size
	unsigned getRunThreadBlocksize() const { return running_thread_blocks.size();}

	/// Get running thread blocks list begin
	std::list<std::unique_ptr<ThreadBlock>>::iterator getRunningThreadBlocksBegin()
	{
		return running_thread_blocks.begin();
	}
	// Setters
	//

	/// Set new sizes of a grid before it is launched.
	///
	/// \param global_size Array of 3 elements
	///        representing the global size.
	/// \param local_size Array of 3 elements
	///        representing the local size.
	void SetupSize(unsigned *global_size, unsigned *local_size);

	/// Write initial values into constant memory. Used by driver.
	void GridSetupConstantMemory();

	/// pop an element from pending thread block list,
	/// and create a new thread block pushing into running thread block list
	void WaitingToRunning(int thread_block_id);

	/// push a thread block into finished thread block list
	void PushFinishedThreadBlock(std::unique_ptr<ThreadBlock> threadblock);

};

}   //namespace
#endif
#endif

