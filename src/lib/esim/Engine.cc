/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <csignal>

#include <lib/cpp/IniFile.h>

#include "Engine.h"
#include "Queue.h"


namespace esim
{

misc::Debug Engine::debug;

std::unique_ptr<Engine> Engine::instance;

const char *engine_err_finalization =
	"The finalization process of the event-driven simulation is trying to "
	"empty the event heap by scheduling all pending events. If the number of "
	"scheduled events during this process is too high, it could mean that "
	"some event is recursively scheduling other events, avoiding correct "
	"finalization of the simulation. Please contact development@multi2sim.org "
	"to report this error.\n";

const char *engine_err_max_inflight_events =
	"An excessive number of events are currently in-flight in the event-"
	"driven simulation library. This is probably the result of a timing "
	"model uncontrollably issuing events (typically a processor pipeline "
	"issuing memory accesses without checking for a limit in the number "
	"of in-flight accesses). "
	"If you still believe that this is an expected number of in-flight "
	"events, please increase the value of macro ESIM_OVERLOAD_EVENTS to "
	"avoid this warning. ";


Engine::Engine() : timer("esim::Timer")
{
	// Initialize timer
	timer.Start();

	// Create null event
	null_event = RegisterEvent("Null event", nullptr, nullptr);

	// Debug
	debug << "Event-driven simulation engine initialized\n";
}


void Engine::SignalHandler(int signum)
{
	// Get instance
	Engine *esim = getInstance();

	// If a signal SIGINT has been caught already and not processed, it is
	// time to not defer it anymore. Execution ends here.
	if (esim->signal_received == signum && signum == SIGINT)
	{
		std::cerr << "SIGINT received\n";
		exit(1);
	}

	// Just record that we are receiving a signal. It is not a good idea to
	// process it now, since we might be interfering some critical
	// execution. The signal will be processed at the end of the simulation
	// loop iteration.
	esim->signal_received = signum;
}


bool Engine::Drain(int max_events)
{
	// Keep track of the number of extracted events
	int num_events = 0;

	// Extract events
	while (1)
	{
		// No more elements in heap
		if (heap.size() == 0)
			return false;

		// Get frame from top of the heap
		assert(current_frame == nullptr);
		current_frame = heap.top();
		assert(current_frame->in_heap);

		// Extract from heap
		heap.pop();
		current_frame->in_heap = false;

		// Debug
		Event *event = current_frame->event;
		FrequencyDomain *frequency_domain = event->getFrequencyDomain();
		debug << misc::fmt("[%.2fns] Event '%s/%s' drained\n",
				(double) current_time / 1000,
				frequency_domain->getName().c_str(),
				event->getName().c_str());

		// Set current time to the time of the event
		current_time = current_frame->time;

		// One more events
		num_events++;

		// Run event handler
		EventHandler event_handler = event->getEventHandler();
		event_handler(event, current_frame.get());

		// Free frame
		current_frame = nullptr;

		// Interrupt heap draining after exceeding a given number of
		// events. This can happen if the event handlers of processed
		// events keep scheduling new events, causing the heap to never
		// finish draining.
		if (num_events == max_events)
			return true;
	}

	// Max not exceeded
	return false;
}


void Engine::ProcessEndEvents()
{
	while (end_frames.size())
	{
		// Dequeue frame from the head of the queue
		assert(current_frame == nullptr);
		current_frame = end_frames.front();
		end_frames.pop();

		// Debug
		Event *event = current_frame->event;
		debug << misc::fmt("[%.2fns] End event '%s' triggered\n",
				(double) current_time / 1000,
				event->getName().c_str());

		// Run event handler with null frame
		EventHandler event_handler = event->getEventHandler();
		event_handler(event, current_frame.get());

		// Free frame
		current_frame = nullptr;
	}
}


Engine *Engine::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance = misc::new_unique<Engine>();
	return instance.get();
}


