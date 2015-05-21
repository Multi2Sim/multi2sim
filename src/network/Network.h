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

#include "System.h"
#include "Node.h"
#include "Link.h"
#include "Bus.h"
#include "Buffer.h"
#include "RoutingTable.h"

namespace net
{

class Node;
class NodeData;
class Connection;

class Network
{

	// Network name
	std::string name;

	// Message ID counter
	long long msg_id_counter = 0;

	// List of nodes in the Network
	std::vector<std::unique_ptr<Node>> nodes;

	// Total number of end nodes in the network
	int end_node_count;

	// List of links in the network
	std::vector<std::unique_ptr<Connection>> connections;

	// Last cycle the snapshot is recorded
	long long last_recorded_snapshot = 0;

	// Last offered bandwidth recorded for the snapshot
	long long last_recorded_offered_bandwidth = 0;

	// Routing table
	std::unique_ptr<RoutingTable> routing_table;

	// Parse the config file to add all the nodes belongs to the network
	void ParseConfigurationForNodes(misc::IniFile &config);

	// Parse the config file to add all the links belongs to the network
	void ParseConfigurationForLinks(misc::IniFile &config);

	// Parse the config file to add all the buses belongs to the network
	void ParseConfigurationForBuses(misc::IniFile &config);

	// Parse the config file to add all the bus ports belongs to a bus
	void ParseConfigurationForBusPorts(misc::IniFile &config);


	//
	// Default Values
	//

	// Default input buffer size
	int default_input_buffer_size;

	// Default output buffer size
	int default_output_buffer_size;

	// Default bandwidth
	int default_bandwidth;

	// Defaule packet size
	int packet_size;

	// Network frequency
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

	/// Destructor
	virtual ~Network() {};

	/// Configuration Parser
	void ParseConfiguration(const std::string &section,
			misc::IniFile &config);

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

	/// Set routing table
	void setRoutingTable(std::unique_ptr<RoutingTable> routing_table)
	{
		this->routing_table = std::move(routing_table);
	}

	/// Get routing table
	const RoutingTable *getRoutingTable() const 
	{
		return routing_table.get();
	}

	/// Check if a node can be sent throught this network
	bool CanSend(Node *source_node, Node *destination_node, int size);

	/// Send an event through network
	void Send(Node *source_node, Node *destination_node, int size,
			esim::EventType *receive,
			esim::EventFrame *receive_frame);

	/// Request to send a packet
	void RequestSend(Node *source_node, Node *destination_node, int size, 
			esim::EventType *receive_event,
			esim::EventFrame *receive_frame,
			esim::EventType *retry_event,
			esim::EventFrame *retry_frame);




	///
	/// Nodes
	///

	/// Add node to the network
	virtual void AddNode(std::unique_ptr<Node> node) 
	{
		nodes.push_back(std::move(node));
	}
	
	/// Produce a node by INI file section
	virtual std::unique_ptr<Node> ProduceNodeByIniSection(
			const std::string &section, 
			misc::IniFile &config);

	/// Produce a node by type string
	virtual std::unique_ptr<Node> ProduceNode(
			const std::string &type, 
			const std::string &name);

	/// find and returns node in the network using node name
	///
	/// \param name
	///	node name
	virtual Node *getNodeByName(const std::string &name) const;

	/// Return the number of nodes
	virtual int getNumberNodes() const { return nodes.size(); }

	/// Return the nodes by index
	virtual Node *getNodeByIndex(int index) { return nodes.at(index).get(); }




	///
	/// Links
	///

	/// Add link to the network
	virtual void AddLink(std::unique_ptr<Link> link);

	/// Configure Link by INI file section
	void ProduceLinkByIniSection(
			const std::string &section, 
			misc::IniFile &config);

	/// Produce a link by link data
	virtual std::unique_ptr<Link> ProduceLink(
			const std::string &name, 
			Node *source_node, 
			Node *destination_node,
			int bandwidth,
			int input_buffer_size,
			int output_buffer_size,
			int virtual_channels);

	///
	/// Buses
	///

	/// Configure Bus by INI file section
	void ProduceBusByIniSection(const std::string &section,
			misc::IniFile &config);

	/// Produce a Bus by bus data
	std::unique_ptr<Bus> ProduceBus(std::string name,
			int bandwidth,
			int lanes);

	/// Produce a Bus Port
	void ProduceBusPortByIniSection(const std::string &section,
				misc::IniFile &config);

	/// Add the Bus to the Connection List of the Network
	void AddBus(std::unique_ptr<Bus> bus);

	/// find and returns Connection in the network using connection name
	///
	/// \param name
	///	node name
	virtual Connection *getConnectionByName(const std::string &name) const;


	/// Return the number of nodes
	virtual int getNumberConnections() const { return connections.size(); }


	/// finds and returns node in the network using user data
	///
	/// \param user_data
	///	user_data which is usually provided by memory system
	// Node *getNodeByUserData(NodeData *user_data);
};


}  // namespace net

#endif

