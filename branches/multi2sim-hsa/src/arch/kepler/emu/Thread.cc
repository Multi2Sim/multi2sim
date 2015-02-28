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


#include "Emu.h"
#include "Grid.h"
#include "Thread.h"
#include "ThreadBlock.h"
#include "Warp.h"

namespace Kepler
{
/*
 * Public Functions
 */


Thread::Thread(Warp *warp, int id)
{
	// Initialization
	this->warp = warp;
	thread_block = warp->getThreadBlock();
	grid = thread_block->getGrid();
	this->id = id + thread_block->getId() * grid->getThreadBlockSize();
	id_in_warp = id % warp_size;
	id_in_thread_block = id;

	// Initialization instruction table
#define DEFINST(_name, _fmt_str, ...) \
		inst_func[INST_##_name] = &Thread::ExecuteInst_##_name;
#include <arch/kepler/asm/Inst.def>
#undef DEFINST

	// Initialize  general purpose registers
	for (int i = 0; i < 256; ++i)
		WriteGPR(i, 0);

	// Initialize CC register
	WriteCC(0);

	// Initialize special registers
	for (int i = 0; i < 82; ++i)
		WriteSR(i, 0);

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
	WriteSR(33, id % grid->getThreadBlockSize3(0));
	WriteSR(34, (id / grid->getThreadBlockSize3(0)) %
			grid->getThreadBlockSize3(1));
	WriteSR(35, id / (grid->getThreadBlockSize3(0) *
			grid->getThreadBlockSize3(1)));
	WriteSR(37, thread_block->getId() %
			grid->getThreadBlockCount3(0));
	WriteSR(38, (thread_block->getId() /
			grid->getThreadBlockCount3(0)) %
			grid->getThreadBlockCount3(1));
	WriteSR(39, thread_block->getId() /
			(grid->getThreadBlockCount3(0) *
					grid->getThreadBlockCount3(1)));

	// Initialize predicate registers
	for (int i = 0; i < 7; ++i)
		WritePred(i, 0);

	WritePred(7, 1);

	/* Add thread to warp */
	//warp->threads[this->id_in_warp] = this;
}

void Thread::Execute(InstOpcode opcode, Inst *inst)
{
	(this->*(inst_func[opcode]))(inst);
}


void Thread::ExecuteSpecial()
{
	ExecuteInst_Special();
}


void Thread::ISAUnimplemented(Inst *inst)
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

void Thread::ISAUnsupportedFeature(Inst *inst)
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
