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

#ifndef DRIVER_COMMON_DRIVER_HPP
#define DRIVER_COMMON_DRIVER_HPP

#include <memory>

// Forward declaration
namespace x86
{
class Emu;
class Cpu;
}  // namespace x86

namespace SI
{
class NDRange;
class Emu;
class Gpu;
}  // namespace SI

namespace Driver
{

class Common
{
protected:
	// Device functional emulators
	x86::Emu *x86_emu;

	// Device timing simulators
	x86::Cpu *x86_cpu;

public:
	Common();
};

class SICommon : Common
{
protected:
	// Device functional emulator
	SI::Emu *si_emu;

	// Device timing simulator
	SI::Gpu *si_gpu;

	// CPU/GPU fused device
	bool fused;

	// NDRange list is shared by OpenGL/CL driver
	static std::vector<std::unique_ptr<SI::NDRange>> ndranges;

public:
	SICommon();
};
	
}  // namespace Driver
#endif