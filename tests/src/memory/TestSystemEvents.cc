/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <regex>

#include <arch/x86/timing/Timing.h>
#include <arch/common/Arch.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <memory/System.h>
#include <network/System.h>

namespace mem
{

const std::string mem_config_0 =
		"[CacheGeometry geo-l1]\n"
		"Sets = 16\n"
		"Assoc = 2\n"
		"BlockSize = 64\n"
		"Latency = 2\n"
		"Policy = LRU\n"
		"Ports = 2\n"
		"\n"
		"[CacheGeometry geo-l2]\n"
		"Sets = 4\n"
		"Assoc = 4\n"
		"BlockSize = 128\n"
		"Latency = 20\n"
		"Policy = LRU\n"
		"Ports = 4\n"
		"\n"
		"; 4 l1 cpu caches\n"
		"\n"
		"[Module mod-l1-0]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net0\n"
		"LowNetworkNode = n0\n"
		"LowModules = mod-l2-0\n"
		"\n"
		"[Module mod-l1-1]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net0\n"
		"LowNetworkNode = n1\n"
		"LowModules = mod-l2-0\n"
		"\n"
		"[Module mod-l1-2]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net1\n"
		"LowNetworkNode = n3\n"
		"LowModules = mod-l2-1\n"
		"\n"
		"[Module mod-l1-3]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net1\n"
		"LowNetworkNode = n4\n"
		"LowModules = mod-l2-1\n"
		"\n"
		"; 1 l2 cpu cache\n"
		"\n"
		"[Module mod-l2-0]\n"
		"Type = Cache\n"
		"Geometry = geo-l2\n"
		"HighNetwork = net0\n"
		"HighNetworkNode = n2\n"
		"LowNetwork = net2\n"
		"LowNetworkNode = n6\n"
		"LowModules = mod-mm\n"
		"\n"
		"[Module mod-l2-1]\n"
		"Type = Cache\n"
		"Geometry = geo-l2\n"
		"HighNetwork = net1\n"
		"HighNetworkNode = n5\n"
		"LowNetwork = net2\n"
		"LowNetworkNode = n7\n"
		"LowModules = mod-mm\n"
		"\n"
		"; 1 mm\n"
		"\n"
		"[Module mod-mm]\n"
		"Type = MainMemory\n"
		"BlockSize = 128\n"
		"Latency = 200\n"
		"HighNetwork = net2\n"
		"HighNetworkNode = n8\n"
		"\n"
		"\n"
		"[Entry core-0]\n"
		"Arch = x86\n"
		"Core = 0\n"
		"Thread = 0\n"
		"DataModule = mod-l1-0\n"
		"InstModule = mod-l1-0\n"
		"\n"
		"[Entry core-1]\n"
		"Arch = x86\n"
		"Core = 1\n"
		"Thread = 0\n"
		"DataModule = mod-l1-1\n"
		"InstModule = mod-l1-1\n"
		"\n"
		"[Entry core-2]\n"
		"Arch = x86\n"
		"Core = 2\n"
		"Thread = 0\n"
		"DataModule = mod-l1-2\n"
		"InstModule = mod-l1-2\n"
		"\n"
		"[Entry core-3]\n"
		"Arch = x86\n"
		"Core = 3\n"
		"Thread = 0\n"
		"DataModule = mod-l1-3\n"
		"InstModule = mod-l1-3\n";

const std::string mem_config_1 =
		"[ CacheGeometry geo-l1 ]\n"
		"Sets = 4\n"
		"Assoc = 1\n"
		"BlockSize = 64\n"
		"Latency = 1\n"
		"Policy = LRU\n"
		"Ports = 2\n"
		"\n"
		"[ CacheGeometry geo-l2 ]\n"
		"Sets = 8\n"
		"Assoc = 1\n"
		"BlockSize = 64\n"
		"Latency = 2\n"
		"Policy = LRU\n"
		"Ports = 4\n"
		"\n"
		"[ CacheGeometry geo-l3 ]\n"
		"Sets = 16\n"
		"Assoc = 1\n"
		"BlockSize = 64\n"
		"Latency = 4\n"
		"Policy = LRU\n"
		"Ports = 8\n"
		"\n"
		"\n"
		"; L1 Caches\n"
		"\n"
		"[ Module mod-l1-0 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net-0\n"
		"LowNetworkNode = n0\n"
		"LowModules = mod-l2-0\n"
		"\n"
		"[ Module mod-l1-1 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net-0\n"
		"LowNetworkNode = n1\n"
		"LowModules = mod-l2-0\n"
		"\n"
		"[ Module mod-l1-2 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net-1\n"
		"LowNetworkNode = n0\n"
		"LowModules = mod-l2-1\n"
		"\n"
		"[ Module mod-l1-3 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l1\n"
		"LowNetwork = net-1\n"
		"LowNetworkNode = n1\n"
		"LowModules = mod-l2-1\n"
		"\n"
		"\n"
		"; L2 Caches\n"
		"\n"
		"[ Module mod-l2-0 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l2\n"
		"HighNetwork = net-0\n"
		"HighNetworkNode = n2\n"
		"LowNetwork = net-2\n"
		"LowNetworkNode = n0\n"
		"LowModules = mod-l3\n"
		"\n"
		"[ Module mod-l2-1 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l2\n"
		"HighNetwork = net-1\n"
		"HighNetworkNode = n2\n"
		"LowNetwork = net-2\n"
		"LowNetworkNode = n1\n"
		"LowModules = mod-l3\n"
		"\n"
		"\n"
		"; L3 Cache\n"
		"\n"
		"[ Module mod-l3 ]\n"
		"Type = Cache\n"
		"Geometry = geo-l3\n"
		"HighNetwork = net-2\n"
		"HighNetworkNode = n2\n"
		"LowNetwork = net-3\n"
		"LowNetworkNode = n0\n"
		"LowModules = mod-mm\n"
		"\n"
		"\n"
		"; Main Memory\n"
		"\n"
		"[ Module mod-mm ]\n"
		"Type = MainMemory\n"
		"BlockSize = 64\n"
		"DirectorySize = 32\n"
		"DirectoryAssoc = 1\n"
		"Latency = 8\n"
		"HighNetwork = net-3\n"
		"HighNetworkNode = n1\n"
		"\n"
		"\n"
		"; Cores\n"
		"\n"
		"[ Entry core-0 ]\n"
		"Arch = x86\n"
		"Core = 0\n"
		"Thread = 0\n"
		"Module = mod-l1-0\n"
		"\n"
		"[ Entry core-1 ]\n"
		"Arch = x86\n"
		"Core = 1\n"
		"Thread = 0\n"
		"Module = mod-l1-1\n"
		"\n"
		"[ Entry core-2 ]\n"
		"Arch = x86\n"
		"Core = 2\n"
		"Thread = 0\n"
		"Module = mod-l1-2\n"
		"\n"
		"[ Entry core-3 ]\n"
		"Arch = x86\n"
		"Core = 3\n"
		"Thread = 0\n"
		"Module = mod-l1-3\n";

const std::string x86_config =
		"[ General ]\n"
		"Cores = 4\n"
		"Threads = 1\n";

const std::string net_config =
		"[ Network.net0 ]\n"
		"DefaultInputBufferSize = 1024\n"
		"DefaultOutputBufferSize = 1024\n"
		"DefaultBandwidth = 256\n"
		"\n"
		"[ Network.net1 ]\n"
		"DefaultInputBufferSize = 1024\n"
		"DefaultOutputBufferSize = 1024\n"
		"DefaultBandwidth = 256\n"
		"\n"
		"[ Network.net2 ]\n"
		"DefaultInputBufferSize = 1024\n"
		"DefaultOutputBufferSize = 1024\n"
		"DefaultBandwidth = 256\n"
		"\n"
		"[ Network.net0.Node.n0 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net0.Node.n1 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net0.Node.n2 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net1.Node.n3 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net1.Node.n4 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net1.Node.n5 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net2.Node.n6 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net2.Node.n7 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net2.Node.n8 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net0.Node.s0 ]\n"
		"Type = Switch \n"
		"\n"
		"[ Network.net1.Node.s1 ]\n"
		"Type = Switch \n"
		"\n"
		"[ Network.net2.Node.s2 ]\n"
		"Type = Switch \n"
		"\n"
		"[ Network.net0.Link.l0 ]\n"
		"Type = Bidirectional\n"
		"Source = n0\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net0.Link.l1 ]\n"
		"Type = Bidirectional\n"
		"Source = n1\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net0.Link.l2 ]\n"
		"Type = Bidirectional\n"
		"Source = n2 \n"
		"Dest = s0\n"
		"\n"
		"[ Network.net1.Link.l3 ]\n"
		"Type = Bidirectional\n"
		"Source = n3 \n"
		"Dest = s1\n"
		"\n"
		"[ Network.net1.Link.l4 ]\n"
		"Type = Bidirectional\n"
		"Source = n4 \n"
		"Dest = s1\n"
		"\n"
		"[ Network.net1.Link.l5 ]\n"
		"Type = Bidirectional\n"
		"Source = n5 \n"
		"Dest = s1\n"
		"\n"
		"[ Network.net2.Link.l6 ]\n"
		"Type = Bidirectional\n"
		"Source = n6 \n"
		"Dest = s2\n"
		"\n"
		"[ Network.net2.Link.l7 ]\n"
		"Type = Bidirectional\n"
		"Source = n7 \n"
		"Dest = s2\n"
		"\n"
		"[ Network.net2.Link.l8 ]\n"
		"Type = Bidirectional\n"
		"Source = n8 \n"
		"Dest = s2\n";

const std::string net_config_1 =
		"[ Network.net-0 ]\n"
		"DefaultInputBufferSize = 256\n"
		"DefaultOutputBufferSize = 256\n"
		"DefaultBandwidth = 64\n"
		"\n"
		"[ Network.net-1 ]\n"
		"DefaultInputBufferSize = 256\n"
		"DefaultOutputBufferSize = 256\n"
		"DefaultBandwidth = 64\n"
		"\n"
		"[ Network.net-2 ]\n"
		"DefaultInputBufferSize = 256\n"
		"DefaultOutputBufferSize = 256\n"
		"DefaultBandwidth = 64\n"
		"\n"
		"[ Network.net-3 ]\n"
		"DefaultInputBufferSize = 256\n"
		"DefaultOutputBufferSize = 256\n"
		"DefaultBandwidth = 64\n"
		"\n"
		"[ Network.net-0.Node.n0 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-0.Node.n1 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-0.Node.n2 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-1.Node.n0 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-1.Node.n1 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-1.Node.n2 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-2.Node.n0 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-2.Node.n1 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-2.Node.n2 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-3.Node.n0 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-3.Node.n1 ]\n"
		"Type = EndNode\n"
		"\n"
		"[ Network.net-0.Node.s0 ]\n"
		"Type = Switch\n"
		"\n"
		"[ Network.net-1.Node.s0 ]\n"
		"Type = Switch\n"
		"\n"
		"[ Network.net-2.Node.s0 ]\n"
		"Type = Switch\n"
		"\n"
		"[ Network.net-3.Node.s0 ]\n"
		"Type = Switch\n"
		"\n"
		"[ Network.net-0.Link.l0 ]\n"
		"Type = Bidirectional\n"
		"Source = n0\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-0.Link.l1 ]\n"
		"Type = Bidirectional\n"
		"Source = n1\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-0.Link.l2 ]\n"
		"Type = Bidirectional\n"
		"Source = n2\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-1.Link.l0 ]\n"
		"Type = Bidirectional\n"
		"Source = n0\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-1.Link.l1 ]\n"
		"Type = Bidirectional\n"
		"Source = n1\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-1.Link.l2 ]\n"
		"Type = Bidirectional\n"
		"Source = n2\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-2.Link.l0 ]\n"
		"Type = Bidirectional\n"
		"Source = n0\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-2.Link.l1 ]\n"
		"Type = Bidirectional\n"
		"Source = n1\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-2.Link.l2 ]\n"
		"Type = Bidirectional\n"
		"Source = n2\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-3.Link.l0 ]\n"
		"Type = Bidirectional\n"
		"Source = n0\n"
		"Dest = s0\n"
		"\n"
		"[ Network.net-3.Link.l1 ]\n"
		"Type = Bidirectional\n"
		"Source = n1\n"
		"Dest = s0\n";

// Cleanup instances of singletons
static void Cleanup()
{
	esim::Engine::Destroy();

	net::System::Destroy();

	System::Destroy();

	x86::Timing::Destroy();

	comm::ArchPool::Destroy();
}

// l1_0 has address 0 in M
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_0)
{
	try
	{
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockModified, 0x0);
		module_l1_1->getCache()->getBlock(1, 0)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, module_l1_0);
		module_l2_0->setOwner(0, 0, 1, module_l1_1);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 1, module_l1_1);
		module_mm->setOwner(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(1, 0, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 0);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), module_l1_1);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 88);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 88);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0 in E 
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_1)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l1_1->getCache()->getBlock(1, 0)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, module_l1_0);
		module_l2_0->setOwner(0, 0, 1, module_l1_1);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 1, module_l1_1);
		module_mm->setOwner(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(1, 0, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers and owner for 0x0 sub-block 0 (previously
		// in l1-0)
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 0);

		// Check sharers/owner for 0x40 (0x0 sub-block 1) currently
		// owned by l1-1
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), module_l1_1);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		// Sent: 8B Rx for 0x400, 8B Rx for 0x800 and 8B for evict
		//     which is removing sharer/owner in lower level
		// Received: 72B for 0x400, 72B for 0x800, and 8B ACK for evict
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 24);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 24);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0 in O
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_2)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockOwned, 0x0);
		module_l1_1->getCache()->getBlock(1, 0)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, module_l1_0);
		module_l2_0->setOwner(0, 0, 1, module_l1_1);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 1, module_l1_1);
		module_mm->setOwner(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(1, 0, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 0);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), module_l1_1);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 88);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 88);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0 in N
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_3)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockNonCoherent, 0x0);
		module_l1_1->getCache()->getBlock(1, 0)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, module_l1_0);
		module_l2_0->setOwner(0, 0, 1, module_l1_1);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 1, module_l1_1);
		module_mm->setOwner(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(1, 0, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 0);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), module_l1_1);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 88);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 88);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 and l1_1 have address 0 in S
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_4)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l1_1->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, nullptr);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 0, module_l1_1);
		module_mm->setOwner(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner -- even though l1-1 now has exclusive access
		// to the data, we won't update the owner state since it adds
		// to the coherency information
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 24);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 24);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 and l1_1 have address 0 in S
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
// This is disturbingly identical to the previous test. Keeping it
// just only since the L2 is in state S
TEST(TestSystemEvents, config_0_evict_5)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l1_1->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, nullptr);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 0, module_l1_1);
		module_mm->setOwner(0, 0, 0, nullptr);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 24);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 24);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 and l1_1 have address 0 in S
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_6)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockNonCoherent, 0x0);
		module_l1_1->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, nullptr);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 0, module_l1_1);
		module_mm->setOwner(0, 0, 0, nullptr);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessLoad, 0x400, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x800, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_1->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 0), module_l1_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 88);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 152);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 88);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 272);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 and l1_1 have address 0 in S
