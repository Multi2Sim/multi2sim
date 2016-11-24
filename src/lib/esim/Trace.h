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

#ifndef LIB_CPP_ESIM_TRACE_H
#define LIB_CPP_ESIM_TRACE_H

#include <memory>
#include <string>
#include <sstream>
#include <zlib.h>


namespace esim
{

class TraceSystem
{
	// Unique trace system instance
	static std::unique_ptr<TraceSystem> instance;

	// Path of trace file
	std::string path;

	// Flag indicating whether trace is active
	bool active = false;

	// ZIP file object
	gzFile gz_file;

	// Last cycle when a trace message was printed
	long long last_cycle = -1;

	// Write a message to the trace file. If argument 'print_cycle' is set,
	// a line with the current cycle will be printed if this is the first
	// message for the cycle. The trace system must be active.
	void Write(const std::string &s, bool print_cycle = true);

public:

	/// Return trace system singleton.
	static TraceSystem *getInstance();

	/// Destructor
	~TraceSystem();

	/// Activate the trace system and set the output ZIP trace file to the
	/// given path.
	void setPath(const std::string &path);

	/// Return whether trace system has been activated by the user
	bool isActive() const { return active; }

	/// Dump a message to the trace system if it was activated with a
	/// previous call to setPath(). A line with the current cycle will be
	/// printed if this is the first message for it.
	template<typename T> TraceSystem& operator<<(T value)
	{
		// Convert to string and write
		if (active)
		{
			std::ostringstream os;
			os << value;
			Write(os.str());
		}

		// Return reference to this for chaining
		return *this;
	}
	
	/// Write a line of output in the beginning of the trace file. This
	/// function must be invoked before dumping trace information with
	/// the '<<' operator, that is, before cycle 1 begins in the trace.
	void Header(const std::string &s);
};

class Trace
{
	// Flag indicating whether this trace object is active
	bool active = true;

	// Associated trace system
	TraceSystem *trace_system = nullptr;

public:

	/// Constructor
	Trace();
	
	/// Turn off tracing
	void Off() { active = false; }

	/// Turn on tracing
	void On() { active = true; }

	/// Dump a header into the trace file. Headers are lines printed before
	/// the first cycle. Headers can only be printed before regular cycle-
	/// by-cycle information has been dumped with the "<<" operator.
	void Header(const std::string &s)
	{
		if (trace_system->isActive() && active)
			trace_system->Header(s);
	}

	/// Dump a value into the trace file if both the current trace object
	/// and the trace system are active. This call will cause the current
	/// cycle to be printed in the trace file if it is the first message
	/// for this cycle.
	/// The argument can be of any type accepted by \c std::ostream.
	template<typename T> Trace& operator<<(T value)
	{
		if (active)
			*trace_system << value;
		return *this;
	}

	/// A trace object can be cast into a \c bool (e.g. within an \c if
	/// condition) to check whether it is active or not. This is
	/// useful when many possibly costly operations are performed just
	/// to dump trace information. By checking whether the trace is
	/// active or not in beforehand, multiple dump \c << calls can be
	/// saved.
	operator bool() const { return active && trace_system->isActive(); }
};


}  // namespace esim

#endif

