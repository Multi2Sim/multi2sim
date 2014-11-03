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
#include <memory/Memory.h>

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

ThreadBlock::ThreadBlock(Grid *grid, int id)
{

	//Warp *warp;
	//Thread *thread;
	int warp_count;
	int thread_count;

	// Initialization
	this->id = id;
	this->grid = grid;

	// Create warps
	warp_count = (grid->getThreadBlockSize() + warp_size - 1) /
			warp_size;
	for (int i = 0; i < warp_count; ++i)
		warps.push_back(std::unique_ptr<Warp>(new
				Warp(this, i)));

	// Create threads
	thread_count = grid->getThreadBlockSize();
	for (int i = 0; i < thread_count; ++i)
		threads.push_back(std::unique_ptr<Thread>(new
				Thread(warps[i / warp_size].get(), i)));

	/* Set warps' beginning thread and ending thread */
	for (int i = 0; i < warp_count - 1; ++i)
	{
		warps[i]->setThreadBegin(threads.begin() + warp_size * i);
		warps[i]->setThreadEnd(threads.begin() + warp_size * i + warp_size);
	}
	warps[warp_count - 1]->setThreadBegin
		(threads.begin() + warp_size * (warp_count - 1));
	warps[warp_count - 1]->setThreadEnd
		(threads.end());

	/* Create shared memory */
	//shared_mem = new Memory::Memory();
	shared_mem.setSafe(false);

	/* Flags */
	finished_emu = false;
	num_warps_completed_emu = 0;
	num_warps_at_barrier = 0;

	finished_timing = 0;
	num_warps_completed_timing = 0;
}

unsigned ThreadBlock::getWarpCount() const
{
    return (grid->getThreadBlockSize() + warp_size - 1) /
    		warp_size;
}


void ThreadBlock::Dump(std::ostream &os) const
{
	os<<"ID:	"<<id<<std::endl;
	//os<<"the first of grid inst buffer" <<*grid->getInstBuffer()<<std::endl;
	os<<"the buffer size of grid is"<<grid->getInstBufferSize()<<std::endl;
}


void ThreadBlock::readSharedMem(unsigned address, unsigned length, char* buffer)
{
	shared_mem.Read(address, length, buffer);
}


void ThreadBlock::writeSharedMem(unsigned address, unsigned length, char* buffer)
{
	shared_mem.Write(address, length, buffer);
}


void ThreadBlock::clearWarpAtBarrier()
{
	for (auto p = WarpsBegin(); p != WarpsEnd(); ++p)
	{
		p->get()->setAtBarrier(false);
	}
}

}	//namespace
