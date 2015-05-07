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
		std::string getType() const override
		{
			return "MockupNode";
		}
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
			std::vector<std::string> tokens;
			misc::StringTokenize(section, tokens, ".");
			std::unique_ptr<Node> node = 
				std::unique_ptr<MockupNode>(new MockupNode());
			node->setName(tokens[3]);
			return node;
		}
	};

	class MockupLink : public Link
	{
		void Dump(std::ostream &os = std::cout) const override
		{
			os << misc::fmt("\nMockup link : %s\n", 
					name.c_str());
		}
	};

	class MockupLinkFactory : public LinkFactory
	{
		std::unique_ptr<Link> ProduceLinkByIniSection(Network *network,
				const std::string &section, 
				misc::IniFile &config) override
		{
			std::vector<std::string> tokens;
			misc::StringTokenize(section, tokens, ".");
			std::unique_ptr<Link> link = 
				std::unique_ptr<MockupLink>(new MockupLink());
			link->setName(tokens[3]);
			return link;
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
		"Type = EndNode \n"
		"[Network.mynet.Link.N1-N2]\n"
		"Type = Bidirectional\n"
		"Source = N1\n"
		"Dest = N2";
	misc::IniFile ini;
	ini.LoadFromString(ini_file);

	// Setup network
	Network net("mynet");
	auto node_factory = misc::new_unique<MockupNodeFactory>();
	auto link_factory = misc::new_unique<MockupLinkFactory>();
	net.setNodeFactory(std::move(node_factory));
	net.setLinkFactory(std::move(link_factory));
	net.ParseConfiguration("Network.mynet", ini);

	// Output stream
	std::ostringstream oss;

	// Assertions
	net.Dump(oss);
	std::string target = "\n***** Network mynet *****\n"
			"\tDefault input buffer size: 16\n"
			"\tDefault output buffer size: 16\n"
			"\tDefault bandwidth: 1\n"
			"\nMockup node : N1\n"
			"\nMockup node : N2\n"
			"\nMockup link : N1-N2\n";
	EXPECT_STREQ(target.c_str(), oss.str().c_str());
}

}


