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

#ifndef NETWORK_TRAFFICPATTERN_H
#define NETWORK_TRAFFICPATTERN_H

#include "Node.h"
#include "TrafficGroupPair.h"

namespace net
{

// A generated traffic is a message to be injected to the network
class GeneratedTraffic
{
	// Source of the message
	Node *source;

	// Destination of the message
	Node *destination;

public:

	// Constructor
	GeneratedTraffic(Node *source, Node *destination)
	{
		this->source = source;
		this->destination = destination;
	}
};

// A TrafficPattern is a network load generator. It uniformly selects from 
// nodes and generate messages with interarrival time that follows Poisson 
// distribution
class TrafficPattern
{
protected:

	// The expected number of messages to be injected to the network
	double injection_rate = 0;

	// Next injection time
	double next_injection = 0;

	// A list of traffic group pairs for the traffic to be generated
	std::vector<std::unique_ptr<TrafficGroupPair>> pairs;

public:

	// Virtual distructor
	virtual ~TrafficPattern() {};	

	// Set injection_rate
	void setInjectionRate(double injection_rate)
	{
		this->injection_rate = injection_rate;
	}

	// Try generate traffic 
	virtual std::unique_ptr<GeneratedTraffic> GenerateTraffic()
	{
		return std::unique_ptr<GeneratedTraffic>(nullptr);
	}

	// Get number of traffic pairs
	unsigned int getNumberPairs() const { return pairs.size(); }

	// Get traffic group pair by index
	TrafficGroupPair *getPairByIndex(unsigned int index)
	{
		return pairs.at(index).get();
	}

	// Add traffic pair to traffic pattern
	TrafficGroupPair *AddPair()
	{
		auto pair = misc::new_unique<TrafficGroupPair>();
		TrafficGroupPair *pair_ptr = pair.get();
		pairs.push_back(std::move(pair));
		return pair_ptr;	
	}

};

}

#endif

