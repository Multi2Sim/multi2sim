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
class Connection;

/// A node in a interconnect network is where the packet is generated,
/// forwarded and consumed
class Node
{
public:

	/// Different node types, each indicating which actual subclass
	/// each instance of a node is of.
	enum Type
	{
		TypeInvalid = 0,
		TypeEndNode,
		TypeSwitch,
		TypeBug
	};

protected:

	// Network that it belongs to
	Network *network;
	
	// Node Name
	std::string name;

	// Node type
	Type type;

	// Node index
	int index;

	// Input buffer size
	int input_buffer_size;

	// Output buffer size
	int output_buffer_size;

	// User data, used by the memory system to attach information about
	// which module is associated with this network node.
	void *user_data = nullptr;

	// Input buffer list
	std::vector<std::unique_ptr<Buffer>> input_buffers;

	// Output buffer list
	std::vector<std::unique_ptr<Buffer>> output_buffers;

public:

	/// Constructor
	Node(Network *network,
			int index,
			int input_buffer_size,
			int output_buffer_size,
			const std::string &name,
			Type type,
			void *user_data);

	/// Get name
	std::string getName() const { return name; }

	/// Get the node type
	Type getType() const { return type; }

	/// Get the index of the node
	int getIndex() const { return index; }

	/// Dump the node information. 
	virtual void Dump(std::ostream &os = std::cout) const = 0;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const Node &node)
	{
		node.Dump(os);
		return os;
	}

	/// Add an input buffer of the given size
	Buffer *addInputBuffer(int size, Connection *connection);

	/// Add an output buffer of the given size
	Buffer *addOutputBuffer(int size, Connection *connection);

	/// Return the user data attached by the memory system
	void *getUserData() const { return user_data; }

	/// Assign user data to a network node, usually a module from the
	/// memory system.
	void setUserData(void *user_data) { this->user_data = user_data; }

	/// Return the number of output buffers
	int getNumOutputBuffer() { return output_buffers.size(); }

	/// Rreturn output buffer by index
	Buffer *getOutputBuffer(int index) 
	{ 
		return output_buffers[index].get(); 
	}

	/// Return the number of input buffers
	int getNumInputBuffer() { return input_buffers.size(); }

	/// Rreturn input buffer by index
	Buffer *getInputBuffer(int index) 
	{ 
		return input_buffers[index].get(); 
	}

};


}  // namespace net

#endif

