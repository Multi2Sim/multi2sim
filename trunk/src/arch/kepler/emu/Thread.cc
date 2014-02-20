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
	/* Initialization */
	this->warp = warp;
	thread_block = warp->getThreadBlock();
	grid = thread_block->getGrid();
	this->id = id + thread_block->getId() * grid->getThreadBlockSize();
	id_in_warp = id % warp_size;

	/* General purpose registers */
	for (int i = 0; i < 64; ++i)
		gpr[i].u32 = 0;

	/* Special registers */
	for (int i = 0; i < 82; ++i)
		sr[i].u32 = 0;
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

	/* Predicate registers */
	for (int i = 0; i < 7; ++i)
		pr[i] = 0;
	pr[7] = 1;

	/* Add thread to warp */
	//warp->threads[this->id_in_warp] = this;
}

}	//namespace
