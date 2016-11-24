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

#ifndef NETWORK_SWITCH_H
#define NETWORK_SWITCH_H

#include "Node.h"

namespace net
{

// A switch is a node that passes packets to next link
class Switch : public Node
{
	// Bandwidth of the switch
	int bandwidth;

public:

	/// Constructor
	Switch(Network *network,
			int index,
			int input_buffer_size,
			int output_buffer_size,
			int bandwidth,
			const std::string &name,
			void *user_data) :
			Node(network,
					index,
					input_buffer_size,
					output_buffer_size,
					name,
					user_data),
			bandwidth(bandwidth)
	{};

	/// Dump node information
	void Dump(std::ostream &os) const;

	/// Forward the packet to next hop
	/// 
	/// This function would at first assert the packet is in an input 
	/// buffer of this switch. If so, this function would lookup routing 
	/// table for next hop and the corresponding output buffer for next
	/// hop node. If the output buffer is busy, current event will be 
	/// rescheduled when the output buffer is free. If the output buffer
	/// is full, the output_buffer event will be suspended in the buffer
	/// event queue. Moreover, if the switch's scheduler is not scheduled
	/// for the target output buffer, the event will be rescheduled next 
	/// cycle. Finally, if the switch can forward the packet, an 
	/// output_buffer event will be scheduled after a certain amount of 
	/// latency, which is specified in the node class.
	///
	/// This function is designed to be called from the input buffer
	/// event handler and the input buffer event handler has to check 
	/// if the packets has arrived its destination node before calling
	/// this function.
	///
	/// \param packet
	///	The packet to be forward to next hop. The packet has to 
	/// 	be in an input buffer of current switch.
	///
	void Forward(Packet *packet);

	/// Implements the arbiter in the switch. This function takes in 
	/// an output buffer and returns the input buffer that is authorized
	/// to send packet to that output buffer. This function would first
	/// check if the output buffer is already has a linked input buffer
	/// for current cycle. If yes, the linked input buffer will be 
	/// returned. Otherwise, a new decision will be made. New decision is
	/// made in a round-robin fashion. From the input buffer that is 
	/// linked in previous cycle, the arbiter finds the next input buffer 
	/// whose first packet need to be forward to this particular 
	/// output buffer. The found input buffer is returned. The decision
	/// will also be written into the output buffer's corresponding 
	/// fields. If there is no input buffer that can send a packet, 
	/// a nullptr will be returned.
	///
	/// Requirement: 
	/// 	1. This function needs to be called within an event handler. 
	///	2. User of this function has to guarantee that the output
	///	buffer is not in write busy.
	/// 	3. User of this function has to guarantee that at least one
	///	input buffer, which is not in read busy, has a packet in front
	/// 	need to be forwarded to the output buffer.
	/// 
	/// \param output_buffer
	/// 	The output buffer need to be scheduled
	/// 	
	/// \return
	/// 	The input buffer that is wired with the output buffer
	///
	Buffer *Schedule(Buffer *output_buffer);
};

}

#endif