// l1_0 reads address 1024 and 2048 (address 0 gets evicted)
TEST(TestSystemEvents, config_0_evict_7)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockNonCoherent, 0x0);
		module_l1_1->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockShared, 0x0);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockNonCoherent, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, nullptr);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l2_0->setSharer(0, 0, 0, module_l1_1);
		module_mm->setOwner(0, 0, 0, nullptr);
		module_mm->setSharer(0, 0, 0, module_l2_0);

		// Accesses
		int witness = -4;
		module_l1_0->Access(Module::AccessLoad, 0x240, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x440, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x640, &witness);
		module_l1_0->Access(Module::AccessLoad, 0x840, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x440);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(1, 0, tag, state);
		EXPECT_EQ(tag, 0x840);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(9, 1, tag, state);
		EXPECT_EQ(tag, 0x240);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(9, 0, tag, state);
		EXPECT_EQ(tag, 0x640);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockInvalid);

		// Check block
		module_l1_1->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockInvalid);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x800);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x600);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 2, tag, state);
		EXPECT_EQ(tag, 0x400);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x200);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 1, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 1, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 1, 1, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 2, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 2, 1, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 1, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 1), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 1, 1), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 2, 1), module_l1_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 1), module_l1_0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0, subblock 0 in M
// l1_1 has address 0, subblock 1 in E
// l2_0 has address 0 in E
// l1_2 tries to read address 0
TEST(TestSystemEvents, config_0_load_0)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_2 = memory_system->getModule("mod-l1-2");
		Module *module_l1_3 = memory_system->getModule("mod-l1-3");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l1_2, nullptr);
		ASSERT_NE(module_l1_3, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l2_1, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l1_1->getCache()->getBlock(1, 1)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 1, module_l1_1);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setSharer(0, 3, 1, module_l1_1);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_2->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_1->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_2->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_1->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 2);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_1), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), nullptr);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 16);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 136);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 144);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 144);

		// Check link
		node = module_l2_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_l2_1->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_1->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l1_2->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_2->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_3->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_3->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_1 performs a load on a value that is exclusive in l1_0
