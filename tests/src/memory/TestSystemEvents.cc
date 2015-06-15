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

#include <arch/x86/timing/Timing.h>
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
		"/n"
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

TEST(TestSystemEvents, config_0_evict_0)
{
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

/*
[Commands]
Command[16] = CheckBlock mod-l2-0 0 3 0x400 E
Command[17] = CheckBlock mod-l2-0 0 2 0x800 E
Command[18] = CheckSharers mod-l2-0 0 0 0 None
Command[19] = CheckSharers mod-l2-0 0 0 1 mod-l1-1
Command[20] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckOwner mod-l2-0 0 0 0 None
Command[23] = CheckOwner mod-l2-0 0 0 1 mod-l1-1
Command[24] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[25] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[26] = CheckLink mod-l1-0 Low Out 88
Command[27] = CheckLink mod-l1-0 Low In 152
Command[28] = CheckLink mod-l1-1 Low Out 0
Command[29] = CheckLink mod-l1-1 Low In 0
Command[30] = CheckLink mod-l2-0 High Out 152
Command[31] = CheckLink mod-l2-0 High In 88
Command[32] = CheckLink mod-l2-0 Low Out 16
Command[33] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_1)
{
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
/*
 [Commands]
Command[12] = CheckBlock mod-l1-0 0 1 0x400 E
Command[13] = CheckBlock mod-l1-0 0 0 0x800 E
Command[14] = CheckBlock mod-l1-1 1 0 0x40 M
Command[15] = CheckBlock mod-l2-0 0 0 0x0 E
Command[16] = CheckBlock mod-l2-0 0 3 0x400 E
Command[17] = CheckBlock mod-l2-0 0 2 0x800 E
Command[18] = CheckSharers mod-l2-0 0 0 0 mod-l1-0
Command[19] = CheckSharers mod-l2-0 0 0 1 mod-l1-1
Command[20] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckOwner mod-l2-0 0 0 0 mod-l1-0
Command[23] = CheckOwner mod-l2-0 0 0 1 mod-l1-1
Command[24] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[25] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[26] = CheckLink mod-l1-0 Low Out 16
Command[27] = CheckLink mod-l1-0 Low In 144
Command[28] = CheckLink mod-l1-1 Low Out 0
Command[29] = CheckLink mod-l1-1 Low In 0
Command[30] = CheckLink mod-l2-0 High Out 144
Command[31] = CheckLink mod-l2-0 High In 16
Command[32] = CheckLink mod-l2-0 Low Out 16
Command[33] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_2)
{
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
/*

[Commands]
Command[12] = CheckBlock mod-l1-0 0 1 0x400 E
Command[13] = CheckBlock mod-l1-0 0 0 0x800 E
Command[14] = CheckBlock mod-l1-1 1 0 0x40 M
Command[15] = CheckBlock mod-l2-0 0 0 0x0 M
Command[16] = CheckBlock mod-l2-0 0 3 0x400 E
Command[17] = CheckBlock mod-l2-0 0 2 0x800 E
Command[18] = CheckSharers mod-l2-0 0 0 0 None
Command[19] = CheckSharers mod-l2-0 0 0 1 mod-l1-1
Command[20] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckOwner mod-l2-0 0 0 0 None
Command[23] = CheckOwner mod-l2-0 0 0 1 mod-l1-1
Command[24] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[25] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[26] = CheckLink mod-l1-0 Low Out 88
Command[27] = CheckLink mod-l1-0 Low In 152
Command[28] = CheckLink mod-l1-1 Low Out 0
Command[29] = CheckLink mod-l1-1 Low In 0
Command[30] = CheckLink mod-l2-0 High Out 152
Command[31] = CheckLink mod-l2-0 High In 88
Command[32] = CheckLink mod-l2-0 Low Out 16
Command[33] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_3)
{
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
/*
[Commands]
Command[12] = CheckBlock mod-l1-0 0 1 0x400 E
Command[13] = CheckBlock mod-l1-0 0 0 0x800 E
Command[14] = CheckBlock mod-l1-1 1 0 0x40 M
Command[15] = CheckBlock mod-l2-0 0 0 0x0 M
Command[16] = CheckBlock mod-l2-0 0 3 0x400 E
Command[17] = CheckBlock mod-l2-0 0 2 0x800 E
Command[18] = CheckSharers mod-l2-0 0 0 0 None
Command[19] = CheckSharers mod-l2-0 0 0 1 mod-l1-1
Command[20] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckOwner mod-l2-0 0 0 0 None
Command[23] = CheckOwner mod-l2-0 0 0 1 mod-l1-1
Command[24] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[25] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[26] = CheckLink mod-l1-0 Low Out 88
Command[27] = CheckLink mod-l1-0 Low In 152
Command[28] = CheckLink mod-l1-1 Low Out 0
Command[29] = CheckLink mod-l1-1 Low In 0
Command[30] = CheckLink mod-l2-0 High Out 152
Command[31] = CheckLink mod-l2-0 High In 88
Command[32] = CheckLink mod-l2-0 Low Out 16
Command[33] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_4)
{
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
/*
[Commands]
Command[10] = CheckBlock mod-l1-0 0 1 0x400 E
Command[11] = CheckBlock mod-l1-0 0 0 0x800 E
Command[12] = CheckBlock mod-l1-1 0 0 0x0 S
Command[13] = CheckBlock mod-l2-0 0 0 0x0 E
Command[14] = CheckBlock mod-l2-0 0 3 0x400 E
Command[15] = CheckBlock mod-l2-0 0 2 0x800 E
Command[16] = CheckSharers mod-l2-0 0 0 0 mod-l1-0 mod-l1-1
Command[17] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[18] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[19] = CheckOwner mod-l2-0 0 0 0 None
Command[20] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckLink mod-l1-0 Low Out 16
Command[23] = CheckLink mod-l1-0 Low In 144
Command[24] = CheckLink mod-l1-1 Low Out 0
Command[25] = CheckLink mod-l1-1 Low In 0
Command[26] = CheckLink mod-l2-0 High Out 144
Command[27] = CheckLink mod-l2-0 High In 16
Command[28] = CheckLink mod-l2-0 Low Out 16
Command[29] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_5)
{
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
/*
[Commands]
Command[10] = CheckBlock mod-l1-0 0 1 0x400 E
Command[11] = CheckBlock mod-l1-0 0 0 0x800 E
Command[12] = CheckBlock mod-l1-1 0 0 0x0 S
Command[13] = CheckBlock mod-l2-0 0 0 0x0 S
Command[14] = CheckBlock mod-l2-0 0 3 0x400 E
Command[15] = CheckBlock mod-l2-0 0 2 0x800 E
Command[16] = CheckSharers mod-l2-0 0 0 0 mod-l1-0 mod-l1-1
Command[17] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[18] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[19] = CheckOwner mod-l2-0 0 0 1 None
Command[20] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckLink mod-l1-0 Low Out 16
Command[23] = CheckLink mod-l1-0 Low In 144
Command[24] = CheckLink mod-l1-1 Low Out 0
Command[25] = CheckLink mod-l1-1 Low In 0
Command[26] = CheckLink mod-l2-0 High Out 144
Command[27] = CheckLink mod-l2-0 High In 16
Command[28] = CheckLink mod-l2-0 Low Out 16
Command[29] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_6)
{
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
/*
Command[10] = CheckBlock mod-l1-0 0 1 0x400 E
Command[11] = CheckBlock mod-l1-0 0 0 0x800 E
Command[12] = CheckBlock mod-l1-1 0 0 0x0 S
Command[13] = CheckBlock mod-l2-0 0 0 0x0 N
Command[14] = CheckBlock mod-l2-0 0 3 0x400 E
Command[15] = CheckBlock mod-l2-0 0 2 0x800 E
Command[16] = CheckSharers mod-l2-0 0 0 0 mod-l1-1
Command[17] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[18] = CheckSharers mod-l2-0 0 2 0 mod-l1-0
Command[19] = CheckOwner mod-l2-0 0 0 1 None
Command[20] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[21] = CheckOwner mod-l2-0 0 2 0 mod-l1-0
Command[22] = CheckLink mod-l1-0 Low Out 88
Command[23] = CheckLink mod-l1-0 Low In 152
Command[24] = CheckLink mod-l1-1 Low Out 0
Command[25] = CheckLink mod-l1-1 Low In 0
Command[26] = CheckLink mod-l2-0 High Out 152
Command[27] = CheckLink mod-l2-0 High In 88
Command[28] = CheckLink mod-l2-0 Low Out 16
Command[29] = CheckLink mod-l2-0 Low In 272
*/
}

