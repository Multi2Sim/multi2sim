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

#include "Action.h"
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


bool Engine::Drain(int max_events)
{
	// Keep track of the number of extracted events
	int num_events = 0;

	// Extract events
	while (1)
	{
		// No more elements in heap
		if (events.size() == 0)
			return false;

		// Get event from top of heap
		Event *event = events.top().get();

		// Debug
		EventType *event_type = event->getType();
		FrequencyDomain *frequency_domain = event_type->getFrequencyDomain();
		debug << misc::fmt("[%.2fns] Event '%s/%s' drained\n",
				(double) current_time / 1000,
				frequency_domain->getName().c_str(),
				event_type->getName().c_str());

		// Set current time to the time of the event
		current_time = event->getTime();

		// One more events
		num_events++;

		// Run event handler
		EventHandler event_handler = event_type->getEventHandler();
		current_event = event;
		event_handler(event_type, event->getFrame().get());
		current_event = nullptr;

		// Remove from heap
		events.pop();

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
	while (end_events.size())
	{
		// Get event from the head of the queue
		Event *event = end_events.front().get();

		// Debug
		EventType *event_type = event->getType();
		debug << misc::fmt("[%.2fns] End event '%s' triggered\n",
				(double) current_time / 1000,
				event_type->getName().c_str());

		// Run event handler with null frame
		EventHandler event_handler = event_type->getEventHandler();
		current_event = event;
		event_handler(event_type, nullptr);
		current_event = nullptr;

		// Dequeue
		end_events.pop();
	}
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

		// The event is being run, so decrement the number of in-flight
		// events of its type.
		event_type->decInFlight();

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
		EventHandler handler,
		FrequencyDomain *frequency_domain)
{
	event_types.emplace_back(name, handler, frequency_domain);
	return &event_types.back();
}
	
	
void Engine::Schedule(EventType *event_type,
		std::shared_ptr<EventFrame> event_frame,
		int after,
		int period)
{
	// Sanity
	assert(after >= 0);
	assert(event_frame != nullptr);

	// Frame must not be suspended in a queue
	if (event_frame->isInQueue())
		throw misc::Panic("Attempt to schedule an event "
				"that is currently suspended in a queue");

	// Ignore event if engine is locked
	if (locked)
		return;

	// Null event
	if (event_type == nullptr || event_type == null_event_type)
	{
		debug << misc::fmt("[%.2fns] Null event discarded\n",
				(double) current_time / 1000);
		return;
	}

	// Get frequency domain. All events scheduled here must belong to a
	// valid frequency domain.
	FrequencyDomain *frequency_domain = event_type->getFrequencyDomain();
	if (!frequency_domain)
		throw misc::Panic(misc::fmt("Event '%s' has been "
				"scheduled, but it does not belong to a "
				"valid frequency domain",
				event_type->getName().c_str()));

	// Calculate absolute time for the event based on the event's frequency
	// domain. First, get the actual current time for the current frequency
	// domain, then add the time after which the event should be scheduled.
	long long when = current_time / frequency_domain->getCycleTime() *
			frequency_domain->getCycleTime() +
			frequency_domain->getCycleTime() * after;

	// Create new event and insert it in the event list
	events.emplace(new Event(event_type, event_frame, when, period));

	// Increment the number of in-flight events of this type.
	event_type->incInFlight();

	// Debug
	debug << misc::fmt("[%.2fns] Event '%s/%s' scheduled for [%.2fns]\n",
			(double) current_time / 1000,
			frequency_domain->getName().c_str(),
			event_type->getName().c_str(),
			(double) when / 1000);

	// Warn when heap is overloaded
	if (!max_inflight_events_warning && (int) events.size() >=
			max_inflight_events)
	{
		max_inflight_events_warning = true;
		misc::Warning("[esim] Maximum number of %d "
				"in-flight events exceeds\n\n%s",
				max_inflight_events,
				engine_err_max_inflight_events);
	}
}


void Engine::Next(EventType *event_type,
		int after,
		int period)
{
	// Use current event's frame if this function is invoked within an
	// event handler, or create new frame otherwise.
	std::shared_ptr<EventFrame> event_frame;
	if (current_event)
		event_frame = current_event->getFrame();
	else
		event_frame.reset(new EventFrame);

	// Schedule event
	Schedule(event_type, event_frame, after, period);
}


void Engine::Execute(EventType *event_type)
{
	// Null event
	if (event_type == nullptr || event_type == null_event_type)
		return;

	// Use current event's frame if this function is invoked within an
	// event handler, or create new frame otherwise.
	std::shared_ptr<EventFrame> event_frame;
	if (current_event)
		event_frame = current_event->getFrame();
	else
		event_frame.reset(new EventFrame);

	// Create new event
	std::unique_ptr<Event> event(new Event(event_type, event_frame, 0, 0));

	// Save current event
	Event *old_event = current_event;
	current_event = event.get();

	// Execute event handler
	EventHandler event_handler = event_type->getEventHandler();
	event_handler(event_type, event_frame.get());

	// Restore current event
	current_event = old_event;
}


void Engine::Call(EventType *event_type,
		std::shared_ptr<EventFrame> event_frame,
		EventType *return_event_type,
		int after,
		int period)
{
	// Create new frame if none passed
	if (event_frame == nullptr)
		event_frame.reset(new EventFrame);

	// Set return event
	event_frame->setReturnEventType(return_event_type);

	// Set return frame to the current event's frame, if this function is
	// being invoked from an event handler
	if (current_event)
		event_frame->setParentFrame(current_event->getFrame());

	// Schedule event
	Schedule(event_type, event_frame, after, period);
}


void Engine::Return(int after)
{
	// This function must be invoked within an event handler
	if (!current_event)
		throw misc::Panic("Function cannot be invoked outside of "
				"an event handler");

	// If this is the bottom of the stack, ignore
	if (current_event->getFrame()->getReturnEventType() == nullptr)
		return;

	// Schedule return event
	assert(current_event->getFrame() != nullptr);
	Schedule(current_event->getFrame()->getReturnEventType(),
			current_event->getFrame()->getParentFrame(),
			after);
}


void Engine::EndEvent(EventType *event_type)
{
	// Discard null event
	if (event_type == nullptr || event_type == null_event_type)
		return;

	// Add event to queue of end events
	end_events.emplace(new Event(event_type, nullptr, 0, 0));
}


EventFrame *Engine::getParentFrame()
{
	// Check that we are in an event handler
	if (!current_event)
		throw misc::Panic(misc::fmt("Function %s invoked "
				"outside of an event handler", __FUNCTION__));

	// Return parent frame
	return current_event->getFrame()->getParentFrame().get();
}


void Engine::ParseConfiguration(const std::string &path)
{
	std::map<std::string, FrequencyDomain *> domains;
	std::map<std::string, EventType *> events;

	// Parse the config file as an ini file.
	misc::IniFile config_file(path);

	// Find the 'Actions' section.
	if (!config_file.Exists("Actions"))
	{
		throw misc::Panic(misc::fmt("Esim Actions section "
				"not found in configuration file."));
	}

	// Find and parse each action.  Start at 'Action[0]' and continue
	// sequentially until one isn't found.
	int i = 0;
	while (1)
	{
		// Get the next action.
		std::string current_action = config_file.ReadString("Actions",
				misc::fmt("Action[%d]", i));

		// Check that the action exists.
		if (current_action == "")
			break;

		// Parse the action.
		ParseAction(current_action, domains, events);

		// Increment the action number.
		i++;
	}

	// Register an event for the end to do the checks.
	EventType *check_event = RegisterEventType("EV_DO_CHECKS",
			EventChecks::DoChecksHandler);
	EndEvent(check_event);
}


void Engine::ParseAction(const std::string &line,
		std::map<std::string, FrequencyDomain *> &domains,
		std::map<std::string, EventType *> &events)
{
	// Parse the configuration line to a list of tokens.
	std::vector<std::string> tokens;
	misc::StringTokenize(line, tokens);

	// Every action type has 3 arguments, so assert that we have 3 tokens.
	if (tokens.size() != 3)
		throw Error(misc::fmt("Malformed action.\n"
				"Expected 3 tokens, but instead received %d.\n"
				"\t> %s",
				(int) tokens.size(),
				line.c_str()));

	// Get the action type and parse it accordingly.
	if (tokens[0] == "FrequencyDomain")
		ParseActionDomainRegistration(tokens, domains);
	else if (tokens[0] == "EventType")
		ParseActionEventRegistration(tokens, events, domains);
	else if (tokens[0] == "Event")
		ParseActionEventSchedule(tokens, events);
	else if (tokens[0] == "CheckEvent")
		ParseActionCreateCheck(tokens);
	else
		// Invalid action type.
		throw Error(misc::fmt("Malformed action.\n"
				"Invalid action type '%s'.\n"
				"\t>%s", tokens[0].c_str(), line.c_str()));
}


void Engine::ParseActionDomainRegistration(const std::vector<std::string> &tokens,
		std::map<std::string, FrequencyDomain *> &domains)
{
	// Pull out the action parameters.
	std::string name = tokens[1];
	int frequency = misc::StringToInt(tokens[2]);

	// Register a new domain and store it.
	FrequencyDomain *domain = RegisterFrequencyDomain(name, frequency);
	domains[name] = domain;
}


void Engine::ParseActionEventRegistration(const std::vector<std::string> &tokens,
		std::map<std::string, EventType *> &events,
		std::map<std::string, FrequencyDomain *> &domains)
{
	// Pull out the action parameters.
	std::string name = tokens[1];
	std::string domain = tokens[2];

	// Register a new event type and store it.
	EventType *event = RegisterEventType(name,
			EventChecks::ActionEventHandler, domains[domain]);
	events[name] = event;
}


void Engine::ParseActionEventSchedule(const std::vector<std::string> &tokens,
		std::map<std::string, EventType *> &events)
{
	// Pull out the action parameters.
	std::string name = tokens[1];
	int cycle = misc::StringToInt(tokens[2]);

	// Schedule an event.
	Call(events[name], nullptr, nullptr, cycle);
}


void Engine::ParseActionCreateCheck(const std::vector<std::string> &tokens)
{
	// Pull out the action parameters.
	std::string name = tokens[1];
	long long time = misc::StringToInt64(tokens[2]);

	// Store the check to process later.
	EventChecks *checks = EventChecks::getInstance();
	EventInfo event(name, time);
	checks->AddCheck(event);
}


void Engine::TestLoop(const std::string &config_path)
{
	// Capture errors from parsing config and running esim
	try
	{
		// Process the configuration file.
		ParseConfiguration(config_path);

		// Run the simulation until there are no more events left.
		while (events.size() != 0)
			ProcessEvents();

		// Process the end events.
		ProcessAllEvents();
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		exit(1);
	}
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

