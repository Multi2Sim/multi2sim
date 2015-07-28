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

#include <string>
#include <regex>
#include <exception>
#include <network/EndNode.h>
#include <network/Message.h>
#include <network/Network.h>
#include <network/System.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

namespace net
{

static void Cleanup()
{
	esim::Engine::Destroy();

	System::Destroy();
}

TEST(TestSystemConfiguration, event_config_0_same_src_dest_not_allowed)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		
		// Sending from the node to itself
		network->TrySend(src, src, 1);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Source and destination cannot "
			"be the same\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_1_no_route)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the source to destination
		network->TrySend(src, dst, 8);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("No route from 'n0' to 'n1'").c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, event_config_2_message_too_big)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the source to destination
		network->TrySend(src, dst, 8);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Buffer too small for the "
			"message size\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_3_packetized_message_too_big)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 3\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source node
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Getting the destination node
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Sending from the source to destination
		network->TrySend(src, dst, 4);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Buffer too small for the "
			"message size\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_4_message_sent_cycle_1)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet = msg->getPacket(0);
		EXPECT_EQ(packet->getNode(), src);
		EXPECT_EQ(packet->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);
		EXPECT_EQ(packet->getBusy(), 1);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 0 of the message 0 has "
					"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_4_message_sent_cycle_2)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));
		Node *s0 = network->getNodeByName("s0");

		// Creating the message
		Message *msg = network->TrySend(src, dst, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Checking the location of the packet
		Packet *packet = msg->getPacket(0);
		EXPECT_EQ(packet->getNode(), s0);
		EXPECT_EQ(packet->getBuffer(), s0->getInputBuffer(0));
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);
		EXPECT_EQ(packet->getBusy(), 2);

		// Making sure that the packet traversed the link
		EXPECT_EQ(src->getOutputBuffer(0)->getBufferHead(), nullptr);
		EXPECT_EQ(src->getSentBytes(), 1);

		// Checking the link status
		Link *link = misc::cast<Link *>(network->getConnectionByName
					("link_n0_s0"));
		EXPECT_EQ(link->getTransferredBytes(), 1);
		EXPECT_EQ(link->getBusyCycle(), 1);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 0 of the message 0 has "
					"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_4_message_sent_cycle_3)
{
	// cleanup singleton instance
	Cleanup();

	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));
		Node *s0 = network->getNodeByName("s0");

		// Creating the message
		Message *msg = network->TrySend(src, dst, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Checking the location of the packet
		Packet *packet = msg->getPacket(0);
		EXPECT_EQ(packet->getNode(), s0);
		EXPECT_EQ(packet->getBuffer(), s0->getOutputBuffer(1));
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);
		EXPECT_EQ(packet->getBusy(), 3);

		// Making sure that the packet traversed the switch
		EXPECT_EQ(s0->getInputBuffer(0)->getBufferHead(), nullptr);
		EXPECT_EQ(s0->getReceivedBytes(), 1);
		EXPECT_EQ(s0->getSentBytes(), 0);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 0 of the message 0 has "
					"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_5_same_src_does_not_allowed)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));

		// Creating the message
		network->TrySend(src, src, 1);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Source and destination cannot "
			"be the same\\.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_6_no_route)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		network->TrySend(src, dst, 1);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("No route from 'n0' to 'n1'").c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, event_config_7_message_sent_cycle_0)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet_1 = msg->getPacket(0);
		EXPECT_EQ(packet_1->getNode(), src);
		EXPECT_EQ(packet_1->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);

		// Continue simulation loop
		esim_engine->ProcessEvents();

		// Recheck the packet position
		EXPECT_EQ(packet_1->getNode(), dst);
		EXPECT_NE(packet_1->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer(), dst->getInputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemConfiguration, event_config_8_two_packets_sent_cycle_0)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 2\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 4);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet_1 = msg->getPacket(0);
		Packet *packet_2 = msg->getPacket(1);
		EXPECT_EQ(packet_1->getNode(), src);
		EXPECT_EQ(packet_1->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);
		EXPECT_EQ(packet_2->getNode(), src);
		EXPECT_EQ(packet_2->getBuffer(), src->getOutputBuffer(0));
		EXPECT_NE(packet_2->getBuffer()->getBufferHead(), packet_2);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 0 of the message 0 has "
			"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_8_two_packets_sent_cycle_1)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 2\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 4);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet_1 = msg->getPacket(0);
		Packet *packet_2 = msg->getPacket(1);
		EXPECT_EQ(packet_1->getNode(), dst);
		EXPECT_EQ(packet_1->getBuffer(), dst->getInputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->write_busy, 3);
		EXPECT_EQ(packet_1->getBusy(), 3);
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);
		EXPECT_EQ(packet_2->getNode(), src);
		EXPECT_EQ(packet_2->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet_2->getBuffer()->getBufferHead(), packet_2);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 1 of the message 0 has "
			"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_8_two_packets_sent_cycle_3)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 2\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 4);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet_1 = msg->getPacket(0);
		Packet *packet_2 = msg->getPacket(1);
		EXPECT_EQ(packet_1->getNode(), dst);
		EXPECT_EQ(packet_1->getBuffer(), dst->getInputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);
		EXPECT_EQ(packet_2->getNode(), src);
		EXPECT_EQ(packet_2->getBuffer(), src->getOutputBuffer(0));
		EXPECT_EQ(packet_2->getBuffer()->getBufferHead(), packet_2);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		message = e.getMessage();
	}
	EXPECT_REGEX_MATCH(misc::fmt("Packet 1 of the message 0 has "
			"not arrived.").c_str(), message.c_str());
}

