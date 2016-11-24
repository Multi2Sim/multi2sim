/*
 *  Multi2Sim
 *  Copyright (C) 2015  Amir Kavyan Ziabari (ziabari@gmail.com)
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
	EXPECT_REGEX_MATCH("No route from 'n0' to 'n0'\\.",
		message.c_str());
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
	EXPECT_REGEX_MATCH("No route from 'n0' to 'n1'\\.",
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

TEST(TestSystemConfiguration, event_config_4_message_sent_cycle_4)
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
	try
	{
		// Parse the configuration file
		network_system->ParseConfiguration(&ini_file);

		// Getting the network
		Network *network = network_system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *src = misc::cast<EndNode *>(network->getNodeByName("n0"));
		EndNode *dst = misc::cast<EndNode *>(network->getNodeByName("n1"));

		// Creating the message
		Message *msg = network->TrySend(src, dst, 1);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();

		// Receive event
		network->Receive(dst, msg);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
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
	EXPECT_REGEX_MATCH("No route from 'n0' to 'n0'\\.",
		 message.c_str());
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
	EXPECT_REGEX_MATCH("No route from 'n0' to 'n1'\\.",
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

///
/// Test topology
///
///   N0  -->  S0  -->  S1  -->  S2  -->  S3
///                                  \    |
///                                   v   v
///                                    N1
/// Without route the path will be through S2, but with route
/// path will be through S3
TEST(TestSystemConfiguration, event_config_12_manual_vs_fw_routing)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.net0.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.S0]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S1]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S2]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S3]\n"
			"Type = Switch\n"
			"[Network.net0.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"VC = 2\n"
			"[Network.net0.Link.S0-S1]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S1\n"
			"[Network.net0.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.net0.Link.S2-S3]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = S3\n"
			"[Network.net0.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.net0.Link.S3-N1]\n"
			"Type = Unidirectional\n"
			"Source = S3\n"
			"Dest = N1\n";

	std::string route = "[Network.net0.Routes]\n"
			"N0.to.N1 = S0:1\n"
			"S0.to.N1 = S1\n"
			"S1.to.N1 = S2\n"
			"S2.to.N1 = S3\n"
			"S3.to.N1 = N1";

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

		// Getting the source, destination and switch nodes
		EndNode *N0 = dynamic_cast<EndNode *>(network->getNodeByName("N0"));
		EndNode *N1 = dynamic_cast<EndNode *>(network->getNodeByName("N1"));
		Node *S0 = network->getNodeByName("S0");
		Node *S1 = network->getNodeByName("S1");
		Node *S2 = network->getNodeByName("S2");

		// Create a message
		Message *message = network->TrySend(N0, N1, 1);
		Packet *packet = message->getPacket(0);

		// Simulation loop an checking location of packet - cycle 1
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), N0);
		EXPECT_EQ(packet->getBuffer(), N0->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 2
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S0);
		EXPECT_EQ(packet->getBuffer(), S0->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 3
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S0);
		EXPECT_EQ(packet->getBuffer(), S0->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 4
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S1);
		EXPECT_EQ(packet->getBuffer(), S1->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 5
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S1);
		EXPECT_EQ(packet->getBuffer(), S1->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 6
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S2);
		EXPECT_EQ(packet->getBuffer(), S2->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 7
		// We are not checking the index of buffer since it can change
		// depending on the order code parse the links
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S2);
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);

		// Simulation loop an checking location of packet - cycle 8
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), N1);
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);

		// Receive message
		network->Receive(N1, message);

	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}

	// Cleanup previous instance
	Cleanup();

	// Load the new INI file
	misc::IniFile ini_file_2;
	std::string config_route = config + route;
	ini_file_2.LoadFromString(config_route);

	// Set up network instance again
	system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body for route
	try
	{
		// Parse the new configuration file
		system->ParseConfiguration(&ini_file_2);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *N0 = dynamic_cast<EndNode *>(network->getNodeByName("N0"));
		EndNode *N1 = dynamic_cast<EndNode *>(network->getNodeByName("N1"));
		Node *S0 = network->getNodeByName("S0");
		Node *S1 = network->getNodeByName("S1");
		Node *S2 = network->getNodeByName("S2");
		Node *S3 = network->getNodeByName("S3");

		// Create a message
		Message *message = network->TrySend(N0, N1, 1);
		Packet *packet = message->getPacket(0);

		// Simulation loop an checking location of packet - cycle 1
		// Packet should be in second buffer because of the virtual channel
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), N0);
		EXPECT_EQ(packet->getBuffer(), N0->getOutputBuffer(1));

		// Simulation loop an checking location of packet - cycle 2
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S0);
		EXPECT_EQ(packet->getBuffer(), S0->getInputBuffer(1));

		// Simulation loop an checking location of packet - cycle 3
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S0);
		EXPECT_EQ(packet->getBuffer(), S0->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 4
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S1);
		EXPECT_EQ(packet->getBuffer(), S1->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 5
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S1);
		EXPECT_EQ(packet->getBuffer(), S1->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 6
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S2);
		EXPECT_EQ(packet->getBuffer(), S2->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 7
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S2);
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);

		// Simulation loop an checking location of packet - cycle 8
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S3);
		EXPECT_EQ(packet->getBuffer(), S3->getInputBuffer(0));

		// Simulation loop an checking location of packet - cycle 9
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), S3);
		EXPECT_EQ(packet->getBuffer(), S3->getOutputBuffer(0));

		// Simulation loop an checking location of packet - cycle 10
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet->getNode(), N1);
		EXPECT_EQ(packet->getBuffer()->getBufferHead(), packet);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

///
/// Test topology
///
///   N0  -->  S0  -->  S1  -->  S2  -->  S3 --> N1
/// Without route the path will be through S2, but with route
/// path will be through S3
TEST(TestSystemConfiguration, event_config_13_ideal_fix_latency)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config_ideal =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 2\n"
			"DefaultOutputBufferSize = 2\n"
			"DefaultBandwidth = 2\n"
			"DefaultPacketSize = 1\n"
			"Ideal = true\n";

	std::string config_fix_latency =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 2\n"
			"DefaultOutputBufferSize = 2\n"
			"DefaultBandwidth = 2\n"
			"DefaultPacketSize = 1\n"
			"FixLatency = 3\n";

	std::string rest = 
			"[Network.net0.Node.N0]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.N1]\n"
			"Type = EndNode\n"
			"[Network.net0.Node.S0]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S1]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S2]\n"
			"Type = Switch\n"
			"[Network.net0.Node.S3]\n"
			"Type = Switch\n"
			"[Network.net0.Link.N0-S0]\n"
			"Type = Unidirectional\n"
			"Source = N0\n"
			"Dest = S0\n"
			"VC = 2\n"
			"[Network.net0.Link.S0-S1]\n"
			"Type = Unidirectional\n"
			"Source = S0\n"
			"Dest = S1\n"
			"[Network.net0.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"[Network.net0.Link.S2-S3]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = S3\n"
			"[Network.net0.Link.S2-N1]\n"
			"Type = Unidirectional\n"
			"Source = S2\n"
			"Dest = N1\n"
			"[Network.net0.Link.S3-N1]\n"
			"Type = Unidirectional\n"
			"Source = S3\n"
			"Dest = N1\n";

	// Set up INI file
	misc::IniFile ini_file;
	std::string config = config_ideal + rest;
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

		// Getting the source, destination and switch nodes
		EndNode *N0 = dynamic_cast<EndNode *>(
				network->getNodeByName("N0"));
		EndNode *N1 = dynamic_cast<EndNode *>(
				network->getNodeByName("N1"));

		// Create a message
		Message *message = network->TrySend(N0, N1, 2);
		Packet *packet_0 = message->getPacket(0);
		Packet *packet_1 = message->getPacket(1);

		// Simulation loop an checking location of packet - cycle 1
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet_0->getNode(), N1);
		EXPECT_EQ(packet_1->getNode(), N1);

		// Receive message
		network->Receive(N1, message);
	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}

	// Cleanup previous instance
	Cleanup();

	// Load the new INI file
	misc::IniFile ini_file_2;
	std::string config_route = config_fix_latency + rest;
	ini_file_2.LoadFromString(config_route);

	// Set up network instance again
	system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body for route
	try
	{
		// Parse the new configuration file
		system->ParseConfiguration(&ini_file_2);

		// Getting the network
		Network *network = system->getNetworkByName("net0");

		// Getting the source, destination and switch nodes
		EndNode *N0 = dynamic_cast<EndNode *>(network->getNodeByName("N0"));
		EndNode *N1 = dynamic_cast<EndNode *>(network->getNodeByName("N1"));

		// Create a message
		Message *message = network->TrySend(N0, N1, 2);
		Packet *packet_0 = message->getPacket(0);
		Packet *packet_1 = message->getPacket(1);

		// Simulation loop an checking location of the packets of 
		// the message. The message is placed in the destination
		// node as soon as its send, however, the receive 
		// is performed by the simulator after fix cycles. 
		esim::Engine *esim_engine = esim::Engine::getInstance();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		esim_engine->ProcessEvents();
		EXPECT_EQ(packet_0->getNode(), N1);
		EXPECT_EQ(packet_1->getNode(), N1);

		// Receive message
		network->Receive(N1, message);

	}
	catch (misc::Error &e)
	{
		e.Dump();
		FAIL();
	}
}

}
