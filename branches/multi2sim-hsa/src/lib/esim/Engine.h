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
#include <vector>
#include <map>

#include <lib/cpp/Debug.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/String.h>
#include <lib/cpp/Timer.h>

#include "Event.h"
#include "FrequencyDomain.h"


namespace esim
{

// Forward declarations
class Queue;


/// Exception object produced by objects in the event-driven simulator
class Error : public misc::Error
{

public:

	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		// Add module prefix
		AppendPrefix("esim");
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

	// Queue of end events
	std::queue<std::unique_ptr<Event>> end_events;

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

	// Number of in-flight events before a warning is shown (10k events)
	const int max_inflight_events = 10000;

	// Flag indicating whether a warning has been shown after the maximum
	// number of in-flight events has been exceeded. If true, the warning
	// will not be shown again.
	bool max_inflight_events_warning = false;

	// Number of events to process in Drain() to empty the event heap at
	// the end of the simulation before it is assumed that there is a
	// recursive queuing of events that will cause an infinite loop
	// (1M events).
	const int max_finalization_events = 1000000;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	Engine();

	// Signals received from the user are captured by this function
	static void SignalHandler(int sig);

	// Called by ParseConfiguration to process an action line.
	void ParseAction(const std::string &line,
			std::map<std::string, FrequencyDomain *> &domains,
			std::map<std::string, EventType *> &events);
	void ParseActionDomainRegistration(const std::vector<std::string> &tokens,
			std::map<std::string, FrequencyDomain *> &domains);
	void ParseActionEventRegistration(const std::vector<std::string> &tokens,
			std::map<std::string, EventType *> &events,
			std::map<std::string, FrequencyDomain *> &domains);
	void ParseActionEventSchedule(const std::vector<std::string> &tokens,
			std::map<std::string, EventType *> &events);
	void ParseActionCreateCheck(const std::vector<std::string> &tokens);

	// Drain the event heap, with a maximum number of events specified in
	// the argument. If this number is exceeded, the function returns true.
	// If the heap is drained successfully, the function returns false.
	// Periodic events will not be rescheduled in this process.
	bool Drain(int max_events);

	// Process all events scheduled with a previous call to EndEvent()
	void ProcessEndEvents();

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

	/// Function invoked in every iteration of the main simulation loop.
	/// This function processes all events scheduled for the current cycle
	/// and advances the event-driven simulation time.
	void ProcessEvents();

	/// Function invoked after the main simulation loop has finished. The
	/// function processes all events remaining in the heap and then runs
	/// all events that were scheduled for the end of the simulation with
	/// previous calls to EndEvent().
	void ProcessAllEvents();

	/// Return the current simulated time in picoseconds.
	long long getTime() const { return current_time; }

	/// Return a null event type. This type can be used in calls to
	/// ScheduleEvent() to schedule useless events.
	EventType *getNullEventType() const { return null_event_type; }

	/// If an event handler is currently executing, return the corresponding
	/// event. If no event handler is executing, return `nullptr`.
	Event *getCurrentEvent() { return current_event; }

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
	/// \param handler
	///	Function to execute when the event is triggered.
	///
	/// \param frequency_domain
	///	Frequency domain for the event type, as returned by a previous
	///	call to RegisterFrequencyDomain().  This is only allowed to
	///	left equal to nullptr for events that will only be scheduled
	///	with EndEvent().
	///
	/// \return
	///	This function returns a new object of type EvenType, which can
	///	be used later in calls to ScheduleEvent().
	EventType *RegisterEventType(const std::string &name,
			EventHandler handler,
			FrequencyDomain *frequency_domain = nullptr);

	/// Schedule an event. This function is only used internally and should
	/// not be invoked from outside of this library. Use Call() or Next()
	/// instead. See Next() for the meaning of the arguments.
	void Schedule(EventType *event_type,
			std::shared_ptr<EventFrame> event_frame,
			int after = 0,
			int period = 0);

	/// Schedule an event, using the event frame set by the last invocation
	/// to Call() in the event chain, or nullptr if no event frame was set
	/// before.
	///
	/// \param event_type
	///	Type of event to schedule.
	///
	/// \param after (optional)
	///	Number of cycles after which the event will execute.
	///
	///	A default value of 0 makes the event execute in the current
	///	cycle, but not immediately. The event is normally enqueued in
	///	the event heap and later in the same cycle extracted and run.
	///
	///	When multiple events are scheduled for the same cycle, there is
	///	no guarantee about the order in which they will execute.
	///
	/// \param period (optional)
	///	If specified, the event will be scheduled periodically after its
	///	first occurrence. The period is given in number of cycles with
	///	respect to the event's frequency domain.
	void Next(EventType *event_type,
			int after = 0,
			int period = 0);

	/// Execute an event synchronously. This function executes the event
	/// handler with the current frame right away, as opposed to function
	/// Next() with a value of \a after equal to 0, which would insert it
	/// in the event heap for a deferred execution.
	///
	/// \param event_type
	///	Type of event to execute
	void Execute(EventType *event_type);

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
	/// \param return_event_type
	///	During the execution of the event handler of \a event_type, an
	///	invocation to Return() will cause \a return_event_type to be
	///	scheduled, using the current frame as the event data.
	///
	/// \param after (optional)
	///	Number of cycles after which the called event will execute. See
	///	Next() for details.
	///
	/// \param period (optional)
	///	If specified, the event will be scheduled periodically after its
	///	first occurrence. The period is given in number of cycles with
	///	respect to the event's frequency domain.
	void Call(EventType *event_type,
			std::shared_ptr<EventFrame> event_frame = nullptr,
			EventType *return_event_type = nullptr,
			int after = 0,
			int period = 0);

	/// Schedule the return event specified in the last invocation to
	/// Call() in argument \a return_event_type, using the frame that was
	/// active at that time. This function should only be invoked in the
	/// body of an event handler.
	///
	/// \param after (optional)
	///	Number of cycles after which the return event will execute. See
	///	Schedule() for details.
	void Return(int after = 0);

	/// Schedule an event for the end of the simulation. End events have
	/// no event frame (event frame set to `nullptr`).
	void EndEvent(EventType *event_type);

	/// Return the parent frame in the event stack of the current event
	/// chain, or `nullptr` if the current event is in the bottom of the
	/// stack. This function should be invoked only within an event handler.
	EventFrame *getParentFrame();

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

	/// Run a test event-driven simulation with domains, events and checks
	/// passed in a configuration file.

	/// \param config_path
	/// Path to the configuration file to load.
	void TestLoop(const std::string &config_path);
};


}  // namespace esim

#endif

