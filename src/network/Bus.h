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

	protected:
		// lane owner
		Buffer* owner;

		// lane is busy until this cycle
		long long busy_cycle;

		// Lane index
		int index;

	public:
		//Set the Lane index in the Bus
		void setLaneIndex(int index) { this->index = index; }

		// Get the Lane index
		int getLaneIndex() const { return this->index; }

	};

protected:

	// List of the Lanes in the bus
	std::vector<std::unique_ptr<Lane>> lanes;

public:
	// Constructor
	Bus(int lanes);

	// Get the number of lanes
	int getNumberLanes() const { return lanes.size(); }

	// Get Lane
	Lane* getLaneByIndex(int id) { return lanes.at(id).get(); }

	// Dump information about Bus
	void Dump(std::ostream &os) const;



};

}  // namespace net

#endif

