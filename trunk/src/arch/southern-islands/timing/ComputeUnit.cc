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

#include "ComputeUnit.h"


namespace SI
{

int ComputeUnit::num_wavefront_pools = 4;


ComputeUnit::ComputeUnit(int index) :
		index(index),
		scalar_unit(this),
		branch_unit(this),
		lds_unit(this),
		vector_memory_unit(this)
{
	// Create wavefront pools, and SIMD units
	wavefront_pools.reserve(num_wavefront_pools);
	fetch_buffers.reserve(num_wavefront_pools);
	simd_units.reserve(num_wavefront_pools);
	for (int i = 0; i < num_wavefront_pools; i++)
	{
		wavefront_pools[i] = misc::new_unique<WavefrontPool>(this);
		fetch_buffers[i] = misc::new_unique<FetchBuffer>(this);
		simd_units[i] = misc::new_unique<SimdUnit>(this);
	}
}


void ComputeUnit::Run()
{
}

}