void Engine::EnableSignals()
{
	signal(SIGINT, &SignalHandler);
	signal(SIGABRT, &SignalHandler);
	signal(SIGUSR1, &SignalHandler);
	signal(SIGUSR2, &SignalHandler);
}


void Engine::DisableSignals()
{
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, SIG_DFL);
}


void Engine::ProcessEvents()
{
	// Check for SIGINT signal
	if (signal_received == SIGINT)
	{
		std::cerr << "\nSignal SIGINT received\n";
		Finish("Signal");
	}
	
	// Process events scheduled for this cycle
	while (1)
	{
		// No more elements in heap
		if (heap.size() == 0)
			break;

		// Stop when we find the first event that should run in the
		// future.
		if (heap.top()->time > current_time)
			break;
		
		// Get frame from top of heap
		assert(current_frame == nullptr);
		current_frame = heap.top();
		assert(current_frame->in_heap);

		// Remove frame from the heap
		heap.pop();
		current_frame->in_heap = false;

		// Debug
		Event *event = current_frame->event;
		FrequencyDomain *frequency_domain = event->getFrequencyDomain();
		debug << misc::fmt("[%.2fns] Event '%s/%s' triggered\n",
				(double) current_time / 1000,
				frequency_domain->getName().c_str(),
				event->getName().c_str());

		// The event is being run, so decrement the number of in-flight
		// events of its type.
		event->decInFlight();

		// Run event handler
		EventHandler event_handler = event->getEventHandler();
		event_handler(event, current_frame.get());

		// Reschedule if it is periodic
		int period = current_frame->period;
		if (period > 0)
			Schedule(event, current_frame, period, period);

		// Free frame
		current_frame = nullptr;
	}
	
	// Next simulation cycle
	current_time += shortest_cycle_time;
}


FrequencyDomain *Engine::RegisterFrequencyDomain(const std::string &name,
		int frequency)
{
	// Create frequency domain
	frequency_domains.emplace_back(name, frequency);

	// Update fastest frequency domain
	if (frequency > fastest_frequency)
	{
		fastest_frequency = frequency;
		shortest_cycle_time = 1000000ll / frequency;
	}

	// Return created frequency domain
	return &frequency_domains.back();
}

void Engine::UpdateFastestFrequency()
{
	assert(frequency_domains.size());
	fastest_frequency = 0;
	shortest_cycle_time = 0;
	for (auto &frequency_domain : frequency_domains)
	{
		if (frequency_domain.getFrequency() > fastest_frequency)
		{
			fastest_frequency = frequency_domain.getFrequency();
			shortest_cycle_time = frequency_domain.getCycleTime();
		}
	}
}


