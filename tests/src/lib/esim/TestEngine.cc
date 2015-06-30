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
#include <lib/esim/Queue.h>


namespace esim
{

// Cleanup pointer to singleton instance
static void Cleanup() 
{ 
	Engine::Destroy(); 
}

///
/// Test 0
///

// Initialize event handlers
bool handler_called_0 = false;

void testHandler_0(Event *event, Frame *frame)
{
	handler_called_0 = true;
}

// Tests the ability to schedule an event and execute it at the correct time
TEST(TestEngine, test_schedule_event)
{
	try
	{
		// Set up esim engine
		Engine *engine = Engine::getInstance();

		// Set up frequency domain
		FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"Test frequency domain", 2e3);

		// Register event
		Event *event = engine->RegisterEvent(
				"test event", testHandler_0, domain);
	
		// Schedule event for 10 cycles from now
		engine->Next(event, 10, 0);

		// Get current cycle before
		long long cycle_before = engine->getCycle();

		// Run simulation for 1 cycle
		engine->ProcessEvents();

		// Get current cycle after
		long long cycle_after = engine->getCycle();

		// Check that only 1 cycle was executed
		EXPECT_EQ(1, cycle_after - cycle_before);

		// Check that handler was not called
		EXPECT_FALSE(handler_called_0);

		// Get current cycle before
		cycle_before = engine->getCycle();
	
		// Run simulation for 10 cycles
		for (int i = 0; i < 10; i++)
			engine->ProcessEvents();

		// Get current cycle after
		cycle_after = engine->getCycle();

		// Check that 10 cycles were executed
		EXPECT_EQ(10, cycle_after - cycle_before);
	
		// Check that handler was called
		EXPECT_TRUE(handler_called_0);
	}

	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
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

// Tests the ability to schedule an event in an event frame
TEST(TestEngine, test_event_frame)
{
	try
	{
		// Cleanup pointers to singleton instances
		Cleanup();

		// Set frame
		auto frame = misc::new_shared<DummyFrame_1>();

		// Set up esim engine
		Engine *engine = Engine::getInstance();

		// Set up frequency domain
		FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"Test frequency domain", 2e3);

		// Register event
		Event *event = engine->RegisterEvent(
				"test event", testHandler_1, domain);

		// Schedule next event for 0 cycles from now
		engine->Call(event, frame, nullptr, 0, 0);

		// Run simulation for 1 cycle
		engine->ProcessEvents();

		// Check that handler was executed
		EXPECT_EQ(1, frame->counter);

	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
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
// fast_domain   --    --    --    --    --	(1GHz)
//
// slow_domain       --        --        --     (600MHz)
//
//
TEST(TestEngine, test_frequency_domains)
{
	try
	{
		// Cleanup pointers to singleton instances
		Cleanup();

		// Set up esim engine
		Engine *engine = Engine::getInstance();

		// Set up fast frequency domain
		FrequencyDomain *fast_domain = engine->RegisterFrequencyDomain(
				"Fast frequency domain", 1000);

		// Set up slow frequency domain
		FrequencyDomain *slow_domain = engine->RegisterFrequencyDomain(
				"Slow frequency domain", 600);

		// Set up event
		Event *fast_event = engine->RegisterEvent("fast event", testHandler_2_0, fast_domain);

		// Set up event
		Event *slow_event = engine->RegisterEvent("slow event", testHandler_2_1, slow_domain);

		// Schedule fast event for 100 fast cycles from now
		engine->Next(fast_event, 100, 0);

		// Schedule slow event for 100 slow cycles from now
		engine->Next(slow_event, 100, 0);

		// Run simulation for 100 cycles
		for (int i = 0; i < 101; i++)
		{
			// Check that handler has not been called
			EXPECT_FALSE(handler_called_2_0);

			engine->ProcessEvents();
		}

		// Continue simulation for 66 cycles
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
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}




//
// Test 3
//

// Create queue
Queue q_3;

// Create dummy frame
class DummyFrame_3_0 : public Frame{};

// Create dummy frame
class DummyFrame_3_1 : public Frame{};

// Set up handler variables
bool handler_called_3_1 = false;
bool handler_called_3_3 = false;

// Initialize event handler
void testHandler_3_1(Event *event, Frame *frame)
{
	handler_called_3_1 = true;
	EXPECT_FALSE(handler_called_3_3);
}

// Initialize event handler
void testHandler_3_0(Event *event, Frame *frame)
{
	// Get instance of engine
	Engine *engine = Engine::getInstance();
	
	// Set up frequency domain
	FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"frequency domain", 1000);

	// Register event
	Event *event3 = engine->RegisterEvent("event 4", testHandler_3_1, domain);

	// Add event to the queue
	q_3.Wait(event3);
}

// Initialize event handler
void testHandler_3_3(Event *event, Frame *frame)
{
	handler_called_3_3 = true;
}

// Initialize event handler
void testHandler_3_2(Event *event, Frame *frame)
{
	// Get instance of engine
	Engine *engine = Engine::getInstance();

	// Set up frequency domain
	FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"frequency domain", 1000);

	// Register event
	Event *event4 = engine->RegisterEvent("event 3", testHandler_3_3, domain);

	// Add event to the queue
	q_3.Wait(event4);
}

// Tests to see if events in the queue are executed correctly
TEST(TestEngine, test_event_queue)
{
	try
	{
		// Cleanup pointers to singleton instances
		Cleanup();

		// Set up esim engine
		Engine *engine = Engine::getInstance();

		// Set up fast frequency domain
		FrequencyDomain *domain = engine->RegisterFrequencyDomain(
					"frequency domain", 1000);

		// Set up event
		Event *event1 = engine->RegisterEvent("event 1", testHandler_3_0, domain);

		// Set up event
		Event *event2 = engine->RegisterEvent("event 2", testHandler_3_2, domain);

		// Set frame
		auto frame_3_0 = misc::new_shared<DummyFrame_3_0>();

		// Set frame
		auto frame_3_1 = misc::new_shared<DummyFrame_3_1>();

		// Schedule event for 5 cycles from now
		engine->Call(event1, frame_3_0, nullptr, 5, 0);

		// Schedule slow event 10 cycles from now
		engine->Call(event2, frame_3_1, nullptr, 10, 0);

		// Run simulation for 10 cycles
		for (int i = 0; i < 11; i++)
			engine->ProcessEvents();

		// Check that the handler has not been called
		EXPECT_FALSE(handler_called_3_1);
		EXPECT_FALSE(handler_called_3_3);

		// Wakeup all events in queue
		q_3.WakeupAll();

		// Run simulation for 1 cycle
		engine->ProcessEvents();

		// Check that handler has been called
		EXPECT_TRUE(handler_called_3_1);

		//Check that handler has been called
		EXPECT_TRUE(handler_called_3_3);
	
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}




//
// Test 4
//

// Create queue
Queue q_4;

// Create dummy frame
class DummyFrame_4_0 : public Frame{};

// Create dummy frame
class DummyFrame_4_1 : public Frame{};

// Set up handler variables
bool handler_called_4_1 = false;
bool handler_called_4_3 = false;

// Initialize event handler
void testHandler_4_1(Event *event, Frame *frame)
{
	handler_called_4_1 = true;
}

// Initialize event handler
void testHandler_4_0(Event *event, Frame *frame)
{
	// Get instance of engine
	Engine *engine = Engine::getInstance();

	// Set up frequency domain
	FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"frequency domain", 1000);

	// Register event
	Event *event3 = engine->RegisterEvent("event 4", testHandler_4_1, domain);

	// Add event to the queue
	q_4.Wait(event3);
}

// Initialize event handler
void testHandler_4_3(Event *event, Frame *frame)
{
	handler_called_4_3 = true;
	EXPECT_FALSE(handler_called_4_1);
}

// Initialize event handler
void testHandler_4_2(Event *event, Frame *frame)
{
	// Get instance of engine
	Engine *engine = Engine::getInstance();

	// Set up frequency domain
	FrequencyDomain *domain = engine->RegisterFrequencyDomain(
				"frequency domain", 1000);

	// Register event
	Event *event3 = engine->RegisterEvent("event 3", testHandler_4_3, domain);

	// Add event to the queue with priority
	q_4.Wait(event3, true);
}

// Tests to see if the events in the queue are executed properly with
// one event being a priority
TEST(TestEngine, test_event_queue_priority)
{
	try
	{
		// Cleanup pointers to singleton instances
		Cleanup();

		// Set up esim engine
		Engine *engine = Engine::getInstance();

		// Set up fast frequency domain
		FrequencyDomain *domain = engine->RegisterFrequencyDomain(
					"frequency domain", 1000);

		// Set up event
		Event *event1 = engine->RegisterEvent("event 1", testHandler_4_0, domain);

		// Set up event
		Event *event2 = engine->RegisterEvent("event 2", testHandler_4_2, domain);

		// Set frame
		auto frame_4_0 = misc::new_shared<DummyFrame_4_0>();

		// Set frame
		auto frame_4_1 = misc::new_shared<DummyFrame_4_1>();

		// Schedule event for 5 cycles from now
		engine->Call(event1, frame_4_0, nullptr, 5, 0);

		// Schedule event for 10 cycles from now
		engine->Call(event2, frame_4_1, nullptr, 10, 0);

		// Run simulation for 10 cycles
		for (int i = 0; i < 11; i++)
			engine->ProcessEvents();

		// Check that the handler has not been called
		EXPECT_FALSE(handler_called_4_1);
		EXPECT_FALSE(handler_called_4_3);

		// Wakeup all events in queue
		q_4.WakeupAll();

		// Run simulation for 1 cycle
		engine->ProcessEvents();

		// Check that handler has been called
		EXPECT_TRUE(handler_called_4_1);

		//Check that handler has been called
		EXPECT_TRUE(handler_called_4_3);

	}
	catch (misc::Exception &e)
	{
		e.Dump();
		FAIL();
	}
}

}





