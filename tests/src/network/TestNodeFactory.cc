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
#include <cxxabi.h>

#include <lib/cpp/IniFile.h>
#include <network/Network.h>
#include <network/NodeFactory.h>
#include <network/Node.h>
#include <network/EndNode.h>

namespace net
{

TEST(TestNodeFactory, should_throw_error_if_unknown_type)
{
	// Init variables
	NodeFactory factory;
	Network net("Test network");

	// Death test
	EXPECT_THROW(factory.ProduceNode(&net, "UNKNOWN", "N1"), 
			misc::Error);
}


TEST(TestNodeFactory, should_produce_end_node)
{
	// Init variables
	NodeFactory factory;
	Network net("Test network");

	// Produce node
	std::unique_ptr<Node> node = factory.ProduceNode(
			&net, "EndNode", "N1");

	// Assertion
	EXPECT_TRUE(dynamic_cast<EndNode *>(node.get()) != 0);
}


TEST(TestNodeFactory, should_produce_end_node_from_ini_section)
{
	// Init variable
	NodeFactory factory;
	Network net("mynet");

	// Inifile
	std::string ini = "[ Network.mynet.Node.N1 ]\n"
			"Type = EndNode";
	misc::IniFile ini_file;
	ini_file.LoadFromString(ini);

	// Produce node
	std::unique_ptr<Node> node = factory.ProduceNodeByIniSection(
			&net, "Network.mynet.Node.N1", ini_file);

	// Assertions
	EXPECT_TRUE(dynamic_cast<EndNode *>(node.get()) != 0);
	EXPECT_STREQ(node->getName().c_str(), "N1");
}

}
