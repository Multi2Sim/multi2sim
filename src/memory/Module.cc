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

#include "Module.h"


namespace mem
{


Module::Module(const std::string &name,
		Type type,
		int num_ports,
		int block_size,
		int data_latency)
		:
		name(name),
		type(type),
		block_size(block_size),
		data_latency(data_latency),
		num_ports(num_ports)
{
	// Create ports
	ports.reserve(num_ports);
	for (int i = 0; i < num_ports; i++)
		ports.emplace_back(misc::new_unique<Port>());

	// Block size
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	log_block_size = misc::LogBase2(block_size);
}


}  // namespace mem

