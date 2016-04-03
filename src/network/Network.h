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

#include <lib/cpp/IniFile.h>
#include <lib/cpp/String.h>
#include <lib/esim/Event.h>
#include <lib/esim/Engine.h>

#include "Bus.h"
#include "Buffer.h"
#include "Graph.h"
#include "Link.h"
#include "Message.h"
#include "Node.h"
#include "RoutingTable.h"
#include "System.h"

namespace net
{
class Connection;
class EndNode;
class Node;
class Switch;
class Graph;

class Network
{

	// Network name
	std::string name;

	// Message ID counter
	long long message_id_counter = 0;

	// A hashtable of messages in flight
	std::unordered_map<long long, std::unique_ptr<Message>> message_table;

	// List of nodes in the network
	std::vector<std::unique_ptr<Node>> nodes;

	// Total number of end nodes in the network
	int num_end_nodes = 0;

	// List of connections in the network
	std::vector<std::unique_ptr<Connection>> connections;

	// Last cycle the snapshot is recorded
	long long last_recorded_snapshot = 0;

	// Last offered bandwidth recorded for the snapshot
	long long last_recorded_offered_bandwidth = 0;

	// Routing table
	RoutingTable routing_table;

	// Parse the config file to add all the nodes belongs to the network
	void ParseConfigurationForNodes(misc::IniFile *ini_file);

	// Parse the config file to add all the links belongs to the network
	void ParseConfigurationForLinks(misc::IniFile *ini_file);

	// Parse the config file to add all the buses belongs to the network
	void ParseConfigurationForBuses(misc::IniFile *ini_file);

	// Parse the config file to add all the bus ports belongs to a bus
	void ParseConfigurationForBusPorts(misc::IniFile *ini_file);

	// Parse the routing elements, for manual routing.
	bool ParseConfigurationForRoutes(misc::IniFile *ini_file);




	//
	// Default Values
	//

	// Default input buffer size
	int default_input_buffer_size = 4;

	// Default output buffer size
	int default_output_buffer_size = 4;

	// Default bandwidth
	int default_bandwidth = 4;

	// Defaule packet size - zero means no packeting
	int packet_size = 0;

	// fix latency of the network. If activated
	// the network sends the messages with a fixed latency
	// regardless of the topology.
	// Ideal network, is a network with a fixed_latency
	// of 1.
	int fix_latency = 0;


	
	//
	// Statistics
	//

	// number of transfer that has occurred in the network
	long long transfers = 0;

	// accumulation of latency of all messages in the network
	long long accumulated_latency = 0;

	// Accumulation of size of all messages in the network
	long long accumulated_bytes = 0;




	//
	// Visualization graph
	//

	std::unique_ptr<Graph> graph;

public:

	/// Constructors
	Network(const std::string &name);

	/// Parse the network configuration file.
	void ParseConfiguration(misc::IniFile *ini_file,
			const std::string &section);

	/// Dump the network information.
	void Dump(std::ostream &os = std::cout) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Network &network)
	{
		network.Dump(os);
		return os;
	}

	/// Update the trace header with network information
	void TraceHeader();

	/// Get the name of the network.
	const std::string &getName() const { return name; }

	/// Return the routing table of the network.
	RoutingTable *getRoutingTable() { return &routing_table; }

	/// Set packet size
	void setPacketSize(int packet_size) { this->packet_size = packet_size; }

	/// Get packet size
	int getPacketSize() const { return packet_size; }

	/// Get the condition of the network, to see if it is
	/// and ideal network with a fix latency or not.
	///
	/// \return
	///	 true if network is ideal
	bool hasConstantLatency() { return fix_latency; }

	/// Get the fix delay of the network
	int getFixLatency() const {return fix_latency; }

	/// Create a message to be transfered in the network. The network 
	/// keeps the ownership of the message. Message is destoried when it 
	/// is received by the \a destination node.
	///
	Message *newMessage(EndNode *source_node, EndNode *destination_node,
			int size);

	/// Check if a message can be sent throught from the given source to
	/// the given destination node. A message can be sent if:
	///
	/// - There is enough room in the destination buffer for the next hop
	///   toward the destination.
	///
	/// - The link to be traversed is not busy.
	///
	/// \param source_node
	///	Source node for the message transfer. This must be an end node.
	///
	/// \param destination_node
	///	Destination node for the message transfer. Must be an end node.
	///
	/// \param size
	///	Size in bytes of the message to transfer.
	///
	/// \param retry_event
	///	If the message cannot be sent in the current cycle, this
	///	optional argument indicates an event to be scheduled when the
	///	condition should be evaluated again. This doesn't mean that
	///	when the event occurs the message will be ready to be sent.
	///	But for sure the message will not be ready before the event is
	///	scheduled. If this argument is other than `nullptr`, the
	///	CanSend() function must be invoked within an event handler.
	///
	/// \return
	///	The function returns true if the message can be sent right
	///	away. If not, the function returns false and schedules event
	///	`retry_event` if other than `nullptr`.
	///
	bool CanSend(EndNode *source_node,
			EndNode *destination_node,
			int size,
			esim::Event *retry_event = nullptr);

