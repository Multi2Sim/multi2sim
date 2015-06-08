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

#include "Node.h"
#include "Link.h"

namespace net
{

Link::Link(Network *network,
		const std::string &name,
		Node *src_node,
		Node *dst_node,
		int bandwidth,
		int source_buffer_size,
		int destination_buffer_size,
		int num_virtual_channels) :
		Connection(name, network, bandwidth),
		source_node(src_node),
		destination_node(dst_node),
		num_virtual_channels(num_virtual_channels)
{
	// Create and add source buffer
	Buffer *source_buffer = src_node->addOutputBuffer(source_buffer_size, 
			this);
	addSourceBuffer(source_buffer);

	// Create and add destination buffer
	Buffer *destination_buffer = dst_node->addInputBuffer(
			destination_buffer_size, this);
	addDestinationBuffer(destination_buffer);
}

void Link::Dump(std::ostream &os) const
{
	os << misc::fmt("\n***** Link %s *****\n", name.c_str());
	
	// Dump sources
	for (auto buffer : source_buffers)
	{
		os << misc::fmt("%s \t->", 
				buffer->getNode()->getName().c_str());
	}

	// Dump destinations
	for (auto buffer : destination_buffers)
	{
		os << misc::fmt(" \t %s", 
				buffer->getNode()->getName().c_str());
	}
}

}