TEST(TestSystemEvents, config_0_evict_7)
{
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
/*
Command[12] = CheckBlock mod-l1-0 1 1 0x440 E
Command[13] = CheckBlock mod-l1-0 1 0 0x840 E
Command[14] = CheckBlock mod-l1-0 9 1 0x240 E
Command[15] = CheckBlock mod-l1-0 9 0 0x640 E
Command[16] = CheckBlock mod-l1-0 0 0 0x0 I
Command[17] = CheckBlock mod-l1-1 0 0 0x0 I
Command[18] = CheckBlock mod-l2-0 0 0 0x800 E
Command[19] = CheckBlock mod-l2-0 0 1 0x600 E
Command[20] = CheckBlock mod-l2-0 0 2 0x400 E
Command[21] = CheckBlock mod-l2-0 0 3 0x200 E
Command[22] = CheckSharers mod-l2-0 0 0 1 mod-l1-0
Command[23] = CheckSharers mod-l2-0 0 1 1 mod-l1-0
Command[24] = CheckSharers mod-l2-0 0 2 1 mod-l1-0
Command[25] = CheckSharers mod-l2-0 0 3 1 mod-l1-0
Command[26] = CheckOwner mod-l2-0 0 0 1 mod-l1-0
Command[27] = CheckOwner mod-l2-0 0 1 1 mod-l1-0
Command[28] = CheckOwner mod-l2-0 0 2 1 mod-l1-0
Command[29] = CheckOwner mod-l2-0 0 3 1 mod-l1-0
*/
}

