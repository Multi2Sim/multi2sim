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

namespace net
{

class Node;
class NodeData;
class Link;

class Network
{

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

	// Parse the config file to add all the nodes belongs to the network
	void ParseConfigurationForLinks(misc::IniFile &config);




	//
	// Default Values
	//

	// Default buffer size
	int default_input_buffer_size;
	int default_output_buffer_size;
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
	virtual Node *getNodeByIndex(int index) { return nodes[index].get(); }




	///
	/// Links
	///

	/// Add link to the network
	virtual void AddLink(std::unique_ptr<Link> link);

	/// Produce a Link by INI file section
	virtual std::unique_ptr<Link> ProduceLinkByIniSection(
			const std::string &section, 
			misc::IniFile &config);

	/// Produce a node by type string
	virtual std::unique_ptr<Link> ProduceLink(
			const std::string &name, 
			Node *source_node, 
			Node *destination_node);

	/// find and returns node in the network using node name
	///
	/// \param name
	///	node name
	virtual Link *getLinkByName(const std::string &name) const;



	/// finds and returns node in the network using user data
	///
	/// \param user_data
	///	user_data which is usually provided by memory system
	// Node *getNodeByUserData(NodeData *user_data);
};


}  // namespace net

#endif

