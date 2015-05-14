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
	int id;

	// Size of the message
	int size;

public:

	//
	// Setters and getter
	//
	
	/// Set source node
	void setSourceNode(Node *source_node) 
	{ 
		this->source_node = source_node; 
	}

	/// Get source node
	Node *getSourceNode() const { return source_node; }

	/// Set destination node
	void setDestinationNode(Node *destination_node) 
	{ 
		this->destination_node = destination_node; 
	}

	/// Get destination node
	Node *getDestinationNode() const { return destination_node; }

	/// Set message size
	void setSize(int size) { this->size = size; }

	/// Get message size
	int getSize() const { return size; }

}

}  // namespace net

#endif
