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

#include <cassert>

#include "FetchBuffer.h"


namespace SI
{

int FetchBuffer::fetch_latency = 1;
int FetchBuffer::fetch_width = 1;
int FetchBuffer::fetch_buffer_size = 10;
int FetchBuffer::issue_latency = 1;
int FetchBuffer::issue_width = 5;
int FetchBuffer::max_instructions_issued_per_type = 1;
	
	
std::shared_ptr<Uop> FetchBuffer::Remove(std::list<std::shared_ptr<Uop>>::iterator it)
{
	assert(it != buffer.end());
	std::shared_ptr<Uop> uop = *it;
	buffer.erase(it);
	return uop;
}

}

