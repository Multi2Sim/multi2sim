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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_DISASSEMBLER_H
#define ARCH_COMMON_DISASSEMBLER_H

#include <string>

namespace comm
{

class Disassembler
{
	// Name of disassembler
	std::string name;

public:

	/// Constructor
	Disassembler(const std::string &name);

	/// Check whether a token is found in the begninning of a format string.
	///
	/// \param fmt
	///	Format string in the beginning of which to search for the token.
	///
	/// \param token
	///	Token to search for.
	///
	/// \param length
	///	This argument is optional (see overloaded function isToken()).
	///	If specified, the function returns here the length of the token
	//	if found.
	static bool isToken(const std::string &fmt, const std::string &token,
			int &length);

	/// Alternative version of function isToken() where the length of the
	/// obtained token is not returned (argument \a length is omitted).
	static bool isToken(const std::string &fmt, const std::string &token)
	{
		int length;
		return isToken(fmt, token, length);
	}

};


}  // namespace comm

#endif

