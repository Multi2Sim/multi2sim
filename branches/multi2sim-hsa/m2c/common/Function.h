/*
 *  Multi2Sim
 *  Copyright (C) 2014  Nathan Lilienthal (nathan@nixpulvis.com)
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

#ifndef M2C_COMMON_FUNCTION_H
#define M2C_COMMON_FUNCTION_H

#include "PassInfo.h"
#include "PassInfoPool.h"


namespace comm
{

/// This class is used as a base class representing a function of any
/// architecture. Each back-end should derive its own function class.
class Function
{
	// PassInfoPool containing the info per pass for the Function.
	PassInfoPool<FunctionPassInfo> pass_info_pool;

public:

	/// Virtual destructor
	virtual ~Function() { }

};

}  // namespace comm

#endif
