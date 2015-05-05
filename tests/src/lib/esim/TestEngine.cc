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
	Engine *engine = Engine::getInstance();
	FrequencyDomain domain("Test frequency domain", 2e3);
	EventType type("test event", test_handler, &domain);
	engine->RegisterFrequencyDomain("Test frequency domain", 2e3);
	engine->Next(&type, 10, 0);

	long long cycle_before = engine->getCycle();
	engine->ProcessEvents();
	long long cycle_after = engine->getCycle();
	EXPECT_EQ(1, cycle_after - cycle_before);

	for (int i = 0; i < 10; i++)
	{
		engine->ProcessEvents();
	}
	cycle_after = engine->getCycle();
	EXPECT_EQ(11, cycle_after - cycle_before);
	EXPECT_TRUE(handler_called);
}

}


