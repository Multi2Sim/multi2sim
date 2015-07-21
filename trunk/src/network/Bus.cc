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

#include "Bus.h"
#include "Network.h"

namespace net
{


Bus::Bus(Network *network, const std::string &name, int bandwidth, int lanes) :
		Connection(name, network, bandwidth)
{
	for (int i = 0; i < lanes; i++)
	{
		auto lane = misc::new_unique<Lane>(getNumberLanes());
		this->lanes.emplace_back(std::move(lane));
	}
}


void Bus::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n***** Bus %s *****\n", name.c_str());
}

void Bus::TransferPacket(Packet *packet)
{

}
}
