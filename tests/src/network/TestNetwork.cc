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

#include <lib/cpp/IniFile.h>
#include <network/Network.h>

namespace net
{

TEST(TestNetwork, should_create_according_to_ini_file)
{
	class MockupNode : public Node
	{
		void Dump(std::ostream &os = std::cout) const override
		{
			os << misc::fmt("\nMockup node : %s\n",
					name.c_str());
		}
	};

	class MockupNodeFactory : public NodeFactory
	{
		std::unique_ptr<Node> ProduceNodeByIniSection(Network *network,
				const std::string &section, 
				misc::IniFile &config) override
		{
			std::unique_ptr<Node> node = 
				std::unique_ptr<MockupNode>(new MockupNode());
			node->setName(section);
			return node;
		}
	};

	// Setup ini file
	std::string ini_file = 
		"[ Network.mynet ]\n"
		"DefaultInputBufferSize = 16 \n"
		"DefaultOutputBufferSize = 16 \n"
		"DefaultBandwidth = 1 \n"
		"\n"
		"[ Network.mynet.Node.N1 ]\n"
		"Type = EndNode \n"
		"[ Network.mynet.Node.N2 ]\n"
		"Type = EndNode ";
	misc::IniFile ini;
	ini.LoadFromString(ini_file);

	// Setup network
	Network net("mynet");
	auto factory = misc::new_unique<MockupNodeFactory>();
	net.setNodeFactory(std::move(factory));
	net.ParseConfiguration("Network.mynet", ini);

	// Output stream
	std::ostringstream oss;

	// Assertions
	net.Dump(oss);
	std::string target = "\n***** Network mynet *****\n"
			"\tDefault input buffer size: 16\n"
			"\tDefault output buffer size: 16\n"
			"\tDefault bandwidth: 1\n"
			"\nMockup node : Network.mynet.Node.N1\n"
			"\nMockup node : Network.mynet.Node.N2\n";
	EXPECT_STREQ(target.c_str(), oss.str().c_str());
}

}


