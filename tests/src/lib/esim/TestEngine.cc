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

bool handler_called = false;

void test_handler(EventType *type, EventFrame *frame)
{
	handler_called = true;
}

TEST(TestEngine, should_be_able_to_schedule_event)
{
	// Set up esim engine
	Engine *engine = Engine::getInstance();
	FrequencyDomain domain("Test frequency domain", 2e3);
	EventType type("test event", test_handler, &domain);
	engine->RegisterFrequencyDomain("Test frequency domain", 2e3);

	// Schedule next event in next 10 events
	engine->Next(&type, 10, 0);

	// Process events should go to next cycle, and the scheduled
	// event should not be triggered
	long long cycle_before = engine->getCycle();
	engine->ProcessEvents();
	long long cycle_after = engine->getCycle();
	EXPECT_EQ(1, cycle_after - cycle_before);
	EXPECT_FALSE(handler_called);

	// Skip for 10 cycles, the handler should be invoked 
	for (int i = 0; i < 10; i++)
		engine->ProcessEvents();
	cycle_after = engine->getCycle();
	EXPECT_EQ(11, cycle_after - cycle_before);
	EXPECT_TRUE(handler_called);
}

class MockupEventFrame : public EventFrame
{
public:
	int counter = 0;
};

void test_handler_2(EventType *type, EventFrame *frame)
{
	MockupEventFrame *data = dynamic_cast<MockupEventFrame *>(frame);
	data->counter++;
}

TEST(TestEngine, should_able_to_schedule_event_with_event_frame)
{
	// Set frame
	auto frame = misc::new_shared<MockupEventFrame>();

	// Set up esim engine
	Engine *engine = Engine::getInstance();
	FrequencyDomain domain("Test frequency domain", 2e3);
	EventType type("test event", test_handler_2, &domain);
	engine->RegisterFrequencyDomain("Test frequency domain", 2e3);

	// Schedule next event 
	engine->Schedule(&type, frame, 0);

	// Process event
	engine->ProcessEvents();

	// Assertions
	EXPECT_EQ(1, frame->counter);
}

}


