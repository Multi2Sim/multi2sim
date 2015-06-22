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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Event.h>


namespace esim
{

///
/// Test 0
///

// Initialize event handlers
bool handler_called_0 = false;

void testHandler_0(Event *event, Frame *frame)
{
	handler_called_0 = true;
}

TEST(TestEngine, should_be_able_to_schedule_event)
{
	// Set up esim engine
	Engine *engine = Engine::getInstance();
	FrequencyDomain domain("Test frequency domain", 2e3);
	Event event("test event", testHandler_0, &domain);
	engine->RegisterFrequencyDomain("Test frequency domain", 2e3);

	// Schedule next event in next 10 events
	engine->Next(&event, 10, 0);

	// Process events should go to next cycle, and the scheduled
	// event should not be triggered
	long long cycle_before = engine->getCycle();
	engine->ProcessEvents();
	long long cycle_after = engine->getCycle();
	EXPECT_EQ(1, cycle_after - cycle_before);
	EXPECT_FALSE(handler_called_0);

	// Skip for 10 cycles, the handler should be invoked
	for (int i = 0; i < 10; i++)
		engine->ProcessEvents();
	cycle_after = engine->getCycle();
	EXPECT_EQ(11, cycle_after - cycle_before);
	EXPECT_TRUE(handler_called_0);
}




//
// Test 1
//

// Create dummy frame
class DummyFrame_1 : public Frame
{
public:
	int counter = 0;
};

// Create test handler
void testHandler_1(Event *event, Frame *frame)
{
	DummyFrame_1 *data = dynamic_cast<DummyFrame_1 *>(frame);
	data->counter++;
}

TEST(TestEngine, should_able_to_schedule_event_with_event_frame)
{
	// Set frame
	auto frame = misc::new_shared<DummyFrame_1>();

	// Set up esim engine
	Engine *engine = Engine::getInstance();

	// Reset engine
	engine->Reset();

	//Set up frequency domain
	FrequencyDomain domain("Test frequency domain", 2e3);
	Event event("test event", testHandler_1, &domain);
	engine->RegisterFrequencyDomain("Test frequency domain", 2e3);

	// Schedule next event
	engine->Schedule(&event, frame, 0);

	// Process event
	engine->ProcessEvents();

	// Assertions
	EXPECT_EQ(1, frame->counter);
}




//
// Test 2
//

// Initialize event handler variables
bool handler_called_2_0 = false;
bool handler_called_2_1 = false;

// Initialize event handler
void testHandler_2_0(Event *fast_event, Frame *frame)
{
	handler_called_2_0 = true;
	std::cout << "Handler_2_0 called\n";
}

// Initialize event handler
void testHandler_2_1(Event *slow_event, Frame *frame)
{
	handler_called_2_1 = true;
}

// Tests if events scheduled in two frequency domains are executed correctly
//
// Cycle Distribution
//
// fast_domain   --    --    --    --    --
//
// slow_domain       --        --        --
//
//
TEST(TestEngine, test_frequency_domains)
{
	// Set up esim engine
	Engine *engine = Engine::getInstance();

	// Reset engine
	engine->Reset();

	// Set up fast frequency domain
	FrequencyDomain *fast_domain = engine->RegisterFrequencyDomain(
			"Fast frequency domain", 1000);

	// Set up slow frequency domain
	FrequencyDomain *slow_domain = engine->RegisterFrequencyDomain(
			"Slow frequency domain", 600);

	// Set up event
	Event fast_event("fast event", testHandler_2_0, fast_domain);

	// Set up event
	Event slow_event("slow event", testHandler_2_1, slow_domain);

	// Schedule fast event for 100 fast cycles from now
	engine->Next(&fast_event, 100, 0);

	// Schedule slow event for 100 slow cycles from now
	engine->Next(&slow_event, 100, 0);

	// Run simulation
	for (int i = 0; i < 101; i++)
	{
		// Check that handler has not been called
		EXPECT_FALSE(handler_called_2_0);

		engine->ProcessEvents();
	}

	// Continue simulation
	for (int i = 0; i < 67; i++)
	{
		// Check that handler has been called
		EXPECT_TRUE(handler_called_2_0);

		// Check that handler has not been called
		EXPECT_FALSE(handler_called_2_1);

		engine->ProcessEvents();
	}

	//Check that handler has been called
	EXPECT_TRUE(handler_called_2_1);
}

}




