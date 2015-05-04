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

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

namespace net 
{

class Message
{

	// Source node
	Node *source_node;

	// Destination node
	Node *destination_node;

	// Id of the message
	unsigned long long id;

	// Size of the message
	unsigned int size;

public:

	/// Constructor
	Message() {};

	/// Destructor
	~Message() {};

	/// Create message
	static Message *CreateMessage(Node *source, Node *destination, 
			unsigned int size);

	/// Set source node
	void setSource(Node *source) { source_node = source; }

	/// Set destination node
	void setSource(Node *destination) { destination_node = destination; }

}

}  // namespace net

#endif
