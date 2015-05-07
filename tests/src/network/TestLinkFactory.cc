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
#include <network/EndNode.h>
#include <network/Link.h>
#include <network/LinkFactory.h>

namespace net
{

TEST(TestLinkFactory, should_produce_link_by_ini_section)
{

	// Init variable
	LinkFactory factory;
	Network net("mynet");

	// Add nodes
	auto S1 = misc::new_unique<EndNode>();
	S1->setName("S1");
	auto S2 = misc::new_unique<EndNode>();
	S2->setName("S2");
	net.addNode(std::move(S1));
	net.addNode(std::move(S2));

	// Ini file
	std::string ini = "[ Network.mynet.Link.S1-S2 ]\n"
		"Type = Bidirectional\n"
		"Source = S1\n"
		"Dest = S2";
	misc::IniFile ini_file;
	ini_file.LoadFromString(ini);

	// Produce link
	std::unique_ptr<Link> link = factory.ProduceLinkByIniSection(&net, 
			"Network.mynet.Link.S1-S2", ini_file);			

	// Assertions
	EXPECT_TRUE(dynamic_cast<Link *>(link.get()) != 0);
	EXPECT_STREQ("S1-S2", link->getName().c_str());
	EXPECT_STREQ("S1", link->getSourceNode()->getName().c_str());
	EXPECT_STREQ("S2", link->getDestinationNode()->getName().c_str());

}


TEST(TestLinkFactory, should_produce_link)
{
	// Init variable
	LinkFactory factory;
	Network net("Test net");
	EndNode src;
	EndNode dest;

	// Produce link
	std::unique_ptr<Link> link = factory.ProduceLink(&net, "N1-S1", 
			&src, &dest);

	// Assertion
	EXPECT_TRUE(dynamic_cast<Link *>(link.get()) != 0);
	EXPECT_STREQ(link->getName().c_str(), "N1-S1");

}

}
