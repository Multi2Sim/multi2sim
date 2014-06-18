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

#include "Engine.h"


namespace esim
{

misc::Debug Engine::debug;

std::unique_ptr<Engine> Engine::instance;


Engine::Engine()
		:
		timer("esim::Timer")
{
	// Initialize timer
	timer.Start();

	// Create null event
	null_event_type = RegisterEventType("Null event", nullptr, nullptr);

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


Engine *Engine::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new Engine());
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
		if (events.size() == 0)
			break;

		// Get event from top of heap
		Event *event = events.top().get();

		// Stop when we find the first event that should run in the
		// future.
		if (event->getTime() > current_time)
			break;
		
		// Debug
		EventType *event_type = event->getType();
		FrequencyDomain *frequency_domain = event_type->getFrequencyDomain();
		debug << misc::fmt("[%.2fns] Event '%s/%s' triggered\n",
				(double) current_time / 1000,
				frequency_domain->getName().c_str(),
				event_type->getName().c_str());

		// Run event handler
		EventHandler event_handler = event_type->getEventHandler();
		current_event = event;
		event_handler(event_type, event->getFrame().get());
		current_event = nullptr;

		// Reschedule event if it is periodic
		int period = event->getPeriod();
		if (period > 0)
			Schedule(event_type, event->getFrame(), period, period);

		// Remove from heap
		events.pop();
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


EventType *Engine::RegisterEventType(const std::string &name,
		FrequencyDomain *frequency_domain,
		EventHandler handler)
{
	event_types.emplace_back(name, frequency_domain, handler);
	return &event_types.back();
}
	
	
void Engine::Schedule(EventType *event_type,
		const std::shared_ptr<EventFrame> &event_frame,
		int after,
		int period)
{
	// Event cannot be null. Empty events should be scheduled using
	// null_event instead.
	assert(event_type);
	assert(after >= 0);

	// Ignore event if engine is locked
	if (locked)
		return;
	
	// Special event to be ignored
	if (event_type == null_event_type)
	{
		debug << misc::fmt("[%.2fns] Null event discarded\n",
				(double) current_time / 1000);
		return;
	}
	
	// Calculate absolute time for the event based on the event's frequency
	// domain. First, get the actual current time for the current frequency
	// domain, then add the time after which the event should be scheduled.
	FrequencyDomain *frequency_domain = event_type->getFrequencyDomain();
	long long when = current_time / frequency_domain->getCycleTime() *
			frequency_domain->getCycleTime() +
			frequency_domain->getCycleTime() * after;
	
	// Create new event and insert it in the event list
	events.emplace(new Event(event_type, event_frame, when, period));
		
	// Debug
	debug << misc::fmt("[%.2fns] Event '%s/%s' scheduled for [%.2fns]\n",
			(double) current_time / 1000,
			frequency_domain->getName().c_str(),
			event_type->getName().c_str(),
			(double) when / 1000);

	/* Warn when heap is overloaded FIXME */
/*	if (!esim_overload_shown && esim_event_heap->count >= ESIM_OVERLOAD_EVENTS)
	{
		esim_overload_shown = 1;
		warning("%s: number of in-flight events exceeds %d.\n%s",
			__FUNCTION__, ESIM_OVERLOAD_EVENTS, esim_err_overload);
	}*/
}


void Engine::Call(EventType *event_type,
		const std::shared_ptr<EventFrame> &event_frame,
		int after,
		EventType *return_event_type)
{
	// Remember return event type and frame
	assert(current_event->getFrame() != nullptr);
	event_frame->setParentFrame(current_event->getFrame());
	event_frame->setReturnEventType(return_event_type);

	// Schedule event
	Schedule(event_type, event_frame, after);
}


void Engine::Return(int after)
{
	// Check for stack underflow
	if (current_event->getFrame()->getParentFrame() == nullptr)
	{
		// Throw exception
		EventType *event_type = current_event->getType();
		FrequencyDomain *frequency_domain =
				event_type->getFrequencyDomain();
		throw Exception(misc::fmt("Event stack underflow\n\n"
				"\tCurrent time:            %lldps\n"
				"\tFrequency domain:        %s\n"
				"\tCurrent cycle in domain: %lld\n"
				"\tCurrent event:           %s\n",
				current_time,
				frequency_domain->getName().c_str(),
				frequency_domain->getCycle(),
				event_type->getName().c_str()));
	}

	// Schedule return event
	Schedule(current_event->getFrame()->getReturnEventType(),
			current_event->getFrame()->getParentFrame(),
			after);
}


}  // namespace esim