TEST(TestSystemEvents, config_0_load_0)
{
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
	Module *module_l1_1 = memory_system->getModule("mod-l1-1");
	Module *module_l2_0 = memory_system->getModule("mod-l2-0");
	Module *module_mm = memory_system->getModule("mod-mm");
	ASSERT_NE(module_l1_0, nullptr);
	ASSERT_NE(module_l1_1, nullptr);
	ASSERT_NE(module_l1_2, nullptr);
	ASSERT_NE(module_l2_0, nullptr);
	ASSERT_NE(module_mm, nullptr);

	// Set block states
	module_l1_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockExclusive, 0x0);
	module_l1_1->getCache()->getBlock(1, 1)->setStateTag(Cache::BlockModified, 0x40);
	module_l2_0->getCache()->getBlock(0, 3)->setStateTag(Cache::BlockExclusive, 0x0);
	module_mm->getCache()->getBlock(0, 7)->setStateTag(Cache::BlockExclusive, 0x0);
	module_l2_0->setOwner(0, 3, 0, module_l1_0);
	module_l2_0->setOwner(0, 3, 1, module_l1_0);
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
/*
Command[11] = CheckBlock mod-l1-0 0 1 0x0 S
Command[12] = CheckBlock mod-l1-1 1 1 0x40 S
Command[13] = CheckBlock mod-l1-2 0 1 0x0 S
Command[14] = CheckBlock mod-l2-0 0 3 0x0 S
Command[15] = CheckBlock mod-l2-1 0 3 0x0 S
Command[16] = CheckBlock mod-mm 0 7 0x0 E
Command[17] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[18] = CheckSharers mod-l2-0 0 3 1 mod-l1-1
Command[19] = CheckSharers mod-mm 0 7 0 mod-l2-0 mod-l2-1
Command[20] = CheckOwner mod-l2-0 0 3 0 None
Command[21] = CheckOwner mod-l2-1 0 3 0 None
Command[22] = CheckOwner mod-mm 0 7 0 None
Command[23] = CheckLink mod-l1-0 Low Out 8
Command[24] = CheckLink mod-l1-0 Low In 8
Command[25] = CheckLink mod-l1-1 Low Out 72
Command[26] = CheckLink mod-l1-1 Low In 8
Command[27] = CheckLink mod-l2-0 High Out 16
Command[28] = CheckLink mod-l2-0 High In 80
Command[29] = CheckLink mod-l2-0 Low Out 136
Command[30] = CheckLink mod-l2-0 Low In 8
Command[31] = CheckLink mod-mm High Out 144
Command[32] = CheckLink mod-mm High In 144
Command[33] = CheckLink mod-l2-1 Low Out 8
Command[34] = CheckLink mod-l2-1 Low In 136
Command[35] = CheckLink mod-l2-1 High Out 72
Command[36] = CheckLink mod-l2-1 High In 8
Command[37] = CheckLink mod-l1-2 Low Out 8
Command[38] = CheckLink mod-l1-2 Low In 72
Command[39] = CheckLink mod-l1-3 Low Out 0
Command[40] = CheckLink mod-l1-3 Low In 0
*/
}