TEST(TestSystemEvents, config_0_load_1)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_1->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_1->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 2);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 16);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_1 performs a load on a value that is modified in l1_0
TEST(TestSystemEvents, config_0_load_2)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockModified, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_1->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_1->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 2);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 loads a value from empty hierarchy
TEST(TestSystemEvents, config_0_load_3)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 15, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 15, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 15, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 15, 0), module_l2_0);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 136);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_1 performs a load on a value that is modified in MM
TEST(TestSystemEvents, config_0_load_4)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockModified, 0x0);
		module_mm->setOwner(0, 7, 0, nullptr);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 136);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_1 has address 0x0, 0x40 in M
// l1_0 reads address 0
TEST(TestSystemEvents, config_0_load_5)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l2_1, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set block states
		module_l1_1->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockModified, 0x0);
		module_l1_1->getCache()->getBlock(1, 1)->setStateTag(Cache::BlockModified, 0x40);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 3, 0, module_l1_1);
		module_l2_0->setOwner(0, 3, 1, module_l1_1);
		module_l2_0->setSharer(0, 3, 0, module_l1_1);
		module_l2_0->setSharer(0, 3, 1, module_l1_1);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_1->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_1->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 2);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 1, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 16);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 144);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 88);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 152);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// Perform a load on a value that is invalid in l1_0, exclusive in l2_0
