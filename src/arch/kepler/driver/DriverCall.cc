/*
 *  Multi2Sim
 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#include <lib/cpp/String.h>
#include <memory/Memory.h>

#include "Driver.h"


namespace Kepler
{

// Driver version numbers
const int Driver::version_major = 2;
const int Driver::version_minor = 0;


/// ABI Call 'Init'
///
/// This function return the version information of the driver.
///
/// \param version_ptr
///	This argument is a pointer to a data structure in guest memory where
///	the driver will place version information. The data structure has the
///	following layout:
///
///	struct {
///		int major;
///		int minor;
///	};
///
/// \return
///	The function always returns 0.
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	unsigned version_ptr;
	memory->Read(args_ptr, 4, (char *) &version_ptr);

	// Debug
	debug << misc::fmt("\tversion_ptr = 0x%x\n", version_ptr);

	// Return version numbers
	memory->Write(version_ptr, 4, (char *) &version_major);
	memory->Write(version_ptr + 4, 4, (char *) &version_minor);

	// Success
	return 0;
}


/// ABI Call 'MemAlloc'
///
/// ...
int Driver::CallMemAlloc(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

/// ABI Call 'MemRead'
///
/// ...
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


/// ABI Call 'MemWrite'
///
/// ...
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


/// ABI Call 'LaunchKernel'
///
/// ...
int Driver::CallLaunchKernel(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


}  // namespace Kepler
