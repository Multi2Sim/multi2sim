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

#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H

namespace net
{
class Message;
class Node;
class Buffer;

class Packet
{
	// Message that this packet is part of
	Message *message;

	// Size of the packet
	int size;

	// The index of the packet in a message
	int session_id;

	// In transit until cycle
	long long busy;

	// Current position in the network, which node it is at
	Node *node;

	// Current position in the network, which buffer it is at
	Buffer *buffer;


public:

	/// Constructor
	Packet(Message *message, int size);

	/// Set session id
	void setSessionId(int session_id) { this->session_id = session_id; }

	/// Get message
	Message *getMessage() const { return message; }

	/// Get size
	int getSize() const { return size; }

	/// Update the node that the packet is at
	void setNode(Node *node) { this->node = node; }

	/// Update the buffer that the packet is at
	void setBuffer(Buffer *buffer) { this->buffer = buffer; }

	/// Update the cycle until which the packet is in transit
	void setBusy(long long busy) { this->busy = busy; }

};

}  // namespace net

#endif
