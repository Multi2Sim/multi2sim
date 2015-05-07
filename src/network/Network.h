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

#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include "System.h"
#include "Node.h"
#include "Link.h"
#include "Bus.h"
#include "Buffer.h"

#include <lib/cpp/IniFile.h>
#include <lib/cpp/String.h>

#include "NodeFactory.h"

namespace net
{

class Node;
class NodeData;
class Link;

class Network
{

	//
	// Class Members
	//

	// Network name
	std::string name;

	// Message ID counter
	long long msg_id_counter;

	// List of nodes in the Network
	std::vector<std::unique_ptr<Node>> nodes;

	// Total number of end nodes in the network
	int end_node_count;

	// List of links in the network
	std::vector<std::unique_ptr<Link>> links;

	// Last cycle the snapshot is recorded
	long long last_recorded_snapshot;

	// Last offered bandwidth recorded for the snapshot
	long long last_recorded_offered_bandwidth;

	// Parse the config file to add all the nodes belongs to the network
	void ParseConfigurationForNodes(misc::IniFile &config);

	// A node factory that produces nodes
	std::unique_ptr<NodeFactory> node_factory;




	//
	// Default Values
	//

	// DefaultBufferSizes
	int default_input_buffer_size;
	int default_output_buffer_size;
	int default_bandwidth;

	// Network Specific Values
	int packet_size;
	int net_frequency;




	//
	// Statistics
	//

	// number of transfer that has occurred in the network
	long long transfers;

	// accumulation of latency of all messages in the network
	long long accumulated_latency;

	// Accumulation of size of all messages in the network
	long long offered_bandwidth;

public:

	/// Constructors
	Network(const std::string &name);

	/// Configuration Parser
	void ParseConfiguration(const std::string &section,
			misc::IniFile &config);

	/// Inject node factory
	void setNodeFactory(std::unique_ptr<NodeFactory> node_factory)
	{
		this->node_factory = std::move(node_factory);
	}

	/// Dump the network formation
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Network &network)
	{
		network.Dump(os);
		return os;
	}

	/// Get the string 
	std::string getName() const { return name; }

	/// find and returns node in the network using node name
	///
	/// \param name
	///	node name
	Node *getNodeByName(const std::string &name);

	/// finds and returns node in the network using user data
	///
	/// \param user_data
	///	user_data which is usually provided by memory system
	Node *getNodeByUserData(NodeData *user_data);

	/// Adding a new end-node
	///
	/// \param name
	///	node name
	///
	/// \param input_buffer_size
	///	size of input buffers.
	///
	/// \param ouput_buffer_size
	///	size of output buffers
	///
	/// \param user_data
	///	User data provided by memory system for end-node mod association
	Node *newEndNode(const std::string &name, int input_buffer_size,
			int output_buffer_size, NodeData *user_data);

	/// Adding a new switch
	///
	/// \param name
	///	node name
	///
	/// \param input_buffer_size
	///	size of input buffers.
	///
	/// \param ouput_buffer_size
	///	size of output buffers
	///
	/// \param bandwidth
	///	crossbar bandwidth of the switch
	Node *newSwitchNode(const std::string &name, int input_buffer_size,
			int output_buffer_size, int bandwidth);

	/// Adding a Bus
	///
	/// \param name
	///	bus name
	///
	/// \param bandwidth
	///	bandwidth of each lane in the bus.
	///
	/// \param lanes
	///	number of lanes the bus have
	///
	/// \param delay
	///	fixed delay that can reperesent bus fixed wire delay
	Node *newBusNode(const std::string &name, int bandwidth, int lanes,
			int delay);

	/// Adding photonic Hub
	///
	/// \param name
	///	photonic bus name
	///
	/// \param channels
	///	number of channels in photonic link
	///
	/// \param wavelength
	///	number of wavelength each channel support
	Node *newPhotonicNode(const std::string &name, int channels,
			int wavelength);

	/// Adding New Link
	///
	/// \param srcNode
	///	source of the connection
	///
	/// \param dstNode
	/// destination of the connection
	///
	/// \param bandwidth
	/// bandwidth of the connection
	///
	/// \param srcBufferSize
	/// size of the buffer, connected to the link at source
	///
	/// \param dstBufferSize
	/// size of the buffer, connected to the link at destination
	///
	/// \param virtualChannelCount
	/// number of virtual channels on the physical link
	void newLink(Node *srcNode, Node *dstNode, int bandwidth,
			int srcBufferSize, int dstBufferSize,
			int virtualChannelCount);

	/// Adding New symmetric Bidirectional Links
	///
	/// \param firstNode
	///	first node for the bidirectional link
	///
	/// \param secondNode
	/// second node for the bidirectional link
	///
	/// \param bandwidth
	/// bandwidth of the bidirectional link
	///
	/// \param srcBufferSize
	/// size of the source buffer of the link
	///
	/// \param dstBufferSize
	/// size of the destination buffer of the link
	///
	/// \param virtualChannelCount
	/// number of virtual channels on the bidirectional link
	void newBidirectionalLink(Node *firstNode, Node *secondNode, int bandwidth,
			int srcBufferSize, int dstBufferSize,
			int virtualChannelCount);

	/// Adding New bus port
	///
	/// \param srcNode
	///	first node for the bidirectional link
	///
	/// \param dstNode
	/// second node for the bidirectional link
	///
	/// \param srcBufferSize
	/// size of the source buffer of the link
	///
	/// \param dstBufferSize
	/// size of the destination buffer of the link
	void newBusPort(Node *srcNode, Node *dstNode, int busSrcBufferSize,
			int busDstBufferSize);

	/// Adding New symmetric ports for Bidirectional bus
	///
	/// \param firstNode
	///	first node for the bidirectional bus
	///
	/// \param secondNode
	/// second node for the bidirectional bus
	///
	/// \param srcBufferSize
	/// size of the source port of the bus
	///
	/// \param dstBufferSize
	/// size of the destination port of the bus
	void newBidirectionalBusPort(Node *firstNode, Node *secondNode,
			int busSrcBufferSize, int busDstBufferSize);

	/// Adding New photonic Channel
	///
	/// \param firstNode
	///	source node for the photonic Channel
	///
	/// \param secondNode
	/// destination node for the photonic Channel
	///
	/// \param srcBufferSize
	/// size of the source port of the photonic Channel
	///
	/// \param dstBufferSize
	/// size of the destination port of the photonic Channel
	void newPhotonicChannelPort(Node *srcNode, Node *dstNode,
			int srcBufferSize, int dstBufferSize);

	/// Adding New symmetric ports for Bidirectional photonic Channel
	///
	/// \param firstNode
	///	first node for the bidirectional photonic Channel
	///
	/// \param secondNode
	/// second node for the bidirectional photonic Channel
	///
	/// \param srcBufferSize
	/// size of the source port of the photonic Channel
	///
	/// \param dstBufferSize
	/// size of the destination port of the photonic Channel
	void newBidirectionalPhotonicChannelPort(Node *firstNode,
			Node *secondNode, int srcBufferSize,
			int dstBufferSize);


};


}  // namespace net

#endif

