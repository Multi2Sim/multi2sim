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

namespace net
{

class Buffer;
class Bus;

enum NodeType
{
	NodeTypeInvalid = 0,
	NodeTypeEndNode,
	NodeTypeSwitch,
	NodeTypeBus,
	NodeTypePhotonic
};


class Node
{

	//
	// Class Members
	//

	// Network that it belongs to
	Network *network;

	// Node Name
	std::string name;

	// Node Type
	enum NodeType kind;

	// Node index
	int id;

	// User Data. This is information required for memory system.
	void *user_data;

	// Node Bandwidth. Required for Bus and Switch
	int bandwidth;

	// Input/Output Buffer List
	std::vector<std::unique_ptr<Buffer>> input_buffer_list;
	std::vector<std::unique_ptr<Buffer>> output_buffer_list;

	// Bus and Photonic Nodes
	std::vector<std::unique_ptr<Bus>> bus_list;

	// For Bus and Photonic:
	// List of source and destination buffer since Buses don't have input
	// and output buffers of their own.
	std::vector<std::unique_ptr<Buffer>> src_buffer_list;
	std::vector<std::unique_ptr<Buffer>> dst_buffer_list;

	// For Bus and Photonic Scheduling:
	// index of the last node that has been scheduled to send a packet
	// or message on the bus.
	int last_node_index;

	// For Bus and Photonic Scheduling:
	// index of the last bus (in case there are multiple lanes) that
	// has been assigned to a node to transmit data on.
	int last_bus_index;

	// Private Constructor
	Node();


public:

	/// Get instance of singleton
	static Node *getInstance();


};

}  // namespace net

#endif

