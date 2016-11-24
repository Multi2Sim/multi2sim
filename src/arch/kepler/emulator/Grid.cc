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

#include "Emulator.h"
#include "Grid.h"
#include "ThreadBlock.h"
#include "Warp.h"
#include "Thread.h"


namespace Kepler
{

Grid::Grid(Function *function)
{
	// Initialization
	this->emulator = emulator->getInstance();
	id = emulator->getGridSize();
	const char * temp_buffer;
	temp_buffer = function->getTextBuffer();
	instruction_buffer_size = function->getTextSize();
	instruction_buffer.resize(instruction_buffer_size);
	kernel_function_name = function->getName();

	// Instruction byte
	unsigned long long inst_byte;
	for (int i = 0; i < instruction_buffer_size; i++)
	{
		inst_byte = (unsigned char) temp_buffer[i];
		if(i % 8 < 4) // 0 1 2 3 byte
		{
			instruction_buffer[i / 8] |= inst_byte << (i * 8 + 32);
		}
		else // 4 5 6 7 byte
		{
			instruction_buffer[i / 8] |= inst_byte << (i * 8 - 32);
		}
	}
	state = GridStateInvalid;

	//for(int i = 0; i < inst_buffer_size / 8; i++)
	//	std::cout<<"in function	"<<__FUNCTION__<<
	//	"the inst_buffer["<<i <<"] is"<<inst_buffer[i]<<std::endl;
	// Add to list  (no need? )
	//emu->addGrid(function);
}

void Grid::Dump(std::ostream &os) const
{
}

void Grid::SetupSize(unsigned *thread_block_count,
		unsigned *thread_block_size)
{

	// Setup thread-block counts
	for (int i = 0; i < 3; i++)
		thread_block_count3[i] = thread_block_count[i];
	this->thread_block_count = thread_block_count3[0] *
			thread_block_count3[1] * thread_block_count3[2];

	// Setup thread-block sizes
	for (int i = 0; i < 3; i++)
		thread_block_size3[i] = thread_block_size[i];
	this->thread_block_size = thread_block_size3[0] *
			thread_block_size3[1] * thread_block_size3[2];

	// Calculate thread counts
	for (int i = 0; i < 3; i++)
		thread_count3[i] = thread_block_count[i] * thread_block_size[i];
	thread_count = thread_count3[0] * thread_count3[1] *
			thread_count3[2];

	// Create lists

	//this->pending_thread_blocks = new std::list<std::unique_ptr<ThreadBlock>>;
	for (unsigned i = 0; i < this->thread_block_count; ++i)
	{
		std::unique_ptr<ThreadBlock> tb;
		pending_thread_blocks.push_back(std::move(tb));
	}
}


void Grid::GridSetupConstantMemory()
{
	emulator->WriteConstantMemory(0x28, 3*sizeof(unsigned),
					(const char *) thread_block_size3);
	emulator->WriteConstantMemory(0x34, 3*sizeof(unsigned),
					(const char *) thread_block_count3);
	unsigned v = 0x1000000;
	emulator->WriteConstantMemory(0x100000, sizeof(unsigned),
					(const char *) &v);
}

void Grid::WaitingToRunning(int thread_block_id, unsigned *id_3d)
{
	running_thread_blocks.push_back
		(std::unique_ptr<ThreadBlock>(new ThreadBlock(this, thread_block_id,
					id_3d)));
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
