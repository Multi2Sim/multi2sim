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

#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include "System.h"
#include "Network.h"

#include <lib/cpp/IniFile.h>

namespace net
{

class Buffer;
class Bus;
class Network;

// A node in a interconnect network is where the packet is generated, 
// forwarded and consumed
class Node
{
protected:

	// Network that it belongs to
	Network *network;

	// Node Name
	std::string name;

	// Node index
	int id;

	// Input/Output Buffer List
	std::vector<std::unique_ptr<Buffer>> input_buffers;
	std::vector<std::unique_ptr<Buffer>> output_buffers;

	/*
	// Bus and photonic nodes
	std::vector<std::unique_ptr<Bus>> bus_lanes;

	// For Bus and Photonic:
	// List of source and destination buffer since Buses don't have input
	// and output buffers of their own.
	std::vector<std::unique_ptr<Buffer>> src_buffers;
	std::vector<std::unique_ptr<Buffer>> dst_buffers;

	// For Bus and Photonic Scheduling:
	// index of the last node that has been scheduled to send a packet
	// or message on the bus.
	int last_node_index;

	// For Bus and Photonic Scheduling:
	// index of the last bus (in case there are multiple lanes) that
	// has been assigned to a node to transmit data on.
	int last_bus_index;
	*/

public:

	/// Get name
	virtual std::string getName() const { return name; }

	/// Set name
	void setName(const std::string &name) { this->name = name; }

	/// Get the type of the node
	virtual std::string getType() const = 0;

	/// Get the id of the node
	virtual int getID() const {return id;}

	/// Dump the node information. 
	virtual void Dump(std::ostream &os) const = 0;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Node &node)
	{
		node.Dump(os);
		return os;
	}

	/// Adding input buffer
	Buffer* AddInputBuffer(int size);

	/// Adding output buffer
	Buffer* AddOutputBuffer(int size);

	/// Finding a Node's buffer by its name. Function looks up first input
	/// buffers and if the buffer is not found, it looks up output buffers
	/// to find the buffer.
	///
	/// \param buffer_name
	///	name of the buffer.
	//Buffer *getBufferByName(const std::string &buffer_name);
};


}  // namespace net

#endif