TEST(TestSystemEvents, config_0_load_1)
{
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
/*
Command[8] = CheckBlock mod-l1-0 0 1 0x0 S
Command[9] = CheckBlock mod-l1-1 0 1 0x0 S
Command[10] = CheckBlock mod-l2-0 0 3 0x0 E
Command[11] = CheckBlock mod-mm 0 7 0x0 E
Command[12] = CheckSharers mod-l2-0 0 3 0 mod-l1-0 mod-l1-1
Command[13] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[14] = CheckOwner mod-l2-0 0 3 0 None
Command[15] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[16] = CheckLink mod-l1-0 Low Out 8
Command[17] = CheckLink mod-l1-0 Low In 8
Command[18] = CheckLink mod-l1-1 Low Out 8
Command[19] = CheckLink mod-l1-1 Low In 72
Command[20] = CheckLink mod-l2-0 High Out 80
Command[21] = CheckLink mod-l2-0 High In 16
Command[22] = CheckLink mod-l2-0 Low Out 0
Command[23] = CheckLink mod-l2-0 Low In 0
Command[24] = CheckLink mod-mm High Out 0
Command[25] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_load_2)
{
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
/*
Command[7] = Access mod-l1-1 1 LOAD 0x0
Command[8] = CheckBlock mod-l1-0 0 1 0x0 S
Command[9] = CheckBlock mod-l1-1 0 1 0x0 S
Command[10] = CheckBlock mod-l2-0 0 3 0x0 E
Command[11] = CheckBlock mod-mm 0 7 0x0 E
Command[12] = CheckSharers mod-l2-0 0 3 0 mod-l1-0 mod-l1-1
Command[13] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[14] = CheckOwner mod-l2-0 0 3 0 None
Command[15] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[16] = CheckLink mod-l1-0 Low Out 72
Command[17] = CheckLink mod-l1-0 Low In 8
Command[18] = CheckLink mod-l1-1 Low Out 8
Command[19] = CheckLink mod-l1-1 Low In 72
Command[20] = CheckLink mod-l2-0 High Out 80
Command[21] = CheckLink mod-l2-0 High In 80
Command[22] = CheckLink mod-l2-0 Low Out 0
Command[23] = CheckLink mod-l2-0 Low In 0
Command[24] = CheckLink mod-mm High Out 0
Command[25] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_load_3)
{
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
/*
Command[1] = CheckBlock mod-l1-0 0 1 0x0 E
Command[2] = CheckBlock mod-l2-0 0 3 0x0 E
Command[3] = CheckBlock mod-mm 0 15 0x0 E
Command[4] = CheckSharers mod-mm 0 15 0 mod-l2-0
Command[5] = CheckOwner mod-mm 0 15 0 mod-l2-0
Command[6] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[7] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[8] = CheckLink mod-l1-0 Low In 72
Command[9] = CheckLink mod-l1-0 Low Out 8
Command[10] = CheckLink mod-l1-1 Low In 0
Command[11] = CheckLink mod-l1-1 Low Out 0
Command[12] = CheckLink mod-l2-0 High In 8
Command[13] = CheckLink mod-l2-0 High Out 72
Command[14] = CheckLink mod-l2-0 Low In 136
Command[15] = CheckLink mod-l2-0 Low Out 8
Command[16] = CheckLink mod-mm High In 8
Command[17] = CheckLink mod-mm High Out 136
*/
}

