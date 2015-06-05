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

TEST(TestSystemCoherenceProtocol, config_0_evict_0)
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

/*
Command[10] = Access mod-l1-0 1 LOAD 0x400
Command[11] = Access mod-l1-0 1 LOAD 0x800
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

}

