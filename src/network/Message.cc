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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>

#include "Packet.h"
#include "Message.h"

namespace net
{

Message::Message(long long id, Node *source_node, Node *destination_node, int size) :
		id(id),
		source_node(source_node),
		destination_node(destination_node),
		size(size)
{
}


void Message::Packetize(int packet_size)
{
	int packet_count = (size - 1) / packet_size + 1;

	for (int i = 0; i < packet_count; i++)
	{
		auto packet = misc::new_unique<Packet>(this, packet_size);
		this->packets.push_back(std::move(packet));
	}
}

void Message::Send()
{
	throw misc::Panic(misc::fmt("Function %s not implemented.",
			__FUNCTION__));
}

}  // namespace net
