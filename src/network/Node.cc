/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
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

#include <cstring>

#include "Node.h"
#include "Buffer.h"

namespace net
{

Node::Node(Network *network,
		int index,
		int input_buffer_size,
		int output_buffer_size,
		const std::string &name,
		void *user_data) :
		network(network),
		name(name),
		index(index),
		input_buffer_size(input_buffer_size),
		output_buffer_size(output_buffer_size),
		user_data(user_data)
{
}


Buffer *Node::addInputBuffer(int size, Connection *connection)
{
	std::string name = misc::fmt("in_buf_%d",
				(unsigned int) input_buffers.size());
	input_buffers.emplace_back(misc::new_unique<Buffer>(
			name,
			size,
			input_buffers.size(),
			this,
			connection));
	return input_buffers.back().get();
}


Buffer *Node::addOutputBuffer(int size, Connection *connection)
{
	std::string name = misc::fmt("out_buf_%d",
			(unsigned int) output_buffers.size());
	output_buffers.emplace_back(misc::new_unique<Buffer>(
			name,
			size,
			output_buffers.size(),
			this,
			connection));
	return output_buffers.back().get();
}

}
