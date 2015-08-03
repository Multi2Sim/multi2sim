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

#include "Emulator.h"
#include "Grid.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"


namespace Kepler
{

Thread::Thread(Warp *warp, int id)
{
	// Initialization
	this->emulator = emulator->getInstance();
	this->warp = warp;
	thread_block = warp->getThreadBlock();
	grid = thread_block->getGrid();
	this->id = id + thread_block->getId() * grid->getThreadBlockSize();
	id_in_warp = id % warp_size;
	id_in_thread_block = id;

	// Local Memory Initialization
	local_memory = misc::new_unique<mem::Memory>();
	local_memory->setSafe(false);
	local_memory_size = 1 << 20; // current 1MB for local memory
	local_memory_top_address = 0;
	local_memory_top_generic_address = local_memory_top_address + id *
			local_memory_size +	emulator->getGlobalMemoryTotalSize() +
			emulator->getSharedMemoryTotalSize();

	// local mem top generic address record in const mem c[0x0][0x24]
	emulator->WriteConstantMemory(0x24, sizeof(unsigned),
			(const char *) &local_memory_top_generic_address);

	// Initialization instruction table
#define DEFINST(_name, _fmt_str, ...) \
		inst_func[Instruction::INST_##_name] = &Thread::ExecuteInst_##_name;
#include "../disassembler/Instruction.def"
#undef DEFINST

	// Initialize  general purpose registers
	for (int i = 0; i < 256; ++i)
		WriteGPR(i, 0);

	// Initialize CC register
	this->WriteCC_CF(0);
	this->WriteCC_OF(0);
	this->WriteCC_SF(0);
	this->WriteCC_ZF(0);

	// Initialize special registers
	for (int i = 0; i < 82; ++i)
		WriteSpecialRegister(i, 0);

	/*
	sr[33].u32 = id % grid->getThreadBlockSize3(0);
	sr[34].u32 = (id / grid->getThreadBlockSize3(0)) %
			grid->getThreadBlockSize3(1);
	sr[35].u32 = id / (grid->getThreadBlockSize3(0) *
			grid->getThreadBlockSize3(1));
	sr[37].u32 = thread_block->getId() %
			grid->getThreadBlockCount3(0);
	sr[38].u32 = (thread_block->getId() /
			grid->getThreadBlockCount3(0)) %
			grid->getThreadBlockCount3(1);
	sr[39].u32 = thread_block->getId() /
			(grid->getThreadBlockCount3(0) *
					grid->getThreadBlockCount3(1));

	*/
	WriteSpecialRegister(33, id % grid->getThreadBlockSize3(0));
	WriteSpecialRegister(34, (id / grid->getThreadBlockSize3(0)) %
			grid->getThreadBlockSize3(1));
	WriteSpecialRegister(35, id / (grid->getThreadBlockSize3(0) *
			grid->getThreadBlockSize3(1)));
	WriteSpecialRegister(37, thread_block->getId() %
			grid->getThreadBlockCount3(0));
	WriteSpecialRegister(38, (thread_block->getId() /
			grid->getThreadBlockCount3(0)) %
			grid->getThreadBlockCount3(1));
	WriteSpecialRegister(39, thread_block->getId() /
			(grid->getThreadBlockCount3(0) *
					grid->getThreadBlockCount3(1)));

	// Currently Set LMEMHIOFF to 0
	WriteSpecialRegister(55, 0);

	// Virual Thread Lane ID
	WriteSpecialRegister(0, id_in_warp);

	// Initialize predicate registers
	for (int i = 0; i < 7; ++i)
		WritePredicate(i, 0);
	WritePredicate(7, 1);

	// Add thread to warp
	//warp->threads[this->id_in_warp] = this;
}


void Thread::Execute(Instruction::Opcode opcode, Instruction *inst)
{
	(this->*(inst_func[opcode]))(inst);
}


void Thread::ExecuteSpecial()
{
	ExecuteInst_Special();
}


void Thread::ISAUnimplemented(Instruction *inst)
{
	throw misc::Panic(misc::fmt("%s: Unimplemented Kepler "
			"instruction\n"
			"\n\t"
			"The NVIDIA Kepler instruction set is partially "
			"supported by Multi2Sim. If your program is using an "
			"unimplemented instruction, please report a bug on "
			"www.multi2sim.org requesting support for it.",
			inst->getName()));
}


void Thread::ISAUnsupportedFeature(Instruction *inst)
{
	throw misc::Panic(misc::fmt("%s: Unsupported Kepler "
			"instruction feature\n"
			"\n\t"
			"The NVIDIA Kepler instruction set is partially "
			"supported by Multi2Sim. If your program is using an "
			"unsupported instruction feature, please report a bug on "
			"www.multi2sim.org requesting support for it.",
			inst->getName()));
}

}	//namespace
