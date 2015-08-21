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

#include <arch/southern-islands/emulator/Wavefront.h>
#include <arch/southern-islands/emulator/WorkGroup.h>

#include "ComputeUnit.h"
#include "Uop.h"
#include "WavefrontPool.h"


namespace SI
{

long long Uop::id_counter = 0;


Uop::Uop(Wavefront *wavefront, WavefrontPoolEntry *wavefront_pool_entry,
		long long cycle_created,
		WorkGroup *work_group,
		int wavefront_pool_id) :
		wavefront(wavefront),
		wavefront_pool_entry(wavefront_pool_entry),
		work_group(work_group),
		wavefront_pool_id(wavefront_pool_id)
{
	// Assign unique identifier
	id = ++id_counter;
	id_in_wavefront = wavefront->getUopId();
	compute_unit = wavefront_pool_entry->getWavefrontPool()->getComputeUnit();
	id_in_compute_unit = compute_unit->getUopId();
	
	// Allocate room for the work-item info structures
	work_item_info_list.resize(WorkGroup::WavefrontSize);
}

}

