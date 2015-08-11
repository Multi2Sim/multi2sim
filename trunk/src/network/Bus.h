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

#ifndef NETWORK_BUS_H
#define NETWORK_BUS_H

#include "Connection.h"
#include "Buffer.h"

namespace net
{
class Buffer;

class Lane
{

	friend class Bus;

	// Lane bandwidth
	int bandwidth;

	// Bus lane index
	int index;



	//
	// Statistics
	//

	// Number of bytes that was transfered through the lane
	long long transferred_bytes = 0;

	// Number of cycles that the lane was busy
	long long busy_cycles = 0;

	// Number of packets that traversed the lane
	long long transferred_packets = 0;



public:

	// Constructor
	Lane(int bandwidth) : bandwidth(bandwidth) {}

	// Dumping lane statistics
	void Dump(std::ostream &os = std::cout) const;

	// Getter for bandwidth
	int getBandwidth() const { return bandwidth; }

	// Increase the amount of transfered bytes
	void incTransferredBytes(int bytes) { transferred_bytes += bytes; }

	// Increase the number of transfered packets
	void incTransferredPackets() { transferred_packets++; }

	// Increase the number of cycles that the lane was busy
	void incBusyCycles(long long cycles) { busy_cycles += cycles;}




	//
	// Public fields
	//

	// Last cycle lane was scheduled
	long long scheduled_when = 0;

	// The last buffer that was scheduled
	Buffer *scheduled_buffer = nullptr;

	// lane is busy until this cycle
	long long busy = -1;

};


class Bus : public Connection
{

private:

	// List of the Lanes in the bus
	std::vector<std::unique_ptr<Lane>> lanes;




	//
	// Arbitration and scheduling
	//

	// Arbitration for each lane
	Buffer *LaneArbitration(Lane *lane);

	// Arbitration between available lanes
	Lane *Arbitration(Buffer *current_buffer);

public:

	// Constructor
	Bus(Network *network, const std::string &name,
			int bandwidth, int num_lanes);

	// Get the number of lanes
	int getNumberLanes() const { return lanes.size(); }

	// Get Lane
	Lane* getLaneByIndex(int id) { return lanes.at(id).get(); }

	// Dump information about Bus
	void Dump(std::ostream &os) const;

	/// Transfer the packet from an output buffer
	void TransferPacket(Packet *packet);




	//
	// Public fields
	//

	// The index of last node scheduled on the bus
	int last_node_index = -1;

};

}  // namespace net

#endif

