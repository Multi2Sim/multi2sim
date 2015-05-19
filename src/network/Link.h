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

#include "Connection.h"
namespace net
{

class Node;
class Buffer;

class Link : public Connection
{
protected: 
	// Link source node
	Node *source_node;

	// Link source buffer
	Buffer *source_buffer;

	// Link destination node
	Node *destination_node;

	// Link destination buffer
	Buffer *destination_buffer;

	// Descriptive Name. Name is shared between two links in case of
	// Bidirectional link.
	std::string descriptive_name;

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

	// Number of bytes that was transfered through the links
	long long transferred_bytes;

public:

	~Link() {};

	/// Set descriptive Name
	void setDescriptiveName(const std::string &name) {
		this->descriptive_name = name;
	}

	/// Set virtual channels
	void setVirtualChannels(const int vc) {this->virtual_channels = vc;}

	/// Set source node
	void setSourceNode(Node *node) { this->source_node = node; }

	/// Get source node
	Node *getSourceNode() const { return source_node; }

	/// Set destination node
	void setDestinationNode(Node *node) { this->destination_node = node; }

	/// Get destination node
	Node *getDestinationNode() const { return destination_node; }

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Link &link)
	{
		link.Dump(os);
		return os;
	}

	void Dump(std::ostream &os) const;


};


}  // namespace net

#endif

