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
#include <network/Switch.h>
#include <network/Link.h>

namespace net
{

TEST(Network, should_create_according_to_ini_file)
{
	// Setup ini file
	std::string ini_file = 
		"[ Network.mynet ]\n"
		"DefaultInputBufferSize = 16 \n"
		"DefaultOutputBufferSize = 16 \n"
		"DefaultBandwidth = 1 \n"
		"\n"
		"[ Network.mynet.Node.N1 ]\n"
		"Type = EndNode \n"
		"[ Network.mynet.Node.S1 ]\n"
		"Type = Switch \n"
		"[Network.mynet.Link.N1-S1]\n"
		"Type = Bidirectional\n"
		"Source = N1\n"
		"Dest = S1";
	misc::IniFile ini;
	ini.LoadFromString(ini_file);

	// Setup network
	Network net("mynet");
	net.ParseConfiguration("Network.mynet", ini);

	// Assert N1 is created 
	Node *n1 = net.getNodeByName("N1");
	EXPECT_TRUE(n1 != nullptr);
	EXPECT_TRUE(dynamic_cast<EndNode *>(n1) != 0);

	// Assert S1 is created
	Node *s1 = net.getNodeByName("S1");
	EXPECT_TRUE(s1 != nullptr);
	EXPECT_TRUE(dynamic_cast<Switch *>(s1) != 0);

	// Assert link n1-s1 is created
	Link *n1_s1 = net.getLinkByName("N1-S1");
	EXPECT_TRUE(n1_s1 != nullptr);
}

}


