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

#ifndef NETWORK_LINK_H
#define NETWORK_LINK_H

#include <lib/cpp/String.h>

namespace net
{

class Network;
class Node;
class Buffer;

class Link
{
protected: 

	// Network associated with the link
	Network *network;

	// Link source node
	Node *source_node;

	// Link source buffer
	Buffer *source_buffer;

	// Link destination node
	Node *destination_node;

	// Link destination buffer
	Buffer *destination_buffer;

	// Link bandwidth
	int bandwidth;

	// Node Name
	std::string name;

	// Number of virtual channels on link
	int virtual_channels;




	//
	// Scheduling and arbitration
	//

	// Link busy cycle for event scheduling
	long long busy;

	// Last cycle a buffer was assigned to a link in virtual channel
	// arbitration
	long long scheduled_when;

	// Last buffer that was has the ownership of physical link in virtual
	// channel arbitration
	Buffer *scheduled_buffer;




	//
	// Statistics
	//

	// Number of cycles that the link was busy
	long long busy_cycles;

	// Number of bytes that was transfered through the links
	long long transferred_bytes;

public:

	/// Virtual destructor
	virtual ~Link() {};

	/// Set name
	void setName(const std::string &name) { this->name = name; }

	/// Get the name of the link
	std::string getName() const { return name; }

	/// Set source node
	void setSourceNode(Node *node) { this->source_node = node; }

	/// Get source node
	Node *getSourceNode() const { return source_node; }

	/// Set destination node
	void setDestinationNode(Node *node) { this->destination_node = node; }

	/// Get destination node
	Node *getDestinationNode() const { return destination_node; }

	/// Dump link information
	virtual void Dump(std::ostream &os) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Link &link)
	{
		link.Dump(os);
		return os;
	}


};


}  // namespace net

#endif

