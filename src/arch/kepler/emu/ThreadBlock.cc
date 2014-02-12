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


#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <mem-system/memory.h>

#include "Emu.h"
#include "Grid.h"
#include "ThreadBlock.h"
#include "Thread.h"
#include "Warp.h"


/*
 * Public Functions
 */
namespace Kepler
{

ThreadBlock::ThreadBlock(Grid *grid)
{
	Warp *warp;
	Thread *thread;
	int i;
	int warp_count;
	/* Initialization */
	this->id = id;
	this->grid = grid;

	/* Create warps */
	warp_count = (grid->thread_block_size + warp_size - 1) /
			warp_size;
	/*
	this->warps = (KplWarp **) xcalloc(this->warp_count, sizeof(KplWarp *));
	this->running_warps = list_create();
	this->finished_warps = list_create();
	for (i = 0; i < this->warp_count; ++i)
	{
		warp = new(KplWarp, i, this, grid);
		this->warps[i] = warp;
		list_add(this->running_warps, warp);
	}
*/
	/* Create threads */
	/*
	this->thread_count = grid->thread_block_size;
	this->threads = (KplThread **) xcalloc(this->thread_count,
			sizeof(KplThread *));
	for (i = 0; i < this->thread_count; ++i)
	{
		thread = new(KplThread, i, this->warps[i / kpl_emu_warp_size]);
		this->threads[i] = thread;
	}
*/
	/* Create shared memory */
	this->shared_mem = new Memory::Memory();
	this->shared_mem.safe = false;

	/* Flags */
	this->finished_emu = false;
}

void ThreadBlock::Dump(std::ostream &os = std::cout) const
{
}

}	//namespace
