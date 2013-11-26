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

#include <arch/fermi/asm/Arg.h>
#include <arch/fermi/asm/Binary.h>

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


/// ?
class Grid
{
	// Emulator
	Emu *emu;

	// ID
	int id;
	String name;

	// State
	GridState state;

	// CUDA function
	CUDAFunction *function;

	// Number of register used by each thread
	unsigned int num_gpr;

	// 3D work size counters
	int thread_count3[3];  // Total number of threads
	int thread_block_size3[3];  // Number of threads in a thread block
	int thread_block_count3[3];  // Number of thread blocks

	// 1D work size counters. Each counter is equal to the multiplication
	// of each component in the corresponding 3D counter
	int thread_count;
	int thread_block_size;

	// Array of thread blocks
	int thread_block_count;
	ThreadBlock **thread_blocks;

	// Lists of thread blocks
	std::list<std::unique_ptr<ThreadBlock>> pending_thread_blocks;
	std::list<std::unique_ptr<ThreadBlock>> running_thread_blocks;
	std::list<std::unique_ptr<ThreadBlock>> finished_thread_blocks;

	// List of Grid
        Grid *grid_list_prev;
        Grid *grid_list_next;
        Grid *pending_grid_list_prev;
        Grid *pending_grid_list_next;
        Grid *running_grid_list_prev;
        Grid *running_grid_list_next;
        Grid *finished_grid_list_prev;
        Grid *finished_grid_list_next;

	void *inst_buffer;
	unsigned int inst_buffer_size;

	// Local memory top to assign to local arguments.
	// Initially it is equal to the size of local variables in 
	// kernel function.
	unsigned local_mem_top;

public:

	/// Constructor
	Grid(Emu *emu, CUDAFunction *function);

	/// Dump the state of the Grid in a plain-text format into an output
	/// stream.
	void Dump(std::ostream &os) const;

	/// Short-hand notation for dumping ND-range.
	friend std::ostream &operator<<(std::ostream &os,
			const Grid &grid) {
		grid.Dump(os);
		return os;
	}

	/// Getters
	///
	/// Get local memory top address
	unsigned getLocalMemTop() const { return local_mem_top; }

	/// Setters
	///
	/// Set new size parameters of the Grid before it gets launched.
	///
	/// \param global_size Array of 3 elements
	///        representing the global size.
	/// \param local_size Array of 3 elements
	///        representing the local size.
	void SetupSize(unsigned *global_size, unsigned *local_size);

};

}  // namespace

#endif

