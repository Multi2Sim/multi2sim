/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <arch/kepler/emulator/Warp.h>
#include <arch/kepler/emulator/ThreadBlock.h>

#include "SM.h"
#include "Uop.h"
#include "WarpPool.h"


namespace Kepler
{

long long Uop::id_counter = 0;

Uop::Uop(FetchBuffer *fetch_buffer, Warp *warp, WarpPoolEntry *warp_pool_entry,
		long long cycle_created, ThreadBlock *block, int warp_pool_id,
		int instruction_pc) :
				fetch_buffer(fetch_buffer),
				warp(warp),
				warp_pool_entry(warp_pool_entry),
				block(block),
				warp_pool_id(warp_pool_id),
				instruction_pc(instruction_pc)
{
	// Assign unique identifier
	id = ++id_counter;
	id_in_warp = warp->getUopId();
	sm = warp_pool_entry->getWarpPool()->getSM();
	id_in_sm = sm->getUopId();

	// Allocate room for the thread info structures
	thread_info_list.resize(Kepler::warp_size);

	for (int i = 0; i < 4; i++)
	{
		source_register_index[i] = -1;
		destination_register_index[i] = -1;
		source_predicate_index[i] = -1;
	}

	for (int i = 0; i < 2; i++)
		destination_predicate_index[i] = -1;
}

} // namespace Kepler
