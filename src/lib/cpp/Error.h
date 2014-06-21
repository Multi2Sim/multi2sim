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

#ifndef LIB_CPP_ERROR_H
#define LIB_CPP_ERROR_H

#include <stdexcept>


namespace misc
{

/// Base class for all errors produced by an invalid user input.
class Error : public std::runtime_error
{

public:

	/// Constructor
	///
	/// \param module
	///	Name of the module producing the error. This name will be
	///	displayed in brackets and preceed the error message.
	///
	/// \param message
	///	Error message. This message will be preceeded by string `Error:`
	Error(const std::string &module, const std::string &message) :
			std::runtime_error("[" + module + "] Error: " +
					message)
	{
	}
};

} // namespace misc

#endif

