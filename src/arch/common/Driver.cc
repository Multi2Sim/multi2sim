/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include <arch/kepler/driver/Driver.h>
#include <arch/hsa/driver/Driver.h>
#include <arch/southern-islands/driver/Driver.h>
#include <lib/cpp/Misc.h>

#include "Driver.h"


namespace comm
{


std::unique_ptr<DriverPool> DriverPool::instance;


DriverPool* DriverPool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new driver pool
	instance = misc::new_unique<DriverPool>();
	return instance.get();	
}


DriverPool::DriverPool()
{
	// HSA driver
	HSA::Driver *hsa_driver = HSA::Driver::getInstance();
	Register(hsa_driver);

	// Kepler driver
	Kepler::Driver *kepler_driver = Kepler::Driver::getInstance();
	Register(kepler_driver);

	// Southern Islands driver
	SI::Driver *si_driver = SI::Driver::getInstance();
	Register(si_driver);
}


void DriverPool::Register(Driver *driver)
{
	// Add it to the pool
	drivers.push_back(driver);
}


Driver *DriverPool::getDriverByPath(const std::string &path)
{
	// Traverse all drivers
	for (auto driver : drivers)
		if (driver->getPath() == path)
			return driver;
	
	// Not found
	return nullptr;
}


}  // namespace comm

