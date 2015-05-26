/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "gtest/gtest.h"

#include <network>
#include <network/TrafficPattern.h>

namespace net
{

TEST(TrafficPattern, should_generate_traffic)
{
	class MockupNetwork : public Network
	{
	}

	class MockupRandomGenerator : public RandomGenerator
	{
		double RandomExp(double lambda) override
		{
			return 2.0f;
		}

		double RandomInRange(int min, int max) override
		{
			return min;
		}
	}

	// Setup network
	MockupNetwork mockup_network("mynet");
	std::unique_ptr<Node> node = 
			mockup_network.ProduceNode("EndNode", "N1");
	mockup_network.AddNode(std::move(node));
	node = mockup_network.ProduceNode("EndNode", "N2");
	mockup_network.AddNode(std::move(node));
	node = mockup_network.ProduceNode("EndNode", "N3");
	mockup_network.AddNode(std::move(node));
	
	// Setup traffic pattern
	TrafficPattern traffic;
	TrafficGroupPair *pair = traffic.AddPair();
	pair.AddSourceNode(mockup_network.getNodeByName("N1"));
	pair.AddSourceNode(mockup_network.getNodeByName("N2"));
	pair.AddDestinationNode(mockup_network.getNodeByName("N3"));

	// Init esim engine
	esim::Engine engine = esim::Engine.getInstance();

	// Inject dependency
	auto random_generator = misc::new_unique<MockupRandomGenerator>();
	traffic.setRandomGenerator(random_generator.get());

	// Generate traffic at the beginning
	std::unique_ptr<GeneratedTraffic> generated = 
			traffic.GenerateTraffic();
	ASSERT_NE(nullptr, generated.get());
	EXPECT_EQ(mockup_network.getNodeByName("N1"), generated.getSource());
	EXPECT_EQ(mockup_network.getNodeByName("N3"), 
			generated.getDestination());

	// No traffic should generate in next cycle
	esim.ProcessEvent();
	generated = traffic.GenerateTraffic();
	ASSERT_EQ(nullptr, generated.get());

	// Traffic should be generated in a third cycle
	esim.ProcessEvent();
	generated = traffic.GeneratedTraffic();
	ASSERT_NE(nullptr, generated.get());
	EXPECT_EQ(mockup_network.getNodeByName("N1"), generated.getSource());
	EXPECT_EQ(mockup_network.getNodeByName("N3"), 
			generated.getDestination());
}

}