TEST(TestSystemEvents, config_0_load_6)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set Block States
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_2 performs a load on a value that is exclusive in l1_0
TEST(TestSystemEvents, config_0_load_7)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l1_2 = memory_system->getModule("mod-l1-2");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l1_2, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l2_1, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_2->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l1_2->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_1->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_1->isSharer(0, 3, 0, module_l1_2), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 2);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_1), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), nullptr);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 performs a load on a value that is owned in l2_0
TEST(TestSystemEvents, config_0_load_8)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockOwned, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockOwned);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 performs a load on a value that is shared in l2_0
TEST(TestSystemEvents, config_0_load_9)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockShared, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessLoad, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), nullptr);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_2 performs a load on a subblock of a block that another subblock 
// of that block is in exclusive state in l1_0
TEST(TestSystemEvents, config_0_load_10)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_2 = memory_system->getModule("mod-l1-2");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_2, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l2_1, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States L1
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(
				Cache::BlockExclusive, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(
				Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(
				Cache::BlockExclusive, 0x0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_2->Access(Module::AccessLoad, 0x40, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block l1_0
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block l2_0
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check block mm
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 2);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_1), true);
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), nullptr);

		// Check block l1_2
		module_l1_2->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockShared);

		//Check block l2_1
		module_l2_1->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 0), 0);
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_1->isSharer(0, 3, 1, module_l1_2), true);
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 00), nullptr);
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 1), nullptr);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

