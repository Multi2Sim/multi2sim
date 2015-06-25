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

#include "Gpu.h"


namespace SI
{

int Gpu::num_compute_units = 32;


Gpu::Gpu()
{
	// Create compute units
	compute_units.reserve(num_compute_units);
	for (int i = 0; i < num_compute_units; i++)
		compute_units.emplace_back(misc::new_unique<ComputeUnit>(i));
}


void Gpu::Run()
{
	// Advance one cycle in each compute unit
	for (auto &compute_unit : compute_units)
		compute_unit->Run();
}

}