TEST(TestSystemEvents, config_0_load_4)
{
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
/*
Command[4] = CheckBlock mod-l1-0 0 1 0x0 E
Command[5] = CheckBlock mod-l2-0 0 3 0x0 E
Command[6] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[7] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[8] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[9] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[10] = CheckBlock mod-mm 0 7 0x0 M
Command[11] = CheckLink mod-l1-0 Low In 72
Command[12] = CheckLink mod-l1-0 Low Out 8
Command[13] = CheckLink mod-l1-1 Low In 0
Command[14] = CheckLink mod-l1-1 Low Out 0
Command[15] = CheckLink mod-l2-0 High In 8
Command[16] = CheckLink mod-l2-0 High Out 72
Command[17] = CheckLink mod-l2-0 Low In 136
Command[18] = CheckLink mod-l2-0 Low Out 8
Command[19] = CheckLink mod-mm High In 8
Command[20] = CheckLink mod-mm High Out 136
*/
}

TEST(TestSystemEvents, config_0_load_5)
{
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
/*
Command[11] = CheckBlock mod-l1-1 0 1 0x0 S
Command[12] = CheckBlock mod-l1-1 1 1 0x40 S
Command[13] = CheckBlock mod-l1-0 0 1 0x0 S
Command[14] = CheckBlock mod-l2-0 0 3 0x0 E
Command[15] = CheckBlock mod-mm 0 7 0x0 E
Command[16] = CheckSharers mod-l2-0 0 3 0 mod-l1-0 mod-l1-1
Command[17] = CheckSharers mod-l2-0 0 3 1 mod-l1-1
Command[18] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[19] = CheckOwner mod-l2-0 0 3 0 None
Command[20] = CheckOwner mod-l2-1 0 3 0 None
Command[21] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[22] = CheckLink mod-l1-0 Low In 72
Command[23] = CheckLink mod-l1-0 Low Out 8
Command[24] = CheckLink mod-l1-1 Low In 16
Command[25] = CheckLink mod-l1-1 Low Out 144
Command[26] = CheckLink mod-l2-0 High Out 88
Command[27] = CheckLink mod-l2-0 High In 152
Command[28] = CheckLink mod-l2-0 Low Out 0
Command[29] = CheckLink mod-l2-0 Low In 0
Command[30] = CheckLink mod-mm High Out 0
Command[31] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_load_6)
{
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

/*
Command[5] = CheckBlock mod-l1-0 0 1 0x0 E
Command[6] = CheckBlock mod-l2-0 0 3 0x0 E
Command[7] = CheckBlock mod-mm 0 7 0x0 E
Command[8] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[9] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[10] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[11] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[12] = CheckLink mod-l1-0 Low In 72
Command[13] = CheckLink mod-l1-0 Low Out 8
Command[14] = CheckLink mod-l1-1 Low In 0
Command[15] = CheckLink mod-l1-1 Low Out 0
Command[16] = CheckLink mod-l2-0 High In 8
Command[17] = CheckLink mod-l2-0 High Out 72
Command[18] = CheckLink mod-l2-0 Low In 0
Command[19] = CheckLink mod-l2-0 Low Out 0
*/
}

