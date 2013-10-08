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

#ifndef ARCH_COMMON_ASM_H
#define ARCH_COMMON_ASM_H

#include <string>

namespace Common
{

class Asm
{
public:
	
	/* Return if 'token' is found in the beginning of 'fmt'. If so, the
	 * length of 'token' is returned in optional argument 'length'. */
	static bool IsToken(const std::string &fmt, const std::string &token);
	static bool IsToken(const std::string &fmt, const std::string &token,
			int &length);
};


} /* namespace Common */

#endif

