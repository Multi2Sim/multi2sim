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

#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include <lib/cpp/String.h>

namespace net
{

class Network;
class Buffer;

class Connection
{
protected:
	// Network associated with the connection
	Network *network;

	// Connection Name
	std::string name;

	// Connection bandwidth
	int bandwidth;

	// Connection Latency
	int fix_latency;

	// Number of cycles that the connection was busy
	long long busy_cycles;

	// List of the source buffers connected to the bus
	std::vector< Buffer* > source_buffers;

	// List of the destination buffers connected to the bus
	std::vector< Buffer* > destination_buffers;


public:

	/// Set name
	void setName(const std::string &name) { this->name = name; }

	/// Get the name of the connection
	std::string getName() const { return name; }

	/// Set bandwidth -- not constant for future use
	void setBandwidth(int bandwidth) { this->bandwidth = bandwidth;}

	/// Dump connection information
	virtual void Dump(std::ostream &os) const = 0;

	std::vector<Buffer *> getDestinationBuffers() {
		return destination_buffers;
	}

	// Adding ports to the bus source list
	void addSourceBuffer(Buffer * buffer);

	// Adding ports to the bus destination list
	void addDestinationBuffer(Buffer * buffer);

};
}



#endif
