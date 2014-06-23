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

#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <lib/cpp/Debug.h>


namespace mem
{

/// Memory system
class System
{
public:

	/// Memory system trace
	static misc::Debug trace;

	/// Memory system debugger
	static misc::Debug debug;

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

};

}  // namespace mem

#endif