TEST(TestSystemEvents, config_0_load_7)
{
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
	Module *module_l1_2 = memory_system->getModule("mod-l1-1");
	Module *module_l2_0 = memory_system->getModule("mod-l2-0");
	Module *module_mm = memory_system->getModule("mod-mm");
	ASSERT_NE(module_l1_0, nullptr);
	ASSERT_NE(module_l1_1, nullptr);
	ASSERT_NE(module_l1_2, nullptr);
	ASSERT_NE(module_l2_0, nullptr);
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

/*
Command[7] = Access mod-l1-2 1 LOAD 0x0
Command[8] = CheckBlock mod-l1-0 0 1 0x0 S
Command[9] = CheckBlock mod-l1-2 0 1 0x0 S
Command[10] = CheckBlock mod-l2-0 0 3 0x0 S
Command[11] = CheckBlock mod-l2-1 0 3 0x0 S
Command[12] = CheckBlock mod-mm 0 7 0x0 E
Command[13] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[14] = CheckSharers mod-l2-1 0 3 0 mod-l1-2
Command[15] = CheckSharers mod-mm 0 7 0 mod-l2-0 mod-l2-1
Command[16] = CheckOwner mod-l2-0 0 3 0 None
Command[17] = CheckOwner mod-l2-1 0 3 0 None
Command[18] = CheckOwner mod-mm 0 7 0 None
*/
}

TEST(TestSystemEvents, config_0_load_8)
{
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

/*
Command[5] = CheckBlock mod-l1-0 0 1 0x0 S
Command[6] = CheckBlock mod-l2-0 0 3 0x0 O
Command[7] = CheckBlock mod-mm 0 7 0x0 E
Command[8] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[9] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[10] = CheckOwner mod-l2-0 0 3 0 None
Command[11] = CheckOwner mod-mm 0 7 0 mod-l2-0
*/
}

TEST(TestSystemEvents, config_0_load_9)
{
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

/*
Command[4] = CheckBlock mod-l1-0 0 1 0x0 S
Command[5] = CheckBlock mod-l2-0 0 3 0x0 S
Command[6] = CheckBlock mod-mm 0 7 0x0 E
Command[7] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[8] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[9] = CheckOwner mod-l2-0 0 3 0 None
Command[10] = CheckOwner mod-mm 0 7 0 None
*/
}

TEST(TestSystemEvents, config_0_ncstore_0)
{
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

/*
Command[1] = CheckBlock mod-l1-0 0 1 0x0 N
Command[2] = CheckBlock mod-l2-0 0 3 0x0 E
Command[3] = CheckBlock mod-mm 0 15 0x0 E
Command[4] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[5] = CheckSharers mod-mm 0 15 0 mod-l2-0
Command[6] = CheckOwner mod-l2-0 0 3 0 None
Command[7] = CheckOwner mod-mm 0 15 0 mod-l2-0
Command[8] = CheckLink mod-l1-0 Low Out 8
Command[9] = CheckLink mod-l1-0 Low In 72
Command[10] = CheckLink mod-l2-0 High Out 72
Command[11] = CheckLink mod-l2-0 High In 8
Command[12] = CheckLink mod-l2-0 Low Out 8
Command[13] = CheckLink mod-l2-0 Low In 136
Command[14] = CheckLink mod-mm High Out 136
Command[15] = CheckLink mod-mm High In 8
*/

}

