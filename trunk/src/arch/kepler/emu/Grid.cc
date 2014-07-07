/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
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


#include <driver/cuda/function.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "Emu.h"
#include "Grid.h"
#include "ThreadBlock.h"
#include "Warp.h"
#include "Thread.h"

//#include "isa.h"

namespace Kepler
{

Grid::Grid(Function *function)
{
	/* Initialization */
	this->emu = emu->getInstance();
	id = emu->getGridSize();

	inst_buffer = function->getInstructionBuffer();
	inst_buffer_size = function->getInstructionBinarySize();
	gpr_count = function->getNumberofGPR();

	state = GridStateInvalid;

	/* Add to list */
	emu->GridsPushBack(this);
}

void Grid::Dump(std::ostream &os) const
{
}

void Grid::SetupSize(unsigned *thread_block_count,
		unsigned *thread_block_size)
{

	/* Setup thread-block counts */
	for (int i = 0; i < 3; i++)
		thread_block_count3[i] = thread_block_count[i];
	this->thread_block_count = thread_block_count3[0] *
			thread_block_count3[1] * thread_block_count3[2];

	/* Setup thread-block sizes */
	for (int i = 0; i < 3; i++)
		thread_block_size3[i] = thread_block_size[i];
	this->thread_block_size = thread_block_size3[0] *
			thread_block_size3[1] * thread_block_size3[2];

	/* Calculate thread counts */
	for (int i = 0; i < 3; i++)
		thread_count3[i] = thread_block_count[i] * thread_block_size[i];
	thread_count = thread_count3[0] * thread_count3[1] *
			thread_count3[2];

	/* Create lists */

	//this->pending_thread_blocks = new std::list<std::unique_ptr<ThreadBlock>>;
	for (unsigned i = 0; i < this->thread_block_count; ++i)
	{
		std::unique_ptr<ThreadBlock> tb;
		pending_thread_blocks.push_back(std::move(tb));
	}
	//this->running_thread_blocks = list_create();
	//this->finished_thread_blocks = list_create();

	/* Debug */
	/*kpl_isa_debug("%s:%d: block count = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_block_count3[0],
			this->thread_block_count3[1],
			this->thread_block_count3[2]);
	kpl_isa_debug("%s:%d: block size = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_block_size3[0],
			this->thread_block_size3[1],
			this->thread_block_size3[2]);
	kpl_isa_debug("%s:%d: grid size = (%d,%d,%d)\n",
			__FILE__, __LINE__, this->thread_count3[0],
			this->thread_count3[1], this->thread_count3[2]);
			*/
}

void Grid::GridSetupConstantMemory()
{
	unsigned v;

	emu->WriteConstMem(0x8, 3*sizeof(unsigned), (const char*)thread_block_size3);
	emu->WriteConstMem(0x14, 3*sizeof(unsigned), (const char*)thread_block_count3);
	v = 0x1000000;
	emu->WriteConstMem(0x100000, sizeof(unsigned), (const char*)&v);
}

void Grid::WaitingToRunning(int thread_block_id)
{
	running_thread_blocks.push_back
		(std::unique_ptr<ThreadBlock>(new ThreadBlock(this, thread_block_id)));
	pending_thread_blocks.pop_front();
}

void Grid::PushFinishedThreadBlock(std::unique_ptr<ThreadBlock> threadblock)
{
	finished_thread_blocks.push_back(std::move(threadblock));
}

void Grid::PopRunningThreadBlock()
{
	running_thread_blocks.pop_front();
}

}	//namespace
