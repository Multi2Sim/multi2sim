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
#include "southern-islands/ABI.h"

using namespace SI;

namespace Driver
{

int OpenCLABIInitImpl()
{
	
	std::string spacename;

	struct OpenCLVersion
	{
		int major;
		int minor;
	};

	// Compare runtime version against driver
	// OpenCLDriver *driver = OpenCLDriver::getInstance();
	// OpenCLVersion version = 

	// Pass namespace as the 2nd parameter, use it to initialize function table

	// Initialize function table, ignore OpenCLABICallInvalid and OpenCLABICallInit
	if (spacename == "SI")
	{
		for (int i = 2; i < Driver::OpenCLABICallCount; ++i)
			Driver::OpenCLABICallTable[i] = OpenCLABICallTable[i];
	}

	// Return Success
	return 0;
}

}  // namespace Driver