Event *Engine::RegisterEvent(const std::string &name,
		EventHandler handler,
		FrequencyDomain *frequency_domain)
{
	events.emplace_back(name, handler, frequency_domain);
	return &events.back();
}
	
	
void Engine::Schedule(Event *event,
		std::shared_ptr<Frame> frame,
		int after,
		int period)
{
	// Sanity
	assert(after >= 0);
	assert(frame != nullptr);

	// Frame must not be suspended in a queue
	if (frame->in_queue)
		throw misc::Panic("Cannot schedule an event in an event chain "
				"that is currently suspended in a queue");

	// Frame must not be enqueued in the heap
	if (frame->in_heap)
		throw misc::Panic("Cannot schedule a second event for this "
				"event queue before the first event occurs. Do "
				"you have two consecutive calls to Next() or "
				"Call()?");

	// Ignore event if engine is locked
	if (locked)
		return;

	// Null event
	if (event == nullptr || event == null_event)
	{
		debug << misc::fmt("[%.2fns] Null event discarded\n",
				(double) current_time / 1000);
		return;
	}

	// Get frequency domain. All events scheduled here must belong to a
	// valid frequency domain.
	FrequencyDomain *frequency_domain = event->getFrequencyDomain();
	if (!frequency_domain)
		throw misc::Panic(misc::fmt("Event '%s' has been "
				"scheduled, but it does not belong to a "
				"valid frequency domain",
				event->getName().c_str()));

	// Calculate absolute time for the event based on the event's frequency
	// domain. First, get the actual current time for the current frequency
	// domain, then add the time after which the event should be scheduled.
	frame->time = current_time / frequency_domain->getCycleTime() *
			frequency_domain->getCycleTime() +
			frequency_domain->getCycleTime() * after;

	// Set event and period
	frame->event = event;
	frame->period = period;

	// Assign a schedule sequence number of the frame, use to disambiguate
	// the order of those events scheduled for the same cycle
	frame->schedule_sequence = ++schedule_sequence_counter;

	// Insert frame into the heap
	heap.emplace(frame);
	frame->in_heap = true;

	// Increment the number of in-flight events of this type.
	event->incInFlight();

	// Debug
	debug << misc::fmt("[%.2fns] Event '%s/%s' scheduled for [%.2fns]\n",
			(double) current_time / 1000,
			frequency_domain->getName().c_str(),
			event->getName().c_str(),
			(double) frame->time / 1000);

	// Warn when heap is overloaded
	if (!max_inflight_events_warning && (int) heap.size() >=
			max_inflight_events)
	{
		max_inflight_events_warning = true;
		misc::Warning("[esim] Maximum number of %d "
				"in-flight events exceeds\n\n%s",
				max_inflight_events,
				engine_err_max_inflight_events);
	}
}


void Engine::Next(Event *event,
		int after,
		int period)
{
	// Use current event's frame if this function is invoked within an
	// event handler, or create new frame otherwise.
	std::shared_ptr<Frame> frame = current_frame;
	if (!frame)
		frame = misc::new_shared<Frame>();

	// Schedule event
	Schedule(event, frame, after, period);
}


void Engine::Execute(Event *event, std::shared_ptr<Frame> frame,
		Event *receive_event)
{
	// Null event
	if (event == nullptr || event == null_event)
		return;

	// Save old current frame
	std::shared_ptr<Frame> old_current_frame = current_frame;

	// Create new frame if none exists
	frame->parent_frame = current_frame;
	frame->return_event = receive_event;
	current_frame = frame;

	// Execute event handler
	EventHandler event_handler = event->getEventHandler();
	event_handler(event, current_frame.get());

	// Restore previous current frame
	current_frame = old_current_frame;
}


void Engine::Call(Event *event,
		std::shared_ptr<Frame> frame,
		Event *return_event,
		int after,
		int period)
{
	// Create new frame if none passed
	if (frame == nullptr)
		frame = misc::new_shared<Frame>();

	// Set return event and frame
	frame->return_event = return_event;
	frame->parent_frame = current_frame;

	// Schedule event
	Schedule(event, frame, after, period);
}


void Engine::Return(int after)
{
	// This function must be invoked within an event handler
	if (!current_frame)
		throw misc::Panic("Function cannot be invoked outside of "
				"an event handler");

	// If this is the bottom of the stack, ignore
	if (current_frame->return_event == nullptr)
		return;

	// Schedule return event
	Schedule(current_frame->return_event,
			current_frame->parent_frame,
			after);
}


void Engine::EndEvent(Event *event)
{
	// Discard null event
	if (event == nullptr || event == null_event)
		return;
	
	// Create frame
	auto frame = misc::new_shared<Frame>();
	frame->event = event;

	// Add event to queue of end events
	end_frames.emplace(frame);
}


void Engine::ProcessAllEvents()
{
	// Drain event heap. If the maximum number of finalization events was
	// exceeded, issue a warning, and stop.
	bool overflow = Drain(max_finalization_events);
	if (overflow)
	{
		misc::Warning("[esim] Maximum number of finalization events "
				"exceeded\n\n%s",
				engine_err_finalization);
		return;
	}

	// Process events scheduled for the end of the simulation
	ProcessEndEvents();

	// Drain heap again
	Drain(max_finalization_events);
}



}  // namespace esim


