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


class NodeData
{
public:

	// virtual destructor
	~NodeData();
};

class Node
{

	/// Node types
	enum NodeType
	{
		NodeTypeInvalid = 0,
		NodeTypeEndNode,
		NodeTypeSwitch,
		NodeTypeBus,
		NodeTypePhotonic
	};

	/// String map for type
	misc::StringMap TypeMap;

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
	NodeData *user_data;

	// Node Bandwidth. Required for Bus and Switch
	int bandwidth;

	// Input/Output Buffer List
	std::vector<std::unique_ptr<Buffer>> input_buffers;
	std::vector<std::unique_ptr<Buffer>> output_buffers;

	// Bus and Photonic Nodes
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


public:

	/// Node object constructor
	///
	/// \param name
	///	node name
	///
	/// \param node_type
	///	node type identifier
	///
	/// \param input_buffer_size
	///	buffer size of node's input buffers
	///
	/// \param output_buffer_size
	///	buffer size of node's output buffers
	///
	/// \param bandwidth
	///	if switch, Bus or photonic, this is the bandwidth of crossbar
	///	or bus lanes
	///
	/// \param index
	///	node index in Network::nodes list
	///
	/// \param user_data
	///	user_data associated with the node which is used in memory
	///	system.
	Node(const std::string &name, int input_buffer_size, NodeType node_type,
			int output_buffer_size, int bandwidth, int index,
			NodeData *user_data);

	/// Finding a Node's buffer by its name. Function looks up first input
	/// buffers and if the buffer is not found, it looks up output buffers
	/// to find the buffer.
	///
	/// \param buffer_name
	///	name of the buffer.
	Buffer *getBufferByName(const std::string &buffer_name);




};

}  // namespace net

#endif