TEST(TestSystemConfiguration, event_config_8_two_packets_sent_cycle_5)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 2\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 4);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *packet_1 = msg->getPacket(0);
		Packet *packet_2 = msg->getPacket(1);
		EXPECT_EQ(packet_1->getNode(), dst);
		EXPECT_EQ(packet_1->getBuffer(), dst->getInputBuffer(0));
		EXPECT_EQ(packet_1->getBuffer()->getBufferHead(), packet_1);
		EXPECT_EQ(packet_2->getNode(), dst);
		EXPECT_EQ(packet_2->getBuffer(), dst->getInputBuffer(0));
		EXPECT_NE(packet_2->getBuffer()->getBufferHead(), packet_2);

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemConfiguration, event_config_9_message_four_nodes_2_lanes)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n2]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n3]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"Lanes = 2\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1\n"
			"[Network.net0.Busport.port2]\n"
			"Bus = b0\n"
			"Node = n2\n"
			"[Network.net0.Busport.port3]\n"
			"Bus = b0\n"
			"Node = n3";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *n0 = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *n1 = misc::cast<EndNode *>(network->getNodeByName("n1"));
		EndNode *n2 = misc::cast<EndNode *>(network->getNodeByName("n2"));
		EndNode *n3 = misc::cast<EndNode *>(network->getNodeByName("n3"));
		// Creating the message
		Message *msg_1 = network->TrySend(n0, n1, 1);
		Message *msg_2 = network->TrySend(n2, n3, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *message_1_packet_1 = msg_1->getPacket(0);
		Packet *message_2_packet_1 = msg_2->getPacket(0);
		EXPECT_EQ(message_1_packet_1->getNode(), n0);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);
		EXPECT_EQ(message_2_packet_1->getNode(), n2);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Continue simulation loop
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		EXPECT_EQ(message_1_packet_1->getNode(), n1);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n1->getInputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);
		EXPECT_EQ(message_2_packet_1->getNode(), n3);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n3->getInputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
				message_2_packet_1);

		// Receive event
		network->Receive(n1, msg_1);
		network->Receive(n3, msg_2);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemConfiguration, event_config_10_message_four_nodes_1_lane)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n2]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n3]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"Lanes = 1\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1\n"
			"[Network.net0.Busport.port2]\n"
			"Bus = b0\n"
			"Node = n2\n"
			"[Network.net0.Busport.port3]\n"
			"Bus = b0\n"
			"Node = n3";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *n0 = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *n1 = misc::cast<EndNode *>(network->getNodeByName("n1"));
		EndNode *n2 = misc::cast<EndNode *>(network->getNodeByName("n2"));
		EndNode *n3 = misc::cast<EndNode *>(network->getNodeByName("n3"));
		// Creating the message
		Message *msg_1 = network->TrySend(n0, n1, 1);
		Message *msg_2 = network->TrySend(n2, n3, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		Packet *message_1_packet_1 = msg_1->getPacket(0);
		Packet *message_2_packet_1 = msg_2->getPacket(0);
		EXPECT_EQ(message_1_packet_1->getNode(), n0);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);
		EXPECT_EQ(message_2_packet_1->getNode(), n2);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Continue simulation loop
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		EXPECT_EQ(message_1_packet_1->getNode(), n1);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n1->getInputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);
		EXPECT_EQ(message_2_packet_1->getNode(), n2);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
				message_2_packet_1);

		// At this point message_1 is received
		// Continue simulation loop
		esim_engine->ProcessEvents();

		// Checking the location of the packets
		EXPECT_EQ(message_2_packet_1->getNode(), n3);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n3->getInputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
				message_2_packet_1);

		// Receive message
		network->Receive(n3, msg_2);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

