/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_DEBUG_H
#define LIB_CPP_DEBUG_H

#include <cassert>
#include <string>


namespace misc
{


/// Class used to dump debug information, activated when using one of the
/// <tt>--xxx-debug</tt> command-line options. The following steps should be
/// followed to add a new debug category:
///
/// - A public variable of type Debug must be defined, preferably as a public
///   static member of a class.
///
/// - When a command-line option is detected that activates that debug category,
///   the file name passed by the user to dump the debug information should be
///   activated with a call to setPath() on the Debug variable.
///
/// - The debug variable can be used as an output stream (similar to \c
///   std::cout) to dump debug information using the \c << operator. This
///   operator will only take effect if an output file was set for the debug
///   variable with a previous call to setPath().
///
/// - The debug variable can be used as a boolean expression (e.g. <tt>if
///   (debug)</tt>) to check whether it was activated with a call to setPath().
///   This can be useful to avoid formating debug information in
///   performance-critical code sections, if the debug category is disabled.
///
class Debug
{
	// Path to dump debug info
	std::string path;

	// Prefix used before every debug message
	std::string prefix;

	// Output stream
	std::ostream *os;

	// Flag indicating whether debug category is active
	bool active;

	// Close debugger
	void Close();

public:
	
	/// Constructor
	Debug();

	/// Destructor
	~Debug();

	/// Set the path to dump debug information. Special names
	/// \c stdout and \c stderr can be used to refer to the
	/// standard output and standard error output, respectively.
	void setPath(const std::string &path);

	/// Return the current path where the debug information is being dumped,
	/// as set with a previous call to setPath(). Strings \c stdout
	/// and \c stderr are returned if the path was set to the
	/// standard output or standard error output, respectively.
	const std::string &getPath() { return path; }

	/// Turn off debug
	void Off() { active = false; }

	/// Turn on debug
	void On() { active = true; }

	/// Dump a value into the output stream currently pointed to by the
	/// debug object. If the debugger has not been initialized with a call
	/// to setPath(), this call is ignored. The argument can be of any
	/// type accepted by an \c std::ostream object.
	template<typename T> Debug& operator<<(T val)
	{
		if (os && active)
			*os << prefix << val;
		Flush();
		return *this;
	}

	/// A debugger can be cast into a \c bool (e.g. within an \c if
	/// condition)
	/// to check whether it has an active output stream or not. This is
	/// useful when many possibly costly operations are performed just
	/// to dump debug information. By checking whether the debugger is
	/// active or not in beforehand, multiple dump \c << calls can be
	/// saved.
	operator bool() { return os; }

	/// A variable of type Debug can also be cast into an \c std::ostream
	/// object, returning a reference to its internal output stream. This
	/// operation is useful when passing the debug variable as an argument
	/// to \c Dump() functions that use an \c std::ostream reference. The
	/// user must make sure that this only happens when a proper output
	/// stream has been internally initialized after a call to setPath().
	operator std::ostream &()
	{
		assert(os);
		return *os;
	}

	/// Flush the internal stream
	void Flush();

	/// Set a prefix to be displayed before every debug message.
	void setPrefix(const std::string &prefix)
	{
		this->prefix = prefix.empty() ? "" : prefix + ' ';
	}
};



} // namespace misc

#endif
