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

#include "Driver.h"


namespace Kepler
{


/// ABI Call 'Init'
///
/// ...
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
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
