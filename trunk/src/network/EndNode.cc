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

#include "EndNode.h"

namespace net
{

EndNode::EndNode(Network *network,
		int index,
		int input_buffer_size,
		int output_buffer_size,
		const std::string &name,
		void *user_data) :
		Node(network,
				index,
				input_buffer_size,
				output_buffer_size,
				name,
				user_data)
{}


void EndNode::Dump(std::ostream &os) const
{
	// Dumping the end-node name
	os << misc::fmt("[ Network.%s.Node.%s ]\n", network->getName().c_str(),
			getName().c_str());

	// Getting the current cycle
	long long cycle = System::getInstance()->getCycle();

	// Dumping the statistics information
	os << misc::fmt("SentBytes = %lld\n", sent_bytes);
	os << misc::fmt("SentPackets = %lld\n", sent_packets);
	os << misc::fmt("SendRate = %0.4f\n", cycle ?
			(double) sent_bytes / cycle : 0.0 );
	os << misc::fmt("ReceivedBytes = %lld\n", received_bytes);
	os << misc::fmt("ReceivedPackets = %lld\n", received_packets);
	os << misc::fmt("ReceiveRate = %0.4f\n", cycle ?
			(double) received_bytes / cycle : 0.0 );

	// Dumping input buffers' information
	for (auto &buffer : input_buffers)
		buffer->Dump(os);

	//Dumping output buffers' information
	for (auto &buffer : output_buffers)
		buffer->Dump(os);

	// Creating an empty line in dump
	os << "\n";

}

}
