/*
 *  Multi2Sim
 *  Copyright (C) 2015  Shi Dong (dong.sh@husky.neu.edu)
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
#include <lib/cpp/Error.h>
#include <memory/System.h>
#include <memory/Manager.h>
#include <arch/x86/disassembler/Disassembler.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/timing/Timing.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>

namespace x86 
{

static void Cleanup() {
	Timing::Destroy();
	Emulator::Destroy();
	mem::System::Destroy();
	comm::ArchPool::Destroy();
}

TEST(TestX86TimingFetchStage, simple_fetch)
{


	// CPU configuration file
	std::string config_string =
			"[ General ]\n"
			"[ TraceCache ]\n"
			"Present = f";
	misc::IniFile config_ini;
	config_ini.LoadFromString(config_string);
	try
	{
		Timing::ParseConfiguration(&config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading x86 configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}

	// Get instance of Timing
	Emulator *emulator = Emulator::getInstance();
	Timing::getInstance();

	// Memory configuration file
	std::string mem_config_string =
			"[ General ]\n"
			"[ Module mod-mm ]\n"
			"Type = MainMemory\n"
			"Latency = 50\n"
			"BlockSize = 64\n"
			"[ Entry core-1 ]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"Module = mod-mm\n";
	misc::IniFile mem_config_ini;
	mem_config_ini.LoadFromString(mem_config_string);

	// Configure
	try
	{
		mem::System::getInstance()->ReadConfiguration(&mem_config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading mem configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}


	// Code to execute
	// mov eax, ebx
	unsigned char code[] = {
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8
	};

	// Create a context]
	Context *context = emulator->newContext();
	context->Initialize();
	auto mmu = misc::new_shared<mem::Mmu>("mmu");
	context->mmu = mmu;
	mem::Mmu::Space space("space", mmu.get());
	context->mmu_space = &space;
	mem::Memory *memory = context->getMemory();
	memory->setHeapBreak(misc::RoundUp(memory->getHeapBreak(),
				mem::Memory::PageSize));

	// Allocate memory and save the instructions into memory
	mem::Manager manager(memory);
	unsigned eip = manager.Allocate(sizeof(code), 128);
	memory->Write(eip, sizeof(code), (const char *)code);

	// Update context status, including eip
	context->setUinstActive(true);
	context->setState(Context::StateRunning);
	context->getRegs().setEip(eip);

	// Map the thread onto cpu hardware
	Cpu *cpu = Timing::getInstance()->getCpu();
	Thread *thread = cpu->getThread(0, 0);
	thread->MapContext(context);
	thread->Schedule();

	// Fetch
	thread->setFetchNeip(eip);
	//timing->Run();
	thread->Fetch();
	EXPECT_EQ(32, thread->getFetchQueueSize());
	EXPECT_EQ(64, thread->getFetchQueueOccupency());

	// After fetching,
	esim::Engine *engine = esim::Engine::getInstance();
	for (int i = 0; i < 51; i++)
	{
		EXPECT_EQ(32, thread->getFetchQueueSize());
		EXPECT_EQ(64, thread->getFetchQueueOccupency());

		// Process events
		// thread->Decode();
		// 	Why the result is different if this line
		// 	is behind engine->ProcessEvents();?
		engine->ProcessEvents();
		thread->Decode();
		//timing->Run();
	}

	// Decoding
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(28, thread->getFetchQueueSize());
	EXPECT_EQ(56, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(24, thread->getFetchQueueSize());
	EXPECT_EQ(48, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(20, thread->getFetchQueueSize());
	EXPECT_EQ(40, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(16, thread->getFetchQueueSize());
	EXPECT_EQ(32, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(12, thread->getFetchQueueSize());
	EXPECT_EQ(24, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(8, thread->getFetchQueueSize());
	EXPECT_EQ(16, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(4, thread->getFetchQueueSize());
	EXPECT_EQ(8, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(0, thread->getFetchQueueSize());
	EXPECT_EQ(0, thread->getFetchQueueOccupency());

	Cleanup();
}


TEST(TestX86TimingFetchStage, fetch_same_block)
{


	// CPU configuration file
	std::string config_string =
			"[ General ]\n"
			"[ TraceCache ]\n"
			"Present = f";
	misc::IniFile config_ini;
	config_ini.LoadFromString(config_string);
	try
	{
		Timing::ParseConfiguration(&config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading x86 configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}

	// Get instance of Timing
	Emulator *emulator = Emulator::getInstance();
	Timing::getInstance();

	// Memory configuration file
	std::string mem_config_string =
			"[ General ]\n"
			"[ Module mod-mm ]\n"
			"Type = MainMemory\n"
			"Latency = 50\n"
			"BlockSize = 64\n"
			"[ Entry core-1 ]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"Module = mod-mm\n";
	misc::IniFile mem_config_ini;
	mem_config_ini.LoadFromString(mem_config_string);

	// Configure
	try
	{
		mem::System::getInstance()->ReadConfiguration(&mem_config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading mem configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}


	// Code to execute
	// mov eax, ebx
	unsigned char code[] = {
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0xEB, 0xF0, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8
	};

	// Create a context]
	Context *context = emulator->newContext();
	context->Initialize();
	auto mmu = misc::new_shared<mem::Mmu>("mmu");
	context->mmu = mmu;
	mem::Mmu::Space space("space", mmu.get());
	context->mmu_space = &space;
	mem::Memory *memory = context->getMemory();
	memory->setHeapBreak(misc::RoundUp(memory->getHeapBreak(),
				mem::Memory::PageSize));

	// Allocate memory and save the instructions into memory
	mem::Manager manager(memory);
	unsigned eip = manager.Allocate(sizeof(code), 128);
	memory->Write(eip, sizeof(code), (const char *)code);

	// Update context status, including eip
	context->setUinstActive(true);
	context->setState(Context::StateRunning);
	context->getRegs().setEip(eip);

	// Map the thread onto cpu hardware
	Cpu *cpu = Timing::getInstance()->getCpu();
	Thread *thread = cpu->getThread(0, 0);
	thread->MapContext(context);
	thread->Schedule();

	// Fetch
	thread->setFetchNeip(eip);
	//timing->Run();
	thread->Fetch();
	EXPECT_EQ(32, thread->getFetchQueueSize());
	EXPECT_EQ(64, thread->getFetchQueueOccupency());

	// After fetching,
	esim::Engine *engine = esim::Engine::getInstance();
	for (int i = 0; i < 51; i++)
	{
		EXPECT_EQ(32, thread->getFetchQueueSize());
		EXPECT_EQ(64, thread->getFetchQueueOccupency());

		// Process events
		// thread->Decode();
		// 	Why the result is different if this line
		// 	is behind engine->ProcessEvents();?
		engine->ProcessEvents();
		thread->Decode();
		//timing->Run();
	}

	// Decoding
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(28, thread->getFetchQueueSize());
	EXPECT_EQ(56, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(24, thread->getFetchQueueSize());
	EXPECT_EQ(48, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(20, thread->getFetchQueueSize());
	EXPECT_EQ(40, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(16, thread->getFetchQueueSize());
	EXPECT_EQ(32, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(12, thread->getFetchQueueSize());
	EXPECT_EQ(24, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(8, thread->getFetchQueueSize());
	EXPECT_EQ(16, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(4, thread->getFetchQueueSize());
	EXPECT_EQ(8, thread->getFetchQueueOccupency());
	engine->ProcessEvents();
	thread->Decode();
	EXPECT_EQ(0, thread->getFetchQueueSize());
	EXPECT_EQ(0, thread->getFetchQueueOccupency());

	// Never fetch again
	for (int i = 0; i < 51; i++)
	{
		EXPECT_EQ(0, thread->getFetchQueueSize());
		EXPECT_EQ(0, thread->getFetchQueueOccupency());

		// Process events
		// thread->Decode();
		// 	Why the result is different if this line
		// 	is behind engine->ProcessEvents();?
		engine->ProcessEvents();
		thread->Decode();
		//timing->Run();
	}

	Cleanup();
}

TEST(TestX86TimingFetchStage, fetch_another_block)
{


	// CPU configuration file
	std::string config_string =
			"[ General ]\n"
			"[ TraceCache ]\n"
			"Present = f";
	misc::IniFile config_ini;
	config_ini.LoadFromString(config_string);
	try
	{
		Timing::ParseConfiguration(&config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading x86 configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}

	// Get instance of Timing
	Emulator *emulator = Emulator::getInstance();
	Timing *timing = Timing::getInstance();

	// Memory configuration file
	std::string mem_config_string =
			"[ General ]\n"
			"[ Module mod-mm ]\n"
			"Type = MainMemory\n"
			"Latency = 50\n"
			"BlockSize = 64\n"
			"[ Entry core-1 ]\n"
			"Arch = x86\n"
			"Core = 0\n"
			"Thread = 0\n"
			"Module = mod-mm\n";
	misc::IniFile mem_config_ini;
	mem_config_ini.LoadFromString(mem_config_string);

	// Configure
	try
	{
		mem::System::getInstance()->ReadConfiguration(&mem_config_ini);
	}
	catch (misc::Exception &e)
	{
		std::cerr << "Exception in reading mem configuration" <<
				e.getMessage() << "\n";
		ASSERT_TRUE(false);
	}


	// Code to execute
	// mov eax, ebx
	unsigned char code[] = {
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0xeb, 0xc0, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8,
		0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8, 0x89, 0xD8
	};

	// Create a context]
	Context *context = emulator->newContext();
	context->Initialize();
	auto mmu = misc::new_shared<mem::Mmu>("mmu");
	context->mmu = mmu;
	mem::Mmu::Space space("space", mmu.get());
	context->mmu_space = &space;
	mem::Memory *memory = context->getMemory();
	memory->setHeapBreak(misc::RoundUp(memory->getHeapBreak(),
				mem::Memory::PageSize));

	// Allocate memory and save the instructions into memory
	mem::Manager manager(memory);
	unsigned eip = manager.Allocate(sizeof(code), 128);
	memory->Write(eip, sizeof(code), (const char *)code);

	// Update context status, including eip
	context->setUinstActive(true);
	context->setState(Context::StateRunning);
	context->getRegs().setEip(eip);

	// Map the thread onto cpu hardware
	Cpu *cpu = Timing::getInstance()->getCpu();
	Thread *thread = cpu->getThread(0, 0);
	thread->MapContext(context);
	thread->Schedule();

	// Fetch
	thread->setFetchNeip(eip);

	int expected_fetch_queue_size[] = {
		0,
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 10
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 20
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 30
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 40
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 50
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 60
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 70
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 80
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 90
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 100
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 110
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 120
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 130
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 140
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 150
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 160
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 170
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 180
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 190
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 200
		32, 32, 32, 32, 32, 32, 32, 32, 44, 56, // 210
		68, 80, 92, 104, 0, 0, 0, 0, 0, 0, // 220
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 230
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 240
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 250
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 260
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 270
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 280
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 290
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 300
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 310
		19, 19, 28, 24, 20, 16, 13, 13, 13, 13, // 320
	};



	// After fetching,
	esim::Engine *engine = esim::Engine::getInstance();
	for (int i = 0; i < 530; i++)
	{
		printf("Cycle: %d, uops: %d, bytes: %d\n", i,
			thread->getFetchQueueSize(),
			thread->getFetchQueueOccupency());
		EXPECT_EQ(expected_fetch_queue_size[i], thread->getFetchQueueSize());
		//EXPECT_EQ(64, thread->getFetchQueueOccupency());

		// Process events
		// thread->Decode();
		// 	Why the result is different if this line
		// 	is behind engine->ProcessEvents();?
		engine->ProcessEvents();
		//thread->Decode();
		timing->Run();
	}

	Cleanup();
}

}  // namespace x86