// l1_0 nc stores in address 0x0
TEST(TestSystemEvents, config_0_ncstore_0)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 15, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 15, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 15, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 15, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 136);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 and l1_1 have address 0x0 in S
// l1_0 attempts NC Store
TEST(TestSystemEvents, config_0_ncstore_1)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockShared, 0x0);
		module_l1_1->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockShared, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setSharer(0, 3, 0, module_l1_1);
		module_l2_0->setOwner(0, 3, 0, nullptr);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l1_1->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 2);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0x0 in E
// l1_0 attempts NC Store
TEST(TestSystemEvents, config_0_ncstore_2)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
        	EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// L1-3 has addressess 0x0 in E
// l1_0 attempts NC Store
TEST(TestSystemEvents, config_0_ncstore_3)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l1_3 = memory_system->getModule("mod-l1-3");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l1_3, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l2_1, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_3->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_1->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_1->setSharer(0, 3, 0, module_l1_3);
		module_l2_1->setOwner(0, 3, 0, module_l1_3);
		module_mm->getCache()->getBlock(0, 15)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setSharer(0, 15, 0, module_l2_1);
		module_mm->setOwner(0, 15, 0, module_l2_1);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l1_3->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_l2_1->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block
		module_mm->getCache()->getBlock(0, 15, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 15, 0), 2);
		EXPECT_EQ(module_mm->isSharer(0, 15, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->isSharer(0, 15, 0, module_l2_1), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 15, 0), nullptr);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check sharers
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_1->isSharer(0, 3, 0, module_l1_3), true);

		// Check owner
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 0), nullptr);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l1_3->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_3->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_l2_1->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_1->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_1->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 144);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 16);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

