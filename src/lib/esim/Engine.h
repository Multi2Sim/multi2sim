/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_ESIM_ENGINE_H
#define LIB_CPP_ESIM_ENGINE_H

#include <memory>
#include <list>
#include <queue>
#include <stdexcept>
#include <vector>

#include <lib/cpp/Debug.h>
#include <lib/cpp/String.h>
#include <lib/cpp/Timer.h>

#include "Event.h"
#include "FrequencyDomain.h"


namespace esim
{

/// Exception object produced by objects in the event-driven simulator
class Exception : std::runtime_error
{

public:

	/// Constructor
	Exception(const std::string &message)
			:
			std::runtime_error("[esim] " + message)
	{
	}
};


/// Event-driven simulator engine
class Engine
{
	// Unique instance of this class
	static std::unique_ptr<Engine> instance;

	/// Debugger
	static misc::Debug debug;

	// Flag set when simulation should finish
	bool finish = false;

	// Reason why we finish
	std::string finish_why;

	// Signal received from the user
	volatile int signal_received = false;

	// Global timer
	misc::Timer timer;

	// Registered event types
	std::list<EventType> event_types;

	// Registered frequency domains
	std::list<FrequencyDomain> frequency_domains;

	// Heap of pending events
	std::priority_queue<std::unique_ptr<Event>,
			std::vector<std::unique_ptr<Event>>,
			Event::CompareUniquePtrs> events;

	// Null event type used to schedule useless events
	EventType *null_event_type = nullptr;

	// If true, no new events will be scheduled
	bool locked = false;

	// Current simulation time in picoseconds
	long long current_time = 0;

	// Frequency of the fastest frequency domain
	int fastest_frequency = 0;

	// Cycle time of the fastest frequency domain
	long long shortest_cycle_time = 0;

	// Event whose handler is currently being executed
	Event *current_event = nullptr;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	Engine();

	// Signals received from the user are captured by this function
	static void SignalHandler(int sig);

public:

	/// Obtain the instance of the event-driven simulator singleton.
	static Engine *getInstance();

	/// Force end of simulation with a specific reason.
	void Finish(const std::string &why)
	{
		finish = true;
		finish_why = why;
	}

	/// Return whether the simulation finished
	bool hasFinished() { return finish; }

	/// Return the number of micro-seconds ellapsed since the start of the
	/// simulation. This is real time, not simulated time.
	long long getRealTime() { return timer.getValue(); }

	/// Intercept signals received by the user, and invoke the internal
	/// SignalHandler() function. Signals are enabled by default, as soon as
	/// the event-driven simulator is instantiated.
	void EnableSignals();

	/// Disable signal interception, and restore the default handlers.
	void DisableSignals();

	/// Function invoked in every iteration of the main simulation loop
	void ProcessEvents();

	/// Return the current simulated time in picoseconds.
	long long getTime() const { return current_time; }

	/// Return a null event type. This type can be used in calls to
	/// ScheduleEvent() to schedule useless events.
	EventType *getNullEventType() const { return null_event_type; }

	/// Register a new frequency domain.
	///
	/// \param name
	///	Name of the frequency domain
	///
	/// \param frequency
	///	Frequency in MHz.
	///
	/// \return
	///	This function returns an object of type FrequencyDomain, which
	///	can be used later in calls to RegisterEventType().
	FrequencyDomain *RegisterFrequencyDomain(const std::string &name,
			int frequency);

	/// Register a new event type.
	///
	/// \param name
	///	Name of the event type
	///
	/// \param frequency_domain
	///	Frequency domain for the event type, as returned by a previous
	///	call to RegisterFrequencyDomain().
	///
	/// \param handler
	///	Function to execute when the event is triggered.
	///
	/// \return
	///	This function returns a new object of type EvenType, which can
	///	be used later in calls to ScheduleEvent().
	EventType *RegisterEventType(const std::string &name,
			FrequencyDomain *frequency_domain,
			EventHandler handler);

	/// Schedule an event.
	///
	/// \param event_type
	///	Type of event to schedule. A null event (event with no effect)
	///	can be scheduled using the result of getNullEventType() for this
	///	argument.
	///
	/// \param event_frame
	///	Data associated with the event, given as a shared pointer. The
	///	event-driven simulation framework will free the event data when
	///	the last reference to it disappears.
	///
	/// \param after
	///	Number of cycles after which the event will execute. A value of
	///	0 will not execute the event immediately, but instead will
	///	schedule it for the end of the current cycle. When multiple
	///	events are scheduled for the same cycle, there is no guarantee
	///	in which order they will execute.
	///
	/// \param period (optional)
	///	If specified, the event will be scheduled periodically after its
	///	first occurence. The period is given in number of cycles with
	///	respect to the event's frequency domain.
	void Schedule(EventType *event_type,
			const std::shared_ptr<EventFrame> &event_frame,
			int after,
			int period = 0);

	/// Schedule an event, remembering the current event frame. This
	/// function should only be invoked in the body of an event handler.
	///
	/// \param event_type
	///	Type of event to schedule
	///
	/// \param event_frame
	///	Data associated with the event, given as a shared pointer. This
	///	object will be freed automatically when the last reference to
	///	it disappears.
	///
	/// \param after
	///	Number of cycles after which the called event will execute. See
	///	Schedule() for details.
	///
	/// \param return_event_type
	///	During the execution of the event handler of \a event_type, an
	///	invocation to Return() will cause \a return_event_type to be
	///	scheduled, using the current frame as the event data.
	void Call(EventType *event_type,
			const std::shared_ptr<EventFrame> &event_frame,
			int after,
			EventType *return_event_type);

	/// Schedule the return event specified in the last invocation to
	/// Call() in argument \a return_event_type, using the frame that was
	/// active at that time. This function should only be invoked in the
	/// body of an event
	///
	/// \param after
	///	Number of cycles after which the return event will execute. See
	///	Schedule() for details.
	void Return(int after);

	/// Activate debug information for the event-driven simulator.
	///
	/// \param path
	///	Path to dump debug information. Strings \c stdout and \c stderr
	///	are special values referring to the standard output and standard
	///	error output, respectively.
	static void setDebugPath(const std::string &path)
	{
		debug.setPath(path);
		debug.setPrefix("[esim]");
	}

	/// Load and process a configuration file for event-driven simulation.
	///
	/// \param path
	///	Path to the configuration file to load.
	void ParseConfiguration(const std::string &path);
};


}  // namespace esim

#endif

