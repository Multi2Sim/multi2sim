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

#include "Emu.h"

namespace Kepler
{

class KplEmu;

class Grid
{
	/* Emulator */
	KplEmu *emu;

	/* ID */
	int id;

	/* Number of general purpose registers used by a thread */
	unsigned int num_gpr;

	/* Lists of thread blocks */
	struct list_t *pending_thread_blocks;
	struct list_t *running_thread_blocks;
	struct list_t *finished_thread_blocks;

	/* Sizes */
	int thread_count3[3];
	int thread_count;
	int thread_block_size3[3];
	int thread_block_size;
	int thread_block_count3[3];
	int thread_block_count;

	/* Call-back function run right before freeing grid, using the value in
	 * 'free_notify_data' as an argument. */
	void (*free_notify_func)(void *);
	void *free_notify_data;

	/* The following fields are used by architectural simulation only. */

	unsigned shared_mem_top;

public:

	///Constructor
	Grid(KplEmu *emu);

	/// Dump grid in a human-readable fashion into an output stream (or
	/// standard output if argument \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Operator \c << overloaded, invoking function Dump()
	friend std::ostream &operator<<(std::ostream &os, const Grid &grid) {
		grid.Dump(os);
		return os;
	}

	/// Setup sizes of a grid. Used by driver.
	void SetupSize(unsigned *global_size, unsigned *local_size);

	/// Write initial values into constant memory. Used by driver.
	void SetupConstantMemory();
};
}   //namespace
#endif