TEST(TestSystemEvents, config_0_ncstore_1)
{
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

/*
Command[9] = CheckBlock mod-l1-0 0 1 0x0 N
Command[10] = CheckBlock mod-l1-1 0 1 0x0 S
Command[11] = CheckBlock mod-l2-0 0 3 0x0 E
Command[12] = CheckBlock mod-mm 0 7 0x0 E
Command[13] = CheckSharers mod-l2-0 0 3 0 mod-l1-0 mod-l1-1
Command[14] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[15] = CheckOwner mod-l2-0 0 3 0 None
Command[16] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[17] = CheckLink mod-l1-0 Low Out 0
Command[18] = CheckLink mod-l1-0 Low In 0
Command[19] = CheckLink mod-l2-0 High Out 0
Command[20] = CheckLink mod-l2-0 High In 0
Command[21] = CheckLink mod-l2-0 Low Out 0
Command[22] = CheckLink mod-l2-0 Low In 0
Command[23] = CheckLink mod-mm High Out 0
Command[24] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_ncstore_2)
{
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

/*
Command[8] = CheckBlock mod-l1-0 0 1 0x0 N
Command[9] = CheckBlock mod-l2-0 0 3 0x0 E
Command[10] = CheckBlock mod-mm 0 7 0x0 E
Command[11] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[12] = CheckOwner mod-l2-0 0 3 0 None
Command[13] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[14] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[15] = CheckLink mod-l1-0 Low Out 8
Command[16] = CheckLink mod-l1-0 Low In 8
Command[17] = CheckLink mod-l2-0 High Out 8
Command[18] = CheckLink mod-l2-0 High In 8
Command[19] = CheckLink mod-l2-0 Low Out 0
Command[20] = CheckLink mod-l2-0 Low In 0
Command[21] = CheckLink mod-mm High Out 0
Command[22] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_ncstore_3)
{
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
	Module *module_l2_1 = memory_system->getModule("mod-l2-0");
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

/*
Command[8] = CheckBlock mod-l1-0 0 1 0x0 N
Command[9] = CheckBlock mod-l1-3 0 1 0x0 S
Command[10] = CheckBlock mod-l2-0 0 3 0x0 S
Command[11] = CheckBlock mod-l2-1 0 3 0x0 S
Command[12] = CheckBlock mod-mm 0 15 0x0 E
Command[13] = CheckSharers mod-mm 0 15 0 mod-l2-0 mod-l2-1
Command[14] = CheckOwner mod-mm 0 15 0 None
Command[15] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[16] = CheckOwner mod-l2-0 0 3 0 None
Command[17] = CheckSharers mod-l2-1 0 3 0 mod-l1-3
Command[18] = CheckOwner mod-l2-1 0 3 0 None
Command[19] = CheckLink mod-l1-0 Low Out 8
Command[20] = CheckLink mod-l1-0 Low In 72
Command[21] = CheckLink mod-l1-3 Low Out 8
Command[22] = CheckLink mod-l1-3 Low In 8
Command[23] = CheckLink mod-l2-0 High Out 72
Command[24] = CheckLink mod-l2-0 High In 8
Command[25] = CheckLink mod-l2-0 Low Out 8
Command[26] = CheckLink mod-l2-0 Low In 136
Command[27] = CheckLink mod-l2-1 High Out 8
Command[28] = CheckLink mod-l2-1 High In 8
Command[29] = CheckLink mod-l2-1 Low Out 8
Command[30] = CheckLink mod-l2-1 Low In 8
Command[31] = CheckLink mod-mm High Out 144
Command[32] = CheckLink mod-mm High In 16
*/
}

TEST(TestSystemEvents, config_0_ncstore_4)
{
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

/*
Command[8] = CheckBlock mod-l1-0 0 1 0x0 N
Command[9] = CheckBlock mod-l2-0 0 3 0x0 M
Command[10] = CheckBlock mod-mm 0 7 0x0 E
Command[11] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[12] = CheckOwner mod-l2-0 0 3 0 None
Command[13] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[14] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[15] = CheckLink mod-l1-0 Low Out 80
Command[16] = CheckLink mod-l1-0 Low In 80
Command[17] = CheckLink mod-l2-0 High Out 80
Command[18] = CheckLink mod-l2-0 High In 80
Command[19] = CheckLink mod-l2-0 Low Out 0
Command[20] = CheckLink mod-l2-0 Low In 0
Command[21] = CheckLink mod-mm High Out 0
Command[22] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_ncstore_5)
{
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
	module_l1_0->Access(Module::AccessNCStore, 0x400, &witness);

	// Simulation loop
	esim::Engine *esim_engine = esim::Engine::getInstance();
	while (witness < 0)
		esim_engine->ProcessEvents();

/*
Command[7] = Access mod-l1-0 1 NCStore 0x0
Command[8] = CheckBlock mod-l1-0 0 1 0x0 N
Command[9] = CheckBlock mod-l2-0 0 3 0x0 M
Command[10] = CheckBlock mod-mm 0 7 0x0 E
Command[11] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[12] = CheckOwner mod-l2-0 0 3 0 None
Command[13] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[14] = CheckOwner mod-mm 0 7 0 mod-l2-0
Command[15] = CheckLink mod-l1-0 Low Out 80
Command[16] = CheckLink mod-l1-0 Low In 80
Command[17] = CheckLink mod-l2-0 High Out 80
Command[18] = CheckLink mod-l2-0 High In 80
Command[19] = CheckLink mod-l2-0 Low Out 0
Command[20] = CheckLink mod-l2-0 Low In 0
Command[21] = CheckLink mod-mm High Out 0
Command[22] = CheckLink mod-mm High In 0
*/
}

