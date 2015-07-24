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

class Bus : public Connection
{

public:

	class Lane
	{

	public:

		// Constructor
		Lane() {}



		//
		// Statistics
		//

		// Number of bytes that was transfered through the lane
		long long transferred_bytes = 0;

		// Number of cycles that the lane was busy
		long long busy_cycles = 0;

		// Number of packets that traversed the lane
		long long transferred_packets = 0;



		//
		// Public fields
		//

		// Last cycle lane was scheduled
		long long sched_when;

		// The last buffer that was scheduled
		Buffer *sched_buffer;

		// lane is busy until this cycle
		long long busy = -1;

	};

private:

	// List of the Lanes in the bus
	std::vector<std::unique_ptr<Lane>> lanes;

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
	int last_node_index;

};

}  // namespace net

#endif

