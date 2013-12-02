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


#include "ABI.h"
#include "OpenCLDriver.h"

namespace Driver
{
	
std::unique_ptr<OpenCLDriver> OpenCLDriver::instance;

OpenCLDriver::OpenCLDriver()
{
	// Obtain instance of SI emulator
	// si_emu = SI::Emu::getInstance();
	x86_emu = x86::Emu::getInstance();
}

OpenCLDriver *OpenCLDriver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new OpenCLDriver());
	return instance.get();
}

void OpenCLDriver::RequestWork(SI::NDRange *ndrange)
{
	// FIXME
}

void OpenCLDriver::NDRangeComplete(SI::NDRange *ndrange)
{
	// FIXME
}

int OpenCLDriver::DriverCall()
{
	// FIXME
	// Get x86 context and read parameters, function code in ebx

	return 0;
}

}  // namespace Driver