TEST(TestSystemEvents, config_0_store_0)
{
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


	// Accesses
	int witness = -1;
	module_l1_0->Access(Module::AccessStore, 0x0, &witness);

	// Simulation loop
	esim::Engine *esim_engine = esim::Engine::getInstance();
	while (witness < 0)
		esim_engine->ProcessEvents();

/*
Command[1] = CheckBlock mod-l1-0 0 1 0x0 M
Command[2] = CheckBlock mod-l2-0 0 3 0x0 E
Command[3] = CheckBlock mod-mm 0 15 0x0 E
Command[4] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[5] = CheckSharers mod-mm 0 15 0 mod-l2-0
Command[6] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[7] = CheckOwner mod-mm 0 15 0 mod-l2-0
Command[8] = CheckLink mod-l1-0 Low Out 8
Command[9] = CheckLink mod-l1-0 Low In 72
Command[10] = CheckLink mod-l2-0 High Out 72
Command[11] = CheckLink mod-l2-0 High In 8
Command[12] = CheckLink mod-l2-0 Low Out 8
Command[13] = CheckLink mod-l2-0 Low In 136
Command[14] = CheckLink mod-mm High Out 136
Command[15] = CheckLink mod-mm High In 8
*/
}

TEST(TestSystemEvents, config_0_store_1)
{
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
	module_l2_0->getCache()->getBlock(0, 1)->setStateTag(Cache::BlockOwned, 0x0);
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

/*
Command[5] = CheckBlock mod-l1-0 0 1 0x0 M
Command[6] = CheckBlock mod-l2-0 0 3 0x0 E
Command[7] = CheckBlock mod-mm 0 7 0x0 E
Command[8] = CheckSharers mod-l2-0 0 3 0 mod-l1-0
Command[9] = CheckSharers mod-mm 0 7 0 mod-l2-0
Command[10] = CheckOwner mod-l2-0 0 3 0 mod-l1-0
Command[11] = CheckOwner mod-mm 0 7 0 mod-l2-0
*/
}

TEST(TestSystemEvents, config_1_evict_0)
{
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

/*
Command[12] = CheckBlock mod-l1-0 0 0 0x0 I
Command[13] = CheckBlock mod-l1-1 0 0 0x200 E
Command[14] = CheckBlock mod-l2-0 0 0 0x200 E
Command[15] = CheckBlock mod-l3 0 0 0x0 M
Command[16] = CheckBlock mod-l3 8 0 0x200 E
Command[17] = CheckBlock mod-mm 0 0 0x0 E
Command[18] = CheckBlock mod-mm 8 0 0x200 E
Command[19] = CheckSharers mod-l2-0 0 0 0 mod-l1-1
Command[20] = CheckSharers mod-l3 0 0 0 None
Command[21] = CheckSharers mod-l3 8 0 0 mod-l2-0
Command[22] = CheckOwner mod-l2-0 0 0 0 mod-l1-1
Command[23] = CheckOwner mod-l3 0 0 0 None
Command[24] = CheckOwner mod-l3 8 0 0 mod-l2-0
*/
}

}




