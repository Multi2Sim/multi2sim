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

#include <string>


namespace misc
{


class Debug
{
	std::string path;
	std::ostream *os;

	void Close();
public:
	
	/// Constructor
	Debug();

	/// Destructor
	~Debug();

	void setPath(const std::string &path);
	const std::string &getPath() { return path; }

	/* Dump a value into the output stream currently pointed to by the
	 * debug object. If the debugger has not been initialized with a call
	 * to SetPath(), this call is ignored. The argument can be of any
	 * type accepted by an std::ostream object. */
	template<typename T> Debug& operator<<(T val) {
		if (os)
			*os << val;
		return *this;
	}

	/* A debugger can be cast into a bool (e.g. within an 'if' condition)
	 * to check whether it has an active output stream or not. This is
	 * useful when many possibly costly operations are performed just
	 * to dump debug information. By checking whether the debugger is
	 * active or not in beforehand, multiple dump (<<) calls can be
	 * saved. */
	operator bool() { return os; }

	/* A debugger can also be cast into an std::ostream, returning a
	 * reference to its internal 'os' object. This should be done only when
	 * the output stream has been initialized properly with SetPath(). */
	operator std::ostream &() { return *os; }
};



} /* namespace Misc */

#endif
