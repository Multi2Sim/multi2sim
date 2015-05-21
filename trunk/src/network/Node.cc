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

Buffer* Node::AddInputBuffer(int size)
{
	auto buffer = misc::new_unique<Buffer> ();
	std::string name = misc::fmt("in_buf_%d",
			(unsigned int) input_buffers.size());
	buffer->setIndex(input_buffers.size());
	buffer->setName(name);
	buffer->setSize(size);
	buffer->setNode(this);

	Buffer* return_buffer = buffer.get();
	input_buffers.emplace_back(std::move(buffer));

	return return_buffer;
}

Buffer* Node::AddOutputBuffer(int size)
{
	auto buffer = misc::new_unique<Buffer> ();
	std::string name = misc::fmt("out_buf_%d",
			(unsigned int) output_buffers.size());
	buffer->setIndex(output_buffers.size());
	buffer->setName(name);
	buffer->setSize(size);
	buffer->setNode(this);

	Buffer* return_buffer = buffer.get();
	output_buffers.push_back(std::move(buffer));

	return return_buffer;

}


}
