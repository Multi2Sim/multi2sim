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
	const std::vector<std::unique_ptr<Node>> nodes;

	// Total number of end nodes in the network
	int end_node_count;

	// List of links in the network
	const std::vector<std::unique_ptr<Link>> links;

	// Last cycle the snapshot is recorded
	long long last_recorded_snapshot;

	// Last offered bandwidth recorded for the snapshot
	long long last_recorded_offered_bandwidth;

	///
	/// Statistics
	///

	// number of transfer that has occurred in the network
	long long transfers;

	// accumulation of latency of all messages in the network
	long long accumulated_latency;

	// Accumulation of size of all messages in the network
	long long offered_bandwidth;




public:

	/// constructor for object
	Network(const std::string &name);

	///
	/// Node Related Functions
	///

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


	///
	/// Connection Related Functions
	///



};
}  // namespace net

#endif