// l1_0 has address 0x0 in O
// l1_0 attempts NC Store
TEST(TestSystemEvents, config_0_ncstore_4)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockOwned, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 has address 0x0 in M
// l1_0 attempts NC Store
TEST(TestSystemEvents, config_0_ncstore_5)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockModified, 0x0);
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessNCStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockNonCoherent);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 80);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 80);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l1_0 stores in address 0x0
TEST(TestSystemEvents, config_0_store_0)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 15, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 15, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 15, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 15, 0), module_l2_0);

		// Check link
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 72);

		// Check link
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check link
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 136);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getSentBytes(), 136);

		// Check link
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


// l2_0 has 0x0 in owned; l1_0 stores in address 0x0
TEST(TestSystemEvents, config_0_store_1)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockOwned, 0x0);
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setOwner(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_0->Access(Module::AccessStore, 0x0, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);

		// Check sharers
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);

		// Check owner
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


TEST(TestSystemEvents, config_0_store_2)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l1_2 = memory_system->getModule("mod-l1-2");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l2_1 = memory_system->getModule("mod-l2-1");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set L1 Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(
				Cache::BlockShared, 0x0);
		module_l1_2->getCache()->getBlock(0, 1)->setStateTag(
				Cache::BlockShared, 0x0);

		// Set L2 Block States and sharer/owner
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(
				Cache::BlockShared, 0x0);
		module_l2_1->getCache()->getBlock(0, 3)->setStateTag(
				Cache::BlockShared, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_1->setSharer(0, 3, 0, module_l1_2);
		
		// Set MM block states and sharer/owner
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setSharer(0, 7, 0, module_l2_1);

		// Accesses
		int witness = -1;
		module_l1_1->Access(Module::AccessStore, 0x48, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block L1_0
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockShared);

		// Check block l1_1
		module_l1_1->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block l1_2
		module_l1_2->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockInvalid);

		// Check L2-0 state, owners and sharers 
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 1, module_l1_1), true);
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 1), module_l1_1);
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), nullptr);

		// Check L2-1 state, owners and sharers
		module_l2_1->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockInvalid);
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 1), 0);
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 1), nullptr);
		EXPECT_EQ(module_l2_1->getNumSharers(0, 3, 0), 0);
		EXPECT_EQ(module_l2_1->getOwner(0, 3, 0), nullptr);

		// Check MM state, owners and sharers
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// There should be no communication with l1_0 or any other
		// sharers
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check L1-1 communications. Send a write-request, and 
		// receives a block
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check L1-2 communications. Receives an INV
		// and sends an ACK
		node = module_l1_2->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check L2-0 communications. It receives a write-request from
		// L1-1, and sends the write-request to MM.
		// Receives an ACK from MM indicating the hierarchy is updated
		// and send the requested data to L1-1.
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 72);
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check L2-1 communications. Receives an INV from MM, and 
		// forwards it to L1-2. Receives an ACK from L1-2 and forwards
		// it to MM
		node = module_l2_1->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 8);
		node = module_l2_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check MM communications. It receives a write-request from
		// L2-0, invalidates the L2-1 and acknowledges the invalidation
		// of the L2-1 to L2-0
		node = module_mm->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 16);
		EXPECT_EQ(node->getSentBytes(), 16);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}


