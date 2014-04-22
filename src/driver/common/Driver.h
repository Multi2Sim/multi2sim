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

#ifndef DRIVER_COMMON_DRIVER_H
#define DRIVER_COMMON_DRIVER_H

#include <memory>

// Forward declaration
namespace x86
{
	class Emu;
	class Cpu;
	class Context;
}  // namespace x86

namespace Driver
{

class Common
{
protected:
	// Device x86 functional emulators, shared by all drivers
	static x86::Emu *x86_emu;

	// Device x86 timing simulators, shared by all drivers
	static x86::Cpu *x86_cpu;

public:
	Common();

	virtual int DriverCall(x86::Context *ctx, int ioctl_code) = 0;
};
	
}  // namespace Driver
#endif