TEST(TestSystemConfiguration, event_config_11_packet_four_nodes_1_lane)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 8\n"
			"DefaultOutputBufferSize = 8\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 1\n"
			"[Network.net0.Node.n0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n2]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.n3]\n"
			"Type = EndNode\n"
			"[Network.net0.Bus.b0]\n"
			"Lanes = 1\n"
			"[Network.net0.Busport.port0]\n"
			"Bus = b0\n"
			"Node = n0\n"
			"[Network.net0.Busport.port1]\n"
			"Bus = b0\n"
			"Node = n1\n"
			"[Network.net0.Busport.port2]\n"
			"Bus = b0\n"
			"Node = n2\n"
			"[Network.net0.Busport.port3]\n"
			"Bus = b0\n"
			"Node = n3";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	try
	{
		// Parse the configuration file
		system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source and destination nodes
		EndNode *n0 = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *n1 = misc::cast<EndNode *>(network->getNodeByName("n1"));
		EndNode *n2 = misc::cast<EndNode *>(network->getNodeByName("n2"));
		EndNode *n3 = misc::cast<EndNode *>(network->getNodeByName("n3"));
		// Creating the message
		Message *msg_1 = network->TrySend(n0, n1, 2);
		Message *msg_2 = network->TrySend(n2, n3, 2);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();

		// Getting pointers to packets
		Packet *message_1_packet_1 = msg_1->getPacket(0);
		Packet *message_1_packet_2 = msg_1->getPacket(1);
		Packet *message_2_packet_1 = msg_2->getPacket(0);
		Packet *message_2_packet_2 = msg_2->getPacket(1);

		// Checking the location of first packet of first message
		EXPECT_EQ(message_1_packet_1->getNode(), n0);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);

		// Checking the location of second packet of first message
		EXPECT_EQ(message_1_packet_2->getNode(), n0);
		EXPECT_EQ(message_1_packet_2->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_NE(message_1_packet_2->getBuffer()->getBufferHead(),
				message_1_packet_2);

		// Checking the location of first packet of second message
		EXPECT_EQ(message_2_packet_1->getNode(), n2);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Checking the location of second packet of second message
		EXPECT_EQ(message_2_packet_2->getNode(), n2);
		EXPECT_EQ(message_2_packet_2->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_NE(message_2_packet_2->getBuffer()->getBufferHead(),
						message_2_packet_2);

		// Continue simulation loop to next cycle
		esim_engine->ProcessEvents();

		// Checking the location of first packet of first message
		EXPECT_EQ(message_1_packet_1->getNode(), n1);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n1->getInputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);

		// Checking the location of second packet of first message
		EXPECT_EQ(message_1_packet_2->getNode(), n0);
		EXPECT_EQ(message_1_packet_2->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_EQ(message_1_packet_2->getBuffer()->getBufferHead(),
				message_1_packet_2);

		// Checking the location of first packet of second message
		EXPECT_EQ(message_2_packet_1->getNode(), n2);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Checking the location of second packet of second message
		EXPECT_EQ(message_2_packet_2->getNode(), n2);
		EXPECT_EQ(message_2_packet_2->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_NE(message_2_packet_2->getBuffer()->getBufferHead(),
						message_2_packet_2);

		// Continue simulation loop to next cycle
		esim_engine->ProcessEvents();

		// Checking the location of first packet of first message
		EXPECT_EQ(message_1_packet_1->getNode(), n1);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n1->getInputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);

		// Checking the location of second packet of first message
		EXPECT_EQ(message_1_packet_2->getNode(), n0);
		EXPECT_EQ(message_1_packet_2->getBuffer(), n0->getOutputBuffer(0));
		EXPECT_EQ(message_1_packet_2->getBuffer()->getBufferHead(),
				message_1_packet_2);

		// Checking the location of first packet of second message
		EXPECT_EQ(message_2_packet_1->getNode(), n3);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n3->getInputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Checking the location of second packet of second message
		EXPECT_EQ(message_2_packet_2->getNode(), n2);
		EXPECT_EQ(message_2_packet_2->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_2->getBuffer()->getBufferHead(),
						message_2_packet_2);

		// Continue simulation loop to next cycle
		esim_engine->ProcessEvents();

		// Checking the location of first packet of first message
		EXPECT_EQ(message_1_packet_1->getNode(), n1);
		EXPECT_EQ(message_1_packet_1->getBuffer(), n1->getInputBuffer(0));
		EXPECT_EQ(message_1_packet_1->getBuffer()->getBufferHead(),
				message_1_packet_1);

		// Checking the location of second packet of first message
		EXPECT_EQ(message_1_packet_2->getNode(), n1);
		EXPECT_EQ(message_1_packet_2->getBuffer(), n1->getInputBuffer(0));
		EXPECT_NE(message_1_packet_2->getBuffer()->getBufferHead(),
				message_1_packet_2);

		// Checking the location of first packet of second message
		EXPECT_EQ(message_2_packet_1->getNode(), n3);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n3->getInputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Checking the location of second packet of second message
		EXPECT_EQ(message_2_packet_2->getNode(), n2);
		EXPECT_EQ(message_2_packet_2->getBuffer(), n2->getOutputBuffer(0));
		EXPECT_EQ(message_2_packet_2->getBuffer()->getBufferHead(),
						message_2_packet_2);

		// At this point message_1 is received
		// Continue simulation loop
		esim_engine->ProcessEvents();

		// Checking the location of first packet of second message
		EXPECT_EQ(message_2_packet_1->getNode(), n3);
		EXPECT_EQ(message_2_packet_1->getBuffer(), n3->getInputBuffer(0));
		EXPECT_EQ(message_2_packet_1->getBuffer()->getBufferHead(),
						message_2_packet_1);

		// Checking the location of second packet of second message
		EXPECT_EQ(message_2_packet_2->getNode(), n3);
		EXPECT_EQ(message_2_packet_2->getBuffer(), n3->getInputBuffer(0));
		EXPECT_NE(message_2_packet_2->getBuffer()->getBufferHead(),
						message_2_packet_2);

		// Receive message
		network->Receive(n3, msg_2);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

}