TEST(TestSystemEvents, config_0_store_3)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_0);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_mm, nullptr);

		// Set L1 Block States
		module_l1_0->getCache()->getBlock(0, 1)->setStateTag(
				Cache::BlockExclusive, 0x0);

		// Set L2 Block States and sharer/owner
		module_l2_0->getCache()->getBlock(0, 3)->setStateTag(
				Cache::BlockExclusive, 0x0);
		module_l2_0->setSharer(0, 3, 0, module_l1_0);
		module_l2_0->setOwner(0, 3, 0, module_l1_0);
		
		// Set MM block states and sharer/owner
		module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->setSharer(0, 7, 0, module_l2_0);
		module_mm->setOwner(0, 7, 0, module_l2_0);

		// Accesses
		int witness = -1;
		module_l1_1->Access(Module::AccessStore, 0x40, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check l1_0
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 1, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check l1_1
		module_l1_1->getCache()->getBlock(1, 1, tag, state);
		EXPECT_EQ(tag, 0x40);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check L2-0 state, owners and sharers 
		module_l2_0->getCache()->getBlock(0, 3, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 0, module_l1_0), true);
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 0), module_l1_0);
		EXPECT_EQ(module_l2_0->getNumSharers(0, 3, 1), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 3, 1, module_l1_1), true);
		EXPECT_EQ(module_l2_0->getOwner(0, 3, 1), module_l1_1);

		// Check MM state, owners and sharers
		module_mm->getCache()->getBlock(0, 7, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);
		EXPECT_EQ(module_mm->getNumSharers(0, 7, 0), 1);
		EXPECT_EQ(module_mm->isSharer(0, 7, 0, module_l2_0), true);
		EXPECT_EQ(module_mm->getOwner(0, 7, 0), module_l2_0);

		// Check communications from L1_0
		net::Node *node = module_l1_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
		EXPECT_EQ(node->getSentBytes(), 0);

		// Check communications from L1_1
		node = module_l1_1->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 72);
		EXPECT_EQ(node->getSentBytes(), 8);

		// Check communications from L2_0
		node = module_l2_0->getHighNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 8);
		EXPECT_EQ(node->getSentBytes(), 72);
		node = module_l2_0->getLowNetworkNode();
		EXPECT_EQ(node->getReceivedBytes(), 0);
		EXPECT_EQ(node->getSentBytes(), 0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

// l1_0, l2_0, l3_0, and mm have address 0 in E
// Cycle 1 - l1_0 writes address 0 (block in l1_0 turns M)
// Cycle 2 - l1_1 reads address 0x200 (conflict in l1_0 and l2_0, but not in l3)
TEST(TestSystemEvents, config_1_evict_0)
{
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem_1;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net_1;
		ini_file_mem_1.LoadFromString(mem_config_1);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net_1.LoadFromString(net_config_1);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net_1);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem_1);

		// Get modules
		Module *module_l1_0 = memory_system->getModule("mod-l1-0");
		Module *module_l1_1 = memory_system->getModule("mod-l1-1");
		Module *module_l2_0 = memory_system->getModule("mod-l2-0");
		Module *module_l3 = memory_system->getModule("mod-l3");
		Module *module_mm = memory_system->getModule("mod-mm");
		ASSERT_NE(module_l1_0, nullptr);
		ASSERT_NE(module_l1_1, nullptr);
		ASSERT_NE(module_l2_0, nullptr);
		ASSERT_NE(module_l3, nullptr);
		ASSERT_NE(module_mm, nullptr);

		//Set Block States
		module_l1_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l3->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_mm->getCache()->getBlock(0, 0)->setStateTag(Cache::BlockExclusive, 0x0);
		module_l2_0->setOwner(0, 0, 0, module_l1_0);
		module_l3->setOwner(0, 0, 0, module_l2_0);
		module_mm->setOwner(0, 0, 0, module_l3);
		module_l2_0->setSharer(0, 0, 0, module_l1_0);
		module_l3->setSharer(0, 0, 0, module_l2_0);
		module_mm->setSharer(0, 0, 0, module_l3);

		// Accesses
		int witness = -2;
		module_l1_0->Access(Module::AccessStore, 0x0, &witness);
		module_l1_1->Access(Module::AccessLoad, 0x200, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
			esim_engine->ProcessEvents();

		// Check block
		unsigned tag;
		Cache::BlockState state;
		module_l1_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockInvalid);

		// Check block
		module_l1_1->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x200);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l2_0->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x200);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_l3->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockModified);

		// Check block
		module_l3->getCache()->getBlock(8, 0, tag, state);
		EXPECT_EQ(tag, 0x200);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(0, 0, tag, state);
		EXPECT_EQ(tag, 0x0);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check block
		module_mm->getCache()->getBlock(8, 0, tag, state);
		EXPECT_EQ(tag, 0x200);
		EXPECT_EQ(state, Cache::BlockExclusive);

		// Check sharers
		EXPECT_EQ(module_l2_0->getNumSharers(0, 0, 0), 1);
		EXPECT_EQ(module_l2_0->isSharer(0, 0, 0, module_l1_1), true);

		// Check sharers
		EXPECT_EQ(module_l3->getNumSharers(0, 0, 0), 0);

		// Check sharers
		EXPECT_EQ(module_l3->getNumSharers(8, 0, 0), 1);
		EXPECT_EQ(module_l3->isSharer(8, 0, 0, module_l2_0), true);

		// Check owner
		EXPECT_EQ(module_l2_0->getOwner(0, 0, 0), module_l1_1);

		// Check owner
		EXPECT_EQ(module_l3->getOwner(0, 0, 0), nullptr);

		// Check owner
		EXPECT_EQ(module_l3->getOwner(8, 0, 0), module_l2_0);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