	/// Send a message through network.
	///
	/// \param source_node
	///	Source node for message transfer. Must be an end node.
	///
	/// \param destination_node
	///	Final destination for message. Must be an end node.
	///
	/// \param size
	///	Size in bytes of the message, which will be split into packets
	///	internally.
	///
	/// \param receive_event
	///	If this optional argument is specified and other than `nullptr`,
	///	the given event will be scheduled once the full message is
	///	received by the destination node. If the Send() function is
	///	invoked within an event handler, the current event frame will
	///	be available once `receive_event` is scheduled.
	///
	/// \return
	///	The function returns a new message object internally used in
	///	the network to track the progress of the packets. Careful: the
	///	validity of this object is limited until the time the message
	///	reaches its destination (when `receive_event` would be
	///	scheduled).
	///
	Message *Send(EndNode *source_node,
			EndNode *destination_node,
			int size,
			esim::Event *receive_event = nullptr);

	/// Send a message through the network, only if it is possible to send
	/// it right away. A message can be sent if:
	///
	/// - There is enough room in the destination buffer for the next hop
	///   toward the destination.
	///
	/// - The link to be traversed is not busy.
	///
	/// This function must be invoked within an event handler.
	///
	/// \param source_node
	///	Source node for the message transfer. This must be an end node.
	///
	/// \param destination_node
	///	Final destination node for the message. Must be an end node.
	///
	/// \param size
	///	Size in bytes of the message to transfer. The message will be
	///	internally split into packets.
	///
	/// \param receive_event
	///	If this optional argument is specified and other than `nullptr`,
	///	this event will be scheduled once the message has been fully
	///	received in the destination node. The current event frame will
	///	be available at that time. This argument has no effect if the
	///	message could not be sent right away.
	///
	/// \param retry_event
	///	If this optional argument is specified and other than `nullptr`,
	///	this event will be scheduled in order for the caller to invoke
	///	`TrySend` again, and check whether the message can be sent this
	///	time. The current event frame will be available at that time.
	///	This argument has no effect if the message was sent right away.
	///
	/// \return
	///	If the message was sent right away, the function returns a
	///	pointer to the created message object. Careful: the validity of
	///	this object is limited until the time when the message is fully
	///	received by the destination node. If the message was not sent
	///	right away, the function returns `nullptr`.
	///
	Message *TrySend(EndNode *source_node,
			EndNode *destination_node,
			int size,
			esim::Event *receive_event = nullptr,
			esim::Event *retry_event = nullptr);


	/// Absorb a message located at the head of the input buffer of a
	/// node. The caller must make sure that this is the actual location
	/// of the message. This function should be called when the
	/// `receive_event` is triggered after a call to Send() or TrySend().
	/// The message object is freed in this call.
	void Receive(EndNode *node, Message *message);




	//
	// Nodes
	//

	/// Create an end node in the network
	EndNode *addEndNode(
			int input_buffer_size, int output_buffer_size,
			const std::string &name,
			void *user_data);

	/// Create a switch in the network
	Switch *addSwitch(int input_buffer_size,
			int output_buffer_size,
			int bandwidth,
			const std::string &name);

	/// find and returns node in the network using node name
	///
	/// \param name
	///	node name
	Node *getNodeByName(const std::string &name) const;

	/// Return the number of nodes.
	int getNumNodes() const { return nodes.size(); }

	/// Return the number of end nodes
	int getNumEndNodes() const { return num_end_nodes; }

	/// Return a node by its index.
	Node *getNode(int index)
	{
		assert(index >= 0 && index < (int) nodes.size());
		return nodes[index].get();
	}

	/// Finds and returns node in the network given its user data.
	///
	/// \param user_data
	///	User data attached by the memory system.
	Node *getNodeByUserData(void *user_data) const;




	//
	// Links
	//

	/// Add a link to the network
	Link *addLink(const std::string name,
			Node *source_node,
			Node *dest_node,
			int bandwidth,
			int source_buffer_size,
			int dest_buffer_size,
			int num_virtual_channels);

	/// Add a bidirectional link to the network
	void addBidirectionalLink(const std::string name,
			Node *source_node,
			Node *dest_node,
			int bandwidth,
			int source_buffer_size,
			int dest_buffer_size,
			int num_virtual_channels);




	//
	// Buses
	//

	/// Add the Bus to the Connection List of the Network.
	Bus *addBus(const std::string name,
			int bandwith,
			int lanes);

	/// Find and returns connection in the network using connection name.
	///
	/// \param name
	///	connection name
	Connection *getConnectionByName(const std::string &name) const;

	/// Return a connection by its index.
	Connection *getConnection(int index)
	{
		assert(index >= 0 && index < (int) connections.size());
		return connections[index].get();
	}

	/// Return the number of connection
	int getNumConnections() const { return connections.size(); }

	/// Generating the report dump
	void DumpReport(const std::string &path);
	
	/// Dump the network information.
	void DumpReport(std::ostream &os = std::cout) const;

	/// Generating the static graph file
	void StaticGraph(const std::string &path);
};


}  // namespace net

#endif