/* TODO: This test will be added again when the support for the test
is provided in the multi2sim 
TEST(TestSystemEvents, test_flood)
{
	static std::string mem_config_flood =
			"[ CacheGeometry geo-l1 ]\n"
			"Sets = 1\n"
			"Assoc = 1\n"
			"BlockSize = 64\n"
			"Latency = 1\n"
			"MSHR = 256\n"
			"Ports = 1\n"
			"\n"
			"[ Module mod-l1 ]\n"
			"Type = Cache\n"
			"Geometry = geo-l1\n"
			"LowNetwork = net0\n"
			"LowNetworkNode = n0\n"
			"LowModules = mod-l2\n"
			"\n"
			"[ Module mod-l2 ]\n"
			"Type = MainMemory\n"
			"HighNetwork = net0\n"
			"HighNetworkNode = n1\n"
			"BlockSize = 256\n"
			"Latency = 10\n"
			"\n"
			"[ Entry core-0 ]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"DataModule = mod-l1\n"
			"InstModule = mod-l1\n"
			"\n"
			"[ Entry core-1 ]\n"
			"Arch = x86\n"
			"Core = 1\n"
			"Thread = 0\n"
			"DataModule = mod-l1\n"
			"InstModule = mod-l1\n"
			"\n"
			"[ Entry core-2 ]\n"
			"Arch = x86\n"
			"Core = 2\n"
			"Thread = 0\n"
			"DataModule = mod-l1\n"
			"InstModule = mod-l1\n"
			"\n"
			"[ Entry core-3 ]\n"
			"Arch = x86\n"
			"Core = 3\n"
			"Thread = 0\n"
			"DataModule = mod-l1\n"
			"InstModule = mod-l1\n";

	const std::string net_config_flood =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 10\n"
			"DefaultOutputBufferSize = 10\n"
			"DefaultBandwidth = 72\n"
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
			"[ Network.net0.Link.l0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"Bandwidth = 72\n"
			"\n"
			"[ Network.net0.Link.l1 ]\n"
			"Type = Bidirectional\n"
			"Source = s0\n"
			"Dest = n1\n"
			"Bandwidth = 72\n";

	std::string message = "place";
	try
	{
		// Cleanup singleton instances
		Cleanup();

		// Load configuration files
		misc::IniFile ini_file_mem;
		misc::IniFile ini_file_x86;
		misc::IniFile ini_file_net;
		ini_file_mem.LoadFromString(mem_config_flood);
		ini_file_x86.LoadFromString(x86_config);
		ini_file_net.LoadFromString(net_config_flood);

		// Set up x86 timing simulator
		x86::Timing::ParseConfiguration(&ini_file_x86);
		x86::Timing::getInstance();

		// Set up network system
		net::System *network_system = net::System::getInstance();
		network_system->ParseConfiguration(&ini_file_net);

		// Set up memory system
		System *memory_system = System::getInstance();
		memory_system->ReadConfiguration(&ini_file_mem);

		// Get modules
		Module *module_l1 = memory_system->getModule("mod-l1");
		Module *module_l2 = memory_system->getModule("mod-l2");
		ASSERT_NE(module_l1, nullptr);
		ASSERT_NE(module_l2, nullptr);

		// Set block states
		module_l1->getCache()->getBlock(0, 0)->
			      setStateTag(Cache::BlockInvalid, 0x0);
		module_l2->getCache()->getBlock(0, 0)->
			      setStateTag(Cache::BlockExclusive, 0x0);
		module_l2->setOwner(0, 0, 0, module_l1);
		module_l2->setSharer(0, 0, 0, module_l1);

		// Spam the main memory with accesses
		int witness = -3;
		module_l1->Access(Module::AccessLoad, 0x040, &witness);
		module_l1->Access(Module::AccessLoad, 0x050, &witness);
		module_l1->Access(Module::AccessLoad, 0x060, &witness);

		// Simulation loop
		esim::Engine *esim_engine = esim::Engine::getInstance();
		while (witness < 0)
		{
			esim_engine->ProcessEvents();
		}
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	esim::Engine *esim_engine = esim::Engine::getInstance();
	EXPECT_TRUE(esim_engine->getCycle() == 5);
	EXPECT_REGEX_MATCH(misc::fmt("Violation in number of outstanding"
				"memory accesses").c_str(), message.c_str());
}
*/
// TODO: Add find_and_lock, find_and_lock_port, find_and_lock_action, and
// find_and_lock_finish tests.

}








